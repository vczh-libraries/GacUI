# Execution

- Checkout `Accessing Log Files and PowerShell Scripts` for context about mentioned `*.md` and `*.ps1` files.
  - All `*.md` and `*.ps1` files should already be existing, you should not create any new files.

## Goal and Constraints

- You are going to apply changes to the source code following `Copilot_Execution.md`.

## Copilot_Execution.md Structure

- `# !!!EXECUTION!!!`: This file always begin with this title.
- `# UPDATES`:
  - `## UPDATE`: There could be multiple occurrences. Each one has an exact copy of the update description I gave you.
- `# EXECUTION PLAN`.
- `# FIXING ATTEMPTS`.

## Step 1. Identify the Problem

- The execution document is in `Copilot_Execution.md`
- Find `# Update` in the LATEST chat message.
  - Ignore any these titles in the chat history.

### Execute the Plan (only when no title appears in the LATEST chat message)

Ignore this section if there is any title in the LATEST chat message
I am starting a fresh new request.

- Apply all code changes in `Copilot_Execution.md` to the source code.
  - Make sure indentation and line breaks are applied correctly, following the same style in the target file.
- After applying each step in `Copilot_Execution.md`, mark the step as completed by appending `[DONE]` after the step title. This allow you to find where you are if you are interrupted.

### Update the Source Code and Document (only when "# Update" appears in the LATEST chat message)

Ignore this section if there is no "# Update" in the LATEST chat message
I am going to propose some change to the source code.

- Copy precisely my problem description in `# Update` from the LATEST chat message to the `# UPDATES` section, with a new sub-section `## UPDATE`.
- Follow my update to change the source code.
- Update the document to keep it consistent with the source code.

## Step 2. Make Sure the Code Compiles but DO NOT Run Unit Test

- Check out `Compile the Solution` for details about compiling the solution but DO NOT run unit test.
  - `Compile the Solution` is the only way to build the project. DO NOT call any other tools or scripts.
  - Each attempt of build-fix process should be executed in a sub agent.
    - One build-fix process includes one attempt following `Build Unit Test` and `Fix Compile Errors`.
    - The main agent should call different sub agent for each build-fix process.
    - Do not build and retrieve build results in the main agent.

### Use a sub agent to run the following instructions (`Build Unit Test` and `Fix Compile Errors`)

#### Build Unit Test

- Find out if there is any warning or error.
  - `Compile the Solution` has the instruction about how to check compile result.

#### Fix Compile Errors

- If there is any compilation error, address all of them:
  - If there is any compile warning, only fix warnings that caused by your code change. Do no fix any other warnings.
  - If there is any compile error, you need to carefully identify, is the issue in the callee side or the caller side. Check out similar code before making a decision.
  - For every attempt of fixing the source code:
    - Explain why the original change did not work.
    - Explain what you need to do.
    - Explain why you think it would solve the build break.
    - Log these in `Copilot_Execution.md`, with section `## Fixing attempt No.<attempt_number>` in `# FIXING ATTEMPTS`.
- After finishing fixing, exit the current sub agent and tell the main agent to go back to `Step 2. Make Sure the Code Compiles but DO NOT Run Unit Test`.
- When the code compiles:
  - DO NOT run any tests, the code will be verified in future tasks.

# Step 3. Verify Coding Style

- Code changes in `Copilot_Execution.md` may not consider about indentation and coding style.
  - Go over each code change and ensure:
    - Indentation is correct and consistent with the surrounding code.
    - Coding style especially line breaks follows the same conventions as the surrounding code.
- Ensure any empty line does not contain spaces or tabs.

# External Tools Environment and Context

- You are on Windows running in Visual Studio Code.
- Submitting CLI commands is not recommended unless you have no choice.
- There is some rules to follow to submit correct powershell commands:
  - DO NOT call `msbuild` or other executable files by yourself.
  - DO NOT create or delete any file unless explicitly directed.
  - MUST run any powershell script in this format: `& absolute-path.ps1 parameters...`.
  - MUST run tasks via Cursor for compiling and running test cases.

# Accessing Log Files and PowerShell Scripts

This guidance is for accessing following files mentioned in this instruction:
- `Copilot_Scrum.md`
- `Copilot_Task.md`
- `Copilot_Planning.md`
- `Copilot_Execution.md`
- `Copilot_KB.md`
- `copilotPrepare.ps1`
- `copilotBuild.ps1`
- `copilotExecute.ps1`
- `copilotDebug_Start.ps1`
- `copilotDebug_Stop.ps1`
- `copilotDebug_RunCommand.ps1`
- `Build.log`
- `Execute.log`

They are in the `REPO-ROOT/.github/TaskLogs` folder. `REPO-ROOT` is the root folder of the repo.

## If you are running in Visual Studio

You will find the `TaskLogs` project in the current solution, which should contain these files.

## Important Rules for Markdown Document or Log

- Do not print "````````" or "````````markdown" in markdown file.
- It is totally fine to have multiple top level `# Topic`.
- When mentioning a C++ name in markdown file:
  - If it is defined in the standard C++ library or third-party library, use the full name.
  - If it is defined in the source code, use the full name if there is ambiguity, and then mention the file containing its definition.

# Unit Test Projects to Work with

## Compile the Solution

- Run the `Build Unit Tests` task.
- DO NOT use msbuild by yourself.

### The Correct Way to Read Compiler Result

- The only source of trust is the raw output of the compiler.
  - It is saved to `REPO-ROOT/.github/TaskLogs/Build.log`. `REPO-ROOT` is the root folder of the repo.
  - Wait for the task to finish before reading the log file. DO NOT HURRY.
    - A temporary file `Build.log.unfinished` is created during building. It will be automatically deleted as soon as the building finishes . If you see this file, it means the building is not finished yet.
  - When build succeeds, the last several lines will show the following 3 lines, otherwise there are either warnings or errors. You can check the last 10 lines to make sure if build succceeded:
    - "Build succeeded."
    - "0 Warning(s)"
    - "0 Error(s)"
- DO NOT TRUST related tools Visual Studio Code offers you, like `get_errors` or `get_task_output`, etc.

## Executing Unit Test

- Run the `Run Unit Tests` task.
- DO NOT call executables or scripts yourself.

### The Correct Way to Read Test Result

- The only source of trust is the raw output of the unit test process.
  - It is saved to `REPO-ROOT/.github/TaskLogs/Execute.log`. `REPO-ROOT` is the root folder of the repo.
  - Wait for the task to finish before reading the log file. DO NOT HURRY.
    - A temporary file `Execute.log.unfinished` is created during testing. It will be automatically deleted as soon as the testing finishes. If you see this file, it means the testing is not finished yet.
  - When all test case passes, the last several lines will show the following 2 lines, otherwise it crashed at the last showing test case. You can check the last 5 lines to make sure if all test cases passed:
    - "Passed test files: X/X"
    - "Passed test cases: Y/Y"
- DO NOT TRUST related tools Visual Studio Code offers you, like `get_errors` or `get_task_output`, etc.

## Debugging Unit Test

Debugging would be useful when you lack of necessary information.
In this section I offer you a set of powershell scripts that work with CDB (Microsoft's Console Debugger).
CDB accepts exactly same commands as WinDBG.

### Start a Debugger

`REPO-ROOT` is the root folder of the repo.
Find out `Unit Test Project Structure` to understand the solution folder and the unit test project name you are working with.
Additional information could be found in THE FIRST LINE in `REPO-ROOT/.github/TaskLogs/Execute.log`.
Execute the following powershell commands:

```
cd SOLUTION-ROOT
start powershell {& REPO-ROOT\.github\TaskLogs\copilotDebug_Start.ps1 -Executable PROJECT-NAME}
```

The `start powershell {}` is necessary otherwise the script will block the execution forever causing you to wait infinitely.
The script will finish immediately, leaving a debugger running in the background. You can send commands to the debugger.
The process being debugged is paused at the beginning, you are given a chance to set break-points.
After you are prepared, send the `g` command to start running.

### Stop a Debugger

You must call this script do stop the debugger.
Do not stop the debugger using any command.
This script is also required to run before compiling only when Visual Studio Code tasks are not available to you.

```
& REPO-ROOT\.github\TaskLogs\copilotDebug_Stop.ps1
```

If there is any error message, it means the debugger is not alive, it is good.

### Sending Commands to Debugger

```
& REPO-ROOT\.github\TaskLogs\copilotDebug_Stop.ps1 -Command "Commands"
```

Multiple commands can be executed sequencially separated by ";".
The debugger is configured to be using source mode, which means you can see source files and line numbers in the call stack, and step in/out/over are working line by line.
CDB accepts exactly same commands as WinDBG, and here are some recommended commands:
- **g**: continue until hitting a break-point or crashing.
- **k**n: print current call stack.
- **kn LINES**: print first `LINES` of the current call stack.
- **.frame NUMBER**: inspect the call stack frame labeled with `NUMBER`. `kn` will show the number, file and line along with the call stack.
- **dv**: list all available variables in the current call stack frame.
- **dx EXPRESSION**: evaluate the `EXPRESSION` and print the result. `EXPRESSION` can be any valid C programming language expression. When you specify a type (especially when doing casting), full namespaces are required, do not start with `::`.
- **bp `FILE:LINE`**: set a break-point at the specified line in `FILE`, starting from 0. A pair of "`" characters are required around the target, this is not a markdown syntax.
- **bl**, **.bpcmds**, **be NUMBERS**, **bd NUMBERS**, **bc NUMBERS**, **bsc NUMBER CONDITION**: list, list with attached commands, enable, disable, delete, attach a command to break-point(s).
- **p**: step over, aka execute the complete current line.
- **t**: step in, aka execute the currrent line, if any function is called, goes into the function.
**pt**: step out, aka run until the end of the current function.

An `.natvis` file is automatically offered with the debugger,
it formats some primitive types defined in the `Vlpp` project,
including `WString` and other string types, `Nullable`, `Variant`, container types, etc.
The formmating applies to the **dx** command,
when you want to see raw data instead of formatting printing,
use **dx (EXPRESSION),!**.

You can also use `dv -rX` to expand "X" levels of fields, the default option is `-r0` which only expand one level of fields.

### Commands to Avoid

- Only use **dv** without any parameters.
- DO NOT use **dt**.
- DO NOT use **q**, **qd**, **qq**, **qqd** etc to stop the debugger, always use `copilotDebug_Stop.ps1`.

## Understanding the Building Tools

**WARNING**: Information offered in this section is for background knowledge only.
You should always run `Build Unit Tests` and `Run Unit Tests` instead of running these scripts or calling msbuild or other executable by yourself.
Only when you cannot access tools offered by Visual Studio Code, scripts below are allowed to use.

`REPO-ROOT` is the root folder of the repo.
`SOLUTION-ROOT` is the folder containing the solution file.
`PROJECT-NAME` is the name of the project.

When verifying test projects on Windows, msbuild is used to build a solution (`*.sln`) file.
A solution contains many project (`*.vcxproj`) files, a project generates an executable (`*.exe`) file.

Before building, ensure the debugger has stopped, otherwise the running unit test process will cause a linking failure.
If there is any error message, it means the debugger is not alive, it is good.

```
& REPO-ROOT\.github\TaskLogs\copilotDebug_Stop.ps1
```

The `Build Unit Tests` task calls msbuild to build the only solution which contains all test cases.
Inside the task, it runs `copilotBuild.ps1`

```
cd SOLUTION-ROOT
& REPO-ROOT\.github\TaskLogs\copilotBuild.ps1
```

The `Run Unit Tests` task runs all generated *.exe file for each *.vcxproj that is created for test cases.
To run test cases in `SOLUTION-ROOT\PROJECT-NAME\PROJECT-NAME.vcxproj`

```
cd SOLUTION-ROOT
& REPO-ROOT\.github\TaskLogs\copilotExecute.ps1 -Executable PROJECT-NAME
```

Test cases are organized in multiple test files.
In `PROJECT-NAME\PROJECT-NAME.vcxproj.user` there is a filter, when it is effective, you will see filtered test files marked with `[SKIPPED]` in `Execute.log`.
The filter is defined in this XPath: `/Project/PropertyGroup@Condition="'$(Configuration)|$(Platform)'=='Debug|x64'"/LocalDebuggerCommandArguments`.
The filter is effective only when the file exists and the element exists with one or multiple `/F:FILE-NAME.cpp`, listing all test files to execute, unlited files are skipped.
But if the element exists but there is no `/F:FILE-NAME.cpp`, it executes all test files, non is skipped.

**IMPORTANT**:

ONLY WHEN test files you want to run is skipped, you can update the filter to activate it. This would typically happen when:
- A new test file is added.
- A test file is renamed.

You can clean up the filter to remove unrelated files, that is either not existing or it is totally unrelated to the current task you are working.
If the current task does not work on that test file, but it tests closely related topic, you should better keep it in the list.

DO NOT delete this *.vcxproj.user file.
DO NOT clean the filter (aka delete all `/FILE-NAME.cpp`) by yourself. I put a filter there because running everything is slow and unnecessary for the current task.

### Unit Test Project Structure

In this project, the only unit test solution is `REPO-ROOT\Test\GacUISrc\GacUISrc.sln` therefore `SOLUTION-ROOT` is `REPO-ROOT\Test\GacUISrc`.
Here is the list of all unit test projects under this solution and they are executed in the following order:
- UnitTest

