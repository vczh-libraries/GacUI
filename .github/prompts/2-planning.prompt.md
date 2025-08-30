# Planning

- The design document is in `Copilot_Task.md`. You must carefully read through the file, it has the goal, the whole idea as well as analysis. If `Copilot_Task.md` mentions anything about updating the knowledge base, ignore it as it will be addressed later.
- If there is anything in the chat message, they are extra instructions for you in higher priority.
- Before solving my problem, read `Copilot_Planning.md` and find if there is any `# !!!FINISHED!!!` mark.
  - If there is only a title, you are on a fresh start.
  - If the mark exists, follow my instructions to update the document.
    - There might be `# Update` in the chat history, those usually have been implemented, only check the LATEST chat message.
    - Copy my update precisely to the beginning of the document. There must be a `# UPDATES`, following every attempt of update in each `# UPDATE`.
  - If the mark does not exist, it means you are accidentally stopped. Please continue the work.
- Your goal is to write a very detailed planning document to `Copilot_Planning.md`. DO NOT update any other file including source code.
- You need to write complete two main sections in `Copilot_Planning.md`, `Improvement Plan` and `Test Plan`.
- **Improvement Plan**: as an experienced C++ developer for large scale systems, you need to:
  - Read through and understand the task in `Copilot_Task.md`. If any type name or function name is mentioned in the task, you must first try to find the full name as well as the file where it is defined.
  - C++ source files depends on each other, by just implementing the task it may not enough. Find out what will be affected.
  - Propose any code change you would like to do. It must be detailed enough to say which part of code will be replaced with what new code.
  - Explain why you want to make these changes.
  - When offering comments for code changes, do not just repeat what has been done, say why this has to be done.
    - If the code is simple and obvious, no comment is needed. Actually most of the code should not have comments.
    - Do not say something like `i++; // add one to i`, which offers no extra information. Usually no comments should be offered for such code, except there is any hidden or deep reason.
- **Test Plan**: as an experienced C++ developer for large scale systems, you need to:
  - Design test cases that cover all aspects of the changes made in the Improvement Plan.
  - Ensure test cases are clear enough to be easily understood and maintained.
  - Carefully think about corner cases to cover.
  - For refactoring work, existing test cases might have already most of the scenarios. Carefully review them and only add new test cases if necessary.
  - If you think any current test case must be updated or improved, explain why.
- Append everything to `Copilot_Planning.md`. Make sure only wrap code in code block, do not wrap markdown content in code block.
- Append `# !!!FINISHED!!!` to `Copilot_Planning.md` to indicate the work has been finished.

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

