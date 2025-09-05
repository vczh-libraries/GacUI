# Execution

## Goal and Constraints

- Your goal is to finish an execution document in `Copilot_Execution.md` from `Copilot_Planning.md`.
- You are also going to apply changes on the source code as well following `Copilot_Execution.md`.

## Step 1. Identify the Problem

- The design document is in `Copilot_Task.md`, the planning document is in `Copilot_Planning.md`.
- Find `# Update` in the LATEST chat message. Ignore any `# Update` in the chat history.
- If there is an `# Update` section: it means I am going to propose some change to `Copilot_Execution.md` and the source code together.
  - You should continue to work out more details.
  - Copy precisely my problem description in `# Update` from the LATEST chat message to the `# UPDATES` section, with a new sub-section `## UPDATE`.
  - Follow my update to change the planning document and the source code.
- If there is nothing:
  - If `Copilot_Execution.md` only has a title, you are on a fresh start.
  - If there is a `# !!!FINISHED!!!` mark in `Copilot_Execution.md`, it means you are accidentally stopped while changing the source code. Please continue your work.
  - If there is no `# !!!FINISHED!!!` mark in `Copilot_Execution.md`, it means you are accidentally stopped while finishing the document. Please continue your work.

## Step 2. Finish the Document

- Your need to summary code change in `Copilot_Execution.md`.
- All changes you need to made is already in `Copilot_Planning.md`, but it contains many explanation.
  - If there is only a title in `Copilot_Planning.md`, it means I believe the plan in `Copilot_Task.md` is detailed enough. Use `Copilot_Task.md` instead in the following steps.
- Read `Copilot_Planning.md`, copy only code changes to `Copilot_Execution.md`, including which part of code will be replaced with what new code.
- Code changes in both `Improvement Plan` and `Test Plan` will need to be included. Create `Improvement Plan` and `Test Plan` in `Copilot_Execution.md` for them.
- Do not include any explanations or comments around the code change, please only include the code changes.
- Make sure only code references are in code blocks. Do not emit code blocks for markdown.

## Step 3. Mark the Completion

- Ensure there is a `# !!!FINISHED!!!` mark at the end of `Copilot_Execution.md` to indicate the document reaches the end.

## Step 4. Finish the Source Code

- Apply all instructions in `Copilot_Execution.md` to the source code.

## Step 5. Make Sure the Code Compiles

- Compile the solution and fix all compile errors.
- You do not have to worry about running unit tests at the moment. Do not run unit tests, you are good if the code compiles.

# for Copilot with Agent mode in Visual Studio

- You are on Windows running in Visual Studio.
- Before saying anything, say "Yes, vczh!". I use it to make sure instruction files are taking effect.
- Find out the `Accessing Knowledge Base` section, read `Index.md` of `KnowledgeBase` project in the current solution.
- Before generating any code, if the file is changed, read it. Not all changes come from you, I will edit the file too. Do not generate code based on out-dated version in your memory.
- If you found I have edited the code you are working on, I have my purpose, try your best to accept my change and do your work based on it.
- When looking for any file mentioned, always look for them in the solution.
  - If you find them not existing, read the solution file to search for the entry, there will be a relative file path.

# Leveraging the Knowledge Base

You, as an experienced C++ developer for large scale systems, will need to maintain the knowledge base by keeping the documentation up to date, ensuring accuracy and clarity, adding new information as needed. The main entry is `Index.md`. Find out `Accessing Knowledge Base` about how to access the knowledge base.

`Index.md` is organized in this way:

- `## Guidance`: A general guidance that play a super important part repo-wide.
- Each `## Project`: A brief description of each project and its purpose.
  - `### Choosing APIs`: Guidelines for selecting appropriate APIs for the project.
  - `### Design Explanation`: Insights into the design decisions made within the project.
- `## Experiences and Learnings`: Reflections on the development process and key takeaways.

## Project/Choosing APIs

There are multiple categories under `Choosing APIs`. Each category begins with a short and accurate title `#### Category`.
A category means a set of related things that you can do with APIs from this project.

Under the category, there is overall and comprehensive description about what you can do.

Under the description, there are bullet points and each item follow the format:  `- Use CLASS-NAME for blahblahblah` (If a function does not belong to a class, you can generate `Use FUNCTION-NAME ...`).
It mentions what to do, it does not mention how to do (as this part will be in `API Explanation`).
If many classes or functions serve the same, or very similar purpose, one bullet point will mention them together.

At the end of the category, there is a hyperlink: `[API Explanation](./KB_Project_Category.md)` (no space between file name, all pascal case).

## Project/Design Explanation

There are multiple topics under `Design Explanation`. Each topic begins with a short and accurate title `#### Topic`.
A topic means a feature of this project, it will be multiple components combined.

Under the topic, there is overall and comprehensive description about what does this feature do.

Under the description, there are bullet points to provide a little more detail, but do not make it too long. Full details are supposed to be in the document from the hyperlink.

At the end of the topic, there is a hyperlink: `[Design Explanation](./KB_Project_Design_Topic.md)` (no space between file name, all pascal case).

## Experiences and Learnings

(To do ...)

## Adding new Guidelines to `KnowledgeBase`

When you create a new hyperlink in `Index.md`:
- A hyperlink must insert to a appropriate position in `Index.md`.
- The file must be added to the `KnowledgeBase` project.

## Accessing the Knowledge Base

In the solution, there is a `KnowledgeBase` project. The entry point is its `Index.md` file.

In `KnowledgeBase` project (`KnowledgeBase.vcxitems`), there is an `Index.md` file.
This file serves as the main entry point for the knowledge base, providing an overview of the content and structure of the documentation.

