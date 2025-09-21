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

You can convert an `IEnumerable<T>` to `IEnumerable<Pair<vint, T>>` using the `indexed` function, which is designed for `for(auto [index, x] : indexed(xs))` to iterate xs with an index. This is also an example of `Pair<K, V>` with structured binding.

## Extra Content

Check out comments before `#ifndef VCZH_COLLECTIONS_OPERATION` for a full list of operators available in the LINQ implementation.

The LINQ operations in Vlpp provide lazy evaluation, meaning operations are not executed immediately but are deferred until the results are actually needed. This allows for efficient chaining of operations without creating intermediate collections.

Common LINQ operations include:
- Filtering operations: `Where()`, `Take()`, `Skip()`, `TakeWhile()`, `SkipWhile()`
- Projection operations: `Select()`, `SelectMany()`
- Ordering operations: `OrderBy()`, `OrderByDescending()`, `Reverse()`
- Aggregation operations: `Aggregate()`, `All()`, `Any()`, `Count()`, `First()`, `Last()`
- Set operations: `Distinct()`, `Concat()`, `Union()`, `Intersect()`, `Except()`
- Conversion operations: converting to other collection types

The implementation is designed to be compatible with the existing collection infrastructure and provides similar functionality to .NET's LINQ while maintaining C++ idioms and performance characteristics.