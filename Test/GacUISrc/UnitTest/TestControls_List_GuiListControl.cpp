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
using namespace vl::presentation::controls::list;
using namespace vl::presentation::unittest;

TEST_FILE
{
	const auto resourceListControl= LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiListControl" ClientSize="x:480 y:320">
        <Table BorderVisible="false" CellPadding="5" AlignmentToParent="left:0 top:5 right:0 bottom:0">
          <att.Rows>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:1.0</_>
            <_>composeType:Absolute absolute:160</_>
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
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					for (vint i = 1; i <= 10; i++)
					{
						auto item = Ptr(new TextItem(L"Item " + itow(i)));
						listControl->GetItems().Add(item);
					}
				});
				protocol->OnNextIdleFrame(L"10 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SelectItemsByClick(0, false, false, false);
				});
				protocol->OnNextIdleFrame(L"Highlight First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->SelectItemsByClick(1, false, false, false);
				});
				protocol->OnNextIdleFrame(L"Highlight Second", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					for (vint i = 11; i <= 20; i++)
					{
						auto item = Ptr(new TextItem(L"Item " + itow(i)));
						listControl->GetItems().Add(item);
					}
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(19);
					listControl->SelectItemsByClick(19, false, false, false);
				});
				protocol->OnNextIdleFrame(L"Ensure Last Item Visible", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->GetVerticalScroll()->SetPosition(0);
					listControl->SelectItemsByClick(0, false, false, false);
				});
				protocol->OnNextIdleFrame(L"Ensure First Item Visible", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiListControl/Scrolling"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListControl
				);
		});

#define ATTACH_ITEM_EVENT(EVENT)\
		listControl->EVENT.AttachLambda([=](auto*, auto& arguments)\
		{\
			auto item = Ptr(new TextItem(L ## #EVENT L" " + itow(arguments.itemIndex)));\
			vint index = logs->GetItems().Add(item);\
			logs->EnsureItemVisible(index);\
			logs->SelectItemsByClick(index, false, false, false);\
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
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					auto logs = FindObjectByName<GuiTextList>(window, L"logs");
					ATTACH_ITEM_EVENTS;

					for (vint i = 1; i <= 20; i++)
					{
						auto item = Ptr(new TextItem(L"Item " + itow(i)));
						listControl->GetItems().Add(item);
					}
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

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
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");

					auto itemStyle = listControl->GetArranger()->GetVisibleStyle(1);
					TEST_ASSERT(itemStyle != nullptr);
					TEST_ASSERT(listControl->GetArranger()->GetVisibleIndex(itemStyle) == 1);
					auto location = protocol->LocationOf(itemStyle);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					listControl->EnsureItemVisible(19);

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
				WString::Unmanaged(L"Controls/List/GuiListControl/LeftMouseEvents"),
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
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					for (vint i = 1; i <= 20; i++)
					{
						auto item = Ptr(new TextItem(L"Item " + itow(i)));
						listControl->GetItems().Add(item);
					}
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiListControl/MiddleMouseEvents"),
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
					auto listControl = FindObjectByName<GuiTextList>(window, L"list");
					for (vint i = 1; i <= 20; i++)
					{
						auto item = Ptr(new TextItem(L"Item " + itow(i)));
						listControl->GetItems().Add(item);
					}
				});
				protocol->OnNextIdleFrame(L"20 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/List/GuiListControl/RightMouseEvents"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceListControl
				);
		});

#undef ATTACH_ITEM_EVENT
	});
}