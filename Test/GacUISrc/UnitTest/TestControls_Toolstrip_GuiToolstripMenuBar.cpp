#include "TestControls.h"

TEST_FILE
{
	const auto resourceMenuBar = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiButton" ClientSize="x:320 y:240">
        <Button ref.Name="button" Text="This is a Button">
          <ev.Clicked-eval><![CDATA[{
            button.Text = "Clicked!";
          }]]></ev.Clicked-eval>
        </Button>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiToolstripMenuBar")
	{
		TEST_CASE(L"Click")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripMenuBar/Click"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceMenuBar
				);
		});
	});
}