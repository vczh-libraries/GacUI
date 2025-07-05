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
                    <RibbonLargeDropdownButton ref.Name="buttonLarge" Text="Cert" LargeImage-uri="res://ListViewImages/LargeImages/Cert">
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
                        <ToolstripButton ref.Name="buttonCert" Command-eval="self.commandCert"/>
                        <ToolstripDropdownButton ref.Name="buttonData" Command-eval="self.commandData">
                          <att.SubMenu-set>
                            <MenuItemButton Text="Data1"/>
                            <MenuItemButton Text="Data2"/>
                            <MenuSplitter/>
                            <MenuItemButton Text="Data3"/>
                          </att.SubMenu-set>
                        </ToolstripDropdownButton>
                        <ToolstripSplitButton ref.Name="buttonLink" Command-eval="self.commandLink">
                          <att.SubMenu-set>
                            <MenuItemButton Text="Link1"/>
                            <MenuItemButton Text="Link2"/>
                            <MenuSplitter/>
                            <MenuItemButton Text="Link3"/>
                          </att.SubMenu-set>
                        </ToolstripSplitButton>
                      </att.Buttons>
                    </RibbonButtons>
                    <RibbonSplitter/>
                    <RibbonButtons MaxSize="Large" MinSize="Icon">
                      <att.Buttons>
                        <ToolstripDropdownButton ref.Name="buttonFolder" Command-eval="self.commandFolder">
                          <att.SubMenu-set>
                            <MenuItemButton Text="Folder1"/>
                            <MenuItemButton Text="Folder2"/>
                            <MenuSplitter/>
                            <MenuItemButton Text="Folder3"/>
                          </att.SubMenu-set>
                        </ToolstripDropdownButton>
                        <ToolstripSplitButton ref.Name="buttonLight" Command-eval="self.commandLight">
                          <att.SubMenu-set>
                            <MenuItemButton Text="Light1"/>
                            <MenuItemButton Text="Light2"/>
                            <MenuSplitter/>
                            <MenuItemButton Text="Light3"/>
                          </att.SubMenu-set>
                        </ToolstripSplitButton>
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

		TEST_CASE(L"Dropdowns")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				const wchar_t* ClickBlankFrameName = L"Reset";

				auto clickFrames = [=](const WString& firstFrame, const WString& buttonName)
				{
					protocol->OnNextIdleFrame(firstFrame, [=]()
					{
						protocol->_LDown();
					});
					protocol->OnNextIdleFrame(L"Down[" + buttonName + L"]", [=]()
					{
						protocol->_LUp();
					});
					protocol->OnNextIdleFrame(L"Up[" + buttonName + L"]", [=]()
					{
						protocol->LClick({ { {1},{1} } });
					});
				};

				auto clickButtonFrames = [=, &clickFrames](const WString& firstFrame, const WString& buttonName)
				{
					protocol->OnNextIdleFrame(firstFrame, [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto button = FindObjectByName<GuiToolstripButton>(window, buttonName);
						auto location = protocol->LocationOf(button);
						protocol->MouseMove(location);
					});
					clickFrames(L"Hover[" + buttonName + L"]", buttonName);
				};

				auto clickSplitButtonFrames = [=, &clickFrames](const WString& firstFrame, const WString& buttonName, bool large)
				{
					protocol->OnNextIdleFrame(firstFrame, [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto button = FindObjectByName<GuiToolstripButton>(window, buttonName);
						auto location = large ? protocol->LocationOf(button, 0.5, 0.0, 0, 2) : protocol->LocationOf(button, 0.0, 0.5, 2, 0);
						protocol->MouseMove(location);
					});
					clickFrames(L"Hover[" + buttonName + L"]", buttonName);
					protocol->OnNextIdleFrame(ClickBlankFrameName, [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto button = FindObjectByName<GuiToolstripButton>(window, buttonName);
						auto location = large ? protocol->LocationOf(button, 0.5, 1.0, 0, -2) : protocol->LocationOf(button, 1.0, 0.5, -2, 0);
						protocol->MouseMove(location);
					});
					clickFrames(L"Hover[" + buttonName + L"]", buttonName);
				};

				clickButtonFrames(L"Ready", L"buttonLarge");
				clickButtonFrames(ClickBlankFrameName, L"buttonCert");
				clickButtonFrames(ClickBlankFrameName, L"buttonData");
				clickSplitButtonFrames(ClickBlankFrameName, L"buttonLink", true);
				
				protocol->OnNextIdleFrame(ClickBlankFrameName, [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetClientSize({ 320,window->GetClientSize().y });
				});

				clickButtonFrames(L"Width = 320", L"buttonCert");
				clickButtonFrames(ClickBlankFrameName, L"buttonData");
				clickSplitButtonFrames(ClickBlankFrameName, L"buttonLink", false);
				
				protocol->OnNextIdleFrame(ClickBlankFrameName, [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetClientSize({ 200,window->GetClientSize().y });
				});

				clickButtonFrames(L"Width = 200", L"buttonCert");
				clickButtonFrames(ClickBlankFrameName, L"buttonData");
				clickSplitButtonFrames(ClickBlankFrameName, L"buttonLink", false);
				
				protocol->OnNextIdleFrame(ClickBlankFrameName, [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetClientSize({ 480,window->GetClientSize().y });
				});
				
				protocol->OnNextIdleFrame(L"Width = 480", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Ribbon/GuiRibbonButtons/Dropdowns"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceRibbonButtons
				);
		});
	});
}