# Verifying

- All instructions in `Copilot_Execution.md` has already been applied to the source code.
- Your goal is to verify if they are good enough. You need to compiler the whole solution. Fix the code to avoid all compile errors.
  - If there is any compile warning, only fix warnings that caused by your code change. Do no fix any other warnings.
  - Run the unit test and see if they passed. If anything is good, you will only see test files and test cases that are executed.
    - Make sure added test cases are actually executed.
    - When all test cases passes, there will be a summarize about how many test cases are executed. Otherwise it crashed.
    - If any test case fails on a test assertion, the content of `TEST_ASSERT` or other macros will be printed to the output.
    - If any test case just crashes, the failed test case will be the last one printed. In this case, you might need to add logging to the code.
      - In any test case, `TEST_PRINT` would help.
      - In other source code, `vl::console::Console::WriteLine` would help. In `Vlpp` project, you should `#include` `Console.h`. In other projects, the `Console` class should just be available.
      - When added logging are not longer necessary, you should remove all of them.
  - Find the `Verifying your code edit` section in `copilot-instructions.md`, it has everything you need to know about how to verify your code edit.
- For every attempt of fixing the source code:
  - Explain why the original change did not work.
  - Explain what you need to do.
  - Explain why you think it would solve the build break or test break.
  - Log these in `Copilot_Execution.md`, with section `# Fixing attempt No.<attempt_number>`.
