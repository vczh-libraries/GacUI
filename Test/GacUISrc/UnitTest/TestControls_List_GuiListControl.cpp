#include "TestControls_List.h"

TEST_FILE
{
	const auto resourceListControl = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        func InitializeItems(start:int, count:int) : void
        {
          for (item in range[1, count])
          {
            list.Items.Add(new TextItem^($"Item $(start + item)"));
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiListControl" ClientSize="x:480 y:320">
        <Table BorderVisible="false" CellPadding="5" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
            <_>composeType:Absolute absolute:220</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <GroupBox Text="Items">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>

          <Cell Site="row:0 column:1">
            <GroupBox Text="Logs">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <TextList ref.Name="logs" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
                <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
              </TextList>
            </GroupBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiListControl")
	{
		TEST_CASE(L"Scrolling")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(0), BoxValue<vint>(10)));
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
					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(10), BoxValue<vint>(10)));
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
				WString::Unmanaged(L"Controls/List/GuiListControl/GuiTextList/Scrolling"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListControl
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

					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(0), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
					TEST_ASSERT(itemStyle != nullptr);
					TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 0);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 1st", [=]()
				{
					protocol->LClick();
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
					TEST_ASSERT(itemStyle != nullptr);
					TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 1);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
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

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(19);
					TEST_ASSERT(itemStyle != nullptr);
					TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 19);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
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
				WString::Unmanaged(L"Controls/List/GuiListControl/GuiTextList/LeftMouseEvents"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListControl
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

					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(0), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
					TEST_ASSERT(itemStyle != nullptr);
					TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 0);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 1st", [=]()
				{
					protocol->MClick();
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
					TEST_ASSERT(itemStyle != nullptr);
					TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 1);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
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

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(19);
					TEST_ASSERT(itemStyle != nullptr);
					TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 19);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
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
				WString::Unmanaged(L"Controls/List/GuiListControl/GuiTextList/MiddleMouseEvents"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListControl
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

					Value::From(window).Invoke(L"InitializeItems", (Value_xs(), BoxValue<vint>(0), BoxValue<vint>(20)));
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(0);
					TEST_ASSERT(itemStyle != nullptr);
					TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 0);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 1st", [=]()
				{
					protocol->RClick();
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
					TEST_ASSERT(itemStyle != nullptr);
					TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 1);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
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

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(19);
					TEST_ASSERT(itemStyle != nullptr);
					TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 19);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
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
				WString::Unmanaged(L"Controls/List/GuiListControl/GuiTextList/RightMouseEvents"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListControl
				);
		});

		TEST_CASE(L"MouseWheel")
		{
			// TODO
		});

#undef ATTACH_ITEM_EVENT
	});
}