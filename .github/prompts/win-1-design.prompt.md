# Design

- Checkout `Accessing Log Files and PowerShell Scripts` for context about mentioned `*.md` and `*.ps1` files.
  - All `*.md` and `*.ps1` files should already be existing, you should not create any new files.

## Goal and Constraints

- Your goal is to finish a design document in `Copilot_Task.md` to address a problem.
- You are only allowed to update `Copilot_Task.md` and mark a task completed in `Copilot_Scrum.md`.
- You are not allowed to modify any other files.
- Anything in the instruction is a guidance to complete `Copilot_Task.md`.
- DO NOT modify any source code.

## Copilot_Task.md Structure

- `# !!!TASK!!!`: This file always begin with this title.
- `# PROBLEM DESCRIPTION`: An exact copy of the problem description I gave you.
- `# UPDATES`:
  - `## UPDATE`: There could be multiple occurrences. Each one has an exact copy of the update description I gave you.
- `# INSIGHTS AND REASONING`.

## Step 1. Identify the Problem

- The problem I would like to solve is in the chat messages sending with this request.
- Find `# Problem` or `# Update` in the LATEST chat message. Ignore any `# Problem` or `# Update` in the chat history.
- If there is a `# Problem` section: it means you are on a fresh start.
  - Find and execute `copilotPrepare.ps1` to clean up everything from the last run.
  - After `copilotPrepare.ps1` finishes, copy precisely my problem description in `# Problem` from the LATEST chat message under a `# PROBLEM DESCRIPTION`.
    - If the problem description is `Next`:
      - Find the first incomplete task in `Copilot_Scrum.md`, and follow the intruction below to process that task.
    - If the problem description is like `Complete task No.X`:
      - Locate the specific task in `Copilot_Scrum.md`.
      - There is a bullet list of all tasks at the beginning of `# TASKS`. Mark the specific task as being processed by changing `[ ]` to `[x]`.
      - Find the details of the specific task, copy everything in this task to `# PROBLEM DESCRIPTION`.
- If there is an `# Update` section: it means I am going to propose some change to `Copilot_Task.md`.
  - Copy precisely my problem description in `# Update` from the LATEST chat message to the `# PROBLEM DESCRIPTION` section, with a new sub-section `## UPDATE`.
  - Follow my update to change the design document.
- If there is nothing: it means you are accidentally stopped. Please continue your work.
  - Read `Copilot_Task.md` througly, it is highly possibly that you were working on the request described in the last section in `# PROBLEM DESCRIPTION`.

## Step 2. Understand the Goal and Quality Requirement

- Analyse the source code and provide a high-level design document.
- The design document must present your idea, about how to solve the problem in architecture-wide level.
- The design document must describe the what to change, keep the description in high-level without digging into details about how to update the source code.
- The design document must explain the reason behind the proposed changes.
- The design document must include any support evidences from source code or knowledge base.
- It is completely OK and even encouraged to have multiple solutions. You must explain each solution in a way mentioned above, and provide a comparison of their pros and cons.

## Step 3. Finish the Document

- Your goal is to write a design document to `Copilot_Task.md`. DO NOT update any other file including source code.
- Whatever you think or found, write it down in the `# INSIGHTS AND REASONING` section.

## Step 4. Mark the Completion

- Ensure there is a `# !!!FINISHED!!!` mark at the end of `Copilot_Task.md` to indicate the document reaches the end.

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

