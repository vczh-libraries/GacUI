#include "TestControls.h"

using namespace gacui_unittest_template;

template<typename TTextBox>
void RunTextBoxSmokeTest(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox> && std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>)
{
	TEST_CASE(L"Basic")
	{
		TooltipTimer timer;
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Initial text");
				TEST_ASSERT(textBox->GetEnabled() == true);
				textBox->SetText(L"Changed text");
			});
			protocol->OnNextIdleFrame(L"After text change", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Changed text");
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Basic"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	});

	TEST_CASE(L"Typing")
	{
		TooltipTimer timer;
		GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				textBox->SetFocused();
				textBox->SelectAll();
				protocol->TypeString(L"Hello");
			});
			protocol->OnNextIdleFrame(L"Typed Hello", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Hello");
				protocol->TypeString(L" World");
			});
			protocol->OnNextIdleFrame(L"Typed World", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Hello World");
				protocol->KeyPress(VKEY::KEY_LEFT);
				protocol->TypeString(L"!");
			});
			protocol->OnNextIdleFrame(L"Inserted punctuation", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Hello Worl!d");
				textBox->SelectAll();
				protocol->TypeString(L"Replaced");
			});
			protocol->OnNextIdleFrame(L"Selection replaced", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Replaced");
				protocol->KeyPress(VKEY::KEY_BACK);
				protocol->KeyPress(VKEY::KEY_LEFT);
				protocol->KeyPress(VKEY::KEY_DELETE);
			});
			protocol->OnNextIdleFrame(L"Backspace/Delete applied", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Replac");
				auto caretPoint = protocol->LocationOf(textBox, 0.0, 0.0, 2, 8);
				protocol->LClick(caretPoint);
				protocol->KeyPress(VKEY::KEY_HOME, true, false, false);
				protocol->TypeString(L"Start ");
			});
			protocol->OnNextIdleFrame(L"Typed at start", [&, protocol]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<TTextBox>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"Start Replac");
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Typing"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	});
}

TEST_FILE
{
	const auto resource_SinglelineTextBox = LR"GacUISrc(
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

	const auto resource_MultilineTextBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiMultilineTextBox Test" ClientSize="x:480 y:320">
        <MultilineTextBox ref.Name="textBox" Text="Initial text">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
        </MultilineTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resource_DocumentTextBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentTextBox Test" ClientSize="x:480 y:320">
        <DocumentTextBox ref.Name="textBox" Text="Initial text" EditMode="Editable">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </DocumentTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resource_DocumentLabel = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentLabel Test" ClientSize="x:480 y:320">
        <DocumentLabel ref.Name="textBox" Text="Initial text" EditMode="Editable">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
        </DocumentLabel>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resource_DocumentViewer = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentViewer Test" ClientSize="x:480 y:320">
        <DocumentViewer ref.Name="textBox" Text="Initial text" EditMode="Editable">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
        </DocumentViewer>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		RunTextBoxSmokeTest<GuiSinglelineTextBox>(resource_SinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));
	});

	TEST_CATEGORY(L"GuiMultilineTextBox")
	{
		RunTextBoxSmokeTest<GuiMultilineTextBox>(resource_MultilineTextBox, WString::Unmanaged(L"GuiMultilineTextBox"));
	});

	TEST_CATEGORY(L"GuiDocumentTextBox")
	{
		RunTextBoxSmokeTest<GuiDocumentLabel>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
	});

	TEST_CATEGORY(L"GuiDocumentLabel")
	{
		RunTextBoxSmokeTest<GuiDocumentLabel>(resource_DocumentLabel, WString::Unmanaged(L"GuiDocumentLabel"));
	});

	TEST_CATEGORY(L"GuiDocumentViewer")
	{
		RunTextBoxSmokeTest<GuiDocumentViewer>(resource_DocumentViewer, WString::Unmanaged(L"GuiDocumentViewer"));
	});
}
