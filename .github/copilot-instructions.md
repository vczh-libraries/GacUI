# General Instruction

- `REPO-ROOT` refers to the root directory of the repository.
- `SOLUTION-ROOT` refers to the root directory of a solution (`*.sln` or `*.slnx`).
- Solutions and projects you need to work on could be found in `REPO-ROOT/.github/Project.md`.
- Following `Leveraging the Knowledge Base`, find knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.
- Before writing to a source file, read it again and make sure you respect my parallel editing.
- If any `*.prompt.md` file is referenced, take immediate action following the instructions in that file.

## External Tools Environment and Context

- If you are on Windows OS:
  - Always prefer the offered script files instead of direct CLI commands.
  - DO NOT call `msbuild` or other executable files directly.
  - DO NOT create or delete any file unless explicitly directed.
  - MUST run any PowerShell script in this format: `& absolute-path.ps1 parameters...`.

- If you are on Linux, offered powershell script files won't work and here are replacements:
  - You still need to maintain `*.sln`, `*.slnx`, `*.vcxitems`, `*.vcxproj`, `*.vcxproj.filters`.
  - DO NOT call `cmake` (as cmake is not in use), `make`, `clang++`, `g++`.
  - DO NOT call `gdb` or `lldb` unless you can interact with it, otherwise a running debugger will cause subsequent building to fail.
  - All `makefile` files are generated out of these solution and project files.
  - All `vmake` files are in `REPO-ROOT/Test/Linux` or its sub folders.
    - If `vmake` is directly in that folder, that is the only project you can and need to work on.
    - Otherwise, any important `*.vcxproj` will have a corresponding folder containing the `vmake` for that project.
  - DO NOT modify `makefile` as they will be re-generated and your modification will be lost. Modify `vmake` instead. In `vmake` you can:
    - Add a `*.vcxitems` or `*.vcxproj` project to add every file they use
    - Remove C++ source files added from projects that only work for Windows
    - Add new C++ source files for Linux replacement, etc.
  - All following commands should run in the folder containing the `vmake` file:
    - `vmake --make` to generate `makefile` according to the latest content in solution and project files.
    - `vbuild --build` to incrementally build the project.
    - `vbuild --full-build` to fully rebuild the project.
    - An executable file `./Bin/UnitTest` is generated after a successful `vbuild`.

## Coding Guidelines and Tools

The C++ project in this repo is built and tested using its own msbuild wrappers.
You must strictly follow the instructions in the following documents,
otherwise it won't work properly.

- Adding/Removing/Renaming Source Files: `REPO-ROOT/.github/Guidelines/SourceFileManagement.md`
- Building a Solution: `REPO-ROOT/.github/Guidelines/Building.md`
- Running a Project:
  - Unit Test: `REPO-ROOT/.github/Guidelines/Running-UnitTest.md`
  - CLI Application: `REPO-ROOT/.github/Guidelines/Running-CLI.md` (to edit ...)
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
  - Detailed document can be found in `REPO-ROOT/.github/KnowledgeBase/Index.md` under `# Copy of Online Manual` / `## GacUI`, in the `GacUI XML Resource` item and all sub items.
- Syntax of Workflow Script (the script language in GacUI XML Resource):
  - Detailed document can be found in `REPO-ROOT/.github/KnowledgeBase/Index.md` under `# Copy of Online Manual` / `## Workflow Script`, in the `Syntax` item and all sub items.

## Accessing Task Documents

If you need to find any document for the current working task, they are in the `REPO-ROOT/.github/TaskLogs` folder:
- `Copilot_Scrum.md`
- `Copilot_Task.md`
- `Copilot_Planning.md`
- `Copilot_Execution.md`
- `Copilot_KB.md`

### Important Rules for Writing Markdown Files

- Do not print "````````" or "````````markdown" in a markdown file.
- It is totally fine to have multiple top level `# Topic`.
- When mentioning a C++ name in markdown file:
  - If it is defined in the standard C++ library or third-party library, use the full name.
  - If it is defined in the source code, use the full name if there is ambiguity, and then mention the file containing its definition.

## Accessing Script Files

If you need to find any script or log files, they are in the `REPO-ROOT/.github/Scripts` folder:
- `copilotPrepare.ps1`
- `copilotBuild.ps1`
- `copilotExecute.ps1`
- `copilotDebug_Start.ps1`
- `copilotDebug_Stop.ps1`
- `copilotDebug_RunCommand.ps1`
- `Build.log`
- `Execute.log`

## Writing C++ Code

- This project uses C++ 20, you are recommended to use new C++ 20 feature aggressively.
- All code should be cross-platform. In case when an OS feature is needed, a Windows version and a Linux version should be prepared in different files, following the `*.Windows.cpp` and `*.Linux.cpp` naming convention, and keep them as small as possible.
- DO NOT MODIFY any source code in the `Import` folder, they are dependencies.
- DO NOT MODIFY any source code in the `Release` folder, they are generated release files.
- You can modify source code in the `Source` and `Test` folder.
- Use tabs for indentation in C++ source code.
- Use double spaces for indentation for JSON or XML embedded in C++ source code.
- Use `auto` to define variables if it is doable. Use `auto&&` when the type is big or when it is a collection type.
- The project only uses a very minimal subset of the standard library. I have substitutions for most of the STL constructions. Always use mine if possible:
  - Always use `vint` instead of `int`.
  - Always use `L'x'`, `L"x"`, `wchar_t`, `const wchar_t` and `vl::WString`, instead of `std::string` or `std::wstring`.
  - Use my own collection types vl::collections::* instead of std::*
  - Regular expression utilities are offered by `vl::regex::Regex`, here are important syntax differences with other regular expression implementation:
    - "." means the dot character, "/." or "\." (or "\\." in C++ string literal) means any character.
    - Both "/" and "\" escape characters, you are recommended to use "/" in C++ string literals.
    - Therefore you need "//" for the "/" character and "/\\" or "/\\\\" for the "\" character in C++ string literals.
  - Check out `REPO-ROOT/.github/KnowledgeBase/Index.md` for more information of how to choose correct C++ data types.
- Rules for C++ header files:
  - Guard them with macros instead of `#pragma once`.
  - In a class/struct/union declaration, member names must be aligned in the same column at least in the same public, protected or private section.
  - Keep the coding style consistent with other header files in the same project.
- Extra Rules for C++ header files in `Source` folder:
  - Do not use `using namespace` statement, full name of types are always required.
- Rules for cpp files:
  - Use `using namespace` statement if necessary to prevent from repeating namespace everywhere.
  - `vl::stream::` is an exception, always use `stream::` with `using namespace vl;`, DO NOT use `using namespace vl::stream;`.

## Leveraging the Knowledge Base

- When making design or coding decisions, you must leverage the knowledge base to make the best choice.
- The main entry is `REPO-ROOT/.github/KnowledgeBase/Index.md`, it is organized in this way:
  - `## Guidance`: A general guidance that plays a super important part repo-wide.
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

Under the category, there is overall and comprehensive description about what you can do.

Under the description, there are bullet points and each item follow the format:  `- Use CLASS-NAME for blahblahblah` (If a function does not belong to a class, you can generate `Use FUNCTION-NAME ...`).
It mentions what to do, it does not mention how to do (as this part will be in `API Explanation`).
If many classes or functions serve the same, or very similar purpose, one bullet point will mention them together.

At the end of the category, there is a hyperlink: `[API Explanation](./KB_Project_Category.md)` (no space between file name, all pascal case).

### Project/Design Explanation

There are multiple topics under `Design Explanation`. Each topic begins with a short and accurate title `#### Topic`.
A topic means a feature of this project, it will be multiple components combined.

Under the topic, there is overall and comprehensive description about what does this feature do.

Under the description, there are bullet points to provide a little more detail, but do not make it too long. Full details are supposed to be in the document from the hyperlink.

At the end of the topic, there is a hyperlink: `[Design Explanation](./KB_Project_Design_Topic.md)` (no space between file name, all pascal case).

### Experiences and Learnings

(to edit ...)
