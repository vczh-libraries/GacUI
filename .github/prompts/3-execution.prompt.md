# Execution

- The design document is in `Copilot_Task.md`, the planning document is in `Copilot_Planning.md`.
- If there is anything in the chat message, they are extra instructions for you in higher priority.
- Before solving my problem, read `Copilot_Execution.md` and find if there is any `# !!!FINISHED!!!` mark.
  - If there is only a title, you are on a fresh start.
  - If the mark does not exist, it means you are accidentally stopped. Please continue editing `Copilot_Execution.md`.
  - If the mark exists:
    - There might be `# Update` in the chat history, those usually have been implemented, only check the LATEST chat message.
    - If there is an `# Update` section in the LATEST chat message:
      - Copy my update precisely to the beginning of the document. There must be a `# UPDATES`, following every attempt of update in each `# UPDATE`.
    - If the LATEST chat message is empty:
      - It means you are accidentally stopped, but the editing on `Copilot_Execution.md` is actually finished, please continue editing source code.
- Your goal is to summary code change in `Copilot_Execution.md` and apply them to source files.
- **Copilot_Execution.md**:
  - All changes you need to made is already in `Copilot_Planning.md`, but it contains many explanation.
    - If there is only a title in `Copilot_Planning.md`, it means I believe the plan in `Copilot_Task.md` is detailed enough. Use `Copilot_Task.md` instead in the following steps.
  - Read `Copilot_Planning.md`, copy only code changes to `Copilot_Execution.md`, including which part of code will be replaced with what new code.
  - Code changes in both `Improvement Plan` and `Test Plan` will need to be included. Create `Improvement Plan` and `Test Plan` in `Copilot_Execution.md` for them.
  - Do not include any explanations or comments around the code change, please only include the code changes.
  - Make sure only wrap code in code block, do not wrap markdown content in code block.
  - Append `# !!!FINISHED!!!` to `Copilot_Execution.md` to indicate the work has been finished.
- **Execution**:
  - Apply all instructions in `Copilot_Execution.md` to the source code.
- **Verifying**
  - Fix all compile errors.
  - You do not have to worry about running unit tests at the moment. Do not run unit tests, you are good if the code compiles.

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

