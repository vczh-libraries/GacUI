# Debugging

Involved files:
- [W06_Debugging/Main.cpp](https://github.com/vczh-libraries/Release/blob/master/Tutorial/Console_Workflow/W06_Debugging/Main.cpp)

It is able to debug a running Workflow script, if it is not converted to C++. In this section, we will use the debugger to debug this Workflow script:
```
module sampleModule;
using myapi::*;

func main(): void
{
    var name = App::Get("Please enter your name:");
    App::Print($"Hello, $(name)!");
}
```
We will set a break point at**App::Print**, and change the**name**variable, and continue running the script.

Firstly, after initializing the assembly, we start debugging:
```
auto debugger = Ptr(new MyDebugger);
SetDebuggerForCurrentThread(debugger);
```
**MyDebugger**inherits**WfDebugger**, it will be explained later.

Secondly, we set a break point:
```
debugger->AddCodeLineBreakPoint(assembly.Obj(), 0, 9, true);
```
"0" means the first file, and "9" means the 10th line, which is**App::Print($"Hello, $(name)!");**.

We run the script as usual. After the**main**function is executed, we stop debugging:
```
auto mainFunction = LoadFunction<void()>(globalContext, L"main");
mainFunction();
SetDebuggerForCurrentThread(nullptr);
```


Now, before**App::Print**is called, the debugger will be notified. Please note that, after a break point is triggered, if**WfDebugger::Run**or other similar functions are not called,**mainFunction() will be blocked forever.**

In order to take action when a break point is triggered, we need to override**WfDebugger::OnBlockExecution**like this:
```
class MyDebugger : public WfDebugger
{
protected:
    void OnBlockExecution()override
    {
        // get the context for the current thread
        auto context = GetCurrentThreadContext();

        // whatever the input is, change the "name" variable to be "MyDebugger"
        // it will print "Hello, MyDebugger!" after continuing
        auto assembly = context->globalContext->assembly;
        vint functionIndex = context->GetCurrentStackFrame().functionIndex;
        auto functionMetadata = assembly->functions[functionIndex];
        vint indexName = functionMetadata->localVariableNames.IndexOf(L"name");
        context->StoreLocalVariable(indexName, BoxValue(WString(L"MyDebugger")));

        // continue
        Run();
    }
};
```


Details of Workflow runtime API will not be explained here. Basically what this function does is that, we first get the thread context that we are currently debugging against, and then use the metadata in the assembly to know how to change the**name**variable. After it is done, continue running the script.

Since we only set one break point, so we don't need to tell which break point is triggered in**OnBlockExecution**.

Here is what we need to know when debugging Workflow scripts:
- Threads are by default not assigned with any debugger.
- A debugger is activated when it is assigned to a specific thread.
- The same debugger can be assigned to multiple threads.
- Each call to a function returned from**LoadFunction**creates a separate**WfRuntimeThreadContext**object. Do not assume that you will always get the same**WfRuntimeThreadContext**object per thread.

