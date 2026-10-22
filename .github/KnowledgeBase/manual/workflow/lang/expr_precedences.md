# Precedences

Precedences of all kinds of expressions are listed below in descending precedence. Parentheses group an expression: **(EXPRESSION)** can be used wherever a precedence-1 expression is required. For example, **(1 + 2) * 3** evaluates the addition first.

## 1

- Literals, names and other primitive expressions.
- **\<EXPR-PRECEDENCE-1\>**.observe ( ... )
- **\<EXPR-PRECEDENCE-1\>**.observe as NAME ( ... )
- **\<EXPR-PRECEDENCE-1\>**( ... )
- **\<EXPR-PRECEDENCE-1\>**.NAME
- **\<EXPR-PRECEDENCE-1\>**::NAME
- **\<EXPR-PRECEDENCE-1\>**[ ... ]
- **\<EXPR-PRECEDENCE-1\>** in ...
- **\<EXPR-PRECEDENCE-1\>** not in ...
- **\<EXPR-PRECEDENCE-1\>** of TYPE
- **\<EXPR-PRECEDENCE-1\>** as TYPE
- **\<EXPR-PRECEDENCE-1\>** as *
- **\<EXPR-PRECEDENCE-1\>** is TYPE
- **\<EXPR-PRECEDENCE-1\>** is not TYPE
- **\<EXPR-PRECEDENCE-1\>** is null
- **\<EXPR-PRECEDENCE-1\>** is not null

The collection operand after **in** or **not in** is a full expression. When combining a membership result with another operator, write **(value in values) and condition** or **(value not in values) and condition** to keep that operator outside the collection operand.

## 2 (unary operators)

- **+**
- **-**
- **not**
- **cast**
- **variant in** These unary operators group from right to left. For example, **cast int variant in value** means **cast int (variant in value)**.

## 3

- **^**

## 4

- *****
- **/**
- **%**

## 5

- **+**
- **-**
- **&**
- **|**

## 6

- **shl**
- **shr**

## 7

- **\<**
- **\>**
- **\<=**
- **\>=**
- **==**
- **!=**

## 8

- **xor**

## 9

- **and**

## 10

- **or**

## 11

- **??**

## 12 (if expression)

- **? :**

## 13 (assignment expression)

- **=**

