# Running Workflow Scripts

Involved files:
- [Main.cpp](https://github.com/vczh-libraries/Release/blob/master/Tutorial/Console_Workflow/W01_RunningWorkflowScripts/Main.cpp)

Running a Workflow script is quite simple, just start the reflection, compile the code, and call the function you need.

A typical Workflow script needs to use primitive types and a very simple predefined Workflow library. Both compiling and running a Workflow script needs type information from above types. In order to load these types, two functions need to be called:
```
LoadPredefinedTypes();
WfLoadLibraryTypes();
GetGlobalTypeManager()->Load();
```


Now, we begin to run this Workflow script:
```
module sampleModule;

func main(): string
{
    return "Hello, world!";
}
```


**WfLoadTable**function creates the parser table for parsing Workflow scripts. Loading this table takes times, and it takes a few seconds when debugging your host C++ application. So it is recommended to cache the table.

Firstly we need to compile the code and get the assembly:
```
List<WString> codes;
codes.Add(WString(ScriptCode, false))

List<Ptr<ParsingError>> errors;
auto table = WfLoadTable();
auto assembly = Compile(table, codes, errors);
```
If it fails to compile, you will get a null assembly, and the**errors**variable stores all mistakes that are found in the script. Note that the compiler stops at the first syntax error.

Secondly we need to create the runtime environment for executing the script:
```
auto globalContext = Ptr(new WfRuntimeGlobalContext(assembly));
auto initializeFunction = LoadFunction<void()>(globalContext, L"<initialize>");
initializeFunction();
```
Only one**WfRuntimeGlobalContext**is allowed to create per assembly. In most cases you cannot load the assembly from file twice in order to get two assembilies and two context for separate execution environment. Because Workflow scripts could create classes and interfaces, all of them will also be registered to the global type manager, just like C++ reflection. Loading them twice will result in an error.

The next step is to initialize global variables. In a Workflow script, global variables should be initialized with default values, but by loading the assembly these variables are not initialized. So you always need to run the exported function**\<initialize\>**to do it.

Now everything is prepared, we can call the**main**function to get the result:
```
auto mainFunction = LoadFunction<WString()>(globalContext, L"main");
Console::WriteLine(mainFunction());
```
Unlike C++, an Workflow script is not required to have a**main**function. Here**main**is just a trivial name.

