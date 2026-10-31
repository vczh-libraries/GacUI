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
        <ToolstripCommand ref.Name="commandAlignLeft" ShortcutBuilder="Ctrl+L" Text="Left" Image-uri="res://ToolstripImages/AlignLeft" ev.Executed-eval="self.Text = 'Left';"/>
        <ToolstripCommand ref.Name="commandAlignCenter" ShortcutBuilder="Ctrl+C" Text="Center" Image-uri="res://ToolstripImages/AlignCenter" ev.Executed-eval="self.Text = 'Center';"/>
        <ToolstripCommand ref.Name="commandAlignRight" ShortcutBuilder="Ctrl+R" Text="Right Format" Image-uri="res://ToolstripImages/AlignRight" ev.Executed-eval="self.Text = 'Right';"/>
        <ToolstripCommand ref.Name="commandUndo" Text="Undo" Image-uri="res://ToolstripImages/Undo" ev.Executed-eval="self.Text = 'Undo';"/>
        <ToolstripCommand ref.Name="commandRedo" Text="Redo" Image-uri="res://ToolstripImages/Redo" ev.Executed-eval="self.Text = 'Redo';"/>
        <ToolstripCommand ref.Name="commandCut" Text="Cut" Image-uri="res://ToolstripImages/Cut" Enabled="false"/>
        <ToolstripCommand ref.Name="commandCopy" Text="Copy" Image-uri="res://ToolstripImages/Copy" Enabled="false"/>
        <ToolstripCommand ref.Name="commandPaste" Text="Paste" Image-uri="res://ToolstripImages/Paste" Enabled="false"/>
        <ToolstripCommand ref.Name="commandDelete" Text="Delete" Image-uri="res://ToolstripImages/Delete" Enabled="false"/>

        <ToolstripMenu ref.Name="menu">
          <ToolstripGroupContainer>
            <ToolstripGroup>
              <ToolstripToolBarInMenu>
                <ToolstripGroupContainer>
                  <ToolstripGroup>
                    <ToolstripDropdownButton ref.Name="buttonLink" Alt="L" Image-uri="res://ToolstripImages/Link">
                      <att.SubMenu-set>
                        <MenuItemButton ref.Name="buttonLinkRtf" Alt="R" Command-ref="commandLinkRtf"/>
                        <MenuItemButton ref.Name="buttonLinkHtml" Alt="H" Command-ref="commandLinkHtml"/>
                        <MenuItemButton ref.Name="buttonPrivate" Alt="P" Command-ref="commandPrivate"/>
                      </att.SubMenu-set>
                    </ToolstripDropdownButton>
                  </ToolstripGroup>
                  <ToolstripGroup>
                    <ToolstripButton ref.Name="buttonAlignLeft" Alt="L" Command-ref="commandAlignLeft"/>
                    <ToolstripButton ref.Name="buttonAlignCenter" Alt="C" Command-ref="commandAlignCenter"/>
                    <ToolstripButton ref.Name="buttonAlignRight" Alt="R" Command-ref="commandAlignRight"/>
                  </ToolstripGroup>
                </ToolstripGroupContainer>
              </ToolstripToolBarInMenu>
            </ToolstripGroup>
            <ToolstripGroup>
              <MenuItemButton ref.Name="buttonUndo" Alt="U" Command-ref="commandUndo"/>
              <MenuItemButton ref.Name="buttonRedo" Alt="R" Command-ref="commandRedo"/>
            </ToolstripGroup>
            <ToolstripGroup>
              <MenuItemButton ref.Name="buttonCut" Alt="X" Command-ref="commandCut"/>
              <MenuItemButton ref.Name="buttonCopy" Alt="C" Command-ref="commandCopy"/>
              <MenuItemButton ref.Name="buttonPaste" Alt="V" Command-ref="commandPaste"/>
            </ToolstripGroup>
            <ToolstripGroup>
              <MenuItemButton ref.Name="buttonDelete" Alt="D" Command-ref="commandDelete"/>
            </ToolstripGroup>
          </ToolstripGroupContainer>
        </ToolstripMenu>

        <Button ref.Name="button" Text="Show Menu">
          <att.BoundsComposition-set AlignmentToParent="left:10 top:10 right:-1 bottom:-1"/>
          <ev.Clicked-eval><![CDATA[{
            menu.ShowPopup(button, true);
          }]]></ev.Clicked-eval>
        </Button>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	for (auto closeOwner : { false,true })
	TEST_CASE(closeOwner ? L"Close context menu owner" : L"Hide context menu owner")
	{
		GuiWindow* owner = nullptr;
		vint closed = 0;
		bool verify = true;
		GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [&]()
			{
				GetApplication()->GetMainWindow()->SetClientSize(Size(640, 480));
				owner = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::MainWindow"));
				owner->SetLocation({ {70},{60} });
				owner->Show();
				auto menu = FindObjectByName<GuiToolstripMenu>(owner, L"menu");
				auto sub = FindObjectByName<GuiToolstripButton>(owner, L"buttonLink");
				sub->GetSubMenu()->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { if (verify) TEST_ASSERT(closed++ == 0); });
				menu->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { if (verify) TEST_ASSERT(closed++ == 1); });
				owner->WindowReadyToClose.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
				{
					if (!verify) return;
					auto menu = FindObjectByName<GuiToolstripMenu>(owner, L"menu");
					auto sub = FindObjectByName<GuiToolstripButton>(owner, L"buttonLink");
					TEST_ASSERT(closed == 2 && !menu->GetOpening() && !sub->GetSubMenuOpening());
					TEST_ASSERT(!sub->GetSubMenu()->GetOpening());
					TEST_ASSERT(!dynamic_cast<IGuiMenuService*>(menu->QueryService(IGuiMenuService::Identifier))->GetOpeningMenu());
					TEST_ASSERT(owner->GetText() == L"GuiToolstripMenuBar");
				});
			});
			protocol->OnNextIdleFrame(L"Secondary context menu owner", [&, protocol]()
			{
				protocol->LClick(protocol->LocationOf(FindObjectByName<GuiControl>(owner, L"button")));
			});
			protocol->OnNextIdleFrame(L"Context menu visible", [&, protocol]()
			{
				protocol->LClick(protocol->LocationOf(FindObjectByName<GuiControl>(owner, L"buttonLink")));
			});
			protocol->OnNextIdleFrame(L"Context menu and submenu visible", [&]()
			{
				TEST_ASSERT(FindObjectByName<GuiToolstripButton>(owner, L"buttonLink")->GetSubMenuOpening());
				if (closeOwner) owner->Close(); else owner->Hide();
			});
			protocol->OnNextIdleFrame(L"Main survives context menu owner", [&]()
			{
				TEST_ASSERT(closed == 2 && !owner->GetOpening());
				verify = false;
				owner->Show();
			});
			protocol->OnNextIdleFrame(L"Owner reopened", [&, protocol]()
			{
				protocol->LClick(protocol->LocationOf(FindObjectByName<GuiControl>(owner, L"button")));
			});
			protocol->OnNextIdleFrame(L"Context menu reopened", [&, protocol]()
			{
				protocol->LClick(protocol->LocationOf(FindObjectByName<GuiControl>(owner, L"buttonUndo")));
				TEST_ASSERT(owner->GetText() == L"Undo");
			});
			protocol->OnNextIdleFrame(L"Reopened context command executes", [&]()
			{
				SafeDeleteControl(owner);
				GetApplication()->GetMainWindow()->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			closeOwner ? L"Controls/Toolstrip/GuiToolstripMenuOwner/Close" : L"Controls/Toolstrip/GuiToolstripMenuOwner/Hide",
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"), resourceToolBar);
	});

	TEST_CASE(L"GuiToolstripMenu")
	{
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindObjectByName<GuiControl>(window, L"button");
				auto location = protocol->LocationOf(button);
				protocol->LClick(location);
			});
			protocol->OnNextIdleFrame(L"Show Menu", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindObjectByName<GuiControl>(window, L"buttonUndo");
				auto location = protocol->LocationOf(button);
				protocol->LClick(location);
			});
			protocol->OnNextIdleFrame(L"Undo", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindObjectByName<GuiControl>(window, L"button");
				auto location = protocol->LocationOf(button);
				protocol->LClick(location);
			});
			protocol->OnNextIdleFrame(L"Show Menu", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindObjectByName<GuiControl>(window, L"buttonLink");
				auto location = protocol->LocationOf(button);
				protocol->LClick(location);
			});
			protocol->OnNextIdleFrame(L"Show Sub Menu", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindObjectByName<GuiControl>(window, L"buttonLinkRtf");
				auto location = protocol->LocationOf(button);
				protocol->LClick(location);
			});
			protocol->OnNextIdleFrame(L"Click Rtf", [=]()
			{
				protocol->KeyPress(VKEY::KEY_L, true, false, false);
			});
			protocol->OnNextIdleFrame(L"[CTRL]+[L]", [=]()
			{
				protocol->KeyPress(VKEY::KEY_C, true, false, false);
			});
			protocol->OnNextIdleFrame(L"[CTRL]+[C]", [=]()
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
			WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripMenu"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceToolBar
			);
	});
}
