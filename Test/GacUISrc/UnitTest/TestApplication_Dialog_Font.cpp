#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceOpenAndClose = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Dialog Font Test" ClientSize="x:480 y:640">
        <FontDialog ref.Name="fontDialog" ShowEffect="true"/>
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" CellPadding="5">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>

          <Cell Site="row:0 column:0" PreferredMinSize="y:20">
            <Label ref.Name="labelFont" Text="Sample Text"/>
          </Cell>

          <Cell Site="row:1 column:0">
            <Button Text="Change Font Simple">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                fontDialog.ShowEffect = false;
                fontDialog.SelectedFont = labelFont.DisplayFont;
                if (fontDialog.ShowDialog())
                {
                  labelFont.Font = fontDialog.SelectedFont;
                }
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>

          <Cell Site="row:2 column:0">
            <Button Text="Change Font">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                fontDialog.ShowEffect = true;
                fontDialog.SelectedFont = labelFont.DisplayFont;
                if (fontDialog.ShowDialog())
                {
                  labelFont.Font = fontDialog.SelectedFont;
                }
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Dialog Font")
	{
		TEST_CASE(L"Open and Close (Simple)")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();;
					auto button = FindControlByText<GuiButton>(window, L"Change Font Simple");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					auto window = From(GetApplication()->GetWindows())
						.Where([](GuiWindow* w) { return w->GetText() == L"Choose Font"; })
						.First();
					auto textBoxFont = FindObjectByName<GuiSinglelineTextBox>(window, L"nameControl", L"textBox");
					auto textBoxSize = FindObjectByName<GuiSinglelineTextBox>(window, L"sizeControl", L"textBox");

					textBoxFont->SetText(L"Segoe UI");
					textBoxSize->SetText(L"24");
				});
				protocol->OnNextIdleFrame(L"Set Font", [=]()
				{
					auto window = From(GetApplication()->GetWindows())
						.Where([](GuiWindow* w) { return w->GetText() == L"Choose Font"; })
						.First();
					auto button = FindControlByText<GuiButton>(window, L"OK");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"OK", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto label = FindObjectByName<GuiLabel>(window, L"labelFont");
					TEST_ASSERT(label->GetFont());
					TEST_ASSERT(label->GetFont().Value().fontFamily == L"Segoe UI");
					TEST_ASSERT(label->GetFont().Value().size == 24);
					TEST_ASSERT(label->GetFont().Value().bold == false);
					TEST_ASSERT(label->GetFont().Value().italic == false);
					TEST_ASSERT(label->GetFont().Value().underline == false);
					TEST_ASSERT(label->GetFont().Value().strikeline == false);
					window->Hide();
				});
			});

			UnitTestScreenConfig config;
			config.FastInitialize(1024, 768);
			config.fontConfig.supportedFonts->Add(L"Segoe UI");
			config.fontConfig.supportedFonts->Add(L"Fake Font 1");
			config.fontConfig.supportedFonts->Add(L"Fake Font 2");

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_Font/OpenAndClose_Simple"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceOpenAndClose,
				config
			);
		});

		TEST_CASE(L"Open and Close")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();;
					auto button = FindControlByText<GuiButton>(window, L"Change Font");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					auto window = From(GetApplication()->GetWindows())
						.Where([](GuiWindow* w) { return w->GetText() == L"Choose Font"; })
						.First();
					auto textBoxFont = FindObjectByName<GuiSinglelineTextBox>(window, L"nameControl", L"textBox");
					auto textBoxSize = FindObjectByName<GuiSinglelineTextBox>(window, L"sizeControl", L"textBox");

					textBoxFont->SetText(L"Segoe UI");
					textBoxSize->SetText(L"24");

					FindObjectByName<GuiSelectableButton>(window, L"checkBold")->SetSelected(true);
					FindObjectByName<GuiSelectableButton>(window, L"checkItalic")->SetSelected(true);
					FindObjectByName<GuiSelectableButton>(window, L"checkUnderline")->SetSelected(true);
					FindObjectByName<GuiSelectableButton>(window, L"checkStrikeline")->SetSelected(true);
				});
				protocol->OnNextIdleFrame(L"Set Font", [=]()
				{
					auto window = From(GetApplication()->GetWindows())
						.Where([](GuiWindow* w) { return w->GetText() == L"Choose Font"; })
						.First();
					auto button = FindControlByText<GuiButton>(window, L"OK");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"OK", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto label = FindObjectByName<GuiLabel>(window, L"labelFont");
					TEST_ASSERT(label->GetFont());
					TEST_ASSERT(label->GetFont().Value().fontFamily == L"Segoe UI");
					TEST_ASSERT(label->GetFont().Value().size == 24);
					TEST_ASSERT(label->GetFont().Value().bold == true);
					TEST_ASSERT(label->GetFont().Value().italic == true);
					TEST_ASSERT(label->GetFont().Value().underline == true);
					TEST_ASSERT(label->GetFont().Value().strikeline == true);
					window->Hide();
				});
			});

			UnitTestScreenConfig config;
			config.FastInitialize(1024, 768);
			config.fontConfig.supportedFonts->Add(L"Segoe UI");
			config.fontConfig.supportedFonts->Add(L"Fake Font 1");
			config.fontConfig.supportedFonts->Add(L"Fake Font 2");

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_Font/OpenAndClose"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceOpenAndClose,
				config
			);
		});
	});
}