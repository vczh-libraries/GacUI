# Refine

- Check out `Accessing Task Documents`, `(Windows Specific) Accessing Script Files`, and `(Linux Specific) Accessing Script Files` in `REPO-ROOT/.github/copilot-instructions.md` for context about mentioned `*.md`, `*.ps1` and `*.sh` files.
- All `*.md`, `*.ps1` and `*.sh` files should exist; you should not create any new files unless explicitly instructed.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find the knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.

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
- `# Orders`: Bullet points of each learning and its counter in this format `- TITLE [COUNTER]`.
- `# Refinements`:
  - `## Title`: Learning and its actual content.

## Step 1. Find the Earliest Backup Folder

- Firstly you need to do a simple clean up:
  - Folder names are time stamps, you need to read a few of earilest documents.
  - Sometimes the same document will be backed up multiple times with new content increasing as instructed in the original request.
  - You should identify them, and leave the latest one while deleting earlier ones. The kept document should contain the most content.
  - Stop cleaning up at the first group of documents. Or when the second earilest document has a different task than the earliest one, you will have nothing to delete and just stops the cleaning up.
  - You can use `copilotRemember.ps1 -Earliest COUNT` to help you identify the earliest COUNT backup folders.
- Secondly, pick the earliest backup folder after cleaning up and continue.

## Step 2. Read All Documents

- Read all files in the earliest backup folder. These may include:
  - `Copilot_Investigate.md`

## Step 3. Extract Findings

- Extract useful information that reflect my preferences:
  - `# PROBLEM DESCRIPTION` section, you need to careful read the original request in this section and tell what it wanted to achieve:
    - If it is a feature request, ignore it.
    - If it is a refactoring request, pay attention to the reason for refactoring and the expected improvement. This usually reflect my preferences.
  - `## UPDATE` sections under `# UPDATES`, usually contains my corrections to previous proposals in the document.
- Try to extract knowledges from the above content, focus on:
  - Coding guidelines, best practices and preferences for using any specific language or library.
  - Software design principles, best practices for engineering.
  - Knowledges that can be generalized across projects.
  - Knowledges that is useful for maintaining this project.

## Step 4. Write Learnings

- For each finding, determine the appropriate learning file based on the categorization in `Goal and Constraints`.
- Each finding must have a short title that includes the key idea.
  - This document will be read by you in the future.
  - Even when I would like to see a short title and concentrated content, you should still ensure both title and content:
    - Include enough constraints so that you know clearly what it actually covers.
    - For example, when mentioning a function name, if the name is too general, including its class name or namespace is always a good idea.
- You must determine whether the finding is new or matches an existing learning:
  - If the finding is new, add `- TITLE [1]` to `# Orders` and add a new `## Title` section under `# Refinements` with the detailed description.
  - If the finding matches an existing entry in `# Orders`, increase its counter.
    - When the finding does not conflict with the existing content, you can modify the content.
    - Otherwise, keep the counter and update the content.
      - This happens when I improve something and change my mind about a previous preference.
- Keep `# Orders` sorted by counter in descending order.

## Step 5. Delete the Processed Folder

- After all learnings from the earliest backup folder have been written, delete the earliest backup folder that was processed.
  - No continuation for a second round is required.
