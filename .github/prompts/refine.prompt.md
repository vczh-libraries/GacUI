# Refine

- Check out `Accessing Script Files` for context about mentioned `*.ps1` files.
- All `*.md` and `*.ps1` files should exist; you should not create any new files unless explicitly instructed.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.

## Goal and Constraints

- Your goal is to extract learnings from completed task logs and write them to learning files.
- The `KnowledgeBase` and `Learning` folders mentioned in this document are in `REPO-ROOT/.github/`.
- You are not allowed to modify any source code.
- Write learnings to these files, including not only best practices but the user's preferences:
  - `KnowledgeBase/Learning.md`: Learnings that apply across projects, including C++, library usage, and general best practices.
  - `Learning/Learning_Coding.md`: Learnings specific to this project's source code.
  - `Learning/Learning_Testing.md`: Learnings specific to this project's test code.
- Put learnings in `Learning/` instead of `KnowledgeBase/` when the knowledge is specific to this project.

## Document Structure (Learning.md, Learning_Coding.md, Learning_Testing.md)

- `# !!!LEARNING!!!`: This file always begins with this title.
- `# Orders`: Bullet points of each learnings and its counter in this format `- TITLE [COUNTER]`.
- `# Refinements`:
  - `## Title`: Learning and its actual content.

## Step 1. Find the Earliest Backup Folder

- Find and execute `copilotPrepare.ps1 -Earliest` to get the absolute path to the earliest backup folder in `Learning`.
- If the script fails, it means there is no material to learn from, stops. Otherwise continue to process this folder.

## Step 2. Read All Documents

- Read all files in the earliest backup folder. These may include:
  - `Copilot_Task.md`
  - `Copilot_Planning.md`
  - `Copilot_Execution.md`
  - `Copilot_Execution_Finding.md`

## Step 3. Extract Findings

- Focus on the following sections across all documents:
  - All `## UPDATE` sections in each document.
  - `# Comparing to User Edit` from `Copilot_Execution_Finding.md`.
- From these sections, identify learnings about:
  - Best practices and coding preferences.
  - Mistakes made and corrections applied.
  - Patterns the user prefers or dislikes.
  - Any insight into the user's philosophy about code quality, style, or approach.

## Step 4. Write Learnings

- For each finding, determine the appropriate learning file based on the categorization in `Goal and Constraints`.
- Each finding must have a short title that includes the key idea.
  - This document will be read by you in the future.
  - Even when I would like to see a short title and concentrated content, you should still ensure both title and content:
    - Include enough constraints so that you know clearly what it actually covers.
    - For example, when mentioning a function name, if the naming is too general, including the its class name or namespace is always a good idea.
- You must determine if the finding is new or matches an existing learning:
- If the finding is new, add `- TITLE [1]` to `# Orders` and add a new `## Title` section under `# Refinements` with the detailed description.
- If the finding matches an existing entry in `# Orders`, increase its counter.
  - When the finding does not conflict with the existing content, you can modify the content.
  - Otherwise, keep the counter, update the content
    - It happens when I improved and have a different idea with what I used to agree.
- Keep `# Orders` sorted by counter in descending order.

## Step 5. Delete the Processed Folder

- After all learnings from the earliest backup folder have been written, delete the earliest backup folder that was processed.
  - No continuation for the second round required.
