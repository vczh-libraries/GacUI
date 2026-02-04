# Types

This type list`missing document: /home/registered.html`shows all types that you can use in a Workflow script. You can use[using declaration](../.././workflow/lang/module.md)to import names so that to make a type name shorter.

## Identifier


- **NAME**: uses a name that is imported by**using declarations**.
- **::NAME**: uses a name that is in the global namespace. If you define anything directly in a module, instead of putting it in a namespace, you are defining a concept in the global namespace.
- **IDENTIFIER::NAME**: uses a name that is declared in**IDENTIFIER**.**IDENTIFIER**could be a[namespace](../.././workflow/lang/module.md), a[class](../.././workflow/lang/class.md), or an[interface](../.././workflow/lang/interface.md). Note that you are not allowed to define a new type in a[struct](../.././workflow/lang/struct.md).

## Types in parentheses

Parentheses can change associations in a type, it is useful when there is ambiguity. For example:
```
var x: func():string{} = null;
```
This is ambiguous,**x**could be a function returning an enumerable of string, it could also be an enumerable of functions that returns a string. In order to tell the difference, we could use parentheses like this:
```
var x: func():(string{}) = null;
var y: (func():string){} = null;
```


## Primitive types

Here is all keyword for primitive types supported by Workflow script. Not all primitive types have a keyword, like**system::DateTime**, the only way to use it is to write**system::DateTime**, or shorten the name using a**using declaration**.
- **void**:**system::Void**. It can only appear at the function return type, indicating that this function do not have a return value.
- **object**:**system::Object**. Values of all types can be implicitly converted to**object**. A value of**object**can be explicitly converted to any type, conversion failure results in throwing an exception.**null**is also a valid value for**object**, casting**null**to an optional type results in**null**, casting**null**to any other type results in throwing an exception.
- **interface**:**system::Interface**. This is a reference type, the base type for all interfaces. it means that**interface***and**interface^**are valid types, but**interface**itself is not.
- **int**: When the compiler is running in an x86 process, it is**system::Int32**. When the compiler is running in an x64 process, it is**system::Int64**.
- **uint**: When the compiler is running in an x86 process, it is**system::UInt32**. When the compiler is running in an x64 process, it is**system::UInt64**.
- **float**:**system::Float**.
- **double**:**system::Double**.
- **string**:**system::String**. You can either use**'**or**"**to create a value of**string**. The following strings equal to each other:
```
var a = "a'\"";
var b = 'a\'"';
var c = "a" & "'" & '"';
```
Unlike other programming languages, the "**&**" operator joins two strings to one, instead of "**+**".
- **char**:**system::Char**. Workflow does not support**char**operations, you are not able to create any value of**char**, you are not able read**char**from**string**, unless it is returned from a C++ function.
- **bool**:**system::Boolean**. It could be**true**or**false**.

You could check out howpredefined types`missing document: /home/registered/vlppreflection.html`are mapped to C++ types.

### Type Conversions


- All primitive types could be implicitly converted to**string**.
- **string**could be explicitly converted to any primitive type.
- **float**could be implicitly converted to**double**.
- Signed integer types could be implicitly converted to any larger signed integer types.
- Unsigned integer types could be implicitly converted to any larger unsigned integer types.
- 8-bits and 16-bits integer types could be implicitly converted to**float**.
- 32-bits and 64-bits integer types could be implicitly converted to**double**.
- All numeric types could be explicitly converted to each other.
- Enum types could be explicitly converted to**system::UInt64**.
- **system::UInt64**could be explicitly converted to enum types.

## Reference types and Pointers

Any class types, interface types, or the type "**interface**", are all reference types. Optional types are not reference types. A reference type itself is not a valid type, pointers to a reference type are valid types.

### Shared pointers

Type**T^**means**Ptr\<T\>**in C++.

Althought**T**could be any type in C++, but in Workflow script, pointers could only point to reference types.

### Raw pointers

Type**T***means**T***in C++.

Althought**T**could be any type in C++, but in Workflow script, pointers could only point to reference types.

You can**delete**a raw pointer value as in C++. But it could result in throwing an exception, when there are shared pointers that holding this object.

## Optional types

Type**T?**means**Nullable\<T\>**in C++.

Althought**T**could be any type in C++, but in Workflow script, optional types could only point to primitive types, structs and enums.

**null**is a valid value for optional types.

Value of any valid type could be implicitly converted to its optional type.

Value of an optional types could be implicitly converted to its containing type.

An optional type**A?**can be implicitly or explicitly converted to**B?**, when**A**can be implicitly or explicitly converted to**B**.

## Function types

A boxed function type is**Ptr\<IValueFunctionProxy\>**, but it can also be unboxed to**Func\<T\>**.

Both function and member can be pack in a function type:
```
module sampleModule;

func A(): string
{
    return "A";
}

class B
{
    func M(): string
    {
        return "B";
    }
}

interface IC
{
    func M(): string;
}

var c = new IC^
{
    override func M(): string
    {
        return "C";
    }
}.M;

func main(): string
{
    var fs /* : (func(): string){} */ = {
        A;
        new B^().M;
        c;
        ["D"];
        func():string {return "E";};
    };

    var r = "";
    for (f in fs)
    {
        r = r & f();
    }
    return r;
}
```

- Function**A**can be stored in a variable of functions type.
- A class method can be stored in a variable of function type. We allocate**B**as a shared pointer. Even we have lost the pointer holding a**B**, but another pointer is in the function type, the reference count is maintained.
- An interface method can also be stored in a variable of function type.
- An ordered lambda expression can be stored in a variable of function type.
- A lambda expression can be stored in a variable of function type.

A function is allowed to have parameters, so as a function type.
```
module sampleModule;

func main(): string
{
    var add: func(int, int):int = [$1 + $2];
    var mul = func(x:int, y:int):int {return x * y;};
    return mul(add(1, 2), add(3, 4));
}
```


An ordered lambda expression does not have an explicit type, because type of parameters is not specified, so the place receiving it must have an explicit type.

A lambda expression has an explicit type, so the type of variable**mul**can be inferred.

## Collection types

Here are all supported collection types:
- `empty list item`
  **TValue {}**
  This is an readonly collection, it can be iterated, it has no random access.
  non-generic version:**system::Enumerable^**
  reflectable type:**system::IValueEnumerable**
- `empty list item`
  **TValue []**
  This is a list, it has random access.
  non-generic version:**system::List^**
  reflectable type:**system::IValueList**
- `empty list item`
  **const TValue []**
  This is a read-only list, it has random access.
  non-generic version:**system::ReadonlyList^**
  reflectable type:**system::IValueReadonlyList**
- `empty list item`
  **observe TValue []**
  This is an observable list, it has random access, changing the list triggers events.
  non-generic version:**system::ObservableList^**
  reflectable type:**system::IValueObservableList**
- `empty list item`
  **TValue [TKey]**
  This is a dictionary, it maps from a key to a value, it has random access on keys and values.
  non-generic version:**system::Dictionary^**
  reflectable type:**system::IValueDictionary**
- `empty list item`
  **const TValue [TKey]**
  This is a readonly dictionary, it maps from a key to a value, it has random access on keys and values.
  non-generic version:**system::ReadonlyDictionary^**
  reflectable type:**system::IValueReadonlyDictionary**

When calling methods of collection types, all keys and values are**object**, because Workflow doesn't support generic interface.

When using the "**[]**" operator, or using**for**statement, keys and values are strong-typed.

### Allocating lists

Collections and lists can be allocated by constructor expressions:
```
var a: int{} = {1 2 3 4 5};
var b: int[] = {1 2 3 4 5};
var c: const int[] = {1 2 3 4 5};
var d: observe int[] = {1 2 3 4 5};
var e /* : int[] */ = {1 2 3 4 5};
```
We can also use**{1; 2; 3; 4; 5;}**.

If we don't specify variable types, a constructor expression that making a list becomes**T []**.

Range expressions are very useful to create collections:
```
var a = range (1, 5); /* {2 3 4} of int{}       */
var b = range [1, 5); /* {1 2 3 4} of int{}     */
var c = range (1, 5]; /* {2 3 4 5} of int{}     */
var d = range [1, 5]; /* {1 2 3 4 5} of int{}   */
```

- Round parenthesis means the value on that side is included.
- Square parenthesis means the value on that side is not included.

### Iterating over lists

**TValue {}**,**TValue []**,**const TValue []**,**observe TValue []**can be used in**for**statements:
```
func main(): string
{
    var xs = {1 2 3 4 5};
    var sum = 0;
    for (x in xs)
    {
        sum = sum + x;
    }
    return sum;
}
```


**For**statements can iterate over a list in an opposite direction:
```
func main(): string
{
    var xs = {1 2 3 4 5};
    var sum = 0;
    for (x in revsersed xs)
    {
        sum = sum + x;
    }
    return sum;
}
```


Range expressions works with**for**statements very well:
```
func main(): string
{
    var sum = 0;
    for (x in range[1, 5])
    {
        sum = sum + x;
    }
    return sum;
}
```


### Allocating dictionaries

Dictionaries can be allocated by constructor expressions:
```
var a: string[int] = {1:"one" 2:"two" 3:"three"};
var b: const string[int] = {1:"one" 2:"two" 3:"three"};
var c /* : string[int] */ = {1:"one" 2:"two" 3:"three"};
```
We can also use**{1:"one"; 2:"two"; 3:"three";}**.

If we don't specify variable types, a constructor expression that making a dictionary becomes**TValue [TKey]**.

