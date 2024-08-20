#include "TestControls_List.h"

namespace gacui_unittest_template
{
	extern void GuiSelectableListControl_SingleSelect_TestCases(
		WString resourceXml,
		WString pathFragment
	)
	{
		TEST_CASE(L"Properties")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					TEST_ASSERT(listControl->GetMultiSelect() == false);

					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SetSelected(1, true);
				});
				protocol->OnNextIdleFrame(L"Select 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SetSelected(1, false);
				});
				protocol->OnNextIdleFrame(L"Unselect 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->ClearSelection();
				});
				protocol->OnNextIdleFrame(L"Clear", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/SingleSelect/Properties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"SelectItemsByClick")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(0, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(2, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(4, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 5th", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(6, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 7th", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(5, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 6th", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(1, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(5, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Click 6th", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(1, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->ClearSelection();
				});
				protocol->OnNextIdleFrame(L"Clear", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/SingleSelect/SelectItemsByClick"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"SelectItemsByKey")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_DOWN, false, false);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_DOWN, false, false);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_DOWN, true, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_DOWN, false, true);
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_DOWN, true, true);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[SHIFT]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_UP, false, false);
				});
				protocol->OnNextIdleFrame(L"[UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_HOME, true, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_NEXT, false, false);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_END, false, true);
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_PRIOR, false, false);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->ClearSelection();
				});
				protocol->OnNextIdleFrame(L"Clear", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/SingleSelect/SelectItemsByKey"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Click")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto SelectItemsByClick = [=](vint itemIndex, bool ctrl, bool shift, bool leftButton)
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					if (leftButton)
					{
						LClickListItem(protocol, listControl, ctrl, shift, false, itemIndex);
					}
					else
					{
						RClickListItem(protocol, listControl, ctrl, shift, false, itemIndex);
					}
				};

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					SelectItemsByClick(0, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					SelectItemsByClick(2, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 3rd", [=]()
				{
					SelectItemsByClick(4, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 5th", [=]()
				{
					SelectItemsByClick(6, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 7th", [=]()
				{
					SelectItemsByClick(5, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 6th", [=]()
				{
					SelectItemsByClick(1, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 2nd", [=]()
				{
					SelectItemsByClick(5, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Click 6th", [=]()
				{
					SelectItemsByClick(1, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->ClearSelection();
				});
				protocol->OnNextIdleFrame(L"Clear", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/SingleSelect/Click"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Key")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SetFocused();

					protocol->KeyPress(VKEY::KEY_DOWN, false, false, false);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_DOWN, false, false, false);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_DOWN, true, false, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_DOWN, false, true, false);
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_DOWN, true, true, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[SHIFT]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_UP, false, false, false);
				});
				protocol->OnNextIdleFrame(L"[UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_HOME, true, false, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_NEXT, false, false, false);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_END, false, true, false);
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_PRIOR, false, false, false);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->ClearSelection();
				});
				protocol->OnNextIdleFrame(L"Clear", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/SingleSelect/Key"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});
	}

	extern void GuiSelectableListControl_MultiSelect_TestCases(
		WString resourceXml,
		WString pathFragment
	)
	{
		TEST_CASE(L"Properties")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					TEST_ASSERT(listControl->GetMultiSelect() == false);
					listControl->SetMultiSelect(true);
					TEST_ASSERT(listControl->GetMultiSelect() == true);

					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SetSelected(1, true);
				});
				protocol->OnNextIdleFrame(L"Select 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SetSelected(1, false);
				});
				protocol->OnNextIdleFrame(L"Unselect 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SetSelected(2, true);
				});
				protocol->OnNextIdleFrame(L"Select 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->ClearSelection();
				});
				protocol->OnNextIdleFrame(L"Clear", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MultiSelect/Properties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"SelectItemsByClick")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SetMultiSelect(true);

					listControl->SelectItemsByClick(0, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(2, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 3rd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(4, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 5th", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(6, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 7th", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(5, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 6th", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(1, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(5, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Click 6th", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByClick(1, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->ClearSelection();
				});
				protocol->OnNextIdleFrame(L"Clear", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MultiSelect/SelectItemsByClick"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"SelectItemsByKey")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SetMultiSelect(true);

					listControl->SelectItemsByKey(VKEY::KEY_DOWN, false, false);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_DOWN, false, false);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_DOWN, true, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_DOWN, false, true);
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_DOWN, true, true);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[SHIFT]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_UP, false, false);
				});
				protocol->OnNextIdleFrame(L"[UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_HOME, true, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_NEXT, false, false);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_END, false, true);
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->SelectItemsByKey(VKEY::KEY_PRIOR, false, false);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->ClearSelection();
				});
				protocol->OnNextIdleFrame(L"Clear", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MultiSelect/SelectItemsByKey"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Click")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto SelectItemsByClick = [=](vint itemIndex, bool ctrl, bool shift, bool leftButton)
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					if (leftButton)
					{
						LClickListItem(protocol, listControl, ctrl, shift, false, itemIndex);
					}
					else
					{
						RClickListItem(protocol, listControl, ctrl, shift, false, itemIndex);
					}
				};

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SetMultiSelect(true);

					SelectItemsByClick(0, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					SelectItemsByClick(2, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 3rd", [=]()
				{
					SelectItemsByClick(4, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 5th", [=]()
				{
					SelectItemsByClick(6, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 7th", [=]()
				{
					SelectItemsByClick(5, true, false, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Click 6th", [=]()
				{
					SelectItemsByClick(1, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 2nd", [=]()
				{
					SelectItemsByClick(5, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Click 6th", [=]()
				{
					SelectItemsByClick(1, true, true, true);
				});
				protocol->OnNextIdleFrame(L"Ctrl+Shift+Click 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->ClearSelection();
				});
				protocol->OnNextIdleFrame(L"Clear", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MultiSelect/Click"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Key")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SetMultiSelect(true);
					listControl->SetFocused();

					protocol->KeyPress(VKEY::KEY_DOWN, false, false, false);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_DOWN, false, false, false);
				});
				protocol->OnNextIdleFrame(L"[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_DOWN, true, false, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_DOWN, false, true, false);
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_DOWN, true, true, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[SHIFT]+[DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_UP, false, false, false);
				});
				protocol->OnNextIdleFrame(L"[UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_HOME, true, false, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_NEXT, false, false, false);
				});
				protocol->OnNextIdleFrame(L"[PAGE DOWN]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_END, false, true, false);
				});
				protocol->OnNextIdleFrame(L"[SHIFT]+[END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();

					protocol->KeyPress(VKEY::KEY_PRIOR, false, false, false);
				});
				protocol->OnNextIdleFrame(L"[PAGE UP]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");

					listControl->ClearSelection();
				});
				protocol->OnNextIdleFrame(L"Clear", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MultiSelect/Key"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});
	}
}