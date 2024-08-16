#include "TestControls.h"

TEST_FILE
{
	const auto resourceHTracker = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiScroll" ClientSize="x:320 y:240">
        <HTracker ref.Name="tracker" TotalSize="10" SmallMove="1" BigMove="3">
          <att.BoundsComposition-set PreferredMinSize="x:150 y:20" AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
        </HTracker>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceVTracker = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiScroll" ClientSize="x:320 y:240">
        <VTracker ref.Name="tracker" TotalSize="10" SmallMove="1" BigMove="3">
          <att.BoundsComposition-set PreferredMinSize="x:20 y:150" AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
        </VTracker>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"HTracker")
	{
		TEST_CASE(L"Properties")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					TEST_ASSERT(tracker->GetTotalSize() == 10);
					TEST_ASSERT(tracker->GetPageSize() == 0);
					TEST_ASSERT(tracker->GetSmallMove() == 1);
					TEST_ASSERT(tracker->GetBigMove() == 3);
					TEST_ASSERT(tracker->GetPosition() == 0);
					TEST_ASSERT(tracker->GetMinPosition() == 0);
					TEST_ASSERT(tracker->GetMaxPosition() == 10);
					TEST_ASSERT(tracker->GetAutoFocus() == true);
					tracker->SetPosition(1);
					TEST_ASSERT(tracker->GetPosition() == 1);
					tracker->SetPosition(-1);
					TEST_ASSERT(tracker->GetPosition() == 0);
					tracker->SetPosition(1);
					TEST_ASSERT(tracker->GetPosition() == 1);
				});
				protocol->OnNextIdleFrame(L"1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					tracker->SetPosition(3);
					TEST_ASSERT(tracker->GetPosition() == 3);
				});
				protocol->OnNextIdleFrame(L"3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					tracker->SetPosition(11);
					TEST_ASSERT(tracker->GetPosition() == 10);
					tracker->SetPosition(3);
					TEST_ASSERT(tracker->GetPosition() == 3);
					tracker->SetPosition(10);
					TEST_ASSERT(tracker->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"10", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/HTracker/Properties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceHTracker
				);
		});

		TEST_CASE(L"Key")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					tracker->SetFocused();
					TEST_ASSERT(tracker->GetFocused());
				});
				protocol->OnNextIdleFrame(L"Focused", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_END);
					TEST_ASSERT(tracker->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"[END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_HOME);
					TEST_ASSERT(tracker->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"[HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_NEXT);
					TEST_ASSERT(tracker->GetPosition() == 3);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_RIGHT);
					TEST_ASSERT(tracker->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"[RIGHT]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
					TEST_ASSERT(tracker->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_PRIOR);
					TEST_ASSERT(tracker->GetPosition() == 7);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_LEFT);
					TEST_ASSERT(tracker->GetPosition() == 6);
				});
				protocol->OnNextIdleFrame(L"[LEFT]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
					TEST_ASSERT(tracker->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/HTracker/Key"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceHTracker
				);
		});

		TEST_CASE(L"Mouse")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->MouseMove({ {20},{45} });
					TEST_ASSERT(tracker->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
				{
					protocol->_LDown();
				});
				protocol->OnNextIdleFrame(L"Drag", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->MouseMove({ {200},{100} });
					TEST_ASSERT(tracker->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"Drag to End", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->MouseMove({ {0},{0} });
					TEST_ASSERT(tracker->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"Drag to Front", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->MouseMove({ {75},{45} });
					TEST_ASSERT(tracker->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Drag to Center", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->_LUp();
					TEST_ASSERT(tracker->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Release", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->MouseMove({ {76},{45} });
					TEST_ASSERT(tracker->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/HTracker/Mouse"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceHTracker
				);
		});
	});

	TEST_CATEGORY(L"VTracker")
	{
		TEST_CASE(L"Properties")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					TEST_ASSERT(tracker->GetTotalSize() == 10);
					TEST_ASSERT(tracker->GetPageSize() == 0);
					TEST_ASSERT(tracker->GetSmallMove() == 1);
					TEST_ASSERT(tracker->GetBigMove() == 3);
					TEST_ASSERT(tracker->GetPosition() == 0);
					TEST_ASSERT(tracker->GetMinPosition() == 0);
					TEST_ASSERT(tracker->GetMaxPosition() == 10);
					TEST_ASSERT(tracker->GetAutoFocus() == true);
					tracker->SetPosition(1);
					TEST_ASSERT(tracker->GetPosition() == 1);
					tracker->SetPosition(-1);
					TEST_ASSERT(tracker->GetPosition() == 0);
					tracker->SetPosition(1);
					TEST_ASSERT(tracker->GetPosition() == 1);
				});
				protocol->OnNextIdleFrame(L"1", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					tracker->SetPosition(3);
					TEST_ASSERT(tracker->GetPosition() == 3);
				});
				protocol->OnNextIdleFrame(L"3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					tracker->SetPosition(11);
					TEST_ASSERT(tracker->GetPosition() == 10);
					tracker->SetPosition(3);
					TEST_ASSERT(tracker->GetPosition() == 3);
					tracker->SetPosition(10);
					TEST_ASSERT(tracker->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"10", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/VTracker/Properties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceVTracker
				);
		});

		TEST_CASE(L"Key")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					tracker->SetFocused();
					TEST_ASSERT(tracker->GetFocused());
				});
				protocol->OnNextIdleFrame(L"Focused", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_END);
					TEST_ASSERT(tracker->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"[END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_HOME);
					TEST_ASSERT(tracker->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"[HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_NEXT);
					TEST_ASSERT(tracker->GetPosition() == 3);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_DOWN);
					TEST_ASSERT(tracker->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
					TEST_ASSERT(tracker->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_PRIOR);
					TEST_ASSERT(tracker->GetPosition() == 7);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_UP);
					TEST_ASSERT(tracker->GetPosition() == 6);
				});
				protocol->OnNextIdleFrame(L"[UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
					TEST_ASSERT(tracker->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/VTracker/Key"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceVTracker
				);
		});

		TEST_CASE(L"Mouse")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->MouseMove({ {25},{43} });
					TEST_ASSERT(tracker->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
				{
					protocol->_LDown();
				});
				protocol->OnNextIdleFrame(L"Drag", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->MouseMove({ {100},{200} });
					TEST_ASSERT(tracker->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"Drag to End", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->MouseMove({ {0},{0} });
					TEST_ASSERT(tracker->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"Drag to Front", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->MouseMove({ {25},{98} });
					TEST_ASSERT(tracker->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Drag to Center", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->_LUp();
					TEST_ASSERT(tracker->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Release", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tracker = FindObjectByName<GuiScroll>(window, L"tracker");
					protocol->MouseMove({ {25},{99} });
					TEST_ASSERT(tracker->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/VTracker/Mouse"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceVTracker
				);
		});
	});
}