# VlppReflection Knowledge Base

Project introduction remains in [Index.md](./Index.md#vlppreflection).

### Choosing APIs

#### Reflection Compilation Levels

Three different compilation modes for reflection support with varying runtime capabilities.

The reflection system supports three compilation levels:
- Full reflection: Complete metadata and runtime support for type registration and function calls
- Metadata-only (`VCZH_DESCRIPTABLEOBJECT_WITH_METADATA`): Type metadata without runtime support
- No reflection (`VCZH_DEBUG_NO_REFLECTION`): Reflection disabled entirely
Always prefer code compatible with `VCZH_DEBUG_NO_REFLECTION` when possible.

[API Explanation](./KB_VlppReflection_CompilationLevels.md)

#### Type Metadata Access

Runtime type information retrieval and manipulation through the reflection system.

- Use `vl::reflection::description::GetTypeDescriptor<T>` for type metadata access when reflection is enabled
- Use `vl::reflection::description::Value` for boxing any value type similar to C# object or std::any
- Use `Description<T>` base class for making classes reflectable
- Use `AggregatableDescription<T>` for classes that can be inherited in Workflow scripts
- Use `IDescriptable` interface for reflectable interfaces without other base interfaces

[API Explanation](./KB_VlppReflection_TypeMetadata.md)

#### Type Registration Structure

Organized approach for registering types with proper file organization and macro usage.

All type registration must occur in `vl::reflection::description` namespace with specific file organization:
- Type lists and interface proxies in `.h` files
- Type metadata registration in `.cpp` files
- Registration code in dedicated files
- Follow established patterns from existing source code examples

[API Explanation](./KB_VlppReflection_TypeRegistrationStructure.md)

#### Enum Registration

Registration patterns for enumeration types with support for simple lists and combinable flags.

- Use `BEGIN_ENUM_ITEM` and `END_ENUM_ITEM` for simple enumeration lists
- Use `BEGIN_ENUM_ITEM_MERGABLE` and `END_ENUM_ITEM` for combinable flag enumerations
- Use `ENUM_CLASS_ITEM` for enum class members
- Use `ENUM_ITEM` for enum members
- Use `ENUM_ITEM_NAMESPACE` and `ENUM_NAMESPACE_ITEM` for enums defined inside other types

[API Explanation](./KB_VlppReflection_EnumRegistration.md)

#### Struct Registration

Registration patterns for structure types with field access capabilities.

- Use `BEGIN_STRUCT_MEMBER` and `END_STRUCT_MEMBER` for struct registration
- Use `STRUCT_MEMBER` to register each accessible field
- Use `ATTRIBUTE_TYPE`, `ATTRIBUTE_MEMBER` to attach attributes to the struct or its fields

[API Explanation](./KB_VlppReflection_StructRegistration.md)

#### Class and Interface Registration

Comprehensive registration system for classes and interfaces with methods, properties, and events.

- Use `BEGIN_CLASS_MEMBER` and `END_CLASS_MEMBER` for class registration
- Use `BEGIN_INTERFACE_MEMBER` and `END_INTERFACE_MEMBER` for inheritable interfaces
- Use `BEGIN_INTERFACE_MEMBER_NOPROXY` and `END_INTERFACE_MEMBER` for non-inheritable interfaces
- Use `CLASS_MEMBER_BASE` for reflectable base class declaration
- Use `CLASS_MEMBER_FIELD` for member field registration
- Use `CLASS_MEMBER_CONSTRUCTOR` for constructor registration with `Ptr<Class>(types...)` or `Class*(types...)`
- Use `CLASS_MEMBER_EXTERNALCTOR` for external function constructors
- Use `CLASS_MEMBER_METHOD` for method registration with parameter names
- Use `CLASS_MEMBER_METHOD_OVERLOAD` for overloaded method registration
- Use `CLASS_MEMBER_EXTERNALMETHOD` for external function methods
- Use `CLASS_MEMBER_STATIC_METHOD` for static method registration
- Use `CLASS_MEMBER_EVENT` for event registration
- Use `CLASS_MEMBER_PROPERTY_READONLY`, `CLASS_MEMBER_PROPERTY` for property registration
- Use `CLASS_MEMBER_PROPERTY_READONLY_FAST`, `CLASS_MEMBER_PROPERTY_FAST` for standard getter/setter patterns
- Use `CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST`, `CLASS_MEMBER_PROPERTY_EVENT_FAST` for properties with change events
- Use `NO_PARAMETER` for parameterless functions
- Use `{ L"arg1" _ L"arg2" ... }` for parameter name lists
- Use `ATTRIBUTE_TYPE`, `ATTRIBUTE_MEMBER`, `ATTRIBUTE_PARAMETER` to attach attributes to types, members, and parameters

[API Explanation](./KB_VlppReflection_ClassInterfaceRegistration.md)

#### Interface Proxy Implementation

Proxy generation for interfaces to enable inheritance in Workflow scripts.

- Use `BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR` for interfaces without base interfaces using raw pointers
- Use `BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR` for interfaces without base interfaces using Ptr<T>
- Use `BEGIN_INTERFACE_PROXY_RAWPTR` for interfaces with base interfaces using raw pointers
- Use `BEGIN_INTERFACE_PROXY_SHAREDPTR` for interfaces with base interfaces using Ptr<T>
- Use `END_INTERFACE_PROXY` to complete proxy definition
- Use `INVOKE_INTERFACE_PROXY_NOPARAMS` for void methods without parameters
- Use `INVOKEGET_INTERFACE_PROXY_NOPARAMS` for return value methods without parameters
- Use `INVOKE_INTERFACE_PROXY` for void methods with parameters
- Use `INVOKEGET_INTERFACE_PROXY` for return value methods with parameters

[API Explanation](./KB_VlppReflection_InterfaceProxy.md)

#### Attribute Registration

Attach metadata attributes to types, members, and method parameters during reflection registration.

Attributes are instances of reflectable structs whose constructor arguments are serializable primitive values.
They are stored centrally in the owning type descriptor and can be queried at runtime via the `IAttributeBag` / `IAttributeInfo` interfaces.
Attributes survive metaonly metadata serialization and deserialization, and appear in the logged text output.

- Use `ATTRIBUTE_TYPE(TYPE, ...)` to attach an attribute to the enclosing type descriptor
- Use `ATTRIBUTE_MEMBER(TYPE, ...)` to attach an attribute to the most recently registered member (field, property, event, method, or constructor)
- Use `ATTRIBUTE_PARAMETER(PARAMETER_NAME, TYPE, ...)` to attach an attribute to a named parameter of the most recently registered method or constructor
- Use `IAttributeBag::GetAttributeCount` and `IAttributeBag::GetAttribute` to query attributes at runtime
- Use `IAttributeInfo::GetAttributeType`, `IAttributeInfo::GetAttributeValueCount`, `IAttributeInfo::GetAttributeValue` to inspect attribute content

[API Explanation](./KB_VlppReflection_AttributeRegistration.md)

### Design Explanation
