# Design

## Goal and Constraints

- Your goal is to finish a design document in `Copilot_Task.md` to address a problem.
- You are only allowed to update `Copilot_Task.md` and add `[PROCESSED]` mark in `Copilot_Scrum.md`.
- You are not allowed to modify any other files.

## Step 1. Identify the Problem

- The problem I would like to solve is in the chat messages sending with this request.
- Find `# Problem` or `# Update` in the LATEST chat message. Ignore any `# Problem` or `# Update` in the chat history.
- If there is a `# Problem` section: it means you are on a fresh start.
  - Make sure the current working directory is set to the folder containing the solution file, which I believe is the default location.
  - You must execute `.\copilotPrepare.ps1` to clean up everything from the last run.
  - You must execute `.\copilotPrepare.ps1`, must not be `copilotPrepare.ps1`, as PowerShell refuses to run a script file if there is only a simple file name.
  - After executing, copy precisely my problem description in `# Problem` from the LATEST chat message under a `# PROBLEM DESCRIPTION`.
    - If the problem description is like `Complete task a-b`:
      - `a` and `b` are numbers, they locates the specific task in `Copilot_Scrum.md`.
      - Append `[PROCESSED]` to the title of the specific task in `Copilot_Scrum.md`. Only append it after the task, not the phrase.
- If there is an `# Update` section: it means I am going to propose some change to `Copilot_Task.md`.
  - You should continue to work out more details.
  - Copy precisely my problem description in `# Update` from the LATEST chat message to the `# PROBLEM DESCRIPTION` section, with a new sub-section `## UPDATE`.
  - Follow my update to change the design document.
- If there is nothing: it means you are accidentally stopped. Please continue your work.
  - Read `Copilot_Task.md` througly, it is highly possibly that you were working on the request described in the last section in `# PROBLEM DESCRIPTION`.

## Step 2. Understand the Goal and Quality Requirement

- As an experienced C++ developer for large scale systems, you need to:
  - Analyse the source code and provide a high-level design document.
  - The design document must present your idea, about how to solve the problem in architecture-wide level.
  - The design document must describe the what to change, keep the description in high-level without digging into details about how to update the source code.
  - The design document must explain the reason behind the proposed changes.
  - It is completely OK and even encouraged to have multiple solutions. You must explain each solution in a way mentioned above, and provide a comparison of their pros and cons.

## Step 3. Finish the Document

- Your goal is to write a design document to `Copilot_Task.md`. DO NOT update any other file including source code.
- When mentioned any C++ type name, you must use its full name followed by the file which has its definition.
- Append everything to `Copilot_Task.md`.
- Make sure only code references are in code blocks. Do not emit code blocks for markdown.

## Step 4. Mark the Completion

- Ensure there is a `# !!!FINISHED!!!` mark at the end of `Copilot_Task.md` to indicate the document reaches the end.

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

