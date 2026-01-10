#include "TestControls.h"

using namespace gacui_unittest_template;

static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)
{
	TEST_CATEGORY(L"Scaffold")
	{
		TEST_CASE(L"SmokingTest")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					protocol->TypeString(L"0123456789");
				});

				protocol->OnNextIdleFrame(L"Typing", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"0123456789");
					protocol->KeyPress(VKEY::KEY_LEFT);
					protocol->TypeString(L"X");
					TEST_ASSERT(textBox->GetText() == L"012345678X9");
				});

				protocol->OnNextIdleFrame(L"Moved Cursor and Edited", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/")
				+ controlName
				+ WString::Unmanaged(L"/Key/Scaffold_SmokingTest"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});
}

TEST_FILE
{
	const auto resourceSinglelineTextBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiSinglelineTextBox Key Test" ClientSize="x:480 y:320">
        <SinglelineTextBox ref.Name="textBox">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </SinglelineTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		RunSinglelineTextBoxTestCases(resourceSinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));
	});
}
