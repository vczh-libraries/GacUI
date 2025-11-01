# Task

## Goal and Constraints

- You must ensure the source code compiles.
- You must ensure all tests pass.

## Step 1. Implement Request

- Follow the chat message to implement the task.

## Step 2. Compile

- Check out `Compile the Solution` for details about compiling the solution but DO NOT run unit test yet.
  - `Compile the Solution` is the only way to build the project. DO NOT call any other tools or scripts.
- If there is any compilation error, address all of them:
  - If there is any compile warning, only fix warnings that caused by your code change. Do no fix any other warnings.
  - If there is any compile error, you need to carefully identify, is the issue in the callee side or the caller side. Check out similar code before making a decision.
  - Go back to `Step 2. Compile`

## Step 3. Run Unit Test

- Check out `Executing Unit Test` for details about running unit test projects.
  - `Executing Unit Test` is the only way to run the unit test. DO NOT call any other tools or scripts.
- Run the unit test and see if they passed. If anything is good, you will only see test files and test cases that are executed.
  - Make sure added test cases are actually executed.
  - If any test case fails on a test assertion, the content of `TEST_ASSERT` or other macros will be printed to the output.
  - If any test case crashes, the failed test case will be the last one printed. In this case, you might need to add logging to the code.
    - In any test case, `TEST_PRINT` would help.
    - In other source code, `vl::console::Console::WriteLine` would help. In `Vlpp` project, you should `#include` `Console.h`. In other projects, the `Console` class should just be available.
    - When added logging are not longer necessary, you should remove all of them.

## Step 4. Fix Failed Test Cases

- If there are failed test cases, fix the code to make it work.
  - If your change did not change the test result, make sure you followed `Step 2. Compile` to compile the code.
  - If the test result still not changed after redoing `Step 2. Compile` and `Step 3. Run Unit Test`, these two steps are absolutely no problem, the only reason is that your change is not correct.
- You must carefully identify, if the cause is in the source code or in the failed test. In most of the cases, the cause is in the source code.
- DO NOT delete any test case.

## Step 5. Check it Again

- Go back to `Step 2. Compile`, follow all instructions and all steps again.

# External Tools Environment and Context

- You are on Windows running in Visual Studio Code.
- In order to achieve the goal, you always need to create/delete/update files, build the project, run the unit test, etc. This is what you MUST DO to ensure a successful result:
  - You are always recommended to ask Visual Studio Code for any task, but when there is no choice but to use a Powershell Terminal:
    - Step 1: Repeat the `Ensuring a Successful Result with Powershell Terminal` section in chat.
    - Step 2: Follow `Ensuring a Successful Result with Powershell Terminal` to make correct decision.

## Ensuring a Successful Result with Powershell Terminal

- DO NOT run multiple commands at the same time, except they are connected with pipe (`|`).
- DO NOT call `msbuild` or other executable files by yourself.
- DO NOT create any new file unless explicitly directed.
- MUST run any powershell script in this format: `& absolute-path.ps1 parameters...`.
- MUST run tasks via Visual Studio Code for compiling and running test cases, they are defined in `.vscode/tasks.json`, DO NOT change this file.
- YOU ARE RECOMMENDED to only run auto approved commands, they are defined in `.vscode/settings.json`, DO NOT change this file.

# General Instructions

- Find out the `Accessing the Knowledge Base` section, read `Index.md` of `KnowledgeBase` project in the current solution.
- Before generating any code, if the file is changed, read it. Not all changes come from you, I will edit the file too. Do not generate code based on out-dated version in your memory.
- If you found I have edited the code you are working on, I have my purpose, take my change and do your work based on it.
- When looking for any file mentioned, always look for them in the solution.
  - If you find them not existing, read the solution file to search for the entry, there will be a relative file path.
- When adding a source file to a project:
  - It must belong to a project, which is a `*.vcxproj` or `*.vcxitems` file.
  - It is an XML file.
  - Edit that project file to include the source file.
- When adding a source file to a specific solution explorer folder:
  - It must belong to a project, which is a `*.vcxproj` or `*.vcxitems` file.
  - Find the `*.filters` file with the same name, it is an XML file.
  - Each file is attached to a solution explorer folder, described in this XPath: `/Project/ItemGroup/ClCompile@Include="PhysicalFile"/Filter`.
  - In side the `Filter` tag there is the solution explorer folder.
  - Edit that `*.filters` file to include the source file.

# Unit Test Projects to Work with

## Compile the Solution

- Just let Visual Studio Code to compile the solution, the `Build Unit Tests` should have been configured in `tasks.json`.
  - This task only copmile without running.
- If Visual Studio Code is not well configured, you must warn me in chat with BIG BOLD TEXT and stop immediately.
- DO NOT use msbuild by yourself.
- DO NOT modify `tasks.json`.

## Executing Unit Test

- Just let Visual Studio Code to run the unit test, the `Run Unit Tests` should have been configured in `tasks.json`.
  - If you updated any source files, you should build the unit test before running it, check out `Compile the Solution` for details.
  - Run the `Run Unit Tests` task.
  - When all test cases pass, there will be a summarizing about how many test cases are executed. Otherwise it crashed at the last showing test case.
- If Visual Studio Code is not well configured, you must warn me in chat with BIG BOLD TEXT and stop immediately.
- DO NOT call executables or scripts yourself.
- DO NOT modify `tasks.json`.

