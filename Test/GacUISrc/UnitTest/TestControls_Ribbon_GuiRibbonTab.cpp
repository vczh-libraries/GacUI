#include "TestControls.h"

TEST_FILE
{
	const auto resourceTabWithAlt = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonTab" ClientSize="x:320 y:240">
        <RibbonTab ref.Name="tab">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:-1"/>
          <att.Pages>
            <RibbonTabPage ref.Name="tabPageOptions" Text="Options" Alt="O">
              <att.ContainerComposition-set PreferredMinSize="y:110"/>
              <att.Groups>
                <RibbonGroup Text="Option RibbonGroup">
                  <att.Items>
                    <GroupBox Text="Options">
                      <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
                      <Stack Direction="Vertical" Padding="5" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:5 top:5 right:5 bottom:5">
                        <StackItem>
                          <CheckBox Alt="A" Text="Option 1"/>
                        </StackItem>
                        <StackItem>
                          <CheckBox Alt="B" Text="Option 2"/>
                        </StackItem>
                      </Stack>
                    </GroupBox>
                  </att.Items>
                </RibbonGroup>
              </att.Groups>
            </RibbonTabPage>
            <RibbonTabPage ref.Name="tabPageLabel" Text="Label" Alt="L">
              <att.Groups>
                <RibbonGroup Text="Label RibbonGroup">
                  <att.Items>
                    <Label Text="This is a label!">
                      <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
                    </Label>
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

	const auto resourceTabWithHeaders = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonTab" ClientSize="x:480 y:240">
        <RibbonTab ref.Name="tab">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:-1"/>

          <att.BeforeHeaders-set>
            <Button ref.Name="buttonBefore" Text="BEFORE">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                self.Text = "Clicked BEFORE!";
              }]]></ev.Clicked-eval>
            </Button>
          </att.BeforeHeaders-set>

          <att.AfterHeaders-set>
            <Button ref.Name="buttonAfter" Text="AFTER">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                self.Text = "Clicked AFTER!";
              }]]></ev.Clicked-eval>
            </Button>
          </att.AfterHeaders-set>

          <att.Pages>
            <RibbonTabPage ref.Name="tabPageOptions" Text="Options">
              <att.ContainerComposition-set PreferredMinSize="y:110"/>
              <att.Groups>
                <RibbonGroup Text="Option (RG)"/>
              </att.Groups>
            </RibbonTabPage>
            <RibbonTabPage ref.Name="tabPageLabel" Text="Label" Highlighted="true">
              <att.Groups>
                <RibbonGroup Text="Label (RG)"/>
              </att.Groups>
            </RibbonTabPage>
          </att.Pages>
        </RibbonTab>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceTabWithMenu = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">Toolstrip/ToolstripImagesData.xml</Folder>
  <Folder name="ToolstripImages" content="Link">Toolstrip/ToolstripImagesFolder.xml</Folder>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonTab" ClientSize="x:320 y:320">
        <ToolstripCommand ref.Name="commandUndo" Text="Undo" Image-uri="res://ToolstripImages/Undo"/>
        <ToolstripCommand ref.Name="commandRedo" Text="Redo" Image-uri="res://ToolstripImages/Redo"/>
        <ToolstripCommand ref.Name="commandCut" Text="Cut" Image-uri="res://ToolstripImages/Cut" Enabled="false"/>
        <ToolstripCommand ref.Name="commandCopy" Text="Copy" Image-uri="res://ToolstripImages/Copy" Enabled="false"/>
        <ToolstripCommand ref.Name="commandPaste" Text="Paste" Image-uri="res://ToolstripImages/Paste" Enabled="false"/>
        <ToolstripCommand ref.Name="commandDelete" Text="Delete" Image-uri="res://ToolstripImages/Delete" Enabled="false"/>

        <RibbonToolstripMenu ref.Name="ribbonMenu">
          <ToolstripGroupContainer>
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
        </RibbonToolstrioMenu>

        <RibbonTab ref.Name="tab">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:-1"/>

          <att.BeforeHeaders-set>
            <Button ref.Name="buttonHome" Text="HOME">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                ribbonMenu.ShowPopup(buttonHome, true);
              }]]></ev.Clicked-eval>
            </Button>
          </att.BeforeHeaders-set>

          <att.Pages>
            <RibbonTabPage ref.Name="tabPageOptions" Text="Options">
              <att.ContainerComposition-set PreferredMinSize="y:110"/>
            </RibbonTabPage>
            <RibbonTabPage ref.Name="tabPageLabel" Text="Label" Highlighted="true"/>
          </att.Pages>
        </RibbonTab>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiRibbonTab")
	{
		TEST_CASE(L"Navigation")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tab = FindObjectByName<GuiTab>(window, L"tab");
					auto tabPageOptions = FindObjectByName<GuiTabPage>(window, L"tabPageOptions");
					auto tabPageLabel = FindObjectByName<GuiTabPage>(window, L"tabPageLabel");
					TEST_ASSERT(tab->GetPages().Count() == 2);
					TEST_ASSERT(tab->GetPages()[0] == tabPageOptions);
					TEST_ASSERT(tab->GetPages()[1] == tabPageLabel);
					TEST_ASSERT(tabPageOptions->GetOwnerTab() == tab);
					TEST_ASSERT(tabPageLabel->GetOwnerTab() == tab);
					TEST_ASSERT(tab->GetSelectedPage() == tabPageOptions);
					tab->SetSelectedPage(tabPageLabel);
					TEST_ASSERT(tab->GetSelectedPage() == tabPageLabel);
				});
				protocol->OnNextIdleFrame(L"Show Label", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tab = FindObjectByName<GuiTab>(window, L"tab");
					auto tabPageOptions = FindObjectByName<GuiTabPage>(window, L"tabPageOptions");
					auto tabPageLabel = FindObjectByName<GuiTabPage>(window, L"tabPageLabel");
					TEST_ASSERT(tab->GetSelectedPage() == tabPageLabel);
					tab->SetSelectedPage(tabPageOptions);
					TEST_ASSERT(tab->GetSelectedPage() == tabPageOptions);
				});
				protocol->OnNextIdleFrame(L"Show Options", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					protocol->KeyPress(VKEY::KEY_L);
					auto tab = FindObjectByName<GuiTab>(window, L"tab");
					auto tabPage = FindObjectByName<GuiTabPage>(window, L"tabPageLabel");
					TEST_ASSERT(tab->GetSelectedPage() == tabPage);
				});
				protocol->OnNextIdleFrame(L"[L]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					protocol->KeyPress(VKEY::KEY_O);
					auto tab = FindObjectByName<GuiTab>(window, L"tab");
					auto tabPage = FindObjectByName<GuiTabPage>(window, L"tabPageOptions");
					TEST_ASSERT(tab->GetSelectedPage() == tabPage);
				});
				protocol->OnNextIdleFrame(L"[O]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiSelectableButton>(window, L"Option 1");
					TEST_ASSERT(button->GetSelected() == false);
					protocol->KeyPress(VKEY::KEY_MENU);
					protocol->KeyPress(VKEY::KEY_A);
					TEST_ASSERT(button->GetSelected() == true);
				});
				protocol->OnNextIdleFrame(L"[ALT][A]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiSelectableButton>(window, L"Option 2");
					TEST_ASSERT(button->GetSelected() == false);
					protocol->KeyPress(VKEY::KEY_MENU);
					protocol->KeyPress(VKEY::KEY_B);
					TEST_ASSERT(button->GetSelected() == true);
				});
				protocol->OnNextIdleFrame(L"[ALT][B]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					protocol->KeyPress(VKEY::KEY_MENU);
					protocol->KeyPress(VKEY::KEY_L);
					auto tab = FindObjectByName<GuiTab>(window, L"tab");
					auto tabPage = FindObjectByName<GuiTabPage>(window, L"tabPageLabel");
					TEST_ASSERT(tab->GetSelectedPage() == tabPage);
				});
				protocol->OnNextIdleFrame(L"[ALT][L]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					protocol->KeyPress(VKEY::KEY_MENU);
					protocol->KeyPress(VKEY::KEY_O);
					auto tab = FindObjectByName<GuiTab>(window, L"tab");
					auto tabPage = FindObjectByName<GuiTabPage>(window, L"tabPageOptions");
					TEST_ASSERT(tab->GetSelectedPage() == tabPage);
				});
				protocol->OnNextIdleFrame(L"[ALT][O]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Ribbon/GuiRibbonTab/Navigation"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTabWithAlt
				);
		});

		TEST_CASE(L"BeforeHeaders and AfterHeaders")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tab = FindObjectByName<GuiTab>(window, L"tab");
					auto tabPageOptions = FindObjectByName<GuiTabPage>(window, L"tabPageOptions");
					auto tabPageLabel = FindObjectByName<GuiTabPage>(window, L"tabPageLabel");
					TEST_ASSERT(tab->GetPages().Count() == 2);
					TEST_ASSERT(tab->GetPages()[0] == tabPageOptions);
					TEST_ASSERT(tab->GetPages()[1] == tabPageLabel);
					TEST_ASSERT(tabPageOptions->GetOwnerTab() == tab);
					TEST_ASSERT(tabPageLabel->GetOwnerTab() == tab);
					TEST_ASSERT(tab->GetSelectedPage() == tabPageOptions);
					tab->SetSelectedPage(tabPageLabel);
					TEST_ASSERT(tab->GetSelectedPage() == tabPageLabel);
				});
				protocol->OnNextIdleFrame(L"Show Label", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto tab = FindObjectByName<GuiTab>(window, L"tab");
					auto tabPageOptions = FindObjectByName<GuiTabPage>(window, L"tabPageOptions");
					auto tabPageLabel = FindObjectByName<GuiTabPage>(window, L"tabPageLabel");
					TEST_ASSERT(tab->GetSelectedPage() == tabPageLabel);
					tab->SetSelectedPage(tabPageOptions);
					TEST_ASSERT(tab->GetSelectedPage() == tabPageOptions);
				});
				protocol->OnNextIdleFrame(L"Show Options", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"buttonBefore");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click BEFORE", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"buttonAfter");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click AFTER", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetClientSize({ 320,200 });
				});
				protocol->OnNextIdleFrame(L"Window Shrinked", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Ribbon/GuiRibbonTab/Headers"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTabWithHeaders
				);
		});

		TEST_CASE(L"Menu")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindObjectByName<GuiButton>(window, L"buttonHome");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click HOME", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto menu = FindObjectByName<GuiRibbonToolstripMenu>(window, L"ribbonMenu");
					auto button = FindControlByText<GuiToolstripButton>(menu, L"Undo");
					auto location = protocol->LocationOf(button);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Hover Undo", [=]()
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
				WString::Unmanaged(L"Controls/Ribbon/GuiRibbonTab/Menu"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTabWithMenu
				);
		});

		TEST_CASE(L"ContentComposition")
		{
		});
	});

	TEST_CATEGORY(L"GuiRibbonGroup")
	{
		// Automatically resizing window from max to min in width stepping every 32 pixels
		// Expandable
		// Image
	});

	TEST_CATEGORY(L"GuiRibbonButtons")
	{
		// Automatically resizing window from max to min in width stepping every 32 pixels
		// MaxSize/MinSize
		// GuiRibbonIconLabel
		// <Ribbon(Small|Large)(Dropdown|Split)?Button/>
	});

	TEST_CATEGORY(L"GuiRibbonToolstrips")
	{
	});

	TEST_CATEGORY(L"GuiRibbonGallery")
	{
	});
}