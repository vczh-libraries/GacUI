# Binary Operators

## Type inferences

Here assume **A** is the left operand, and **B** is the right operand.

When type of **A** can be implicitly converted to type of **B**, **A** will be cast to type of **B** before being applied to this operator.

When type of **B** can be implicitly converted to type of **A**, **B** will be cast to type of **A** before being applied to this operator.

If implicitly casting does not exist in both way, an error will be generated.

For unlisted types, it means this operator is not allowed on those types.

### &

When type of converted operands are: - **string**: the result is **string**. - **flagenum**: the result is the same **flagenum**.

### |

- **flagenum**: the result is the same **flagenum**.

### ^

When type of converted operands are: - 8-bits or 16-bits integer: the result is **float**. - other integer types: the result is **double**. - floating-point numbers: the result is in the same floating-point numbers.

### +, -, *, /

When type of converted operands are: - 8-bits, 16-bits or 32-bits signed/unsigned integer: the result is 32-bits signed/unsigned integer. - 64-bits signed/unsigned integer: the result is 64-bits signed/unsigned integer. - floating-point numbers: the result is in the same floating-point numbers.

### %, shl, shr

When type of converted operands are: - 8-bits, 16-bits or 32-bits signed/unsigned integer: the result is 32-bits signed/unsigned integer. - 64-bits signed/unsigned integer: the result is 64-bits signed/unsigned integer.

### \<, \>, \<=, \>=

When type of converted operands are: - Integers, floating-point numbers or **string**: the result is **bool**.

### ==, !=

Any type applies, the result is **bool**.

### and, or, xor

When type of converted operands are: - **bool**: the result is **bool**. - integers: the result is in the same integer type.

## Definitions

### A & B

- Concatinate two strings. - Bit-wise **and** on two **flagenum**.

### A | B

- Bit-wise **or** on two **flagenum**.

### A ^ B

- Exponentiation. For example, **2 ^ 3 == 8.0**.

### A + B

- Adding.

### A - B

- Substracting.

### A * B

- Multiplying.

### A / B

- Dividing. - **(1 / 2) of double** gets **0**. - **(1.0 / 2)** or **(1 of double) / 2** gets **0.5**

### A % B

- Get the remainder from A / B.

### A (\<, \>, \<=, \>=, ==, !=) B

- Compare two values, returns **true** when the specified order is correct.

### A xor B

- **exclusive or** on **bool**. - bit-wise **exclusive or** on integers.

### A and B

- **and** on **bool**. - bit-wise **and** on integers.

For any **A and B** that is performed on two **bool** expressions: - When running Workflow script directly, B will be executed when A is **false**. - When running generated C++ code, B not will be executed when A is **false**.

### A or B

- **or** on **bool**. - bit-wise **or** on integers.

For any **A or B** that is performed on two **bool** expressions: - When running Workflow script directly, B will be executed when A is **true**. - When running generated C++ code, B not will be executed when A is **true**.

