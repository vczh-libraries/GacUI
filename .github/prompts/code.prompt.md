# Task

- Check out `Accessing Task Documents` and `Accessing Script Files` for context about mentioned `*.md` and `*.ps1` files.
- All `*.md` and `*.ps1` files should exist; you should not create any new files unless explicitly instructed.

## Goal and Constraints

- You must ensure the source code compiles.
- You must ensure all tests pass.

## Step 1. Implement Request

- Follow the chat message to implement the task.

## Step 2. Compile

- Check out `External Tools Environment and Context` in `REPO-ROOT/.github/copilot-instructions.md` for accessing scripts for building.
  - Strictly follow the instruction above as this repo does not use ordinary tools.
- Each attempt of build-fix process should be executed in a sub agent.
  - One build-fix process includes one attempt following `Build Unit Test` and `Fix Compile Errors`.
  - The main agent should call different sub agent for each build-fix process.
  - Do not build and retrieve build results in the main agent.

### Use a sub agent to run the following instructions (`Build Unit Test` and `Fix Compile Errors`)

#### Build Unit Test

- Check out `REPO-ROOT/.github/Project.md` to find out what solutions you need to build.
- Find out if there is any warning or error.
  - `External Tools Environment and Context` has the instruction about how to check compile result.

#### Fix Compile Errors

- If there is any compilation error, address all of them:
  - If there is any compile warning, only fix warnings that caused by your code change. Do not fix any other warnings.
  - If there is any compile error, you need to carefully identify, is the issue in the callee side or the caller side. Check out similar code before making a decision.
- After finishing fixing, exit the current sub agent and tell the main agent to go back to `Step 2. Compile`

## Step 3. Run Unit Test

- Check out `External Tools Environment and Context` in `REPO-ROOT/.github/copilot-instructions.md` for accessing scripts for testing and debugging.
  - Strictly follow the instruction above as this repo does not use ordinary tools.
- Each attempt of test-fix process should be executed in a sub agent.
  - One test-fix process includes one attempt following `Execute Unit Test` and `Fix Failed Test Cases`.
  - The main agent should call different sub agent for each test-fix process.
  - Do not test and retrieve test results in the main agent.

### Use a sub agent to run the following instructions (`Execute Unit Test`, `Identify the Cause of Failure` and `Fix Failed Test Cases`)

#### Execute Unit Test

- Check out `REPO-ROOT/.github/Project.md` to find out what projects you need to execute.
- Run the unit test and see if they passed. If everything is good, you will only see test files and test cases that are executed.
  - Make sure added test cases are actually executed.
  - If any test case fails on a test assertion, the content of `TEST_ASSERT` or other macros will be printed to the output.
  - If any test case crashes, the failed test case will be the last one printed. In this case, you might need to add logging to the code.
    - In any test case, `TEST_PRINT` would help.
    - In other source code, `vl::console::Console::WriteLine` would help. In `Vlpp` project, you should `#include` `Console.h`. In other projects, the `Console` class should just be available.
    - When added logging is no longer necessary, you should remove all of them.

#### Identify the Cause of Failure

- Dig into related source code carefully, make your assumption about the root cause.
- `TEST_ASSERT` in test cases or `vl::console::Console::WriteLine` in the source code would help.
  - They can make sure the expected code path is executed.
  - They can print variable values after converting to strings.
- Debug the unit test directly to get accurate clues if you are not confident of the assumption
  - Follow `Debugging a Project` to start a debugger and run WinDBG commands.
  - From there you can set break-points, walk through code by lines, and inspect variables.
  - You must stop the debugger after you finish debugging.
- When you have made a few guesses but did not progress, you are recommended to debug the unit test directly.
  - Break-points are very useful to ensure the expected code path is executed, and you can inspect variable values.

#### Fix Failed Test Cases

- Apply fixes to source files.
- DO NOT delete any test case.
- After finishing fixing, exit the current sub agent and tell the main agent to go back to `Step 2. Compile`
  - `Step 2. Compile` and `Step 3. Run Unit Test` are absolutely no problem. If you didn't see any progress, the only reason is that your change is not correct.

## Step 4. Check it Again

- Go back to `Step 2. Compile`, follow all instructions and all steps again.
