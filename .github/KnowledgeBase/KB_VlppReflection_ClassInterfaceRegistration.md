# Class and Interface Registration

## Comprehensive registration system for classes and interfaces with methods, properties, and events.

VlppReflection provides extensive macros for registering classes and interfaces, enabling complete runtime reflection support including methods, properties, events, constructors, and inheritance relationships. The system supports both classes and interfaces with shared registration patterns.

## Basic Registration Patterns

### Class Registration
- Use `BEGIN_CLASS_MEMBER` and `END_CLASS_MEMBER` for class registration
- Provides foundation for making class types accessible through reflection
- Supports all class features including inheritance, methods, and properties

### Interface Registration
- Use `BEGIN_INTERFACE_MEMBER` and `END_INTERFACE_MEMBER` for inheritable interfaces
- Use `BEGIN_INTERFACE_MEMBER_NOPROXY` and `END_INTERFACE_MEMBER` for non-inheritable interfaces
- Interfaces can be inherited in Workflow scripts when using the regular `BEGIN_INTERFACE_MEMBER`

## Registration Components

### Base Class Declaration
- Use `CLASS_MEMBER_BASE` for reflectable base class declaration
- Enables proper inheritance hierarchy in reflection
- Supports multiple inheritance scenarios

### Field Registration
- Use `CLASS_MEMBER_FIELD` for member field registration
- Exposes public fields through the reflection system
- Enables dynamic field access and manipulation

### Constructor Registration
- Use `CLASS_MEMBER_CONSTRUCTOR` for constructor registration with `Ptr<Class>(types...)` or `Class*(types...)`
- Use `CLASS_MEMBER_EXTERNALCTOR` for external function constructors
- Constructor type determines whether instances are boxed in `Ptr<T>` or not

### Method Registration
- Use `CLASS_MEMBER_METHOD` for method registration with parameter names
- Use `CLASS_MEMBER_METHOD_OVERLOAD` for overloaded method registration
- Use `CLASS_MEMBER_EXTERNALMETHOD` for external function methods
- Use `CLASS_MEMBER_STATIC_METHOD` for static method registration

### Event Registration
- Use `CLASS_MEMBER_EVENT` for event registration
- Events are typically `Event<T>` type fields
- Enables dynamic event subscription and notification

### Property Registration
- Use `CLASS_MEMBER_PROPERTY_READONLY`, `CLASS_MEMBER_PROPERTY` for property registration
- Use `CLASS_MEMBER_PROPERTY_READONLY_FAST`, `CLASS_MEMBER_PROPERTY_FAST` for standard getter/setter patterns
- Use `CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST`, `CLASS_MEMBER_PROPERTY_EVENT_FAST` for properties with change events

## Parameter Name Specifications

### Function Arguments
For constructors and methods, argument names are required in the declaration:
- Use `NO_PARAMETER` for parameterless functions
- Use `{ L"arg1" _ L"arg2" ... }` for parameter name lists
- The `_` macro must be defined as `,` in the implementation file

## Registration Examples

### Basic Class Registration
```cpp
BEGIN_CLASS_MEMBER(MyClass)
    CLASS_MEMBER_FIELD(FirstField)
    CLASS_MEMBER_FIELD(SecondField)
END_CLASS_MEMBER(MyClass)
```

### Interface Registration
```cpp
BEGIN_INTERFACE_MEMBER(IMyInterface)
    CLASS_MEMBER_FIELD(FirstField)
    CLASS_MEMBER_FIELD(SecondField)
END_INTERFACE_MEMBER(IMyInterface)
```

### Complete Class with All Features
```cpp
#define _ ,

BEGIN_CLASS_MEMBER(MyClass)
    CLASS_MEMBER_BASE(BaseClass)
    CLASS_MEMBER_FIELD(fieldName)
    CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(vint), { L"value" })
    CLASS_MEMBER_METHOD(MethodName, { L"arg1" _ L"arg2" })
    CLASS_MEMBER_STATIC_METHOD(StaticMethod, NO_PARAMETER)
    CLASS_MEMBER_EVENT(SomeEvent)
    CLASS_MEMBER_PROPERTY_FAST(PropertyName)
END_CLASS_MEMBER(MyClass)

#undef _
```

## Interface Requirements

### Proxy Requirements
Using `BEGIN_INTERFACE_MEMBER` requires a proxy to EXIST in the header file, which means the interface can be inherited in Workflow script.

Using `BEGIN_INTERFACE_MEMBER_NOPROXY` requires a proxy to NOT EXIST in the header file, which means the interface cannot be inherited in Workflow script.

### Constructor Limitations
There is no constructor in an interface registration - only classes support constructor registration.

## Extra Content

### Advanced Registration Features

#### Method Overloading
For overloaded methods, use specific macros:
- `CLASS_MEMBER_METHOD_OVERLOAD(name, parameter, function-type)`
- `CLASS_MEMBER_METHOD_OVERLOAD_RENAME(new-name, name, parameter, function-type)`
- Function type must be a pointer to member function

#### External Methods
For methods implemented as external functions:
- `CLASS_MEMBER_EXTERNALMETHOD(name, parameters, function-type, source)`
- First parameter acts as `this` pointer
- Should not appear in parameters or function-type

#### Property Shortcuts
Fast property registration shortcuts:
- `CLASS_MEMBER_PROPERTY_READONLY_FAST(X)` for `GetX()` getter and property `X`
- `CLASS_MEMBER_PROPERTY_FAST(X)` for `GetX()` getter, `SetX()` setter, and property `X`
- `CLASS_MEMBER_PROPERTY_EVENT_FAST(X)` includes `XChanged` event

### Best Practices

1. **Complete Registration**: Register all public members that should be accessible
2. **Consistent Naming**: Follow established naming conventions
3. **Parameter Documentation**: Provide meaningful parameter names
4. **Event Integration**: Use events to notify property changes
5. **Type Safety**: Ensure all referenced types are properly registered

### Performance Considerations

- Registration occurs at startup time with minimal runtime overhead
- Dynamic method calls have slight performance cost compared to direct calls
- Property access through reflection is slower than direct field access
- Consider caching reflection results for frequently used operations