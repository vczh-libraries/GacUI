#include "TestControls_Ribbon.h"
#include "TestControls_List.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceRibbonGallery = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">ListViewImagesData.xml</Folder>
  <Folder name="ListViewImages" content="Link">ListViewImagesFolder.xml</Folder>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonGallery" ClientSize="x:480 y:320">
        <RibbonTab ref.Name="tab">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:-1"/>
          <att.Pages>
            <RibbonTabPage ref.Name="tabPageOptions" Text="Options">
              <att.ContainerComposition-set PreferredMinSize="y:110"/>
              <att.Groups>
                <RibbonGroup ref.Name="group1" Text="Gallery" LargeImage-uri="res://ListViewImages/LargeImages/Cert" Expandable="true">
                  <att.Items>
                    <RibbonGallery ref.Name="gallery">
                      <att.BoundsComposition-set PreferredMinSize="x:200"/>
                      <ev.RequestedScrollUp-eval><![CDATA[{
                        self.Text = "RequestedScrollUp";
                      }]]></ev.RequestedScrollUp-eval>
                      <ev.RequestedScrollDown-eval><![CDATA[{
                        self.Text = "RequestedScrollDown";
                      }]]></ev.RequestedScrollDown-eval>
                      <ev.RequestedDropdown-eval><![CDATA[{
                        self.Text = "RequestedDropdown";
                      }]]></ev.RequestedDropdown-eval>
                    </RibbonGallery>
                  </att.Items>
                </RibbonGroup>
              </att.Groups>
            </RibbonTabPage>
          </att.Pages>
        </RibbonTab>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceRibbonGalleryContainer = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">ListViewImagesData.xml</Folder>
  <Folder name="ListViewImages" content="Link">ListViewImagesFolder.xml</Folder>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonGallery" ClientSize="x:480 y:320">
        <RibbonTab ref.Name="tab">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:-1"/>
          <att.Pages>
            <RibbonTabPage ref.Name="tabPageContainer" Text="Container">
              <att.ContainerComposition-set PreferredMinSize="y:110"/>
              <att.Groups>
                <RibbonGroup ref.Name="group1" Text="Gallery" LargeImage-uri="res://ListViewImages/LargeImages/Cert" Expandable="true">
                  <att.Items>
                    <RibbonGallery ref.Name="gallery">
                      <att.BoundsComposition-set PreferredMinSize="x:200"/>
                      <Button ref.Name="button" Text="Click me!">
                        <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
                        <ev.Clicked-eval><![CDATA[{
                          self.Text = "Clicked!";
                        }]]></ev.Clicked-eval>
                      </Button>
                    </RibbonGallery>
                  </att.Items>
                </RibbonGroup>
              </att.Groups>
            </RibbonTabPage>
          </att.Pages>
        </RibbonTab>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceBindableRibbonGallery = LR"GacUISrc(
<Resource>
  <Script name="GalleryItemResource"><Workflow><![CDATA[
    module galleryitem;
    using system::*;

    class GalleryItem
    {
      prop Id:int = 0 {not observe}

      new(){}
      new(id:int)
      {
        Id=id;
      }
    }
  ]]></Workflow></Script>

  <Instance name="GalleryItemTemplateResource">
    <Instance ref.Class="gacuisrc_unittest::GalleryItemTemplate">
      <ref.Parameter Name="ViewModel" Class="GalleryItem"/>
      <TextListItemTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren" PreferredMinSize="x:72">
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren">
          <att.Rows>
            <_>composeType:Percentage percentage:0.5</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:0.5</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:Percentage percentage:0.5</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:0.5</_>
          </att.Columns>

          <Cell Site="row:1 column:1">
            <Label Text-bind="cast (string) ViewModel.Id ?? ''"/>
          </Cell>
        </Table>
      </TextListItemTemplate>
    </Instance>
  </Instance>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        func InitializeGallery(count:int) : void
        {
          var items : observe GalleryItem^[] = {};
          for (item in range[1, count])
          {
            items.Add(new GalleryItem^(item));
          }
          bindableGallery.ItemSource = items;
        }
      ]]></ref.Members>
      <Window ref.Name="self" Text="GuiBindableRibbonGalleryList" ClientSize="x:480 y:320">
        <RibbonTab ref.Name="tab">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:-1"/>
          <att.Pages>
            <RibbonTabPage ref.Name="tabPageBindable" Text="Bindable Gallery">
              <att.ContainerComposition-set PreferredMinSize="y:110"/>
              <att.Groups>
                <RibbonGroup ref.Name="group1" Text="Bindable Gallery">
                  <att.Items>
                    <BindableRibbonGalleryList ref.Name="bindableGallery" MaxCount="5" MinCount="3" env.ItemType="GalleryItem^">
                      <att.BoundsComposition-set PreferredMinSize="x:200"/>
                      <att.ItemTemplate>gacuisrc_unittest::GalleryItemTemplate</att.ItemTemplate>
                    </BindableRibbonGalleryList>
                  </att.Items>
                </RibbonGroup>
              </att.Groups>
            </RibbonTabPage>
          </att.Pages>
        </RibbonTab>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiRibbonGallery")
	{
		TEST_CASE(L"Events")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto gallery = FindObjectByName<GuiRibbonGallery>(window, L"gallery");
					auto location = protocol->LocationOf(gallery, 1.0, 0.2, -10, 0);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click ScrollUp", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto gallery = FindObjectByName<GuiRibbonGallery>(window, L"gallery");
					auto location = protocol->LocationOf(gallery, 1.0, 0.5, -10, 0);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click ScrollDown", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto gallery = FindObjectByName<GuiRibbonGallery>(window, L"gallery");
					auto location = protocol->LocationOf(gallery, 1.0, 0.8, -10, 0);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Dropdown", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Ribbon/GuiRibbonGallery/Events"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceRibbonGallery
			);
		});

		TEST_CASE(L"Container")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"button");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Clicked Button", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Ribbon/GuiRibbonGallery/Container"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceRibbonGalleryContainer
			);
		});
	});

	TEST_CATEGORY(L"GuiBindableRibbonGalleryList")
	{
		TEST_CASE(L"ReactiveView")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					Value::From(window).Invoke(L"InitializeGallery", (Value_xs(), BoxValue<vint>(4)));
				});
				TestReactiveView(protocol, L"4 Items", 230, 480, 50, [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Ribbon/GuiBindableRibbonGalleryList/ReactiveView"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceBindableRibbonGallery
			);
		});

		TEST_CASE(L"Dropdown")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					Value::From(window).Invoke(L"InitializeGallery", (Value_xs(), BoxValue<vint>(10)));
				});
				protocol->OnNextIdleFrame(L"10 Items", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto gallery = FindObjectByName<GuiBindableRibbonGalleryList>(window, L"bindableGallery");
					auto location = protocol->LocationOf(gallery, 1.0, 0.8, -10, 0);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Dropdown", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto gallery = FindObjectByName<GuiBindableRibbonGalleryList>(window, L"bindableGallery");
					auto listControl = gallery->GetListControlInDropdown();
					LClickListItem(protocol, listControl, 1);
				});
				protocol->OnNextIdleFrame(L"Click Second Item", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Ribbon/GuiBindableRibbonGalleryList/Dropdown"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceBindableRibbonGallery
			);
		});
	});
}