# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.6: Add TestControls_Editor_RichText.cpp to unit test project

Create a new unit test source file for rich-text editing tests that target `GuiDocumentLabel` only, and wire it into the unit test project so it is compiled and executed.

This task is only for test scaffolding. It is fine to have an empty `TEST_FILE` block initially, but later tasks in this scrum will fill it with test categories and cases.

### what to be done

- Create `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` in the same physical folder as `TestControls_Editor.cpp` / `TestControls_Editor_Key.cpp`.
- Add the new file to `Test/GacUISrc/UnitTest/UnitTest.vcxproj` so it is compiled as part of the unit test binary.
- Add the new file to `Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters` under the same solution explorer folder as the existing editor test files:
	- Use `<Filter>Source Files\\Controls\\Editor</Filter>` (matching `TestControls_Editor_Key.cpp`).
- Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.user` to activate the new test file.
- Ensure the file contains a `TEST_FILE` block (can be empty in this task if needed).

### rationale

- Rich-text editing tests have different concerns (style runs, hyperlinks, mouse interaction) and are easier to evolve in a dedicated file without bloating key-navigation helpers.
- Wiring the file into the `UnitTest` project and filter early prevents later test work from being blocked by project integration details.

# UPDATES

# INSIGHTS AND REASONING

## Scope and intent

This task is intentionally “plumbing-only”: add a new source file and ensure Visual Studio builds it, places it in the correct solution explorer folder, and can conveniently run it in the debugger. Later tasks (No.7–9) will fill it with rich-text test categories for `GuiDocumentLabel`.

## Proposed changes (files and responsibilities)

### 1) Add the new test file (source scaffolding)

Create `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` as a new unit-test translation unit.

- Use the existing unit-test style and includes (e.g. `#include "TestControls.h"` and `using namespace gacui_unittest_template;`) consistent with `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`.
- Ensure it contains exactly one `TEST_FILE { ... }` block so it registers with the Vlpp unit test framework (per `.github/KnowledgeBase/KB_Vlpp_UnitTesting.md`).
- Keep it minimal in this task (an empty `TEST_FILE` body is acceptable), but make sure it compiles cleanly.

Rationale: this isolates rich-text tests (style runs, hyperlink interaction, etc.) into a dedicated file, avoiding further growth in the existing key-navigation helpers.

### 2) Wire the file into the unit test project (build integration)

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj` to include the new file in the `<ItemGroup>` containing other `ClCompile` test sources.

Evidence/pattern:
- The existing editor test files are already included as `<ClCompile Include="TestControls_Editor.cpp" />` and `<ClCompile Include="TestControls_Editor_Key.cpp" />` in `Test/GacUISrc/UnitTest/UnitTest.vcxproj`.
- Add `<ClCompile Include="TestControls_Editor_RichText.cpp" />` near these entries to keep editor-related tests grouped.

Rationale: the file must be compiled into the unit test binary so `TEST_FILE` registration runs at startup.

### 3) Place the file in the correct solution explorer folder (filters integration)

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters` to attach the new `ClCompile` entry to the editor filter folder.

Evidence/pattern:
- `TestControls_Editor_Key.cpp` is placed under `<Filter>Source Files\Controls\Editor</Filter>` in `Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters`.
- Add a matching entry for `TestControls_Editor_RichText.cpp` using the same filter string to ensure it shows up beside the other editor test files in the Solution Explorer.

Rationale: keep editor tests discoverable and consistently organized.

### 4) Activate the new test file for debugging (vcxproj.user)

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.user` (Debug|x64) so starting the unit test project runs the new test file by default.

Evidence/pattern:
- The current `LocalDebuggerCommandArguments` already uses multiple `/F:...` filters (e.g. `/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp`) to run only selected test files.
- The unit test framework explicitly supports `/F:TestFile` to run specific test files (per `.github/KnowledgeBase/KB_Vlpp_UnitTesting.md`).

Proposal:
- Append `/F:TestControls_Editor_RichText.cpp` to `LocalDebuggerCommandArguments` (keeping existing filters), so both existing editor tests and the new rich-text file run under the debugger.
- Add the resulting arguments string into `LocalDebuggerCommandArgumentsHistory` as a new entry, keeping the existing history intact.

Rationale: later tasks add tests to this file; this makes iteration fast without requiring manual debugger-argument edits each time.

# !!!FINISHED!!!
