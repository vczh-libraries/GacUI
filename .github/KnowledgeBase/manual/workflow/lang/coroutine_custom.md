# Design Your Own Coroutine

A coroutine provider is a class with methods that following a naming convention.

A coroutine provider could be written in either C++ with reflection, or just Workflow.

## Naming your coroutine

If you want a coroutine to be written in this way:
```
func MyCoroutine(): void
$Foo {
    /* ... */
}
```
you need to define the coroutine provider like this:
```
using system::*;

class FooCoroutine
{
    class InternalState
    {
        /* ... */
    }

    static CreateAndRun(creator: func(InternalState*): (Coroutine^)): void
    {
        /* ... */
    }
}
```


If you want a coroutine to be written in this way:
```
func MyCoroutine(): Foo^
${
    /* ... */
}
```
or
```
$interface StringFoo : Foo<string>;
func MyCoroutine(): StringFoo^
${
    /* ... */
}
```
you need to define the coroutine provider like this:
```
using system::*;

interface Foo
{
    /* ... */
}

class FooCoroutine
{
    class InternalState
    {
        /* ... */
    }

    static Create(creator: func(InternalState*): (Coroutine^)): Foo^
    {
        /* ... */
    }
}
```



- **FooCoroutine::InternalState**could be a class or an interface.
- **Create**and**CreateAndRun**could both appear. This makes coroutine functions being able to return both**void**and**Foo^**.
- The return type of**Create**must be compatible with the return type of coroutine functions:
  - If**Create**returns**Foo^**, then coroutine functions could return**Foo^**or any of its base type.
  - If**Create**returns**Foo^**, then coroutine functions could return a generic version of**Foo^**.
- The only argument of**Create**or**CreateAndRun**should be a function:
  - It takes a raw pointer of an object for storing internal states.
  - It must return**Coroutine^**.

When a coroutine function is called, either**Create**or**CreateAndRun**will be called. You are responsible to create the object for storing internal states, and then a**Coroutine^**will be given to you.

The coroutine function takes and stores the internal state object. When any method in the coroutine provider is called because of coroutine providers,**return**or**$**, this object will be used as the first argument.

Nothing in the coroutine function is executed before the first call to**Coroutine::Resume**.

Whenever a coroutine operation is called, the coroutine pauses, and another call to**Coroutine::Resume**is required to continue the coroutine function.

After**Coroutine::Resume**is called,**Coroutine::Failure**and**Coroutine::Status**must be read in order to correctly handle coroutine operations.

## Define coroutine operations

If you want to support
```
$Do1;
$Do2 something;
$Do3 something, otherthings;
```
you are required to add the following methods to your coroutine provider:
```
static Do1AndPause(internalState: InternalState*): void { /* ... */ }
static Do2AndPause(internalState: InternalState*, something: Something): void { /* ... */ }
static Do3AndPause(internalState: InternalState*, something: Something, otherthings: Otherthings): void { /* ... */ }
```


If you want to support both
```
$Do1;
$Do2 something;
$Do3 something, otherthings;
```
and
```
var x = $Do1;
var y = $Do2 something;
var z = $Do3 something, otherthings;
```
you are required to add the following methods to your coroutine provider:
```
static Do1AndRead(internalState: InternalState*): void { /* ... */ }
static Do2AndRead(internalState: InternalState*, something: Something): void { /* ... */ }
static Do3AndRead(internalState: InternalState*, something: Something, otherthings: Otherthings): void { /* ... */ }
```


Overloading is allowed.

When a coroutine operation is called, one of these methods will be called, and the created**Coroutine^**will pause right after that.

All methods for coroutine operations must return**void**. After the coroutine is resumed, the return value from the coroutine operation comes from the second argument in**Coroutine::Resume**.

This argument is a**CoroutineResult^**.
- If the coroutine operation succeeds, the return value is stored in the**Result**property.
- If the coroutine operation succeeds, the exception is stored in the**Failure**property.
- If**Failure**is not null,**Result**will be ignored. and an exception is thrown. If there is no**try-catch**in this coroutine function, the coroutine stops immediately with this exception.

## Define the return statement

If you want to support
```
return;
```
you are required to add this method to your coroutine provider:
```
static ReturnAndExit(internalState: InternalState*): void { /* ... */ }
```


If you want to support
```
return something;
```
you are required to add this method to your coroutine provider:
```
static ReturnAndExit(internalState: InternalState*, something: Something): void { /* ... */ }
```


Both methods could appear together.

When**return**is called,**ReturnAndExit**will be called, and the coroutine stops immediately.

## Define the $ expression

If you want to suppor the**$**expression, you are required to add this method to your coroutine provider:
```
static QueryContext(internalState: InternalState*): Something { /* ... */ }
```
and the return value from**$**becomes**Something**.

## Example

Let's see the example[here](../.././workflow/lang/interface_using.md).

The**ReaderCoroutine**defines:
- **$Set values;**: Stores a collection of values.
- **var x = $Get;**: Read a value from the stored collection, returns**null**after all values are read.
- **return value;**: Finish the coroutine with a result.

After the coroutine pauses, you need to know which coroutine operation is called. In this example, the**readObject**field in**ReaderCoroutine::Executer**is used for this purpose.

In**ReaderCoroutine::Executor::Continue**, when**readObject**is**true**, it means**$Get**is called. The default value of**cr.Result**is**null**. If all values are read, the assignment is not executed, so**$Get**returns**null**.

If**$Get**is called before**$Set**, it crashes because the**enumerator**field is**null**.

After that,**co.Resume(false, cr);**is called, this is how the return value from**$Get**is specified.

Finally**Sum**is called, it first stores 10 numbers, and then read them out one by one, and add them together.

In the**main**function, it keeps calling**ReaderCoroutine::Executor::Continue**until the whole coroutine function finishes. The return value is read from the**result**field, because**ReturnAndExit**use this field to store the value given to the return statement.

By executing**main**, methods in**ReaderCoroutine**is called in the following order:
- **Create**
- **Executor::Continue**
- **SetAndPause**
- Call 10 times:
  - **Executor::Continue**
  - **GetAndRead**
- **Executor::Continue**
- **ReturnAndExit**

