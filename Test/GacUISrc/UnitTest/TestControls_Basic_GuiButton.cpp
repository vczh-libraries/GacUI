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

	TEST_CASE(L"GuiButton")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindObjectByName<GuiButton>(window, L"button");
				auto location = protocol->LocationOf(button);
				protocol->MouseMove(location);
			});
			protocol->OnNextIdleFrame(L"Hover", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindObjectByName<GuiButton>(window, L"button");
				auto location = protocol->LocationOf(button);
				protocol->_LDown(location);
			});
			protocol->OnNextIdleFrame(L"MouseDown", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindObjectByName<GuiButton>(window, L"button");
				auto location = protocol->LocationOf(button);
				protocol->_LUp(location);
			});
			protocol->OnNextIdleFrame(L"MouseUp", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindObjectByName<GuiButton>(window, L"button");
				auto location = protocol->LocationOf(button);
				protocol->MouseMove({ 0,0 });
			});
			protocol->OnNextIdleFrame(L"Close", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Basic/GuiButton/ClickOnMouseUp"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
			);
	});
}