# [State] Entry
[READY][Compile]finished implementing the task

- Follow the chat message to implement the task.

# [State] Compile
[PASSED][Test]there is not compile error
[FAILED][Fix Compile Error]there is any compiler error

- Run the `Build Unit Tests` task.
- The only source of trust is the raw output of the compiler.
  - It is saved to `REPO-ROOT/.github/TaskLogs/Build.log`. `REPO-ROOT` is the root folder of the repo.
  - Wait for the task to finish before reading the log file. DO NOT HURRY.
- DO NOT TRUST related tools Visual Studio Code offers you, like `get_errors` or `get_task_output`, etc.

# [State] Fix Compile Error
[READY][Compile]finished fixing compile error

- If there is any compile warning, only fix warnings that caused by your code change. Do no fix any other warnings.
- If there is any compile error, you need to carefully identify, is the issue in the callee side or the caller side. Check out similar code before making a decision.

# [State] Test
[PASSED][Finished]there is not test failure
[FAILED][Fix Test Failure]there is any test failure

- Run the `Run Unit Tests` task.
- The only source of trust is the raw output of the unit test process.
  - It is saved to `REPO-ROOT/.github/TaskLogs/Execute.log`. `REPO-ROOT` is the root folder of the repo.
  - Wait for the task to finish before reading the log file. DO NOT HURRY.
  - When all test cases pass, there will be a summarizing about how many test cases are executed. Otherwise it crashed at the last showing test case.
- DO NOT TRUST related tools Visual Studio Code offers you, like `get_errors` or `get_task_output`, etc.
- Run the unit test and see if they passed. If anything is good, you will only see test files and test cases that are executed.
  - Make sure added test cases are actually executed.
  - If any test case fails on a test assertion, the content of `TEST_ASSERT` or other macros will be printed to the output.
  - If any test case crashes, the failed test case will be the last one printed. In this case, you might need to add logging to the code.

# [State] Fix Test Failure
[READY][Compile]finished fixing test failure

- If any test case fails on a test assertion, the content of `TEST_ASSERT` or other macros will be printed to the output.
- If any test case crashes, the failed test case will be the last one printed. In this case, you might need to add logging to the code.
  - In any test case, `TEST_PRINT` would help.
  - In other source code, `vl::console::Console::WriteLine` would help. In `Vlpp` project, you should `#include` `Console.h`. In other projects, the `Console` class should just be available.
  - When added logging are not longer necessary, you should remove all of them.
- You must carefully identify, if the cause is in the source code or in the failed test. In most of the cases, the cause is in the source code.
- DO NOT delete any test case.

# [State] Finished

You have done your job!

# General Instructions

- You are on Windows running in Visual Studio Code.
- Submitting CLI commands is not recommended unless you have no choice.
- There is some rules to follow to submit correct powershell commands:
  - DO NOT call `msbuild` or other executable files by yourself.
  - DO NOT create or delete any file unless explicitly directed.
  - MUST run any powershell script in this format: `& absolute-path.ps1 parameters...`.
  - MUST run tasks via Cursor for compiling and running test cases.
