# General Instruction

- `REPO-ROOT` refers to the root directory of the repository.
- `SOLUTION-ROOT` refers to the root directory of a solution (`*.sln` or `*.slnx`).
- Solutions and projects you need to work on can be found in `REPO-ROOT/Project.md`.
- Following `Leveraging the Knowledge Base`, find the knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.
- Before writing to a source file, read it again and make sure you respect my parallel editing.
- If any `*.prompt.md` file is referenced, take immediate action following the instructions in that file.
- **DO NOT ASK ANY QUESTION** if you are not explicitly instructed to ask questions. Run the user's task to the end.
  - The only exception is when there is seriously conflicting information in the user's task. This is rare; try your best to resolve the ambiguity by yourself, and only ask when you are totally stuck.

## (Windows Specific) External Tools Environment and Context

- Always prefer the offered script files instead of direct CLI commands.
- DO NOT call `msbuild` or other executable files directly.
- DO NOT create or delete any file unless explicitly directed.
- MUST run any PowerShell script in this format: `& absolute-path.ps1 parameters...`.
- Multiple PowerShell commands are concatenated with `;` to be executed in one line.

## (Linux Specific) External Tools Environment and Context

- DO NOT run any PowerShell script file as they are for Windows only.
- The bash script `REPO-ROOT/.github/Ubuntu/build.sh` is the only script you should call for building any project. `build.sh` will also run other script files in that folder; you may need to run `chmod +x` if any script file is blocked.
- `build.sh` is usable in `REPO-ROOT/Test/Linux` or `REPO-ROOT/Test/Linux/PROJECT-NAME`. It reads the local `vmake` configuration file, generates a `makefile`, and builds it.
  - Call `build.sh -f` for full rebuild.
  - Call `build.sh` for incremental build.
  - `vmake`, `vmake.txt` and `makefile` all live in the same `Test/Linux` (or `Test/Linux/PROJECT-NAME`) folder:
    - `vmake` is the configuration file derived from MSBuild project files. When the source file list changes, the MSBuild project files are expected to be modified, and `build.sh` will regenerate the rest.
    - `vmake.txt` and `makefile` are generated files, you are not allowed to modify them.
  - Unlike offered PowerShell scripts, `build.sh` does not produce `Build.log` or equivalent log file; running a unit test does not produce `Execute.log` or equivalent log file.
- Always use `lldb` and other interactive tools in a PTY-backed tool session.
- DO NOT call `cmake`, `make`, `clang++`, `g++`, `gdb` directly. `build.sh` and `lldb` are for building and debugging.

## Coding Guidelines and Tools

The C++ project in this repo is built and tested using its own MSBuild wrappers.
You must strictly follow the instructions in the following documents,
otherwise it won't work properly.

- **SUPER IMPORTANT** Your should always follow the coding convention when coding: `REPO-ROOT/.github/Guidelines/Coding.md`
- Adding/Removing/Renaming Source Files: `REPO-ROOT/.github/Guidelines/SourceFileManagement.md`
- Building a Solution: `REPO-ROOT/.github/Guidelines/Building.md`
- Running a Project:
  - Unit Test: `REPO-ROOT/.github/Guidelines/Running-UnitTest.md`
  - CLI Application: `REPO-ROOT/.github/Guidelines/Running-CLI.md`
  - GacUI Application: `REPO-ROOT/.github/Guidelines/Running-GacUI.md` (to edit ...)
- Debugging a Project: `REPO-ROOT/.github/Guidelines/Debugging.md`
- Using Unit Test Framework: `REPO-ROOT/.github/KnowledgeBase/manual/unittest/vlpp.md`
- Using Unit Test Framework for GacUI Application:
  - Running GacUI in Unit Test Framework: `REPO-ROOT/.github/KnowledgeBase/manual/unittest/gacui.md`
  - Snapshots and Frames: `REPO-ROOT/.github/KnowledgeBase/manual/unittest/gacui_frame.md`
  - IO Interaction: `REPO-ROOT/.github/KnowledgeBase/manual/unittest/gacui_io.md`
  - Accessing: `REPO-ROOT/.github/KnowledgeBase/manual/unittest/gacui_controls.md`
  - Snapshot Viewer: `REPO-ROOT/.github/KnowledgeBase/manual/unittest/gacui_snapshots.md`
- Syntax of GacUI XML Resources:
  - Brief Introduction: `REPO-ROOT/.github/Guidelines/GacUIXmlResource.md`
  - Detailed document can be found in `REPO-ROOT/.github/KnowledgeBase/Index.md` under `# Copy of Online Manual` / `## GacUI`, in the `GacUI XML Resource` item and all subitems.
- Syntax of Workflow Script (the script language in GacUI XML Resource):
  - Detailed document can be found in `REPO-ROOT/.github/KnowledgeBase/Index.md` under `# Copy of Online Manual` / `## Workflow Script`, in the `Syntax` item and all subitems.

## Accessing Task Documents

If you need to find any document for the current working task, they are in the `REPO-ROOT/.github/TaskLogs` folder:
- `Copilot_Investigate.md`
- `Copilot_KB.md`

### Important Rules for Writing Markdown Files

- Do not print "````````" or "````````markdown" in a Markdown file.
- It is totally fine to have multiple top-level `# Topic`.
- When mentioning a C++ name in a Markdown file:
  - If it is defined in the standard C++ library or third-party library, use the full name.
  - If it is defined in the source code, use the full name if there is ambiguity, and then mention the file containing its definition.

## (Windows Specific) Accessing Script Files

If you need to find any script or support files, they are in the `REPO-ROOT/.github/Scripts` folder:
- `copilotBuild.ps1`
- `copilotExecute.ps1`
- `copilotDebug_Start.ps1`
- `copilotDebug_Stop.ps1`
- `copilotDebug_RunCommand.ps1`
- `copilotRemember.ps1`
- `Build.log`
- `Execute.log`

## (Linux Specific) Accessing Script Files

If you need to find any script files, they are in the `REPO-ROOT/.github/Ubuntu` folder:
- `build.sh`

## Leveraging the Knowledge Base

- When making design or coding decisions, you must leverage the knowledge base to make the best choice.
- The main entry is `REPO-ROOT/.github/KnowledgeBase/Index.md`, it is organized in this way:
  - `## Guidance`: General guidance that plays an important role repo-wide.
  - Each `## Project`: A brief description of each project and its purpose.
    - `### Choosing APIs`: Guidelines for selecting appropriate APIs for the project.
    - `### Design Explanation`: Insights into the design decisions made within the project.
  - `## Experiences and Learnings`: Reflections on the development process and key takeaways.
  - `# Copy of Online Manual`: A copy of the online manual so that you don't need network access. They are much more detailed, offering selectively important information.
    - Each `## Project`: A copy of online manual for that project, organized in the same way as the knowledge base.
    - Not every project is included.
    - Manual for the unit test framework is in `## Unit Testing`.

### Project/Choosing APIs

There are multiple categories under `Choosing APIs`. Each category begins with a short and accurate title `#### Category`.
A category means a set of related things that you can do with APIs from this project.

Under the category, there is an overall and comprehensive description about what you can do.

Under the description, there are bullet points, and each item follows the format:  `- Use CLASS-NAME for blahblahblah` (If a function does not belong to a class, you can generate `Use FUNCTION-NAME ...`).
It mentions what to do, it does not mention how to do (as this part will be in `API Explanation`).
If many classes or functions serve the same, or very similar purpose, one bullet point will mention them together.

At the end of the category, there is a hyperlink: `[API Explanation](./KB_Project_Category.md)` (no space between file name, all PascalCase).

### Project/Design Explanation

There are multiple topics under `Design Explanation`. Each topic begins with a short and accurate title `#### Topic`.
A topic means a feature of this project; it can involve multiple components combined.

Under the topic, there is an overall and comprehensive description about what this feature does.

Under the description, there are bullet points to provide a little more detail, but do not make it too long. Full details are supposed to be in the document from the hyperlink.

At the end of the topic, there is a hyperlink: `[Design Explanation](./KB_Project_Design_Topic.md)` (no space between file name, all PascalCase).

### Experiences and Learnings

(to edit ...)
