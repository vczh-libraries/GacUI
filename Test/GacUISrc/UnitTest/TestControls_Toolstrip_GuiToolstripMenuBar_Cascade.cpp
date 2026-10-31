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
		for (auto closeOwner : { false,true })
		TEST_CASE(closeOwner ? L"Close cascading menu owner" : L"Hide cascading menu owner")
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
					auto top = FindObjectByName<GuiToolstripButton>(owner, L"menuFile");
					auto sub = FindObjectByName<GuiToolstripButton>(owner, L"menuFileNew");
					sub->GetSubMenu()->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { if (verify) TEST_ASSERT(closed++ == 0); });
					top->GetSubMenu()->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { if (verify) TEST_ASSERT(closed++ == 1); });
					owner->WindowReadyToClose.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
					{
						if (!verify) return;
						auto top = FindObjectByName<GuiToolstripButton>(owner, L"menuFile");
						auto sub = FindObjectByName<GuiToolstripButton>(owner, L"menuFileNew");
						TEST_ASSERT(closed == 2);
						TEST_ASSERT(!top->GetSubMenuOpening() && !sub->GetSubMenuOpening());
						TEST_ASSERT(!top->GetSubMenu()->GetOpening() && !sub->GetSubMenu()->GetOpening());
						TEST_ASSERT(!dynamic_cast<IGuiMenuService*>(top->GetSubMenu()->QueryService(IGuiMenuService::Identifier))->GetOpeningMenu());
						TEST_ASSERT(owner->GetText() == L"GuiToolstripMenuBar");
					});
				});
				protocol->OnNextIdleFrame(L"Secondary menu owner visible", [&, protocol]()
				{
					protocol->LClick(protocol->LocationOf(FindObjectByName<GuiToolstripButton>(owner, L"menuFile")));
				});
				protocol->OnNextIdleFrame(L"Top menu visible", [&, protocol]()
				{
					protocol->MouseMove(protocol->LocationOf(FindObjectByName<GuiToolstripButton>(owner, L"menuFileNew")));
				});
				protocol->OnNextIdleFrame(L"Cascading submenu visible", [&]()
				{
					TEST_ASSERT(FindObjectByName<GuiToolstripButton>(owner, L"menuFileNew")->GetSubMenuOpening());
					if (closeOwner) owner->Close(); else owner->Hide();
				});
				protocol->OnNextIdleFrame(L"Main survives owner and menu cascade", [&]()
				{
					TEST_ASSERT(closed == 2 && !owner->GetOpening());
					verify = false;
					owner->Show();
				});
				protocol->OnNextIdleFrame(L"Owner reopened", [&, protocol]()
				{
					protocol->KeyPress(VKEY::KEY_MENU);
					protocol->KeyPress(VKEY::KEY_F);
					protocol->KeyPress(VKEY::KEY_N);
				});
				protocol->OnNextIdleFrame(L"ALT reopens menu cascade", [&, protocol]()
				{
					TEST_ASSERT(FindObjectByName<GuiToolstripButton>(owner, L"menuFileNew")->GetSubMenuOpening());
					protocol->KeyPress(VKEY::KEY_P);
					TEST_ASSERT(owner->GetText() == L"Plain Text");
				});
				protocol->OnNextIdleFrame(L"Reopened command executes normally", [&]()
				{
					SafeDeleteControl(owner);
					GetApplication()->GetMainWindow()->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				closeOwner ? L"Controls/Toolstrip/GuiToolstripMenuBar/Cascade/CloseOwner" : L"Controls/Toolstrip/GuiToolstripMenuBar/Cascade/HideOwner",
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"), resourceMenuCascade);
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
					auto menuButton = FindObjectByName<GuiControl>(window, L"menuFile");
					auto location = protocol->LocationOf(menuButton);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click on File", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiControl>(window, L"menuEdit");
					auto location = protocol->LocationOf(menuButton);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover on Edit", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiControl>(window, L"menuFile");
					auto location = protocol->LocationOf(menuButton);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover on File", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuButton = FindObjectByName<GuiControl>(window, L"menuFileNew");
					auto location = protocol->LocationOf(menuButton);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover on File/New", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menuFileNew = FindObjectByName<GuiControl>(window, L"menuFileNew");
					auto menuFileSave = FindControlByText<GuiControl>(menuFileNew->GetRelatedControlHost(), L"Save");
					auto location = protocol->LocationOf(menuFileSave, 0.0, 0.5, 8, 0);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover on File/Save", [=]()
				{
					protocol->LClick();
				});
				protocol->OnNextIdleFrame(L"Click", [=]()
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
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_F);
				});
				protocol->OnNextIdleFrame(L"[F]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_N);
				});
				protocol->OnNextIdleFrame(L"[N]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_ESCAPE);
				});
				protocol->OnNextIdleFrame(L"[ESC]", [=]()
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
					protocol->KeyPress(VKEY::KEY_E);
				});
				protocol->OnNextIdleFrame(L"[E]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_ESCAPE);
				});
				protocol->OnNextIdleFrame(L"[ESC]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_F);
				});
				protocol->OnNextIdleFrame(L"[F]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_S);
				});
				protocol->OnNextIdleFrame(L"[S]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_1);
				});
				protocol->OnNextIdleFrame(L"[1]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/GuiToolstripMenuBar/Cascade/AltSubMenu"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceMenuCascade
				);
		});
	});
}
