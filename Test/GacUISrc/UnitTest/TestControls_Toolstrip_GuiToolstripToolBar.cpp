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
        <ToolstripToolBar>
          <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:-1"/>
          <ToolstripGroupContainer>
            <ToolstripGroup>
              <ToolstripDropdownButton Alt="L" Image-uri="ToolstripImages/Link">
                <att.SubMenu-set>
                  <MenuItemButton Alt="R" Text="Rtf" Image-uri="ToolstripImages/Rtf"/>
                  <MenuItemButton Alt="H" Text="Html" Image-uri="ToolstripImages/Html"/>
                  <MenuItemButton Alt="P" Text="Private Format" Image-uri="ToolstripImages/Private"/>
                </att.SubMenu-set>
              </ToolstripDropdownButton>
              <ToolstripSplitButton Alt="A" Image-uri="ToolstripImages/AlignLeft">
                <att.SubMenu-set>
                  <MenuItemButton Alt="L" Text="Left" Image-uri="ToolstripImages/AlignLeft"/>
                  <MenuItemButton Alt="C" Text="Center" Image-uri="ToolstripImages/AlignCenter"/>
                  <MenuItemButton Alt="R" Text="Right Format" Image-uri="ToolstripImages/AlignRight"/>
                </att.SubMenu-set>
              </ToolstripSplitButton>
            </ToolstripGroup>
            <ToolstripGroup>
              <ToolstripButton Alt="U" Image-uri="ToolstripImages/Undo"/>
              <ToolstripButton Alt="R" Image-uri="ToolstripImages/Redo"/>
            </ToolstripGroup>
            <ToolstripGroup>
              <ToolstripButton Alt="X" Image-uri="ToolstripImages/Cut"/>
              <ToolstripButton Alt="C" Image-uri="ToolstripImages/Copy"/>
              <ToolstripButton Alt="V" Image-uri="ToolstripImages/Paste"/>
            </ToolstripGroup>
            <ToolstripGroup>
              <ToolstripButton Alt="D" Image-uri="ToolstripImages/Delete"/>
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
		});

		TEST_CASE(L"ToolstripSplitButton")
		{
		});

		TEST_CASE(L"Alt")
		{
		});
	});
}