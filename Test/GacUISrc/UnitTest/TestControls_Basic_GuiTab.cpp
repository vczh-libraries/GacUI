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
	const auto resourceTabWithAlt= LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiTab" ClientSize="x:320 y:240">
        <Tab ref.Name="tab">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.Pages>
            <TabPage ref.Name="tabPageOptions" Text="Options" Alt="O">
              <Stack Direction="Vertical" Padding="5" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:5 top:5 right:5 bottom:5">
                <StackItem>
                  <CheckBox Alt="A" Text="Option 1"/>
                </StackItem>
                <StackItem>
                  <CheckBox Alt="B" Text="Option 2"/>
                </StackItem>
              </Stack>
            </TabPage>
            <TabPage ref.Name="tabPageLabel" Text="Label" Alt="L">
              <Label Text="This is a label!">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
              </Label>
            </TabPage>
          </att.Pages>
        </Tab>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiTab")
	{
		TEST_CASE(L"Navigation")
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
				WString::Unmanaged(L"Controls/Basic/GuiTab/Navigation"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTabWithAlt
				);
		});
	});
}