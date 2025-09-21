# Collection Types

Dynamic containers implementing IEnumerable interface with comprehensive manipulation capabilities.

## Overview

Every collection type in Vlpp implements `IEnumerable<T>`, providing a unified interface for enumeration. The collection system works similarly to C# collections, with `IEnumerable<T>` and `IEnumerator<T>` providing consistent iteration patterns across all container types.

## Core Collection Types

### Array<T>

Fixed-size collection with random access capabilities.

An `Array<T>` provides random access to elements and can be initialized with a specific size or copied from existing data.

**Initialization:**
- `Array<T>(vint size)` - Creates array with specified size filled with random values
- `Array<T>(T* buffer, vint size)` - Creates array copied from buffer

**Access and Query:**
- Use `Count()` to know the size of the collection
- Use `Get(index)` or `[index]` to get the value from a specified position
- Use `Contains(value)` or `IndexOf(value)` to find a value

**Modification:**
- Use `Resize(size)` to resize an array and keep the values, if the new size is larger than the old size, the array is filled with random values at the end
- Use `Set(index, value)` or `[index] = value` to set the value to a specified position

### List<T>

Dynamic array with insertion and removal operations.

A `List<T>` works like an `Array<T>` with dynamic size, providing efficient insertion and removal operations.

**Access and Query:**
- Use `Count()` to know the size of the collection
- Use `Get(index)` or `[index]` to get the value from a specified position
- Use `Contains(value)` or `IndexOf(value)` to find a value

**Modification:**
- Use `Add(value)` to add a value at the end
- Use `Insert(index, value)` to insert a value to a specified position, which means `l.Insert(l.Count(), value)` equivalents to `l.Add(value)`
- Use `Remove(value)` to remove the first equivalent value
- Use `RemoveAt(index)` to remove the value of the specified position
- Use `RemoveRange(index, count)` to remove consecutive values
- Use `Clear()` to remove all values
- Use `Set(index, value)` or `[index] = value` to set the value to a specified position

### SortedList<T>

Automatically ordered collection maintaining sort order.

A `SortedList<T>` works like a `List<T>` but it always keeps all values in order. It has everything a `List<T>` has except `Insert` and `Set` operations (which would break the ordering).

**Access and Query:**
- Use `Count()` to know the size of the collection
- Use `Get(index)` or `[index]` to get the value from a specified position
- Use `Contains(value)` or `IndexOf(value)` to find a value

**Modification:**
- Use `Add(value)` to insert a value while keeping all values in order
- Use `Remove(value)` to remove the first equivalent value
- Use `RemoveAt(index)` to remove the value of the specified position
- Use `RemoveRange(index, count)` to remove consecutive values
- Use `Clear()` to remove all values

## Associative Collections

### Dictionary<Key, Value>

One-to-one key-value mapping with ordered keys.

A `Dictionary<K, V>` is an one-to-one map that keeps all values in the order of keys. It implements `IEnumerable<Pair<K, V>>`.

**Access and Query:**
- Use `Count()` to know the size of the collection
- Use `Keys()` to get an immutable collection of keys
- Use `Values()` to get an immutable collection of values in the order of keys
- Use `Get(key)` or `[key]` to access a value by its key

**Modification:**
- Use `Add(key, value)` or `Add(pair)` to assign a value with a key, it crashes if the key exists
- Use `Set(key, value)` or `Set(pair)` to assign a value with a key, it overrides the old value if the key exists
- Use `Remove(key)` to remove the value with a key
- Use `Clear()` to remove all values

### Group<Key, Value>

One-to-many key-value mapping with ordered keys.

A `Group<K, V>` is an one-to-many map that keeps all values in the order of keys. It implements `IEnumerable<Pair<K, V>>`.

**Access and Query:**
- Use `Count()` to know the size of keys
- Use `Keys()` to get an immutable collection of keys
- Use `Get(key)` or `[key]` to access all values by its key. `g.GetByIndex(index)` equivalents to `g.Get(g.Keys()[index])`
- Use `Contains(key)` to determine if there is any value assigned with the key
- Use `Contains(key, value)` to determine if the value is assigned with the key

**Modification:**
- Use `Add(key, value)` or `Add(pair)` to assign one more value with a key
- Use `Remove(key, value)` to remove the value with a key
- Use `Remove(key)` to remove all values with a key
- Use `Clear()` to remove all values

## Enumeration and Iteration

### IEnumerable<T> Interface

All collection types implement `IEnumerable<T>`, providing consistent enumeration capabilities across the collection system.

**Range-based for loops:**
```cpp
List<vint> numbers;
// ... populate numbers ...

for (auto number : numbers)
{
    // Process each number
}
```

**Indexed enumeration:**
Use the `indexed` function to convert an `IEnumerable<T>` to `IEnumerable<Pair<vint, T>>` for iteration with indices:

```cpp
for (auto [index, value] : indexed(collection))
{
    // Process value with its index
}
```

This pattern works with structured binding and provides both the index and value for each element.

## Integration with LINQ

All collection types can be used with LINQ operations through the `From(collection)` function, which creates a `LazyList<T>` for functional programming operations:

```cpp
auto result = From(numbers)
    .Where([](vint x) { return x > 0; })
    .Select([](vint x) { return x * 2; })
    .ToList();
```

## Common Patterns

### Dictionary vs Group
- Use `Dictionary<K, V>` when each key maps to exactly one value
- Use `Group<K, V>` when keys can have multiple associated values

### Array vs List vs SortedList
- Use `Array<T>` for fixed-size collections with known size
- Use `List<T>` for dynamic collections with frequent insertions/removals
- Use `SortedList<T>` when you need automatic ordering without manual sorting

### Value Types vs Reference Types
Collections work with both value types and reference types:
- Value types are stored directly in the collection
- Reference types should use `Ptr<T>` for proper memory management

## Extra Content

### Memory Management
Collections handle memory management automatically. When storing reference types, use `Ptr<T>` for proper reference counting and automatic cleanup.

### Thread Safety
Collections are not thread-safe by default. For concurrent access, use appropriate synchronization primitives from VlppOS.

### Collection Copying
Collections support copy semantics - copying a collection creates a new independent instance with copies of all elements (for value types) or shared references (for `Ptr<T>` reference types).

### Comparison Operations
Collections that maintain order (SortedList, Dictionary, Group) require their element types to support comparison operations. Ensure your types implement appropriate comparison operators or provide custom comparators.