# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Create the new rich-text editor test file [DONE]

Create a new file: `Test/GacUISrc/UnitTest/TestControls_Editor_RichText.cpp`

File content:

	#include "TestControls.h"

	using namespace gacui_unittest_template;

	TEST_FILE
	{
		TEST_CATEGORY(L"GuiDocumentLabel_RichText")
		{
		});
	}

## STEP 2: Add the new .cpp to the unit test project (UnitTest.vcxproj) [DONE]

Edit: `Test/GacUISrc/UnitTest/UnitTest.vcxproj`

Find the `<ItemGroup>` that contains:

	<ClCompile Include="TestControls_Editor.cpp" />
	<ClCompile Include="TestControls_Editor_Key.cpp" />
	<ClCompile Include="TestControls_Editor_Key_Multiline.cpp" />

Insert this new entry between `TestControls_Editor_Key.cpp` and `TestControls_Editor_Key_Multiline.cpp`:

	<ClCompile Include="TestControls_Editor_RichText.cpp" />

Resulting snippet:

	<ClCompile Include="TestControls_Editor.cpp" />
	<ClCompile Include="TestControls_Editor_Key.cpp" />
	<ClCompile Include="TestControls_Editor_RichText.cpp" />
	<ClCompile Include="TestControls_Editor_Key_Multiline.cpp" />

## STEP 3: Place the new file under the existing Editor filter (UnitTest.vcxproj.filters) [DONE]

Edit: `Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters`

Find the `<ItemGroup>` that contains:

	<ClCompile Include="TestControls_Editor.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>
	<ClCompile Include="TestControls_Editor_Key.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>

Insert a new `ClCompile` entry after `TestControls_Editor_Key.cpp`:

	<ClCompile Include="TestControls_Editor_RichText.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>

Resulting snippet:

	<ClCompile Include="TestControls_Editor.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>
	<ClCompile Include="TestControls_Editor_Key.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>
	<ClCompile Include="TestControls_Editor_RichText.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>

## STEP 4: Activate the new test file in debugger filters (UnitTest.vcxproj.user) [DONE]

Edit: `Test/GacUISrc/UnitTest/UnitTest.vcxproj.user`

In the `<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">`, change:

	<LocalDebuggerCommandArguments>/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp</LocalDebuggerCommandArguments>

To:

	<LocalDebuggerCommandArguments>/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp /F:TestControls_Editor_RichText.cpp</LocalDebuggerCommandArguments>

Also update `<LocalDebuggerCommandArgumentsHistory>` by appending the new full argument string as a new entry (keep existing history content intact):

	<LocalDebuggerCommandArgumentsHistory>/F:TestControls_Editor_GuiSinglelineTextBox_Key.cpp|/F:TestApplication_Tooltip.cpp /F:TestControls_Editor_GuiSinglelineTextBox.cpp /F:TestControls_Editor_GuiSinglelineTextBox_Key.cpp|/F:TestControls_Editor_GuiSinglelineTextBox.cpp /F:TestControls_Editor_GuiSinglelineTextBox_Key.cpp|/F:TestControls_Editor.cpp /F:TestControls_Editor_GuiSinglelineTextBox_Key.cpp|/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp|/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp /F:TestControls_Editor_RichText.cpp|</LocalDebuggerCommandArgumentsHistory>

## STEP 5: Test plan (build + execution sanity)

- Build the `UnitTest` project (Debug|x64).
- Run the unit test binary with the updated debugger arguments; confirm the test file is included and `GuiDocumentLabel_RichText` appears.
- Confirm the new file appears under `Source Files\\Controls\\Editor` in Solution Explorer.

# FIXING ATTEMPTS

No fixing attempt was needed.

- `Build Unit Tests` completed.
- Build result: succeeded with 1 warning and 0 errors.
- The warning is unrelated to the new file/project entry changes.

## Verification (2026-01-18)

- `Build Unit Tests`: Build succeeded, 0 Warning(s), 0 Error(s)
- `Run Unit Tests`: Passed test files: 3/3; Passed test cases: 196/196
- Confirmed `TestControls_Editor_RichText.cpp` executed and `GuiDocumentLabel_RichText` appeared in the log.

# !!!FINISHED!!!

# !!!VERIFIED!!!
