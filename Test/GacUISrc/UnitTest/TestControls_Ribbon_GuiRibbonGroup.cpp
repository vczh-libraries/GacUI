#include "TestControls_Ribbon.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceRibbonGroup = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">ListViewImagesData.xml</Folder>
  <Folder name="ListViewImages" content="Link">ListViewImagesFolder.xml</Folder>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonTab" ClientSize="x:480 y:320">
        <RibbonTab ref.Name="tab">
          <att.BoundsComposition-set AlignmentToParent="left:0 top:5 right:0 bottom:-1"/>
          <att.Pages>
            <RibbonTabPage ref.Name="tabPageOptions" Text="Options">
              <att.ContainerComposition-set PreferredMinSize="y:110"/>
              <att.Groups>
                <RibbonGroup ref.Name="group1" Text="1st" LargeImage-uri="res://ListViewImages/LargeImages/Cert" Expandable="true">
                  <att.Items>
                    <Label Text="1st Group"/>
                  </att.Items>
                </RibbonGroup>
                <RibbonGroup ref.Name="group2" Text="2nd" LargeImage-uri="res://ListViewImages/LargeImages/Folder">
                  <att.Items>
                    <Label Text="2nd Group"/>
                  </att.Items>
                </RibbonGroup>
                <RibbonGroup ref.Name="group3" Text="3rd" LargeImage-uri="res://ListViewImages/LargeImages/Light">
                  <att.Items>
                    <Label Text="3rd Group"/>
                  </att.Items>
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
				TestReactiveView(protocol, L"Ready", 160, 480, 40, [=]()
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

	TEST_CATEGORY(L"GuiRibbonGroup")
	{
		TEST_CASE(L"ClickCollapsedGroup")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetClientSize({ 240,window->GetClientSize().y });
				});
				protocol->OnNextIdleFrame(L"Width [240]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto group = FindObjectByName<GuiRibbonGroup>(window, L"group1");
					auto location = protocol->LocationOf(group);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click 1st", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto group = FindObjectByName<GuiRibbonGroup>(window, L"group2");
					auto location = protocol->LocationOf(group);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto group = FindObjectByName<GuiRibbonGroup>(window, L"group3");
					auto location = protocol->LocationOf(group);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click 3rd", [=]()
				{
					protocol->LClick(NativePoint{ {1},{1} });
				});
				protocol->OnNextIdleFrame(L"Click Title", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->SetClientSize({ 480,window->GetClientSize().y });
				});
				protocol->OnNextIdleFrame(L"Width [480]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Ribbon/GuiRibbonGroup/ClickCollapsedGroup"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceRibbonGroup
				);
		});
	});

	TEST_CATEGORY(L"ExpandButton")
	{
	});
}