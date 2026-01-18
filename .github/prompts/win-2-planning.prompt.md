# Planning

- Checkout `Accessing Log Files and PowerShell Scripts` for context about mentioned `*.md` and `*.ps1` files.
  - All `*.md` and `*.ps1` files should already be existing, you should not create any new files.

## Goal and Constraints

- Your goal is to finish a planning document in `Copilot_Planning.md` to address a problem from `Copilot_Task.md`.
- You are only allowed to update `Copilot_Planning.md`.
- You are not allowed to modify any other files.
- The phrasing of the request may look like asking for code change, but your actual work is to write the design document.

## Copilot_Planning.md Structure

- `# !!!PLANNING!!!`: This file always begin with this title.
- `# UPDATES`: For multiple `## UPDATE` sections. It should always exist even there is no update.
  - `## UPDATE`: There could be multiple occurrences. Each one has an exact copy of the update description I gave you.
- `# EXECUTION PLAN`.
  - `## STEP X: The Step Title`: One step in the improvement plan.
    - A clear description of what to change in the source code.
    - A clear explanation of why this change is necessary.

## Step 1. Identify the Problem

- The design document is in `Copilot_Task.md`. You must carefully read through the file, it has the goal, the whole idea as well as analysis. If `Copilot_Task.md` mentions anything about updating the knowledge base, ignore it.
- Find `# Problem` or `# Update` in the LATEST chat message.
  - Ignore any these titles in the chat history.
  - If there is nothing: it means you are accidentally stopped. Please continue your work.
    - Read `Copilot_Planning.md` througly, it is highly possibly that you were working on the request described in the last section in `# UPDATES`.

### Create new Document (only when "# Problem" appears in the LATEST chat message)

Ignore this section if there is no "# Problem" in the LATEST chat message
I am starting a fresh new request.

- Add an empty `# UPDATES` section after `# !!!PLANNING!!!`.
- You are going to complete a more detailed planning document according to `Copilot_Task.md`.

### Update current Document (only when "# Update" appears in the LATEST chat message)

Ignore this section if there is no "# Update" in the LATEST chat message
I am going to propose some change to `Copilot_Planning.md`.

- Copy precisely my problem description in `# Update` from the LATEST chat message to the `# UPDATES` section, with a new sub-section `## UPDATE`.
- The new `## UPDATE` should be appended to the end of the existing `# UPDATES` section (aka before `# EXECUTION PLAN`).
- Follow my update to change the planning document.

## Step 2. Understand the Goal and Quality Requirement

- You need to write complete two main sections in `Copilot_Planning.md`, an improvement plan and a test plan.
- Read through and understand the task in `Copilot_Task.md`.

### Tips for a Feature Planning Task

- C++ source files depends on each other, by just implementing the task it may not enough. Find out what will be affected.
- Propose any code change you would like to do. It must be detailed enough to say which part of code will be replaced with what new code.
- Explain why you want to make these changes.
- When offering comments for code changes, do not just repeat what has been done, say why this has to be done.
  - If the code is simple and obvious, no comment is needed. Actually most of the code should not have comments.
  - Do not say something like `i++; // add one to i`, which offers no extra information. Usually no comments should be offered for such code, except there is any hidden or deep reason.

### Tips for a Test Planning Task

- Design test cases that cover all aspects of the changes made in the improvement plan.
- Ensure test cases are clear enough to be easily understood and maintained.
- Carefully think about corner cases to cover.
- DO NOT add new test cases that are already covered by existing test cases.

## Step 3. Finish the Document

- Your goal is to write a design document to `Copilot_Planning.md`. DO NOT update any other file including source code.
- The code change proposed in the improvement plan must contain actual code. I need to review them before going to the next phrase.
- DO NOT copy `# UPDATES` from `Copilot_Task.md` to `Copilot_Planning.md`.

## Step 4. Mark the Completion

- Ensure there is a `# !!!FINISHED!!!` mark at the end of `Copilot_Planning.md` to indicate the document reaches the end.

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

