#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	/***********************************************************************
	Shared
	***********************************************************************/

		const WString fragmentListViewData = LR"GacUISrc(
  <Script name="ListViewDataResource"><Workflow><![CDATA[
    module treeviewdata;
    using system::*;

    class ListViewData
    {
      prop Text:string = "" {not observe}
    }
  ]]></Workflow></Script>
)GacUISrc";

	/***********************************************************************
	GuiListView
	***********************************************************************/
	{
		const WString resourceTextList = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        func InitializeItems(count:int) : void
        {
          for (item in range[1, count])
          {
            var listViewItem = new ListViewItem^();
            listViewItem.Text = $"Item $(item)";
            listViewItem.SubItems.Add($"1st:$(item * 1)");
            listViewItem.SubItems.Add($"2nd:$(item * 2)");
            listViewItem.SubItems.Add($"3rd:$(item * 3)");
            listViewItem.SubItems.Add($"4th:$(item * 4)");
            list.Items.Add(listViewItem);
          }
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text-format="GuiListView [$(list.SelectedItemIndex)] -&gt; [$(list.SelectedItemText)]" ClientSize="x:640 y:480">
        <ListView ref.Name="list" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false">
          <att.BoundsComposition-set PreferredMinSize="x:400 y:300" AlignmentToParent="left:0 top:5 right:0 bottom:0"/>
          <att.Columns>
            <_ Text="Id"/>
            <_ Size="100" Text="First"/>
            <_ Size="100" Text="Second"/>
            <_ Size="100" Text="Third"/>
            <_ Size="100" Text="Fourth"/>
          </att.Columns>
          <att.DataColumns>
            <_>0</_>
            <_>2</_>
            <_>3</_>
          </att.DataColumns>
        </ListView>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		auto getItems = [](GuiWindow* window)
		{
			auto listControl = FindObjectByName<GuiTextList>(window, L"list");
			return UnboxValue<Ptr<IValueList>>(BoxParameter(listControl->GetItems()));
		};

		auto notifyItemDataModified = [](auto&&...) {};

		TEST_CATEGORY(L"GuiListView")
		{
			GuiVirtualListView_TestCases(
				resourceTextList,
				WString::Unmanaged(L"GuiListView"),
				getItems,
				notifyItemDataModified);
		});
	}
}