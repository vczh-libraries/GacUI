# Lexer Definition

An lexer definition consists of multiple tokens. Names of non-discarded tokens can be used in syntax definitions.

Single line comments are allowed using**//**.

A token can be defined by:
```Lexer
TokenName           :REGEX
```
Spaces are allowed before**":"**but not after, everything after**":"**in this line is considered the regular expression to define the token.

A discarded token can be defined by:
```Lexer
discard TokenName   :REGEX
```
Such token will be recognized and removed from the input. You can use it for comments or whitespaces.

A token fragment can be defined by:
```Lexer
$FragmentName       :REGEX
```
This is not a token, but it can be used in other regular expressions using**{$FragmentName}**, to paste the fragment at that position and avoid repeating the same pattern everywhere. Here is an example of defining integer, hex and binary literal in C++:
```Lexer
$CPP_NUMBER_POSTFIX  :[uU]|[lL]|[uU][lL]|[lL][uU]|[lL][lL]|[uU][lL][lL]|[lL][uU][lL]|[lL][lL][uU]|i8|ui8|i16|ui16|i32|ui32|i64|ui64
INT                  :(/d+('/d+)*)({$CPP_NUMBER_POSTFIX})?
HEX                  :0[xX][0-9a-fA-F]+({$CPP_NUMBER_POSTFIX})?
BIN                  :0[bB][01]+({$CPP_NUMBER_POSTFIX})?
```
Token fragments are just simply copy-pasted, that's why**"()"**is needed here.

