#include "TestControls_List.h"

namespace gacui_unittest_template
{
	static void InitializeItems(GuiWindow* window, vint count)
	{
		Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(count)));
	}

	void GuiVirtualTreeListControl_TestCases(
		WString resourceXml,
		WString pathFragment
	)
	{
#define ATTACH_NODE_EVENT(EVENT)\
		listControl->EVENT.AttachLambda([=](auto*, auto& arguments)\
		{\
			auto logs = FindObjectByName<GuiTextList>(window, L"logs");\
			auto item = Ptr(new TextItem(L ## #EVENT L" " + itow(arguments.itemIndex)));\
			vint index = logs->GetItems().Add(item);\
			logs->EnsureItemVisible(index);\
			logs->SelectItemsByClick(index, false, false, true);\
		})\

#define ATTACH_NODE_EVENTS\
		ATTACH_NODE_EVENT(ItemLeftButtonDown);\
		ATTACH_NODE_EVENT(ItemLeftButtonUp);\
		ATTACH_NODE_EVENT(ItemLeftButtonDoubleClick);\
		ATTACH_NODE_EVENT(ItemMiddleButtonDown);\
		ATTACH_NODE_EVENT(ItemMiddleButtonUp);\
		ATTACH_NODE_EVENT(ItemMiddleButtonDoubleClick);\
		ATTACH_NODE_EVENT(ItemRightButtonDown);\
		ATTACH_NODE_EVENT(ItemRightButtonUp);\
		ATTACH_NODE_EVENT(ItemRightButtonDoubleClick);\
		ATTACH_NODE_EVENT(ItemMouseMove);\
		ATTACH_NODE_EVENT(ItemMouseEnter);\
		ATTACH_NODE_EVENT(ItemMouseLeave)\

		TEST_CASE(L"LeftMouseEvents")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					ATTACH_NODE_EVENTS;
					
					InitializeItems(window, 20);
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
					ATTACH_NODE_EVENTS;

					InitializeItems(window, 20);
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
					ATTACH_NODE_EVENTS;

					InitializeItems(window, 20);
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

#undef ATTACH_NODE_EVENTS
#undef ATTACH_NODE_EVENT
	}
}