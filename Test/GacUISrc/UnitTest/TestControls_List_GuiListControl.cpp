#include "../../../Source//UnitTestUtilities/GuiUnitTestUtilities.h"
#ifdef VCZH_64
#include "../Generated_DarkSkin/Source_x64/DarkSkin.h"
#else
#include "../Generated_DarkSkin/Source_x86/DarkSkin.h"
#endif

using namespace vl;
using namespace vl::stream;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::controls::list;
using namespace vl::presentation::unittest;

TEST_FILE
{
	const auto resourceListControl= LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiListControl" ClientSize="x:480 y:320">
        <Table BorderVisible="false" CellPadding="5" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
            <_>composeType:Absolute absolute:160</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <GroupBox Text="Items">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>

          <Cell Site="row:0 column:1">
            <GroupBox Text="Logs">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="logs" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiListControl")
	{
		TEST_CASE(L"Scrolling")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					for (vint i = 1; i <= 10; i++)
					{
						auto item = Ptr(new TextItem(L"Item " + itow(i)));
						listControl->GetItems().Add(item);
					}
				});
				protocol->OnNextIdleFrame(L"10 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiListControl/Scrolling"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListControl
				);
		});
	});
}