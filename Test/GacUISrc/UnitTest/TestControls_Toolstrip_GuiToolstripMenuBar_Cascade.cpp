#include "TestControls.h"

TEST_FILE
{
	const auto resourceMenuCascade = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiToolstripMenuBar" ClientSize="x:320 y:240">
        <ToolstripMenuBar>
          <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:-1"/>
          <MenuBarButton ref.Name="menuFile" Alt="F" Text="File">
            <att.SubMenu-set>
              <ToolstripGroupContainer>
                <GuiToolstripGroup>
                  <MenuItemButton ref.Name="menuFileNew" Alt="N" Text="New">
                    <att.SubMenu-set>
                      <MenuItemButton Alt="P" Text="Plain Text" ev.Clicked-eval="self.Text = 'Plain Text';"/>
                      <MenuItemButton Alt="X" Text="XML" ev.Clicked-eval="self.Text = 'XML';"/>
                      <MenuItemButton Alt="J" Text="Json" ev.Clicked-eval="self.Text = 'Json';"/>
                    </att.SubMenu-set>
                  </MenuItemButton>
                  <MenuItemButton Alt="O" Text="Open ..." ev.Clicked-eval="self.Text = 'Open ...';"/>
                </GuiToolstripGroup>
                <GuiToolstripGroup>
                  <MenuItemButton Alt="S" Text="Save" ev.Clicked-eval="self.Text = 'Save';"/>
                  <MenuItemButton Alt="S" Text="Save As ..." ev.Clicked-eval="self.Text = 'Save As ...';"/>
                </GuiToolstripGroup>
                <GuiToolstripGroup>
                  <MenuItemButton Alt="X" Text="Exit" ev.Clicked-eval="self.Text = 'Exit';"/>
                </GuiToolstripGroup>
              </ToolstripGroupContainer>
            </att.SubMenu-set>
          </MenuBarButton>
          <MenuBarButton ref.Name="menuEdit" Alt="E" Text="Edit">
            <att.SubMenu-set>
              <ToolstripGroupContainer>
                <GuiToolstripGroup>
                  <MenuItemButton Alt="Z" Text="Undo" ev.Clicked-eval="self.Text = 'Undo';"/>
                  <MenuItemButton Alt="Y" Text="Redo" ev.Clicked-eval="self.Text = 'Redo';"/>
                </GuiToolstripGroup>
                <GuiToolstripGroup>
                  <MenuItemButton Alt="X" Text="Cut" ev.Clicked-eval="self.Text = 'Cut';"/>
                  <MenuItemButton Alt="C" Text="Copy" ev.Clicked-eval="self.Text = 'Copy';"/>
                  <MenuItemButton Alt="P" Text="Paste" ev.Clicked-eval="self.Text = 'Paste';"/>
                </GuiToolstripGroup>
                <GuiToolstripGroup>
                  <MenuItemButton Alt="A" Text="Select All" ev.Clicked-eval="self.Text = 'Select All';"/>
                </GuiToolstripGroup>
              </ToolstripGroupContainer>
            </att.SubMenu-set>
          </MenuBarButton>
          <MenuBarButton ref.Name="menuAbout" Alt="A" Text="About" ev.Clicked-eval="self.Text = 'About';"/>
        </ToolstripMenuBar>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiToolstripMenuBar/Cascade")
	{
		TEST_CASE(L"DisplaySubMenu")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiToolstripButton>(window, L"menuFile");
					auto location = protocol->LocationOf(menuButton);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click on File", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiToolstripButton>(window, L"menuEdit");
					auto location = protocol->LocationOf(menuButton);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover on Edit", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiToolstripButton>(window, L"menuFile");
					auto location = protocol->LocationOf(menuButton);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover on File", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiToolstripButton>(window, L"menuFileNew");
					auto location = protocol->LocationOf(menuButton);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover on File/New", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiToolstripButton>(window, L"menuAbout");
					auto location = protocol->LocationOf(menuButton);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover on About", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripMenuBar/Cascade/DisplaySubMenu"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceMenuCascade
				);
		});

		TEST_CASE(L"ClickSubMenu")
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
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripMenuBar/Cascade/ClickSubMenu"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceMenuCascade
				);
		});

		TEST_CASE(L"AltSubMenu")
		{
		});
	});
}