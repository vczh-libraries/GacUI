#include "TestControls.h"

using namespace gacui_unittest_template;

using GuiDocumentTextBox = GuiDocumentLabel;

template<typename TTextBox>
void RunTextBoxKeyTestCases(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>);

template<typename TTextBox>
void RunTextBoxKeyTestCases_Singleline(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>);

template<typename TTextBox>
void RunTextBoxKeyTestCases_Multiline(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>);

template<typename TTextBox>
void RunTextBoxKeyTestCases_MultiParagraph(const wchar_t* resource, const WString& controlName)
	requires(std::is_base_of_v<GuiControl, TTextBox>&& std::is_base_of_v<GuiDocumentCommonInterface, TTextBox>);

TEST_FILE
{
	const auto resource_SinglelineTextBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiSinglelineTextBox Key Test" ClientSize="x:480 y:320">
        <SinglelineTextBox ref.Name="textBox">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </SinglelineTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resource_MultilineTextBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiMultilineTextBox Test" ClientSize="x:480 y:320">
        <MultilineTextBox ref.Name="textBox">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
        </MultilineTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resource_DocumentTextBox = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentTextBox Test" ClientSize="x:480 y:320">
        <DocumentTextBox ref.Name="textBox" EditMode="Editable">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
        </DocumentTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resource_DocumentLabel = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentLabel Test" ClientSize="x:480 y:320">
        <DocumentLabel ref.Name="textBox" EditMode="Editable">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
        </DocumentLabel>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resource_DocumentViewer = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentViewer Test" ClientSize="x:480 y:320">
        <DocumentViewer ref.Name="textBox" EditMode="Editable">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
        </DocumentViewer>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		RunTextBoxKeyTestCases<GuiSinglelineTextBox>(resource_SinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));
		RunTextBoxKeyTestCases_Singleline<GuiSinglelineTextBox>(resource_SinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));
	});

	TEST_CATEGORY(L"GuiMultilineTextBox")
	{
		RunTextBoxKeyTestCases<GuiMultilineTextBox>(resource_MultilineTextBox, WString::Unmanaged(L"GuiMultilineTextBox"));
		RunTextBoxKeyTestCases_Multiline<GuiMultilineTextBox>(resource_MultilineTextBox, WString::Unmanaged(L"GuiMultilineTextBox"));
	});

	TEST_CATEGORY(L"GuiDocumentTextBox")
	{
		RunTextBoxKeyTestCases<GuiDocumentTextBox>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
		RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentTextBox>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
	});

	TEST_CATEGORY(L"GuiDocumentLabel")
	{
		RunTextBoxKeyTestCases<GuiDocumentLabel>(resource_DocumentLabel, WString::Unmanaged(L"GuiDocumentLabel"));
		RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentLabel>(resource_DocumentLabel, WString::Unmanaged(L"GuiDocumentLabel"));
	});

	TEST_CATEGORY(L"GuiDocumentViewer")
	{
		RunTextBoxKeyTestCases<GuiDocumentViewer>(resource_DocumentViewer, WString::Unmanaged(L"GuiDocumentViewer"));
		RunTextBoxKeyTestCases_MultiParagraph<GuiDocumentViewer>(resource_DocumentViewer, WString::Unmanaged(L"GuiDocumentViewer"));
	});
}
