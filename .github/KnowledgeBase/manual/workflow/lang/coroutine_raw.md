# Raw Coroutine

A raw coroutine expression doesn't involve any coroutine provider, it creates a **system::Coroutine^**. In a raw coroutine, you can pause itself whenever you want, and exchange data with the caller.

## Create a raw coroutine

Here is an example: ``` module sampleModule; using system::*; func main(): string { var r = {""}; var c = $coroutine(arg) { for (i in range [1, 3]) { r[0] = $"$(r[0])$(cast string arg.Result);"; $pause; } raise "Something happened!"; }; var counter = 0; while (c.Status != CoroutineStatus::Stopped) { counter = counter + 1; var cr = new CoroutineResult^(); cr.Result = counter; c.Resume(false, cr); } if (c.Failure is not null) { r[0] = $"$(r[0])$(c.Failure.Message)"; } return r[0]; } ``` The variable **r** is captured by shallow copying, so we use **string[]** instead of **string** to make it modifiable.

In the **main** function, a raw coroutine is created. It exchanges data with the caller 3 times by calling **$pause;**, and eventually throws an exception.

When the raw coroutine is created, nothing is executed. **system::Coroutine::Resume** needs to be called to start or continue the coroutine. - The first argument controls whether **Resume** throws exceptions. If an exception is thrown while executing a coroutine, the **Status** of the coroutine becomes **Stopped**, and **Failure** stores the exception. If the first argument is **true**, the exception will also be thrown. - The second argument becomes **arg** in the raw coroutine, which is declared here: **$coroutine(arg)**. The name is trivial. The type of **arg** is always **system::CoroutineResult^**.

The coroutine calls **$pause** 3 times, it means that **Resume** needs to be called for 4 times to finish the execution.

At the end, the coroutine throws an exception, causing **Status** to become **Stopped**, which stops the **while** loop in the caller.

The exception and then retrived using the **Failure** property.

Since the first argument given to **Resume** is **false**, so the exception is not actually thrown.

The **main** function returns **"1;2;3;Something happened!"**.

## Calling coroutine providers in raw coroutines

Let's see another example. Previous we creates **system::Enumerable^** using a **$Enumerable** coroutine like this: ``` module sampleModule; using system::*; func GetNumbers() : int{} ${ for (i in range [1, 10]) { $Yield i; } } func main(): string { var r = ""; for (i in GetNumbers()) { r = $"$(r)$(i); "; } return r; } ``` The **main** function returns **"1; 2; 3; 4; 5; 6; 7; 8; 9; 10; "**.

By calling **system::EnumerableCoroutine** directly, we are able to do the same thing using a raw coroutine: ``` module sampleModule; using system::*; func GetNumbers() : int{} { return EnumerableCoroutine::Create( [$coroutine{ for (i in range [1, 10]) { EnumerableCoroutine::YieldAndPause($1, i); $pause; } }] ) as int{}; } func main(): string { var r = ""; for (i in GetNumbers()) { r = $"$(r)$(i); "; } return r; } ``` The **main** function returns **"1; 2; 3; 4; 5; 6; 7; 8; 9; 10; "**.

The argument to **Create** is an ordered lambda expression. It has one argument, which is the first argument for every coroutine operator functions.

The type of this argument veries in different coroutine providers.

For **$Async**, **$** becomes **system::AsyncCoroutine::QueryContext($1)** here.

In this coroutine, we don't need the **arg** argument as in the previous example, because no data is comming in, data only going out.

