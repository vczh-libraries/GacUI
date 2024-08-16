#include "TestControls.h"

TEST_FILE
{
	const auto resourceHScroll = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiScroll" ClientSize="x:320 y:240">
        <HScroll ref.Name="scroll" TotalSize="15" PageSize="5" SmallMove="1" BigMove="3">
          <att.BoundsComposition-set PreferredMinSize="x:150 y:20" AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
        </HScroll>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceVScroll = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiScroll" ClientSize="x:320 y:240">
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
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_END);
					TEST_ASSERT(scroll->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"[END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_HOME);
					TEST_ASSERT(scroll->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"[HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_NEXT);
					TEST_ASSERT(scroll->GetPosition() == 3);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_RIGHT);
					TEST_ASSERT(scroll->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"[RIGHT]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
					TEST_ASSERT(scroll->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_PRIOR);
					TEST_ASSERT(scroll->GetPosition() == 7);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_LEFT);
					TEST_ASSERT(scroll->GetPosition() == 6);
				});
				protocol->OnNextIdleFrame(L"[LEFT]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
					TEST_ASSERT(scroll->GetPosition() == 0);
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

		TEST_CASE(L"Click")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				NativePoint sd(25, 45);
				NativePoint bd(33, 45);
				NativePoint si(150, 45);
				NativePoint bi(141, 45);

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(bi);
					TEST_ASSERT(scroll->GetPosition() == 3);
				});
				protocol->OnNextIdleFrame(L"Big Increase", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(si);
					TEST_ASSERT(scroll->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Small Increase", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(bi);
					protocol->LClick(bi);
					protocol->LClick(bi);
					TEST_ASSERT(scroll->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"Big Increase*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(bd);
					TEST_ASSERT(scroll->GetPosition() == 7);
				});
				protocol->OnNextIdleFrame(L"Big Decrease", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(sd);
					TEST_ASSERT(scroll->GetPosition() == 6);
				});
				protocol->OnNextIdleFrame(L"Small Decrease", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(bd);
					protocol->LClick(bd);
					protocol->LClick(bd);
					TEST_ASSERT(scroll->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"Big Decrease*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/HScroll/Click"),
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
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->MouseMove({ {50},{45} });
					TEST_ASSERT(scroll->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
				{
					protocol->_LDown();
				});
				protocol->OnNextIdleFrame(L"Drag", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->MouseMove({ {200},{100} });
					TEST_ASSERT(scroll->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"Drag to End", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->MouseMove({ {0},{0} });
					TEST_ASSERT(scroll->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"Drag to Front", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->MouseMove({ {80},{45} });
					TEST_ASSERT(scroll->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Drag to Center", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->_LUp();
					TEST_ASSERT(scroll->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Release", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->MouseMove({ {81},{45} });
					TEST_ASSERT(scroll->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
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
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_END);
					TEST_ASSERT(scroll->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"[END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_HOME);
					TEST_ASSERT(scroll->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"[HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_NEXT);
					TEST_ASSERT(scroll->GetPosition() == 3);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_DOWN);
					TEST_ASSERT(scroll->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
					protocol->KeyPress(VKEY::KEY_NEXT);
					TEST_ASSERT(scroll->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_PRIOR);
					TEST_ASSERT(scroll->GetPosition() == 7);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_UP);
					TEST_ASSERT(scroll->GetPosition() == 6);
				});
				protocol->OnNextIdleFrame(L"[UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
					protocol->KeyPress(VKEY::KEY_PRIOR);
					TEST_ASSERT(scroll->GetPosition() == 0);
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

		TEST_CASE(L"Click")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				NativePoint sd(22, 52);
				NativePoint bd(22, 61);
				NativePoint si(22, 178);
				NativePoint bi(22, 169);

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(bi);
					TEST_ASSERT(scroll->GetPosition() == 3);
				});
				protocol->OnNextIdleFrame(L"Big Increase", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(si);
					TEST_ASSERT(scroll->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Small Increase", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(bi);
					protocol->LClick(bi);
					protocol->LClick(bi);
					TEST_ASSERT(scroll->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"Big Increase*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(bd);
					TEST_ASSERT(scroll->GetPosition() == 7);
				});
				protocol->OnNextIdleFrame(L"Big Decrease", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(sd);
					TEST_ASSERT(scroll->GetPosition() == 6);
				});
				protocol->OnNextIdleFrame(L"Small Decrease", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->LClick(bd);
					protocol->LClick(bd);
					protocol->LClick(bd);
					TEST_ASSERT(scroll->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"Big Decrease*3", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Basic/GuiScroll/VScroll/Click"),
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
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->MouseMove({ {22},{73} });
					TEST_ASSERT(scroll->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
				{
					protocol->_LDown();
				});
				protocol->OnNextIdleFrame(L"Drag", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->MouseMove({ {100},{200} });
					TEST_ASSERT(scroll->GetPosition() == 10);
				});
				protocol->OnNextIdleFrame(L"Drag to End", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->MouseMove({ {0},{0} });
					TEST_ASSERT(scroll->GetPosition() == 0);
				});
				protocol->OnNextIdleFrame(L"Drag to Front", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->MouseMove({ {22},{103} });
					TEST_ASSERT(scroll->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Drag to Center", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->_LUp();
					TEST_ASSERT(scroll->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Release", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto scroll = FindObjectByName<GuiScroll>(window, L"scroll");
					protocol->MouseMove({ {22},{104} });
					TEST_ASSERT(scroll->GetPosition() == 4);
				});
				protocol->OnNextIdleFrame(L"Hover", [=]()
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