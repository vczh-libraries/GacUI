# Type Aliases

Convenient type aliases for common character encodings to simplify regex usage.

VlppRegex provides convenient type aliases that eliminate the need to specify template parameters when working with the most common character encoding - `wchar_t`. These aliases make the code more readable and easier to work with.

## Core Type Aliases

### RegexString

`RegexString` -> `RegexString_<wchar_t>`

This alias provides a convenient way to work with regex strings without specifying the character type explicitly when using wide characters.

### RegexMatch  

`RegexMatch` -> `RegexMatch_<wchar_t>`

This alias represents match results for wide character regex operations, containing information about successful matches including position, length, and captured groups.

### Regex

`Regex` -> `Regex_<wchar_t>`

This is the main regex class alias for wide character patterns. This is the most commonly used alias when working with `WString` patterns and input text.

## Extended Type Aliases

### RegexToken

`RegexToken` -> `RegexToken_<wchar_t>`

This alias is used for tokenization operations where the regex engine breaks input into discrete tokens based on patterns.

### RegexProc

`RegexProc` -> `RegexProc_<wchar_t>`

This alias represents callback procedures used in advanced regex processing scenarios.

### RegexTokens

`RegexTokens` -> `RegexTokens_<wchar_t>`

This alias represents collections of tokens produced by regex tokenization operations.

## Lexer-Related Aliases

### RegexLexerWalker

`RegexLexerWalker` -> `RegexLexerWalker_<wchar_t>`

This alias is used for advanced lexical analysis where you need to walk through and analyze tokenized input systematically.

### RegexLexerColorizer

`RegexLexerColorizer` -> `RegexLexerColorizer_<wchar_t>`

This alias is used for syntax highlighting and text colorization based on regex pattern matching, commonly used in text editors and IDEs.

### RegexLexer

`RegexLexer` -> `RegexLexer_<wchar_t>`

This alias represents the main lexical analyzer that combines multiple regex patterns for comprehensive text analysis and tokenization.

## Extra Content

### When to Use Aliases vs Templates

**Use aliases when**:
- Working with `WString` and wide character text (most common scenario)
- Writing application code that doesn't need encoding flexibility
- Simplifying code readability
- Following the project's preference for `wchar_t`

**Use templates when**:
- Working with mixed character encodings
- Building reusable library components
- Needing explicit control over character types
- Interfacing with systems using different encodings

### Character Encoding Considerations

These aliases all resolve to `wchar_t`, which has platform-specific behavior:

- **Windows**: `wchar_t` is UTF-16 (16-bit)
- **Other platforms**: `wchar_t` is UTF-32 (32-bit)

This means the same code using these aliases will handle different Unicode encodings transparently across platforms.

### Integration with VlppOS String Types

These aliases work seamlessly with the project's string types:

- `RegexString` works directly with `WString`
- Pattern matching operations accept `WString` input
- Results integrate with `WString` manipulation functions
- Conversion functions like `wtou8`, `wtou16` can be used with results

### Performance Characteristics

Using aliases vs explicit templates has no performance impact:

- Aliases are resolved at compile time
- No runtime overhead compared to template instantiation
- Same optimization opportunities
- Binary size remains identical

### Common Usage Examples

**Basic pattern matching**:
```cpp
Regex pattern(L"\\d+");
auto match = pattern.Match(text);
```

**Lexical analysis**:
```cpp
List<WString> tokenDefs;
tokenDefs.Add(L"\\b(if|else|while)\\b");  // keywords
tokenDefs.Add(L"\\b[a-zA-Z_][a-zA-Z0-9_]*\\b");  // identifiers
tokenDefs.Add(L"\\d+");  // numbers

RegexLexer lexer(tokenDefs);
auto tokens = lexer.Parse(sourceCode);
```

**Syntax highlighting**:
```cpp
List<WString> tokenDefs;
tokenDefs.Add(L"\\bclass\\b");  // token 0: keywords
tokenDefs.Add(L"\"[^\"]*\"");   // token 1: strings

RegexProc proc;
proc.colorizeProc = [](void* argument, vint start, vint length, vint token)
{
    // Apply colors based on token type
    if (token == 0) ApplyKeywordColor(start, length);
    if (token == 1) ApplyStringColor(start, length);
};

RegexLexer lexer(tokenDefs);
RegexLexerColorizer colorizer = lexer.Colorize(proc);
```

### Migration Considerations

When migrating code:

- Replace `RegexString_<wchar_t>` with `RegexString`
- Replace `Regex_<wchar_t>` with `Regex`
- Other template instantiations can be simplified similarly
- No functional changes required
- Code becomes more readable and maintainable

### Template Specialization Background

These aliases exist because VlppRegex is built on a template system:

- Base templates like `Regex_<T>` support any character type
- Most applications only need wide character support
- Aliases eliminate template parameter repetition
- Consistent with the project's preference for `wchar_t` over other character types