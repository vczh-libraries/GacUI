#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	const WString resourceDataGrid = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiBindableDataGrid" ClientSize="x:480 y:320">
        <BindableDataGrid ref.Name="dataGrid" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
        </BindableDataGrid>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiBindableDataGrid")
	{
		TEST_CASE(L"DisplayStringProperties")
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
				WString::Unmanaged(L"Controls/List/GuiBindableDataGrid/Binding/DisplayStringProperties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDataGrid
				);
		});
	});
}