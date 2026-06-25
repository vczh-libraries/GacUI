# Registration Macros and Attributes

VlppReflection registers C++ types by executing type loader code. The source comment above **Description\<T\>** in VlppReflection contains the complete macro walkthrough; this page summarizes the registration workflow and documents the attribute macros used to attach metadata to registered types, members and parameters. These attributes are VlppReflection metadata, not C++ language attributes.

Type registration code is compiled only when **VCZH_DEBUG_NO_REFLECTION** is not defined. Code that declares reflectable classes still needs to compile without runtime reflection, but member registration and attributes are part of the reflection build.

## Registration Workflow

A registration unit usually follows this shape:

- Define one typelist macro that enumerates fully qualified C++ types.
- In headers, expand the typelist with **DECL_TYPE_INFO**.
- In one C++ file, expand the typelist with **IMPL_CPP_TYPE_INFO** or **IMPL_TYPE_INFO_RENAME**.
- Register enum items, struct fields, class members or interface members.
- Create an **ITypeLoader** and call **ADD_TYPE_INFO** for every type in the typelist.
- Install the loader through **GetGlobalTypeManager()-\>AddTypeLoader** before reading reflected metadata.

Member registration macros often need a temporary comma macro because parameter name lists are written inside macro arguments:

```C++
#define MY_TYPELIST(F)\
    F(demo::DisplayNameAttribute)\
    F(demo::SampleClass)

MY_TYPELIST(DECL_TYPE_INFO)

// In one C++ file:
MY_TYPELIST(IMPL_CPP_TYPE_INFO)

#define _ ,

BEGIN_STRUCT_MEMBER(demo::DisplayNameAttribute)
    STRUCT_MEMBER(text)
END_STRUCT_MEMBER(demo::DisplayNameAttribute)

BEGIN_CLASS_MEMBER(demo::SampleClass)
    CLASS_MEMBER_CONSTRUCTOR(Ptr<demo::SampleClass>(), NO_PARAMETER)
    ATTRIBUTE_TYPE(demo::DisplayNameAttribute, L"Sample")

    CLASS_MEMBER_METHOD(Rename, { L"value" })
    ATTRIBUTE_MEMBER(demo::DisplayNameAttribute, L"Renaming")
    ATTRIBUTE_PARAMETER(L"value", demo::DisplayNameAttribute, L"New name")
END_CLASS_MEMBER(demo::SampleClass)

#undef _

class DemoTypeLoader : public Object, public ITypeLoader
{
public:
    void Load(ITypeManager* manager) override
    {
        MY_TYPELIST(ADD_TYPE_INFO)
    }

    void Unload(ITypeManager* manager) override
    {
    }
};
```

The big source comment above **Description\<T\>** also lists these registration macro families:

- Enums: **BEGIN_ENUM_ITEM**, **BEGIN_ENUM_ITEM_MERGABLE**, **ENUM_ITEM**, **ENUM_CLASS_ITEM**, **END_ENUM_ITEM**.
- Structs: **BEGIN_STRUCT_MEMBER**, **STRUCT_MEMBER**, **END_STRUCT_MEMBER**.
- Classes: **BEGIN_CLASS_MEMBER**, **CLASS_MEMBER_BASE**, **CLASS_MEMBER_FIELD**, **CLASS_MEMBER_CONSTRUCTOR**, **CLASS_MEMBER_EXTERNALCTOR(_TEMPLATE)?**, **CLASS_MEMBER_METHOD(_OVERLOAD)?_RENAME**, **CLASS_MEMBER_(STATIC_)?METHOD(_OVERLOAD)?**, **CLASS_MEMBER_(STATIC_)?EXTERNALMETHOD(_TEMPLATE)?**, **CLASS_MEMBER_PROPERTY(_EVENT)?(_READONLY)?(_FAST)?**, **CLASS_MEMBER_PROPERTY_REFERENCETEMPLATE**, **CLASS_MEMBER_EVENT**, **END_CLASS_MEMBER**.
- Interfaces: **BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR**, **BEGIN_INTERFACE_PROXY_RAWPTR**, **BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR**, **BEGIN_INTERFACE_PROXY_SHAREDPTR**, **END_INTERFACE_PROXY**, **BEGIN_INTERFACE_MEMBER**, **BEGIN_INTERFACE_MEMBER_NOPROXY**, **END_INTERFACE_MEMBER**.

A reflectable interface must directly or indirectly inherit **IDescriptable**. If Workflow script classes should be able to implement the interface, register a proxy and use a constructor that accepts **Ptr\<IValueInterfaceProxy\>**. Use **BEGIN_INTERFACE_MEMBER_NOPROXY** only when script implementation is not required.

## Attribute Macros

Attribute macros create **IAttributeInfo** objects and store them in an **IAttributeBag**. **ITypeDescriptor**, **IMemberInfo** and **IParameterInfo** expose attributes through **GetAttributeCount** and **GetAttribute**. Each attribute records its reflected struct type and the boxed values passed to the macro.

The attribute type must satisfy these rules:

- The type must be registered in reflection.
- The registered type descriptor must be a struct, not a class or interface.
- **TYPE{ARG1, ...}** must compile for the arguments passed to the macro.
- Each argument should be **ITypeDescriptor*** or a reflected serializable value type, so metadata logging and generated metadata can write it back.
- Zero arguments are allowed for marker attributes.

Use these macros inside a type member registration block:

- **ATTRIBUTE_TYPE(TYPE, ...)**: attaches the attribute to the current type descriptor.
- **ATTRIBUTE_MEMBER(TYPE, ...)**: attaches the attribute to the most recently registered field, property, event, method or constructor. Put it immediately after the member macro it decorates.
- **ATTRIBUTE_PARAMETER(L"name", TYPE, ...)**: attaches the attribute to a named parameter of the most recently registered method or constructor. The parameter name must match exactly one parameter.

## Adding a Custom Attribute

A custom attribute is just a reflected struct that is later attached by one of the attribute macros. Keep it small and data-only. The field order is the argument order seen by **TYPE{ARG1, ...}**.

```C++
namespace demo
{
    struct DisplayNameAttribute
    {
        WString text;
        auto operator<=>(const DisplayNameAttribute&) const = default;
    };
}

#define DEMO_TYPES(F)\
    F(demo::DisplayNameAttribute)\
    F(demo::SampleClass)

DEMO_TYPES(DECL_TYPE_INFO)

DEMO_TYPES(IMPL_CPP_TYPE_INFO)

#define _ ,

BEGIN_STRUCT_MEMBER(demo::DisplayNameAttribute)
    STRUCT_MEMBER(text)
END_STRUCT_MEMBER(demo::DisplayNameAttribute)

BEGIN_CLASS_MEMBER(demo::SampleClass)
    ATTRIBUTE_TYPE(demo::DisplayNameAttribute, L"Visible sample")

    CLASS_MEMBER_METHOD(Rename, { L"value" })
    ATTRIBUTE_MEMBER(demo::DisplayNameAttribute, L"Rename method")
    ATTRIBUTE_PARAMETER(L"value", demo::DisplayNameAttribute, L"Target name")
END_CLASS_MEMBER(demo::SampleClass)

#undef _
```

To read attributes, iterate the **IAttributeBag**, check **IAttributeInfo::GetAttributeType**, then read each argument through **GetAttributeValueType** and **GetAttributeValue**. If an attribute argument is a type reference, the value type is **ITypeDescriptor** and the boxed value stores an **ITypeDescriptor***.

Workflow source attributes use the syntax **@category:Name**. The Workflow analyzer resolves this to **system::workflow_attributes::att_category_Name**. The resolved type must be a reflected struct with zero fields or one field. A one-field Workflow attribute takes one argument, and Workflow C++ generation currently emits only string values and **ITypeDescriptor*** values for attributes.

## Existing Production Attributes

A scan of the sibling source repositories found production attribute declarations only in the Workflow repo. Test attributes in VlppReflection tests are intentionally excluded. GacUI and generated sources use Workflow attributes, but they do not declare additional production attribute structs.

All existing production attributes live in C++ namespace **vl::__vwsn** and are reflected under **system::workflow_attributes**.

- **att_cpp_File** / **system::workflow_attributes::att_cpp_File**: one **WString argument**. Workflow **@cpp:File("YourFile")** controls the generated C++ file pair for a class.
- **att_cpp_UserImpl** / **system::workflow_attributes::att_cpp_UserImpl**: marker. Workflow **@cpp:UserImpl** leaves a generated constructor, destructor or function body for user code merging.
- **att_cpp_Private** / **system::workflow_attributes::att_cpp_Private**: marker. Workflow **@cpp:Private** emits the decorated generated C++ class member in a private section.
- **att_cpp_Protected** / **system::workflow_attributes::att_cpp_Protected**: marker. Workflow **@cpp:Protected** emits the decorated generated C++ class member in a protected section.
- **att_cpp_Friend** / **system::workflow_attributes::att_cpp_Friend**: one **ITypeDescriptor* argument**. Workflow **@cpp:Friend(typeof(YourType))** emits a C++ friend declaration.
- **att_rpc_Interface** / **system::workflow_attributes::att_rpc_Interface**: marker used by Workflow RPC validation to identify RPC interfaces.
- **att_rpc_Ctor** / **system::workflow_attributes::att_rpc_Ctor**: marker used by Workflow RPC generation for constructor metadata.
- **att_rpc_Byval** / **system::workflow_attributes::att_rpc_Byval**: marker used by Workflow RPC generation for by-value transport decisions.
- **att_rpc_Byref** / **system::workflow_attributes::att_rpc_Byref**: marker used by Workflow RPC generation for by-reference transport decisions.
- **att_rpc_Cached** / **system::workflow_attributes::att_rpc_Cached**: marker used by Workflow RPC generation for cached by-reference behavior.
- **att_rpc_Dynamic** / **system::workflow_attributes::att_rpc_Dynamic**: marker used by Workflow RPC generation for dynamic RPC behavior.
- **att_rpc_IdString** / **system::workflow_attributes::att_rpc_IdString**: one **WString argument**. Workflow RPC metadata uses it for stable string ids.
- **att_rpc_IdNumber** / **system::workflow_attributes::att_rpc_IdNumber**: one **vint argument**. Workflow RPC metadata uses it for stable numeric ids.

The C++ code generation attributes affect generated C++ layout only. For example, **@cpp:Private** and **@cpp:Protected** do not change Workflow visibility rules.

