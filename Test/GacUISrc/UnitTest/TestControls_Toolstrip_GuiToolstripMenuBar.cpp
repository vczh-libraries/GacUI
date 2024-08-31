#include "TestControls.h"

TEST_FILE
{
	const auto resourceMenuBar = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiToolstripMenuBar" ClientSize="x:320 y:240">

        <ToolstripCommand ref.Name="commandFile" Text="File" ShortcutBuilder="Ctrl+F">
          <ev.Executed-eval><![CDATA[{
            self.Text = "Clicked File";
          }]]></ev.Executed-eval>
        </ToolstripCommand>
        <ToolstripCommand ref.Name="commandEdit" Text="Edit" ShortcutBuilder="Ctrl+E">
          <ev.Executed-eval><![CDATA[{
            self.Text = "Clicked Edit";
          }]]></ev.Executed-eval>
        </ToolstripCommand>
        <ToolstripCommand ref.Name="commandAbout" Text="About" ShortcutBuilder="Ctrl+A">
          <ev.Executed-eval><![CDATA[{
            self.Text = "Clicked About";
          }]]></ev.Executed-eval>
        </ToolstripCommand>

        <ToolstripMenuBar>
          <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:-1"/>
          <MenuBarButton Alt="F" Command-ref="commandFile"/>
          <MenuBarButton Alt="E" Command-ref="commandEdit"/>
          <MenuBarButton Alt="A" Command-ref="commandAbout"/>
        </ToolstripMenuBar>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceSubMenu = LR"GacUISrc(
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

	TEST_CATEGORY(L"GuiToolstripMenuBar")
	{
		TEST_CASE(L"Click")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindControlByText<GuiControl>(window, L"File");
					auto location = protocol->LocationOf(menuButton);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover on File", [=]()
				{
					protocol->_LDown();
				});
				protocol->OnNextIdleFrame(L"Left Mouse Down", [=]()
				{
					protocol->_LUp();
				});
				protocol->OnNextIdleFrame(L"Left Mouse Up", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindControlByText<GuiControl>(window, L"Edit");
					auto location = protocol->LocationOf(menuButton);
					protocol->MClick(location);
				});
				protocol->OnNextIdleFrame(L"Middle Click on Edit", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindControlByText<GuiControl>(window, L"About");
					auto location = protocol->LocationOf(menuButton);
					protocol->RClick(location);
				});
				protocol->OnNextIdleFrame(L"Right Click on About", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripMenuBar/Click"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceMenuBar
				);
		});

		TEST_CASE(L"Alt")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_ESCAPE);
				});
				protocol->OnNextIdleFrame(L"[ESC]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_F);
				});
				protocol->OnNextIdleFrame(L"[F]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripMenuBar/Alt"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceMenuBar
				);
		});

		TEST_CASE(L"ShortcutKey")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					protocol->KeyPress(VKEY::KEY_F, true, false, false);
				});
				protocol->OnNextIdleFrame(L"[Ctrl]+[F]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_E, true, false, false);
				});
				protocol->OnNextIdleFrame(L"[Ctrl]+[E]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_A, true, false, false);
				});
				protocol->OnNextIdleFrame(L"[Ctrl]+[A]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripMenuBar/ShortcutKey"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceMenuBar
				);
		});

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
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripMenuBar/DisplaySubMenu"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceSubMenu
				);
		});
	});
}