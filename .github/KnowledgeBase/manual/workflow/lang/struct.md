# Structs

Structs can be defined in **modules**, **namespaces**, **classes** and **interfaces**.

A struct is a tuple with all fields assigned a name: ``` struct Point { x: int; y: int; } ``` Types of fields should only be value types, like: - primitive types - enums - structs - primitive types - nullable types A struct is not allowed to have itself or its nullable type as a direct or indirect field.

Structs cannot inherit from other structs.

## Allocating a struct

The constructor expression is the only way to allocate a value of a struct type: ``` func use(p: Point): Point { return p; } var a: Point = {x:1 y:2}; var b = {x:3 y:4} of Point; var c = use({x:5 y:6}); ``` A constructor expression does not have an explicit type, so the place receiving this expression must have an explicit type, otherwise an error will be generated.

**null** is not allowed to be assigned to a variable of a struct type. But there is an optional type that helps in this case: ``` var d: Point? = null; d = {x:7 y:8}; ```

## Accessing fields

The type of **a** is **Point**, so **a.x** is 1, and **a.y** is 2.

The type of **d** is **Point?**, so **d.x** is 7, and **d.y** is 8.

When **d** becomes **null**, accessing fields in **d** will result in throwing an exception.

Struct fields are readonly, after a struct is allocated, fields cannot be re-assigned. But the struct variable itself can be assigned with a new value.

## Ambiguity

Due to rich kinds of expression that Workflow supports, sometimes the constructor expression could introduce ambiguity. In this case, semicolons are allowed after every fields: ``` var e: Point = {x:1; y:2;}; ``` Please note that, semicolons must either appear after all fields, or completely be comiited.

