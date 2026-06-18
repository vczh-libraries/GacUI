# Workflow Knowledge Base

Project introduction remains in [Index.md](./Index.md#workflow).

### Choosing APIs

### Design Explanation

#### Attribute System

Workflow script attributes (`@category:name`) are translated to reflected struct types following the naming convention `system::workflow_attributes::att_category_name`. The compiler resolves, evaluates, and populates attributes onto type descriptors during assembly generation, and the binary serialization format preserves attribute metadata across assembly load/save cycles. Predefined `@cpp:*` attributes control C++ code generation behavior.

- The naming convention is implemented by `WfLexicalScopeManager::GetWorkflowAttributeTypeName` in `Source/Analyzer/WfAnalyzer.cpp`.
- Predefined attributes include `@cpp:File`, `@cpp:UserImpl`, `@cpp:Private`, `@cpp:Protected`, and `@cpp:Friend`.
- Assembly population is performed by `PopulateAttributesForDeclarations` in `Source/Emitter/WfEmitter_Assembly.cpp`.
- Binary serialization is handled by `IOAttributeBag` in `Source/Runtime/WfRuntimeAssembly.cpp`.
- C++ code generation emits `ATTRIBUTE_TYPE` / `ATTRIBUTE_MEMBER` macros via `WriteAttributeMacro` in `Source/Cpp/WfCpp_WriteReflection.cpp`.

[Design Explanation](./KB_Workflow_Design_AttributeSystem.md)
