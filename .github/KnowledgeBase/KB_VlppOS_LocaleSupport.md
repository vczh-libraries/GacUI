# Locale Support

Cross-platform localization and globalization with culture-aware string operations and formatting.

## Getting Locale Instances

`Locale` is a value type that provides access to culture-specific operations. You can obtain locale instances using these static methods:

- Use `Locale::Invariant()` or `INVLOC` macro for culture-invariant operations
- Use `Locale::SystemDefault()` for OS code page interpretation
- Use `Locale::UserDefault()` for user language and location settings
- Use `Locale::Enumerate(locales)` to get all supported locales

The `Invariant()` locale does not change across OS settings, making it suitable for consistent operations that should not vary by user preferences. Use the `INVLOC` macro as a shorter version instead of writing `vl::Locale::Invariant()`.

The `SystemDefault()` locale returns the locale for OS code page interpretation and is not needed in most cases.

The `UserDefault()` locale returns the locale for user language and location settings, which is typically what you want for user-facing operations.

The `Enumerate(locales)` method populates the argument with all locales supported by the running OS.

## Date and Time Formatting

Once you have a `Locale` value, you can perform locale-aware date and time operations:

- Use `Get*Formats` methods for date-time format enumeration
- Use `FormatDate` and `FormatTime` for locale-aware date/time formatting
- Use `Get*Name` methods for localized week day and month names

When using `Get*Formats` methods, take the first filled format as the default one. The `FormatDate` and `FormatTime` methods convert `DateTime` objects to `WString` representations according to the locale's conventions.

The `Get*Name` methods provide locale-aware week day or month names, which are useful for creating localized calendar interfaces or date displays.

## Number and Currency Formatting

For numeric data formatting:

- Use `FormatNumber` and `FormatCurrency` for locale-aware number formatting

Both methods take a `WString` storing a number as input. You can use `itow` and `ftow` functions to convert integers and floating-point numbers to strings before formatting them according to locale conventions.

## Locale-Aware String Operations

The locale system provides comprehensive string manipulation functions with cultural awareness:

- Use `Compare`, `CompareOrdinal`, `CompareOrdinalIgnoreCase` for locale-aware string comparison
- Use `FindFirst`, `FindLast` for normalized string searching
- Use `StartsWith`, `EndsWith` for normalized string matching

The comparison functions (`Compare`, `CompareOrdinal`, and `CompareOrdinalIgnoreCase`) compare two strings using different approaches based on locale-specific rules.

The search functions (`FindFirst` and `FindLast`) find one string within another. Since strings are normalized before searching, these functions return a pair of numbers indicating the matched substring. The matched substring might not be identical to the substring being searched for, but they are equivalent under the given locale's normalization rules.

The `StartsWith` and `EndsWith` functions test if a substring appears at the expected location, using locale-aware normalization.

All these string manipulation functions internally rewrite the input strings using specified normalization rules before performing their operations, ensuring culturally appropriate behavior.

## Extra Content

### Implementation Injection
You can replace the default locale implementation with a custom one for testing and specialized scenarios:

- Use `InjectLocaleImpl(impl)` to set a custom `ILocaleImpl` implementation
- Use `EjectLocaleImpl(impl)` to remove a specific injected implementation
- Use `EjectLocaleImpl(nullptr)` to reset to the default OS-specific implementation by ejecting all injected implementations
- Use `GetOSLocaleImpl()` to get the OS-dependent default implementation (function not in header file, declare manually)

The injected implementation affects all `Locale` class operations including formatting, string comparison, and cultural operations. This allows you to provide custom localization behavior, create mock locales for testing, or implement specialized cultural rules not supported by the default OS implementation.

Implementation injection should typically be done during application startup before any multi-threaded usage begins, as it affects global state.

### Platform-Independent Fallback Implementation

The `EnUsLocaleImpl` class provides a platform-independent implementation that only supports en-US locale operations. This implementation serves as a final fallback when no OS-specific locale support is available:

- `EnUsLocaleImpl` provides basic English (US) locale functionality
- It implements all `ILocaleImpl` interface methods with en-US specific behavior
- Use this implementation when you need consistent en-US behavior across all platforms (e.g. unit test)
- This is the default implementation for non-Windows platform

### Character Normalization
The locale system performs character normalization as part of its string operations. This means that characters that appear different but have the same semantic meaning (such as composed vs. decomposed Unicode characters) are treated as equivalent during comparisons and searches.

### Performance Considerations
Locale-aware operations are generally more expensive than simple string operations because they involve character normalization and culture-specific rules. For performance-critical code that doesn't need localization, consider using the invariant locale or simple string operations.

### Threading Safety
Locale objects are value types and can be safely used across multiple threads. However, be aware that system locale settings can change at runtime, so cache locale instances appropriately for your use case. The implementation injection functionality should be used with care in multi-threaded environments.

### Best Practices
- Use `UserDefault()` for user-facing string operations
- Use `Invariant()` for internal data processing and storage
- Cache frequently used locale instances rather than creating them repeatedly
- Be aware that locale-aware operations can be slower than simple string comparisons
- When using injection, ensure proper cleanup between tests if different implementations are needed