# Functions and Variables

Functions and variables can be defined in **modules**, **namespaces** and **classes**.

## Variable

A variable could be ``` var a = "Hello"; var b: string = "Hello"; ```

The type is optional, when it can be inferred from the initializer.

Type inferring is available for most expressions, but there are still a few kinds of expression which don't have an explicit type. Like **null**. In this case, you have two choices, one is to specify the variable type, another one is to specify the expression type: ``` var a: MyClass = null; var b = null of MyClass; ```

The initializer can be any expression, it cannot be omitted.

Multiple variables cannot share the same name in one scope, but variables can hide other variables in parent scopes.

**IMPORTANT**: When a variable is class member, the type cannot be omitted.

## Function

Here is an example: ``` func main(): string { return "Hello, world!"; } ```

This function is called **main**. Multiple variables could share the same name in one scope, this is overloading. When such functions are called, One of them is picked according to argument types.

The return type and the function body cannot be omitted.

Functions are allowed to have parameters: ``` func add(x: int, y: int): int { return x + y; } ```

Two arguments are required to call this function: ``` add(1, 2); ```

There is no default value for any parameter. Function overloading is recommended to simulate default values for parameters.

When the return type of a function is **void**, any **return** statement in this function is not allowed to have an expression. When the return type is not **void**, any **return** statement in this function must have an expression.

The compiler does not check if any exit point of a function is missing a return value. If a function exits without a return value, it returns **null**. If the return type is not compatible with **null**, no error will be generated during compiling, it is an undefined behavior.

When such code is generated to C++, the C++ compiler will tell you that it is wrong.

## Overloading Resolution

Firstly, only functions whose have the correct number of parameters are valid candidates, because there is no default value for parameters.

Secondly, function arguments are used to pick one from overloaded functions. But not all arguments are used, because some of them does not have explicit types: - Unresolved name: Some objects, like enum items, are not always required to specify the enum type. Because the place accepting this expression may already have a known type. - Ordered lambda expression with arguments: Parameter types are not allowed on parameters of order lambda expressions, so when it has arguments, the type of such lambda expression is not clear. - **null**: it could be any reference type. - **{ field:value ... }**: it could be any struct type. - **EXPR as ***: the type is inferred by the place accepting this expression.

After all arguments with explicit types are recognized, each valid candidate is examined by counting the number of arguments that need type conversions.

During the examination, some functions may end up being discarded because arguments type are not appropriate. For example, passing **null** to an **int** parameter.

In the reduced candidate list, we pick one that has minimum numbers of argument type conversions. If there are more than one, then an error is generated.

For example: ``` class Base {} class Derived: Base{} func F(x: Base^): void {} func F(x: Derived^): void {} ``` **F(new Derived^())** will always pick the second one, because the first one needs 1 type conversion, but the second needs 0 type conversion. The second one wins.

