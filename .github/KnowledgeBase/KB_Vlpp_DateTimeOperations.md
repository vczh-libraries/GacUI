# Date and Time Operations

Cross-platform date and time handling with timezone conversions and arithmetic operations.

## Overview

The `DateTime` class provides cross-platform date and time functionality with support for timezone conversions, arithmetic operations, and customizable implementations for testing scenarios.

## Current Time Operations

Retrieve current system time in different timezone contexts.

- Use `DateTime::LocalTime()` to get current time in local timezone
- Use `DateTime::UtcTime()` to get current time in UTC timezone

Both static methods return a `DateTime` instance representing the current moment, but interpreted in different timezone contexts. The local time reflects the system's configured timezone, while UTC time provides a standardized reference point.

## DateTime Construction

Create specific date and time instances programmatically.

- Use `DateTime::FromDateTime(year, month, day, hour, minute, second, milliseconds)` for precise date/time creation
- Use `DateTime::FromOSInternal(osInternal)` to create from OS-specific internal representation

The `FromDateTime` method allows you to specify all components of a date and time, from year down to milliseconds. All parameters are validated according to calendar rules.

The `FromOSInternal` method is typically used internally but can be useful when working with OS-specific time representations.

## Timezone Conversions

Convert between local and UTC time representations.

- Use `ToLocalTime()` to convert from UTC to local timezone
- Use `ToUtcTime()` to convert from local to UTC timezone

These instance methods create new `DateTime` objects with the time converted to the target timezone. The conversion takes into account the system's timezone configuration and daylight saving time rules.

## Time Arithmetic

Perform calculations with date and time values.

- Use `Forward(milliseconds)` to add time to a DateTime instance
- Use `Backward(milliseconds)` to subtract time from a DateTime instance

Both methods return new `DateTime` instances with the specified amount of time added or subtracted. The parameter is in milliseconds, allowing for precise time calculations.

## Implementation Injection

Override the default DateTime implementation for testing and customization.

- Use `InjectDateTimeImpl(impl)` to set a custom `IDateTimeImpl` implementation
- Use `EjectDateTimeImpl(impl)` to remove a specific injected implementation from the injection chain
- Use `EjectDateTimeImpl(nullptr)` to remove all injected implementations and restore the default OS-specific implementation

This functionality allows you to provide predictable time values for testing or implement custom time behaviors. The injected implementations form a chain where each implementation can delegate to the previous one in the chain.

When using injection for testing, always ensure proper cleanup by calling `EjectDateTimeImpl(nullptr)` to avoid affecting other test cases:

```cpp
// Test example showing proper injection/ejection pattern
MockDateTimeImpl mockImpl;

// Inject mock implementation
InjectDateTimeImpl(&mockImpl);

// Store results before ejection to ensure cleanup happens even on test failure
DateTime localResult = DateTime::LocalTime();

// Always eject to ensure no side effects on other tests
EjectDateTimeImpl(nullptr);

// Verify results after cleanup
TEST_ASSERT(localResult.year == 2000);
```

## Extra Content

### Platform Abstraction
The `DateTime` class abstracts over platform-specific time APIs, providing consistent behavior across Windows and Linux. The underlying implementation handles differences in time representation and timezone handling between operating systems.

### Precision and Range
DateTime operations maintain millisecond precision and can represent dates across a wide range suitable for most application needs. The exact range depends on the underlying platform implementation.

### Thread Safety
`DateTime` instances are immutable value types and can be safely used across multiple threads. The implementation injection functionality should typically be used during application startup before multi-threaded usage begins.

### Integration with Other Components
DateTime integrates with other parts of the framework, particularly the Locale system for culture-specific date formatting and the unit testing framework for time-dependent test scenarios.