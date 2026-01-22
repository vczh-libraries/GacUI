#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceOpenAndClose = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Dialog Color Test" ClientSize="x:480 y:320">
        <ColorDialog ref.Name="dialogColor" EnabledCustomColor="false"/>
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" CellPadding="5">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>

          <Cell Site="row:0 column:0" PreferredMinSize="y:20">
            <SolidBackground ref.Name="background" Color="#FF0000"/>
          </Cell>

          <Cell Site="row:1 column:0">
            <Button Text="Change Color">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                dialogColor.SelectedColor = background.Color;
                if (dialogColor.ShowDialog())
                {
                  background.Color = dialogColor.SelectedColor;
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

	TEST_CATEGORY(L"Dialog Color")
	{
		TEST_CASE(L"Open and Close")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();;
					auto button = FindControlByText<GuiButton>(window, L"Change Color");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					auto window = From(GetApplication()->GetWindows())
						.Where([](GuiWindow* w) { return w->GetText() == L"Choose Dialog"; })
						.First();
					auto trackerRed = FindObjectByName<GuiScroll>(FindObjectByName<GuiInstanceRootObject>(window, L"colorRed"), L"tracker");
					auto trackerGreen = FindObjectByName<GuiScroll>(FindObjectByName<GuiInstanceRootObject>(window, L"colorGreen"), L"tracker");
					auto trackerBlue = FindObjectByName<GuiScroll>(FindObjectByName<GuiInstanceRootObject>(window, L"colorBlue"), L"tracker");

					trackerRed->SetPosition(0);
					trackerGreen->SetPosition(128);
					trackerGreen->SetPosition(255);
				});
				protocol->OnNextIdleFrame(L"Set Color", [=]()
				{
					auto window = From(GetApplication()->GetWindows())
						.Where([](GuiWindow* w) { return w->GetText() == L"Choose Dialog"; })
						.First();
					auto button = FindControlByText<GuiButton>(window, L"OK");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"OK", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto background = FindObjectByName<GuiSolidBackgroundElement>(window, L"background");
					TEST_ASSERT(background->GetColor() == Color(0, 128, 255));
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_Color/OpenAndClose"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceOpenAndClose
			);
		});
	});
}