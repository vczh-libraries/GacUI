# Pattern Matching Operations

Text pattern matching and searching operations with support for different UTF encodings.

The definition and the string to match can use different UTF encodings.
`Regex_<T>` accepts `ObjectString<T>` as the definition.
`MatchHead<U>`, `Match<U>`, `TestHead<U>`, `Test<U>`, `Search<U>`, `Split<U>` and `Cut<U>` accept `ObjectString<U>` or `const U*` text to match with the regular expression.
`Search<U>`, `Split<U>` and `Cut<U>` append results to a caller-provided `RegexMatch_<U>::List&`; they do not return a collection.
`Split<U>` and `Cut<U>` also take `keepEmptyMatch` to decide whether empty unmatched fragments are included.

## Core Pattern Matching Methods

### MatchHead<U>

`MatchHead` finds the longest prefix of the string which matches the regular expression.

This method attempts to match the pattern starting from the beginning of the input string and returns the longest possible match found at the start. It will return detailed match information including captured groups if the pattern matches.

### Match<U>

`Match` finds the earliest substring which matches the regular expression.

This method searches through the entire input string to find the first occurrence of a substring that matches the pattern. Unlike `MatchHead`, it doesn't require the match to be at the beginning of the string.

### TestHead<U>

`TestHead` performs a similar action to `MatchHead`, but it only returns `bool` without detailed information.

This is an optimization when you only need to know whether the beginning of the string matches the pattern, without requiring access to the actual match data or captured groups.

### Test<U>

`Test` performs a similar action to `Match`, but it only returns `bool` without detailed information.

This is an optimization when you only need to know whether the string contains a substring that matches the pattern, without requiring access to the actual match data or captured groups.

## Advanced Pattern Operations

### Search<U>

`Search` finds all substrings which match the regular expression. All results do not overlap with each other.

This method appends all non-overlapping successful matches found in the input string to `RegexMatch_<U>::List&`. When a match is found, searching continues after that match.

### Split<U>

`Split` uses the regular expression as a splitter, finding all remaining substrings.

This method treats the pattern as a delimiter and appends the parts between successful matches. The appended `RegexMatch_<U>` objects have `Success()` equal to `false`. `keepEmptyMatch` controls whether empty unmatched fragments are appended.

### Cut<U>

`Cut` combines both `Search` and `Split`, finding all substrings in order, regardless if one matches or not.

This method appends all parts of the string in sequence, both the parts that match the pattern and the parts that do not match. `Success()` distinguishes successful pattern matches from unmatched fragments, and `keepEmptyMatch` controls empty unmatched fragments.

## Extra Content

### UTF Encoding Support

One of the key features of VlppRegex is its support for different UTF encodings between the pattern definition and the input text:

- The regex pattern is defined using `Regex_<T>` where `T` is the character type for the pattern
- The input text uses type `U` in the matching methods, through `ObjectString<U>` or `const U*`
- This allows patterns defined in one encoding to match text in another encoding
- Supported character types include `wchar_t`, `char8_t`, `char16_t`, `char32_t`

### Performance Considerations

The VlppRegex engine has specific performance characteristics:

- **DFA Compatible vs Incompatible**: Features that break DFA compatibility, including captures, backreferences, lookahead, and lazy loops, require rich mode and significantly impact performance
- **Escaping Optimization**: Using `/` instead of `\` for escaping can improve readability in C++ code
- **Method Selection**: Choose simpler methods like `Test` or `TestHead` when you only need boolean results
- **Mode Inspection**: Use `IsPureMatch()` and `IsPureTest()` to check whether DFA mode is used for matching and testing

### Syntax Differences from .NET

While mostly compatible with .NET regex syntax, VlppRegex has important differences:

- **Dot Character**: `.` matches literal '.' character, while `/.` or `\.` matches any character
- **Escaping**: Both `/` and `\` perform escaping (prefer `/` for C++ compatibility)
- **Character Classes**: `\s`, `\S`, `\d`, `\D`, `\l`, `\L`, `\w` and `\W` are supported, and each one can also be written with `/`
- **Quantifiers**: Standard quantifiers (`*`, `+`, `?`, `{n,m}`) work as expected

### Error Handling

When using regex operations:

- Invalid patterns trigger `CHECK_ERROR` during `Regex_<T>` construction
- `MatchHead<U>` and `Match<U>` return `nullptr` when no match is found
- `TestHead<U>` and `Test<U>` return `false` when no match is found
- `Search<U>` appends no items when no successful match is found

### Common Usage Patterns

**Prefix test**:
```cpp
Regex regex(L"/d+");
bool hasNumberPrefix = regex.TestHead(input);
```

**Extracting all matches**:
```cpp
Regex regex(L"/w+");
RegexMatch::List matches;
regex.Search(text, matches);
for (auto match : matches) {
    // Process each word
}
```

**Splitting text**:
```cpp
Regex regex(L"[,;]");
RegexMatch::List parts;
regex.Split(csvLine, false, parts);
```

**Complete decomposition**:
```cpp
Regex regex(L"/d+");
RegexMatch::List parts;
regex.Cut(mixedText, false, parts);  // Appends both numbers and non-numbers
```
