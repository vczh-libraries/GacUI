# Design

- Check out `Accessing Task Documents` and `Accessing Script Files` in `REPO-ROOT/.github/copilot-instructions.md` for context about mentioned `*.md` and `*.ps1` files.
- All `*.md` and `*.ps1` files should exist; you should not create any new files unless explicitly instructed.
  - The `Copilot_Scrum.md` file should already exist.
  - If you cannot find the file, you are looking at a wrong folder.
  - `Copilot_Task.md` should be put in the same folder.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.

## Goal and Constraints

- Your goal is to finish a design document in `Copilot_Task.md` to address a problem.
- You are only allowed to update `Copilot_Task.md` and mark a task being taken in `Copilot_Scrum.md`.
- You are not allowed to modify any other files.
- The phrasing of the request may look like asking for code change, but your actual work is to write the design document.

## Copilot_Task.md Structure

- `# !!!TASK!!!`: This file always begins with this title.
- `# PROBLEM DESCRIPTION`: An exact copy of the problem description I gave you.
- `# UPDATES`: For multiple `## UPDATE` sections. It should always exist even there is no update.
  - `## UPDATE`: There could be multiple occurrences. Each one has an exact copy of the update description I gave you.
- `# INSIGHTS AND REASONING`.
- `# AFFECTED PROJECTS`.

## Step 1. Identify the Problem

- The problem I would like to solve is in the chat messages sent with this request.
- Find `# Problem` or `# Update` in the LATEST chat message.
  - Ignore any of these titles in the chat history.
  - If there is nothing: it means you are accidentally stopped. Please continue your work.
    - Read `Copilot_Task.md` thoroughly, it is highly possible that you were working on the request described in the last section in `# PROBLEM DESCRIPTION`.

### Create new Document (only when "# Problem" appears in the LATEST chat message)

Ignore this section if there is no "# Problem" in the LATEST chat message
I am starting a fresh new request.

- Find and execute `copilotPrepare.ps1` to clean up everything from the last run.
- After `copilotPrepare.ps1` finishes, copy precisely my problem description in `# Problem` from the LATEST chat message under a `# PROBLEM DESCRIPTION`.
  - If the problem description is `Next`:
    - Find the first incomplete task in `Copilot_Scrum.md`.
  - If the problem description is like `Complete task No.X`:
    - Locate the specific task in `Copilot_Scrum.md`.
  - There is a bullet list of all tasks at the beginning of `# TASKS`. Mark the specific task as being taken by changing `[ ]` to `[x]`.
  - Find the details of the specific task, copy everything in this task to `# PROBLEM DESCRIPTION`.
- Add an empty `# UPDATES` section after `# PROBLEM DESCRIPTION`.

### Update current Document (only when "# Update" appears in the LATEST chat message)

Ignore this section if there is no "# Update" in the LATEST chat message
I am going to propose some change to `Copilot_Task.md`.

- Copy precisely my problem description in `# Update` from the LATEST chat message to the `# PROBLEM DESCRIPTION` section, with a new sub-section `## UPDATE`.
- The new `## UPDATE` should be appended to the end of the existing `# UPDATES` section (aka before `# INSIGHTS AND REASONING`).
- Follow my update to change the design document.

## Step 2. Understand the Goal and Quality Requirement

- Analyse the source code and provide a high-level design document.
- The design document must present your idea, about how to solve the problem in architecture-wide level.
- The design document must describe the what to change, keep the description in high-level without digging into details about how to update the source code.
- The design document must explain the reason behind the proposed changes.
- The design document must include any support evidences from source code or knowledge base.

### Tips about Designing

- Leverage the source code from dependencies in `Import` folder as much as you can.
- Source code in the `Source` and `Test` folder is subject to modify.
- The project should be highly organized in a modular way. In most of the cases you are using existing code as API to complete a new feature.
- If you think any existing API in the current project should offer enough functionality, but it is currently missing something:
  - Such issue may prevent the current task from being able to complete.
    - You should point it out.
    - I need such information to review incomplete tasks.
    - If the current task cannot be completed without fixing this issue, it is acceptable to only having the analysis.
  - DO NOT make assumption that you can't prove.
- If you have multiple proposals for a task:
  - List all of them with pros and cons.
  - You should decide which is the best one.

## Step 3. Finish the Document

- Your goal is to write a design document to `Copilot_Task.md`. DO NOT update any other file including source code.
- Whatever you think or found, write it down in the `# INSIGHTS AND REASONING` section.
- Fill the `# AFFECTED PROJECTS` section:
  - Solutions and projects you need to work on could be found in `REPO-ROOT/.github/Project.md`.
  - Complete this section in this format:
    - Identify affected solutions, write `- Build the solution in folder <SOLUTION-ROOT>`.
    - For each solution, identify affected unit test projects, write `  - Run Test Project <PROJECT-NAME>`.
      - The list should only include unit test projects.

## Step 4. Mark the Completion

- Ensure there is a `# !!!FINISHED!!!` mark at the end of `Copilot_Task.md` to indicate the document reaches the end.
