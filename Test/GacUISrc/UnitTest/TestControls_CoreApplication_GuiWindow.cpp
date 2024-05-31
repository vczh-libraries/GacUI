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
      <Window ref.Name="self" Text="GuiWindow" ClientSize="x:320 y:240">
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CASE(L"Window Features")
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
			WString::Unmanaged(L"Controls/CoreApplication/WindowFeatures"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
			);
	});
}