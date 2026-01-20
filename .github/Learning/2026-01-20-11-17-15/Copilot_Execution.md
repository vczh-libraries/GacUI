# !!!EXECUTION!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Create `TestControls_Editor_InlineObject.cpp` [DONE]

Create a new file: `Test/GacUISrc/UnitTest/TestControls_Editor_InlineObject.cpp`.

File content:

	#include "TestControls.h"

	using namespace gacui_unittest_template;

	TEST_FILE
	{
		TEST_CATEGORY(L"InlineObject")
		{
		});
	}

## STEP 2: Create `TestControls_Editor_Styles.cpp` [DONE]

Create a new file: `Test/GacUISrc/UnitTest/TestControls_Editor_Styles.cpp`.

File content:

	#include "TestControls.h"

	using namespace gacui_unittest_template;

	TEST_FILE
	{
		TEST_CATEGORY(L"Styles")
		{
		});
	}

## STEP 3: Add both files to `UnitTest.vcxproj` [DONE]

Edit `Test/GacUISrc/UnitTest/UnitTest.vcxproj`.

In the `<ItemGroup>` containing other editor test files, add:

	<ClCompile Include="TestControls_Editor_InlineObject.cpp" />
	<ClCompile Include="TestControls_Editor_Styles.cpp" />

Place them next to the existing editor entries, e.g. right after `TestControls_Editor_RichText.cpp`:

	<ClCompile Include="TestControls_Editor.cpp" />
	<ClCompile Include="TestControls_Editor_Key.cpp" />
	<ClCompile Include="TestControls_Editor_RichText.cpp" />
	<ClCompile Include="TestControls_Editor_InlineObject.cpp" />
	<ClCompile Include="TestControls_Editor_Styles.cpp" />
	<ClCompile Include="TestControls_Editor_Key_Multiline.cpp" />
	<ClCompile Include="TestControls_Editor_Key_Paragraph.cpp" />
	<ClCompile Include="TestControls_Editor_Key_Shared.cpp">
	  <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">/bigobj %(AdditionalOptions)</AdditionalOptions>
	  <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">/bigobj %(AdditionalOptions)</AdditionalOptions>
	  <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">/bigobj %(AdditionalOptions)</AdditionalOptions>
	  <AdditionalOptions Condition="'$(Configuration)|$(Platform)'=='Release|x64'">/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
	<ClCompile Include="TestControls_Editor_Key_Singleline.cpp" />

## STEP 4: Add both files to `UnitTest.vcxproj.filters` [DONE]

Edit `Test/GacUISrc/UnitTest/UnitTest.vcxproj.filters`.

In the `<ItemGroup>` containing other editor test files, add:

	<ClCompile Include="TestControls_Editor_InlineObject.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>
	<ClCompile Include="TestControls_Editor_Styles.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>

Place them next to the existing editor entries, e.g. right after `TestControls_Editor_RichText.cpp`:

	<ClCompile Include="TestControls_Editor.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>
	<ClCompile Include="TestControls_Editor_Key.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>
	<ClCompile Include="TestControls_Editor_RichText.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>
	<ClCompile Include="TestControls_Editor_InlineObject.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>
	<ClCompile Include="TestControls_Editor_Styles.cpp">
	  <Filter>Source Files\Controls\Editor</Filter>
	</ClCompile>

## STEP 5: Activate both files in `UnitTest.vcxproj.user` (Debug|x64) [DONE]

Edit `Test/GacUISrc/UnitTest/UnitTest.vcxproj.user`.

In the `Debug|x64` `<PropertyGroup>`, update `LocalDebuggerCommandArguments` to add two `/F:` switches:

	<LocalDebuggerCommandArguments>/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp /F:TestControls_Editor_RichText.cpp /F:TestControls_Editor_InlineObject.cpp /F:TestControls_Editor_Styles.cpp</LocalDebuggerCommandArguments>

In the same `Debug|x64` `<PropertyGroup>`, append the updated full argument string as a new trailing entry in `LocalDebuggerCommandArgumentsHistory` (keep the existing `|`-separated format and the trailing `|`):

	<LocalDebuggerCommandArgumentsHistory>/F:TestControls_Editor_GuiSinglelineTextBox_Key.cpp|/F:TestApplication_Tooltip.cpp /F:TestControls_Editor_GuiSinglelineTextBox.cpp /F:TestControls_Editor_GuiSinglelineTextBox_Key.cpp|/F:TestControls_Editor_GuiSinglelineTextBox.cpp /F:TestControls_Editor_GuiSinglelineTextBox_Key.cpp|/F:TestControls_Editor.cpp /F:TestControls_Editor_GuiSinglelineTextBox_Key.cpp|/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp|/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp /F:TestControls_Editor_RichText.cpp|/F:TestControls_Editor.cpp /F:TestControls_Editor_Key.cpp /F:TestControls_Editor_RichText.cpp /F:TestControls_Editor_InlineObject.cpp /F:TestControls_Editor_Styles.cpp|</LocalDebuggerCommandArgumentsHistory>

# FIXING ATTEMPTS

# !!!FINISHED!!!

# !!!VERIFIED!!!
