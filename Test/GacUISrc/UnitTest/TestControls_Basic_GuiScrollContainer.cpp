#include "TestControls.h"

TEST_FILE
{
	const auto resourceScrollContainer = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" ClientSize="x:320 y:240">
        <att.Text-format>GuiScrollContainer $(container.HorizontalScroll.Position)/$(container.HorizontalScroll.MaxPosition) : $(container.VerticalScroll.Position)/$(container.VerticalScroll.MaxPosition)</att.Text-format>
        <ScrollContainer ref.Name="container">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <Bounds ref.Name="box" PreferredMinSize="x:100 y:100" AlignmentToParent="left:0 top:0 right:0 bottom:0">
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
				Size ew(evs.x + 1, evs.y);
				Size eh(evs.x, evs.y + 1);
				Size es(evs.x + 1, evs.y + 1);

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(evs);
				});
				protocol->OnNextIdleFrame(L"Maximized", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(ew);
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
					box->SetPreferredMinSize(evs);
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(eh);
				});
				protocol->OnNextIdleFrame(L"Extra Height", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs);
					container->SetViewPosition({ 0,1 });
				});
				protocol->OnNextIdleFrame(L"Scroll Vertically", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 1) && vs == evs);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(evs);
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(es);
				});
				protocol->OnNextIdleFrame(L"Extra Size", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs);
					container->SetViewPosition({ 1,1 });
				});
				protocol->OnNextIdleFrame(L"Scroll Both", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(1, 1) && vs == evs);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(evs);
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
		
		TEST_CASE(L"AlwaysInvisible")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				Size evs1(378, 278);
				Size evsw(398, 278);
				Size evsh(378, 298);
				Size evs2(398, 298);

				Size ew(evs2.x + 1, evs1.y);
				Size eh(evs1.x, evs2.y + 1);
				Size es(evs2.x + 1, evs2.y + 1);

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					container->SetHorizontalAlwaysVisible(false);
					container->SetVerticalAlwaysVisible(false);
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs1);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(evs1);
				});
				protocol->OnNextIdleFrame(L"Maximized", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs2);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(ew);
				});
				protocol->OnNextIdleFrame(L"Extra Width", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evsw);
					container->SetViewPosition({ 1,0 });
				});
				protocol->OnNextIdleFrame(L"Scroll Horizontally", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(1, 0) && vs == evsw);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(evs1);
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs2);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(eh);
				});
				protocol->OnNextIdleFrame(L"Extra Height", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evsh);
					container->SetViewPosition({ 0,1 });
				});
				protocol->OnNextIdleFrame(L"Scroll Vertically", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 1) && vs == evsh);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(evs1);
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs2);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(es);
				});
				protocol->OnNextIdleFrame(L"Extra Size", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs1);
					container->SetViewPosition({ 21,21 });
				});
				protocol->OnNextIdleFrame(L"Scroll Both", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(1, 1) && vs == evs1);
					container->SetViewPosition({ 21,21 });
				});
				protocol->OnNextIdleFrame(L"Scroll Both Again", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(21, 21) && vs == evs1);
					auto box = FindObjectByName<GuiBoundsComposition>(window, L"box");
					box->SetPreferredMinSize(evs1);
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					auto vp = container->GetViewPosition();
					auto vs = container->GetViewSize();
					TEST_ASSERT(vp == Point(0, 0) && vs == evs2);
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScrollContainer/AlwaysInvisible"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceScrollContainer
				);
		});

		TEST_CASE(L"ExtendToFullSize")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					container->SetExtendToFullWidth(true);
				});
				protocol->OnNextIdleFrame(L"Extend Horizontally", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					container->SetExtendToFullHeight(true);
				});
				protocol->OnNextIdleFrame(L"Extra Vertically", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					container->SetExtendToFullWidth(false);
				});
				protocol->OnNextIdleFrame(L"Shrink Horizontally", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto container = FindObjectByName<GuiScrollContainer>(window, L"container");
					container->SetExtendToFullHeight(false);
				});
				protocol->OnNextIdleFrame(L"Shrink Vertically", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScrollContainer/ExtendToFullSize"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceScrollContainer
				);
		});
	});
}