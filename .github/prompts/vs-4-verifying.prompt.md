# Verifying

- Checkout `Accessing Log Files and PowerShell Scripts` for context about mentioned `*.md` and `*.ps1` files.
  - All `*.md` and `*.ps1` files should already be existing, you should not create any new files.

## Goal and Constraints

- All instructions in `Copilot_Execution.md` should have been applied to the source code, your goal is to test it.
- You must ensure the source code compiles.
- You must ensure all tests pass.

## Step 1. Check and Respect my Code Change

- If you spot any difference between `Copilot_Execution.md` and the source code:
  - It means I edited them. I have my reason. DO NOT change the code to match `Copilot_Execution.md`.
  - Write down every differences you spotted, make a `## User Update Spotted` section in the `# UPDATES` section in `Copilot_Execution.md`.

## Step 2. Compile

- Check out `Compile the Solution` for details about compiling the solution but DO NOT run unit test. If there is any compilation error, address all of them:
  - If there is any compile warning, only fix warnings that caused by your code change. Do no fix any other warnings.
  - If there is any compile error, you need to carefully identify, is the issue in the callee side or the caller side. Check out similar code before making a decision.
  - For every attempt of fixing the source code:
    - Explain why the original change did not work.
    - Explain what you need to do.
    - Explain why you think it would solve the build break or test break.
    - Log these in `Copilot_Execution.md`, with section `## Fixing attempt No.<attempt_number>` in `# FIXING ATTEMPTS`.
  - Go back to `Step 2. Compile`

## Step 3. Run Unit Test

- Check out `Executing Unit Test` for details about compiling the solution but DO NOT run unit test. If there is any compilation error, address all of them:
- Run the unit test and see if they passed. If anything is good, you will only see test files and test cases that are executed.
  - Make sure added test cases are actually executed.
  - If any test case fails on a test assertion, the content of `TEST_ASSERT` or other macros will be printed to the output.
  - If any test case crashes, the failed test case will be the last one printed. In this case, you might need to add logging to the code.
    - In any test case, `TEST_PRINT` would help.
    - In other source code, `vl::console::Console::WriteLine` would help. In `Vlpp` project, you should `#include` `Console.h`. In other projects, the `Console` class should just be available.
    - When added logging are not longer necessary, you should remove all of them.

## Step 4. Fix Failed Test Cases

- If there are failed test cases, fix the code to make it work.
- You must carefully identify, if the cause is in the source code or in the failed test. In most of the cases, the cause is in the source code.
- DO NOT delete any test case.
- For every attempt of fixing the source code:
  - Explain why the original change did not work.
  - Explain what you need to do.
  - Explain why you think it would solve the build break or test break.
  - Log these in `Copilot_Execution.md`, with section `## Fixing attempt No.<attempt_number>` in `# FIXING ATTEMPTS`.

## Step 5. Check it Again

- Go back to `Step 2. Compile`, follow all instructions and all steps again.
  - Until the code compiles and all test cases pass. Ensure there is a `# !!!VERIFIED!!!` mark at the end of `Copilot_Execution.md`.

# General Instruction

- You are on Windows running in Visual Studio.
- Before saying anything, say "Yes, vczh!". I use it to make sure instruction files are taking effect.
- Find out the `Accessing Knowledge Base` section, read `Index.md` of `KnowledgeBase` project in the current solution.
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

# Accessing Log Files and PowerShell Scripts

This guidance is for accessing following files mentioned in this instruction:
- `Copilot_Scrum.md`
- `Copilot_Task.md`
- `Copilot_Planning.md`
- `Copilot_Execution.md`
- `Copilot_KB.md`
- `copilotExecute.ps1`
- `copilotPrepare.ps1`

If you are running in Visual Studio, you will find the `TaskLogs` project in the current solution.
Otherwise, locate the `TaskLogs` project in `REPO-ROOT/.github/TaskLogs/TaskLogs.vcxitems`.
`REPO-ROOT` is the root folder of the repo.

`TaskLogs.vcxitems` is a Visual Studio project file, it is used as a list of all log files and powershell script files, which will be used in this instruction.
You need to locate listed files in `TaskLogs.vcxitems`.

## Important Rules for Markdown Document or Log

- Do not print "````````" or "````````markdown" in markdown file.
- It is totally fine to have multiple top level `# Topic`.
- When mentioning a C++ name in markdown file:
  - If it is defined in the standard C++ library or third-party library, use the full name.
  - If it is defined in the source code, use the full name if there is ambiguity, and then mention the file containing its definition.

# Unit Test Projects to Work with

- `UnitTest`

## Calling copilotBuild.ps1 and copilotExecute.ps1

This solution is in `Test\GacUISrc`, after `ls` to this folder, scripts will be accessible with:
- `& ..\..\.github\TaskLogs\copilotBuild.ps1`
  - Check out `Compile the Solution` for usage of this script.
- `& ..\..\.github\TaskLogs\copilotExecute.ps1 -Executable <The-Test-Project-Name>`. 
  - Check out `Executing Unit Test` for usage of this script.

## Compile the Solution

- Just let Visual Studio to compile the solution.
- DO NOT use msbuild by yourself.

## Executing Unit Test

- In `Unit Test Projects to Work with` section there are multiple project names.
- These projects are all `*.vcxproj` files. Locate them. In the parent folder there must be a `*.sln` file. That is the solution the compile.
- You must move the current working directory to the folder containing the `*.sln` file.
  - The `ls` command helps.
  - This must be done because `copilotExecute.ps1` searches `*.sln` from the working directory, otherwise it will fail.
- You must verify your code by executing each project in order. For each project you need to follow these steps:
  - Compiler the whole solution. Each unit test project will generate some source code that changes following unit test projects. That's why you need to compile before each execution.
  - Execute `copilotExecute.ps1 -Executable <PROJECT-NAME>`. `<PROJECT-NAME>` is the project name in the list.
    - When all test cases pass, there will be a summarizing about how many test cases are executed. Otherwise it crashed at the last showing test case.
- You must keep fixing the code until all errors are eliminated.

