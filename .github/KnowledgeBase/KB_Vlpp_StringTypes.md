# String Types and Handling

## Overview

Vlpp provides immutable string types for text processing across different encodings with UTF conversion capabilities. The project prefers `wchar_t` over other char types and uses immutable string types instead of standard C++ string types. The string system offers smart pointer-like memory management, UTF encoding conversion, and locale-aware operations.

## Core String Types

### ObjectString<T>

`ObjectString<T>` is an immutable string template that serves as the foundation for all string types. It cannot be modified after initialization - any updating operations result in a new string value being returned.

**Key characteristics:**
- **Immutable**: Cannot be modified after creation
- **Memory managed**: Automatic memory management with reference counting
- **Template-based**: Works with different character types
- **Cross-platform**: Handles platform differences (UTF-16 on Windows, UTF-32 on other platforms for wchar_t)

### Specific String Type Aliases

The project provides convenient aliases instead of using `ObjectString<T>` directly:

- **`WString`**: Wide character string (`ObjectString<wchar_t>`) - **preferred for general use**
  - UTF-16 on Windows, UTF-32 on other platforms
- **`AString`**: ASCII string (`ObjectString<char>`) - for ASCII operations
- **`U8String`**: UTF-8 string (`ObjectString<char8_t>`) - for UTF-8 handling
- **`U16String`**: UTF-16 string (`ObjectString<char16_t>`) - for explicit UTF-16
- **`U32String`**: UTF-32 string (`ObjectString<char32_t>`) - for explicit UTF-32

**Always use aliases instead of `ObjectString<T>` directly.**

## String Initialization

### Static Initialization Functions

Use these static functions to create string instances:

#### Unmanaged(L"string-literal")
```cpp
auto str = WString::Unmanaged(L"Hello World");
```
- **Use case**: String literals only
- **Performance**: Zero-copy for string literals
- **Safety**: Only works with compile-time string literals

#### CopyFrom(wchar_t*, vint)
```cpp
wchar_t* buffer = GetSomeBuffer();
vint length = GetBufferLength();
auto str = WString::CopyFrom(buffer, length);
```
- **Use case**: Copy from existing buffer when you have the length
- **Memory**: Creates a copy of the data
- **Safety**: Safe - creates independent copy

#### Constructor from wchar_t*
```cpp
wchar_t* buffer = GetSomeBuffer();
WString str(buffer);  // Automatically determines length and copies
```
- **Use case**: Copy from null-terminated buffer when length is unknown
- **Memory**: Creates a copy of the data

#### TakeOver(wchar_t*, vint)
```cpp
wchar_t* buffer = new wchar_t[100];
// ... fill buffer ...
auto str = WString::TakeOver(buffer, actualLength);
// buffer is now owned by str, will be deleted automatically
```
- **Use case**: Take ownership of a dynamically allocated buffer
- **Memory**: Takes ownership, `delete[]` called automatically
- **Warning**: Buffer must be allocated with `new[]`

## String to Number Conversion

### String to Integer
- **`wtoi(WString)`**: Convert to `vint` (signed integer)
- **`wtoi64(WString)`**: Convert to `vint64_t` (64-bit signed integer)
- **`wtou(WString)`**: Convert to `vuint` (unsigned integer)
- **`wtou64(WString)`**: Convert to `vuint64_t` (64-bit unsigned integer)

### String to Floating Point
- **`wtof(WString)`**: Convert to `double`

### Examples
```cpp
WString numberStr = L"12345";
vint value = wtoi(numberStr);
double floatValue = wtof(L"123.45");
```

## Number to String Conversion

### Integer to String
- **`itow(vint)`**: Convert signed integer to `WString`
- **`i64tow(vint64_t)`**: Convert 64-bit signed integer to `WString`
- **`utow(vuint)`**: Convert unsigned integer to `WString`
- **`u64tow(vuint64_t)`**: Convert 64-bit unsigned integer to `WString`

### Floating Point to String
- **`ftow(double)`**: Convert double to `WString`

### Examples
```cpp
vint number = 12345;
WString str = itow(number);  // "12345"
double value = 123.45;
WString floatStr = ftow(value);  // "123.45"
```

## Case Conversion

### Case Transformation Functions
- **`wupper(WString)`**: Convert all letters to uppercase
- **`wlower(WString)`**: Convert all letters to lowercase

### Examples
```cpp
WString original = L"Hello World";
WString upper = wupper(original);  // "HELLO WORLD"
WString lower = wlower(original);  // "hello world"
```

## UTF Encoding Conversion

### Template-Based Conversion

#### ConvertUtfString<From, To>
```cpp
WString wideStr = L"Hello ??";
U8String utf8Str = ConvertUtfString<wchar_t, char8_t>(wideStr);
U16String utf16Str = ConvertUtfString<wchar_t, char16_t>(wideStr);
```
- **Use case**: Template-based conversion when you don't know exact types
- **Template parameters**: `From` and `To` are char types (`wchar_t`, `char8_t`, `char16_t`, `char32_t`)

### Direct Conversion Functions (AtoB pattern)

Where `A` and `B` can be:
- **`w`**: `WString` (wide string)
- **`u8`**: `U8String` (UTF-8)
- **`u16`**: `U16String` (UTF-16)
- **`u32`**: `U32String` (UTF-32)
- **`a`**: `AString` (ASCII)

#### Common Conversion Functions
```cpp
// Wide string conversions
U8String utf8 = wtou8(wideStr);     // WString to UTF-8
U16String utf16 = wtou16(wideStr);  // WString to UTF-16
U32String utf32 = wtou32(wideStr);  // WString to UTF-32
AString ascii = wtoa(wideStr);      // WString to ASCII

// Back to wide string
WString fromUtf8 = u8tow(utf8);     // UTF-8 to WString
WString fromUtf16 = u16tow(utf16);  // UTF-16 to WString
WString fromUtf32 = u32tow(utf32);  // UTF-32 to WString
WString fromAscii = atow(ascii);    // ASCII to WString

// Between UTF encodings
U16String u8to16 = u8tou16(utf8);   // UTF-8 to UTF-16
U8String u16to8 = u16tou8(utf16);   // UTF-16 to UTF-8
```

## Best Practices

### Character Type Preferences
1. **Always prefer `wchar_t` and `WString`** for general text processing
2. **Use `char8_t` and `U8String`** for UTF-8 when specifically needed
3. **Avoid `char` and `std::string`** - use project's string types instead

### Initialization Best Practices
1. **Use `WString::Unmanaged(L"...")`** for string literals
2. **Use constructor or `CopyFrom`** when you need to copy external data
3. **Use `TakeOver`** only when you want to transfer ownership of allocated memory

### Conversion Best Practices
1. **Use direct `AtoB` functions** when you know the specific encodings
2. **Use `ConvertUtfString<From, To>`** in template code or when types are variable
3. **Cache conversion results** when the same conversion is used multiple times

### Memory Management
1. **Strings are immutable** - operations create new string instances
2. **Automatic memory management** - no need to manually free string memory
3. **Reference counting** - efficient copying and assignment

## Extra Content

### Platform Differences
The `wchar_t` type behaves differently across platforms:
- **Windows**: `wchar_t` is 16-bit (UTF-16)
- **Linux/Unix**: `wchar_t` is 32-bit (UTF-32)

The string conversion system automatically handles these differences internally.

### Performance Considerations
- **String literals with `Unmanaged`**: Zero-copy initialization
- **Immutable strings**: Thread-safe sharing but creates new instances for modifications
- **UTF conversions**: May involve memory allocation and encoding conversion overhead
- **Case conversions**: Create new string instances rather than modifying in-place

### Thread Safety
All string types are immutable and thread-safe for reading. Multiple threads can safely access the same string instance simultaneously. However, string creation and conversion operations may allocate memory and should be considered for performance in highly concurrent scenarios.