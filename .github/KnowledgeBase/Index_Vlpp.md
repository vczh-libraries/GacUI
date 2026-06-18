# Vlpp Knowledge Base

Project introduction remains in [Index.md](./Index.md#vlpp).

### Choosing APIs

#### String Types and Handling

Immutable string types for text processing across different encodings with UTF conversion capabilities.

- Use `WString` for general purpose wide character strings (UTF-16 on Windows, UTF-32 on other platforms)
- Use `AString` for ASCII string operations
- Use `U8String` for UTF-8 string handling
- Use `U16String` and `U32String` for specific UTF encoding requirements
- Use `ObjectString<T>` template when you need custom character types
- Use `Unmanaged`, `CopyFrom`, `TakeOver` static functions for string initialization
- Use `wtoi`, `wtoi64`, `wtou`, `wtou64` for string to integer conversion
- Use `itow`, `i64tou`, `utow`, `u64tow` for integer to string conversion
- Use `ftow`, `wtof` for double and string conversion
- Use `wupper`, `wlower` for case conversion
- Use `ConvertUtfString<From, To>` for template-based UTF string conversion
- Use `AtoB` functions (like `wtou8`, `u8tow`) for direct UTF encoding conversion

[API Explanation](./KB_Vlpp_StringTypes.md)

#### Exception Handling

Error reporting and exception management for fatal errors and recoverable conditions.

- Use `Error` base class for fatal errors that should never happen
- Use `Exception` base class for recoverable errors and control flow
- Use `CHECK_ERROR(condition, message)` to raise errors on assertion failures
- Use `CHECK_FAIL(message)` to raise errors without conditions

[API Explanation](./KB_Vlpp_ExceptionHandling.md)

#### Object Model and Memory Management

Reference counting smart pointers and object lifecycle management following specific inheritance patterns.

- Use `Object` base class for all reference types
- Use `Interface` base class for all interface types with virtual inheritance
- Use `Ptr<T>` for shared ownership of reference types instead of raw pointers
- Use `ComPtr<T>` for COM objects on Windows API
- Use `Nullable<T>` to add nullptr semantics to value types
- Use `struct` for value types and `class` for reference types

[API Explanation](./KB_Vlpp_ObjectModel.md)

#### Lambda Expressions and Callable Types

Function objects and event handling with type-safe callable containers.

- Use `Func<T(TArgs...)>` for function objects similar to std::function
- Use `Event<void(TArgs...)>` for multi-subscriber event handling
- Use lambda expressions for callbacks instead of native functions when possible
- Use `Func(callable-object)` for automatic type inference

[API Explanation](./KB_Vlpp_LambdaExpressions.md)

#### Primitive Value Types

Container types for organizing and manipulating related data values.

- Use `Pair<Key, Value>` for two-value tuples with key and value fields
- Use `Tuple<T...>` for multiple value organization without defining structs
- Use `Variant<T...>` for type-safe unions that can hold one of several types
- Use `Pair(k, v)` and `Tuple(a, b, c...)` for type inference
- Use `get<0>` method for tuple value access
- Use `Get<T>()`, `TryGet<T>()` for variant value access
- Use `Apply` with `Overloading` for variant type-specific handling

[API Explanation](./KB_Vlpp_PrimitiveTypes.md)

#### Date and Time Operations

Cross-platform date and time handling with timezone conversions and arithmetic operations.

- Use `DateTime::LocalTime()` and `DateTime::UtcTime()` for current time retrieval
- Use `DateTime::FromDateTime()` for creating specific date/time instances
- Use `ToLocalTime()` and `ToUtcTime()` for timezone conversions
- Use `Forward()` and `Backward()` for time arithmetic operations
- Use `InjectDateTimeImpl` to replace implementation for testing and customization
- Use `EjectDateTimeImpl` to remove injected implementations and restore previous ones

[API Explanation](./KB_Vlpp_DateTimeOperations.md)

#### Collection Types

Dynamic containers implementing IEnumerable interface with comprehensive manipulation capabilities.

- Use `Array<T>` for fixed-size collections with random access
- Use `List<T>` for dynamic arrays with insertion and removal operations
- Use `SortedList<T>` for automatically ordered collections
- Use `Dictionary<Key, Value>` for one-to-one key-value mappings
- Use `Group<Key, Value>` for one-to-many key-value relationships
- Use `Count()`, `Get(index)`, `Contains(value)`, `IndexOf(value)` for common operations
- Use `Add()`, `Insert()`, `Remove()`, `RemoveAt()`, `Clear()` for modification operations
- Use `Keys()`, `Values()` for dictionary access patterns

[API Explanation](./KB_Vlpp_CollectionTypes.md)

#### LINQ Operations

Functional programming operations on collections with lazy evaluation and method chaining.
Check out comments before `#ifndef VCZH_COLLECTIONS_OPERATION` for a full list of operators.

- Use `LazyList<T>` for LINQ-style operations on any IEnumerable collection
- Use `From(collection)` to create LazyList from collections
- Use method chaining with `Skip()`, `Reverse()`, `Where()`, `Select()` for data transformation
- Use `indexed` function for enumeration with index access
- Use range-based for loops with any IEnumerable implementation

[API Explanation](./KB_Vlpp_LinqOperations.md)

#### Sorting and Ordering

Algorithms for arranging data with support for both total and partial ordering relationships.

- Use `Sort(T*, vint)` for quick sort on raw pointer ranges
- Use lambda expressions returning `std::strong_ordering` or `std::weak_ordering` as comparators
- Use `PartialOrderingProcessor` for partial ordering scenarios where Sort doesn't work
- Use `<=>` operator to obtain ordering values for comparators

[API Explanation](./KB_Vlpp_SortingOrdering.md)

#### Console Operations

Basic input/output operations for console applications.

- Use `Console::Write` and `Console::WriteLine` for console output in CLI applications
- Use `Console::TryRead` for nullable line input that handles console input, redirection, and EOF

[API Explanation](./KB_Vlpp_ConsoleOperations.md)

#### Memory Leak Detection

Global storage management and memory leak detection for debugging and testing.

- Use `BEGIN_GLOBAL_STORAGE_CLASS`, `INITIALIZE_GLOBAL_STORAGE_CLASS`, `FINALIZE_GLOBAL_STORAGE_CLASS`, `END_GLOBAL_STORAGE_CLASS` for global variable management
- Use `FinalizeGlobalStorage()` before memory leak detection
- Use `GetStorageName().IsInitialized()` to check availability
- Use `_CrtDumpMemoryLeaks()` on Windows for leak detection

[API Explanation](./KB_Vlpp_MemoryLeakDetection.md)

#### Unit Testing Framework

Testing infrastructure with hierarchical test organization and assertion capabilities.

- Use `TEST_FILE` to define test file scope
- Use `TEST_CATEGORY(name)` for grouping related tests
- Use `TEST_CASE(name)` for individual test implementations
- Use `TEST_ASSERT(expression)` for test assertions
- Use nested `TEST_CATEGORY` for hierarchical organization
- Use `TEST_PRINT` for logging information to CLI in tests

[API Explanation](./KB_Vlpp_UnitTesting.md)

### Design Explanation
