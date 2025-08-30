# Design

- The problem I would like to solve is in the chat messages sending with this request, under the `# Problem` section. If there are other sections, they are extra instructions for you in higher priority.
- Before solving my problem, read `Copilot_Task.md` and find if there is any `# !!!FINISHED!!!` mark.
  - If there is only a title, you are on a fresh start.
  - If the mark exists:
    - There might be `# Update` in the chat history, those usually have been implemented, only check the LATEST chat message.
    - If there is an `# Update` section in the LATEST chat message, it means I am going to propose some change to `Copilot_Task.md`.
      - There might be `# Update` in the chat history, those usually have been implemented, only take care of the one in the LATEST chat message.
      - Everything under the `# Update` section is used to update the problem description. It must also be copied to the `# PROBLEM DESCRIPTION` section, with a new sub-section `## UPDATE`.
      - Follow my update to change the design document.
    - If there is only a `# Problem` section in the LATEST chat message, it means the content of the file is out-dated.
      - you must execute `.\copilotPrepare.ps1` to clean up everything from the last run.
      - You must execute `.\copilotPrepare.ps1`, must not be `copilotPrepare.ps1`, as PowerShell refuses to run a script file if there is only a simple file name.
      - Make sure the current directory is set to the folder containing the solution file, which I believe is the default location.
  - If the mark does not exist, it means you are accidentally stopped. Please continue the work, the problem description should be in `Copilot_Task.md`.
- Your goal is to write a design document to `Copilot_Task.md`. DO NOT update any other file including source code.
- Repeat my problem description in `Copilot_Task.md` precisely under a `# PROBLEM DESCRIPTION` section. In case when you accidentally stop later, you will know what you are told to do.
  - If the problem description is like `Complete task a-b`, it means you should find the actual problem description from `Copilot_Scrum.md`.
    - Read the whole file througly, understand the context. Copy everything of the task (aka a-b) from `Copilot_Scrum.md` precisely in words. 
    - Update `Copilot_Scrum.md` to the title of the particular task (aka a-b), append `[PROCESSED]` (important: only append it after the task, not the phrase).
- As an experienced C++ developer for large scale systems, you need to:
  - Analyse the source code and provide a high-level design document.
  - The design document must present your idea, about how to solve the problem in architecture-wide level.
  - The design document must describe the what to change, keep the description in high-level without digging into details about how to update the source code.
  - The design document must explain the reason behind the proposed changes.
  - It is completely OK and even encouraged to have multiple solutions. You must explain each solution in a way mentioned above, and provide a comparison of their pros and cons.
- When mentioned any C++ type name, you must use its full name followed by the file which has its definition.
- Append everything to `Copilot_Task.md`. Make sure only wrap code in code block, do not wrap markdown content in code block.
- Append `# !!!FINISHED!!!` to `Copilot_Task.md` to indicate the work has been finished.

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

