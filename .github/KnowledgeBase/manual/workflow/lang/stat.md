# Statements

All supported statements are listed below.

## if-else

The condition must be contained in round parentheses.

**if (condition) { ... }** only executes statements in braces when **condition** is **true**.

**if (condition) { ... } else {... } ** executes statements in the first braces when **condition** is **true**.

**if (condition) { ... } else {... } ** executes statements in the second braces when **condition** is **false**.

When **condition** is a readonly variable instead of an expression: **var VAR: TYPE = EXPR**, **TYPE** must be any pointer type or optional type, and the condition is evaluated to **true** when **VAR** is not null.

## switch-case

For any statement like: **switch (expression) { case VALUE1: { S1 } case VALUE1: { S1 } ... default: { S } }**:

When **expression == VALUE1** is **true**, it executes **S1**.

When **expression == VALUE2** is **true**, it executes **S2**.

When any values fail, it executes **S**.

There can be any number of **case** branches.

The **default** branch is optional.

## for-in

See **Collection types** in [Types](../.././workflow/lang/type.md).

## while

**while (condition) {...}** evaluates **condition**.

When it is true, executes statements and do the whole thing again.

When it is false, it stops.

## goto

**goto LABEL;** jumps to the **LABEL:;** statement.

**goto LABEL;** is only allowed to jump to a label in the current function, it cannot jump across the boundary of lambda expressions.

**LABEL:;** must be the last statement in any braces. The name **LABEL** must be unique in the current function.

## break, continue, return

**break;** stops the most inner **for** or **while** statement.

**break;** stops the current iteration of the most inner **for** or **while** statement and starts a new iteration.

**return;** stops the current function. If the function returns anything other than **void**, an expression is needed before **;** to be the return value.

## delete

See [Implementing (interface)](../.././workflow/lang/interface_new.md).

See [Allocating (class)](../.././workflow/lang/class_new.md).

## raise

**raise OBJECT;** raises an expression.

**OBJECT** must be **string** or **system::Exception^**.

## try-catch-finally

**try { A } catch(ex) { B }** executes **A**. When an exception is thrown during excuting **A**, **A** stops immediately, the exception stores in **ex** and executes **B**.

**try { A } catch(ex) { B } finally { C }** executes **A**. When an exception is thrown during excuting **A**, **A** stops immediately, the exception stores in **ex** and executes **B**. After **A** or **B** is executed, whether an exception is thrown or not, **C** will be executed.

The type of **ex** is **system::Exception^**.

