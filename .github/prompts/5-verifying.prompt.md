# Verifying

- Check out `Accessing Task Documents` and `Accessing Script Files` in `REPO-ROOT/.github/copilot-instructions.md` for context about mentioned `*.md`, `*.ps1` and `*.sh` files.
- All `*.md`, `*.ps1` and `*.sh` files should exist; you should not create any new files unless explicitly instructed.
  - The `Copilot_Execution.md` file should already exist.
  - If you cannot find the file, you are looking at a wrong folder.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.

## Goal and Constraints

- All instructions in `Copilot_Execution.md` should have been applied to the source code, your goal is to test it.
- You must ensure the source code compiles.
- You must ensure all tests pass.
- Until the code compiles and all test cases pass, ensure there is a `# !!!VERIFIED!!!` mark at the end of `Copilot_Execution.md`.

## Step 1. Check and Respect my Code Change

- If you spot any difference between `Copilot_Execution.md` and the source code:
  - It means I edited them. I have my reason. DO NOT change the code to match `Copilot_Execution.md`.
  - Write down every difference you spotted, make a `## User Update Spotted` section in the `# UPDATES` section in `Copilot_Execution.md`.

## Step 2. Make Sure the Code Compiles

- Check out `External Tools Environment and Context` in `REPO-ROOT/.github/copilot-instructions.md` for accessing scripts for building.
  - Strictly follow the instruction above as this repo does not use ordinary tools.
- Each attempt of build-fix process should be executed in a sub agent.
  - One build-fix process includes one attempt with the following instructions.
  - The main agent should call different sub agent for each build-fix process.
  - Do not build and retrieve build results in the main agent.

### Use a sub agent to run all following instructions (`Build the Solution`, `Fix Compile Errors`, `Code Generation`, `Finishing Code Change`)

#### Build the Solution

- Check out `# AFFECTED PROJECTS` in `Copilot_Execution.md` to find out what solutions you need to build.
- Find out if there is any warning or error.
  - `External Tools Environment and Context` in `REPO-ROOT/.github/copilot-instructions.md` has the instruction about how to check compile result.
- DO NOT delete the `Build.log` if it is generated.

#### Fix Compile Errors

- If there is any compilation error, address all of them:
  - If there is any compile warning, only fix warnings that caused by your code change. Do not fix any other warnings.
  - If there is any compile error, you need to carefully identify, is the issue in the callee side or the caller side. Check out similar code before making a decision.
  - For every attempt of fixing the source code:
    - Explain why the original change did not work.
    - Explain what you need to do.
    - Explain why you think it would solve the build break.
    - Log these in `Copilot_Execution.md`, with section `## Fixing attempt No.<attempt_number>` in `# FIXING ATTEMPTS`.

### Code Generation

- Check out `## Projects for Verification` in `REPO-ROOT/Project.md`.
  - Pay attention to the `### Code Generation Projects` section, `code generation projects` below references this section.
- Check out `# AFFECTED PROJECTS` in `Copilot_Execution.md`.
- Find out if any code generation is necessary.
  - If there is no need to run any code generation, you can skip this step.
  - Otherwise, pay attention to:
    - What configuration is needed to build the solution.
    - What project should be executed, and what configuration is needed.
    - It is possible that a project needs to be executed multiple times in different configuration.
    - It is possible that building is required between two runs of code generation projects.
      - The building and future code generation project execution should be handled by the next sub agent.

### Finishing Code Change

- Exit the current sub agent and tell the main agent to go back to `Step 2. Make Sure the Code Compiles`.
  - If there is any unfinished work according to `Code Generation`, make sure to tell the main agent about them.

## Step 3. Run Unit Test

- Check out `External Tools Environment and Context` in `REPO-ROOT/.github/copilot-instructions.md` for accessing scripts for testing and debugging.
  - Strictly follow the instruction above as this repo does not use ordinary tools.
- Each attempt of test-fix process should be executed in a sub agent.
  - One test-fix process includes one attempt following `Execute Unit Test` and `Fix Failed Test Cases`.
  - The main agent should call different sub agent for each test-fix process.
  - Do not test and retrieve test results in the main agent.
- DO NOT delete the `Execute.log` if it is generated.

### Use a sub agent to run the following instructions (`Execute Unit Test`, `Identify the Cause of Failure`, `Fix Failed Test Cases`)

#### Execute Unit Test

- Check out `# AFFECTED PROJECTS` in `Copilot_Execution.md` to find out what projects you need to execute.
- Run the unit test and see if they passed. If anything is good, you will only see test files and test cases that are executed.
  - Make sure added test cases are actually executed.
  - If any test case fails on a test assertion, the content of `TEST_ASSERT` or other macros will be printed to the output.
  - If any test case crashes, the failed test case will be the last one printed. In this case, you might need to add logging to the code.
    - In any test case, `TEST_PRINT` would help.
    - In other source code, `vl::console::Console::WriteLine` would help. In `Vlpp` project, you should `#include` `Console.h`. In other projects, the `Console` class should just be available.
    - When added logging are not longer necessary, you should remove all of them.

#### Identify the Cause of Failure

- You can refer to `Copilot_Task.md` and `Copilot_Planning.md` to understand the context, keep the target unchanged.
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
- Even when a failure is not related to your change, you should take care of it.

#### Fix Failed Test Cases

- Apply fixings to source files.
- DO NOT delete any test case.
- For every attempt of fixing the source code:
  - Explain why the original change did not work.
  - Explain what you need to do.
  - Explain why you think it would solve the build break or test break.
  - Log these in `Copilot_Execution.md`, with section `## Fixing attempt No.<attempt_number>` in `# FIXING ATTEMPTS`.
- After finishing fixing, exit the current sub agent and tell the main agent to go back to `Step 2. Make Sure the Code Compiles`
  - `Step 2. Make Sure the Code Compiles` and `Step 3. Run Unit Test` are absolutely no problem. If you didn't see any progress, the only reason is that your change is not correct.

## Step 4. Check it Again

- Go back to `Step 2. Make Sure the Code Compiles`, follow all instructions and all steps again.
- DO NOT delete the `Build.log` or `Execute.log` if they are generated.
  - They should remain untouched after finishing these instructions.
