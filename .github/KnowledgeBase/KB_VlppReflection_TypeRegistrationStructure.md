# Type Registration Structure

## Organized approach for registering types with proper file organization and macro usage.

All type registration must occur in `vl::reflection::description` namespace with specific file organization. The reflection system requires a structured approach to type registration with clear separation between declarations and implementations, following established patterns from existing source code examples.

## File Organization Requirements

### Header File (.h) Structure
- Type lists and interface proxies in `.h` files
- Contains type declarations and interface proxy definitions
- Must include all necessary forward declarations
- Should follow the established macro patterns

### Implementation File (.cpp) Structure  
- Type metadata registration in `.cpp` files
- Contains the actual registration logic and metadata
- Implements the type loading mechanisms
- Follows specific namespace and macro conventions

### Dedicated Registration Files
- Registration code must be in dedicated files
- Do not mix registration code with regular implementation
- Follow established patterns from existing source code examples
- Maintain clear separation of concerns

## Registration Structure Pattern

### Header File Example (.h)
```cpp
namespace vl::reflection::description
{
#ifndef VCZH_DEBUG_NO_REFLECTION

    #define MY_TYPES(F)\
        F(::my::namespaces::First)\
        F(::my::namespaces::ISecond)\

    MY_TYPES(DECL_TYPE_INFO)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
#pragma warning(push)
#pragma warning(disable:4250)

    BEGIN_INTERFACE_PROXY...(::my::namespaces::ISecond)
      ...
    END_INTERFACE_PROXY(::my::namespaces::ISecond)

#pragma warning(pop)
#endif
#endif

    extern bool LoadMyTypes();
}
```

### Implementation File Example (.cpp)
```cpp
namespace vl::reflection::description
{

#ifndef VCZH_DEBUG_NO_REFLECTION

    MY_TYPES(IMPL_CPP_TYPE_INFO)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
#define _ ,

    BEGIN_CLASS_MEMBER(::my::namespaces::ISecond)
        CLASS_MEMBER_METHOD(ThisFunction, NO_PARAMETER)
        CLASS_MEMBER_METHOD(ThatFunction, { L"arg1" _ L"arg2" })
        ...
    END_CLASS_MEMBER(::my::namespaces::ISecond)

    BEGIN_INTERFACE_MEMBER(::my::namespaces::ISecond)
        vint Func(vint a, vint b) override
        {
            INVOKEGET_INTERFACE_PROXY_NOPARAMS(Func, a, b);
        }
        ...
    END_INTERFACE_MEMBER(::my::namespaces::ISecond)

#undef _

    class MyTypeLoader : public Object, public virtual ITypeLoader
    {
    public:
        void Load(ITypeManager* manager) override
        {
            MY_TYPES(ADD_TYPE_INFO)
        }

        void Unload(ITypeManager* manager) override
        {
        }
    };

#endif
#endif

    bool LoadMyTypes()
    {
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
        if (auto manager = GetGlobalTypeManager())
        {
            return manager->AddTypeLoader(Ptr(new MyTypeLoader));
        }
#endif
        return false;
    }
}
```

## Key Components

### Type Lists
Define macros that enumerate all types to be registered:
- Use the `#define TYPE_LIST(F)` pattern
- Include all fully qualified type names
- Apply the macro to various registration operations

### Namespace Requirements
- All registration must occur in `vl::reflection::description` namespace
- Maintain consistent namespace usage throughout
- Use fully qualified names for registered types

### Conditional Compilation
- Wrap registration code with appropriate reflection level checks
- Support different compilation modes (full, metadata-only, no reflection)
- Use proper preprocessor guards

## Extra Content

### Best Practices

1. **Consistency**: Follow the exact patterns used in existing registration files
2. **Organization**: Keep related registrations together in the same file
3. **Dependencies**: Ensure proper dependency ordering between type registrations
4. **Documentation**: Comment complex registration patterns for clarity

### Common Pitfalls

- Missing namespace qualifications
- Incorrect conditional compilation guards  
- Mixing registration with implementation logic
- Inconsistent file organization patterns

### Maintenance

- Regularly review registration files for consistency
- Update registration patterns when new reflection features are added
- Ensure all new types follow the established registration structure