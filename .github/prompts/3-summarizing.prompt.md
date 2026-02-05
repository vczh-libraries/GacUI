# Summarizing

- Check out `Accessing Task Documents` and `Accessing Script Files` for context about mentioned `*.md` and `*.ps1` files.
- All `*.md` and `*.ps1` files should exist; you should not create any new files unless explicitly instructed.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.

## Goal and Constraints

- Your goal is to finish an execution document in `Copilot_Execution.md` according to `Copilot_Task.md` and `Copilot_Planning.md`.
- You are only allowed to update `Copilot_Execution.md`.
- You are not allowed to modify any other files.
- The phrasing of the request may look like asking for code change, but your actual work is to write the design document.

## Copilot_Execution.md Structure

- `# !!!EXECUTION!!!`: This file always begins with this title.
- `# UPDATES`: For multiple `## UPDATE` sections. It should always exist even there is no update.
  - `## UPDATE`: There could be multiple occurrences. Each one has an exact copy of the update description I gave you.
- `# AFFECTED PROJECTS`.
- `# EXECUTION PLAN`.
- `# FIXING ATTEMPTS`.

## Step 1. Identify the Problem

- The design document is in `Copilot_Task.md`, the planning document is in `Copilot_Planning.md`.
- Find `# Problem` or `# Update` in the LATEST chat message.
  - Ignore any of these titles in the chat history.
  - If there is nothing:
    - If there is a `# !!!FINISHED!!!` mark in `Copilot_Execution.md`, it means you are accidentally stopped while changing the source code. Please continue your work.
    - If there is no `# !!!FINISHED!!!` mark in `Copilot_Execution.md`, it means you are accidentally stopped while finishing the document. Please continue your work.

### Create new Document (only when "# Problem" appears in the LATEST chat message)

Ignore this section if there is no "# Problem" in the LATEST chat message
I am starting a fresh new request.

- Add an empty `# UPDATES` section after `# !!!EXECUTION!!!`.
- You are going to complete an execution document according to `Copilot_Planning.md`.

### Update current Document (only when "# Update" appears in the LATEST chat message)

Ignore this section if there is no "# Update" in the LATEST chat message
I am going to propose some change to `Copilot_Execution.md`.

- Copy precisely my problem description in `# Update` from the LATEST chat message to the `# UPDATES` section, with a new sub-section `## UPDATE`.
- The new `## UPDATE` should be appended to the end of the existing `# UPDATES` section (aka before `# AFFECTED PROJECTS`).
- Follow my update to change the execution document.

## Step 2. Finish the Document

- You need to summarize code change in `Copilot_Execution.md`.
- All changes you need to make are already in `Copilot_Planning.md`, but it contains many explanations.
- Read `Copilot_Planning.md`, copy the following parts to `Copilot_Execution.md`:
  - `# EXECUTION PLAN`
    - Copy EVERY code block exactly as written
    - If Planning has 1000 lines of test code, Execution must have those same 1000 lines
    - Remove only the explanatory text between code blocks
    - Keep ALL actual code
  - DO NOT copy `# UPDATES` from `Copilot_Planning.md` to `Copilot_Execution.md`. The `# UPDATES` in `Copilot_Execution.md` is for update requests for `Copilot_Execution.md` and the actual source code.
  - In each code change, ensure the context information is complete:
    - Which file to edit?
    - Insert/Delete/Update which part of the file? Is it better to define "which part" by line number or surrounding code?
    - The code block to be written to the file.
    - In `Copilot_Planning.md`, the code block might be incomplete or containing above metadata, do not update `Copilot_Planning.md` but instead fix them in `Copilot_Execution.md` following the rule:
      - Each code block only contain consecutive code to be written to the file.
      - If the original code block contains metadata, do not include it.
      - If the original code block contains code change in multiple places or even multiple files, split it.
      - If the original code block omits surrounding code that is necessary to understand the change, expand it to complete.

## Step 3. Document Quality Check List

- Is `Copilot_Execution.md` contains enough information so that one can follow the document to make actual code change, without having to refer to `Copilot_Planning.md`?
- Does `Copilot_Execution.md` include all code changes mentioned in `Copilot_Planning.md`?
- Fill the `# AFFECTED PROJECTS` section:
  - Solutions and projects you need to work on could be found in `REPO-ROOT/.github/Project.md`.
  - When creating `Copilot_Execution.md` from the first time, copy `# AFFECTED PROJECTS` section from `Copilot_Planning.md`. Otherwise, review the list whenever `Copilot_Execution.md` is updated, and fix this section in the following format:
    - Identify affected solutions, write `- Build the solution in folder <SOLUTION-ROOT>`.
    - For each solution, identify affected unit test projects, write `  - Run Test Project <PROJECT-NAME>`.
      - The list should only include unit test projects.

## Step 4. Completion
- Ensure there is a `# !!!FINISHED!!!` mark at the end of `Copilot_Execution.md` to indicate the document reaches the end.
