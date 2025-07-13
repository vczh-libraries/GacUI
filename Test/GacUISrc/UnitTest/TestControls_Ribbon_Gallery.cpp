#include "TestControls_Ribbon.h"

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
	});
}