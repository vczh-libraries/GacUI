# Task

## Goal and Constraints

- You must ensure the source code compiles.
- You must ensure all tests pass.

## Step 1. Implement Request

- Follow the chat message to implement the task.

## Step 2. Compile

- Check out `Compile the Solution` for details about compiling the solution but DO NOT run unit test yet.
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
- After finishing fixing, exit the current sub agent and tell the main agent to go back to `Step 2. Compile`

## Step 3. Run Unit Test

- Check out `Executing Unit Test` for details about running unit test projects.
  - `Executing Unit Test` is the only way to run the unit test. DO NOT call any other tools or scripts.
  - Each attempt of test-fix process should be executed in a sub agent.
    - One test-fix process includes one attempt following `Execute Unit Test` and `Fix Failed Test Cases`.
    - The main agent should call different sub agent for each test-fix process.
    - Do not test and retrieve test results in the main agent.

### Use a sub agent to run the following instructions (`Execute Unit Test` and `Fix Failed Test Cases`)

#### Execute Unit Test

- Run the unit test and see if they passed. If anything is good, you will only see test files and test cases that are executed.
  - Make sure added test cases are actually executed.
  - If any test case fails on a test assertion, the content of `TEST_ASSERT` or other macros will be printed to the output.
  - If any test case crashes, the failed test case will be the last one printed. In this case, you might need to add logging to the code.
    - In any test case, `TEST_PRINT` would help.
    - In other source code, `vl::console::Console::WriteLine` would help. In `Vlpp` project, you should `#include` `Console.h`. In other projects, the `Console` class should just be available.
    - When added logging are not longer necessary, you should remove all of them.

#### Fix Failed Test Cases

- If there are failed test cases, fix the code to make it work.
  - If your change did not change the test result, make sure you followed `Step 2. Compile` to compile the code.
  - If the test result still not changed after redoing `Step 2. Compile` and `Step 3. Run Unit Test`, these two steps are absolutely no problem, the only reason is that your change is not correct.
- You must carefully identify, if the cause is in the source code or in the failed test. In most of the cases, the cause is in the source code.
- DO NOT delete any test case.
- After finishing fixing, exit the current sub agent and tell the main agent to go back to `Step 2. Compile`

## Step 4. Check it Again

- Go back to `Step 2. Compile`, follow all instructions and all steps again.

# External Tools Environment and Context

- You are on Windows running in Visual Studio Code.
- Submitting CLI commands is not recommended unless you have no choice.
- There is some rules to follow to submit correct powershell commands:
  - DO NOT call `msbuild` or other executable files by yourself.
  - DO NOT create or delete any file unless explicitly directed.
  - MUST run any powershell script in this format: `& absolute-path.ps1 parameters...`.
  - MUST run tasks via Cursor for compiling and running test cases.

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

## Understanding the Building Tools

**WARNING**: Information offered in this section is for background knowledge only. You should always run `Build Unit Tests` and `Run Unit Tests` instead of running these scripts or calling msbuild or other executable by yourself. 

`REPO-ROOT` is the root folder of the repo.
`SOLUTION-ROOT` is the folder containing the solution file.
`PROJECT-NAME` is the name of the project.

When verifying test projects on Windows, msbuild is used to build a solution (`*.sln`) file.
A solution contains many project (`*.vcxproj`) files, a project generates an executable (`*.exe`) file.

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

