# Vlpp Primitive Value Types

Container types for organizing and manipulating related data values in the Vlpp library.

## Nullable<T>

`Nullable<T>` adds `nullptr` to `T`. `Nullable<T>` can be assigned with `T`, it becomes non-empty, otherwise it is empty.

To reset a `Nullable<T>` to empty, use the `Reset()` method. To detect if it is empty, use the `operator bool` operator.

The `Value()` method can only be called if you are sure it is non-empty, and it returns the value inside it. `Value()` returns a immutable value, you can't change any data inside value, but you can assign it with a new value.

A `Nullable<T>` can be compared with another one in the same type in the standard C++ way.

### Usage Examples

```cpp
// Create nullable integer
Nullable<vint> nullableInt;
if (!nullableInt) {
    // nullableInt is empty
}

// Assign a value
nullableInt = 42;
if (nullableInt) {
    vint value = nullableInt.Value(); // value is 42
}

// Reset to empty
nullableInt.Reset();
```

## Pair<Key, Value>

`Pair<K, V>` store a `K` key and a `V` value, it is just an easy way to represent a tuple of 2 values. Use `key` and `value` field to access these values.

`Pair(k, v)` could be used to initialize a pair without specifying data type, make the code more readable.

A `Pair<K, V>` can be compared with another one in the same type in the standard C++ way.

A `Pair<K, V>` can be used with structured binding.

### Usage Examples

```cpp
// Create a pair with type inference
auto pair = Pair(L"name", 25);

// Access fields
WString name = pair.key;
vint age = pair.value;

// Structured binding
auto [personName, personAge] = pair;

// Comparison
Pair<WString, vint> pair1(L"Alice", 30);
Pair<WString, vint> pair2(L"Bob", 25);
bool isEqual = (pair1 == pair2); // false
```

### Structured Binding with Collections

You can convert an `IEnumerable<T>` to `IEnumerable<Pair<vint, T>>` using the `indexed` function, which is designed for `for(auto [index, x] : indexed(xs))` to iterate xs with an index. This is also an example of `Pair<K, V>` with structured binding.

```cpp
List<WString> names;
names.Add(L"Alice");
names.Add(L"Bob");

// Iterate with index using structured binding
for(auto [index, name] : indexed(names)) {
    // index is vint, name is WString
    Console::WriteLine(itow(index) + L": " + name);
}
```

## Tuple<T...>

`Tuple<...>` is an easy way to organize multiple values without defining a `struct`.

`Tuple(a, b, c...)` could be used to initialize a tuple without specifying data types, make the code more readable. Values cannot be changed in a tuple, but you can assign it with another tuple.

`get<0>` method can be used to access the first value. You can use any other index but it has to be a compiled time constant.

A `Tuple<...>` can be compared with another one in the same type in the standard C++ way.

A `Tuple<...>` can be used with structured binding.

### Usage Examples

```cpp
// Create tuple with type inference
auto tuple = Tuple(L"Alice", 25, true);

// Access elements using get<index>()
WString name = tuple.get<0>();
vint age = tuple.get<1>();
bool isActive = tuple.get<2>();

// Structured binding
auto [personName, personAge, active] = tuple;

// Comparison
auto tuple1 = Tuple(L"Alice", 25, true);
auto tuple2 = Tuple(L"Bob", 30, false);
bool isEqual = (tuple1 == tuple2); // false

// Assignment
auto tuple3 = tuple1; // Copy assignment
```

## Variant<T...>

`Variant<T...>` represents any but only one value of different types. It must be initialized or assigned with a value, a `Variant<T...>` could not be empty.

If you really want a nullable variable, add `nullptr_t` to the type list instead of using `Nullable<Variant<...>>`.

Use the `Index()` method to know the type stored inside a `Variant<T...>`, the return value starting from 0.

Use the `Get<T>()` method to get the value from a `Variant<T...>` if you are sure about the type.

Use the `TryGet<T>()` method to get the pointer to the value from a `Variant<T...>`, when the type is not the value stored in it, it returns `nullptr`.

Use the `Apply` with a callback to read the value in a generic way. The callback must be a lambda expression that could handle all different types, usually a template lambda expression.

You can use `Overloading` with `Apply` to handle the value of different types implicitly.

The `TryApply` method is similar to `Apply`, but you don't have to handle every cases.

### Usage Examples

```cpp
// Create variant that can hold string, int, or bool
Variant<WString, vint, bool> value = L"Hello";

// Check which type is stored
vint typeIndex = value.Index(); // 0 for WString, 1 for vint, 2 for bool

// Get value if you know the type
if (value.Index() == 0) {
    WString str = value.Get<WString>();
}

// Try to get value safely
if (auto ptr = value.TryGet<WString>()) {
    WString str = *ptr;
}

// Handle different types using Apply with Overloading
value.Apply(Overloading(
    [](WString& str) { 
        Console::WriteLine(L"String: " + str); 
    },
    [](const vint& num) { 
        Console::WriteLine(L"Number: " + itow(num)); 
    },
    [](bool flag) { 
        Console::WriteLine(flag ? L"True" : L"False"); 
    }
));

// Using TryApply - only handle some cases
value.TryApply(Overloading(
    [](WString& str) { 
        Console::WriteLine(L"Found string: " + str); 
    }
    // Other types are ignored
));

// Template lambda for generic handling
value.Apply([](auto& val) {
    // Handle any type generically
    using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_same_v<T, WString>) {
        Console::WriteLine(L"String type");
    }
});
```

### Nullable Variant

For a variant that can also be empty, include `nullptr_t` in the type list:

```cpp
Variant<WString, vint, bool, nullptr_t> nullableValue = nullptr;

if (nullableValue.Index() == 3) {
    // Value is null
}

// Assign a real value
nullableValue = L"Not null anymore";
```

## Extra Content

### Design Considerations

The primitive value types in Vlpp are designed to be:

1. **Immutable by design**: Values inside these containers cannot be modified directly, promoting safer code patterns
2. **C++20 compatible**: Full support for structured binding and modern C++ features
3. **Performance optimized**: Value types with minimal overhead compared to standard library equivalents
4. **Type-safe**: Strong typing prevents common errors while providing flexibility

### Best Practices

1. **Use type inference**: Prefer `auto` with `Pair()`, `Tuple()` constructors for cleaner code
2. **Leverage structured binding**: Use `auto [a, b, c] = tuple` for readable unpacking
3. **Handle all Variant cases**: When using `Apply()`, handle all possible types to avoid runtime issues
4. **Prefer TryGet for safety**: Use `TryGet<T>()` instead of `Get<T>()` when type is uncertain
5. **Use Nullable judiciously**: Only use `Nullable<T>` when `nullptr` semantics are truly needed

### Memory and Performance Notes

- All primitive value types are stack-allocated and have minimal memory overhead
- `Variant<T...>` uses union-like storage, only occupying space for the largest type plus type information
- Structured binding operations have zero runtime cost in optimized builds
- Comparison operations use lexicographical ordering for `Pair` and `Tuple`