# Async Coroutine

This coroutine provider is **system::AsyncCoroutine**. - It has a **Create** method, so this coroutine function declaration is **legal**: ``` using system::*; func GetNumbers(): Async^ ${ /* legal */ } ``` - It has a **CreateAndRun** method, so this coroutine function declaration is **legal**: ``` using system::*; func GetNumbers(): void $Async{ /* legal */ } ``` - It does not have a **AwaitAndPause** method, but it has a **AwaitAndRead** method, so both statements in the coroutine are **legal**: ``` $Await EXPR; /* legal */ var NAME = $YieAwaitld EXPR; /* legal */ ``` - It has a **ReturnAndExit** method, which has a parameter, so this statement in the coroutine is **illegal**: ``` $return; /* illegal */ ``` but this statement in the coroutine is **legal**: ``` $return EXPR; /* legal */ ``` - It has a **QueryContext** method, which has no parameter, so the **$** expression is available in the coroutine.

## Using return

**return EXPR;** stops a **$Async** coroutine and generates a return value.

If there is a **static func StoreResult** in the return type of the function, the return value becomes **TYPE::StoreResult(EXPR)**.

If this **$Async** does not have a meaningful return value, like when the coroutine function returns **void** or **system::Async^**, it should return **null**.

Since a function exists without a **return** statement also returns **null**, so the **return** statement is not always necessary.

``` module sampleModule; using system::*; func Do1(): void $Async { return null; } func Do2(): Async^ ${ return null; } $interface StringAsync : Async<string>; func Do3(): StringAsync^ ${ return "Text"; } ``` - **Do1** creates an **system::Async^** that will eventually generates **null**, because this function returns **void**. Calling **Do1** results in executing the created **system::Async^** immediately, but since it returns **void**, so the created **system::Async^** cannot be retrived. - **Do2** creates an **system::Async^** that will eventually generates **null**, because this function returns **system::Async^**. Calling **Do1** gets the created **system::Async^**, but it is not automatically executed. - **Do3** creates an **StringAsync^** that will eventually generates **"Text"**. The return type **StringAsync^** has a **static func StoreResult**, having one argument of **string**. This cause the compiler to generate an error when the return value cannot be implicitly converted to **string**.

## Using $Await

**$Await AnotherTask;** executes **AnotherTask** and pauses the current **$Async** coroutine. It will be resumed when **AnotherTask** finishes.

**var NAME = $Await AnotherTask;** executes **AnotherTask** and pauses the current **$Async** coroutine. It will be resumed when **AnotherTask** finishes, and the return value from **AnotherTask** will be stored in **NAME**.

## Using the $ expression

The **$** expression in a **$Async** coroutine returns **system::AsyncContext^**.

When an **system::Async^** is executed outside from a **$Async** coroutine, an **system::AsyncContext^** is required to be passed to the **Async::Execute** method.

It could be null, which is also the default value for this parameter.

**$Await** will pass this **system::AsyncContext^** object to all nested **system::Async^** values.

The **$** expression returns this particular **system::AsyncContext^** object, which has two members: - **func Cancel(): bool**, makes **$.IsCancelled()** returns **true**. - **func IsCancelled(): bool** is designed to be called in any **$Async** coroutine. If a **$Async** coroutine has many steps, it could check **$.IsCancelled()** frequently and stops the coroutine when it is required. This does not happen automatically, a **$Async** can ignore **$.IsCancelled()**.

## Executing system::Async^

**system::Async::Delay** function creates a task that ends after a specific period of time.

**system::Async::Execute** executes an task in a background thread. The first argument is a callback that will be triggered when the task is end. The second argument is an optional object that becomes **$** in a **$Async** coroutine.

If **system::Async::Execute** is not called for the first time for that particular object, it fails and returns **false**.

Calling a **$Async** function which returning **void**, result in starting the task immediately.

## Hosting system::Async^

Calling **system::AsyncScheduler::RegisterDefaultScheduler** or **system::AsyncScheduler::RegisterSchedulerForCurrentThread** are required before executing a task in a particular thread.

You are recommended to call **system::AsyncScheduler::UnregisterDefaultScheduler** or **system::AsyncScheduler::UnregisterSchedulerForCurrentThread** when no task is needed to run in a particular thread.

**system::Async::Execute** use the caller thread's scheduler to run itself. If **RegisterSchedulerForCurrentThread** is called for this thread, it will not use the scheduler that is registered by **RegisterDefaultScheduler**.

Usually this interface is implemented in C++, which should use **vl::reflection::description::IAsyncScheduler** as the name of the interface.

There are 3 methods that need to be implemented: - `empty list item` **Execute**: This function runs a callback in any thread it likes. This function is called after **$Await** finishes waiting. For GacUI, the scheduler for the UI thread is different from other thread, because **Execute** of the UI thread's scheduler runs callbacks in the UI thread. This means that, for any **$Async** that is called from the UI thread, all code will eventually be executed in the UI thread, including tasks that are awaited using **$Await**. If a task don't want to be executed in the UI thread even when it is scheculed by the UI thread, it needs to explicitly call **AsyncScheduler::ExecuteInBackground**, which means thiat task cannot be implemented using coroutine. - `empty list item` **ExecuteInBackground**: This function runs a callback in any thread it likes. It will not be called by generated code from **$Async** coroutines. For GacUI, **ExecuteInBackground** runs callbacks in a random non-UI thread. It doesn't matter whether this scheduler is for the UI thread or not. - `empty list item` **DelayExecute**: This function runs a callback in any thread it likes after a specified period of time. It will not be called by generated code from **$Async** coroutines. For GacUI, the scheduler for the UI thread is different from other thread, because **DelayExecute** of the UI thread's scheduler runs callbacks in the UI thread.

**IMPORTANT**: There is no default scheduler provided by Workflow script. If you don't implement any scheduler, **$Async** coroutine fails.

## Example

Let's see an example of a scheduler. **$Async** doesn't necessary mean multi-threading, everything could be synchronized. The following scheduler queued all tasks that will be executed on by one when it is required.

``` class SyncScheduler : public Object, public IAsyncScheduler, public Description<SyncScheduler> { public: List<Func<void()>>		tasks; void Run(const Func<void()>& callback) { auto scheduler = Ptr(new SyncScheduler); IAsyncScheduler::RegisterDefaultScheduler(scheduler); callback(); while (scheduler->tasks.Count() > 0) { auto firstTask = scheduler->tasks[0]; scheduler->tasks.RemoveAt(0); firstTask(); } IAsyncScheduler::UnregisterDefaultScheduler(); } void Execute(const Func<void()>& callback) override { tasks.Add(callback); } void ExecuteInBackground(const Func<void()>& callback) override { tasks.Add(callback); } void DelayExecute(const Func<void()>& callback, vint milliseconds) override { // the required waiting time is ignored tasks.Add(callback); } }; ``` Assume that this class is registered using this name: **test::SyncScheduler**.

By calling **SyncScheduler::Run**, the scheduler registers itself as a global task scheduler, executes the callback in the caller's thread, executes all generated new tasks in the caller's thread until they are all cleared, unregisters itself.

The following Workflow script creates some **Async** objects by using coroutines or implementing the interface, and then call **SyncScheduler::Run** to run all code. ``` module sampleModule; using system::*; using test::*; var s = ""; $interface IStringAsync : Async<string>; func GetStringAsync(x : int) : IStringAsync^ ${ s = $"$(s)[$(x)]"; return cast string x; } func GetStringAsync2(x : int) : IStringAsync^ { var future = Future::Create(); AsyncScheduler::GetSchedulerForCurrentThread().ExecuteInBackground(func():void { s = $"$(s)[$(x)]"; future.Promise.SendResult(cast string x); }); return new IStringAsync^(using future of Async^); } func GetMultipleStrings() : void $Async{ for(i in range[0, 1]) { var x = $Await GetStringAsync(i); s = $"$(s)[+$(x)]"; var y = $Await GetStringAsync2(i); s = $"$(s)[-$(y)]"; $Await Async::Delay(0); s = $"$(s)!"; } } func main():string { SyncScheduler::Run(GetMultipleStrings); return s; } ``` - The **main** main function starts the task scheduler, asking it to call **GetMultipleStrings**. - **GetMultipleStrings** calls both **GetStringAsync** and **GetStringAsync2** for twice. - **GetStringAsync** implements **Async** using coroutine. - **GetStringAsync2** implements **Async** using **Future** and **Promise**. - Eventually **"[0][+0][0][-0]![1][+1][1][-1]!"** is written to **s**, which is also the return value from **main**. The difference between **GetStringAsync** and **GetStringAsync2** is that, **GetStringAsync** starts the task by calling **AsyncScheduler::Execute**, while **GetStringAsync2** starts the task by calling **AsyncScheduler::ExecuteInBackground**. Both scheduler functions are the same in this example.

