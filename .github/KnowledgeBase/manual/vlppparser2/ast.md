# AST Definition

An AST definition consists of multiple**Enum**and**Class**. A**@public**can be used before any type definition to make it visible from another AST definition file.

Single line comments are allowed using**//**.

## Enum


```AST
enum Season
{
    Spring,
    Summer,
    Autumn,
    Winter,
}
```


## Class


```AST
class Base
{
    var name : token;
}

class Derived : Base
{
    var season : Season;
    var baseArray : Base[];
}
```


Inheriting between classes works as other programming languages.

Here are a list of supported field types:
- **token**: vl::glr::ParsingToken
- **enum type**
- **class type**: vl::Ptr\<ClassType\>
- **array to class type**: vl::collections::List\<vl::Ptr\<ClassType\>\>

A**@ambiguous**can be used before a class definition, indicating that any syntax rule of exactly this type is allow to produce results with ambiguity. The type of such rule is rewritten to**ClassTypeToResolve**, with a member**var candidates : ClassType[];**to store all results that could be produced from the same input. A rule of a sub type to an ambiguous type is not automatically allowed to be ambiguous, but you can explicitly specific the type to make such rule use the ambiguous base type.

