# !!!PLANNING!!!

# UPDATES

## UPDATE
Just use `InlineObject` and `Styles` as the test category

# EXECUTION PLAN

## STEP 1: Add `TestControls_Editor_InlineObject.cpp`

Create `Test/GacUISrc/UnitTest/TestControls_Editor_InlineObject.cpp` in the same folder as the existing editor tests, using the same scaffolding pattern as `TestControls_Editor_RichText.cpp`.

    #include "TestControls.h"

    using namespace gacui_unittest_template;

    TEST_FILE
    {
        TEST_CATEGORY(L"InlineObject")
        {
        });
    }

Why this is necessary:

- This creates a dedicated translation unit for editor tests related to inline objects, keeping future growth isolated from key-navigation focused files.
- A `TEST_FILE` block ensures the file is registered by the unit test framework even before real test cases are added.

## STEP 2: Add `TestControls_Editor_Styles.cpp`

Create `Test/GacUISrc/UnitTest/TestControls_Editor_Styles.cpp` in the same folder as the existing editor tests, using the same scaffolding pattern as `TestControls_Editor_RichText.cpp`.

    #include "TestControls.h"

    using namespace gacui_unittest_template;

    TEST_FILE
    {
        TEST_CATEGORY(L"Styles")
        {
        });
    }

Why this is necessary:

- This creates a dedicated translation unit for editor style-related tests, preventing `TestControls_Editor_Key.cpp` from becoming a catch-all.
- Having a category placeholder makes it easy for later tasks to add cases without reorganizing files again.

## STEP 3: Add both files to `UnitTest.vcxproj`

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj` to compile the new sources by adding two `ClCompile` items in the same `<ItemGroup>` that already contains `TestControls_Editor*.cpp` entries (place them adjacent to the existing editor items for readability):

    <ClCompile Include="TestControls_Editor.cpp" />
    <ClCompile Include="TestControls_Editor_Key.cpp" />
    <ClCompile Include="TestControls_Editor_RichText.cpp" />
    <ClCompile Include="TestControls_Editor_InlineObject.cpp" />
    <ClCompile Include="TestControls_Editor_Styles.cpp" />

Why this is necessary:

- Without these `ClCompile` entries, Visual Studio will not build/link the new test translation units, so they cannot be executed or debugged.

## STEP 4: Add both files to `UnitTest.vcxproj.filters`

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters` to place the new files under the same solution explorer folder as other editor tests (`Source Files\\Controls\\Editor`).

Add two `ClCompile` nodes consistent with existing editor test entries:

    <ClCompile Include="TestControls_Editor_InlineObject.cpp">
      <Filter>Source Files\Controls\Editor</Filter>
    </ClCompile>
    <ClCompile Include="TestControls_Editor_Styles.cpp">
      <Filter>Source Files\Controls\Editor</Filter>
    </ClCompile>

Why this is necessary:

- Keeping editor tests grouped under `Source Files\Controls\Editor` preserves solution organization and makes the new files discoverable for later tasks.

## STEP 5: Activate both files in `UnitTest.vcxproj.user` (Debug|x64)

Update `Test/GacUISrc/UnitTest/UnitTest.vcxproj.user` to include both new files in the default `Debug|x64` filter list so the unit test runner can select them via `/F:`.

Change `LocalDebuggerCommandArguments` to include two additional `/F:` switches:

    <LocalDebuggerCommandArguments>/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp /F:TestControls_Editor_RichText.cpp /F:TestControls_Editor_InlineObject.cpp /F:TestControls_Editor_Styles.cpp</LocalDebuggerCommandArguments>

Append the updated full argument string to `LocalDebuggerCommandArgumentsHistory` as a new trailing entry, keeping the existing `|`-separated format:

    ...|/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp /F:TestControls_Editor_RichText.cpp /F:TestControls_Editor_InlineObject.cpp /F:TestControls_Editor_Styles.cpp|

Why this is necessary:

- This preserves the existing workflow of running only editor-focused tests by default when debugging the unit test project.
- Adding to history keeps the previous argument sets available while making the new configuration the most recent option.

# TEST PLAN

1) Build integration

- Build `Test/GacUISrc/UnitTest/UnitTest.vcxproj` in Visual Studio for `Debug|x64` to confirm the two new `.cpp` files compile and link.
- Optionally also build `Debug|Win32` to confirm project inclusion is not accidentally gated by configuration.

2) Solution explorer placement

- Verify both files appear under `Source Files -> Controls -> Editor` in Solution Explorer (driven by `UnitTest.vcxproj.filters`).

3) Test selection / execution wiring

- Start debugging `UnitTest` under `Debug|x64` and confirm the command line includes:
  - `/F:TestControls_Editor_InlineObject.cpp`
  - `/F:TestControls_Editor_Styles.cpp`
- Confirm the console output includes entries for the two new test files (even if they contain only empty categories initially).

# !!!FINISHED!!!
