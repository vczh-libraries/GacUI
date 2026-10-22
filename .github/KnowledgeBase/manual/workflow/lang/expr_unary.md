# Unary Operators

## Type inferences

For **+**, **-** and **not**, the result has the same type as **VALUE**. **cast** returns the specified or inferred target type. **variant in VALUE** has type **object** and requires a statically variant operand.

## Definitions

### + VALUE

- For numbers, returns **VALUE**.
- It is not allowed on other types.

### - VALUE

- For signed integers, returns -1 * **VALUE**.
- For floating-point numbers, returns -1 * **VALUE**.
- It is not allowed on other types.

### cast TYPE VALUE, cast * VALUE

Explicitly convert **VALUE** to **TYPE**. In **cast * VALUE**, the target type comes from the place receiving the result. A conversion that is not allowed is a compiler error; an allowed conversion can still fail at runtime and throw an exception. See [strong casting expressions](../.././workflow/lang/expr.md) and [variant conversion rules](../.././workflow/lang/variant.md).

### variant in VALUE

Extracts the active payload from a [variant](../.././workflow/lang/variant.md) as **object**, evaluating **VALUE** once. It shares the right-associative unary precedence of **+**, **-**, **not** and **cast**. For example, **cast int variant in value** extracts the payload and then performs an ordinary object-to-integer cast. Nonvariant operands are compiler errors, even when an object contains a variant box provided by native C++ code. Extraction removes one layer only; it preserves the value held by an active **object** option.

### not VALUE

- For integers, returns the bit-wise reversed value of **VALUE**.
- For **bool**, switch **true** and **false**.
- It is not allowed on other types.

