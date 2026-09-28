# Console Operations

## Overview

Vlpp provides basic console input/output operations for command-line interface applications. These functions provide simple and cross-platform ways to output text to the console, essential for CLI tools, debugging output, and simple user interaction in console applications.

## Basic Output Functions

### Console::Write Function

The primary function for outputting text to the console without a line break.

```cpp
Console::Write(L"Hello ");
Console::Write(L"World");
// Output: Hello World (on same line)
```

**Function usage:**
```cpp
void Console::Write(const WString& text);
```

**Key characteristics:**
- **No automatic line break**: Text is written directly to the console without adding a newline character
- **Wide string support**: Accepts `WString` parameters, supporting Unicode text output
- **Cross-platform**: Works consistently across Windows and Linux platforms
- **Immediate output**: Text appears immediately in the console

### Console::WriteLine Function

The primary function for outputting text to the console with an automatic line break.

```cpp
Console::WriteLine(L"First line");
Console::WriteLine(L"Second line");
// Output:
// First line
// Second line
```

**Function usage:**
```cpp
void Console::WriteLine(const WString& text);
```

**Key characteristics:**
- **Automatic line break**: Adds a newline character after the text
- **Wide string support**: Accepts `WString` parameters for Unicode text
- **Cross-platform compatibility**: Consistent behavior across operating systems
- **Sequential output**: Each call produces a new line of output

## Basic Input Functions

### Console::TryRead Function

Use `Console::TryRead` for line input from either an attached console or redirected stdin. It returns `Nullable<WString>`, with null meaning no line is available, such as EOF or an unavailable input handle.

```cpp
auto line = Console::TryRead();
if (line)
{
    Console::WriteLine(L"Input: " + line.Value());
}
```

**Function usage:**
```cpp
Nullable<WString> Console::TryRead();
```

**Key characteristics:**
- **Redirection aware**: Handles both interactive console input and stdin redirection
- **EOF aware**: Returns null instead of forcing callers to treat EOF as an empty line
- **Wide string result**: Produces `WString` so it composes with console output APIs
- **Console::Read compatibility**: `Console::Read` calls `TryRead` and returns `WString::Empty` when `TryRead` returns null

Under `VCZH_WASM`, `Source/Console.Wasm.cpp` calls the worker's synchronous `globalThis.vlConsoleRead` callback. Install it before initializing the module. Return `undefined` for no input, or a JavaScript string for a present line; `""` is a present empty line. The bridge converts UTF-16 to `WString`, preserving Unicode and embedded zero code units, and reports callback exceptions or unsupported return types as C++ `Error` values. The unit-test HTML in Tools installs a callback that always returns `undefined`.

## Usage Patterns

### Simple Text Output

For basic text output to the console:

```cpp
Console::WriteLine(L"Application starting...");
Console::WriteLine(L"Processing data...");
Console::WriteLine(L"Operation completed.");
```

### Mixed Output Formatting

Combining `Write` and `WriteLine` for formatted output:

```cpp
Console::Write(L"Progress: ");
Console::Write(L"50");
Console::WriteLine(L"%");
// Output: Progress: 50%
```

### String Concatenation

Using string operations with console output:

```cpp
WString userName = L"John";
Console::WriteLine(L"Hello, " + userName + L"!");
// Output: Hello, John!
```

### Numeric Output

Converting numbers to strings for console display:

```cpp
vint count = 42;
Console::WriteLine(L"Total items: " + itow(count));
// Output: Total items: 42

double value = 3.14159;
Console::WriteLine(L"Pi value: " + ftow(value));
// Output: Pi value: 3.14159
```

## Integration with Application Types

### Console Applications

Use console operations in main functions for CLI applications:

```cpp
int main(int argc, char** argv)
{
    Console::WriteLine(L"My Console Application");
    Console::WriteLine(L"Version 1.0");
    
    // Application logic here
    
    Console::WriteLine(L"Application exiting...");
    return 0;
}
```

### Debug Output

Console operations are useful for debugging and diagnostic output:

```cpp
void ProcessData(const List<vint>& data)
{
    Console::WriteLine(L"Processing " + itow(data.Count()) + L" items");
    
    for (vint i = 0; i < data.Count(); i++)
    {
        Console::Write(L"Item " + itow(i) + L": ");
        Console::WriteLine(itow(data[i]));
    }
}
```

### Error Reporting

Simple error message display:

```cpp
try
{
    // Some operation that might fail
}
catch (const Exception& ex)
{
    Console::WriteLine(L"Error: Operation failed");
    Console::WriteLine(L"Please check your input and try again");
}
```

## Best Practices

### Use WString Consistently

Always use `WString` with console operations for proper Unicode support:

```cpp
// Good: Using WString
Console::WriteLine(L"Unicode text: 你好世界");

// Avoid: Don't mix with narrow strings
// Console::WriteLine("Narrow string");  // Wrong - type mismatch
```

### Format Strings Properly

Build formatted output using string operations and conversion functions:

```cpp
vint fileSize = 1024;
WString fileName = L"document.txt";

Console::WriteLine(L"File: " + fileName);
Console::WriteLine(L"Size: " + itow(fileSize) + L" bytes");
```

### Handle Long Output

Break long text into manageable lines:

```cpp
WString longMessage = L"This is a very long message that should be broken into multiple lines for better readability";

Console::WriteLine(L"Status Report:");
Console::WriteLine(L"  " + longMessage.Left(40));
Console::WriteLine(L"  " + longMessage.Right(longMessage.Length() - 40));
```

## Cross-Platform Considerations

### Character Encoding

Console operations handle character encoding appropriately across platforms:
- **Windows**: UTF-16 wide character support
- **Linux**: UTF-32 wide character support with proper conversion
- **Automatic handling**: The framework manages encoding differences transparently

### Line Endings

`Console::WriteLine` writes CRLF (`\r\n`) on both Windows and Linux, as implemented in `Source/Console.cpp`.

### Console Availability

Console operations assume a console window is available:
- **Console applications**: Always have console access
- **GUI applications**: May not have console access on some platforms
- **Service applications**: May redirect console output to logs
- **Redirected input**: Use `Console::TryRead` when stdin may be redirected or may reach EOF

## Extra Content

### Performance Considerations

Console I/O is generally slower than file I/O, so for large amounts of output consider:
- Buffering text before output
- Using file operations for extensive logging
- Limiting console output in performance-critical applications

### Alternative Output Methods

For different output needs, consider:
- File operations from VlppOS for persistent output
- Stream operations for more complex I/O scenarios
- Logging frameworks for structured application logging

### Integration with Unit Testing

Console operations can be useful in unit tests for diagnostic output, but we favor TEST_PRINT.

```cpp
TEST_CASE(L"MyTestCase")
{
    Console::WriteLine(L"Starting test: MyTestCase");
    
    // Test logic here
    TEST_ASSERT(condition);
    
    TEST_PRINT(L"Test completed successfully");
});
```

### Error Handling

Console I/O raises `Error` when disabled through `Console::Disable()` (`Source/Console.Windows.cpp` and `Source/Console.Linux.cpp`). When console access is unavailable, output may silently fail. For critical output, consider combining with file logging:

```cpp
void LogMessage(const WString& message)
{
    // Output to console for immediate feedback
    Console::WriteLine(message);
    
    // Also log to file for persistence
    // (using VlppOS file operations)
}
```

### Internationalization

Since console operations use `WString`, they support international characters natively:

```cpp
Console::WriteLine(L"English: Hello World");
Console::WriteLine(L"Chinese: 你好世界");
Console::WriteLine(L"Japanese: こんにちは世界");
Console::WriteLine(L"Arabic: مرحبا بالعالم");
```

The actual display depends on the console's font and locale settings, but the framework properly handles the character encoding.
