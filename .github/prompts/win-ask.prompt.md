# Analysis

- This is analysis work, DO NOT update any source code.
- Find out the `Accessing the Knowledge Base` section. Understand the organization of the knowledge base.
- You are in a large C++ project, you must try your best to read any source code that may possibly related to the analysis work.
- Follow the chat message, try your best to answer an analysis work.

# External Tools Environment and Context

- You are on Windows running in Visual Studio Code.
- In order to achieve the goal, you always need to create/delete/update files, build the project, run the unit test, etc. This is what you MUST DO to ensure a successful result:
  - You are always recommended to ask Visual Studio Code for any task, but when there is no choice but to use a Powershell Terminal:
    - Step 1: Repeat the `Ensuring a Successful Result with Powershell Terminal` section in chat.
    - Step 2: Follow `Ensuring a Successful Result with Powershell Terminal` to make correct decision.

## Ensuring a Successful Result with Powershell Terminal

- DO NOT run multiple commands at the same time, except they are connected with pipe (`|`).
- DO NOT call `msbuild` or other executable files by yourself.
- DO NOT create any new file unless explicitly directed.
- MUST run any powershell script in this format: `& absolute-path.ps1 parameters...`.
- MUST run tasks via Visual Studio Code for compiling and running test cases, they are defined in `.vscode/tasks.json`, DO NOT change this file.
- YOU ARE RECOMMENDED to only run auto approved commands, they are defined in `.vscode/settings.json`, DO NOT change this file.

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

