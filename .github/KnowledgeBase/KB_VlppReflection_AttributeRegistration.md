# Attribute Registration

## Attach metadata attributes to types, members, and method parameters during reflection registration.

VlppReflection provides three macros for attaching attribute metadata to reflected types and their members.
An attribute is an instance of a reflectable struct whose constructor arguments are serializable primitive values (e.g., `WString`, `vint`, `bool`, `float`, `double`).
Attributes are stored centrally in the owning type descriptor and can be queried at runtime or through metaonly metadata.

## Attribute Macros

### ATTRIBUTE_TYPE(TYPE, ...)

Attaches an attribute to the enclosing type descriptor.
Can appear anywhere inside a `BEGIN_*_MEMBER` / `END_*_MEMBER` block.
Zero or more arguments are allowed; `ATTRIBUTE_TYPE(MyAttr)` is equivalent to `MyAttr{}`.

### ATTRIBUTE_MEMBER(TYPE, ...)

Attaches an attribute to the most recently registered member (field, property, event, method, or constructor).
Must appear after at least one member registration macro (e.g., `CLASS_MEMBER_FIELD`, `CLASS_MEMBER_METHOD`, `CLASS_MEMBER_CONSTRUCTOR`, `CLASS_MEMBER_EVENT`, `CLASS_MEMBER_PROPERTY_*`, `STRUCT_MEMBER`).
Multiple `ATTRIBUTE_MEMBER` calls can follow the same member to attach multiple attributes.

### ATTRIBUTE_PARAMETER(PARAMETER_NAME, TYPE, ...)

Attaches an attribute to a named parameter of the most recently registered method or constructor.
`PARAMETER_NAME` is a wide string literal (e.g., `L"x"`) that must match exactly one parameter name.
Must appear after a method or constructor registration macro.

## Attribute Type Requirements

- The attribute type must be a reflected struct (`TypeDescriptorFlags::Struct`).
- It must be registered via `BEGIN_STRUCT_MEMBER` / `END_STRUCT_MEMBER` before use.
- `TYPE{ ARG1, ARG2, ... }` must be a valid C++ aggregate initialization expression.
- Each argument is boxed as a `Value` and its type descriptor must have a non-null `GetSerializableType()`.

## Argument Type Inference

The attribute macros use a C++20 `BoxingProxy` technique to infer each argument's type from the struct's field type rather than from the C++ deduced literal type.
During aggregate initialization of the attribute struct, each argument is wrapped in a `BoxingProxy` whose `operator FieldType()` is called by the compiler with `FieldType` deduced from the corresponding struct field.
This means:
- An integer literal like `1` written for a `vint` field is boxed as `vint` (= `vint64_t` on x64, `vint32_t` on Win32), not as `int`.
- A wide string literal like `L"name"` written for a `WString` field is boxed as `WString`, not as `const wchar_t*`.

There is no need to explicitly cast arguments to the field type.

## Storage Architecture

All attributes are stored centrally in the owning `ITypeDescriptor`'s `AttributeBagSource` base class.
- `AttributeBagSource` uses `collections::Group<IMemberInfo*, Ptr<IAttributeInfo>>` where a `nullptr` key represents type-level attributes and non-null keys represent member/parameter-level attributes.
- `MemberInfoBase` (the base for all member info implementations) delegates `GetAttributeCount` / `GetAttribute` back to the owner type descriptor's `AttributeBagSource`.
- Attributes are returned in registration order.

## Runtime Query

Use the `IAttributeBag` interface (inherited by `ITypeDescriptor`, `IMemberInfo`, and `IParameterInfo`) to query attributes:
- `GetAttributeCount()` — returns the number of attributes on this target.
- `GetAttribute(index)` — returns the `IAttributeInfo` at the given index.

Use `IAttributeInfo` to inspect an attribute:
- `GetAttributeType()` — returns the `ITypeDescriptor` of the attribute struct.
- `GetAttributeValueCount()` — returns the number of constructor argument values.
- `GetAttributeValue(index)` — returns the boxed `Value` of the argument at the given index.

## Metaonly Metadata

Attributes are serialized into metaonly binary metadata by `GenerateMetaonlyTypes` and deserialized by `LoadMetaonlyTypes`.
Each attribute value is serialized through `ISerializableType::Serialize` and deserialized through `ISerializableType::Deserialize`.
Attributes appear in the logged text output (`.txt` baseline files) in the format:
```
@Attribute:<AttributeTypeName>(<ArgTypeName>:<SerializedData>, ...)
@ParamAttribute:<ParameterName>:<AttributeTypeName>(<ArgTypeName>:<SerializedData>, ...)
```

## Usage Example

### Define attribute structs
```cpp
struct MyAttribute
{
    WString name;
    vint number;
    auto operator<=>(const MyAttribute&) const = default;
};

struct EmptyAttribute
{
    auto operator<=>(const EmptyAttribute&) const = default;
};
```

### Register attribute structs
```cpp
BEGIN_STRUCT_MEMBER(MyAttribute)
    STRUCT_MEMBER(name)
    STRUCT_MEMBER(number)
END_STRUCT_MEMBER(MyAttribute)

BEGIN_STRUCT_MEMBER(EmptyAttribute)
END_STRUCT_MEMBER(EmptyAttribute)
```

### Use attribute macros in struct registration
```cpp
BEGIN_STRUCT_MEMBER(MyStruct)
    STRUCT_MEMBER(x)
    ATTRIBUTE_MEMBER(MyAttribute, L"field-x", 42)
    STRUCT_MEMBER(y)
    ATTRIBUTE_MEMBER(EmptyAttribute)
END_STRUCT_MEMBER(MyStruct)
```

### Use attribute macros in class registration
```cpp
#define _ ,

BEGIN_CLASS_MEMBER(MyClass)
    ATTRIBUTE_TYPE(MyAttribute, L"type", 1)
    ATTRIBUTE_TYPE(EmptyAttribute)

    CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(vint), {L"seed"})
    ATTRIBUTE_MEMBER(MyAttribute, L"ctor", 2)
    ATTRIBUTE_PARAMETER(L"seed", MyAttribute, L"param", 3)

    CLASS_MEMBER_EVENT(Changed)
    ATTRIBUTE_MEMBER(MyAttribute, L"event", 4)

    CLASS_MEMBER_FIELD(fieldValue)
    ATTRIBUTE_MEMBER(MyAttribute, L"field", 5)

    CLASS_MEMBER_PROPERTY_FAST(Value)
    ATTRIBUTE_MEMBER(MyAttribute, L"property", 6)

    CLASS_MEMBER_METHOD(DoWork, {L"x" _ L"y"})
    ATTRIBUTE_MEMBER(MyAttribute, L"method", 7)
    ATTRIBUTE_PARAMETER(L"x", MyAttribute, L"param-x", 8)
    ATTRIBUTE_PARAMETER(L"y", EmptyAttribute)
END_CLASS_MEMBER(MyClass)

#undef _
```

## Compilation Level Support

- Attribute macros are available when `VCZH_DEBUG_NO_REFLECTION` is **not** defined.
- They are excluded when `VCZH_DEBUG_NO_REFLECTION` is defined (no reflection mode).
- They work in both full reflection and metaonly reflection (`VCZH_DEBUG_METAONLY_REFLECTION`) builds.

## Error Handling

- `ATTRIBUTE_MEMBER` raises `CHECK_ERROR` if no member has been registered yet.
- `ATTRIBUTE_PARAMETER` raises `CHECK_ERROR` if the last registered member is not a method or constructor.
- `ATTRIBUTE_PARAMETER` raises `CHECK_ERROR` if the named parameter does not exist or is ambiguous.
- A `static_assert` fires at compile time if `TYPE{ ARG1, ... }` is not a valid expression.
- `CHECK_ERROR` is raised if the attribute type is not a reflected struct or if an argument is not serializable.
