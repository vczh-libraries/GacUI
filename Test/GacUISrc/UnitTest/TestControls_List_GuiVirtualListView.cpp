#include "TestControls_List.h"
using namespace gacui_unittest_template;

TEST_FILE
{
	/***********************************************************************
	Shared
	***********************************************************************/

	const WString fragmentImages = LR"GacUISrc(
  <Folder name="UnitTestConfig">
    <Xml name="ImageData">
      <ImageData>
        <Image Path="LargeImages/Cert" Format="Png" Width="32" Height="32"/>
        <Image Path="LargeImages/Data" Format="Png" Width="32" Height="32"/>
        <Image Path="LargeImages/Link" Format="Png" Width="32" Height="32"/>
        <Image Path="LargeImages/Folder" Format="Png" Width="32" Height="32"/>
        <Image Path="LargeImages/Light" Format="Png" Width="32" Height="32"/>
        <Image Path="SmallImages/Cert" Format="Png" Width="16" Height="16"/>
        <Image Path="SmallImages/Data" Format="Png" Width="16" Height="16"/>
        <Image Path="SmallImages/Link" Format="Png" Width="16" Height="16"/>
        <Image Path="SmallImages/Folder" Format="Png" Width="16" Height="16"/>
        <Image Path="SmallImages/Light" Format="Png" Width="16" Height="16"/>
      </ImageData>
    </Xml>
  </Folder>
  <Folder name="LargeImages">
    <Image name="Cert" content="File">Cert_32.png</Image>
    <Image name="Data" content="File">Data_32.png</Image>
    <Image name="Link" content="File">Link_32.png</Image>
    <Image name="Folder" content="File">Folder_32.png</Image>
    <Image name="Light" content="File">Light_32.png</Image>
  </Folder>
  <Folder name="SmallImages">
    <Image name="Cert" content="File">Cert_16.png</Image>
    <Image name="Data" content="File">Data_16.png</Image>
    <Image name="Link" content="File">Link_16.png</Image>
    <Image name="Folder" content="File">Folder_16.png</Image>
    <Image name="Light" content="File">Light_16.png</Image>
  </Folder>
)GacUISrc";

	const WString fragmentLoadImages = LR"GacUISrc(
        largeImages = {
          (cast (GuiImageData^) self.ResolveResource("res", "LargeImages/Cert", true));
          (cast (GuiImageData^) self.ResolveResource("res", "LargeImages/Data", true));
          (cast (GuiImageData^) self.ResolveResource("res", "LargeImages/Link", true));
          (cast (GuiImageData^) self.ResolveResource("res", "LargeImages/Folder", true));
          (cast (GuiImageData^) self.ResolveResource("res", "LargeImages/Light", true));
        };
        smallImages = {
          (cast (GuiImageData^) self.ResolveResource("res", "SmallImages/Cert", true));
          (cast (GuiImageData^) self.ResolveResource("res", "SmallImages/Data", true));
          (cast (GuiImageData^) self.ResolveResource("res", "SmallImages/Link", true));
          (cast (GuiImageData^) self.ResolveResource("res", "SmallImages/Folder", true));
          (cast (GuiImageData^) self.ResolveResource("res", "SmallImages/Light", true));
        };
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
		const WString resourceTextList = LR"GacUISrc(
<Resource>)GacUISrc" + fragmentImages + LR"GacUISrc(
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        var largeImages : GuiImageData^[] = null;
        var smallImages : GuiImageData^[] = null;
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
            listViewItem.LargeImage = largeImages[(item - 1) % 5];
            listViewItem.SmallImage = smallImages[(item - 1) % 5];
            list.Items.Add(listViewItem);
          }
        }
      ]]></ref.Members>
      <ref.Ctor><![CDATA[{)GacUISrc" + fragmentLoadImages + LR"GacUISrc(
      }]]></ref.Ctor>
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