# General Instruction

- `REPO-ROOT` refers to the root directory of the repository.
- `SOLUTION-ROOT` refers to the root directory of a solution (`*.sln` or `*.slnx`).
- Solutions and projects you need to work on could be found in `REPO-ROOT/Project.md`.
- Following `Leveraging the Knowledge Base`, find knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.
- Before writing to a source file, read it again and make sure you respect my parallel editing.
- If any `*.prompt.md` file is referenced, take immediate action following the instructions in that file.
- **DO NOT ASK ANY QUESTION** if you are not explicitly instructed to make questions. Run the user's task to the end.
  - The only exception will be there are serious conflict information in user's task. This is rare, try your best effort to resolve the ambiguity by yourself, and only ask when you are totally stuck.

## (Windows Specific) External Tools Environment and Context

- Always prefer the offered script files instead of direct CLI commands.
- DO NOT call `msbuild` or other executable files directly.
- DO NOT create or delete any file unless explicitly directed.
- MUST run any PowerShell script in this format: `& absolute-path.ps1 parameters...`.
- Multiple powershell commands are concatenated with `;` to be executed in one line.

## (Linux Specific) External Tools Environment and Context

- DO NOT run any powershell script file as they are for Windows only.
- The bash script `REPO-ROOT/.github/Ubuntu/build.sh` is the only script you should call for building any project. `build.sh` will also run other script files in that folder, run `chmod +x` if any script file is blocked.
- `build.sh` is usable in `REPO-ROOT/Test/Linux` or `REPO-ROOT/Test/Linux/PROJECT-NAME`. It reads the local `vmake` configuration file, generates a `makefile`, and builds it.
  - Call `build.sh -f` for full rebuild.
  - Call `build.sh` for incremental build.
  - `vmake`, `vmake.txt` and `makefile` all live in the same `Test/Linux` (or `Test/Linux/PROJECT-NAME`) folder:
    - `vmake` is the configuration file derived from MSBuild project files. When the source file list changes, the MSBuild project files are expected to be modified, and `build.sh` will regenerate the rest.
    - `vmake.txt` and `makefile` are generated files, you are not allowed to modify them.
  - Unlike offered powershell scripts, `build.sh` does not produce `Build.log` or equivalent log file, running unit test does not produce `Execute.log` or equivalent log file.
- Always use `lldb` and other interactable tools in PTY-backed tool session.
- DO NOT call `cmake`, `make`, `clang++`, `g++`, `gdb` directly. `build.sh` and `lldb` are for building and debugging.

## Coding Guidelines and Tools

The C++ project in this repo is built and tested using its own msbuild wrappers.
You must strictly follow the instructions in the following documents,
otherwise it won't work properly.

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
- `Copilot_Investigate.md`

### Important Rules for Writing Markdown Files

- Do not print "````````" or "````````markdown" in a markdown file.
- It is totally fine to have multiple top level `# Topic`.
- When mentioning a C++ name in markdown file:
  - If it is defined in the standard C++ library or third-party library, use the full name.
  - If it is defined in the source code, use the full name if there is ambiguity, and then mention the file containing its definition.

## (Windows Specific) Accessing Script Files

If you need to find any script or log files, they are in the `REPO-ROOT/.github/Scripts` folder:
- `copilotPrepare.ps1`
- `copilotPrepareReview.ps1`
- `copilotBuild.ps1`
- `copilotExecute.ps1`
- `copilotDebug_Start.ps1`
- `copilotDebug_Stop.ps1`
- `copilotDebug_RunCommand.ps1`
- `Build.log`
- `Execute.log`

## (Linux Specific) Accessing Script Files

If you need to find any script files, they are in the `REPO-ROOT/.github/Ubuntu` folder:
- `build.sh`

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
  - Always use `FilePath` for file path operations.
  - Use my own collection types vl::collections::* instead of std::*
  - Regular expression utilities are offered by `vl::regex::Regex`, here are important syntax differences with other regular expression implementation:
    - "." means the dot character, "/." or "\." (or "\\." in C++ string literal) means any character.
    - Both "/" and "\" escape characters, you are recommended to use "/" in C++ string literals.
    - Therefore you need "//" for the "/" character and "/\\" or "/\\\\" for the "\" character in C++ string literals.
    - Constructing a `Regex` object is expensive. If a regular expression is used multiple times or multiple places, make a variable to reuse it, but it should not be a global variable.
  - Check out `REPO-ROOT/.github/KnowledgeBase/Index.md` for more information of how to choose correct C++ data types.
- Rules for expression inavailability of values:
  - If any number is expected to be valid only when non-negative, you could use `-1` to represent invalid value.
  - If an object is expected to be valid only when non-null, you could use `nullptr` on `T*` or `Ptr<T>` to represent invalid value.
  - Use `Nullable<T>` to represent any invalid value if possible.
    - DO NOT use `Nullable<T*>`, `Nullable<Ptr<T>>` or `Nullable<Nullable<T>>`, this is too confusing.
  - Only when there is no other choice, use an extra `bool` variable.
    - This could happen when "null" semantic is valid.
- Rules for C++ header files:
  - Guard them with macros instead of `#pragma once`.
  - In a class/struct/union declaration, member names must be aligned in the same column at least in the same public, protected or private section.
  - Keep the coding style consistent with other header files in the same project.
- Extra Rules for C++ header files in `Source` folder:
  - Do not use `using namespace` statement, full name of types are always required.
- Rules for cpp files:
  - Use `using namespace` statement if necessary to prevent from repeating namespace everywhere.
  - `vl::stream::` is an exception, always use `stream::` with `using namespace vl;`, DO NOT use `using namespace vl::stream;`.

### Advanced C++ Coding Rules

- DO NOT make helper functions that only used once, especially it is only called in one destructor.
- When generating Workflow script, avoid building text, you should always build the AST. The AST type for a complete Workflow script module is `WfModule`.

You are always recommended to debug the compiled binary if you find it difficult to figure out what is going on, or after several failed attemps of "read and guess".

### Advanced C++ Coding Rules for Reflectable Types

- DO NOT make global variables with types that carry constructors or destructors, even when they are implicit.
  - This could mess up the order of initialization, finalization or memory leak detector.
  - One exception will be `WString` which is initialized using `WString::Unmanaged`, such constructor and destructor does not do memory management.
  - Another exception will be `Pair`, `Nullable` or `Tuple` with valid types here.
  - If pointers are needed, you could only use `T*` and do initialization or finalization explicitly. All such objects should be destroyed in `main`, `wmain`, `WinMain` or `GuiMain`, before memory leak detector runs.
- Prefer latest C++ features (up to C++ 20).
- Prefer template variadic arguments, over hard-coded-counting solutions.
- Any interface or class `X` should inherit from `vl::reflection::Description<X>`.
  - If such class (not including interface) should be inheritable in Workflow script, use `AggregatableDescription` instead of `Description`.
- No `const` is allowed for methods or reference types.
- Prefer `IValue*` interfaces for container types on interfaces.
- Container types and some other types supports range-based for loop. Always prefer range-based for loop over other loops.
  - You can used `indexed(container)` to convert a container of type `T` to `Pair<T, vint>`, to read the corrent index.
  - Avoid using expression, which creates temporary objects, in `for(... : HERE)` or `for(... : indexed(HERE))`. The current C++ destroys the temporary object too early therefore this becomes UB.
- Prefer Inverse of Control (IoC) and other design patterns, over trivial virtual functions, over switch-case on types, over if-else on types.
- Prefer static dispatching over dynamic dispatching when possible and reasonable.

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
