# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.7: Add TestControls_Editor_InlineObject.cpp and TestControls_Editor_Styles.cpp to unit test project

Create two new unit test source files to host editor tests that are not key-navigation focused, and wire them into the unit test project so they are compiled and can be activated by `UnitTest.vcxproj.user`.

This task is only for test scaffolding. It is fine to have empty `TEST_FILE` blocks initially, but later tasks can add test categories and cases.

### what to be done

- Create `Test/GacUISrc/UnitTest/TestControls_Editor_InlineObject.cpp` in the same physical folder as `TestControls_Editor.cpp` / `TestControls_Editor_Key.cpp`.
- Create `Test/GacUISrc/UnitTest/TestControls_Editor_Styles.cpp` in the same physical folder as `TestControls_Editor.cpp` / `TestControls_Editor_Key.cpp`.
- Add both new files to `Test/GacUISrc/UnitTest/UnitTest.vcxproj` so they are compiled as part of the unit test binary.
- Add both new files to `Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters` under the same solution explorer folder as the existing editor test files:
	- Use `<Filter>Source Files\\Controls\\Editor</Filter>` (matching `TestControls_Editor_Key.cpp`).
- Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.user` to activate both new test files (following the same pattern used for other editor test files).
- Ensure each new file contains a `TEST_FILE` block (can be empty in this task if needed).

### rationale

- Splitting editor unit tests by feature area keeps `TestControls_Editor_Key.cpp` focused on keyboard behaviors and avoids turning a single file into a catch-all for unrelated editor coverage.
- Adding project / filters / user entries upfront prevents later tasks from being blocked by solution integration details, and makes it easy to activate new editor-focused tests incrementally.

# UPDATES

# INSIGHTS AND REASONING

## Scope and intent

This task is a minimal scaffolding and integration task:

- Add two new unit-test translation units to host editor-related tests that are not primarily about keyboard navigation.
- Ensure Visual Studio builds them (project inclusion), shows them in the correct solution explorer folder (filters), and can run them via `/F:` filtering (user debugger arguments).

No functional test content is required in this task beyond a compilable `TEST_FILE` block per file (per `.github/KnowledgeBase/KB_Vlpp_UnitTesting.md`).

## Proposed changes (files and responsibilities)

### 1) Add `TestControls_Editor_InlineObject.cpp` (source scaffolding)

Create `Test/GacUISrc/UnitTest/TestControls_Editor_InlineObject.cpp` modeled after existing editor test files:

- Include `TestControls.h`.
- `using namespace gacui_unittest_template;`.
- Provide exactly one `TEST_FILE { ... }` block.

Pattern evidence:

- `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` uses this minimal include + `TEST_FILE` structure and compiles with an empty category.
- `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp` shows the canonical editor-test include and namespace setup.

### 2) Add `TestControls_Editor_Styles.cpp` (source scaffolding)

Create `Test/GacUISrc/UnitTest/TestControls_Editor_Styles.cpp` with the same minimal structure as above.

Rationale: keep editor coverage split by feature area so later work can grow each file without bloating `TestControls_Editor_Key.cpp`.

### 3) Wire both files into the unit test project (build integration)

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj`:

- Add `<ClCompile Include="TestControls_Editor_InlineObject.cpp" />`
- Add `<ClCompile Include="TestControls_Editor_Styles.cpp" />`

Place them near other editor test sources (e.g. next to `TestControls_Editor.cpp`, `TestControls_Editor_Key.cpp`, `TestControls_Editor_RichText.cpp`) to keep the item group organized.

Evidence/pattern:

- `Test/GacUISrc/UnitTest/UnitTest.vcxproj` already includes `TestControls_Editor.cpp`, `TestControls_Editor_Key.cpp`, and `TestControls_Editor_RichText.cpp` as `ClCompile` items.

### 4) Place both files in the correct solution explorer folder (filters integration)

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters`:

- Add `ClCompile` entries for both new files with:
	- `<Filter>Source Files\Controls\Editor</Filter>`

Evidence/pattern:

- `TestControls_Editor.cpp`, `TestControls_Editor_Key.cpp`, and `TestControls_Editor_RichText.cpp` are already attached to `Source Files\Controls\Editor`.

### 5) Activate both files for debugging (vcxproj.user)

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.user` (Debug|x64) so the default debugger run includes the new test files:

- Append `/F:TestControls_Editor_InlineObject.cpp` and `/F:TestControls_Editor_Styles.cpp` to `LocalDebuggerCommandArguments`.
- Append the resulting full argument string as a new trailing entry in `LocalDebuggerCommandArgumentsHistory`, following the existing `|`-separated history format.

Evidence/pattern:

- The current `LocalDebuggerCommandArguments` already filters editor tests using `/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp /F:TestControls_Editor_RichText.cpp`.
- `.github/KnowledgeBase/KB_Vlpp_UnitTesting.md` documents `/F:TestFile` as the intended mechanism for selecting test files.

# !!!FINISHED!!!
