#include "TestControls_List.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceRibbonGroup = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">Toolstrip/ToolstripImagesData.xml</Folder>
  <Folder name="ToolstripImages" content="Link">Toolstrip/ToolstripImagesFolder.xml</Folder>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonTab" ClientSize="x:480 y:320">
        <RibbonTab ref.Name="tab">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:-1"/>
          <att.Pages>
            <RibbonTabPage ref.Name="tabPageOptions" Text="Options">
              <att.ContainerComposition-set PreferredMinSize="y:110"/>
              <att.Groups>
                <RibbonGroup Text="1st Group">
                </RibbonGroup>
                <RibbonGroup Text="2nd Group">
                </RibbonGroup>
                <RibbonGroup Text="3rd Group">
                </RibbonGroup>
              </att.Groups>
            </RibbonTabPage>
            <RibbonTabPage ref.Name="tabPageLabel" Text="Label" Highlighted="true"/>
          </att.Pages>
        </RibbonTab>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiRibbonGroup")
	{
		TEST_CASE(L"ReactiveView")
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
				WString::Unmanaged(L"Controls/Ribbon/GuiRibbonGroup/ReactiveView"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceRibbonGroup
				);
		});
	});
}