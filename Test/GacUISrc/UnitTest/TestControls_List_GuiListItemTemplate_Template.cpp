#include "TestControls_List.h"

namespace gacui_unittest_template
{
	static void InitializeItems(GuiWindow* window, vint count)
	{
		Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(count)));
	}

	void GuiListItemTemplate_TestCases(
		WString resourceXml,
		WString pathFragment
	)
	{
		TEST_CASE(L"MouseVisualEffect")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 1st", [=]()
				{
					protocol->LClick();
				});
				protocol->OnNextIdleFrame(L"Select 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetEnabled(false);
				});
				protocol->OnNextIdleFrame(L"Disabled", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetEnabled(true);
				});
				protocol->OnNextIdleFrame(L"Enabled", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MouseVisualEffect"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Context")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetContext(BoxValue(WString(L"This")));
				});
				protocol->OnNextIdleFrame(L"This", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetContext(BoxValue(WString(L"That")));
				});
				protocol->OnNextIdleFrame(L"That", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetContext({});
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/Context"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Font")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					FontProperties font;
					font.fontFamily = L"This Font";
					font.size = 18;
					listControl->SetFont(font);
				});
				protocol->OnNextIdleFrame(L"18", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					FontProperties font;
					font.fontFamily = L"That Font";
					font.size = 24;
					listControl->SetFont(font);
				});
				protocol->OnNextIdleFrame(L"24", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetFont({});
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(19);
					listControl->SetSelected(19, true);
				});
				protocol->OnNextIdleFrame(L"Scroll to End", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/Font"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"DisplayItemBackground")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
					protocol->LClick();
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
				});
				protocol->OnNextIdleFrame(L"Select 1st and Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetDisplayItemBackground(false);
				});
				protocol->OnNextIdleFrame(L"DisplayItemBackground = false", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(2);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
					protocol->LClick();
					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove(location);
					}
				});
				protocol->OnNextIdleFrame(L"Select 3rd and Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetDisplayItemBackground(true);
				});
				protocol->OnNextIdleFrame(L"DisplayItemBackground = true", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					{
						auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
						auto location = protocol->LocationOf(itemStyle);
						protocol->MouseMove({ location.x + 1,location.y });
					}
				});
				protocol->OnNextIdleFrame(L"Move", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/DisplayItemBackground"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});
	}

	extern void GuiListItemTemplate_WithAxis_TestCases(
		WString resourceXml,
		WString pathFragment
	)
	{
		TEST_CASE(L"ArrangerAndAxis")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 20);
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetArranger(Ptr(new FixedSizeMultiColumnItemArranger));
				});
				protocol->OnNextIdleFrame(L"Grid", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
				});
				protocol->OnNextIdleFrame(L"Rotate (DownLeft)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
				});
				protocol->OnNextIdleFrame(L"Rotate (LeftUp)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
				});
				protocol->OnNextIdleFrame(L"Rotate (UpRight)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
				});
				protocol->OnNextIdleFrame(L"Rotate (RightDown)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetArranger(Ptr(new FixedHeightItemArranger));
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/ArrangerAndAxis"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"ArrangerAndAxisWithScrolls")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 100);
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetArranger(Ptr(new FixedSizeMultiColumnItemArranger));
				});
				protocol->OnNextIdleFrame(L"Grid", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
				});
				protocol->OnNextIdleFrame(L"Rotate (DownLeft)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(0);
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(99);
					listControl->SetSelected(99, true);
				});
				protocol->OnNextIdleFrame(L"Select Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
				});
				protocol->OnNextIdleFrame(L"Rotate (LeftUp)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(0);
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(99);
					listControl->SetSelected(99, true);
				});
				protocol->OnNextIdleFrame(L"Select Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
				});
				protocol->OnNextIdleFrame(L"Rotate (UpRight)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(0);
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(99);
					listControl->SetSelected(99, true);
				});
				protocol->OnNextIdleFrame(L"Select Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
				});
				protocol->OnNextIdleFrame(L"Rotate (RightDown)", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(0);
					listControl->SetSelected(0, true);
				});
				protocol->OnNextIdleFrame(L"Select First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(99);
					listControl->SetSelected(99, true);
				});
				protocol->OnNextIdleFrame(L"Select Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->SetArranger(Ptr(new FixedHeightItemArranger));
				});
				protocol->OnNextIdleFrame(L"Reset", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/ArrangerAndAxisWithScrolls"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});
	}
}