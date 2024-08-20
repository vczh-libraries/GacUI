#include "TestControls_List.h"

namespace gacui_unittest_template
{
	static void InitializeItems(GuiWindow* window, vint start, vint count)
	{
		Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(start), BoxValue<vint>(count)));
	}

	NativePoint GetListItemLocation(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, vint index, vint offsetX)
	{
		auto itemStyle = listControl->GetArranger()->GetVisibleStyle(index);
		TEST_ASSERT(itemStyle != nullptr);
		TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == index);
		auto location = protocol->LocationOf(itemStyle, (offsetX == -1 ? 0.5 : 0.0), 0.5, (offsetX == -1 ? 0 : offsetX), 0);
		return location;
	}

	void HoverListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, vint index, vint offsetX)
	{
		auto location = GetListItemLocation(protocol, listControl, index, offsetX);
		protocol->MouseMove(location);
	}

	void LClickListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, vint index, vint offsetX)
	{
		auto location = GetListItemLocation(protocol, listControl, index, offsetX);
		protocol->LClick(location);
	}

	void LClickListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, bool ctrl, bool shift, bool alt, vint index, vint offsetX)
	{
		auto location = GetListItemLocation(protocol, listControl, index, offsetX);
		protocol->LClick(location, ctrl, shift, alt);
	}

	void LDBClickListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, vint index, vint offsetX)
	{
		auto location = GetListItemLocation(protocol, listControl, index, offsetX);
		protocol->LDBClick(location);
	}

	void RClickListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, vint index, vint offsetX)
	{
		auto location = GetListItemLocation(protocol, listControl, index, offsetX);
		protocol->RClick(location);
	}

	void RClickListItem(UnitTestRemoteProtocol* protocol, GuiListControl* listControl, bool ctrl, bool shift, bool alt, vint index, vint offsetX)
	{
		auto location = GetListItemLocation(protocol, listControl, index, offsetX);
		protocol->LClick(location, ctrl, shift, alt);
	}

	void GuiListControl_TestCases(
		WString resourceXml,
		WString pathFragment
	)
	{
		TEST_CASE(L"Scrolling")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 0, 10);
				});
				protocol->OnNextIdleFrame(L"10 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SelectItemsByClick(0, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Highlight First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->SelectItemsByClick(1, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Highlight Second", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					InitializeItems(window, 10, 10);
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->EnsureItemVisible(19);
					listControl->SelectItemsByClick(19, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Ensure Last Item Visible", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiSelectableListControl>(window, L"list");
					listControl->GetVerticalScroll()->SetPosition(0);
					listControl->SelectItemsByClick(0, false, false, true);
				});
				protocol->OnNextIdleFrame(L"Ensure First Item Visible", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/Scrolling"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

#define ATTACH_ITEM_EVENT(EVENT)\
		listControl->EVENT.AttachLambda([=](auto*, auto& arguments)\
		{\
			auto logs = FindObjectByName<GuiTextList>(window, L"logs");\
			auto item = Ptr(new TextItem(L ## #EVENT L" " + itow(arguments.itemIndex)));\
			vint index = logs->GetItems().Add(item);\
			logs->EnsureItemVisible(index);\
			logs->SelectItemsByClick(index, false, false, true);\
		})\

#define ATTACH_ITEM_EVENTS\
		ATTACH_ITEM_EVENT(ItemLeftButtonDown);\
		ATTACH_ITEM_EVENT(ItemLeftButtonUp);\
		ATTACH_ITEM_EVENT(ItemLeftButtonDoubleClick);\
		ATTACH_ITEM_EVENT(ItemMiddleButtonDown);\
		ATTACH_ITEM_EVENT(ItemMiddleButtonUp);\
		ATTACH_ITEM_EVENT(ItemMiddleButtonDoubleClick);\
		ATTACH_ITEM_EVENT(ItemRightButtonDown);\
		ATTACH_ITEM_EVENT(ItemRightButtonUp);\
		ATTACH_ITEM_EVENT(ItemRightButtonDoubleClick);\
		ATTACH_ITEM_EVENT(ItemMouseMove);\
		ATTACH_ITEM_EVENT(ItemMouseEnter);\
		ATTACH_ITEM_EVENT(ItemMouseLeave)\

		TEST_CASE(L"LeftMouseEvents")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					ATTACH_ITEM_EVENTS;
					
					InitializeItems(window, 0, 20);
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					HoverListItem(protocol, listControl, 0);
				});
				protocol->OnNextIdleFrame(L"Hover 1st", [=]()
				{
					protocol->LClick();
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					HoverListItem(protocol, listControl, 1);
				});
				protocol->OnNextIdleFrame(L"Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Scroll", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					HoverListItem(protocol, listControl, 19);
				});
				protocol->OnNextIdleFrame(L"Hover 20th", [=]()
				{
					protocol->LDBClick();
				});
				protocol->OnNextIdleFrame(L"DbClick 20st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/LeftMouseEvents"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"MiddleMouseEvents")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					ATTACH_ITEM_EVENTS;

					InitializeItems(window, 0, 20);
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					HoverListItem(protocol, listControl, 0);
				});
				protocol->OnNextIdleFrame(L"Hover 1st", [=]()
				{
					protocol->MClick();
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					HoverListItem(protocol, listControl, 1);
				});
				protocol->OnNextIdleFrame(L"Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Scroll", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					HoverListItem(protocol, listControl, 19);
				});
				protocol->OnNextIdleFrame(L"Hover 20th", [=]()
				{
					protocol->MDBClick();
				});
				protocol->OnNextIdleFrame(L"DbClick 20st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/MiddleMouseEvents"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"RightMouseEvents")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					ATTACH_ITEM_EVENTS;

					InitializeItems(window, 0, 20);
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					HoverListItem(protocol, listControl, 0);
				});
				protocol->OnNextIdleFrame(L"Hover 1st", [=]()
				{
					protocol->RClick();
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					HoverListItem(protocol, listControl, 1);
				});
				protocol->OnNextIdleFrame(L"Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					listControl->EnsureItemVisible(19);
				});
				protocol->OnNextIdleFrame(L"Scroll", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					HoverListItem(protocol, listControl, 19);
				});
				protocol->OnNextIdleFrame(L"Hover 20th", [=]()
				{
					protocol->RDBClick();
				});
				protocol->OnNextIdleFrame(L"DbClick 20st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/") + pathFragment + WString::Unmanaged(L"/RightMouseEvents"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"MouseWheel")
		{
			// TODO
		});

#undef ATTACH_ITEM_EVENT
	}
}