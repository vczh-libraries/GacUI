# Sorting and Ordering

## Overview

Vlpp provides algorithms for arranging data with support for both total and partial ordering relationships. The sorting system supports standard comparison operations using modern C++ comparison operators and handles cases where data cannot be fully ordered through specialized partial ordering processors.

## Quick Sort Implementation

### Sort(T*, vint) Function

The primary sorting function performs quick sort on raw pointer ranges with custom comparators.

```cpp
// Sort an array of integers
vint numbers[] = {5, 2, 8, 1, 9};
Sort(numbers, 5, [](vint a, vint b) { return a <=> b; });
```

**Function signature:**
```cpp
template<typename T, typename Compare>
void Sort(T* begin, vint count, Compare compare);
```

**Parameters:**
- **`T* begin`**: Pointer to the first element of the array to sort
- **`vint count`**: Number of elements in the array
- **`Compare compare`**: Lambda expression or function object for comparison

**Key characteristics:**
- **In-place sorting**: Modifies the original array
- **Quick sort algorithm**: Efficient O(n log n) average case performance
- **Custom comparators**: Flexible comparison logic through lambda expressions

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

#### std::strong_ordering
Use for types with total ordering where all elements can be compared:
```cpp
Sort(numbers, count, [](vint a, vint b) -> std::strong_ordering {
    return a <=> b;  // Returns strong_ordering::less, equal, or greater
});
```

#### std::weak_ordering  
Use for types where equivalent elements may not be identical:
```cpp
Sort(strings, count, [](const WString& a, const WString& b) -> std::weak_ordering {
    return a.Compare(b) <=> 0;  // Case-insensitive comparison
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

### PartialOrderingProcessor

For scenarios where not all elements can be compared (partial ordering), use `PartialOrderingProcessor` instead of the standard `Sort` function.

```cpp
// Example: Dependency sorting where some items have no ordering relationship
PartialOrderingProcessor<MyType> processor;

// Add items and their relationships
processor.AddItem(item1);
processor.AddItem(item2);
processor.AddItem(item3);

// Define partial ordering relationships
processor.AddDependency(item2, item1);  // item2 depends on item1
processor.AddDependency(item3, item1);  // item3 depends on item1

// Process to get topologically sorted result
auto sortedItems = processor.Process();
```

**Use cases for partial ordering:**
- **Dependency resolution**: When items have prerequisites
- **Task scheduling**: When some tasks must complete before others
- **Type hierarchies**: When comparing types with inheritance relationships
- **Version constraints**: When some versions are incomparable

### When Sort Doesn't Work

The standard `Sort` function requires total ordering - every pair of elements must be comparable. Use `PartialOrderingProcessor` when:

- Some elements cannot be meaningfully compared
- Circular dependencies need to be detected
- Topological sorting is required
- The comparison relationship is not transitive across all elements

## Best Practices

### Choosing the Right Approach

1. **Use `Sort()` when:**
   - All elements have a clear total ordering
   - Performance is critical (quick sort is very efficient)
   - Working with simple data types (numbers, strings)

2. **Use `PartialOrderingProcessor` when:**
   - Not all elements can be compared
   - Dealing with dependency graphs
   - Need topological sorting
   - Circular dependency detection is important

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

- **Quick Sort**: Average O(n log n), worst case O(n²)
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
- Consider providing both strong and weak ordering overloads as needed

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