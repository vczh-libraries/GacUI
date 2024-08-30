#include "TestControls_List.h"

namespace gacui_unittest_template
{
	static void InitializeItems(GuiWindow* window, GuiVirtualTreeListControl* listControl, vint count)
	{
		Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(count - 7)));
		auto lastNode = listControl->GetNodeRootProvider()->GetRootNode()->GetChild(count - 7 - 1);
		lastNode->SetExpanding(true);
		lastNode->GetChild(1)->SetExpanding(true);
		lastNode->GetChild(2)->SetExpanding(true);
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
			auto item = Ptr(new TextItem(L ## #EVENT L" " + listControl->GetNodeRootProvider()->GetTextValue(arguments.node)));\
			vint index = logs->GetItems().Add(item);\
			logs->EnsureItemVisible(index);\
			logs->SelectItemsByClick(index, false, false, true);\
		})\

#define ATTACH_NODE_EVENTS\
		ATTACH_NODE_EVENT(NodeLeftButtonDown);\
		ATTACH_NODE_EVENT(NodeLeftButtonUp);\
		ATTACH_NODE_EVENT(NodeLeftButtonDoubleClick);\
		ATTACH_NODE_EVENT(NodeMiddleButtonDown);\
		ATTACH_NODE_EVENT(NodeMiddleButtonUp);\
		ATTACH_NODE_EVENT(NodeMiddleButtonDoubleClick);\
		ATTACH_NODE_EVENT(NodeRightButtonDown);\
		ATTACH_NODE_EVENT(NodeRightButtonUp);\
		ATTACH_NODE_EVENT(NodeRightButtonDoubleClick);\
		ATTACH_NODE_EVENT(NodeMouseMove);\
		ATTACH_NODE_EVENT(NodeMouseEnter);\
		ATTACH_NODE_EVENT(NodeMouseLeave)\

		TEST_CASE(L"LeftMouseEvents")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					ATTACH_NODE_EVENTS;
					
					InitializeItems(window, listControl, 20);
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
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					ATTACH_NODE_EVENTS;

					InitializeItems(window, listControl, 20);
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
					auto listControl = FindObjectByName<GuiVirtualTreeListControl>(window, L"list");
					ATTACH_NODE_EVENTS;

					InitializeItems(window, listControl, 20);
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