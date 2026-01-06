#include "TestControls.h"

TEST_FILE
{
	const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiSinglelineTextBox Test" ClientSize="x:480 y:320">
        <SinglelineTextBox ref.Name="textBox" Text="Initial text">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </SinglelineTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		TEST_CASE(L"Basic")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"Initial text");
					TEST_ASSERT(textBox->GetEnabled() == true);
					textBox->SetText(L"Changed text");
				});
				protocol->OnNextIdleFrame(L"After text change", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"Changed text");
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/GuiSinglelineTextBox/Basic"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});

		TEST_CASE(L"Typing")
		{
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					textBox->SelectAll();
					protocol->TypeString(L"Hello");
				});
				protocol->OnNextIdleFrame(L"Typed Hello", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"Hello");
					protocol->TypeString(L" World");
				});
				protocol->OnNextIdleFrame(L"Typed World", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"Hello World");
					protocol->KeyPress(VKEY::KEY_LEFT);
					protocol->TypeString(L"!");
				});
				protocol->OnNextIdleFrame(L"Inserted punctuation", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"Hello Worl!d");
					textBox->SelectAll();
					protocol->TypeString(L"Replaced");
				});
				protocol->OnNextIdleFrame(L"Selection replaced", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"Replaced");
					protocol->KeyPress(VKEY::KEY_BACK);
					protocol->KeyPress(VKEY::KEY_LEFT);
					protocol->KeyPress(VKEY::KEY_DELETE);
				});
				protocol->OnNextIdleFrame(L"Backspace/Delete applied", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"Replac");
					auto caretPoint = protocol->LocationOf(textBox, 0.0, 0.5, 5, 0);
					protocol->LClick(caretPoint);
					protocol->TypeString(L"Start ");
				});
				protocol->OnNextIdleFrame(L"Mouse typed at start", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"textBox");
					TEST_ASSERT(textBox->GetText() == L"Start Replac");
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/GuiSinglelineTextBox/Typing"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource
			);
		});
	});
}
