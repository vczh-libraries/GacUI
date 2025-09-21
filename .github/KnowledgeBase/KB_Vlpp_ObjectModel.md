# Object Model and Memory Management

Reference counting smart pointers and object lifecycle management following specific inheritance patterns.

## Core Concepts

Always use `struct` for value types and `class` for reference types.
All reference types must inherits from `Object` or other reference types.
All interface types must virtual inherits from `Interface` or other interface types.
A reference type must virtual inherits an interface type to implement it.

## Ptr<T> - Smart Pointer

Prefer `Ptr<T>` to hold an initialized reference type instead of using C++ pointers, e.g. `auto x = Ptr(new X(...));`.
`Ptr<T>` is similar to `std::shared_ptr<T>`.
There is no `std::weak_ptr<T>` equivalent constructions, use raw C++ pointers in such cases, but you should try your best to avoid it.

### Basic Usage

```cpp
// Creating a Ptr
auto obj = Ptr(new MyClass(...));

// Assignment and copying
Ptr<MyClass> another = obj;  // Reference counting

// Checking if empty
if (obj) {
    // Ptr is not empty
}
if (obj == nullptr) {
    // Ptr is empty
}
```

### Type Conversion

If `T*` could be implicitly converted to `U*`, `Ptr<U>` could be initialized with `Ptr<T>`.

If `T*` could be `dynamic_cast` to `U*`, use `Cast<U>()` method instead.

```cpp
Ptr<BaseClass> base = Ptr(new DerivedClass(...));
Ptr<DerivedClass> derived = base.Cast<DerivedClass>();
```

### Resetting

To reset a `Ptr<T>`, assign it with `{}` or `nullptr`.

```cpp
obj = {};        // Reset to empty
obj = nullptr;   // Reset to empty
```

## ComPtr<T> - COM Object Pointer

`ComPtr<T>` is similar to `Ptr<T>` but it is for COM objects with Windows API only.

```cpp
ComPtr<IUnknown> comObj = ...;
```

## Nullable<T> - Optional Value Types

`Nullable<T>` adds `nullptr` semantics to value types. `Nullable<T>` can be assigned with `T`, it becomes non-empty, otherwise it is empty.

### Basic Operations

```cpp
Nullable<vint> value;        // Empty by default
value = 42;                  // Now contains 42

// Checking if empty
if (value) {
    // Has value
    vint actual = value.Value();
}

// Resetting to empty
value.Reset();

// Comparison
Nullable<vint> other = 42;
if (value == other) {
    // Equal comparison
}
```

### Key Methods

- **`Reset()`**: Makes the nullable empty
- **`operator bool`**: Returns true if non-empty
- **`Value()`**: Returns the contained value (only call if non-empty)

The `Value()` method can only be called if you are sure it is non-empty, and it returns the value inside it. `Value()` returns a immutable value, you can't change any data inside value, but you can assign it with a new value.

A `Nullable<T>` can be compared with another one in the same type in the standard C++ way.

## Inheritance Patterns

### Reference Types (class)

```cpp
class MyRefType : public Object
{
    // Reference type implementation
};
```

### Interface Types (class)

```cpp
class IMyInterface : public virtual Interface
{
public:
    virtual void DoSomething() = 0;
};
```

### Implementing Interfaces

```cpp
class MyImplementation : public Object, public virtual IMyInterface
{
public:
    void DoSomething() override
    {
        // Implementation
    }
};
```

### Multiple Interface Inheritance

```cpp
class IAdvancedInterface : public virtual IMyInterface
{
public:
    virtual void DoAdvancedThing() = 0;
};
```

## Special Cases

### Exception Classes

Use `class` for defining new `Error` or `Exception` sub classes, although they are value types.

```cpp
class MyCustomError : public Error
{
public:
    MyCustomError(const WString& message) : Error(message) {}
};
```

### Function and Event Types

`Func<F>` and `Event<F>` are also classes, although they are value types.

### Collection Types

Collection types are also value types, although they implements `IEnumerable<T>` and `IEnumerator<T>`.
This is also a reason we always use references instead of pointers on `IEnumerable<T>` and `IEnumerator<T>`.

### Struct in Ptr<T>

When really necessary, a struct could be used in `Ptr<T>` for sharing. But prefer `Nullable<T>` when `nullptr` is helpful but sharing is not necessary.

```cpp
struct SharedData : public Object
{
    vint value;
    WString name;
};

Ptr<SharedData> sharedPtr = Ptr(new SharedData{42, L"Hello"});
```

## Memory Management Best Practices

### Reference Counting

- `Ptr<T>` uses reference counting for automatic memory management
- Objects are automatically deleted when the last `Ptr<T>` is destroyed
- No need for manual memory management in most cases

### Avoiding Circular References

Since there's no weak pointer equivalent, be careful of circular references:

```cpp
// Potential circular reference problem
class Parent : public Object
{
public:
    List<Ptr<Child>> children;
};

class Child : public Object
{
public:
    Ptr<Parent> parent;  // This can create cycles
};
```

Use raw pointers for back-references when appropriate:

```cpp
class Child : public Object
{
public:
    Parent* parent;  // Raw pointer to avoid cycles
};
```

### Value vs Reference Types

- **Use `struct`**: For value types (simple data containers, POD types)
- **Use `class`**: For reference types (complex objects, polymorphic types)
- **Inherit from `Object`**: All reference types
- **Virtual inherit from `Interface`**: All interface types

## Extra Content

### Thread Safety

`Ptr<T>` reference counting is thread-safe, meaning multiple threads can safely copy and destroy `Ptr<T>` instances. However, the pointed-to object itself is not automatically thread-safe.

### Performance Considerations

- Reference counting has overhead compared to raw pointers
- `Ptr<T>` creation and destruction involve atomic operations
- Consider using raw pointers for temporary references within a single function
- Use `Nullable<T>` instead of `Ptr<T>` for value types that need null semantics

### Design Philosophy

The object model enforces clear distinctions between:
- Value types (lightweight, copyable) vs Reference types (heavyweight, shared)
- Concrete classes vs Interfaces
- Automatic memory management vs Manual lifetime control

This design promotes:
- Clear ownership semantics
- Reduced memory leaks
- Type safety
- Polymorphic behavior through interfaces