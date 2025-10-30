# Generic (interface)

Interfaces do not actually support generic, but sometimes it works like generic in [coroutines](../.././workflow/lang/coroutine.md).

Here is an example of a coroutine that created by Workflow: ``` module sampleModule; using system::*; interface Reader { } $interface IntReader: Reader<int?>; $interface StringReader: Reader<string?>; func Read(): Reader^ { return new Reader^{}; } func ReadInt(): IntReader^ { return new IntReader^(using Read()); } func ReadString(): StringReader^ { return new StringReader^(using Read()); } class ReaderCoroutine { class Executor { var readObject: bool = false; var source: Enumerable^ = {}; var enumerator: Enumerator^ = null; var co: Coroutine^ = null; var result: object = null; func Continue(): bool { if (co.Status == CoroutineStatus::Stopped) { return false; } var cr = new CoroutineResult^(); if (co.Failure is not null) { cr.Failure = co.Failure; } else if (readObject and enumerator is not null) { if (enumerator.Next()) { cr.Result = enumerator.Current; } } co.Resume(false, cr); return true; } } static func SetAndPause(impl: Executor*, numbers: Enumerable^): void { impl.source = numbers; impl.enumerator = numbers.CreateEnumerator(); impl.readObject = false; } static func GetAndRead(impl: Executor*, reader: Reader^): void { impl.readObject = true; } static func ReturnAndExit(impl: Executor*, value: object): void { impl.result = value; } static func Create(creator: func(Executor*): (Coroutine^)): ReaderCoroutine::Executor^ { var e = new Executor^(); e.co = creator(cast Executor* e); return e; } } func Sum(): ReaderCoroutine::Executor^ $Reader{ $Set range [1, 10]; var sum = 0; while (true) { var current = $Get ReadInt(); if (current is null) { break; } sum = sum + current; } return sum; } func main(): string { var e = Sum(); while (e.Continue()) {} return cast string e.result; } ``` The **Sum** function does: - calls **$Set** to set a number collection. - calls **$Read** to read numbers one by one. - returns the sum of all numbers. The **main** function gets the coroutine created from **Sum**, drives the coroutine to run until a result is produced.

## $interface IDerived: IBase\<TYPE\>;

In the example there is: ``` $interface IntReader: Reader<int?>; $interface StringReader: Reader<string?>; ``` which expands to: ``` interface IntReader : Reader { static func CastResult(value : ::system::Object) : (int?) { return (cast (int?) value); } static func StoreResult(value : int?) : (::system::Object) { return value; } } ``` So as: ``` $interface StringReader: Reader<string?>; ```

**$interface** simply creates an interface which inherits from the one that is specified, and add static functions **CastResult** and **StoreResult** for casting values between **object** and the generic argument that is specified.

This is how **$Get** knows the type of the return value. obviously in: ``` var current = $Get ReadInt(); ``` the type of **current** is **int?**, because **ReadInt** returns **IntReader^**, in which the **CastResult** function returns **int?**.

It doesn't matter what the original interface looks like.

## new TYPE^(using OBJECT)

This expression creates an object of interface **TYPE**, redirecting all methods to **OBJECT**.

It needs to meet the following requirements: - **TYPE** and the type of **OBJECT** should both be interfaces. - **TYPE** must be different from the type of **OBJECT**. - **TYPE** must inherit from the type of **OBJECT**. It doesn't have to be the direct base type. - **TYPE** must not have more members than the type of **OBJECT**.

**new TYPE*** is also available, according to how **TYPE** is declared.

For example: ``` interface Reader { func Dummy(): void; } $interface IntReader: Reader<int?>; func ReadInt(): IntReader^ { return new IntReader^(using Read()); } ``` **ReadInt** expands to: ``` func ReadInt() : (IntReader^) { return new (IntReader^) { var source = Read(); override func Dummy() : (::system::Void) { source.Dummy(); } }; } ``` It is clear that, if **OBJECT** is a raw pointer, then the created object holds a raw pointer to **OBJECT**. In this case, you must be very careful to object life cycles.

