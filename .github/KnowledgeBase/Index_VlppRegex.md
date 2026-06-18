# VlppRegex Knowledge Base

Project introduction remains in [Index.md](./Index.md#vlppregex).

### Choosing APIs

#### Pattern Matching Operations

Text pattern matching and searching operations with support for different UTF encodings.

- Use `Regex_<T>` for pattern definition with `ObjectString<T>` encoding
- Use `MatchHead<U>` for finding longest prefix matching the pattern
- Use `Match<U>` for finding earliest substring matching the pattern
- Use `TestHead<U>` for boolean prefix matching without detailed results
- Use `Test<U>` for boolean substring matching without detailed results
- Use `Search<U>` for finding all non-overlapping matches
- Use `Split<U>` for using pattern as delimiter to split text
- Use `Cut<U>` for combined search and split operations

[API Explanation](./KB_VlppRegex_PatternMatching.md)

#### Type Aliases

Convenient type aliases for common character encodings to simplify regex usage.

- Use `RegexString` instead of `RegexString_<wchar_t>`
- Use `RegexMatch` instead of `RegexMatch_<wchar_t>`
- Use `Regex` instead of `Regex_<wchar_t>`
- Use `RegexToken` instead of `RegexToken_<wchar_t>`
- Use `RegexProc` instead of `RegexProc_<wchar_t>`
- Use `RegexTokens` instead of `RegexTokens_<wchar_t>`
- Use `RegexLexerWalker` instead of `RegexLexerWalker_<wchar_t>`
- Use `RegexLexerColorizer` instead of `RegexLexerColorizer_<wchar_t>`
- Use `RegexLexer` instead of `RegexLexer_<wchar_t>`

[API Explanation](./KB_VlppRegex_TypeAliases.md)

### Design Explanation
