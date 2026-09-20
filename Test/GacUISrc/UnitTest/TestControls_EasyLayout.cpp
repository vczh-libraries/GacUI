#include "TestControls.h"

using namespace vl::presentation::compositions::eazy_layout;

namespace easy_layout_xml_tests
{
	WString Resource(const WString& content)
	{
		return L"<Resource>\n<Instance name=\"Main\">\n<Instance ref.Class=\"easy_test::MainWindow\">\n<Window ref.Name=\"self\" Text=\"Easy layout\" ClientSize=\"x:320 y:240\">\n"
			+ content + L"\n</Window>\n</Instance>\n</Instance>\n</Resource>";
	}
}

TEST_FILE
{
	TEST_CASE(L"XML instantiates every descriptor with defaults and both constant property syntaxes")
	{
		for (vint syntax = 0; syntax < 3; syntax++)
		for (vint customNamespace = 0; customNamespace < 2; customNamespace++)
		{
			auto field = [=](const WString& name, const WString& value)
			{
				return syntax == 2 ? L"<att." + name + L">" + value + L"</att." + name + L">" : WString::Empty;
			};
			auto attribute = [=](const WString& name, const WString& value)
			{
				return syntax == 1 ? L" " + name + L"=\"" + value + L"\"" : WString::Empty;
			};
			auto content = L"<ez:Layout ref.Name=\"vertical\"><ez:Bottom ref.Name=\"bottom\"/><ez:Top ref.Name=\"top\"/></ez:Layout>"
				L"<ez:Layout><ez:Right ref.Name=\"right\"/><ez:Left ref.Name=\"left\"/></ez:Layout>"
				L"<ez:Layout><ez:Fill ref.Name=\"fill\"" + attribute(L"Percentage", L"3") + attribute(L"Direction", L"Horizontal") + L">"
				+ field(L"Percentage", L"3") + field(L"Direction", L"Horizontal") + L"<Button Text=\"Payload\"/></ez:Fill></ez:Layout>"
				L"<ez:Layout><ez:Row ref.Name=\"row\"" + attribute(L"CellOption", L"composeType:Absolute absolute:12") + attribute(L"CellSpan", L"1") + L">"
				+ field(L"CellOption", L"composeType:Absolute absolute:12") + field(L"CellSpan", L"1")
				+ L"<ez:Column ref.Name=\"column\"" + attribute(L"CellOption", L"composeType:Percentage percentage:2") + attribute(L"CellSpan", L"1") + L">"
				+ field(L"CellOption", L"composeType:Percentage percentage:2") + field(L"CellSpan", L"1") + L"<Bounds/></ez:Column></ez:Row></ez:Layout>"
				L"<ez:Layout><ez:Column><ez:Row><Bounds/></ez:Row></ez:Column></ez:Layout>";
			auto resource = easy_layout_xml_tests::Resource(content);
			if (customNamespace)
			{
				resource = L"<Resource><Instance name=\"Main\"><Instance ref.Class=\"easy_test::MainWindow\" "
					L"xmlns=\"presentation::controls::Gui*;presentation::compositions::Gui*Composition\" "
					L"xmlns:ez=\"presentation::compositions::eazy_layout::GuiEasy*Composition;presentation::compositions::eazy_layout::GuiEasy*Layout\">"
					L"<Window ref.Name=\"self\" Text=\"Easy layout\" ClientSize=\"x:320 y:240\">" + content + L"</Window></Instance></Instance></Resource>";
			}
			GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Descriptors initialized", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					TEST_ASSERT(FindObjectByName<GuiEasyTopLayout>(window, L"top"));
					TEST_ASSERT(FindObjectByName<GuiEasyBottomLayout>(window, L"bottom"));
					TEST_ASSERT(FindObjectByName<GuiEasyLeftLayout>(window, L"left"));
					TEST_ASSERT(FindObjectByName<GuiEasyRightLayout>(window, L"right"));
					auto fill = FindObjectByName<GuiEasyFillLayout>(window, L"fill");
					TEST_ASSERT(fill->GetPercentage() == (syntax ? 3 : 1));
					TEST_ASSERT(fill->GetDirection() == (syntax ? GuiEasyLayoutDirection::Horizontal : GuiEasyLayoutDirection::Inherited));
					auto row = FindObjectByName<GuiEasyRowLayout>(window, L"row");
					auto column = FindObjectByName<GuiEasyColumnLayout>(window, L"column");
					TEST_ASSERT(row->GetCellSpan() == 1 && column->GetCellSpan() == 1);
					TEST_ASSERT(row->GetCellOption().composeType == (syntax ? GuiCellOption::Absolute : GuiCellOption::MinSize));
					TEST_ASSERT(column->GetCellOption().composeType == (syntax ? GuiCellOption::Percentage : GuiCellOption::MinSize));
					if (syntax)
					{
						TEST_ASSERT(row->GetCellOption().absolute == 12);
						TEST_ASSERT(column->GetCellOption().percentage == 2);
					}
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(L"EasyLayout/Descriptors" + itow(syntax) + itow(customNamespace), L"easy_test::MainWindow", resource);
		}
	});

	TEST_CASE(L"XML builds empty and nested layouts after initial bindings and preserves payload events on rebuild")
	{
		const auto content = LR"GacUISrc(
<ez:Layout ref.Name="outer" Padding="2">
  <ez:Top><CheckBox ref.Name="toggle" Text="Change stored layout" Selected="false"/></ez:Top>
  <ez:Fill>
    <ez:Layout ref.Name="layout" Padding-bind="toggle.Selected ? 9 : 3" Border-bind="toggle.Selected">
      <ez:Fill ref.Name="first" Direction-bind="toggle.Selected ? Vertical : Horizontal">
        <Button ref.Name="button" Text-bind="self.Text"><ev.Clicked-eval><![CDATA[{ self.Text = "Clicked"; }]]></ev.Clicked-eval></Button>
      </ez:Fill>
      <ez:Fill><att.Percentage>2</att.Percentage><Bounds ref.Name="payload" PreferredMinSize="x:20 y:20"/></ez:Fill>
    </ez:Layout>
  </ez:Fill>
  <ez:Bottom>
    <ez:Layout ref.Name="grid" Border="false" Padding="0">
      <ez:Row ref.Name="row" CellSpan-bind="toggle.Selected ? 2 : 1" CellOption="composeType:Absolute absolute:(10+10)">
        <ez:Column CellOption="composeType:Percentage percentage:1"><Bounds PreferredMinSize="x:20 y:20"/></ez:Column>
      </ez:Row>
      <ez:Row><att.CellOption>composeType:Absolute absolute:20</att.CellOption><ez:Column><ez:Layout ref.Name="empty" Padding-eval="7"/></ez:Column></ez:Row>
    </ez:Layout>
  </ez:Bottom>
</ez:Layout>
)GacUISrc";
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Initial bindings applied before build", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				auto empty = FindObjectByName<GuiEasyLayoutComposition>(window, L"empty");
				auto button = FindObjectByName<GuiButton>(window, L"button");
				TEST_ASSERT(layout->GetPadding() == 3 && !layout->GetBorder());
				TEST_ASSERT(layout->GetAlignmentToParent() == Margin(0, 0, 0, 0));
				TEST_ASSERT(empty->GetAlignmentToParent() == Margin(7, 7, 7, 7));
				TEST_ASSERT(button->GetBoundsComposition()->GetAlignmentToParent() == Margin(0, 0, 0, 0));
				TEST_ASSERT(button->GetText() == window->GetText());
				FindObjectByName<GuiSelectableButton>(window, L"toggle")->SetSelected(true);
			});
			protocol->OnNextIdleFrame(L"Bindings change stored values", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				TEST_ASSERT(layout->GetPadding() == 9 && layout->GetBorder());
				TEST_ASSERT(FindObjectByName<GuiEasyFillLayout>(window, L"first")->GetDirection() == GuiEasyLayoutDirection::Vertical);
				TEST_ASSERT(FindObjectByName<GuiEasyRowLayout>(window, L"row")->GetCellSpan() == 2);
				TEST_ASSERT(layout->GetAlignmentToParent() == Margin(0, 0, 0, 0));
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetCellPadding() == 3 && table->GetColumns() == 2);
				layout->BuildLayout();
				FindObjectByName<GuiEasyLayoutComposition>(window, L"grid")->BuildLayout();
			});
			protocol->OnNextIdleFrame(L"Rebuilt with new values", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				TEST_ASSERT(layout->GetAlignmentToParent() == Margin(9, 9, 9, 9));
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetCellPadding() == 9 && table->GetRows() == 2);
				protocol->LClick(protocol->LocationOf(FindObjectByName<GuiButton>(window, L"button")));
			});
			protocol->OnNextIdleFrame(L"Payload event and binding survived", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				TEST_ASSERT(window->GetText() == L"Clicked");
				TEST_ASSERT(FindObjectByName<GuiButton>(window, L"button")->GetText() == L"Clicked");
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(L"EasyLayout/Bindings", L"easy_test::MainWindow", easy_layout_xml_tests::Resource(content));
	});
}
