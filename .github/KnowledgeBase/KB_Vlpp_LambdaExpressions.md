# Lambda Expressions and Callable Types

Function objects and event handling with type-safe callable containers for Vlpp.

## Prefer Lambda Expressions for Callbacks

Prefer lambda expressions for callbacks, unless for handling GacUI events or when the source file shows a trend for native functions.

Prefer lambda expressions as local functions, since C++ doesn't directly support local functions, it is not possible to make lambda local functions recursive in any way.
A lambda local function can only call other lambda local functions that defined before itself, by capturing their names as references.

## Func<T(TArgs...)>

`Func<F>` work just like `std::function<F>`, it can be initialized with:
- A `Func<F>` with different type, when arguments and return types can be implicitly converted in the expected direction.
- A lambda expression.
- A pointer to a native function.
- A pointer to a method of a class, in this case the first argument should be the pointer to the class or its subclass.

If a `Func<F>` is not assigned with any callable object, it is empty and can be detected using the `operator bool` operator.

`Func(callable-object)` could automatically infer the function type, even though it is unnecessary to say `Func` in most of the cases.

## Event<void(TArgs...)>

`Event<F>` can be assigned with multiple callable objects which are compatible with `Func<F>`, by calling the `Add` method.
The `Add` method returns a handle which could be used in `Remove` to revert the assigning.
When an `Event<F>` is called, all assigned callable objects are executed.

## Extra Content

### Performance Considerations

`Func<F>` uses type erasure similar to `std::function<F>`, which may involve heap allocation for large callable objects. For performance-critical code, consider using templates to avoid the overhead of function object wrapping.

### Memory Management

Both `Func<F>` and `Event<F>` follow value semantics and manage their internal resources automatically. They can safely capture objects by value or by reference, but be careful with reference captures when the referenced objects may be destroyed before the callable object is invoked.

### Thread Safety

`Event<F>` is not thread-safe by default. If you need to access them from multiple threads, appropriate synchronization mechanisms should be used to ensure thread safety.

### Usage Examples

```cpp
// Basic Func usage
Func<vint(vint, vint)> addFunc = [](vint a, vint b) { return a + b; };
auto result = addFunc(3, 4); // result is 7

// Event usage
Event<void(vint)> numberEvent;
auto handle1 = numberEvent.Add([](vint x) { Console::WriteLine(itow(x)); });
auto handle2 = numberEvent.Add([](vint x) { Console::WriteLine(L"Number: " + itow(x)); });

numberEvent(42); // Both handlers will be called

// Remove specific handler
numberEvent.Remove(handle1);
numberEvent(24); // Only the second handler will be called
```