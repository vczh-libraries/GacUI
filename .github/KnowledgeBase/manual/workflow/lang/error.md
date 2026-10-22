# Index of Errors

This index lists every numbered Workflow compiler diagnostic, including RPC and C++ generation errors. A code can have several messages; all message templates are listed under that code. Words in braces, such as `{type}`, are filled in by the compiler. `{candidates}` can span several lines. Message spelling and punctuation follow the compiler.

Examples are intentionally invalid. Unless a context is stated, each code block is a complete module. An example may produce more than one diagnostic. Links beside examples lead to the corresponding compiler test or implementation. Runtime exceptions are separate from these compile-time diagnostics.

The numbered messages are defined in [WfAnalyzer_Errors.cpp](https://github.com/vczh-libraries/Workflow/blob/master/Source/Analyzer/WfAnalyzer_Errors.cpp). The compiler reports a source location together with each message.

## Parser errors without a code

Lexical and grammatical errors occur before semantic analysis and have no numbered Workflow code. The following messages come from the [shared parser](https://github.com/vczh-libraries/VlppParser2/blob/master/Source/SyntaxBase.cpp).

### Unrecognized token

```Text
Unrecognized token: "{token}".
```

Example: `#` outside a string or comment is not a Workflow token.

### Incorrect input

```Text
Parser stops at incorrect input: "{token}".
```

```Workflow
module test;
var value = ;
```

### Incomplete input

```Text
Input is incomplete.
```

```Workflow
module test;
func main():void {
```

### Unexpected AST type

```Text
Unexpected type of the created AST.
```

Example context: a C++ parser client requests a module AST type while selecting an expression grammar entry, then parses `1`. The produced expression AST does not match the requested type. The standard Workflow parsing helpers pair the grammar entry and AST type correctly.

## A: Expressions

### A0: Malformed format string

```Text
A0: Wrong format string syntax.

A0: Wrong format string syntax. (Does not find matched close bracket.)

A0: Wrong format string syntax. ({parserMessage})
```

Format-string errors append either the unmatched-parenthesis detail or the parser error for the embedded expression.

```Workflow
module test;

func main():void
{
    raise $"$(1";
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A0_WrongFormatStringSyntax2.txt).

### A1: Invalid observed property

```Text
A1: Simple observe expression should observe a property under the observed object.
```

```Workflow
module test;

func main():void
{
    bind(a.observe(b.c));
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A1_WrongSimpleObserveExpression.txt).

### A2: Invalid or missing observed event

```Text
A2: Simple observe expression should react to an event under the observed object.

A2: Observe expression should react to at least one event.
```

```Workflow
module test;

func main():void
{
    bind(a.observe(b on c.d));
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A2_WrongSimpleObserveEvent.txt).

### A3: Observe outside bind

```Text
A3: Observe expression should appear in a bind expression.
```

```Workflow
module test;

func main():void
{
    a.observe(b);
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A3_ObserveNotInBind.txt).

### A4: Invalid nesting inside bind

```Text
A4: Observe expression should not appear in the event expression in another observe expression.

A4: Bind expression should not appear in another bind expression.

A4: Attach expression should not appear in another bind expression.

A4: Detach expression should not appear in another bind expression.
```

```Workflow
module test;

func main():void
{
    bind(a.observe as _a(bind(_a.b) on _a.bChanged));
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A4_BindInBind.txt).

### A5: Mixed constructor entries

```Text
A5: Key-value pairs are not allowed in list constructor expression.

A5: Field-value pairs are expected in struct constructor expression.

A5: The same field cannot be assigned more than once in struct constructor expression.
```

```Workflow
module test;

func main():void
{
    var x = {1  2:3};
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A5_ConstructorMixMapAndList.txt).

### A6: Mixed class and interface construction

```Text
A6: Arguments are not allowed in new interface expression.

A6: Members are not allowed in new class expression.
```

```Workflow
module test;
interface I {}
var value = new I^();
```

The target is an interface, but parentheses select class construction. An interface implementation uses new I^ { ... }.

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A6_InterfaceWithClassConstruction.txt).

### A7: Name or event used as a value

```Text
A7: Symbol "{scope}" cannot be used as an expression.

A7: Event "{event}" of type "{ownerType}" cannot be used as an expression.
```

```Workflow
module test;

namespace ns
{
    var x = ns;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A7_ScopeNameIsNotExpression.txt).

### A8: Wrong reference kind

```Text
A8: Expression does not reference to a declaration.

A8: Expression does not reference to an event.
```

```Workflow
module test;

func main():void
{
    var x : int = 0;
    var y = x::Child;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A8_ExpressionIsNotScopeName.txt).

### A9: Unresolved expression type

```Text
A9: Expression referencing to symbol "{symbol}" failed to resolve its type.

A9: Expression "null" failed to resolve its type.

A9: Expression "{}" failed to resolve its type.

A9: Ordered lambda expression failed to resolve its type.
```

```Workflow
module test;

func main():void
{
    var x = {};
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A9_ConstructorCannotResolveType.txt).

### A10: Invalid implicit conversion

```Text
A10: Expression "null" cannot implicitly convert to "{toType}".

A10: Expression "{}" cannot implicitly convert to "{toType}".

A10: Ordered lambda expression cannot implicitly convert to "{toType}".

A10: Expression of type "{fromType}" cannot implicitly convert to "{toType}".
```

```Workflow
module test;

func main():void
{
    var x : string = {};
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A10_ConstructorToStruct.txt).

### A11: Invalid explicit conversion

```Text
A11: Expression of type "{fromType}" cannot explicitly convert to "{toType}".
```

```Workflow
module test;
using system::*;

var x = 1 of variant<bool, string>;
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A11_VariantInferMissing.txt).

### A12: Invalid weak cast

```Text
A12: Expression cannot weakly cast to "{toType}" because it cannot be "null".
```

```Workflow
module test;
using system::*;

var x = 1 as variant<int, string>;
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A12_VariantWeakTV.txt).

### A13: Literal out of range

```Text
A13: Integer literal "{literal}" out of range.

A13: Floating literal "{literal}" out of range.
```

```Workflow
module test;
using system::*;

func main():void
{
    var x = 18446744073709551615;
    var y = 18446744073709551616;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A13_IntegerLiteralOutOfRange.txt).

### A14: Incompatible result types

```Text
A14: Failed to merge type "{firstType}" with type "{secondType}" together to calculate the result type.
```

```Workflow
module test;
using system::*;

func main():void
{
    {1 true "text"};
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A14_IncompatibleList.txt).

### A15: Non-integer range

```Text
A15: Elements in a range expression should be integer, not "{type}".
```

```Workflow
module test;
using system::*;

func main():void
{
    range [true, false];
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A15_RangeOfBoolean.txt).

### A16: Unsupported operand type

```Text
A16: Unary operator cannot apply on expression of type "{type}".

A16: Binary operator cannot apply on expression of type "{type}".
```

```Workflow
module test;
using system::*;

func f(x:int):void { var y = variant in x; }
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A16_VariantExtractScalar.txt).

### A17: Unsupported index access

```Text
A17: Container of type "{containerType}" cannot be accessed using index.
```

```Workflow
module test;
using system::*;

func f(x:variant<int[], string>):void { var y = x[0]; }
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A17_VariantIndex.txt).

### A18: Non-enumerable expression

```Text
A18: Expression of type "{type}" is not an enumerable type.
```

```Workflow
module test;
using system::*;

func f(x:variant<int[], string>):void { for (y in x) {} }
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A18_VariantEnumeration.txt).

### A19: Non-callable expression

```Text
A19: Expression of type "{type}" is not an invokable function type.
```

```Workflow
module test;
using system::*;

func main():void
{
    var x : int = 0;
    x(0);
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A19_CallStruct.txt).

### A20: Wrong argument count

```Text
A20: Function {function}" is not allowed to call with {count} arguments.
```

```Workflow
module test;
using system::*;

class A
{
    new(x : int, y : string) {}
}

class B : A
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A20_CallWrongArgumentCount3.txt).

### A21: Wrong argument type

```Text
A21: The {index}-th argument of function {function} cannot implicitly convert from "{fromType}" to "{toType}".
```

```Workflow
module test;
using system::*;

func f(x:variant<int, string>):void { var y = bind(x); }
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A21_VariantBindingResult.txt).

### A22: Ambiguous overload

```Text
A22: Cannot decide which function to call in multiple targets: {candidates}.
```

This fixture uses the native test::ObservableValue class registered by the compiler test host. Its constructor overloads make the null argument ambiguous.

```Workflow
module test;
using test::*;

func main():void
{
    new ObservableValue^(0, null);
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A22_CannotPickOverloadedConstructor.txt).

### A23: Missing constructor

```Text
A23: Class "{type}" does not contain any constructor.

A23: Interface "{type}" does not contain any constructor receiving an "{proxyType}".
```

```Workflow
module test;
var value = new variant<int, string>();
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A23_VariantNewClass.txt).

### A24: Wrong constructor return type

```Text
A24: The return type of {function} cannot implicitly convert from "{fromType}" to "{toType}".
```

This fixture uses the native test::ObservableValue class registered by the compiler test host. The selected constructor returns a raw pointer, but the expression requests a shared pointer.

```Workflow
module test;
using test::*;

func main():void
{
    new ObservableValue^(0, null of bool?);
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A24_CallRawConstructorToShared.txt).

### A25: Non-assignable expression

```Text
A25: {expression} is not assignable.
```

```Workflow
module test;

func main():void
{
    1=2;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A25_AssignToConstant.txt).

### A26: Instance member used as static

```Text
A26: {expression} cannot be called as a static member.
```

```Workflow
module test;

interface IMyInterface
{
    func GetName() : string;
    func SetName(value : string) : void;
    event NameChanged();
    prop Name : string {GetName, SetName : NameChanged}
}

func main():void
{
    IMyInterface::Name;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A26_CallPropertyOutsideOfInterface.txt).

### A27: Instance member in a static context

```Text
A27: {expression} cannot be called in static functions of its class or interface.
```

```Workflow
module test;

class A
{
    new(a : int){}
}

class B : A
{
    var x : int = 0;

    new() :A(x) {}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A27_CallPropertyInsideBaseConstructorCall.txt).

### A28: Instance member in a field initializer

```Text
A28: {expression} cannot be used to initialize other fields in the same type or new interface expression.
```

```Workflow
module test;

class A
{
    var a : int = 10;
    var b : int = a;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A28_FieldsInitializeUsingEachOther3.txt).

### A29: This outside an instance context

```Text
A29: The "this" expression can only be used in class methods or functions in new interface expressions
```

```Workflow
module test;

var a = this;
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A29_ThisInGlobalVariable.txt).

### A30: Invalid flag or string operands

```Text
A30: Expression of type "{type}" cannot be used in union (&) expression because it is not string or flag enum.

A30: Expression of type "{type}" cannot be used in intersect (|) expression because it is not flag enum.
```

```Workflow
module test;
var value = 1 | 2;
```

The | operator requires flag-enum operands. Integers use the separate bitwise operators.

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A30_FlagOperatorOnInteger.txt).

### A31: Non-constant expression

```Text
A31: Expression is not constant. A constant expression consists of:
    constant primitive values
    enum items
    constant unary (+, -, not) expressions
    constant binary (|) expressions
    constant range expressions
    constant constructor expressions (to create values of structs, lists or dictionarys)
    type() or typeof() expressions.
```

This fixture uses the native @test:Int attribute registered by the compiler test host. Its argument refers to a variable instead of a constant expression.

```Workflow
module test;

var i = 100;
var s = "Text";

@test:Int(i)
var x = 0;
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A31_ExpressionNotConstant.txt).

### A32: Invalid mixin cast

```Text
A32: Mixin cast cannot implicitly convert from "{fromType}" to "{toType}". It is only for converting from an interface to a derived interface.
```

```Workflow
module test;

interface IA{}
interface IB{}

var a = new IB^(using new IA^{});
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A32_MixinCastBetweenUnrelatedInterfaces.txt).

### A33: Missing expected type

```Text
A33: Cannot resolve type "*". This expression should only appear on a place which has an explicit type, including variables and arguments.
```

```Workflow
module test;

var x = 0 as *;
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/A33_WeakExpectedTypeCastToImplicitType.txt).

## B: Types

### B0: Void value type

```Text
B0: Void is not a type for a value.
```

```Workflow
module test;

func main(args : void):void
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/B0_VoidArgument.txt).

### B1: Interface value type

```Text
B1: Interface is not a type for a value.
```

```Workflow
module test;

func main(args : interface):void
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/B1_InterfaceArgument.txt).

### B2: Invalid raw pointer

```Text
B2: A raw pointer should point to a class or interface.

B2: A raw pointer "{type}" should point to a class or interface.
```

```Workflow
module test;

func main(): ::system::String*
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/B2_RawPointerToStruct.txt).

### B3: Invalid shared pointer

```Text
B3: A shared pointer should point to a class or interface.

B3: A shared pointer "{type}" should point to a class or interface.
```

```Workflow
module test;

func main(): ::system::String^
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/B3_SharedPointerToStruct.txt).

### B4: Invalid nullable type

```Text
B4: A nullable value should point to a struct.

B4: A nullable value "{type}" should point to a struct.
```

```Workflow
module test;

func main(): ::system::Function?
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/B4_NullableToClass.txt).

### B5: Invalid child type

```Text
B5: Only a reference type have child types.
```

```Workflow
module test;
func f(value:variant<int, string>::Child):void {}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/B5_VariantChild.txt).

### B6: Name is not a type

```Text
B6: "{scope}" is not a type.

B6: "{symbol}" is not a type.
```

```Workflow
module test;

class MyClass
{
    prop x : x = 0 {}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/B6_PropertyIsNotType.txt).

### B7: Reference type used as a value type

```Text
B7: Type "{type}" is not a type for a value.
```

```Workflow
module test;

func main(): ::system::Function
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/B7_ClassNotForValue.txt).

## C: Statements and coroutines

### C0: Break outside a loop

```Text
C0: Break statement should appear in a loop.
```

```Workflow
module test;

func main():void
{
    break;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C0_BreakNotInLoop.txt).

### C1: Continue outside a loop

```Text
C1: Continue statement should appear in a loop.
```

```Workflow
module test;

func main():void
{
    continue;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C1_ContinueNotInLoop.txt).

### C2: Re-raise outside catch

```Text
C2: Re-raise exception statement should appear in catch.
```

```Workflow
module test;

func main():void
{
    raise;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C2_RethrowNotInCatch.txt).

### C3: Incomplete try statement

```Text
C3: Try statement should not appear without both catch and finally.
```

```Workflow
module test;

func main():void
{
    try{}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C3_TryMissCatchAndFinally.txt).

### C4: Missing return value

```Text
C4: Return statement requires an expression of type "{type}".
```

```Workflow
module test;

func main():int
{
    return;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C4_ReturnMissExpression.txt).

### C5: Invalid delete operand

```Text
C5: Delete statement cannot apply on an expression of type "{type}", which is expected to be a raw pointer.
```

```Workflow
module test;
using system::*;

func main():void
{
    delete null of Function^;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C5_DeleteNonRawPointer.txt).

### C6: Unexpected return value

```Text
C6: Return statement cannot have a value inside void functions, constructors, destructors or $coroutine expressions.
```

```Workflow
module test;
using system::*;

static
{
    return 0;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C6_ReturnValueInStaticInit.txt).

### C7: Invalid pause placement

```Text
C7: $pause statement should appear inside a $coroutine expression, and it cannot be nested.
```

```Workflow
module test;
using system::*;

func main():void
{
    $pause;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C7_CoPauseInFunction.txt).

### C8: Coroutine operation outside a coroutine function

```Text
C8: $Operator statement should appear inside a coroutine function (which has a functiona body like ${} or $Provider{}).

C8: $.Operator expression should appear inside a coroutine function (which has a functiona body like ${} or $Provider{}).
```

```Workflow
module test;
using system::*;

func main():void
{
    $Await;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C8_CoOperatorInFunction.txt).

### C9: Missing coroutine provider

```Text
C9: Cannot find a coroutine provider based on the function return type, all of the following types do not exist: {candidates}.

C9: Cannot find a coroutine provider based on the provider name "{provider}", all of the following types do not exist: {candidates}.
```

```Workflow
module test;
using system::*;

func main():void
$Delay{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C9_CoProviderNotExists2.txt).

### C10: Missing coroutine operation

```Text
C10: Static function "ReturnAndExit" does not exist in coroutine provider "{type}".

C10: Static functions "{operator}AndPause" and "{operator}AndRead" do not exist in coroutine provider "{type}".

C10: Static function "{operator}AndRead" does not exist in coroutine provider "{type}".

C10: Static function "QueryContext" does not exist in coroutine provider "{type}".
```

```Workflow
module test;
using system::*;

class XCoroutine
{
    static func CreateAndRun(creator : func(object) : (Coroutine^)) : void {}
}

func main():void
$X{
    return;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C10_CoOperatorNotExists.txt).

### C11: Unresolved coroutine result type

```Text
C11: Failed to resolve the result type of coroutine operator "{operator}", since there is no argument.

C11: Failed to resolve the result type of coroutine operator "{operator}", no appropriate static function "CastResult" is found in the following types. It requires exactly one argument of type "object" with a return type which is not "void": {candidates}.
```

```Workflow
module test;
using system::*;

class XCoroutine
{
    static func AwaitAndRead(impl : object):void {}
    static func CreateAndRun(creator : func(object) : (Coroutine^)) : void {}
}

func main():void
$X{
    var result = $Await;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C11_CoOperatorCannotResolveResultType.txt).

### C12: Missing coroutine creator

```Text
C12: The required static function "Create" does not exist in coroutine provider "{type}". It is required to have exactly one argument of a function type, which consumes a pointer type and returns system::Coroutine^

C12: The required static function "CreateAndRun" does not exist in coroutine provider "{type}". It is required to have exactly one argument of a function type, which consumes a pointer type and returns system::Coroutine^
```

```Workflow
module test;
using system::*;

class XCoroutine
{
}

func main():void
$X{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C12_CoProviderCreatorNotExists.txt).

### C13: Missing goto target

```Text
C13: Goto label "{label}" does not exists. A goto label can only jump to the end of any block statement that contains this goto statement in the current function.
```

```Workflow
module test;
using system::*;

func main():void
{
    goto NotExists;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C13_GotoLabelNotExists.txt).

### C14: Ambiguous goto target

```Text
C14: Too many goto targets "{label}" found.
```

```Workflow
module test;
using system::*;

func main():void
{
    {
        goto Label;
        Label:;
    }
    Label:;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C14_TooManyGotoTargets.txt).

### C15: State switch outside a state

```Text
C15: $switch statement should appear inside a $state declaration.
```

```Workflow
module test;
using system::*;

func main():void
{
    $switch (){}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C15_StateSwitchNotInStateDecl.txt).

### C16: State transition outside a state

```Text
C16: $goto_state or $push_state statement should appear inside a $state declaration.
```

```Workflow
module test;
using system::*;

func main():void
{
    $goto_state X();
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C16_GotoStateNotInStateDecl.txt).

### C17: Missing state input

```Text
C17: State input "{name}" does not exist.
```

```Workflow
module test;

using system::*;

class SM : StateMachine
{
    $state_machine
    {
        $state_input A(x: int, y: int);

        $state default()
        {
            $switch ()
            {
                case B():
                {
                }
            }
        }
    }
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C17_StateInputNotExists.txt).

### C18: Wrong state input argument count

```Text
C18: The number of arguments doesn not match the declaration of state input "{name}".
```

```Workflow
module test;

using system::*;

class SM : StateMachine
{
    $state_machine
    {
        $state_input A(x: int, y: int);

        $state default()
        {
            $switch ()
            {
                case A(x):
                {
                }
            }
        }
    }
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C18_StateInputArgumentNotMatch.txt).

### C19: Missing state

```Text
C19: State "{name}" does not exist.
```

```Workflow
module test;

using system::*;

class SM : StateMachine
{
    $state_machine
    {
        $state default()
        {
            $goto_state A();
        }
    }
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C19_StateNotExists.txt).

### C20: Wrong state argument count

```Text
C20: The number of arguments doesn not match the declaration of state "{name}".
```

```Workflow
module test;

using system::*;

class SM : StateMachine
{
    $state_machine
    {
        $state A(x: int)
        {
        }

        $state default()
        {
            $goto_state A();
        }
    }
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/C20_StateArgumentNotMatch.txt).

## D: Declarations

### D0: Incomplete function declaration

```Text
D0: Function should have a name.

D0: Function should be implemented.

D0: Interface method should not be implemented.
```

```Workflow
module test;

func main():void;
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D0_FunctionNoBlock.txt).

### D1: Conflicting declarations

```Text
D1: Duplicated declaration "{name}", it has already beed defined as a {declarationKind}.

D1: Duplicated $state_machine declaration.
```

```Workflow
module test;

namespace system
{
    class String
    {
    }
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D1_DuplicatedClass.txt).

### D2: Duplicate symbol

```Text
D2: Duplicated symbol "{symbol}".
```

```Workflow
module test;

func main(args:int, args:string[]):void
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D2_DuplicatedFunctionArgument.txt).

### D3: Unimplemented interface method

```Text
D3: Interface method not implemented: {symbol}.
```

```Workflow
module test;
using system::*;

func main():void
{
    new Subscription^
    {
        override func Open() : bool
        {
        }

        override func Close() : bool
        {
        }
    };
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D3_ImplementMissFunction.txt).

### D4: Unknown interface implementation

```Text
D4: Interface "{interfaceType}" does not contain method "{name}" which is in type "{methodType}".
```

```Workflow
module test;
using system::*;

func main():void
{
    new Subscription^
    {
        override func Open() : bool
        {
        }

        override func UpdateNotExists() : bool
        {
        }

        override func Update() : bool
        {
        }

        override func Close() : bool
        {
        }
    };
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D4_ImplementUnexistsFunction.txt).

### D5: Ambiguous interface implementation

```Text
D5: Cannot decide which function to implement in multiple targets:{candidates}.
```

Example context: native reflection exposes two distinct methods named Run with the same func():void signature on one interface. A new interface expression with override func Run():void {} cannot choose which reflected method to implement. This diagnostic is checked after matching both the name and the complete function type; ordinary Workflow duplicate declarations can be rejected earlier.

[Compiler check](https://github.com/vczh-libraries/Workflow/blob/master/Source/Analyzer/WfAnalyzer_ValidateSemantic_Expression.cpp).

### D6: Duplicate method implementation

```Text
D6: There are some other methods named "{name}" whose types are also "{type}".
```

```Workflow
module test;
using system::*;

func main():void
{
    new Subscription^
    {
        override func Open() : bool
        {
        }

        override func Update() : bool
        {
        }

        override func Update() : bool
        {
        }

        override func Close() : bool
        {
        }
    };
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D6_ImplementDuplicateFunction.txt).

### D7: Declaration in the wrong scope

```Text
D7: Event "{name}" cannot be defined outside of classes or interfaces.

D7: Property "{name}" cannot be defined outside of classes or interfaces.

D7: Constructor cannot be defined outside of classes.

D7: Destructor cannot be defined outside of classes.

D7: Auto-property cannot be defined outside of classes.

D7: State machine cannot be defined outside of classes.

D7: Static initialization declaration cannot be defined inside classes or interfaces.
```

```Workflow
module test;
using system::*;

new(){}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D7_ConstructorOutsideOfClasses.txt).

### D8: Invalid anonymous interface member

```Text
D8: "{name}" cannot be defined in an new interface expression, only functions, variables and auto-properties are allowed.
```

```Workflow
module test;

interface IMyInterface
{
}

func main() : void
{
    new IMyInterface^
    {
        class A{}
    };
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D8_ClassInNewInterface.txt).

### D9: Invalid enum values

```Text
D9: Item values in enum "{name}" should be consecutive and starts from zero, like 0, 1, 2, 3, 4, ...

D9: Item values in flag enum "{name}" should be consecutive powers of 2 and starts from zero, like 0, 1, 2, 4, 8, ...
```

```Workflow
module test;

enum Seasons
{
    Spring = 0,
    Summer = 1,
    Autumn = 2,
    Winter = 4,
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D9_EnumValue.txt).

### D10: Missing flag item

```Text
D10: Item "{name} does not exists in the current flag enum "{owner}".
```

```Workflow
module test;

flagenum Seasons
{
    Spring = 0,
    Summer = 1,
    Autumn = 2,
    Winter = 4,

    Good = SUMMER | WINTER,
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D10_FlagValueNotExists.txt).

### D11: Duplicate enum item

```Text
D11: Item "{name} already exists in the current enum "{owner}".
```

```Workflow
module test;

flagenum Seasons
{
    Spring = 0,
    Summer = 1,
    Autumn = 2,
    Winter = 4,
    Winter = 8,
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D11_DuplicatedEnumValue.txt).

### D12: Invalid struct member type

```Text
D12: Type of member "{name}" of struct "{owner}" is not value type.
```

```Workflow
module test;

struct Point
{
    x : object;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D12_StructContainsObject.txt).

### D13: Recursive struct values

```Text
D13: Recursive references are found in these struct definitions:{candidates}.
```

```Workflow
module test;
using system::*;

struct Recursive { data:variant<int, Recursive>; }
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D13_VariantRecursiveStruct.txt).

### D14: Duplicate struct member

```Text
D14: Member "{name}" already exists in struct "{owner}".
```

```Workflow
module test;

struct Point
{
    x : int;
    x : int;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D14_DuplicatedStructMember.txt).

### D15: Invalid attribute

```Text
D15: Attribute "{category}:{name}" does not exist.

D15: Value of attribute "{category}:{name}" is missing.
```

```Workflow
module test;

@cpp:NotExists
var x = 0;
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D15_AttributeNotExists.txt).

### D16: Invalid state machine base

```Text
D16: State machine class "{name}" should inherit from "system::StateMachine" directly.
```

```Workflow
module test;

class SM
{
    $state_machine{}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D16_SmcNotInheritFromStateMachine.txt).

### D17: Missing default state

```Text
D17: Missing default state.
```

```Workflow
module test;

using system::*;

class SM : StateMachine
{
    $state_machine{}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/D17_SmcMissingDefaultState.txt).

## E: Using paths

### E0: Invalid using wildcard

```Text
E0: Wild card "*" should only appear in the last item of the using path and should appear once.
```

```Workflow
module test;

using system;
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/E0_NoWildCard.txt).

## F: Name resolution

### F0: Unknown top-level name

```Text
F0: Top qualified symbol "{name}" does not exist.
```

```Workflow
module test;

func main(): ::not_exists
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/F0_TypeTopQualifiedSymbolNotExists.txt).

### F1: Unknown child name or member

```Text
F1: Symbol "{name}" does not exist in "{scope}".

F1: Member "{name}" does not exist in "{type}".
```

```Workflow
module test;

func main(): ::system::not_exists
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/F1_TypeChildSymbolNotExists.txt).

### F2: Unknown name or enum item

```Text
F2: Symbol "{name}" does not exist in the current scope.

F2: Enum item "{name}" does not exist in enum type "{type}".
```

```Workflow
module test;

func main():not_exists
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/F2_TypeReferenceNotExists.txt).

### F3: Ambiguous name

```Text
F3: Symbol "{name}" references to too many targets: {candidates}.
```

```Workflow
module test;

func main():void
{
    var x : int = 0;
    {
        var x : double = 1.0;
        var y = x;
    }
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/F3_ExpressionReferenceToVariablesInNestedBlocks.txt).

## G: Classes and interfaces

### G1: Invalid member modifier

```Text
G1: Class member "{name}" cannot be static or override because it is not a function or a auto-property.

G1: Class member "{name}" cannot be static because it is in a new interface expression.

G1: Auto property "{name}" cannot be normal outside of classes or interfaces.

G1: Auto property "{name}" cannot be static  .
```

```Workflow
module test;

class A
{
    static prop P : string {}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G1_StaticAutoPropertyInClass.txt).

### G2: Namespace inside a class

```Text
G2: Namespace "{name}" cannot be a class member.
```

```Workflow
module test;

class A
{
    namespace b
    {
    }
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G2_NamespaceInClass.txt).

### G3: Missing property accessor or event

```Text
G3: Cannot find the getter "{getter}" of property "{name}" in type "{class}". A property getter should be a non-static method in the same type.

G3: Cannot find the setter "{setter}" of property "{name}" in type "{class}". A property setter should be a non-static method in the same type.

G3: Cannot find the event "{event}" of property "{name}" in type "{class}".
```

```Workflow
module test;

interface A
{
    func GetY() : int;
    func SetX(value : int) : void;
    prop X : int {GetX, SetX}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G3_UnexistingPropertyGetter.txt).

### G4: Ambiguous property accessor or event

```Text
G4: Too many symbols found for the getter "{getter}" of property "{name}" in type "{class}".

G4: Too many symbols found for the setter "{setter}" of property "{name}" in type "{class}".

G4: Too many symbols found for the event "{event}" of property "{name}" in type "{class}".
```

```Workflow
module test;

interface A
{
    func GetX() : int;
    func SetX() : void;
    func SetX(value : int) : void;
    prop X : int {GetX, SetX}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G4_TooManyPropertySetter.txt).

### G5: Wrong property accessor signature

```Text
G5: Cannot match the getter "{getter}" of property "{name}" in type "{class}". A property getter should have no argument, and its return type should be identical to the property type.

G5: Cannot match the setter "{getter}" of property "{name}" in type "{class}". A property setter should have no return value and have only one argument, and the argument type should be identical to the property type.
```

```Workflow
module test;

interface A
{
    func GetX() : int;
    func SetX() : void;
    prop X : int {GetX, SetX}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G5_PropertySetterWrongArgumentType.txt).

### G6: Invalid base type

```Text
G6: A base type of the type "{name}" should be another custom type, it cannot be any predefined type, pointer type, shared pointer type, nullable type, collection type, or function type

G6: Base type "{type}" of class "{name}" is not a class, or it is a class but it is not aggregatable.

G6: Base type "{type}" of interface "{name}" is not an interface.

G6: Interface "{type}" should contain a constructor receiving an "{proxyType}" to be the base type of "{name}".
```

```Workflow
module test;

class A
{
    new() :B*() {}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G6_WrongBaseConstructorType.txt).

### G7: Interface constructor mode on a class

```Text
G7: Cannot use interface constructor type on class "{name}".
```

Example context: a C++ compiler client constructs a WfClassDeclaration with kind = WfClassKind::Class and constructorType = WfConstructorType::SharedPtr. Structure validation rejects that AST. The text grammar sets a class declaration to Undefined automatically, so class C {} does not cause this error.

[Compiler check](https://github.com/vczh-libraries/Workflow/blob/master/Source/Analyzer/WfAnalyzer_ValidateStructure_Declaration.cpp).

### G8: Override without an interface method

```Text
G8: Function "{name}" cannot be override because it doesn't implement any interface methods.

G8: Auto-property "{name}" cannot be override because it doesn't implement any interface methods.
```

```Workflow
module test;

interface A
{
    override func B() : void;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G8_OverrideInInterface.txt).

### G9: Missing field type

```Text
G9: Type of field "{name}" is missing.
```

```Workflow
module test;

class A
{
    var x = 0;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G9_MissingFieldType.txt).

### G10: Invalid inheritance graph

```Text
G10: Class "{name}" inherits from another class "{type}" for multiple times.

G10: Recursive inheriting are found in these class definitions:{candidates}.

G10: Recursive inheriting are found in these interface definitions:{candidates}.
```

```Workflow
module test;

interface A : A
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G10_DuplicatedBaseInterface1.txt).

### G11: Initializer for a non-base type

```Text
G11: Type "{type}" is not a base type of this class.
```

```Workflow
module test;

class A
{
    new() :B() {}
}

class B
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G11_WrongBaseConstructorTypes.txt).

### G12: Duplicate base initializer

```Text
G12: Base type "{type}" has already been initialized.
```

```Workflow
module test;

class A
{
}

class B : A
{
    new() :A() ,A() {}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G12_DuplicatedBaseConstructorTypes.txt).

### G13: Invalid destructor or auto-property initialization

```Text
G13: Class "{class}" has too many destructors.

G13: New interface expression has too many destructors.

G13: Auto property "{name}" should be initialized.

G13: Auto property "{name}" cannot be initialized in interface "{class}".
```

```Workflow
module test;

class A
{
    delete {}
    delete {}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/G13_DuplicatedDestructor.txt).

## H: RPC definitions

### H0: RPC attribute on a non-interface

```Text
H0: @rpc:Interface can only apply to an interface definition, but not "{fullName}".
```

```Workflow
module test;

@rpc:Interface
class A
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H0_RpcInterface_NotOnInterface_Class.txt).

### H1: Non-serializable RPC base

```Text
H1: @rpc:Interface cannot be applied to interface "{interface}" because its base type "{base}" is not serializable.
```

```Workflow
module test;

interface IBase
{
}

@rpc:Interface
interface IDerived : IBase
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H1_RpcInterface_BaseNotSerializable.txt).

### H2: Non-serializable RPC member

```Text
H2: @rpc:Interface cannot be applied to interface "{interface}" because its member "{member}" is not serializable.
```

```Workflow
module test;

@rpc:Interface
interface IMy
{
prop P : IMy* {}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H2_RpcInterface_MemberNotSerializable_Property.txt).

### H3: RPC constructor on a non-RPC interface

```Text
H3: @rpc:Ctor can only apply to an interface definition with @rpc:Interface.
```

```Workflow
module test;

@rpc:Ctor
class A
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H3_RpcCtor_NotOnRpcInterface_Class.txt).

### H4: Invalid RPC transfer attribute target

```Text
H4: @rpc:{attribute} can only apply to a property, a method or a parameter.
```

```Workflow
module test;

interface IMy
{
}

@rpc:Byref
var x = 0;
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H4_RpcByref_WrongTarget.txt).

### H5: RPC attribute outside an RPC interface

```Text
H5: @rpc:{attribute} can only be used inside an interface type with @rpc:Interface.
```

```Workflow
module test;

interface IMy
{
@rpc:Byref
func M() : int[];
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H5_RpcByref_NotInsideRpcInterface.txt).

### H6: RPC transfer on a non-collection

```Text
H6: @rpc:{attribute} cannot be used on member "{member}" because it does not have a strong typed collection types.
```

```Workflow
module test;

@rpc:Interface
interface IMy
{
@rpc:Byref
prop P : int {}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H6_RpcByref_NotStrongCollection_Property.txt).

### H7: RPC property attribute on another declaration

```Text
H7: @rpc:{attribute} can only apply to a property.
```

```Workflow
module test;

@rpc:Cached
func main() : void
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H7_RpcCached_WrongTarget.txt).

### H8: Conflicting RPC attributes

```Text
H8: @rpc:{attribute} cannot be used on member "{member}" because it already has @rpc:{otherAttribute}.
```

```Workflow
module test;

@rpc:Interface
interface IMy
{
@rpc:Byval
@rpc:Byref
func M() : int[];
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H8_RpcByref_WithByval.txt).

### H9: Duplicate generated RPC name

```Text
H9: RPC generated {category} name "{generatedName}" is duplicated.
```

Example context: generated RPC metadata attempts to insert two event entries with the same full name, such as Service.Changed. The second entry is rejected. The same check covers generated type and method names. This is a metadata-generation consistency check; ordinary source-level duplicates may be rejected before this stage.

[Compiler check](https://github.com/vczh-libraries/Workflow/blob/master/Source/Analyzer/Rpc/WfAnalyzer_ValidateRPC.cpp).

### H10: Reserved RPC return type

```Text
H10: @rpc:Interface cannot be applied to interface "{interface}" because its member "{member}" uses reserved RPC type "{type}" in a return value.
```

```Workflow
module test;
using system::*;

@rpc:Interface
interface IMy
{
  func M() : system::RpcException;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H10_RpcReservedType_Return_RpcException.txt).

### H11: Reserved RPC parameter type

```Text
H11: @rpc:Interface cannot be applied to interface "{interface}" because its function argument "{member}" uses reserved RPC type "{type}".
```

```Workflow
module test;
using system::*;

@rpc:Interface
interface IMy
{
  func M(x : system::RpcException) : void;
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H11_RpcReservedType_FunctionArgument_RpcException.txt).

### H12: Reserved RPC event argument type

```Text
H12: @rpc:Interface cannot be applied to interface "{interface}" because its event argument "{member}" uses reserved RPC type "{type}".
```

```Workflow
module test;
using system::*;

@rpc:Interface
interface IMy
{
  event E(system::RpcException);
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/H12_RpcReservedType_EventArgument_RpcException.txt).

## I: RPC wrapper generation

### I0: Missing RPC property mode

```Text
I0: RPC wrapper generation requires property "{member}" to have @rpc:Cached or @rpc:Dynamic.
```

Example context: an RPC wrapper generator receives an interface property named Service.Count with neither @rpc:Cached nor @rpc:Dynamic in its metadata. Normal RPC analysis supplies a default property mode before wrapper generation; this error identifies incomplete generator input, rather than requiring every source property to write the default explicitly.

[Compiler check](https://github.com/vczh-libraries/Workflow/blob/master/Source/Analyzer/Rpc/WfAnalyzer_GenerateRpc.cpp).

### I1: Missing RPC return transfer mode

```Text
I1: RPC wrapper generation requires method "{member}" to have @rpc:Byval or @rpc:Byref on its collection return value.
```

Example context: wrapper input contains func Read():int[] without either @rpc:Byval or @rpc:Byref on its return value. Normal RPC analysis adds the default transfer mode before this stage; manually supplied or modified metadata must preserve it.

[Compiler check](https://github.com/vczh-libraries/Workflow/blob/master/Source/Analyzer/Rpc/WfAnalyzer_GenerateRpc.cpp).

### I2: Missing RPC parameter transfer mode

```Text
I2: RPC wrapper generation requires parameter "{member}" to have @rpc:Byval or @rpc:Byref because it is a collection parameter.
```

Example context: wrapper input contains func Write(values:int[]):void without either @rpc:Byval or @rpc:Byref on values. Normal RPC analysis adds the default transfer mode before this stage; manually supplied or modified metadata must preserve it.

[Compiler check](https://github.com/vczh-libraries/Workflow/blob/master/Source/Analyzer/Rpc/WfAnalyzer_GenerateRpc.cpp).

### I3: Ambiguous generated RPC identifier

```Text
I3: RPC mangled name "{identifier}" is ambiguous between "{previousName}" and "{currentName}".
```

```Workflow
module test;
namespace A
{
  @rpc:Interface
  interface B {}
}
@rpc:Interface
interface A__B {}
```

During RPC wrapper generation, the distinct full names A::B and A__B both become the identifier A__B. Avoid names that collide after :: becomes __ and punctuation in generated member names becomes _.

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/I3_RpcMangledInterfaceNameConflict.txt).

## J: Variants

### J0: Invalid variant type

```Text
J0: Invalid variant type. {reason}
```

The reason gives the violated variant-type rule. This example has no options.

```Workflow
module test;
using system::*;

func f(x:variant<>):void {}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/J0_VariantNoOptions.txt).

### J1: Invalid variant expression

```Text
J1: Invalid variant expression. {reason}
```

The reason gives the violated variant-expression rule. Here the constructor has no exact option type; construct an int[] first and explicitly convert it to the variant.

```Workflow
module test;
using system::*;

var x:variant<int[], string> = {};
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/J1_VariantConstructorSelection.txt).

## CPP: C++ generation

### CPP1: Impossible C++ class order

```Text
CPP1: (C++ Code Generation) Cannot decide order of the following classes. It is probably caused by inheritance relationships of internal classes inside these classes:{candidates}.
```

This error appears when generating C++, after ordinary Workflow analysis succeeds.

```Workflow
module test;
using system::*;

class A
{
    class Inside : B {}
}

class B
{
    class Inside : A {}
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/CPP1_InternalClassCycleInheritance.txt).

### CPP2: Incompatible C++ file grouping

```Text
CPP2: (C++ Code Generation) @cpp:File atrribute values for these classes are invalid. Generating classes to source files specified by these attribute values will create source files which do not compile. It is probably caused by inheritance relationships of internal classes inside these classes:{candidates}.
```

This error appears when generating C++. The two declarations assigned to Custom cannot be emitted together around the intervening base class.

```Workflow
module test;
using system::*;

@cpp:File("Custom")
class A
{
}

class B : A
{
}

@cpp:File("Custom")
class C : B
{
}
```

[Example test](https://github.com/vczh-libraries/Workflow/blob/master/Test/Resources/AnalyzerError/CPP2_NonCustomFileClassForceIntoCustomFile.txt).

