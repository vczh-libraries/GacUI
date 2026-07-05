# VlppRegex Knowledge Base

Project introduction remains in [Index.md](./Index.md#vlppregex).

### Choosing APIs

#### Pattern Matching Operations

Text pattern matching and searching operations with support for different UTF encodings between pattern definitions and input text.

- Use `Regex_<T>` for pattern definition with `ObjectString<T>` encoding
- Use `MatchHead<U>` for finding longest prefix matching the pattern
- Use `Match<U>` for finding earliest substring matching the pattern
- Use `TestHead<U>` for boolean prefix matching without detailed results
- Use `Test<U>` for boolean substring matching without detailed results
- Use `Search<U>` for appending all non-overlapping successful matches to `RegexMatch_<U>::List`
- Use `Split<U>` for appending delimiter-separated unmatched fragments to `RegexMatch_<U>::List`
- Use `Cut<U>` for appending both successful and failed fragments in order

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
