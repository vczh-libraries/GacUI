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
          <MenuBarButton ref.Name="menuFile" Text="File">
            <att.SubMenu-set>
              <ToolstripGroupContainer>
                <GuiToolstripGroup>
                  <MenuItemButton ref.Name="menuFileNew" Text="New">
                    <att.SubMenu-set>
                      <MenuItemButton Text="Plain Text"/>
                      <MenuItemButton Text="XML"/>
                      <MenuItemButton Text="Json"/>
                    </att.SubMenu-set>
                  </MenuItemButton>
                  <MenuItemButton Text="Open ..."/>
                </GuiToolstripGroup>
                <GuiToolstripGroup>
                  <MenuItemButton Text="Save"/>
                  <MenuItemButton Text="Save As ..."/>
                </GuiToolstripGroup>
                <GuiToolstripGroup>
                  <MenuItemButton Text="Exit"/>
                </GuiToolstripGroup>
              </ToolstripGroupContainer>
            </att.SubMenu-set>
          </MenuBarButton>
          <MenuBarButton ref.Name="menuEdit" Text="Edit">
            <att.SubMenu-set>
              <ToolstripGroupContainer>
                <GuiToolstripGroup>
                  <MenuItemButton Text="Undo"/>
                  <MenuItemButton Text="Redo"/>
                </GuiToolstripGroup>
                <GuiToolstripGroup>
                  <MenuItemButton Text="Cut"/>
                  <MenuItemButton Text="Copy"/>
                  <MenuItemButton Text="Paste"/>
                </GuiToolstripGroup>
                <GuiToolstripGroup>
                  <MenuItemButton Text="Select All"/>
                </GuiToolstripGroup>
              </ToolstripGroupContainer>
            </att.SubMenu-set>
          </MenuBarButton>
          <MenuBarButton ref.Name="menuAbout" Text="About"/>
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