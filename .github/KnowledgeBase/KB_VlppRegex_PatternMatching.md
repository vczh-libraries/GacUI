# Pattern Matching Operations

Text pattern matching and searching operations with support for different UTF encodings.

The definition and the string to match could be in different UTF encoding.
`Regex_<T>` accepts `ObjectString<T>` as the definition.
`MatchHead<U>`, `Match<U>`, `TestHead<U>`, `Test<U>`, `Search<U>`, `Split<U>` and `Cut<U>` accepts `ObjectString<U>` to match with the regular expression.

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

This method returns a collection of all non-overlapping matches found in the input string. When multiple matches are possible at the same position, it will choose one and continue searching from after that match.

### Split<U>

`Split` use the regular expression as a splitter, finding all remaining substrings.

This method treats the pattern as a delimiter and splits the input string wherever the pattern matches, returning the parts between the matches. This is similar to string split operations but with the power of regular expressions.

### Cut<U>

`Cut` combines both `Search` and `Split`, finding all substrings in order, regardless if one matches or not.

This method returns all parts of the string in sequence, both the parts that match the pattern and the parts that don't match. This gives you a complete decomposition of the input string.

## Extra Content

### UTF Encoding Support

One of the key features of VlppRegex is its support for different UTF encodings between the pattern definition and the input text:

- The regex pattern is defined using `Regex_<T>` where `T` is the character type for the pattern
- The input text uses type `U` in the matching methods  
- This allows patterns defined in one encoding to match text in another encoding
- Supported character types include `wchar_t`, `char8_t`, `char16_t`, `char32_t`

### Performance Considerations

The VlppRegex engine has specific performance characteristics:

- **DFA Compatible vs Incompatible**: Features that break DFA compatibility (like backreferences) significantly impact performance
- **Escaping Optimization**: Using `/` instead of `\` for escaping can improve readability in C++ code
- **Method Selection**: Choose simpler methods like `Test` or `TestHead` when you only need boolean results

### Syntax Differences from .NET

While mostly compatible with .NET regex syntax, VlppRegex has important differences:

- **Dot Character**: `.` matches literal '.' character, while `/.` or `\.` matches all characters
- **Escaping**: Both `/` and `\` perform escaping (prefer `/` for C++ compatibility)
- **Character Classes**: Standard character classes work the same as .NET
- **Quantifiers**: Standard quantifiers (`*`, `+`, `?`, `{n,m}`) work as expected

### Error Handling

When using regex operations:

- Invalid patterns will cause compilation errors when constructing `Regex_<T>`
- Invalid input strings generally won't cause errors but may produce no matches
- Method calls on empty or invalid regex objects may result in undefined behavior

### Common Usage Patterns

**Simple validation**: (note that `^` is not required and `$` does not make sense here)
```cpp
Regex regex(L"[0-9]+");
bool isNumber = regex.TestHead(input);
```

**Extracting all matches**:
```cpp
Regex regex(L"\\w+");
auto matches = regex.Search(text);
for (auto match : matches) {
    // Process each word
}
```

**Splitting text**:
```cpp
Regex regex(L"[,;]");
auto parts = regex.Split(csvLine);
```

**Complete decomposition**:
```cpp
Regex regex(L"\\d+");
auto parts = regex.Cut(mixedText);  // Returns both numbers and non-numbers
```