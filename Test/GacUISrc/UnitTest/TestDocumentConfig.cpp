#include "../../../Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::unittest;
using namespace vl::glr;
using namespace vl::presentation;
using namespace vl::presentation::controls;

TEST_FILE
{
	xml::Parser parser;
	parser.OnError.Add([](const ErrorArgs& args)
	{
		auto error = const_cast<ErrorArgs&>(args).ToParsingError();
		TEST_PRINT(L"XML parsing error encountered on row: " + itow(error.codeRange.start.row) + L", column: " + itow(error.codeRange.start.column) + L", nessage: " + error.message);
		TEST_ASSERT(false);
	});

	auto LoadDoc = [&](const WString& text)
	{
		auto xmlDoc = xml::XmlParseDocument(text, parser);
		auto resource = Ptr(new GuiResource);
		auto item = Ptr(new GuiResourceItem);
		item->SetContent(L"XML", xmlDoc);
		resource->AddItem(L"xml-in-memory", item);

		List<GuiResourceError> errors;
		auto model = DocumentModel::LoadFromXml(item, xmlDoc, Ptr(new GuiResourcePathResolver(resource, WString::Empty)), errors);
		for (auto&& error : errors)
		{
			TEST_PRINT(L"DocumentMode. parsing error encountered at row: " + itow(error.position.row) + L", column:" + itow(error.position.column) + L", message: " + error.message);
		}
		TEST_ASSERT(errors.Count() == 0 && model);
		return model;
	};

	auto SaveDoc = [](Ptr<DocumentModel> model)
	{
		auto xmlDoc = model->SaveToXml();
		return stream::GenerateToStream([&](stream::TextWriter& writer)
		{
			xml::XmlPrint(xmlDoc, writer);
		});
	};

	TEST_CASE(L"Test Simple DocumentModel Serialization")
	{
		auto inputXml1 = LR"XML(
<Doc>
  <Content>
    <p align="Center">
      <div style="Title">
        <b>Text in Title style.</b>
      </div>
    </p>
    <p>
      <nop>With another paragraph</nop>
    </p>
  </Content>
  <Styles>
    <Style name="#Default-Override">
      <face>Segoe UI</face>
      <size>14</size>
    </Style>
    <Style name="Title">
      <size>1.5x</size>
      <b>true</b>
      <color>#0080FF</color>
    </Style>
  </Styles>
</Doc>
)XML";

		auto inputXml2 = LR"XML(
<Doc>
  <Content>
    <p>
      Unlike usual XML, spaces between texts and tags are never ignored.
    </p>
  </Content>
</Doc>
)XML";

		auto inputXml3 = LR"XML(
<Doc>
  <Content>
    <p>
      <nop>This is a paragraph without style.</nop><br/>
      <nop>There is<sp/>for spaces</nop><br/>
      <nop>and multiple lines in one paragraph.</nop>
    </p>
  </Content>
</Doc>
)XML";

		auto inputXml4 = LR"XML(
<Doc>
  <Content>
    <p>
      <nop>There are styles like <b>bold</b>, <i>italic</i>, <u>underline</u> and <s>strikeline</s>.</nop><br/>
      <nop>These styles has a reverse version to for example <b>make <b->part of the text not bold</b-></b>.</nop><br/>
      <div style="Bold">It also works <b->inside a div</b->.</div>
    </p>
  </Content>
  <Styles>
    <Style name="Bold">
      <b>true</b>
    </Style>
  </Styles>
</Doc>
)XML";

		auto inputXml5 = LR"XML(
<Doc>
  <Content>
    <p>
      <object name="MyObject"/> can be inserted into a document.<br/>
      <nop>So as <img source="not://existing"/>.</nop>
    </p>
  </Content>
</Doc>
)XML";
		const wchar_t* inputXmls[] = { inputXml1, inputXml2, inputXml3, inputXml4, inputXml5 };
		const wchar_t* expectedXmls[] = {
			LR"XML(<Doc><Content><p align="Center"><div style="Title"><b><nop>Text in Title style.</nop></b></div></p><p><nop>With another paragraph</nop></p></Content><Styles><Style name="#Default-Override"><face>Segoe UI</face><size>14</size></Style><Style name="Title"><size>1.5x</size><color>#0080FF</color><b>true</b></Style></Styles></Doc>)XML",
			LR"XML(<Doc><Content><p><br/>      Unlike usual XML, spaces between texts and tags are never ignored.<br/>   <sp/></p></Content><Styles/></Doc>)XML",
			LR"XML(<Doc><Content><p><nop>This is a paragraph without style.</nop><br/><nop>There is</nop><sp/><nop>for spaces</nop><br/><nop>and multiple lines in one paragraph.</nop></p></Content><Styles/></Doc>)XML",
			LR"XML(<Doc><Content><p><nop>There are styles like </nop><b><nop>bold</nop></b><nop>, </nop><i><nop>italic</nop></i><nop>, </nop><u><nop>underline</nop></u><sp/>and<sp/><s><nop>strikeline</nop></s><nop>.</nop><br/><nop>These styles has a reverse version to for example </nop><b><nop>make </nop><b-><nop>part of the text not bold</nop></b-></b><nop>.</nop><br/><div style="Bold"><nop>It also works </nop><b-><nop>inside a div</nop></b-><nop>.</nop></div></p></Content><Styles><Style name="Bold"><b>true</b></Style></Styles></Doc>)XML",
			LR"XML(<Doc><Content><p><object name="MyObject"/><nop> can be inserted into a document.</nop><br/><nop>So as </nop><img width="0" height="0" baseline="-1" frameIndex="0" source="not://existing"/><nop>.</nop></p></Content><Styles/></Doc>)XML",
		};

		// TODO: Fix in Vlpp where indexed(FromArray(...)) crashes because the enumerable is destructed too early
		auto xs = FromArray(inputXmls);
		for (auto [inputXml, index] : indexed(xs))
		{
			TEST_PRINT(L"Testing document: " + itow(index + 1));
			auto expectedXml = expectedXmls[index];
			auto model = LoadDoc(inputXml);
			auto actualXml = SaveDoc(model);

			if (expectedXml != actualXml)
			{
				TEST_PRINT(WString::Unmanaged(L"Expected: ") + expectedXml);
				TEST_PRINT(L"Actual: " + actualXml);
			}
			TEST_ASSERT(expectedXml == actualXml);
		}
	});

	TEST_CATEGORY(L"UserInput_ConvertToPlainText")
	{
		// Test Case 1: Basic formatting removal with alignment reset
		auto inputXml1 = LR"XML(
<Doc>
  <Content>
    <p align="Center">
      <div style="Title">
        <b><i><u>Formatted text content</u></i></b>
      </div>
    </p>
    <p>
      <nop>Plain paragraph</nop>
    </p>
  </Content>
  <Styles>
    <Style name="Title">
      <size>1.5x</size>
      <color>#0080FF</color>
    </Style>
  </Styles>
</Doc>
)XML";

		// Test Case 2: Multi-paragraph range conversion (convert middle paragraph only)
		auto inputXml2 = LR"XML(
<Doc>
  <Content>
    <p align="Right">
      <b>First paragraph with formatting</b>
    </p>
    <p align="Center">
      <div style="Title">
        <i>Middle paragraph to convert</i>
      </div>
    </p>
    <p align="Left">
      <u>Last paragraph with formatting</u>
    </p>
  </Content>
  <Styles>
    <Style name="Title">
      <size>1.5x</size>
      <color>#0080FF</color>
    </Style>
  </Styles>
</Doc>
)XML";

		// Test Case 3: Images and embedded objects removal
		auto inputXml3 = LR"XML(
<Doc>
  <Content>
    <p align="Center">
      <nop>Text before </nop><img source="test.png"/><nop> and after image</nop><br/>
      <object name="TestObject"/><nop> with embedded object</nop>
    </p>
  </Content>
</Doc>
)XML";

		// Test Case 4: Invalid range (beginParagraph > endParagraph) - no changes
		auto inputXml4 = LR"XML(
<Doc>
  <Content>
    <p align="Center">
      <b>This should remain unchanged</b>
    </p>
    <p align="Right">
      <i>This should also remain unchanged</i>
    </p>
  </Content>
</Doc>
)XML";

		// Test Case 5: Single paragraph document
		auto inputXml5 = LR"XML(
<Doc>
  <Content>
    <p align="Left">
      <div style="Complex">
        <b><i>Single paragraph with complex formatting</i></b>
      </div>
    </p>
  </Content>
  <Styles>
    <Style name="Complex">
      <size>2x</size>
      <color>#FF0000</color>
    </Style>
  </Styles>
</Doc>
)XML";

		// Expected outputs after conversion
		const wchar_t* inputXmls[] = { inputXml1, inputXml2, inputXml3, inputXml4, inputXml5 };
		const wchar_t* expectedXmls[] = {
			// Case 1: Both paragraphs converted, alignment reset
			LR"XML(<Doc><Content><p><nop>Formatted text content</nop></p><p><nop>Plain paragraph</nop></p></Content><Styles><Style name="Title"><size>1.5x</size><color>#0080FF</color></Style></Styles></Doc>)XML",
			
			// Case 2: Only middle paragraph converted
			LR"XML(<Doc><Content><p align="Right"><b><nop>First paragraph with formatting</nop></b></p><p><nop>Middle paragraph to convert</nop></p><p align="Left"><u><nop>Last paragraph with formatting</nop></u></p></Content><Styles><Style name="Title"><size>1.5x</size><color>#0080FF</color></Style></Styles></Doc>)XML",
			
			// Case 3: Images and objects removed, alignment reset
			LR"XML(<Doc><Content><p><nop>Text before  and after image<br/> with embedded object</nop></p></Content><Styles/></Doc>)XML",
			
			// Case 4: No changes due to invalid range
			LR"XML(<Doc><Content><p align="Center"><b><nop>This should remain unchanged</nop></b></p><p align="Right"><i><nop>This should also remain unchanged</nop></i></p></Content><Styles/></Doc>)XML",
			
			// Case 5: Single paragraph converted
			LR"XML(<Doc><Content><p><nop>Single paragraph with complex formatting</nop></p></Content><Styles><Style name="Complex"><size>2x</size><color>#FF0000</color></Style></Styles></Doc>)XML"
		};

		// Test parameters: beginParagraph, endParagraph pairs
		struct TestParams { vint begin, end; };
		const TestParams testParams[] = {
			{0, 1},  // Case 1: Convert all paragraphs
			{1, 1},  // Case 2: Convert only middle paragraph
			{0, 0},  // Case 3: Convert single paragraph
			{1, 0},  // Case 4: Invalid range (begin > end)
			{0, 0}   // Case 5: Convert single paragraph document
		};

		// Execute test cases with individual TEST_CASE for each scenario
		auto xs = FromArray(inputXmls);
		for (auto [inputXml, index] : indexed(xs))
		{
			TEST_CASE(L"Case " + itow(index + 1))
			{
				auto expectedXml = expectedXmls[index];
				auto params = testParams[index];
				
				auto model = LoadDoc(inputXml);
				GuiDocumentCommonInterface::UserInput_ConvertToPlainText(model, params.begin, params.end);
				auto actualXml = SaveDoc(model);
				
				if (expectedXml != actualXml)
				{
					TEST_PRINT(WString::Unmanaged(L"Expected: ") + expectedXml);
					TEST_PRINT(L"Actual: " + actualXml);
				}
				TEST_ASSERT(expectedXml == actualXml);
			});
		}
	});

	TEST_CATEGORY(L"UserInput_JoinParagraphs_ListWString")
	{
		TEST_CASE(L"Empty List")
		{
			List<WString> paragraphs;
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, false);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"");
		});

		TEST_CASE(L"Single Paragraph Without Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"Single paragraph content");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, false);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"Single paragraph content");
		});

		TEST_CASE(L"Single Paragraph With Spaces Flag")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"Single paragraph content");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, true);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"Single paragraph content");
		});

		TEST_CASE(L"Two Paragraphs Without Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"First");
			paragraphs.Add(L"Second");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, false);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"FirstSecond");
		});

		TEST_CASE(L"Two Paragraphs With Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"First");
			paragraphs.Add(L"Second");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, true);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"First Second");
		});

		TEST_CASE(L"Multiple Paragraphs Without Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"First");
			paragraphs.Add(L"Second");
			paragraphs.Add(L"Third");
			paragraphs.Add(L"Fourth");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, false);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"FirstSecondThirdFourth");
		});

		TEST_CASE(L"Multiple Paragraphs With Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"First");
			paragraphs.Add(L"Second");
			paragraphs.Add(L"Third");
			paragraphs.Add(L"Fourth");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, true);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"First Second Third Fourth");
		});

		TEST_CASE(L"Empty String Paragraphs Without Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"First");
			paragraphs.Add(L"");
			paragraphs.Add(L"Third");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, false);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"FirstThird");
		});

		TEST_CASE(L"Empty String Paragraphs With Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"First");
			paragraphs.Add(L"");
			paragraphs.Add(L"Third");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, true);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"First  Third");
		});

		TEST_CASE(L"All Empty Strings Without Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"");
			paragraphs.Add(L"");
			paragraphs.Add(L"");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, false);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"");
		});

		TEST_CASE(L"All Empty Strings With Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"");
			paragraphs.Add(L"");
			paragraphs.Add(L"");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, true);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"  ");
		});

		TEST_CASE(L"Mixed Content Without Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"Hello");
			paragraphs.Add(L"");
			paragraphs.Add(L"World");
			paragraphs.Add(L"!");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, false);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"HelloWorld!");
		});

		TEST_CASE(L"Mixed Content With Spaces")
		{
			List<WString> paragraphs;
			paragraphs.Add(L"Hello");
			paragraphs.Add(L"");
			paragraphs.Add(L"World");
			paragraphs.Add(L"!");
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(paragraphs, true);
			TEST_ASSERT(paragraphs.Count() == 1);
			TEST_ASSERT(paragraphs[0] == L"Hello  World !");
		});
	});

	TEST_CATEGORY(L"UserInput_JoinParagraphs_DocumentModel")
	{
		TEST_CASE(L"Single Paragraph Should Remain Unchanged")
		{
			auto inputXml = LR"XML(<Doc><Content><p><nop>Only one paragraph</nop></p></Content></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><nop>Only one paragraph</nop></p></Content><Styles/></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, false);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});

		TEST_CASE(L"Two Simple Paragraphs Without Spaces")
		{
			auto inputXml = LR"XML(<Doc><Content><p><nop>First paragraph</nop></p><p><nop>Second paragraph</nop></p></Content></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><nop>First paragraph</nop><nop>Second paragraph</nop></p></Content><Styles/></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, false);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});

		TEST_CASE(L"Two Simple Paragraphs With Spaces")
		{
			auto inputXml = LR"XML(<Doc><Content><p><nop>First paragraph</nop></p><p><nop>Second paragraph</nop></p></Content></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><nop>First paragraph</nop><sp/><nop>Second paragraph</nop></p></Content><Styles/></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, true);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});

		TEST_CASE(L"Multiple Paragraphs Without Spaces")
		{
			auto inputXml = LR"XML(<Doc><Content><p><nop>First</nop></p><p><nop>Second</nop></p><p><nop>Third</nop></p></Content></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><nop>First</nop><nop>Second</nop><nop>Third</nop></p></Content><Styles/></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, false);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});

		TEST_CASE(L"Multiple Paragraphs With Spaces")
		{
			auto inputXml = LR"XML(<Doc><Content><p><nop>First</nop></p><p><nop>Second</nop></p><p><nop>Third</nop></p></Content></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><nop>First</nop><sp/><nop>Second</nop><sp/><nop>Third</nop></p></Content><Styles/></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, true);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});

		TEST_CASE(L"Formatted Text Without Spaces")
		{
			auto inputXml = LR"XML(<Doc><Content><p><b><nop>Bold text</nop></b></p><p><i><nop>Italic text</nop></i></p></Content></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><b><nop>Bold text</nop></b><i><nop>Italic text</nop></i></p></Content><Styles/></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, false);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});

		TEST_CASE(L"Formatted Text With Spaces")
		{
			auto inputXml = LR"XML(<Doc><Content><p><b><nop>Bold text</nop></b></p><p><i><nop>Italic text</nop></i></p></Content></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><b><nop>Bold text</nop></b><sp/><i><nop>Italic text</nop></i></p></Content><Styles/></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, true);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});

		TEST_CASE(L"Empty Paragraphs Mixed With Content")
		{
			auto inputXml = LR"XML(<Doc><Content><p><nop>First</nop></p><p></p><p><nop>Third</nop></p></Content></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><nop>First</nop><sp/><sp/><nop>Third</nop></p></Content><Styles/></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, true);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});

		TEST_CASE(L"Complex Nested Formatting With Styles")
		{
			auto inputXml = LR"XML(<Doc><Content><p><div style="Title"><b><nop>Styled title</nop></b></div></p><p><nop>Regular paragraph</nop></p></Content><Styles><Style name="Title"><size>1.5x</size><b>true</b></Style></Styles></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><div style="Title"><b><nop>Styled title</nop></b></div><sp/><nop>Regular paragraph</nop></p></Content><Styles><Style name="Title"><size>1.5x</size><b>true</b></Style></Styles></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, true);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});

		TEST_CASE(L"Mixed Content Types With Images")
		{
			auto inputXml = LR"XML(<Doc><Content><p><nop>Text content</nop></p><p><img width="16" height="16" baseline="-1" frameIndex="0" source="test.png"/></p></Content></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><nop>Text content</nop><sp/><img width="16" height="16" baseline="-1" frameIndex="0" source="test.png"/></p></Content><Styles/></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, true);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});

		TEST_CASE(L"Embedded Objects")
		{
			auto inputXml = LR"XML(<Doc><Content><p><nop>Before object</nop></p><p><object name="TestObject"/></p><p><nop>After object</nop></p></Content></Doc>)XML";
			auto expectedXml = LR"XML(<Doc><Content><p><nop>Before object</nop><sp/><object name="TestObject"/><sp/><nop>After object</nop></p></Content><Styles/></Doc>)XML";
			
			auto model = LoadDoc(inputXml);
			GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, true);
			auto actualXml = SaveDoc(model);
			
			TEST_PRINT(L"Expected: " + WString::Unmanaged(expectedXml));
			TEST_PRINT(L"Actual: " + actualXml);
			TEST_ASSERT(expectedXml == actualXml);
		});
	});

	TEST_CATEGORY(L"UserInput_JoinLinesInsideParagraph_WString")
	{
		// Tests for string line joining within paragraphs
		// Will be implemented in subsequent task
	});

	TEST_CATEGORY(L"UserInput_JoinLinesInsideParagraph_DocumentParagraphRun")
	{
		// Tests for DocumentParagraphRun line joining
		// Will be implemented in subsequent task
	});

	TEST_CATEGORY(L"UserInput_FormatText_ListWString")
	{
		// Tests for string list formatting
		// Will be implemented in subsequent task
	});

	TEST_CATEGORY(L"UserInput_FormatText_WStringToList")
	{
		// Tests for string to list formatting conversion
		// Will be implemented in subsequent task
	});

	TEST_CATEGORY(L"UserInput_FormatDocument")
	{
		// Tests for comprehensive document formatting
		// Will be implemented in subsequent task
	});
}