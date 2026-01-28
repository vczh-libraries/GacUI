# Scrum

- Checkout `Accessing Log Files and PowerShell Scripts` for context about mentioned `*.md` and `*.ps1` files.
  - All `*.md` and `*.ps1` files should already be existing, you should not create any new files.

## Goal and Constraints

- Your goal is to finish a design document in `Copilot_Scrum.md` to address a problem.
- You are only allowed to update `Copilot_Scrum.md`.
- You are not allowed to modify any other files.
- The phrasing of the request may look like asking for code change, but your actual work is to write the design document.
- "Task" in the request always mean a task under the `# TASKS` section in the design document.

## Copilot_Scrum.md Structure

- `# !!!SCRUM!!!`: This file always begin with this title.
- `# DESIGN REQUEST`: An exact copy of the problem description I gave you.
- `# UPDATES`: For multiple `## UPDATE` sections. It should always exist even there is no update.
  - `## UPDATE`: There could be multiple occurrences. Each one has an exact copy of the update description I gave you.
- `# TASKS`:
  - A bullet list of all tasks, each task is in the format of `- [ ] TASK No.X: The Task Title`.
  - `## TASK No.X: The Task Title`: A task.
    - A comprehensive description about the goal of this task.
    - `### what to be done`: A clear definition of what needs to be changed or implemented.
    - `### rationale`: Reasons about why you think it is necessary to have this task, why you think it is the best for the task to be in this order.
- `# Impact to the Knowledge Base`:
  - `## ProjectName`: What needs to be changed or added to the knowledge base in this project.

## Step 1. Identify the Problem

- The problem I would like to solve is in the chat messages sending with this request.
- Find `# Problem` or `# Update` or `# Learn` in the LATEST chat message. 
  - Ignore any these titles in the chat history.
  - If there is nothing: it means you are accidentally stopped. Please continue your work.

### Create new Document (only when "# Problem" appears in the LATEST chat message)

Ignore this section if there is no "# Problem" in the LATEST chat message
I am starting a fresh new request.

- You should override `Copilot_Scrum.md` with only one title `# !!!SCRUM!!!`.
  - At the moment, `Copilot_Scrum.md` may contain old tasks from previous requests, even it may look like the document is already finished for the current scrum, always clean it up.
- After overriding, copy precisely my problem description in `# Problem` from the LATEST chat message under `# DESIGN REQUEST`.
- Add an empty `# UPDATES` section after `# DESIGN REQUEST`.

### Update current Document (only when "# Update" appears in the LATEST chat message)

Ignore this section if there is no "# Update" in the LATEST chat message
I am going to propose some change to `Copilot_Scrum.md`.

- Copy precisely my problem description in `# Update` from the LATEST chat message to the `# DESIGN REQUEST` section, with a new sub-section `## UPDATE`.
- The new `## UPDATE` should be appended to the end of the existing `# UPDATES` section (aka before `# TASKS`).
- When the number of tasks needs to be changed, due to inserting/removing/splitting/merging tasks:
  - Adjust task number of unaffected tasks accordingly, throughout the document.
  - Replace the affected tasks with new content, DO NOT TOUCH unaffected tasks.
- Follow my update to change the design document.

### Learning (only when "# Learn" appears in the LATEST chat message)

Ignore this section if there is no "# Learn" in the LATEST chat message
I made important updates to the source code manually during the execution of the last task.

- Skip every steps before Step 6.
- Find the `Step 6. Learning` section for more instruction.

## Step 2. Understand the Goal and Quality Requirement

- Your goal is to help me break down the problem into small tasks, write it down to `Copilot_Scrum.md`.
- Each task should be:
  - Small enough to only represent a single idea or feature upgrade.
  - Deliver a complete piece of functionality.
  - Not mixing functionality and test cases in the same task.
    - In case when new unit test is required, writing new test cases should be in separate tasks.
    - If test cases are better to be categorized and grouped with multiple `TEST_CATEGORY`, create each task for each `TEST_CATEGORY`.
    - For refactoring work, existing test cases might have already covered most of the scenarios. Carefully review them and only add new test cases if necessary.
    - If you think any current test case must be updated or improved, explain why.
  - For a test planning task:
    - The test plan is about writing unit test cases. Do not include end-to-end tests or manual tests.
    - Sometimes it is difficult to make relevant unit test, for example a change to the UI, you can skip the test plan but you need to provide a rationale for it.
    - You do not need to design test cases or think about code coverage at this moment. Instead consider about how to ensure testability, how many existing components are affected so that they need to be tested, and figure out if existing test cases already covered affected components or not.
  - Well ordered. At the end of each task, the project should be able to compile or test.
- During making the design document:
  - Take in consideration of the knowledge base, finding anything that would be helpful for the current problem.
  - Read through the code base carefully. The project is complicated, one small decision may have widely impact to the other part of the project.
  - Think thoroughly.

## Step 3. Finish the Document

- Break the problem into tasks.
- In each task, describe a task in a high level and comprehensive description.
  - A task must be at least updating some code, it must not be like learning or analysing or whatever just reading.
  - Reading, thinking and planning is your immediate work to do to complete the design document.
- Following the description, there should also be:
  - `### what to be done`: A clear definition of what needs to be changed or implemented.
    - Keep it high-level, you can mention what should be done to update a certain group of classes, but do not include any actual code change.
  - `### rationale`:
    - Reasons about why you think it is necessary to have this task, why you think it is the best for the task to be in this order.
    - Any support evidences from source code or knowledge base. 
      - If you can't find anything from the knowledge base, think about what can be added to the knowledge base, but you do not need to actually update the knowledge base at the moment.

### Tips about Designing

- Leverage the source code from dependencies in `Import` folder as much as you can.
- Source code in the `Source` and `Test` folder is subject to modify.
- The project should be highly organized in a modular way. In most of the cases you are using existing code as API to complete a new feature.
- If you think any existing API in the current project should offer enough functionality, but it is currently missing something:
  - Point it out. A separate task to update them is recommended.
  - DO NOT make assumption that you can't prove.
- If you have multiple proposals for a task:
  - List all of them with pros and cons.
  - You should decide which is the best one.

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

## Step 6. Learning (only when "# Learn" appears in the LATEST chat message)

- Ignore this section if there is no "# Learn" in the LATEST chat message

### Step 6.1. Identify the Last Complteted Task

- Identify the last completed task.
- The current `Copilot_Task.md`, `Copilot_Planning.md` and `Copilot_Execution.md` are associated to that task.

### Step 6.2. Identify My Inputs

- Read through `Copilot_Execution.md`. There may be some fixing attempts, that were done by you.
- Compare existing source code with `Copilot_Execution.md`, finding what is changed.
  - Don't rely on `git` to identify changes, as I always commit them periodaically. You need to compare the actual source code with `Copilot_Execution.md`.
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

### Step 6.3 Write Down Findings

- If every changes are ignored by the rule above, skip thi step.
- Create a new file `Copilot_Execution_Finding.md` with a topic `# Comparing to User Edit`.
  - `Copilot_Execution_Finding.md` should stay in the same folder as `Copilot_Execution.md`.
- Add your finding to `Copilot_Execution_Finding.md`.

### Step 6.4 Learn

- There will be multiple `# UPDATES` or `# FIXING ATTEMPTS` or `# Comparing to User Edit` sections in `Copilot_Task.md`, `Copilot_Planning.md` and `Copilot_Execution.md`.
- These 3 files recorded how you interpreted the last completed task, and how I wanted you to adjust  your understanding.
- Find out what you can learn from the updates, about my philosophy and preferences.
- Check all future tasks, apply what you have learned, and adjust your approach accordingly.
  - For each unfinished tasks that can be improved, update releated learnings in `Copilot_Scrum.md`.

### Step 6.5 Backup

- You must only run this step after finishing all above steps.
  - In the above steps, necessary change will be updated to `Copilot_Execution.md` and `Copilot_Scrum.md`.
- Find and execute `copilotPrepare.ps1 -Backup`. You MUST use the `-Backup` parameter.
  - Remember the first line of the output, it has the absolute path to the backup folder.
  - By running the backup script, `Copilot_Task.md`, `Copilot_Planning.md` and `Copilot_Execution.md` will be backed up and deleted.
  - If you created `Copilot_Execution_Finding.md`, in Step 6.3:
    - In the backup folder, there will be a copied `Copilot_Execution.md`, append all contents from `Copilot_Execution_Finding.md` to the end of it.
    - Delete the `Copilot_Execution_Finding.md` you created.

# External Tools Environment and Context

- You are on Windows running in Visual Studio Code.
- Submitting CLI commands is not recommended unless you have no choice.
- There is some rules to follow to submit correct powershell commands:
  - DO NOT call `msbuild` or other executable files by yourself.
  - DO NOT create or delete any file unless explicitly directed.
  - MUST run any powershell script in this format: `& absolute-path.ps1 parameters...`.
  - MUST run tasks via Cursor for compiling and running test cases.

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
- `copilotDebug_Start.ps1`
- `copilotDebug_Stop.ps1`
- `copilotDebug_RunCommand.ps1`
- `Build.log`
- `Execute.log`

They are in the `REPO-ROOT/.github/TaskLogs` folder. `REPO-ROOT` is the root folder of the repo.

## If you are running in Visual Studio

You will find the `TaskLogs` project in the current solution, which should contain these files.

## Important Rules for Markdown Document or Log

- Do not print "````````" or "````````markdown" in markdown file.
- It is totally fine to have multiple top level `# Topic`.
- When mentioning a C++ name in markdown file:
  - If it is defined in the standard C++ library or third-party library, use the full name.
  - If it is defined in the source code, use the full name if there is ambiguity, and then mention the file containing its definition.

