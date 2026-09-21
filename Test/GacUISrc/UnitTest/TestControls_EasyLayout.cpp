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
	TEST_CASE(L"XML shared splitters resize both axes and rebuilding retains bound controls")
	{
		const auto content = LR"GacUISrc(
<ez:Layout ref.Name="layout">
  <ez:Column CellOption="composeType:Absolute absolute:180">
    <ez:Row CellOption="composeType:Absolute absolute:80">
      <Button ref.Name="button" Text-bind="self.Text"><ev.Clicked-eval><![CDATA[{ self.Text = "Retained"; }]]></ev.Clicked-eval></Button>
    </ez:Row>
    <ez:Splitter ref.Name="horizontal"/>
    <ez:Row CellOption="composeType:Percentage percentage:1"><Label Text="Bottom left"/></ez:Row>
  </ez:Column>
  <ez:Splitter ref.Name="vertical"/>
  <ez:Column CellOption="composeType:Percentage percentage:1">
    <ez:Row><Label Text="Top right"/></ez:Row>
    <ez:Splitter/>
    <ez:Row><ez:Layout Border="false"><ez:Fill><Label Text="Nested"/></ez:Fill><ez:Splitter/><ez:Fill><Label Text="Fills"/></ez:Fill></ez:Layout></ez:Row>
  </ez:Column>
</ez:Layout>
)GacUISrc";
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Shared splitter grid initialized", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				TEST_ASSERT(FindObjectByName<GuiEasySplitterLayout>(window, L"horizontal"));
				TEST_ASSERT(FindObjectByName<GuiEasySplitterLayout>(window, L"vertical"));
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->Children().Count() == 6);
				auto splitter = dynamic_cast<GuiColumnSplitterComposition*>(table->Children()[4]);
				auto point = protocol->LocationOf(splitter);
				protocol->_LDown(point);
				point.x += 25;
				protocol->MouseMove(point);
				protocol->_LUp(point);
			});
			protocol->OnNextIdleFrame(L"Column enlarged by dragging", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetColumnOption(0).absolute == 205);
				auto splitter = dynamic_cast<GuiRowSplitterComposition*>(table->Children()[5]);
				auto point = protocol->LocationOf(splitter);
				protocol->_LDown(point);
				point.y += 15;
				protocol->MouseMove(point);
				protocol->_LUp(point);
			});
			protocol->OnNextIdleFrame(L"Row enlarged by dragging", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetRowOption(0).absolute == 95);
				window->SetClientSize({ 400,300 });
			});
			protocol->OnNextIdleFrame(L"Window resizing retains dragged sizes", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetColumnOption(0).absolute == 205 && table->GetRowOption(0).absolute == 95);
				layout->BuildLayout();
			});
			protocol->OnNextIdleFrame(L"Rebuild restores configured sizes", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto layout = FindObjectByName<GuiEasyLayoutComposition>(window, L"layout");
				auto table = dynamic_cast<GuiTableComposition*>(layout->Children()[0]->Children()[0]);
				TEST_ASSERT(table->GetColumnOption(0).absolute == 180 && table->GetRowOption(0).absolute == 80);
				protocol->LClick(protocol->LocationOf(FindObjectByName<GuiButton>(window, L"button")));
			});
			protocol->OnNextIdleFrame(L"Retained control updates its binding", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				TEST_ASSERT(window->GetText() == L"Retained");
				TEST_ASSERT(FindObjectByName<GuiButton>(window, L"button")->GetText() == L"Retained");
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(L"EasyLayout/Splitters", L"easy_test::MainWindow", easy_layout_xml_tests::Resource(content));
	});

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
				L"<ez:Layout><ez:Column><ez:Row><Bounds/></ez:Row></ez:Column></ez:Layout>"
				L"<ez:Layout Padding=\"0\"><att.Layouts><ez:Fill/><ez:Splitter ref.Name=\"splitter\"/><ez:Fill/></att.Layouts></ez:Layout>";
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
					TEST_ASSERT(FindObjectByName<GuiEasySplitterLayout>(window, L"splitter"));
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

	TEST_CASE(L"Mixed docking payloads stretch to the row height through resizing and rebuilding")
	{
		const auto content = LR"GacUISrc(
<ez:Layout>
  <ez:Bottom>
    <ez:Layout Border="false">
      <ez:Row>
        <ez:Column CellSpan="2">
          <ez:Layout ref.Name="row" Border="false">
            <ez:Right>
              <ComboBox ref.Name="combo" SelectedIndex="0">
                <att.ListControl><TextList><att.Items><TextItem Text="One"/><TextItem Text="Two"/></att.Items></TextList></att.ListControl>
              </ComboBox>
            </ez:Right>
            <ez:Right><CheckBox ref.Name="check" Text="Check"/></ez:Right>
            <ez:Left><Button ref.Name="first" Text="A"/></ez:Left>
            <ez:Left><Button ref.Name="second" Text="B"/></ez:Left>
            <ez:Left><Button ref.Name="third" Text="C"/></ez:Left>
            <ez:Left><Bounds ref.Name="payload" PreferredMinSize="x:10 y:55"/></ez:Left>
          </ez:Layout>
        </ez:Column>
      </ez:Row>
      <ez:Row><ez:Column><Label Text="Left"/></ez:Column><ez:Column><Label Text="Right"/></ez:Column></ez:Row>
    </ez:Layout>
  </ez:Bottom>
</ez:Layout>
)GacUISrc";
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			auto assertRow = []()
			{
				auto window = GetApplication()->GetMainWindow();
				auto payload = FindObjectByName<GuiBoundsComposition>(window, L"payload");
				auto expected = payload->GetGlobalBounds();
				TEST_ASSERT(expected.Height() == 55);
				TEST_ASSERT(payload->GetAlignmentToParent() == Margin(0, 0, 0, 0));
				vint right = -1;
				for (auto name : { L"first",L"second",L"third",L"combo",L"check" })
				{
					auto bounds = FindObjectByName<GuiControl>(window, name)->GetBoundsComposition();
					auto actual = bounds->GetGlobalBounds();
					TEST_ASSERT(bounds->GetAlignmentToParent() == Margin(0, 0, 0, 0));
					TEST_ASSERT(actual.Top() == expected.Top() && actual.Bottom() == expected.Bottom());
					TEST_ASSERT(actual.Left() > right);
					right = actual.Right();
				}
				TEST_ASSERT(FindObjectByName<GuiEasyLayoutComposition>(window, L"row")->GetAlignmentToParent() == Margin(0, 0, 0, 0));
			};
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				assertRow();
				GetApplication()->GetMainWindow()->SetClientSize({ 500,300 });
			});
			protocol->OnNextIdleFrame(L"Row expanded", [=]()
			{
				assertRow();
				auto window = GetApplication()->GetMainWindow();
				FindObjectByName<GuiEasyLayoutComposition>(window, L"row")->BuildLayout();
				window->SetClientSize({ 320,240 });
			});
			protocol->OnNextIdleFrame(L"Row rebuilt and reduced", [=]()
			{
				assertRow();
				GetApplication()->GetMainWindow()->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(L"EasyLayout/MixedHeights", L"easy_test::MainWindow", easy_layout_xml_tests::Resource(content));
	});
}
