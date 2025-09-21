# Exception Handling

There are `Error` and `Exception`.

`Error` is a base class, representing fatal errors which you can't recover by try-catch.
In some rare cases when you have to catch it, you must always raise it again in the catch statement.
It can be used to report a condition that should never happen.
Use `CHECK_ERROR(condition, L"string-literal")` to raise an `Error` when the assertion fails.
Use `CHECK_FAIL(L"string-literal")` to raise an `Error`. It is similar to `CHECK_ERROR` but it allows you to say a failed assertion if the condition cannot be described by just a condition.

`Exception` is a base class, representing errors that you may want to catch.
It can be used to report error that made by the user.
Some code also use `Exception` as control flows. For example, you could define your own `Exception` sub class, raise it inside a deep recursion and catch it from the outside, as a way of quick exiting.

## Special Cases

Use `class` for defining new `Error` or `Exception` sub classes, although they are value types.

## Extra Content

### Error Usage Patterns

When using `Error`, the philosophy is that these represent programming errors or system inconsistencies that should never occur in properly written code. Examples include:

- Array bounds violations that should have been checked
- Invalid state transitions that violate class invariants
- Null pointer dereferences that should have been prevented
- Resource corruption that indicates system failure

### Exception Usage Patterns

`Exception` is more appropriate for user-facing errors or expected failure conditions that the application should handle gracefully:

- File not found when user specifies an invalid path
- Network connectivity issues during data transfer
- Invalid user input that needs validation feedback
- Resource exhaustion that can be recovered from

### Control Flow Applications

The mention of using `Exception` for control flow refers to advanced patterns where exceptions are used for non-local exits:

- Breaking out of deeply nested loops
- Early returns from complex recursive algorithms
- Implementing coroutine-like behavior
- Managing complex state machine transitions

### Best Practices

1. Always prefer `Exception` over `Error` unless the condition truly represents a programming error
2. When catching `Error`, always re-throw it after any necessary cleanup
3. Use descriptive string literals in `CHECK_ERROR` and `CHECK_FAIL` to aid debugging
4. Consider defining custom `Exception` subclasses for specific error domains
5. Document whether functions can throw `Error` vs `Exception` vs both