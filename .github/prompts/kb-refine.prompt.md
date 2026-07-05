# Knowledge Base Refine

- Check out `Accessing Task Documents`, `(Windows Specific) Accessing Script Files`, and `(Linux Specific) Accessing Script Files` in `REPO-ROOT/.github/copilot-instructions.md` for context about mentioned `*.md`, `*.ps1` and `*.sh` files.
- All `*.md`, `*.ps1` and `*.sh` files should exist; you should not create any new files unless explicitly instructed.
  - The `Copilot_KB.md` file should already exist, it may or may not contain content from the last knowledge base writing.
  - If you cannot find the file, you are looking at a wrong folder.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find the knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.
  - `Index.md` below means the main entry file.
  - `Index_<PROJECT>.md` below means the project-specific guidance file linked from the corresponding project section in `Index.md`.

## Goal and Constraints

- Your goal is to update any knowledge base pages that fall behind the current code base.
- You are only allowed to update `Index_<PROJECT>.md` and pages referenced by it for the current repo.
- You are not allowed to modify any other files.
- Code references must be wrapped in either `single-line` or ```multi-line``` quotes.

## Identify Changes

You need to identify source changes in both the following two ways, in order to come up with a complete list of changes that need to be reflected in the knowledge base:
- Go through all files in `REPO-ROOT/.github/Learning/**/*.md`:
  - These files recorded what has been done in recent changes.
- Go through all knowledge base pages referenced by `Index_<PROJECT>.md`:
  - These files are potential candidates for updates, by go through all of them you can compare details with the source code, and find out what as been changed without putting into the the knowledge base.

## Limit Scope of Changing

- When a knowledge base page enumerates a list of something, like sub classes for a type, enum items, overloaded functions, options, etc:
  - You need to delete any items that are no longer exist.
  - You need to add any items that are not in the knowledge base.
  - You need to update any items that have been changed.
- Otherwise, when there is a complete new feature or topic that is not in the knowledge base:
  - Ignore them, we only update existing knowledges, we do not add new stuff.
- This instruction is supposed to be executed very frequently, so most of the time you will find that nothing needs to update, this is completely normal.

## Updating Knowledge Base

- You need to update the knowledge base pages according to the changes you identified in the previous step.
