# Scrum

- Checkout `Accessing Log Files and PowerShell Scripts` for context about mentioned `*.md` and `*.ps1` files.
  - All `*.md` and `*.ps1` files should already be existing, you should not create any new files.

## Goal and Constraints

- Your goal is to finish a design document in `Copilot_Scrum.md` to address a problem.
- You are only allowed to update `Copilot_Scrum.md`.
- You are not allowed to modify any other files.

## Copilot_Scrum.md Structure

- `# !!!SCRUM!!!`: This file always begin with this title.
- `# DESIGN REQUEST`: An exact copy of the problem description I gave you.
- `# UPDATES`:
  - `## UPDATE`: There could be multiple occurrences. Each one has an exact copy of the update description I gave you.
- `# TASKS`:
  - A bullet list of all tasks, each task is in the format of `- [ ] TASK No.X: The Task Title`.
  - `## TASK No.X: The Task Title`: A task.
    - A comprehensive description about the goal of this task.
    - `### what to be done`: A clear definition of what needs to be changed or implemented.
    - `### how to test it`: A clear definition of what needs to be tested in unit test.
    - `### rationale`: Reasons about why you think it is necessary to have this task, why you think it is the best for the task to be in this order.
- `# Impact to the Knowledge Base`:
  - `## ProjectName`: What needs to be changed or added to the knowledge base in this project.

## Step 1. Identify the Problem

- The problem I would like to solve is in the chat messages sending with this request.
- Find `# Problem` or `# Update` in the LATEST chat message. Ignore any `# Problem` or `# Update` in the chat history.
- If there is a `# Problem` section: it means I am starting a fresh new request.
  - You should override `Copilot_Scrum.md` with only one title `# !!!SCRUM!!!`.
  - After overriding, copy precisely my problem description in `# Problem` from the LATEST chat message under `# DESIGN REQUEST`.
- If there is an `# Update` section: it means I am going to propose some change to `Copilot_Scrum.md`.
  - Copy precisely my problem description in `# Update` from the LATEST chat message to the `# DESIGN REQUEST` section, with a new sub-section `## UPDATE`.
  - Follow my update to change the design document.
- If there is a `# Learn` section: it means I made important updates during the execution of the last task, you should apply them smart enough to future tasks. Find the `Optional Step for Learning` section for more instruction.
- If there is nothing: it means you are accidentally stopped. Please continue your work.

## Step 2. Understand the Goal and Quality Requirement

- Your goal is to help me break down the problem into small tasks, write it down to `Copilot_Scrum.md`.
- Each task should be small enough to only represent a single idea or feature upgrade.
- Each task should be big enough to deliver a complete piece of functionality, do not make a task that only add code that will not be executed.
- The order of tasks is important. At the end of each task, the project should be able to compile and test.
- During making the design document:
  - Take in consideration of the knowledge base, finding anything that would be helpful for the current problem.
  - Read through the code base carefully. The project is complicated, one small decision may have widely impact to the other part of the project.
  - Think thoroughly.

## Step 3. Finish the Document

- Break the problem in to tasks.
- In each task, describe a task in a high level and comprehensive description.
  - A task must be at least updating some code, it must not be like learning or analysing or whatever just reading. Reading, thinking and planning is your immediate work to do.
- Following the description, there should also be:
  - `### what to be done`: A clear definition of what needs to be changed or implemented.
    - Keep it high-level, you can mention what should be done to update a certain group of classes, but do not include any actual code change.
  - `### how to test it`: A clear definition of what needs to be tested in unit test.
    - The test plan is about writing unit test cases. Do not include end-to-end tests or manual tests.
    - Sometimes it is difficult to make relevant unit test, for example a change to the UI, you can skip the test plan but you need to provide a rationale for it.
    - You do not need to design test cases or think about code coverage at this moment. Instead consider about how to ensure testability, how many existing components are affected so that they need to be tested, and figure out if existing test cases already covered affected components or not.
  - `### rationale`:
    - Reasons about why you think it is necessary to have this task, why you think it is the best for the task to be in this order.
    - Any support evidences from source code or knowledge base. 
      - If you can't find anything from the knowledge base, think about what can be added to the knowledge base, but you do not need to actually update the knowledge base at the moment.

## Step 4. Identify Changes to Knowledge Base

- It is important to keep the knowledge base up to date.
- You must read through relevant topics in the knowledge base, including target file of hyperlinks.
- Identify if anything in the knowledge base needs to change, as tasks could impact the knowledge base.
- Identify if anything needs to be added to the knowledge base, as tasks could introduce new concepts.
- Put them under the `# Impact to the Knowledge Base` section.
- I want the knowledge base to include only important information because:
  - Most of the information can be just easily obtained from the code.
  - Keep the knowledge base to only guidelines and design insights will have you find correct piece of code to read.
- It is fine that you find nothing to change or add to the knowledge base.

## Step 5. Mark the Completion

- Ensure there is a `# !!!FINISHED!!!` mark at the end of `Copilot_Scrum.md` to indicate the document reaches the end.

## (Optional) Step 6. Learning (only when # Learn appears in the LATEST chat message)

- Ignore this section if there is no `# Learn` in the LATEST chat message

### Step 6.1

- Identify the last completed task.

### Step 6.2

- Read through `Copilot_Execution.md`. There may be some fixing attempts, that were done by you.
- Compare existing source code with `Copilot_Execution.md`, finding what is changed.
  - During comparing, you need to take into consideration of the fixing attempts, as sometimes you didn't update the main content of the document.
- Identify all differences between the document and the source code:
  - If it is caused by any fixing attempts, ignore it.
  - If it is caused by any `# UPDATE`, ignore it.
  - If any fixing attempt was reverted:
    - It may be canceled by a further fixing attemp, ignore it.
    - Otherwise it was a user edit.
  - Any other difference is a user edit.
- If there is no `# !!!VERIFIED!!!` in `Copilot_Execution.md`, it means you failed to deliver the task, either the code did not compile or some test cases failed. My edit will also reflects the final solution to the task.
- Carefully read through and analyse all user edits, understand my tastes and preferences about the source code.
- Add your finding to `Copilot_Execution.md` at the very end with the topic `# Comparing to User Edit`.
  - If every changes are ignored by the rule above, or if you can't find any user edit, just write `No user edit found`.

### Step 6.3

- There will be multiple `# UPDATES` or `# FIXING ATTEMPTS` or `# Comparing to User Edit` sections in `Copilot_Task.md`, `Copilot_Planning.md` and `Copilot_Execution.md`.
- These 3 files recorded how you interpreted the last completed task, and how I wanted you to adjust  your understanding.
- Find out what you can learn from the updates, about my philosophy and preferences.
- Check all future tasks, apply what you have learned, and adjust your approach accordingly.

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

