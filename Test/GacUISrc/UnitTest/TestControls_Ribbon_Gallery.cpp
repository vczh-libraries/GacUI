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