#include "TestControls.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	TEST_CASE(L"InlineOption")
	{
		const auto resource = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">Toolstrip/ToolstripImagesData.xml</Folder>
  <Folder name="ToolstripImages" content="Link">Toolstrip/ToolstripImagesFolder.xml</Folder>

  <Doc name="InlineOptionDoc">
    <Doc>
      <Content>
        <p>This is the first paragraph. <object name="Button"/> After the inline object.</p>
        <p>This is the second paragraph. <img source="res://ToolstripImages/Copy"/> After the image.</p>
      </Content>
    </Doc>
  </Doc>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentLabel InlineObject Test" ClientSize="x:480 y:320">
        <DocumentLabel ref.Name="textBox" EditMode="Editable" Document-uri="res://InlineOptionDoc">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>

          <DocumentItem Name="Button">
            <Button Text="Click Me!">
              <att.BoundsComposition-set MinSizeLimitation="LimitToElementAndChildren"/>
              <ev.Clicked-eval><![CDATA[{
                self.Text = "Clicked!";
              }]]></ev.Clicked-eval>
            </Button>
          </DocumentItem>
        </DocumentLabel>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindControlByText<GuiButton>(window, L"Click Me!");
				auto location = protocol->LocationOf(button);
				protocol->LClick(location);
			});
			protocol->OnNextIdleFrame(L"Clicked", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/Features/InlineObject"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	});
}
