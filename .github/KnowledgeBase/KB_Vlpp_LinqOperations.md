# LINQ Operations

Functional programming operations on collections with lazy evaluation and method chaining.

## Overview

`LazyList<T>` implements Linq for C++ just like C#. Use `From(collection)` to create a `LazyList<T>` from any collection objects implementing `IEnumerable<T>`.

`LazyList<T>` also implements `IEnumerable<T>`.

In `LazyList<T>` there are many collection operating methods just like Linq for C#. When the expression is too long, line breaks are recommended before the `.` character like:

```cpp
From(xs)
  .Skip(3)
  .Reverse()
  .Where([](auto x){ return x < 5; })
  .Select([](auto x){ return x * 2; })
```

## APIs

- Use `LazyList<T>` for LINQ-style operations on any IEnumerable collection
- Use `From(collection)` to create LazyList from collections
- Use method chaining with `Skip()`, `Reverse()`, `Where()`, `Select()` for data transformation
- Use `indexed` function for enumeration with index access
- Use range-based for loops with any IEnumerable implementation

## Iterating with Collections, Linq, and also C++ Arrays/Pointers/STL Iterators

The C++ range based for loop also works with any collection objects implementing `IEnumerable<T>`.

Use `indexed(xs)` for `for(auto [x, index] : indexed(xs))`. The range wrapper yields `Tuple<const T&, vint>` values with the element before its index (`<Vlpp repo>/Source/Collections/OperationForEach.h`); it is not an `IEnumerable<Pair<vint, T>>`.

## Extra Content

Check out comments before `#ifndef VCZH_COLLECTIONS_OPERATION` for a full list of operators available in the LINQ implementation.

Many LINQ operations in Vlpp defer evaluation until enumeration. `OrderBy()` and `Reverse()` materialize their input, and `Evaluate()` can cache unevaluated input, while aggregations evaluate immediately (`<Vlpp repo>/Source/Collections/Operation.h`).

Common LINQ operations include:
- Filtering operations: `Where()`, `Take()`, `Skip()`
- Projection operations: `Select()`, `SelectMany()`
- Ordering operations: `OrderBy()`, `OrderByKey()`, `OrderBySelf()`, `Reverse()`
- Aggregation operations: `Aggregate()`, `All()`, `Any()`, `Count()`, `First()`, `Last()`
- Set operations: `Distinct()`, `Concat()`, `Union()`, `Intersect()`, `Except()`
- Conversion operations: converting to other collection types

The implementation is designed to be compatible with the existing collection infrastructure and provides similar functionality to .NET's LINQ while maintaining C++ idioms and performance characteristics.