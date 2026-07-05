# Sorting and Ordering

## Overview

Vlpp provides algorithms for arranging data with support for both total and partial ordering relationships. The sorting system supports standard comparison operations using modern C++ comparison operators and handles cases where data cannot be fully ordered through specialized partial ordering processors.

## Quick Sort Implementation

### `Sort(T*, vint)` Functions

The sorting functions perform quick sort on raw pointer ranges. Use the two-argument overload when the element type has a usable `<=>` operator, or the three-argument overload when custom ordering is needed.

```cpp
// Sort an array of integers
vint numbers[] = {5, 2, 8, 1, 9};
Sort(numbers, 5);

// Sort with a custom comparator
Sort(numbers, 5, [](vint a, vint b) { return b <=> a; });
```

**Function signatures:**
```cpp
template<typename T, typename Compare>
void Sort(T* begin, vint count, Compare compare);

template<typename T>
void Sort(T* begin, vint count);
```

**Parameters:**
- **`T* begin`**: Pointer to the first element of the array to sort
- **`vint count`**: Number of elements in the array
- **`Compare compare`**: Lambda expression or function object returning an ordering value

**Key characteristics:**
- **In-place sorting**: Modifies the original array
- **Quick sort algorithm**: Efficient O(n log n) average case performance
- **Default comparator**: Uses `a <=> b` when no comparator is supplied
- **Custom comparators**: Flexible comparison logic through lambda expressions or function objects

## Modern C++ Comparison Support

### Using the Spaceship Operator (<=>)

The sorting system leverages C++20's three-way comparison operator for standardized ordering.

```cpp
// Using spaceship operator directly
std::strong_ordering ordering = a <=> b;

// In sorting comparators
Sort(data, count, [](const MyType& a, const MyType& b) {
    return a <=> b;
});
```

### Comparison Return Types

#### `std::strong_ordering`
Use for types with total ordering where all elements can be compared:
```cpp
Sort(numbers, count, [](vint a, vint b) -> std::strong_ordering {
    return a <=> b;  // Returns strong_ordering::less, equal, or greater
});
```

#### `std::weak_ordering`
Use for types where equivalent elements may not be identical:
```cpp
Sort(strings, count, [](const WString& a, const WString& b) -> std::weak_ordering {
    return a.Compare(b) <=> 0;  // Case-insensitive comparison
});
```

#### `std::partial_ordering`
Use only when every pair that reaches `Sort` is still comparable. If the comparator returns `std::partial_ordering::unordered`, `Sort` raises an `Error`.

```cpp
Sort(items, count, [](const MyType& a, const MyType& b) -> std::partial_ordering {
    return a.PartialCompare(b);
});
```

### Lambda Expression Comparators

Sorting relies on lambda expressions returning ordering values rather than boolean comparisons:

```cpp
// Correct: Using spaceship operator
Sort(items, count, [](const Item& a, const Item& b) {
    return a.priority <=> b.priority;
});

// Multi-level sorting
Sort(items, count, [](const Item& a, const Item& b) {
    if (auto cmp = a.category <=> b.category; cmp != 0)
        return cmp;
    return a.name <=> b.name;
});
```

## Partial Ordering Support

### `PartialOrderingProcessor`

For dependency sorting, use `PartialOrderingProcessor` instead of `Sort`. `PartialOrderingProcessor` is not a template. Initialize it with one relationship source, call `Sort()`, then read `components` and `nodes`.

```cpp
List<WString> items;
items.Add(L"compile");
items.Add(L"link");
items.Add(L"package");

Group<WString, WString> dependencies;
dependencies.Add(L"link", L"compile");
dependencies.Add(L"package", L"link");

PartialOrderingProcessor processor;
processor.InitWithGroup(items, dependencies);
processor.Sort();

for (vint i = 0; i < processor.components.Count(); i++)
{
    auto& component = processor.components[i];
    // component.firstNode points to indexes in processor.nodes.
}
```

**Initialization APIs:**
- **`InitWithGroup(items, depGroup)`**: Use a `Group<T, T>` where `depGroup.Add(a, b)` means `a` depends on `b`
- **`InitWithFunc(items, depFunc)`**: Use a callback where `depFunc(a, b)` returns true when `a` depends on `b`
- **`InitWithSubClass(items, depGroup, subClasses)`**: Group items into subclasses before sorting dependencies

**Result fields:**
- **`components`**: Sorted components. A component can contain multiple nodes when dependencies form a cycle.
- **`nodes`**: Node data referenced by components. With subclass sorting, a node can represent a subclass and exposes `firstSubClassItem` and `subClassItemCount`.

**Use cases for dependency sorting:**
- **Dependency resolution**: When items have prerequisites
- **Task scheduling**: When some tasks must complete before others
- **Type hierarchies**: When subclass groups must be ordered from relationships among original objects
- **Cycle grouping**: When mutually dependent items should be reported in the same component

### When Sort Doesn't Work

The standard `Sort` function requires every pair it compares to produce an ordered result. Use `PartialOrderingProcessor` when:

- Some elements cannot be meaningfully compared
- Topological sorting is required
- Dependency cycles need to be grouped into components
- Relationships are expressed as dependencies instead of pairwise ordering

## Best Practices

### Choosing the Right Approach

1. **Use `Sort()` when:**
   - All element pairs that may be compared produce ordered results
   - Performance is critical (quick sort is very efficient)
   - Working with simple data types (numbers, strings)

2. **Use `PartialOrderingProcessor` when:**
   - Working with dependency graphs
   - Need topological sorting
   - Cycles should be represented as grouped components
   - Items should be sorted through subclass groups

### Comparator Best Practices

```cpp
// Good: Clear, concise comparator
Sort(items, count, [](const Item& a, const Item& b) {
    return a.value <=> b.value;
});

// Good: Multi-level comparison with early exit
Sort(items, count, [](const Item& a, const Item& b) {
    if (auto cmp = a.priority <=> b.priority; cmp != 0)
        return cmp;
    if (auto cmp = a.category <=> b.category; cmp != 0)
        return cmp;
    return a.name <=> b.name;
});

// Good: Custom comparison logic
Sort(items, count, [](const Item& a, const Item& b) {
    // Reverse order (descending)
    return b.value <=> a.value;
});
```

### Memory Considerations

- **In-place sorting**: `Sort()` modifies the original array, no additional memory allocation
- **Pointer-based**: Works directly with memory addresses for efficiency
- **Custom types**: Ensure your types implement efficient comparison operators

## Integration with Collections

### Sorting Collections

The sorting functions work with raw pointers, but can be used with collection types:

```cpp
List<vint> numbers;
numbers.Add(5);
numbers.Add(2);
numbers.Add(8);

// Sort the underlying array
Sort(&numbers[0], numbers.Count(), [](vint a, vint b) { return a <=> b; });
```

### SortedList<T> Alternative

For automatically maintained sorted collections, consider using `SortedList<T>` instead of manual sorting:

```cpp
SortedList<vint> sortedNumbers;
sortedNumbers.Add(5);  // Automatically maintains order
sortedNumbers.Add(2);
sortedNumbers.Add(8);
// Collection is always sorted
```

## Extra Content

### Performance Characteristics

- **Quick Sort**: Average O(n log n), worst case O(n^2)
- **In-place**: No additional memory allocation for sorting
- **Comparison-based**: Performance depends on comparison function complexity
- **Cache-friendly**: Works with contiguous memory arrays

### Thread Safety

The sorting functions are not thread-safe. For concurrent access:
- Ensure exclusive access to the array being sorted
- Use appropriate synchronization primitives from VlppOS
- Consider sorting copies for read-heavy scenarios

### Custom Type Requirements

For custom types used in sorting:
- Implement appropriate comparison operators (`<=>` recommended)
- Ensure comparison is consistent and transitive
- Consider providing strong, weak, or partial ordering overloads as needed

### Debugging Comparators

When sorting doesn't work as expected:
- Verify comparator returns consistent results for same inputs
- Check for transitive property: if A < B and B < C, then A < C
- Ensure strict weak ordering for equivalent elements
- Test with simple cases first

```cpp
// Debug comparator
Sort(items, count, [](const Item& a, const Item& b) {
    auto result = a.value <=> b.value;
    // Add debug output if needed
    return result;
});
```
