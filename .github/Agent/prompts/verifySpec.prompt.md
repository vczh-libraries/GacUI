# Verify Specification Files

The root of this project `REPO-ROOT/.github/Agent` is not the workspace root.
- All folders and files mentioned in the instructions are in the `REPO-ROOT/.github/Agent` folder.
- You are recommended to read `README.md` to understand the whole picture of the project as well as specification organizations.
- You are recommended to read all spec files to understand dependencies and relationships between each component.

You goal is to ensure all specifications are not outdated and have no mistakes.
- Verify spec files in `prompts/spec` folder.
- Ignore spec files in `prompts/snapshot` folder, they are a copy of `prompts/spec`.

This task is usually executed when:
- A big progress has been made in the project.
- Refactoring including file restructuring of spec files.

## Sync Spec with Source Files

Important interfaces, including types and functions, are mentioned in spec files.
You need to verify all of them against the source files.
If the spec does not match the source files, fix them.
Spec mentions types and functions everywhere, ensure all of them exist and correct.

## Verify References

There are bullet lists under `**Referenced by**:` for most of the sessions, their format is:
```
- SpecFile.md: `# This Section`, `### That Section`...
```
Each file occupys one line.
It means the behavior of those sections depend on the current section, when the current section is changed, those sections probably need to check.

Ensure file names and session names exists, and fix the list if you find any missing or oudated dependencies.

## Grammar and Typos

Fix typos and grammar or syntax mistakes in spec files.

## README.md

Scan `README.md` and ensure all information is up to date, especially file lists.
