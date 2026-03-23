#include "TestControls.h"

using namespace gacui_unittest_template;

namespace
{
	Ptr<DocumentStyleProperties> MakeStyleWithBold(bool bold)
	{
		auto style = Ptr(new DocumentStyleProperties);
		style->bold = bold;
		return style;
	}

	Ptr<DocumentStyleProperties> MakeStyleWithItalic(bool italic)
	{
		auto style = Ptr(new DocumentStyleProperties);
		style->italic = italic;
		return style;
	}

	Ptr<DocumentStyleProperties> MakeStyleWithColor(Color color)
	{
		auto style = Ptr(new DocumentStyleProperties);
		style->color = color;
		return style;
	}

	void RegisterStyle(GuiDocumentCommonInterface* textBox, const WString& styleName, const WString& parentStyleName, Ptr<DocumentStyleProperties> properties)
	{
		auto document = textBox->GetDocument();
		auto style = Ptr(new DocumentStyle);
		style->parentStyleName = parentStyleName;
		style->styles = properties;
		document->styles.Add(styleName, style);
	}

	Nullable<WString> SummarizeName(GuiDocumentCommonInterface* textBox, vint begin, vint end)
	{
		return textBox->SummarizeStyleName(TextPos(0, begin), TextPos(0, end));
	}

	Nullable<WString> SummarizeName(GuiDocumentCommonInterface* textBox, vint row, vint begin, vint end)
	{
		return textBox->SummarizeStyleName(TextPos(row, begin), TextPos(row, end));
	}
}

TEST_FILE
{
	const auto resource_DocumentViewer = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentViewer Style Test" ClientSize="x:480 y:320">
        <DocumentViewer ref.Name="textBox" EditMode="Editable">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
        </DocumentViewer>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Styles")
	{
	});
}
