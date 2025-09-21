# Interface Proxy Implementation

## Proxy generation for interfaces to enable inheritance in Workflow scripts.

VlppReflection provides interface proxy macros that generate proxy implementations for interfaces, enabling them to be inherited and implemented in Workflow scripts. The proxy system handles the bridge between C++ interfaces and dynamic Workflow script implementations.

## Proxy Declaration Patterns

### Interfaces Without Base Interfaces
- Use `BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR` for interfaces without base interfaces using raw pointers
- Use `BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR` for interfaces without base interfaces using `Ptr<T>`
- Choose based on the memory management strategy for interface implementations

### Interfaces With Base Interfaces
- Use `BEGIN_INTERFACE_PROXY_RAWPTR` for interfaces with base interfaces using raw pointers
- Use `BEGIN_INTERFACE_PROXY_SHAREDPTR` for interfaces with base interfaces using `Ptr<T>`
- All reflectable base interfaces must be listed (`IDescriptable` doesn't count)

### Proxy Completion
- Use `END_INTERFACE_PROXY` to complete proxy definition
- Must be used regardless of which BEGIN macro was used

## Method Implementation Patterns

### Void Methods Without Parameters
- Use `INVOKE_INTERFACE_PROXY_NOPARAMS` for void methods without parameters
- Handles method calls that don't return values and don't take arguments

### Return Value Methods Without Parameters
- Use `INVOKEGET_INTERFACE_PROXY_NOPARAMS` for return value methods without parameters
- Automatically handles return value retrieval and type conversion

### Void Methods With Parameters
- Use `INVOKE_INTERFACE_PROXY` for void methods with parameters
- Pass all method arguments to the macro

### Return Value Methods With Parameters
- Use `INVOKEGET_INTERFACE_PROXY` for return value methods with parameters
- Automatically handles both parameter passing and return value retrieval

## Implementation Structure

### Proxy Pattern
An interface proxy begins with the appropriate BEGIN macro and ends with `END_INTERFACE_PROXY(name)`:

```cpp
BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(IMyInterface)
    void VoidMethodNoParams() override
    {
        INVOKE_INTERFACE_PROXY_NOPARAMS(VoidMethodNoParams);
    }
    
    int GetValueNoParams() override
    {
        INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetValueNoParams);
    }
    
    void VoidMethodWithParams(int a, const WString& b) override
    {
        INVOKE_INTERFACE_PROXY(VoidMethodWithParams, a, b);
    }
    
    int GetValueWithParams(int a, const WString& b) override
    {
        INVOKEGET_INTERFACE_PROXY(GetValueWithParams, a, b);
    }
END_INTERFACE_PROXY(IMyInterface)
```

### Method Implementation Rules
Inside the proxy, there are functions that implement the interface. In each function implementation there will be only one line of code from the appropriate INVOKE macro.

The `return` keyword is not necessary as `INVOKEGET_INTERFACE_PROXY_NOPARAMS` and `INVOKEGET_INTERFACE_PROXY` already handle return values automatically.

## Proxy Categories

### Raw or Shared Pointer Proxies
This is decided by the natural usage of the interface. If an instance of the interface is usually stored in `Ptr<T>`, use the shared pointer version.

## Interface Inheritance Hierarchy

### Base Interface Considerations
When registering interfaces with inheritance:
1. Register base interfaces first
2. List all reflectable base interfaces in the proxy declaration
3. `IDescriptable` is handled automatically and should not be listed
4. Ensure proper virtual inheritance in C++ interface definitions

## Extra Content

### Advanced Proxy Scenarios

#### Multiple Inheritance
For interfaces that inherit from multiple base interfaces:
```cpp
BEGIN_INTERFACE_PROXY_SHAREDPTR(IDerivedInterface, IBaseInterface1, IBaseInterface2)
    // Implement all methods from all interfaces
END_INTERFACE_PROXY(IDerivedInterface)
```

#### Complex Parameter Types
Proxy methods can handle complex parameter types:
- Custom struct types (must be registered)
- Collection types
- Other interface types
- Enum types
- Nullable types

#### Error Handling
Proxy implementations automatically handle:
- Type conversion errors
- Method invocation failures
- Parameter validation
- Return value conversion

### Performance Considerations

1. **Proxy Overhead**: Proxy calls have additional overhead compared to direct calls
2. **Type Conversion**: Parameter and return value conversion can impact performance
3. **Caching**: Consider caching proxy instances for frequently used interfaces
4. **Memory Management**: Choose appropriate pointer type based on usage patterns

### Integration with Workflow Scripts

Interface proxies enable Workflow scripts to:
- Implement C++ interfaces directly
- Override virtual methods
- Participate in callback scenarios
- Integrate with event systems
- Provide custom business logic implementations

### Best Practices

1. **Choose Appropriate Pointer Type**: Use shared pointers for reference-counted scenarios
2. **Complete Implementation**: Implement all interface methods in the proxy
3. **Error Handling**: Consider error scenarios in proxy implementations
4. **Documentation**: Document the purpose and usage of each proxy
5. **Testing**: Thoroughly test proxy implementations with Workflow scripts