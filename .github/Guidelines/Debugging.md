## Debugging a Project

Debugging would be useful when you lack necessary information.
In this section I offer you a set of PowerShell scripts that work with CDB (Microsoft's Console Debugger).
CDB accepts the exact same commands as WinDBG.

### Start a Debugger

Read `REPO-ROOT/.github/Project.md` to understand the solution folder and the unit test project name you are working with.
Additional information could be found in THE FIRST LINE in `REPO-ROOT/.github/Scripts/Execute.log`.
Execute the following PowerShell commands:

```
cd SOLUTION-ROOT
start powershell {& REPO-ROOT\.github\Scripts\copilotDebug_Start.ps1 -Executable PROJECT-NAME}
```

The `start powershell {}` is necessary; otherwise the script will block the execution forever causing you to wait infinitely.
The script will finish immediately, leaving a debugger running in the background. You can send commands to the debugger.
The process being debugged is paused at the beginning, you are given a chance to set breakpoints.
After you are prepared, send the `g` command to start running.

### Stop a Debugger

You must call this script to stop the debugger.
Do not stop the debugger using any command.
This script is also required to run before compiling only when Visual Studio Code tasks are not available to you.

```
& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
```

If there is any error message, it means the debugger is not alive, it is good.

### Sending Commands to Debugger

```
& REPO-ROOT\.github\Scripts\copilotDebug_RunCommand.ps1 -Command "Commands"
```

The effect of commands lasts across multiple `copilotDebug_RunCommand.ps1` calls. For example, after you executed `.frame X`, you do not need to repeat it to use `dx` under the same call stack frame in later calls, as `.frame X` is already effective.

Multiple commands can be executed sequentially separated by ";".
The debugger is configured to be using source mode, which means you can see source files and line numbers in the call stack, and step in/out/over are working line by line.
CDB accepts exactly same commands as WinDBG, and here are some recommended commands:
- **g**: continue until hitting a breakpoint or crashing.
- **k**n: print current call stack.
- **kn LINES**: print first `LINES` of the current call stack.
- **.frame NUMBER**: inspect the call stack frame labeled with `NUMBER`. `kn` will show the number, file and line along with the call stack.
- **dv**: list all available variables in the current call stack frame.
- **dx EXPRESSION**: evaluate the `EXPRESSION` and print the result. `EXPRESSION` can be any valid C programming language expression. When you specify a type (especially when doing casting), full namespaces are required, do not start with `::`.
- **bp `FILE:LINE`**: set a breakpoint at the specified line in `FILE`, starting from 0. A pair of "`" characters are required around the target, this is not a markdown syntax.
- **bl**, **.bpcmds**, **be NUMBERS**, **bd NUMBERS**, **bc NUMBERS**, **bsc NUMBER CONDITION**: list, list with attached commands, enable, disable, delete, attach a command to breakpoint(s).
- **p**: step over, aka execute the complete current line.
- **t**: step in, aka execute the current line, if any function is called, goes into the function.
- **pt**: step out, aka run until the end of the current function.

An `.natvis` file is automatically offered with the debugger,
it formats some primitive types defined in the `Vlpp` project,
including `WString` and other string types, `Nullable`, `Variant`, container types, etc.
The formatting applies to the **dx** command,
when you want to see raw data instead of formatted printing,
use **dx (EXPRESSION),!**.

You can also use `dv -rX` to expand "X" levels of fields, the default option is `-r0` which only expands one level of fields.

### Commands to Avoid

- Only use **dv** without any parameters.
- DO NOT use **dt**.
- DO NOT use **q**, **qd**, **qq**, **qqd** etc to stop the debugger, always use `copilotDebug_Stop.ps1`.
