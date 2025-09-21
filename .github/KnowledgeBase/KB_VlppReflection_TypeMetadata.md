# Type Metadata Access

## Runtime type information retrieval and manipulation through the reflection system.

When reflection is enabled (when `VCZH_DEBUG_NO_REFLECTION` is not defined), you can access type metadata and perform dynamic operations. The reflection system provides comprehensive runtime type information that can be used for dynamic programming scenarios.

## Core APIs

### GetTypeDescriptor
- Use `vl::reflection::description::GetTypeDescriptor<T>` for type metadata access when reflection is enabled
- Returns a pointer to `ITypeDescriptor` containing comprehensive type information
- Only available when reflection is compiled in (not in `VCZH_DEBUG_NO_REFLECTION` mode)

### Value Boxing
- Use `vl::reflection::description::Value` for boxing any value type similar to C# object or std::any
- Provides type-safe storage and retrieval of arbitrary values
- Supports conversion between different compatible types
- Works as a universal container for any reflectable type

### Reflectable Base Classes
- Use `Description<T>` base class for making classes reflectable
- Use `AggregatableDescription<T>` for classes that can be inherited in Workflow scripts
- Use `IDescriptable` interface for reflectable interfaces without other base interfaces

## Type Hierarchy Requirements

A reflectable class must inherit from `public vl::reflection::Description<the class itself>`.
Use `AggregatableDescription` to allow a class being inherited in a Workflow script class.
Sub types of reflectable classes or interfaces do not automatically become reflectable, they must use `Description<T>` or `AggregatableDescription<T>`.

A reflectable interface must inherit from `public vl::reflection::Description<the class itself>`.
If such interface does not implement any other interface, it must inherit from `public vl::reflection::IDescriptable`.

## Usage Patterns

When accessing reflectable members or functions, `vl::reflection::description::Value` is particularly helpful.
It should be used as a value type and provides similar functionality to `object` in C# or Java, or `std::any` in C++.

When using `Value` to represent "anything", it does not require reflection to be enabled.

A type is reflectable only when it is registered through the reflection system's registration mechanisms.

## Extra Content

### Conditional Usage

Always check if reflection is available before using reflection APIs:

```cpp
#ifndef VCZH_DEBUG_NO_REFLECTION
    auto typeDescriptor = vl::reflection::description::GetTypeDescriptor<MyClass>();
    if (typeDescriptor)
    {
        // Use type descriptor for metadata access
    }
#endif
```

### Value Type Operations

The `Value` class supports:
- Automatic boxing and unboxing of any reflectable type
- Type checking and safe casting operations
- Integration with the broader reflection system
- Serialization and deserialization capabilities

### Type Safety

The reflection system maintains C++ type safety while providing dynamic capabilities:
- Compile-time type checking where possible
- Runtime type verification for dynamic operations
- Exception-based error handling for type mismatches