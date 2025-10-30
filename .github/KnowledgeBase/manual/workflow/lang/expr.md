# Expressions

All supported expressions are listed below.

## null, true, false

- **null** could be **T***, **T^** or **T?**. - **true** is **bool**. - **false** is **bool**.

## 1024, 10.24

For **/d+**, if the place receiving an interger does not have an explicit type, it is **int**, which is **Int32** when the Workflow compiler is running in a 32-bits process, which is **Int64** when the Workflow compiler is running in a 64-bits process. An error is generated if the number exceeds the range supported by the expected type.

It is very easy to specify an expected integer type like: **0 of UInt64**.

For **/d+./d+**, if the place receiving an interger does not have an explicit type, it is **double** when the Workflow compiler is running in a 64-bits process. An error is generated if the number exceeds the range supported by the expected type.

## "string"

A string can be contained in a pair of **'** or **"**.

If a string is contained in **'**, **\'** represents the **'** character.

If a string is contained in **"**, **\"** represents the **"** character.

Character escaping only accepts **\r**, **\n** and **\t** for 3 different space characters just like in C++.

For other characters that is following **\**, it escapes to itself without the **\** prefix. For example, **\\** becomes **\**, **\X** becomes **X**.

## $"$(formatted) string"

If **$** is put before a string, then multiple **$(EXPRESSION)** could be used inside the string.

All these expressions must return a value of types that can be implicitly cast to **string**.

After casting all of them to strings, these strings will be put inside the specified position to form a string. For example, **$"1 + 1 = $(1 + 1)"** becomse **"1 + 1 = 2"**.

## Unary operator expression: op VALUE

See [Unary Operators](../.././workflow/lang/expr_unary.md).

## Binary operator expression: VALUE op VALUE

See [Binary Operators](../.././workflow/lang/expr_binary.md).

## If expression: CONDITION ? VALUE : VALUE

**C ? A : B** returns **A** when **C** is **true**.

**C ? A : B** returns **B** when **A** is **true**.

Only one of **A** or **B** will be evaluated according to the value of **C**.

If **A** and **B** are not in the same type, then it requires that one must be able to be implicitly cast to another.

## Try expression: VALUE ?? DEFAULT

**A ?? B** returns **A**. If evaluating **A** results in throwing an exception, **B** is returned instead.

If **A** and **B** are not in the same type, then it requires that one must be able to be implicitly cast to another.

## Assignment expression: DESTINATION = VALUE

**A = B** assigns **B** to **A**.

If **A** is a property, then the setter function will be called.

The result of this expresison is from the evaluation of **B**.

## Ordered lambda expression: [$1 + $2]

This expression builds a lambda expression, it returns the expression in the square parentheses, with argument types and return type automatically inferred by the explicit type specified in the place receiving this expression.

You can use multiple arguments from **$1** to any number, but all numbers must be consecutive.

For example: ``` var f: func(string, string):string = [$`Hello, $($1) and $($2)!`]; ``` is exactly the same as ``` var f = func(a: string, b: string): string { return `Hello, $(a) and $(b)!`; } ```

## Lambda expression: func(x: int, y: int): int{ return x + y; }

This expression creates a function, with argument types and return type specified explicitly.

Multiple arguments are allowed.

All used local variables from outside scopes are captured by shallow copying.

This means that, if variable holding a class instance is captured by a lambda expression, you cannot reassign another object to this variable, but you can change properties of this object.

This also means that, changing a captured variable outside of the lambda expression after it is created takes no effect inside the lambda expression. ``` func main(): int { var x = 1; var f = func():int { return x + 1; }; x = 2; return f(); } ``` This function returns **2**.

Global variables are not used by capturing, they can always be reassigned in lambda expressions. ``` var x = 1; func main(): int { var f = func():int { return x + 1; }; x = 2; return f(); } ``` This function returns **3**.

## Let expression: let x = 1, y = 2 in (x + y)

This expression creates a new scope for readonly local varaibles, which are accessible in the last expression that is in the round parentheses.

Multiple variables are allowed, they are separated by "**,**"".

The round parentheses for the last expression must not be omitted.

``` func main(): int { return let x = 1, y = 2 in (x + y); } ``` This function returns **3**.

## Range expression: range (1, 10)

See **Collection types** in [Types](../.././workflow/lang/type.md).

## Set testing expression: VALUE in OBJECT, VALUE not in OBJECT

See **Collection types** in [Types](../.././workflow/lang/type.md).

## Constructor expression: {a b c}, {a:x b:y c:z}

See **Collection types** in [Types](../.././workflow/lang/type.md).

See **Allocating a struct** in [Structs](../.././workflow/lang/struct.md).

## New class expression: new TYPE(ARGUMENTS)

See [Allocating (class)](../.././workflow/lang/class_new.md).

## New interface expression: new TYPE{ MEMBERS }

See [Implementing (interface)](../.././workflow/lang/interface_new.md).

## Mixin new interface expression: new TYPE(using OBJECT)

See [Generic (interface)](../.././workflow/lang/interface_using.md).

## Call expression: FUNCTION(ARGUMENTS)

See **Function** in [Functions and Variables](../.././workflow/lang/funcvar.md).

## Instance member accessing: OBJECT.MEMBER

**A.B** uses a member called **B** in an object that is held by a value **A**.

It doesn't matter whether **A** is a shared pointer or a raw pointer.

When **B** are a group of overloading functions, the place receiving this expression is required to have an explicit type. For example: ``` module sampleModule; class C { var x: int = 100; func F():int    { return x; } func F():string { return x; } } func main(): string { var f = new C^().F; return f(); } ``` generates an error, because it is not able to determine which **F** is expected. But: ``` module sampleModule; class C { var x: int = 100; func F():int    { return x; } func F():string { return x; } } func main(): string { var f: func(): string = new C^().F; return f(); } ``` obviously uses the second **F**.

## Static member accessing: OBJECT::MEMBER, ::MEMBER

See [Module](../.././workflow/lang/module.md).

## Index expression: CONTAINER[KEY]

See **Collection types** in [Types](../.././workflow/lang/type.md).

## Null testing expression: OBJECT is null, OBJECT is not null

- **A is null** returns true when **A** is null. - **A is not null** returns true when **A** is not null.

## Type testing expression: OBJECT is TYPE, OBJECT is not TYPE

- **A is T** returns true when **A** is **T**. - **A is not T** returns true when **A** is not **T**.

**T** can be any type. If T is a class or an interface, it tests if the variable **A** is holding an object of or inherits that type. For example, **new C^() is C** returns **true**.

If **T** is a non-reference type, then typically **A** is **object**, this is the only case when you need to test such type.

When **T** is a shared pointer type or a raw pointer type, the type of the pointer is also considered. For example, **new C^() is C*** returns **false**.

## Strong casting expression: cast TYPE OBJECT, cast * OBJECT

For class types or interface types, **TYPE** must be a pointer type.

These expressions can cast objects between shared pointers and raw pointers.

These expressions also test if **OBJECT** is actually or inherits **TYPE**, if not, then the expression fails.

When **cast TYPE OBJECT** fails, it throws an expression.

When **TYPE** is *****, it is inferred by the place receiving this expression.

## Weak casting expression: OBJECT as TYPE, OBJECT as *

For class types or interface types, **TYPE** must be a pointer type.

**TYPE** must be compatible to **null**, this requires **TYPE** to be either a pointer type or an optional type.

These expressions can objects cast between shared pointers and raw pointers.

These expressions also test if **OBJECT** is actually or inherits **TYPE**, if not, then the expression fails.

When **OBJECT as TYPE** fails, it returns **null**.

When **TYPE** is *****, it is inferred by the place receiving this expression.

## Infer expression: OBJECT of TYPE

This expression gives an explicit type **TYPE** to the expression **OBJECT**.

It generates an error when it is not possible to do that.

**null of int** generates such error, because **null** can never be **int**.

## Event expression: attach(EVENT, FUNCTION), detach(EVENT, HANDLER)

See **Using events** in [Properties and Events (interface)](../.././workflow/lang/interface_prop.md).

See **Using events** in [Properties and Events (class)](../.././workflow/lang/class_prop.md).

