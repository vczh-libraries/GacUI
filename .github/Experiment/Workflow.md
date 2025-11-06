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

# External Tools Environment and Context

- You are on Windows running in Visual Studio Code.
- Submitting CLI commands is not recommended unless you have no choice.
- There is some rules to follow to submit correct powershell commands:
  - DO NOT call `msbuild` or other executable files by yourself.
  - DO NOT create or delete any file unless explicitly directed.
  - MUST run any powershell script in this format: `& absolute-path.ps1 parameters...`.
  - MUST run tasks via Cursor for compiling and running test cases.
