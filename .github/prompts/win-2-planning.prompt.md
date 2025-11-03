# Planning

- Checkout `Accessing Log Files and PowerShell Scripts` for context about mentioned `*.md` and `*.ps1` files.
  - All `*.md` and `*.ps1` files should already be existing, you should not create any new files.

## Goal and Constraints

- Your goal is to finish a planning document in `Copilot_Planning.md` to address a problem from `Copilot_Task.md`.
- You are only allowed to update `Copilot_Planning.md`.
- You are not allowed to modify any other files.
- Anything in the instruction is a guidance to complete `Copilot_Planning.md`.
- DO NOT modify any source code.

## Copilot_Planning.md Structure

- `# !!!PLANNING!!!`: This file always begin with this title.
- `# UPDATES`: For multiple `## UPDATE` sections. It should always exist even there is no update.
  - `## UPDATE`: There could be multiple occurrences. Each one has an exact copy of the update description I gave you.
- `# IMPROVEMENT PLAN`.
  - `## STEP X: The Step Title`: One step in the improvement plan.
    - A clear description of what to change in the source code.
    - A clear explanation of why this change is necessary.
- `# TEST PLAN`.

## Step 1. Identify the Problem

- The design document is in `Copilot_Task.md`. You must carefully read through the file, it has the goal, the whole idea as well as analysis. If `Copilot_Task.md` mentions anything about updating the knowledge base, ignore it.
- Find `# Update` in the LATEST chat message. Ignore any `# Update` in the chat history.
- If there is an `# Update` section: it means I am going to propose some change to `Copilot_Planning.md`.
  - Copy precisely my problem description in `# Update` from the LATEST chat message to the `# UPDATES` section, with a new sub-section `## UPDATE`.
  - Follow my update to change the planning document.
- If there is nothing:
  - If `Copilot_Planning.md` only has a title, you are on a fresh start.
    - Add an empty `# UPDATES` section after the title, if it does not already exist.
  - Otherwise, it means you are accidentally stopped. Please continue your work.
    - Read `Copilot_Planning.md` througly, it is highly possibly that you were working on the request described in the last section in `# UPDATES`.

## Step 2. Understand the Goal and Quality Requirement

- You need to write complete two main sections in `Copilot_Planning.md`, an improvement plan and a test plan.
- **Improvement Plan**:
  - Read through and understand the task in `Copilot_Task.md`.
  - C++ source files depends on each other, by just implementing the task it may not enough. Find out what will be affected.
  - Propose any code change you would like to do. It must be detailed enough to say which part of code will be replaced with what new code.
  - Explain why you want to make these changes.
  - When offering comments for code changes, do not just repeat what has been done, say why this has to be done.
    - If the code is simple and obvious, no comment is needed. Actually most of the code should not have comments.
    - Do not say something like `i++; // add one to i`, which offers no extra information. Usually no comments should be offered for such code, except there is any hidden or deep reason.
- **Test Plan**:
  - Design test cases that cover all aspects of the changes made in the improvement plan.
  - Ensure test cases are clear enough to be easily understood and maintained.
  - Carefully think about corner cases to cover.
  - For refactoring work, existing test cases might have already covered most of the scenarios. Carefully review them and only add new test cases if necessary.
  - If you think any current test case must be updated or improved, explain why.

## Step 3. Finish the Document

- Your goal is to write a design document to `Copilot_Planning.md`. DO NOT update any other file including source code.
- The code change proposed in the improvement plan must contain actual code. I need to review them before going to the next phrase.
- DO NOT copy `# UPDATES` from `Copilot_Task.md` to `Copilot_Planning.md`.

## Step 4. Mark the Completion

- Ensure there is a `# !!!FINISHED!!!` mark at the end of `Copilot_Planning.md` to indicate the document reaches the end.

# External Tools Environment and Context

- You are on Windows running in Visual Studio Code.
- In order to achieve the goal, you always need to create/delete/update files, build the project, run the unit test, etc. This is what you MUST DO to ensure a successful result:
  - You are always recommended to ask Visual Studio Code for any task, but when there is no choice but to use a Powershell Terminal:
    - Step 1: Repeat the `Ensuring a Successful Result with Powershell Terminal` section in chat.
    - Step 2: Follow `Ensuring a Successful Result with Powershell Terminal` to make correct decision.

## Ensuring a Successful Result with Powershell Terminal

- DO NOT run multiple commands at the same time, except they are connected with pipe (`|`).
- DO NOT call `msbuild` or other executable files by yourself.
- DO NOT create any new file unless explicitly directed.
- MUST run any powershell script in this format: `& absolute-path.ps1 parameters...`.
- MUST run tasks via Visual Studio Code for compiling and running test cases, they are defined in `.vscode/tasks.json`, DO NOT change this file.
- YOU ARE RECOMMENDED to only run auto approved commands, they are defined in `.vscode/settings.json`, DO NOT change this file.

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

# Accessing Log Files and PowerShell Scripts

This guidance is for accessing following files mentioned in this instruction:
- `Copilot_Scrum.md`
- `Copilot_Task.md`
- `Copilot_Planning.md`
- `Copilot_Execution.md`
- `Copilot_KB.md`
- `copilotPrepare.ps1`
- `copilotBuild.ps1`
- `copilotExecute.ps1`

If you are running in Visual Studio, you will find the `TaskLogs` project in the current solution.
Otherwise, locate the `TaskLogs` project in `REPO-ROOT/.github/TaskLogs/TaskLogs.vcxitems`.
`REPO-ROOT` is the root folder of the repo.

`TaskLogs.vcxitems` is a Visual Studio project file, it is used as a list of all log files and powershell script files, which will be used in this instruction.
You need to locate listed files in `TaskLogs.vcxitems`.

## Important Rules for Markdown Document or Log

- Do not print "````````" or "````````markdown" in markdown file.
- It is totally fine to have multiple top level `# Topic`.
- When mentioning a C++ name in markdown file:
  - If it is defined in the standard C++ library or third-party library, use the full name.
  - If it is defined in the source code, use the full name if there is ambiguity, and then mention the file containing its definition.

