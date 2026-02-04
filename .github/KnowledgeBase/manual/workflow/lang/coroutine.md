# Coroutines

There are two kinds of coroutines:
- Raw Coroutine
- Coroutine Function

Coroutine creates a piece of code that can be paused and resumed as expected, so that this piece of code could receive multiple inputs and produce multiple outputs.

## Raw Coroutine

Raw coroutine is an expression which looks like:
```
$coroutine{ ... }
```
This expression returns**system::Coroutine^**.

See[Raw Coroutine](../.././workflow/lang/coroutine_raw.md).

## Coroutine function

Coroutine function (including lambda expressions) is written in these forms:
```
func ... : void
$NAME {
    ...
}
```
or
```
func ... : Something
$NAME {
    ...
}
```
or
```
func ... : Something
${
    ...
}
```


There are two predefines coroutine in Workflow:
- [$Async](../.././workflow/lang/coroutine_async.md)
- [$Enumerable](../.././workflow/lang/coroutine_enumerable.md)

## Determine the coroutine provider for a coroutine function

Consider the following coroutine function:
```
func ... : TYPE
$NAME {
    ...
}
```

- When**$NAME**has a name, the coroutine provider is determined by the coroutine name**That**.
- When**$NAME**does not has a name (written as**$**), the coroutine provider is determined by the return type**TYPE**. It generates an error when**TYPE**is**void**.

### Searching by return type

Consider the following coroutine:
```
module sampleModule;

using system::*;

$interface StringAsync : Async<string>;

func DoAsync(): StringAsync
$ {
    ...
}
```


The coroutine provider to use is determined like this:
- The coroutine does not have a name, because the function body starts with**${**, so the coroutine is searched by return type.
- The return type is**StringAsync^**, list this type with all parent types in the breadth-first order.
  - **::StringAsync**
  - **::system::Async**
  - **::system::Interface**
- Append**"Coroutine"**after the type name:
  - **::StringAsyncCoroutine**
  - **::system::AsyncCoroutine**
  - **::system::InterfaceCoroutine**
- The first existing type is**::system::AsyncCoroutine**, pick this one.

### Searching by coroutine name

Consider the following coroutine:
```
module sampleModule;

using system::*;

func DoAsync(): void
$Async {
    ...
}
```


The coroutine provider to use is determined like this:
- The coroutine has a name, it is**Async**.
- Search for**Async**in the context, we get**::system::Async**.
- Append**"Coroutine"**after the type name:**::system::AsyncCoroutine**.
- It exists, pick this one.
- If the previous step fails, search for**AsyncCoroutine**in the context.

