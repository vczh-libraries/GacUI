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
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				documentLabel->SetFocused();
				documentLabel->SetCaret({ 0,1 }, { 0,1 });
				protocol->KeyPress(VKEY::KEY_RIGHT, false, true, false);
			});
			protocol->OnNextIdleFrame(L"Select 2nd char @par1", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				documentLabel->SetCaret({ 1,1 }, { 1,1 });
				protocol->KeyPress(VKEY::KEY_RIGHT, false, true, false);
			});
			protocol->OnNextIdleFrame(L"Select 2nd char @par2", [=]()
			{
				protocol->KeyPress(VKEY::KEY_RETURN);
			});
			protocol->OnNextIdleFrame(L"[ENTER] at beginning", [=]()
			{
				protocol->KeyPress(VKEY::KEY_A, true, false ,false);
			});
			protocol->OnNextIdleFrame(L"Select All", [=]()
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

	TEST_CASE(L"InlineObjectWithCaret")
	{
		const auto resource = LR"GacUISrc(
<Resource>
  <Folder name="UnitTestConfig" content="Link">Toolstrip/ToolstripImagesData.xml</Folder>
  <Folder name="ToolstripImages" content="Link">Toolstrip/ToolstripImagesFolder.xml</Folder>

  <Doc name="InlineCaretDoc">
    <Doc>
      <Content>
        <p><img source="res://ToolstripImages/Copy"/>x<img source="res://ToolstripImages/Copy"/></p>
        <p>y<img source="res://ToolstripImages/Copy"/>z</p>
        <p></p>
        <p><img source="res://ToolstripImages/Copy"/></p>
      </Content>
    </Doc>
  </Doc>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentLabel InlineObject Caret Test" ClientSize="x:480 y:320">
        <DocumentLabel ref.Name="textBox" EditMode="Editable" Document-uri="res://InlineCaretDoc">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
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
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				documentLabel->SetFocused();

				auto document = documentLabel->GetDocument();
				TEST_ASSERT(document->paragraphs.Count() == 4);
				auto caretText = document->paragraphs[0]->GetTextForCaret();
				auto readText = document->paragraphs[0]->GetTextForReading();
				TEST_ASSERT(caretText.Length() > 0);
				TEST_ASSERT(readText.Length() > 0);
				TEST_ASSERT(documentLabel->GetCaretBounds(TextPos(0, readText.Length()), true).Width() == 0);
				TEST_ASSERT(documentLabel->GetCaretBounds(TextPos(2, 0), true).Width() == 0);
				TEST_ASSERT(documentLabel->GetCaretBounds(TextPos(3, document->paragraphs[3]->GetTextForCaret().Length()), true).Width() == 0);
				documentLabel->SetCaret({ 0, 0 }, { 0, 0 });
				protocol->KeyPress(VKEY::KEY_RIGHT);

				auto caretBegin = documentLabel->GetCaretBegin();
				auto caretEnd = documentLabel->GetCaretEnd();
				TEST_ASSERT(caretBegin == caretEnd);
				TEST_ASSERT(caretBegin == TextPos(0, 7));
			});
			protocol->OnNextIdleFrame(L"Move right over first inline", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				auto currentCaret = documentLabel->GetCaretBegin();
				documentLabel->SetCaret({ 0, 0 }, { 0, 0 });
				protocol->KeyPress(VKEY::KEY_RIGHT);
				documentLabel->SetCaret(currentCaret, currentCaret);
				protocol->KeyPress(VKEY::KEY_RIGHT);

				auto caretBegin = documentLabel->GetCaretBegin();
				auto caretEnd = documentLabel->GetCaretEnd();
				TEST_ASSERT(caretBegin == caretEnd);
				TEST_ASSERT(caretBegin == TextPos(0, 8));
			});
			protocol->OnNextIdleFrame(L"Move right to end inline", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(documentLabel->GetCaretBegin().row == 0);
				protocol->KeyPress(VKEY::KEY_RIGHT);

				auto caretBegin = documentLabel->GetCaretBegin();
				auto caretEnd = documentLabel->GetCaretEnd();
				TEST_ASSERT(caretBegin == caretEnd);
				TEST_ASSERT(caretBegin == TextPos(0, 15));
			});
			protocol->OnNextIdleFrame(L"Move right over second inline", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(documentLabel->GetCaretBegin().row == 0);
				protocol->KeyPress(VKEY::KEY_END);

				auto caretBegin = documentLabel->GetCaretBegin();
				auto caretEnd = documentLabel->GetCaretEnd();
				TEST_ASSERT(caretBegin == caretEnd);
				TEST_ASSERT(caretBegin == TextPos(3, 7));
			});
			protocol->OnNextIdleFrame(L"[END] from the end of 1st line", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				auto caret = documentLabel->GetCaretBegin();
				TEST_ASSERT(caret.row >= 0);
				protocol->KeyPress(VKEY::KEY_HOME);

				auto caretBegin = documentLabel->GetCaretBegin();
				auto caretEnd = documentLabel->GetCaretEnd();
				TEST_ASSERT(caretBegin == caretEnd);
				TEST_ASSERT(caretBegin == TextPos(3, 0));
			});
			protocol->OnNextIdleFrame(L"[HOME]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				auto text = documentLabel->GetDocument()->paragraphs[0]->GetTextForReading();
				TEST_ASSERT(documentLabel->GetCaretBegin().row >= 0);
				auto caretText = documentLabel->GetDocument()->paragraphs[0]->GetTextForCaret();
				documentLabel->SetCaret(TextPos(0, caretText.Length()), TextPos(0, caretText.Length()));
				protocol->KeyPress(VKEY::KEY_LEFT);

				auto caretBegin = documentLabel->GetCaretBegin();
				auto caretEnd = documentLabel->GetCaretEnd();
				TEST_ASSERT(caretBegin == caretEnd);
				TEST_ASSERT(caretBegin == TextPos(0, 8));
			});
			protocol->OnNextIdleFrame(L"Right skips inline", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(documentLabel->GetCaretBegin().row == 0);
				protocol->KeyPress(VKEY::KEY_RIGHT);
				protocol->KeyPress(VKEY::KEY_DOWN);

				auto caretBegin = documentLabel->GetCaretBegin();
				auto caretEnd = documentLabel->GetCaretEnd();
				TEST_ASSERT(caretBegin == caretEnd);
				TEST_ASSERT(caretBegin == TextPos(1, 9));
			});
			protocol->OnNextIdleFrame(L"Down across lines", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(documentLabel->GetCaretBegin().row == 1);
				protocol->KeyPress(VKEY::KEY_END);

				auto caretBegin = documentLabel->GetCaretBegin();
				auto caretEnd = documentLabel->GetCaretEnd();
				TEST_ASSERT(caretBegin == caretEnd);
				TEST_ASSERT(caretBegin == TextPos(3, 7));
			});
			protocol->OnNextIdleFrame(L"[END] from the end of 2nd line", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				auto text = documentLabel->GetDocument()->paragraphs[1]->GetTextForReading();
				TEST_ASSERT(documentLabel->GetCaretBegin().row >= 0);
				protocol->KeyPress(VKEY::KEY_RETURN, true, false, false);

				auto caretBegin = documentLabel->GetCaretBegin();
				auto caretEnd = documentLabel->GetCaretEnd();
				TEST_ASSERT(caretBegin == caretEnd);
				TEST_ASSERT(caretBegin == TextPos(3, 9));
			});
			protocol->OnNextIdleFrame(L"[CTRL+ENTER]", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				auto text = documentLabel->GetDocument()->paragraphs[1]->GetTextForReading();
				vl::vint crIndex = -1;
				for (vl::vint i = 0; i < text.Length(); i++)
				{
					if (text[i] == L'\r')
					{
						crIndex = i;
						break;
					}
				}
				TEST_ASSERT(crIndex >= -1);
				documentLabel->SetCaret({ 1, 0 }, { 1, 0 });
				protocol->KeyPress(VKEY::KEY_END);

				auto caretBegin = documentLabel->GetCaretBegin();
				auto caretEnd = documentLabel->GetCaretEnd();
				TEST_ASSERT(caretBegin == caretEnd);
				TEST_ASSERT(caretBegin == TextPos(1, 9));
			});
			protocol->OnNextIdleFrame(L"[END] from the beginning of 1st line", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto documentLabel = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				auto text = documentLabel->GetDocument()->paragraphs[1]->GetTextForReading();
				vl::vint crIndex = -1;
				for (vl::vint i = 0; i < text.Length(); i++)
				{
					if (text[i] == L'\r')
					{
						crIndex = i;
						break;
					}
				}
				TEST_ASSERT(crIndex >= -1);
				TEST_ASSERT(documentLabel->GetCaretBegin().row >= 0);
				window->Hide();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/Features/InlineObjectWithCaret"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource
		);
	});
}
