# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Create the new rich-text editor test file

Create `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp` for rich-text editing tests that target `GuiDocumentLabel` only.

What to change:

- Add a new source file with a minimal unit test scaffold so it compiles and registers in the Vlpp unit test framework.
- Keep the file focused on `GuiDocumentLabel` (no other textbox controls in this file).

Why:

- This task is test plumbing only; later tasks will fill categories/cases. A minimal scaffold ensures the project integration work (vcxproj / filters / debugger args) can be reviewed and merged first.

Proposed file content:

    #include "TestControls.h"

    using namespace gacui_unittest_template;

    TEST_FILE
    {
        TEST_CATEGORY(L"GuiDocumentLabel_RichText")
        {
        });
    }

Notes:

- `TEST_CATEGORY` name is intentionally specific to avoid mixing with existing editor key/navigation categories.
- If later tasks need a window resource, add it inside this file near the category (following patterns in `Test/GacUISrc/UnitTest/TestControls_Editor_Key.cpp`).

## STEP 2: Add the new .cpp to the unit test project (UnitTest.vcxproj)

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj` to compile the new translation unit.

What to change:

- In the `<ItemGroup>` that lists `ClCompile` sources, add a new entry near the existing editor-related sources:

    <ClCompile Include="TestControls_Editor.cpp" />
    <ClCompile Include="TestControls_Editor_Key.cpp" />
    <ClCompile Include="TestControls_Editor_RichText.cpp" />
    <ClCompile Include="TestControls_Editor_Key_Singleline.cpp" />

Why:

- Without being compiled into the unit test binary, the `TEST_FILE` registration in the new file will never run.

## STEP 3: Place the new file under the existing Editor filter (UnitTest.vcxproj.filters)

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters` so Solution Explorer shows the file next to other editor tests.

What to change:

- Add a `ClCompile` entry with the same filter used by `TestControls_Editor_Key.cpp`:

    <ClCompile Include="TestControls_Editor_Key.cpp">
      <Filter>Source Files\Controls\Editor</Filter>
    </ClCompile>
    <ClCompile Include="TestControls_Editor_RichText.cpp">
      <Filter>Source Files\Controls\Editor</Filter>
    </ClCompile>

Why:

- Keeps editor tests discoverable and consistent, and avoids later work being blocked by missing filters organization.

## STEP 4: Activate the new test file in debugger filters (UnitTest.vcxproj.user)

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.user` so the default Debug|x64 run includes the new test file.

What to change:

- Append a new `/F:` filter to `LocalDebuggerCommandArguments` (keep existing filters):

    <LocalDebuggerCommandArguments>/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp /F:TestControls_Editor_RichText.cpp</LocalDebuggerCommandArguments>

- Append the new full argument string as a new history entry (preserve existing history content and its trailing `|` separators):

    <LocalDebuggerCommandArgumentsHistory>...|/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp /F:TestControls_Editor_RichText.cpp|</LocalDebuggerCommandArgumentsHistory>

Why:

- Later tasks will add rich-text cases to this file; having it in the default debug filter avoids repeated manual edits and reduces iteration overhead.

## STEP 5: Test plan (build + execution sanity)

Build and run the unit test binary to validate the plumbing change without adding any new test cases yet.

What to verify:

- Build succeeds and `TestControls_Editor_RichText.cpp` is compiled and linked (no missing include or macro registration errors).
- Running `UnitTest` under Debug|x64 with the updated `LocalDebuggerCommandArguments` lists and executes the new test file.
  - Expected: `GuiDocumentLabel_RichText` appears (even if it contains zero test cases for now).
- The new file appears under `Source Files\Controls\Editor` in Solution Explorer.

Why:

- This task is primarily about project wiring; the correct validation is that the new file is compiled, discoverable, and runnable by the existing framework.

# !!!FINISHED!!!
