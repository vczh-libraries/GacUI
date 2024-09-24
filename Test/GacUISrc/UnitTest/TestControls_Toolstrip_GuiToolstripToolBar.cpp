#include "TestControls.h"

TEST_FILE
{
	const auto resourceToolBar = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">Toolstrip/ToolstripImagesData.xml</Folder>
  <Folder name="ToolstripImages" content="Link">Toolstrip/ToolstripImagesFolder.xml</Folder>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiToolstripMenuBar" ClientSize="x:320 y:240">
        <ToolstripCommand ref.Name="commandLinkRtf" Text="Rtf" Image-uri="res://ToolstripImages/Rtf" ev.Executed-eval="self.Text = 'Rtf';"/>
        <ToolstripCommand ref.Name="commandLinkHtml" Text="Html" Image-uri="res://ToolstripImages/Html" ev.Executed-eval="self.Text = 'Html';"/>
        <ToolstripCommand ref.Name="commandPrivate" Text="Private Format" Image-uri="res://ToolstripImages/Private" ev.Executed-eval="self.Text = 'Private Format';"/>
        <ToolstripCommand ref.Name="commandAlignLeft" ShortcutBuilder="Ctrl+L" Text="Left" Image-uri="res://ToolstripImages/AlignLeft" ev.Executed-eval="{self.Text = 'Left'; buttonAlign.Command = commandAlignLeft;}"/>
        <ToolstripCommand ref.Name="commandAlignCenter" ShortcutBuilder="Ctrl+C" Text="Center" Image-uri="res://ToolstripImages/AlignCenter" ev.Executed-eval="{self.Text = 'Center'; buttonAlign.Command = commandAlignCenter;}"/>
        <ToolstripCommand ref.Name="commandAlignRight" ShortcutBuilder="Ctrl+R" Text="Right Format" Image-uri="res://ToolstripImages/AlignRight" ev.Executed-eval="{self.Text = 'Right'; buttonAlign.Command = commandAlignRight;}"/>
        <ToolstripCommand ref.Name="commandUndo" Text="Undo" Image-uri="res://ToolstripImages/Undo" ev.Executed-eval="self.Text = 'Undo';"/>
        <ToolstripCommand ref.Name="commandRedo" Text="Redo" Image-uri="res://ToolstripImages/Redo" ev.Executed-eval="self.Text = 'Redo';"/>
        <ToolstripCommand ref.Name="commandCut" Text="Cut" Image-uri="res://ToolstripImages/Cut" Enabled="false"/>
        <ToolstripCommand ref.Name="commandCopy" Text="Copy" Image-uri="res://ToolstripImages/Copy" Enabled="false"/>
        <ToolstripCommand ref.Name="commandPaste" Text="Paste" Image-uri="res://ToolstripImages/Paste" Enabled="false"/>
        <ToolstripCommand ref.Name="commandDelete" Text="Delete" Image-uri="res://ToolstripImages/Delete" Enabled="false"/>

        <ToolstripToolBar>
          <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:-1"/>
          <ToolstripGroupContainer>
            <ToolstripGroup>
              <ToolstripDropdownButton ref.Name="buttonLink" Alt="L" Image-uri="res://ToolstripImages/Link">
                <att.SubMenu-set>
                  <MenuItemButton ref.Name="buttonLinkRtf" Alt="R" Command-ref="commandLinkRtf"/>
                  <MenuItemButton ref.Name="buttonLinkHtml" Alt="H" Command-ref="commandLinkHtml"/>
                  <MenuItemButton ref.Name="buttonPrivate" Alt="P" Command-ref="commandPrivate"/>
                </att.SubMenu-set>
              </ToolstripDropdownButton>
              <ToolstripSplitButton ref.Name="buttonAlign" Alt="A" Command-ref="commandAlignLeft">
                <att.SubMenu-set>
                  <MenuItemButton ref.Name="buttonAlignLeft" Alt="L" Command-ref="commandAlignLeft"/>
                  <MenuItemButton ref.Name="buttonAlignCenter" Alt="C" Command-ref="commandAlignCenter"/>
                  <MenuItemButton ref.Name="buttonAlignRight" Alt="R" Command-ref="commandAlignRight"/>
                </att.SubMenu-set>
              </ToolstripSplitButton>
            </ToolstripGroup>
            <ToolstripGroup>
              <ToolstripButton ref.Name="buttonUndo" Alt="U" Command-ref="commandUndo"/>
              <ToolstripButton ref.Name="buttonRedo" Alt="R" Command-ref="commandRedo"/>
            </ToolstripGroup>
            <ToolstripGroup>
              <ToolstripButton ref.Name="buttonCut" Alt="X" Command-ref="commandCut"/>
              <ToolstripButton ref.Name="buttonCopy" Alt="C" Command-ref="commandCopy"/>
              <ToolstripButton ref.Name="buttonPaste" Alt="V" Command-ref="commandPaste"/>
            </ToolstripGroup>
            <ToolstripGroup>
              <ToolstripButton ref.Name="buttonDelete" Alt="D" Command-ref="commandDelete"/>
            </ToolstripGroup>
          </ToolstripGroupContainer>
        </ToolstripToolBar>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiToolstripToolBar")
	{
		TEST_CASE(L"ToolstripButton")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiControl>(window, L"buttonUndo");
					auto location = protocol->LocationOf(menuButton);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover on Undo", [=]()
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
					auto menuButton = FindObjectByName<GuiControl>(window, L"buttonRedo");
					auto location = protocol->LocationOf(menuButton);
					protocol->MClick(location);
				});
				protocol->OnNextIdleFrame(L"Middle Click on Redo", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiControl>(window, L"buttonDelete");
					auto location = protocol->LocationOf(menuButton);
					protocol->RClick(location);
				});
				protocol->OnNextIdleFrame(L"Right Click on Delete", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripToolBar/ToolstripButton"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceToolBar
				);
		});

		TEST_CASE(L"ToolstripDropdownButton")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiControl>(window, L"buttonLink");
					auto location = protocol->LocationOf(menuButton);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Dropdown Link", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiControl>(window, L"buttonLinkHtml");
					auto location = protocol->LocationOf(menuButton);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Html", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripToolBar/ToolstripDropdownButton"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceToolBar
				);
		});

		TEST_CASE(L"ToolstripSplitButton")
		{
			// only works with DarkSkin or any template object with:
			//   GuiButton: buttonArrow

			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiControl>(window, L"buttonAlign");
					auto location = protocol->LocationOf(menuButton);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Align", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiControl>(window, L"buttonAlign");
					auto toolstripTemplate = menuButton->TypedControlTemplateObject(false);
					auto handle = FindObjectByName<GuiButton>(toolstripTemplate, L"buttonArrow");
					auto location = protocol->LocationOf(handle);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Dropdown Align", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiControl>(window, L"buttonAlignRight");
					auto location = protocol->LocationOf(menuButton);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click Align Right", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripToolBar/ToolstripSplitButton"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceToolBar
				);
		});

		TEST_CASE(L"Alt and Shortcut")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_U);
				});
				protocol->OnNextIdleFrame(L"[U]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_R);
				});
				protocol->OnNextIdleFrame(L"[R]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_L);
				});
				protocol->OnNextIdleFrame(L"[L]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_ESCAPE);
				});
				protocol->OnNextIdleFrame(L"[ESC]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_ESCAPE);
				});
				protocol->OnNextIdleFrame(L"[ESC]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_L);
				});
				protocol->OnNextIdleFrame(L"[L]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_ESCAPE);
				});
				protocol->OnNextIdleFrame(L"[ESC]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_L);
				});
				protocol->OnNextIdleFrame(L"[L]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_R);
				});
				protocol->OnNextIdleFrame(L"[R]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_A);
				});
				protocol->OnNextIdleFrame(L"[A]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_C);
				});
				protocol->OnNextIdleFrame(L"[C]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_L, true, false, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[L]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_R, true, false, false);
				});
				protocol->OnNextIdleFrame(L"[CTRL]+[R]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripToolBar/AltAndShortcut"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceToolBar
				);
		});
	});
}