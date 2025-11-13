# Verifying

- Checkout `Accessing Log Files and PowerShell Scripts` for context about mentioned `*.md` and `*.ps1` files.
  - All `*.md` and `*.ps1` files should already be existing, you should not create any new files.

## Goal and Constraints

- All instructions in `Copilot_Execution.md` should have been applied to the source code, your goal is to test it.
- You must ensure the source code compiles.
- You must ensure all tests pass.
- Until the code compiles and all test cases pass. Ensure there is a `# !!!VERIFIED!!!` mark at the end of `Copilot_Execution.md`.

## Step 1. Check and Respect my Code Change

- If you spot any difference between `Copilot_Execution.md` and the source code:
  - It means I edited them. I have my reason. DO NOT change the code to match `Copilot_Execution.md`.
  - Write down every differences you spotted, make a `## User Update Spotted` section in the `# UPDATES` section in `Copilot_Execution.md`.

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
  - For every attempt of fixing the source code:
    - Explain why the original change did not work.
    - Explain what you need to do.
    - Explain why you think it would solve the build break or test break.
    - Log these in `Copilot_Execution.md`, with section `## Fixing attempt No.<attempt_number>` in `# FIXING ATTEMPTS`.
- After finishing fixing, exit and tell the main agent to go back to `Step 2. Compile`

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
  - You can reference to `Copilot_Task.md` and `Copilot_Planning.md` for more details before making a decision, about fixing the test case or the source code.
- DO NOT delete any test case.
- For every attempt of fixing the source code:
  - Explain why the original change did not work.
  - Explain what you need to do.
  - Explain why you think it would solve the build break or test break.
  - Log these in `Copilot_Execution.md`, with section `## Fixing attempt No.<attempt_number>` in `# FIXING ATTEMPTS`.
- After finishing fixing, exit and tell the main agent to go back to `Step 2. Compile`

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
- `Build.log`
- `Execute.log`

They are in the `REPO-ROOT/.github/TaskLogs` folder.

`REPO-ROOT` is the root folder of the repo.

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
- DO NOT TRUST related tools Visual Studio Code offers you, like `get_errors` or `get_task_output`, etc.

## Executing Unit Test

- Run the `Run Unit Tests` task.
- DO NOT call executables or scripts yourself.

### The Correct Way to Read Test Result

- The only source of trust is the raw output of the unit test process.
  - It is saved to `REPO-ROOT/.github/TaskLogs/Execute.log`. `REPO-ROOT` is the root folder of the repo.
  - Wait for the task to finish before reading the log file. DO NOT HURRY.
  - When all test cases pass, there will be a summarizing about how many test cases are executed. Otherwise it crashed at the last showing test case.
- DO NOT TRUST related tools Visual Studio Code offers you, like `get_errors` or `get_task_output`, etc.

