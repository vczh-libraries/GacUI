#include "../../../Source/UnitTestUtilities/GuiUnitTestUtilities.h"
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
	TEST_CATEGORY(L"Channels and Diffs")
	{
		const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Hello, world!" ClientSize="x:320 y:240">
        <Button ref.Name="buttonOK" Text="OK">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
          <ev.Clicked-eval><![CDATA[ {
            Application::GetApplication().InvokeInMainThread(self, func():void{self.Hide();});
          } ]]></ev.Clicked-eval>
        </Button>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		auto sharedProxy = [](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto buttonOK = TryFindObjectByName<GuiButton>(window, L"buttonOK");
				protocol->MouseMove(protocol->LocationOf(buttonOK));
			});
			protocol->OnNextIdleFrame(L"Hover", [=]()
			{
				protocol->_LDown();
			});
			protocol->OnNextIdleFrame(L"Press", [=]()
			{
				protocol->_LUp();
			});
		};

		TEST_CASE(L"Async Channel")
		{
			GacUIUnitTest_SetGuiMainProxy(sharedProxy);

			UnitTestScreenConfig globalConfig;
			globalConfig.FastInitialize(1024, 768);
			globalConfig.useChannel = UnitTestRemoteChannel::Async;

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"UnitTestFramework/Channel/Async"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource,
				globalConfig
				);
		});

		TEST_CASE(L"Exit on Half Way")
		{
		});

		TEST_CASE(L"Disconnect and Reconnect")
		{
		});
	});
}