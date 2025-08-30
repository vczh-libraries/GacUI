# Scrum

- The problem I would like to solve is in the chat messages sending with this request. Only check the LATEST chat message for the problem:
  - If there is an `# Problem` section, it means I am starting a fresh new request. You should override `Copilot_Scrum.md` with only one title `# !!!SCRUM!!!`. And then copy precisely my problem description from the LATEST chat message under a `# DESIGN REQUEST`.
  - If there is an `# Update` section, it means I am going to propose some change to `Copilot_Scrum.md`, you should continue to work out more details. It must also be copied to the `# PROBLEM DESCRIPTION` section, with a new sub-section `## UPDATE`.
      - Follow my update to change the design document.
  - If there is a `# Continue` section, read `Copilot_Scrum.md` and find if there is any `# !!!FINISHED!!!` mark. If there is not, it means are accidentally stopped. Please continue to complete the work. The problem to solve should be in the `# DESIGN REQUEST` section.
- Your goal is to help me break down the problem into small tasks, write a design document to `Copilot_Scrum.md`. DO NOT update any other file including source code.
  - Each task should be small enough to only represent a single idea or feature upgrade. But don't break tasks too small, each task should still be self-contained.
  - Each task should consist of improvement plan and test plan, unless testing does not make sense or it is too difficult to complete automatic tests.
  - Each task should be self contained.
  - The order of tasks is important. At the end of any task, the project should be able to compile and test.
- Break the problem in to phrases and tasks.
  - A phrase should begin with a title `## Phrase 1: A short name`. Under a phrase there is a comprehensive description about the goal of this phrase. A phrase contains multiple tasks.
  - A task should begin with a title `### Task 1-1: A short name`. Under a task there is a comprehensive description about the goal of this task. The first number is the phrase number, the second is for the task itself.
  - A task must be at least updating some code, it must not be like learning or analysis or whatever just reading. Reading, thinking and planning is your immediate work to do.
- The reason I would like you to solve the problem in this way instead of making an immediate plan of code improvement is that, it is complicated and should work across the whole project. As an experienced C++ developer for large scale systems, you need to:
  - Take in consideration of the knowledge base, finding anything that would be helpful for the current problem.
  - Read through the code base carefully. The project is complicated, one small decision may have widely impact to the other part of the project.
  - Think thoroughly.
- When making each task, after a comprehensive description, there should also be:
  - A clear definition of what to be done. Keep it high-level, you can mention what should be done to update a certain group of classes, but do not include any actual code change.
  - A clear definition of what to test in Unit Test. You do not need to design test case or think about code coverage at this moment. Instead consider about how to ensure testability, how many existing components are affected so that they need to be tested, and make sure existing test cases covered affected components.
  - A clear definition of what to test manually, since not everything can be covered by unit tests, but you need to try your best to make manual test as less as possible. It will be great if we can actually cover all the scenarios with unit tests.
  - Reasons about why you think it is necessary to have this task.
  - Any support evidences from source code or knowledge base. If you can't find anything from the knowledge base, what do you think need to be added.
- Append `# !!!FINISHED!!!` to `Copilot_Scrum.md` to indicate the design reaches the end.

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

