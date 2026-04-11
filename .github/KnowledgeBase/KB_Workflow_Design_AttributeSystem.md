# Workflow Attribute System

## How Workflow script attributes map to the C++ reflection attribute infrastructure, including naming conventions, predefined attributes, assembly population, and binary serialization.

## Attribute Naming Convention

In Workflow script, attributes are written as `@category:name` (e.g., `@cpp:File`, `@cpp:UserImpl`).
The compiler translates this syntax into a reflected struct type name using a fixed pattern:

```
@category:name  →  system::workflow_attributes::att_category_name
```

This translation is performed by `WfLexicalScopeManager::GetWorkflowAttributeTypeName(category, name)` in `Source/Analyzer/WfAnalyzer.cpp`.
For example:
- `@cpp:File` → `system::workflow_attributes::att_cpp_File`
- `@cpp:UserImpl` → `system::workflow_attributes::att_cpp_UserImpl`
- `@test:Int` → `system::workflow_attributes::att_test_Int`

## Attribute Resolution

`WfLexicalScopeManager::ResolveWorkflowAttribute(category, name)` in `Source/Analyzer/WfAnalyzer.cpp` resolves an attribute to its `ITypeDescriptor`.
It validates:
- The type must exist and be a struct (`TypeDescriptorFlags::Struct`).
- The struct must have zero or one property. Zero properties means no argument; one property defines the argument type.

## Predefined Attributes

The following attributes are predefined in `Source/Library/WfLibraryPredefined.h` under the `vl::__vwsn` namespace.
They are registered in `Source/Library/WfLibraryReflection.cpp` with `IMPL_TYPE_INFO_RENAME` to map to the `system::workflow_attributes::*` type names.

| Workflow Syntax     | Type Name                                       | Argument         | Purpose                                                  |
|---------------------|-------------------------------------------------|------------------|----------------------------------------------------------|
| `@cpp:File`         | `system::workflow_attributes::att_cpp_File`     | `WString`        | Controls which generated C++ file a declaration goes to  |
| `@cpp:UserImpl`     | `system::workflow_attributes::att_cpp_UserImpl` | (none)           | Marks a method as user-implemented in generated C++      |
| `@cpp:Private`      | `system::workflow_attributes::att_cpp_Private`  | (none)           | Makes a member private in generated C++                  |
| `@cpp:Protected`    | `system::workflow_attributes::att_cpp_Protected`| (none)           | Makes a member protected in generated C++                |
| `@cpp:Friend`       | `system::workflow_attributes::att_cpp_Friend`   | `ITypeDescriptor*` | Declares a friend class in generated C++               |

### Non-Serializable Argument Types

Most attribute arguments are serializable primitive values (`WString`, `vint`, `bool`, `float`, `double`).
`@cpp:Friend` is a special case: its argument type is `ITypeDescriptor*`, which is **not** serializable.
The attribute infrastructure handles this by:
- Storing the raw `ITypeDescriptor*` pointer as a boxed `Value` via `BoxValue<ITypeDescriptor*>(td)`.
- During binary serialization, the `ITypeDescriptor*` is stored as the pointed-to type descriptor's full name (a `WString`), and restored via `GetTypeDescriptor(typeName)` during deserialization.

## Assembly Population

When the Workflow compiler generates an assembly from compiled scripts, attributes are extracted from the AST and attached to type descriptors.
This happens in `Source/Emitter/WfEmitter_Assembly.cpp`:

- **`EvaluateAttributeLiteralExpression(manager, expr, argumentTd)`**: Evaluates a literal AST expression node into a boxed `Value`. Supports `bool`, `WString`, `vint`, `vuint`, `float`, `double`, `typeof(T)` (for `ITypeDescriptor*` arguments), and enum references.
- **`PopulateAttributesOnBag(manager, td, memberInfo, atts)`**: Iterates a list of attribute AST nodes, resolves each via `ResolveWorkflowAttribute`, creates an `AttributeInfoImpl`, evaluates the argument value (if any), and registers the attribute on the type descriptor.
- **`PopulateAttributesForDeclarations(manager)`**: Iterates all `declarationTypes` from the manager. For each type, it populates:
  - Type-level attributes from the declaration's attribute list.
  - Member-level attributes from class/interface method, property, and event declarations.
  - Member-level attributes from struct field declarations.
  - For `WfAutoPropertyDeclaration`, attribute lookup falls back to finding the property by name.
- This is called from `GenerateAssembly` after type implementation sorting.

## Binary Serialization

The `WfAssembly` binary format includes attribute data so that deserialized assemblies retain attribute metadata.
This is implemented in `Source/Runtime/WfRuntimeAssembly.cpp` as the `IOAttributeBag` helper:

- **Writer**: For each attribute on a target (type or member), writes the attribute type name, then for each value writes the value's type name and its serialized form. Non-serializable types (e.g., `ITypeDescriptor*`) are serialized by writing the pointed-to type descriptor's full name.
- **Reader**: Reads the attribute type name, creates an `AttributeInfoImpl`, then reads each value. For non-serializable types, reads the type name string and resolves it via `GetTypeDescriptor`.
- `IOAttributeBag` is called for every custom type (`IOCustomType`) and struct (`IOStruct`) during assembly serialization and deserialization, covering type-level, method, event, and property attributes.

## C++ Code Generation

When generating C++ reflection registration code, `WriteAttributeMacro` in `Source/Cpp/WfCpp_WriteReflection.cpp` emits the appropriate `ATTRIBUTE_TYPE` and `ATTRIBUTE_MEMBER` macros:
- `WString` arguments are emitted as string literals.
- `ITypeDescriptor*` arguments are emitted as `GetTypeDescriptor(L"typeName")` calls.
- Attributes with no arguments are emitted with just the type name.
