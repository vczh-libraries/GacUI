# Syntax Definition

An AST definition consists of multiple rules. A**@public**can be used before any type definition to make it visible from another AST definition file.

One rule consits of multiple clauses, each clauses is defined by EBNF-like syntax:
- **RuleOrTokenName:FieldName**: Match the rule or token and store the result to the field. The class type of this clause can be either specified or inferred.**RuleName**here should not be a partial rule.
- **!RuleName**: The whole rule will modify fields in the same object from**RuleName**instead of creating a new object.
- **(Syntax)**: Match**Syntax**, just for fixing operator priority.
- **[Syntax]**: Match**Syntax**optionally.
- **+[Syntax]**: Match**Syntax**optionally, when ambiguity occurs, the opt-in one wins.
- **SyntaxA SyntaxB**: Match**SyntaxA**followed by**SyntaxB**.
- **SyntaxA | SyntaxB**: Match either**SyntaxA**or**SyntaxB**.
- **{Syntax}**: Match zero or more occurrences of**Syntax**.
- **{SyntaxA ; SyntaxB}**: Match zero or more occurrences of**SyntaxA**separating by**SyntaxB**. This is a shortcut for**[SyntaxA {SyntaxB SyntaxA}]**.A rule is defined by:
```Syntax
RuleName : TypeName
::= Clause1
...
;
```

- If any clause matches, the rule matches. If multiple clauses match, ambiguity happens.
- There are optional, alternative and loop syntax in clauses, if multiple paths of a clause match, ambiguity happens.
- Each clause has a type, either specified or inferred. If**:TypeName**is not omitted, it becomes the most detailed common type of all clauses.
- All clauses must be partial or not partial at the same time in a rule.
- **@public**can be used before a rule to make it visible to other syntax files.
- **@parser**can be used before a rule to make a public function generated for the parser class for this rule. Rules without**@parser**is not accessible in C++.
- If both**@public**and**@parser**are used, put**@public**before**@parser**.

## Ordinary Clause

**Syntax as ClassType**creates an ordinary clause. An ordinary clause should not contain**!RuleName**in its syntax. When such clause matches, a new object of**ClassType**is created to store the results of this clause.

## Reuse Clause

**Syntax**creates a reuse clause. A reuse clause should have exactly one**!RuleName**in its all possible paths. Such clause modify fields in the object from that**RuleName**instead of creating a new object.

## Partial Clause

**Syntax as partial ClassType**creates an ordinary clause. A partial clause should not contain**!RuleName**in its syntax. The syntax of the partial clause will be copy-pasted to everywhere that uses this rule, if there are multiple partial clauses in this rule, they are treated like an alternative syntax. The clause that use this rule must use the same ClassType or its sub type.

## Additional Field Assignments

At the very end of the syntax there can be additional field assignment, only for fields that are enum types:**{Field1=EnumItem1, Field2?=EnumItem2 ...}**.

Field assignments cannot be used in the middle of the syntax.

## Switches

Switches can be used when you want to exclude something when matching a rule.**switch A, !B, ...;**must appear at the very top of a syntax definition to declare all used switches in this file. Switches are visible to all syntax files therefore their names must be unique globally. Each switch has its default value, here**A**is on and**B**is off.

If we want to make some syntax that only available with certain switches are turned on or off:**?(Condition1:Syntax1 | Condition2:; | ...)**. This is a switch-enabled alternative syntax. Each branch is available only when its condition is satisfied.

A condition could look like**(A && !B | !C) && D**.

In the switch-enabled alternative syntax, each branch is tested in order, only the first satisfying branch becomes selected.**";"**means an invalid syntax, when it is selected, the whole clause is disabled. If such selected invalid syntax is in another alternative syntax, only that branch becomes invalid.

To specify switch values for a piece of syntax:**!(!A, B, ...; Syntax)**. Only inside**Syntax**referenced switches have new values assigned. In this example,**A**becomes off and**B**becomes on.

Switches with new values will be taken into**Syntax**recursively until it hits another switch value assignment.

## Calculator Example

AST Definition:
```AST
class Expr
{
}

class NumExpr : Expr
{
    var value : token;
}

enum UnaryOp
{
    Positive,
    Negative,
}

class Unary : Expr
{
    var op : UnaryOp;
    var operand : Expr;
}

enum BinaryOp
{
    Add,
    Minus,
    Multiply,
    Divid,
}

class Binary : Expr
{
    var op : BinaryOp;
    var left : Expr;
    var right : Expr;
}
```
Lexer Definition:
```LEXER
ADD:/+
SUB:-
MUL:/*
DIV://
OPEN_BRACE:/(
CLOSE_BRACE:/)
NUM:/d+(./d+)?
discard SPACE:/s+
```
Syntax Definition:
```SYNTAX
Exp0
  ::= NUM:value as NumExpr
  ::= "(" !Exp ")"
  ;

Exp1
  ::= !Exp0
  ::= "+" Exp1:operand as Unary {op = Positive}
  ::= "-" Exp1:operand as Unary {op = Negative}
  ;

Exp2
  ::= !Exp1
  ::= Exp2:left "*" Exp1:right as Binary {op = Multiply}
  ::= Exp2:left "/" Exp1:right as Binary {op = Divid}
  ;

@parser Exp
  ::= !Exp2
  ::= Exp3:left "+" Exp2:right as Binary {op = Add}
  ::= Exp3:left "-" Exp2:right as Binary {op = Minus}
  ;
```


## Switch Example

This sample demos one of the way to solve the ambiguity if-else problem. Actually the best approach is to use**if condition then statements +[else statements]**, so using switches for this problem is not recommended but it can still be an example:
```SYNTAX
switch allow_half_if;

Do
  ::= "do" ";" as DoStat
  ;

Block
  ::= "{" { Stat:stats } "}" as BlockStat
  ;

IfTail
  ::= OtherStat:thenBranch ("else" Stat:elseBranch | ?(allow_half_if: ;)) as partial IfStat
  ::= !(!allow_half_if; If:thenBranch) "else" Stat:elseBranch as partial IfStat
  ::= ?(allow_half_if: If:thenBranch) as partial IfStat
  ;

If
  ::= "if" "cond" "then" IfTail as IfStat
  ;

OtherStat
  ::= !Do
  ::= !Block
  ;

@parser Stat
  ::= !OtherStat
  ::= !(allow_half_if; !If)
  ;
```
For this input
```
if cond then if cond then do; else do;
```

- **Stat**matches the whole input, it has to be**!(allow_half_if; !If)**.
- **If**consumes the first 3 tokens and living**if cond then do; else do;**for**IfTail**.
- **IfFail**has two choices:
  - Using**!(!allow_half_if; If:thenBranch) "else" Stat:elseBranch**to match**if cond then do; else do;**
    - It means using**!(!allow_half_if; If:thenBranch)**to match**if cond then do;**, leaving**else do;**for**"else" Stat:elseBranch**.
    - Now it tries to match**If**with**allow_half_if**turned off.
    - The second level of**If**still consumes 3 tokens and leaving**do;**for**IfTail**also with**allow_half_if**turned off.
    - It has to match**OtherStat:thenBranch ("else" Stat:elseBranch | ?(allow_half_if: ;))**with**do;**.
    - Since**allow_half_if**is off,**?(allow_half_if: ;)**is invalid as all branches in fails, so the current**IfTail**is rewritten to**OtherStat:thenBranch "else" Stat:elseBranch**.
    - Obviously it cannot match**do;**, therefore fails.
  - Using**?(allow_half_if: If:thenBranch)**to match**if cond then do; else do;**
    - Given that when**allow_half_if**is on,**If**accepts all forms, so it succeeds.
- So the "else" associates with the second "if", instead of the first one or both.

