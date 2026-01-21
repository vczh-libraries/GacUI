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
				<p>
					<nop>This is the first paragraph.</nop>
					<sp/>
					<object name="InlineOption"/>
					<sp/>
					<nop>After the inline object.</nop>
				</p>
				<p>
					<nop>This is the second paragraph.</nop>
					<sp/>
					<img source="res://ToolstripImages/Copy"/>
					<sp/>
					<nop>After the image.</nop>
				</p>
			</Content>
		</Doc>
	</Doc>

	<Instance name="MainWindowResource">
		<Instance ref.Class="gacuisrc_unittest::MainWindow">
			<Window ref.Name="self" Text="GuiDocumentLabel InlineObject Test" ClientSize="x:480 y:320">
				<DocumentLabel ref.Name="textBox" EditMode="Editable" Document-uri="res://InlineOptionDoc">
					<att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>

					<DocumentItem Name="InlineOption">
						<Label Text="Inline Option">
							<att.BoundsComposition-set MinSizeLimitation="LimitToElementAndChildren"/>
						</Label>
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
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/Features/InlineObject/InlineOption"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	});
}
