#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	/***********************************************************************
	Shared
	***********************************************************************/

	const WString fragmentLoadImages = LR"GacUISrc(
        var largeImages : GuiImageData^[] = null;
        var smallImages : GuiImageData^[] = null;
        func EnsureImages() : void
        {
          if (largeImages is not null)
          {
            return;
          }
          largeImages = {
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/LargeImages/Cert", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/LargeImages/Data", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/LargeImages/Link", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/LargeImages/Folder", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/LargeImages/Light", true));
          };
          smallImages = {
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/SmallImages/Cert", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/SmallImages/Data", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/SmallImages/Link", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/SmallImages/Folder", true));
            (cast (GuiImageData^) self.ResolveResource("res", "ListViewImages/SmallImages/Light", true));
          };
        }
        func GetLargeImage(index:int) : GuiImageData^
        {
          EnsureImages();
          return largeImages[index];
        }
        func GetSmallImage(index:int) : GuiImageData^
        {
          EnsureImages();
          return smallImages[index];
        }
)GacUISrc";

	const WString fragmentNotLoadImages = LR"GacUISrc(
        func GetLargeImage(index:int) : GuiImageData^
        {
          return null;
        }
        func GetSmallImage(index:int) : GuiImageData^
        {
          return null;
        }
)GacUISrc";

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
		const WString fragmentMembers = LR"GacUISrc(
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
            listViewItem.LargeImage = GetLargeImage((item - 1) % 5);
            listViewItem.SmallImage = GetSmallImage((item - 1) % 5);
            list.Items.Add(listViewItem);
          }
        }
)GacUISrc";

		const WString fragmentWindow = LR"GacUISrc(
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
)GacUISrc";

		const WString resourceWithImage = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">ListViewImagesData.xml</Folder>
  <Folder name="ListViewImages" content="Link">ListViewImagesFolder.xml</Folder>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[)GacUISrc" + fragmentLoadImages + fragmentMembers + LR"GacUISrc(
      ]]></ref.Members>)GacUISrc" + fragmentWindow + LR"GacUISrc(
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		const WString resourceWithoutImage = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[)GacUISrc" + fragmentNotLoadImages + fragmentMembers + LR"GacUISrc(
      ]]></ref.Members>)GacUISrc" + fragmentWindow + LR"GacUISrc(
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
			GuiVirtualListView_ViewAndImages_TestCases(
				resourceWithImage,
				WString::Unmanaged(L"GuiListView"));

			GuiVirtualListView_TestCases(
				resourceWithoutImage,
				WString::Unmanaged(L"GuiListView"),
				getItems,
				notifyItemDataModified);
		});
	}
}