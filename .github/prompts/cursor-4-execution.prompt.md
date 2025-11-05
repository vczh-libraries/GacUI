# Execution

## Goal and Constraints

- You are going to apply changes on the source code as well following `Copilot_Execution.md`.

## Copilot_Execution.md Structure

- `# !!!EXECUTION!!!`: This file always begin with this title.
- `# UPDATES`:
  - `## UPDATE`: There could be multiple occurrences. Each one has an exact copy of the update description I gave you.
- `# IMPROVEMENT PLAN`.
- `# TEST PLAN`.
- `# FIXING ATTEMPTS`.

## Step 1. Identify the Problem

- The execution document is in `Copilot_Execution.md`
- Find `# Update` in the LATEST chat message. Ignore any `# Update` in the chat history.
- If there is an `# Update` section: it means I am going to propose some change to `Copilot_Execution.md` and the source code together.
  - Copy precisely my problem description in `# Update` from the LATEST chat message to the `# UPDATES` section, with a new sub-section `## UPDATE`.
  - Follow my update to change the execution document and the source code.
- If there is nothing:
  - Apply all code changes in `Copilot_Execution.md` to the source code.
  - After applying each step in `Copilot_Execution.md`, mark the step as completed by appending `[DONE]` after the step title. This allow you to find where you are if you are interrupted.

## Step 2. Make Sure the Code Compiles but DO NOT Run Unit Test

- Check out `Compile the Solution` for details about compiling the solution but DO NOT run unit test.
  - `Compile the Solution` is the only way to build the project. DO NOT call any other tools or scripts.
- Find out if there is any warning or error.
  - `Compile the Solution` has the instruction about how to check compile result.
- If there is any compilation error, address all of them:
  - If there is any compile warning, only fix warnings that caused by your code change. Do no fix any other warnings.
  - If there is any compile error, you need to carefully identify, is the issue in the callee side or the caller side. Check out similar code before making a decision.
  - For every attempt of fixing the source code:
    - Explain why the original change did not work.
    - Explain what you need to do.
    - Explain why you think it would solve the build break.
    - Log these in `Copilot_Execution.md`, with section `## Fixing attempt No.<attempt_number>` in `# FIXING ATTEMPTS`.
  - Go back to `Step 2. Make Sure the Code Compiles but DO NOT Run Unit Test`.
- When the code compiles:
  - DO NOT run any tests, the code will be verified in future tasks.

# External Tools Environment and Context

- You are on Windows running in Cursor.
- Submitting CLI commands is not recommended unless you have no choice.
- There is some rules to follow to submit correct powershell commands:
  - DO NOT call `msbuild` or other executable files by yourself.
  - DO NOT create any new file unless explicitly directed.
  - MUST run any powershell script in this format: `& absolute-path.ps1 parameters...`.
  - MUST run tasks via Cursor for compiling and running test cases.

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
- `Build.log`
- `Execute.log`

They are in the `REPO-ROOT/.github/TaskLogs` folder.

`REPO-ROOT` is the root folder of the repo.

## If you are running in Visual Studio

You will find the `TaskLogs` project in the current solution, which should contain these files.

## Important Rules for Markdown Document or Log

- Do not print "````````" or "````````markdown" in markdown file.
- It is totally fine to have multiple top level `# Topic`.
- When mentioning a C++ name in markdown file:
  - If it is defined in the standard C++ library or third-party library, use the full name.
  - If it is defined in the source code, use the full name if there is ambiguity, and then mention the file containing its definition.

# Unit Test Projects to Work with

REPO-ROOT is the absolute path of the current repo.

## Calling copilotBuild.ps1

```
cd REPO-ROOT\Test\GacUISrc
& ..\..\.github\TaskLogs\copilotBuild.ps1
```

## Calling copilotExecute.ps1

```
cd REPO-ROOT\Test\GacUISrc
& ..\..\.github\TaskLogs\copilotExecute.ps1 -Executable UnitTest
```

## Compile the Solution

- Run the `copilotBuild.ps1` script. Check out `Unit Test Projects to Work with` for details.

## Executing Unit Test

- Run the `copilotExecute.ps1` script. Check out `Unit Test Projects to Work with` for details.

