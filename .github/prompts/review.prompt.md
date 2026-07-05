# Review Task for Investigation

- Check out `Accessing Task Documents`, `(Windows Specific) Accessing Script Files`, and `(Linux Specific) Accessing Script Files` in `REPO-ROOT/.github/copilot-instructions.md` for context about mentioned `*.md`, `*.ps1` and `*.sh` files.
- All `*.md`, `*.ps1` and `*.sh` files should exist; you should not create any new files unless explicitly instructed.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find the knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.

## Goal and Constraints

- Your goal is to review a task about to be investigated.
  - The task will be in a tagged file,
  - You are going to change this file and put additional review comments in it.
- You are not allowed to modify any source code, unless directly instructed in answers to review comments.
- You are not going to execute the task, you are just reviewing it.

## Identify the Task File

- The task file will be tagged in the LATEST chat message.

## Format of Review Comments

- Every review comment should have its own section, like:
```
## REVIEW COMMENTS

### SHORT DESCRIPTION

**review comment**: the first round of the review comment

I will put my reply here

**review comment**: the second round of the review comment

I will put my reply here

...

### ANOTHER REVIEW COMMENT

...
```

## Response to my Replies

- Find `## REVIEW COMMENTS` 
  - If you cant find this section, it means no review has been performed yet, skip this step.
- Read all replies to the review comments which are not tagged with `[CLOSED]`. There will be basically two categories:
  - I agree with your comment, which points out that something is wrong, and I instructed you to change in the reply:
    - Delete that specific comment section.
    - Update the content of the task file or source code accordingly.
      - Such code changes are usually interfaces or declarations to be implemented.
      - No need to do building or testing, if the code does not compile, it is fine, as all following work will be done when executing the task.
  - I agree with your comment, which points out missing details in the task. Or I disagree with your comment. Or I provide detailed explanation:
    - If you agree with my reply:
      - Keep the comment and the reply, as this will be taken as important hints when executing the task.
      - Append `[CLOSED]` after the specific comment section, so you don't need to process them the next time.
    - If you have further comments:
      - Put new comments.
      - DO NOT tag it with `[CLOSED]`.

## Review the Task

- If there is no `## REVIEW COMMENTS` section, add it to the very last of the file.
- All new review comments should be added under this section, to the very last of the file:
- You are going to review the task and figure out if:
  - There is any ambiguity or conflict in the task.
  - There is anything unreasonable, it includes but not limited to:
    - Interface to be implemented does not make sense, such interface could be in the task file or in the source code.
    - Potential error handling issues.
    - Potential synchronization and multi-threading issues.
    - Potential performance issues.
- Take into accounts of my replies to all review comments.
