# Enums

Enums can be defined in**modules**,**namespaces**,**classes**and**interfaces**.

There are two kinds of enums: enumerations and flags. The main difference between them is that, the "**&**" and "**|**" operator is not defined for enumerations.

A value of any enum can be explicitly converted to**system::UInt8**. A value of**system::UInt8**can also be explicitly converted to any enum type, but the runtime does not check if the conversion result is a valid enum value or not.

## Enumerations

An enumeration is a type that has finite values.
```
enum Seasons
{
    Spring = 0,
    Summer = 1,
    Autumn = 2,
    Winter = 3,
}
```

- Each item should be assigned an integer.
- The first item must be assigned with 0.
- Any other item must be 1 greater than the previous item.
- The last comma "**,**" must not be omitted.

There is 4 ways to use an enum item:
```
func use(s: Seasons):Seasons
{
    return s;
}

var a: Seasons = Spring;
var b = Seasons::Summer;
var c = Autumn of Seasons;
var d = use(Winter);
```


**var a: Seasons = Spring;**and**var d = use(Winter);**:

When the place accepting an enum item expression has a known type, the name of the enum item is enough. Variable**a**has its type explicitly declared. The parameter type of function**use**is also explicitly declared. So**Seasons::**is not needed.

**var b = Seasons::Summer;**:

Another way is to explicitly say the name of the enum.

**var c = Autumn of Seasons;**:

The third way is to explicitly declare the type of the expression. This usage is similar to**a**and**d**, by explicitly declared that**Autumn**is**Seasons**, the place accepting an enum item is "**\<HERE\> of Seasons**"", so this place has a known type.

## Flags

A flag is a type that has finite values and their combinations.
```
flagenum Seasons
{
    None = 0,
    Spring = 1,
    Summer = 2,
    Autumn = 4,
    Winter = 8,
    GoodSeasons = Spring | Autumn,
    BadSeasons = Summer | Winter,
}
```

- Each item should be assigned an integer.
- The first item must be assigned with 0. Usually the first item is**None**, you are free to use any name.
- Any other item must be twice as much as the previous item.
- Aliases of item combinations must be placed after all items.
- The last comma "**,**" must not be omitted.

Just like enums, flags also have type inferencing:
```
func use(s: Seasons):Seasons
{
    return s;
}

var a: Seasons = Spring | Summer;
var b = Seasons::Summer | Seasons::Autumn;
var c = (Autumn | Winter) of Seasons;
var d = use(Winter | Spring);
```


As you can see, the operator "**&**" and "**|**" are automatically defined for flags. When it is used on flags, it requires that both operands must be the same type. The type of the result is the same to its operands.
- "**|**" returns the union of two operands.
- "**&**" returns the intersection of two operands.These two operators are also defined for integers.

