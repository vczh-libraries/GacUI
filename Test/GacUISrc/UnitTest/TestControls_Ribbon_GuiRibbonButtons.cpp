#include "TestControls_Ribbon.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceRibbonButtons = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">ListViewImagesData.xml</Folder>
  <Folder name="ListViewImages" content="Link">ListViewImagesFolder.xml</Folder>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonTab" ClientSize="x:480 y:320">
        <ToolstripCommand ref.Name="commandCert" Text="Cert" LargeImage-uri="res://ListViewImages/LargeImages/Cert" Image-uri="res://ListViewImages/SmallImages/Cert"/>
        <ToolstripCommand ref.Name="commandData" Text="Data" LargeImage-uri="res://ListViewImages/LargeImages/Data" Image-uri="res://ListViewImages/SmallImages/Data"/>
        <ToolstripCommand ref.Name="commandLink" Text="Link" LargeImage-uri="res://ListViewImages/LargeImages/Link" Image-uri="res://ListViewImages/SmallImages/Link"/>
        <ToolstripCommand ref.Name="commandFolder" Text="Folder" LargeImage-uri="res://ListViewImages/LargeImages/Folder" Image-uri="res://ListViewImages/SmallImages/Folder"/>
        <ToolstripCommand ref.Name="commandLight" Text="Light" LargeImage-uri="res://ListViewImages/LargeImages/Light" Image-uri="res://ListViewImages/SmallImages/Light"/>
        <RibbonTab ref.Name="tab">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:-1"/>
          <att.Pages>
            <RibbonTabPage ref.Name="tabPageOptions" Text="Options">
              <att.ContainerComposition-set PreferredMinSize="y:110"/>
              <att.Groups>
                <RibbonGroup ref.Name="group1" Text="Buttons" LargeImage-uri="res://ListViewImages/LargeImages/Cert" Expandable="true">
                  <att.Items>
                    <RibbonLargeDropdownButton Text="Cert" LargeImage-uri="res://ListViewImages/LargeImages/Cert">
                      <att.SubMenu-set>
                        <MenuItemButton Command-eval="self.commandCert"/>
                        <MenuItemButton Command-eval="self.commandData"/>
                        <MenuItemButton Command-eval="self.commandLink"/>
                        <MenuItemButton Command-eval="self.commandFolder"/>
                        <MenuItemButton Command-eval="self.commandLight"/>
                      </att.SubMenu-set>
                    </RibbonLargeDropdownButton>
                    <RibbonSplitter/>
                    <RibbonButtons MaxSize="Large" MinSize="Icon">
                      <att.Buttons>
                        <ToolstripButton Command-eval="self.commandCert"/>
                        <ToolstripDropdownButton Command-eval="self.commandData"/>
                        <ToolstripSplitButton Command-eval="self.commandLink"/>
                      </att.Buttons>
                    </RibbonButtons>
                    <RibbonSplitter/>
                    <RibbonButtons MaxSize="Large" MinSize="Icon">
                      <att.Buttons>
                        <ToolstripDropdownButton Command-eval="self.commandFolder"/>
                        <ToolstripSplitButton Command-eval="self.commandLight"/>
                      </att.Buttons>
                    </RibbonButtons>
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

	TEST_CATEGORY(L"GuiRibbonButtons")
	{
		TEST_CASE(L"ReactiveView")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				TestReactiveView(protocol, L"Ready", 160, 480, 40, [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Ribbon/GuiRibbonButtons/ReactiveView"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceRibbonButtons
				);
		});
	});
}