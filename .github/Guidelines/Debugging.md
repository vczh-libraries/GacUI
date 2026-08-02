# Debugging a Project

- Go to `Windows Specific` section if you are on Windows.
- Go to `Linux/macOS Specific` section if you are on Linux/macOS.
- Go to `macOS Specific` section if you are on macOS.

## Windows Specific

Debugging can be useful when you lack necessary information.
CDB accepts exactly the same commands as WinDBG.

### Start a Debugger

Read `REPO-ROOT/Project.md` to understand the solution folder and the unit test project name you are working with.
Additional information can be found in the first line of `REPO-ROOT/.github/Scripts/Execute.log`.
Choose the executable for the intended configuration and read its arguments from the matching `*.vcxproj.user` file.

Run CDB directly from `SOLUTION-ROOT` in a PTY-backed tool session:

```powershell
cd SOLUTION-ROOT
& $env:CDBPATH -lines "ABSOLUTE-PATH-TO-EXECUTABLE" ARGUMENTS
```

Keep the session ID returned by the tool. The process is paused at the initial breakpoint, giving you a chance to configure source mode and set breakpoints. Send these commands as newline-terminated stdin, one round at a time, and wait for output between rounds:

```text
l+s;l+t
g
```

Do not leave unbounded debugger output flowing through the terminal. If output repeats continuously, interrupt execution immediately and configure the relevant exception or output filter before continuing.

### Stop a Debugger

- If the debugged process is running or stuck, send Ctrl-C (`\u0003`).
- Send `q` to terminate the debugged process and CDB.
- If CDB does not respond, terminate the exact CDB process first and then the exact debugged process.

### Sending Commands to CDB

Send commands to the same PTY-backed session. The effect of commands lasts for the whole session. For example, after you execute `.frame X`, you do not need to repeat it to use `dx` under the same call stack frame later.

Multiple commands can be executed in sequence, separated by ";".
The `-lines` option and the `l+s;l+t` commands enable source mode, which means you can see source files and line numbers in the call stack, and step in/out/over work line by line.
CDB accepts exactly the same commands as WinDBG, and here are some recommended commands:
- **g**: continue until hitting a breakpoint or crashing.
- **k**n: print current call stack.
- **kn LINES**: print first `LINES` of the current call stack.
- **.frame NUMBER**: inspect the call stack frame labeled with `NUMBER`. `kn` will show the number, file and line along with the call stack.
- **dv**: list all available variables in the current call stack frame.
- **dx EXPRESSION**: evaluate the `EXPRESSION` and print the result. `EXPRESSION` can be any valid C programming language expression. When you specify a type (especially when doing casting), full namespaces are required, do not start with `::`.
- **bp `FILE:LINE`**: set a breakpoint at the specified line in `FILE`, starting from 0. A pair of "`" characters are required around the target, this is not Markdown syntax.
- **bl**, **.bpcmds**, **be NUMBERS**, **bd NUMBERS**, **bc NUMBERS**, **bsc NUMBER CONDITION**: list, list with attached commands, enable, disable, delete, attach a command to breakpoint(s).
- **p**: step over, aka execute the complete current line.
- **t**: step in, aka execute the current line, if any function is called, goes into the function.
- **pt**: step out, aka run until the end of the current function.

Load the installed `vlpp.natvis` file with `.nvload` when structured visualization is needed.
It formats some primitive types defined in the `Vlpp` project,
including `WString` and other string types, `Nullable`, `Variant`, container types, etc.
The formatting applies to the **dx** command,
when you want to see raw data instead of formatted printing,
use **dx (EXPRESSION),!**.

You can also use `dv -rX` to expand "X" levels of fields. The default option is `-r0`, which only expands one level of fields.

### Commands to Avoid

- Only use **dv** without any parameters.
- DO NOT use **dt**.

## Linux/macOS Specific

Like building and running, debugging must be performed from the same folder that contains `vmake`:
- If the repo has only one project, it is in `REPO-ROOT/Test/Linux`.
- If the repo has multiple projects, it is in `REPO-ROOT/Test/Linux/PROJECT-NAME`.
  - The `PROJECT-NAME` name follows `PROJECT-NAME.vcxproj`.
You are required to `cd` to such folder before launching `lldb`, otherwise relative paths to the binary and source files will be wrong.

`lldb` is going to block the terminal and wait for interaction, you should always start `lldb` in a PTY-backed tool session, for example:

```bash
lldb -- ./Bin/UnitTest /C
```

Keep the session ID returned by the tool. Send debugger commands as newline-terminated stdin, one round at a time, and wait for output between rounds.

End the session with:

```text
quit
```

If the debugged process is still running or stuck, send Ctrl-C (`\u0003`), then send:

```text
process kill
quit
```

For non-interactive one-shot debugging, wrap `lldb` with `timeout` so it cannot block forever.
