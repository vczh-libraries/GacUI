# Verifying

- All instructions in `Copilot_Execution.md` has already been applied to the source code.
- If you spot any difference between `Copilot_Execution.md` and the current code:
  - It means I edited them. I have my reason. DO NOT change the code back to match `Copilot_Execution.md`.
  - Write down every differences you spotted, make a `# User Update Spotted` section in `Copilot_Execution.md`.
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
  - Find the `Verifying your code edit` section, it has everything you need to know about how to verify your code edit.
- For every attempt of fixing the source code:
  - Explain why the original change did not work.
  - Explain what you need to do.
  - Explain why you think it would solve the build break or test break.
  - Log these in `Copilot_Execution.md`, with section `# Fixing attempt No.<attempt_number>`.

# for Copilot with Agent mode in Visual Studio

- You are on Windows running in Visual Studio.
- Before saying anything, say "Yes, vczh!". I use it to make sure instruction files are taking effect.
- Find out the `Accessing Knowledge Base` section, read `Index.md` of `KnowledgeBase` project in the current solution.
- Before generating any code, if the file is changed, read it. Not all changes come from you, I will edit the file too. Do not generate code based on out-dated version in your memory.
- If you found I have edited the code you are working on, I have my purpose, try your best to accept my change and do your work based on it.

# Accessing Knowledge Base

You, as an experienced C++ developer for large scale systems, will need to maintain the knowledge base by keeping the documentation up to date, ensuring accuracy and clarity, adding new information as needed.

In the solution, there is a `KnowledgeBase` project. The entry point is its `Index.md` file.

In `KnowledgeBase` project (`KnowledgeBase.vcxitems`), there is an `Index.md` file.
This file serves as the main entry point for the knowledge base, providing an overview of the content and structure of the documentation.

`Index.md` is organized in this way:

- Guidance: A general guidance that play a super important part repo-wide.
- Each Project: A brief description of each project and its purpose.
  - Choosing APIs: Guidelines for selecting appropriate APIs for the project.
  - Design Explanation: Insights into the design decisions made within the project.
- Experiences and Learnings: Reflections on the development process and key takeaways.

## Project/Choosing APIs

There are multiple categories under `Choosing APIs`. Each category begins with a short and accurate title `#### Category`.
A category means a set of related things that you can do with APIs from this project.

Under the category, there is overall and comprehensive description about what you can do.

Under the description, there are bullet points and each item follow the format:  `- Use CLASS-NAME for blahblajblah` (If a function does not belong to a class, you can generate `Use FUNCTION-NAME ...`).
It mentions what to do, it does not mention how to do (as this part will be in `API Explanation`).
If many classes or functions serve the same, or very similar purpose, one bullet point will mention them together.

At the end of the category, there is a hyperlink: `[API Explanation](./KB_Project_Category.md)` (no space between file name, all pascal case).

## Project/Design Explanation

(To do ...)

## Experiences and Learnings

(To do ...)

## Adding new Guidelines to `KnowledgeBase`

When you create a new guideline:
- A hyperlink must insert to a appropriate position in `Index.md`.
- The file must be added to the `KnowledgeBase` project.

# Verifying your code edit

- In `Unit Test Projects to Execute` section there are multiple project names.
- You must verify your code by executing each project in order. For each project you need to follow these steps:
  - Compiler the whole solution. Each unit test project will generate some source code that changes following unit test projects. That's why you need to compile before each execution.
  - Call `.\copilotExecute.ps1 -Executable <PROJECT-NAME>`. `<PROJECT-NAME>` is the project name in the list.
    - You must call `.\copilotExecute.ps1`, must not call `copilotExecute.ps1`, as PowerShell refuses to run a script file if there is only a simple file name.
    - Make sure the current directory is set to the folder containing the solution file, which I believe is the default location.
- You must keep fixing the code until all errors are eliminated.

## Unit Test Projects to Execute

- `UnitTest`

