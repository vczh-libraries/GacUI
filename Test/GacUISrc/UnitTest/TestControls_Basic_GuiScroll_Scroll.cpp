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
	const auto resourceHScroll= LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiTab" ClientSize="x:320 y:240">
        <HScroll ref.Name="scroll" TotalSize="15" PageSize="5" SmallMove="1" BigMove="3">
          <att.BoundsComposition-set PreferredMinSize="x:150 y:20" AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
        </HScroll>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceVScroll= LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiTab" ClientSize="x:320 y:240">
        <VScroll ref.Name="scroll" TotalSize="15" PageSize="5" SmallMove="1" BigMove="3">
          <att.BoundsComposition-set PreferredMinSize="x:20 y:150" AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
        </VScroll>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"HScroll")
	{
		TEST_CASE(L"Properties")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					TEST_ASSERT(scroll->GetTotalSize() == 15);
					TEST_ASSERT(scroll->GetPageSize() == 5);
					TEST_ASSERT(scroll->GetSmallMove() == 1);
					TEST_ASSERT(scroll->GetBigMove() == 3);
					TEST_ASSERT(scroll->GetPosition() == 0);
					TEST_ASSERT(scroll->GetMinPosition() == 0);
					TEST_ASSERT(scroll->GetMaxPosition() == 10);
					TEST_ASSERT(scroll->GetAutoFocus() == true);
					scroll->SetPosition(1);
					TEST_ASSERT(scroll->GetPosition() == 1);
					scroll->SetPosition(-1);
					TEST_ASSERT(scroll->GetPosition() == 0);
					scroll->SetPosition(1);
					TEST_ASSERT(scroll->GetPosition() == 1);
				});
				protocol->OnNextIdleFrame(L"1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					scroll->SetPosition(3);
					TEST_ASSERT(scroll->GetPosition() == 3);
				});
				protocol->OnNextIdleFrame(L"3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					scroll->SetPosition(11);
					TEST_ASSERT(scroll->GetPosition() == 10);
					scroll->SetPosition(3);
					TEST_ASSERT(scroll->GetPosition() == 3);
					scroll->SetPosition(10);
					TEST_ASSERT(scroll->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"10", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/HScroll/Properties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceHScroll
				);
		});

		TEST_CASE(L"Key")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					scroll->SetFocused();
					TEST_ASSERT(scroll->GetFocused());
				});
				protocol->OnNextIdleFrame(L"Focused", [=]()
				{
					protocol->KeyPress(VKEY::KEY_END);
				});
				protocol->OnNextIdleFrame(L"[END]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_HOME);
				});
				protocol->OnNextIdleFrame(L"[HOME]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_NEXT);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_RIGHT);
				});
				protocol->OnNextIdleFrame(L"[RIGHT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]*3", [=]()
				{
					protocol->KeyPress(VKEY::KEY_PRIOR);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_LEFT);
				});
				protocol->OnNextIdleFrame(L"[LEFT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/HScroll/Key"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceHScroll
				);
		});

		TEST_CASE(L"Mouse")
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
				WString::Unmanaged(L"Controls/Basic/GuiScroll/HScroll/Mouse"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceHScroll
				);
		});
	});

	TEST_CATEGORY(L"VScroll")
	{
		TEST_CASE(L"Properties")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					TEST_ASSERT(scroll->GetTotalSize() == 15);
					TEST_ASSERT(scroll->GetPageSize() == 5);
					TEST_ASSERT(scroll->GetSmallMove() == 1);
					TEST_ASSERT(scroll->GetBigMove() == 3);
					TEST_ASSERT(scroll->GetPosition() == 0);
					TEST_ASSERT(scroll->GetMinPosition() == 0);
					TEST_ASSERT(scroll->GetMaxPosition() == 10);
					TEST_ASSERT(scroll->GetAutoFocus() == true);
					scroll->SetPosition(1);
					TEST_ASSERT(scroll->GetPosition() == 1);
					scroll->SetPosition(-1);
					TEST_ASSERT(scroll->GetPosition() == 0);
					scroll->SetPosition(1);
					TEST_ASSERT(scroll->GetPosition() == 1);
				});
				protocol->OnNextIdleFrame(L"1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					scroll->SetPosition(3);
					TEST_ASSERT(scroll->GetPosition() == 3);
				});
				protocol->OnNextIdleFrame(L"3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					scroll->SetPosition(11);
					TEST_ASSERT(scroll->GetPosition() == 10);
					scroll->SetPosition(3);
					TEST_ASSERT(scroll->GetPosition() == 3);
					scroll->SetPosition(10);
					TEST_ASSERT(scroll->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"10", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/VScroll/Properties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceVScroll
				);
		});

		TEST_CASE(L"Key")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					scroll->SetFocused();
					TEST_ASSERT(scroll->GetFocused());
				});
				protocol->OnNextIdleFrame(L"Focused", [=]()
				{
					protocol->KeyPress(VKEY::KEY_END);
				});
				protocol->OnNextIdleFrame(L"[END]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_HOME);
				});
				protocol->OnNextIdleFrame(L"[HOME]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_NEXT);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_DOWN);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]*3", [=]()
				{
					protocol->KeyPress(VKEY::KEY_PRIOR);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_UP);
				});
				protocol->OnNextIdleFrame(L"[UP]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/VScroll/Key"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceVScroll
				);
		});

		TEST_CASE(L"Mouse")
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
				WString::Unmanaged(L"Controls/Basic/GuiScroll/VScroll/Mouse"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceVScroll
				);
		});
	});
}