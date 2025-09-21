# Enum Registration

## Registration patterns for enumeration types with support for simple lists and combinable flags.

VlppReflection provides specific macros for registering enumeration types, supporting both simple enumeration lists and combinable flag enumerations. The registration system accommodates different enum styles including C++ enum classes and traditional enums, as well as enums defined within other types.

## Basic Enum Registration

### Simple Enumeration Lists
- Use `BEGIN_ENUM_ITEM` and `END_ENUM_ITEM` for simple enumeration lists
- These enums represent discrete values that are not intended to be combined
- Each enum value represents a single, distinct option

```cpp
BEGIN_ENUM_ITEM(MyEnum)
    ENUM_CLASS_ITEM(FirstItem)
    ENUM_CLASS_ITEM(SecondItem)
END_ENUM_ITEM(MyEnum)
```

### Combinable Flag Enumerations
- Use `BEGIN_ENUM_ITEM_MERGABLE` and `END_ENUM_ITEM` for combinable flag enumerations
- These enums work like mixable flags, which are usually combined using the `|` operator
- Support bitwise operations for creating composite values

```cpp
BEGIN_ENUM_ITEM_MERGABLE(MyEnum)
    ENUM_CLASS_ITEM(FirstItem)
    ENUM_CLASS_ITEM(SecondItem)
END_ENUM_ITEM(MyEnum)
```

## Enum Member Registration

### Enum Class Members
- Use `ENUM_CLASS_ITEM` for enum class members
- Applied to C++11 scoped enumerations (enum class)
- Provides type safety and scope isolation

### Traditional Enum Members
- Use `ENUM_ITEM` for enum members
- Applied to traditional C-style enums
- Members are accessible in the surrounding scope

### Namespace-Scoped Enums
- Use `ENUM_ITEM_NAMESPACE` and `ENUM_NAMESPACE_ITEM` for enums (not enum classes) defined inside other types
- `ENUM_ITEM_NAMESPACE` declares the type name
- `ENUM_NAMESPACE_ITEM` lists each member
- Handles enums that are nested within classes or namespaces

## Usage Patterns

For any `enum` that works like a list of names:
```cpp
BEGIN_ENUM_ITEM(MyEnum)
    ENUM_CLASS_ITEM(FirstItem)
    ENUM_CLASS_ITEM(SecondItem)
END_ENUM_ITEM(MyEnum)
```

For any `enum` that works like mixable flags:
```cpp
BEGIN_ENUM_ITEM_MERGABLE(MyEnum)
    ENUM_CLASS_ITEM(FirstItem)
    ENUM_CLASS_ITEM(SecondItem)
END_ENUM_ITEM(MyEnum)
```

For items in an `enum class`, use `ENUM_CLASS_ITEM` to list each member.
For items in an `enum`, use `ENUM_ITEM` to list each member.

If the `enum` (not `enum class`) is defined inside other type, use `ENUM_ITEM_NAMESPACE` to declare the type name, followed with `ENUM_NAMESPACE_ITEM` to list each member.

## Extra Content

### Best Practices

1. **Choose Appropriate Type**: Use `BEGIN_ENUM_ITEM_MERGABLE` only for enums that are truly intended to be combined with bitwise operations
2. **Consistent Naming**: Follow consistent naming conventions for enum values
3. **Complete Registration**: Register all enum values that should be accessible through reflection

### Advanced Scenarios

#### Nested Enum Registration
When dealing with enums defined within classes:
```cpp
// For enum defined inside a class
BEGIN_ENUM_ITEM(OuterClass::InnerEnum)
    ENUM_ITEM_NAMESPACE(OuterClass)
    ENUM_NAMESPACE_ITEM(Value1)
    ENUM_NAMESPACE_ITEM(Value2)
END_ENUM_ITEM(OuterClass::InnerEnum)
```

#### Flag Combination Support
For mergable enums, the reflection system automatically supports:
- Bitwise OR operations for combining flags
- Bitwise AND operations for testing flags
- Conversion to/from underlying integer types
- String representation of combined values

### Performance Considerations

- Enum registration has minimal runtime overhead
- Reflection of enum values is typically fast
- Flag combination operations maintain C++ performance characteristics