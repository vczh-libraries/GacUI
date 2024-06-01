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
using namespace vl::presentation::unittest;

TEST_FILE
{
	const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MyControlTemplate">
    <Instance ref.Class="gacuisrc_unittest::MyControlTemplate">
      <ControlTemplate ref.Name="self" ContainerComposition-ref="container" MinSizeLimitation="LimitToElementAndChildren">
        <SolidBorder Color="#FFFFFF"/>
        <Table CellPadding="5" BorderVisible="true" AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>

          <Cell Site="row:0 column:0 columnSpan:2"> <SolidLabel Font-bind="self.Font" Color="#FFFFFF" Text-bind="self.Text"/>                                     </Cell>
          <Cell Site="row:1 column:0 columnSpan:2"> <SolidLabel Font-bind="self.Font" Color="#FFFFFF" Text-bind="self.Font.fontFamily"/>                          </Cell>
          <Cell Site="row:2 column:0">              <SolidLabel Font-bind="self.Font" Color="#FFFFFF" Text-bind="self.VisuallyEnabled ? 'Enabled' : 'Disabled'"/> </Cell>
          <Cell Site="row:3 column:0">              <SolidLabel Font-bind="self.Font" Color="#FFFFFF" Text-bind="self.Focused ? 'Focused' : 'Unfocused'"/>        </Cell>

          <Cell Site="row:2 column:1 rowSpan:3">
            <SolidBorder Color="#FFFFFF"/>
            <Bounds ref.Name="container" AlignmentToParent="left:1 top:1 right:1 bottom:1" MinSizeLimitation="LimitToElementAndChildren"/>
          </Cell>
        </Table>
      </ControlTemplate>
    </Instance>
  </Instance>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow" xmlns:ut="gacuisrc_unittest::*">
      <Window ref.Name="self" Text="MyControlTemplate" ClientSize="x:320 y:240">
        <CustomControl Text="This is a control with a template">
          <att.ControlTemplate>ut:MyControlTemplate</att.ControlTemplate>
        </CustomControl>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CASE(L"Control Template")
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
			WString::Unmanaged(L"Controls/CoreApplication/GuiControl/MyControlTemplate"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
			);
	});

	TEST_CASE(L"Visible")
	{
	});

	TEST_CASE(L"Enabled and VisuallyEnabled")
	{
	});

	TEST_CASE(L"Focus and Alt")
	{
	});

	TEST_CASE(L"Font and DisplayFont")
	{
	});
}