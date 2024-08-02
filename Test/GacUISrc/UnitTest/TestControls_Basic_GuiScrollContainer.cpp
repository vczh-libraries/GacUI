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
	const auto resourceScrollContainer= LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" ClientSize="x:320 y:240">
        <att.Text-format>GuiScrollContainer $(container.HorizontalScroll.Position)/$(container.HorizontalScroll.MaxPosition) : $(container.VerticalScroll.Position)/$(container.VerticalScroll.MaxPosition)</att.Text-format>
        <ScrollContainer ref.Name="container">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <Bounds ref.Name="box" PreferredMinSize="x:100 y:100">
            <SolidBorder Color="#FF0000"/>
          </Bounds>
        </ScrollContainer>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiScrollContainer")
	{
		TEST_CASE(L"AlwaysVisible")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				Size evs(378, 278);

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize({ 378,278 });
				});
				protocol->OnNextIdleFrame(L"Maximized", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize({ 379,278 });
				});
				protocol->OnNextIdleFrame(L"Extra Width", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs);
					container->SetViewPosition({ 1,0 });
				});
				protocol->OnNextIdleFrame(L"Scroll Horizontally", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(1, 0) && vs == evs);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize({ 378,278 });
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs);
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScrollContainer/AlwaysVisible"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceScrollContainer
				);
		});
	});
}