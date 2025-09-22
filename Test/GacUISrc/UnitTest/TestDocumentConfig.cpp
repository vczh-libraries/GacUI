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
		struct TestCase
		{
			const wchar_t* inputXml;
			const wchar_t* expectedXml;
			bool spaceForFlattenedLineBreak;
		};

		TestCase testCases[] = {
			{
				LR"XML(<Doc><Content><p><nop>Only one paragraph</nop></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Only one paragraph</nop></p></Content><Styles/></Doc>)XML",
				false
			},
			{
				LR"XML(<Doc><Content><p><nop>First paragraph</nop></p><p><nop>Second paragraph</nop></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>First paragraph</nop><nop>Second paragraph</nop></p></Content><Styles/></Doc>)XML",
				false
			},
			{
				LR"XML(<Doc><Content><p><nop>First paragraph</nop></p><p><nop>Second paragraph</nop></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>First paragraph</nop><sp/><nop>Second paragraph</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				LR"XML(<Doc><Content><p><nop>First</nop></p><p><nop>Second</nop></p><p><nop>Third</nop></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>First</nop><nop>Second</nop><nop>Third</nop></p></Content><Styles/></Doc>)XML",
				false
			},
			{
				LR"XML(<Doc><Content><p><nop>First</nop></p><p><nop>Second</nop></p><p><nop>Third</nop></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>First</nop><sp/><nop>Second</nop><sp/><nop>Third</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				LR"XML(<Doc><Content><p><b><nop>Bold text</nop></b></p><p><i><nop>Italic text</nop></i></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><b><nop>Bold text</nop></b><i><nop>Italic text</nop></i></p></Content><Styles/></Doc>)XML",
				false
			},
			{
				LR"XML(<Doc><Content><p><b><nop>Bold text</nop></b></p><p><i><nop>Italic text</nop></i></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><b><nop>Bold text</nop></b><sp/><i><nop>Italic text</nop></i></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				LR"XML(<Doc><Content><p><nop>First</nop></p><p></p><p><nop>Third</nop></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>First</nop><sp/><sp/><nop>Third</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				LR"XML(<Doc><Content><p><div style="Title"><b><nop>Styled title</nop></b></div></p><p><nop>Regular paragraph</nop></p></Content><Styles><Style name="Title"><size>1.5x</size><b>true</b></Style></Styles></Doc>)XML",
				LR"XML(<Doc><Content><p><div style="Title"><b><nop>Styled title</nop></b></div><sp/><nop>Regular paragraph</nop></p></Content><Styles><Style name="Title"><size>1.5x</size><b>true</b></Style></Styles></Doc>)XML",
				true
			},
			{
				LR"XML(<Doc><Content><p><nop>Text content</nop></p><p><img width="16" height="16" baseline="-1" frameIndex="0" source="test.png"/></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Text content</nop><sp/><img width="16" height="16" baseline="-1" frameIndex="0" source="test.png"/></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				LR"XML(<Doc><Content><p><nop>Before object</nop></p><p><object name="TestObject"/></p><p><nop>After object</nop></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Before object</nop><sp/><object name="TestObject"/><sp/><nop>After object</nop></p></Content><Styles/></Doc>)XML",
				true
			}
		};

		auto xs = FromArray(testCases);
		for (auto [testCase, index] : indexed(xs))
		{
			TEST_CASE(L"Case " + itow(index + 1))
			{
				auto model = LoadDoc(testCase.inputXml);
				GuiDocumentCommonInterface::UserInput_JoinParagraphs(model, testCase.spaceForFlattenedLineBreak);
				auto actualXml = SaveDoc(model);
				
				if (testCase.expectedXml != actualXml)
				{
					TEST_PRINT(WString::Unmanaged(L"Expected: ") + testCase.expectedXml);
					TEST_PRINT(L"Actual: " + actualXml);
				}
				TEST_ASSERT(testCase.expectedXml == actualXml);
			});
		}
	});

	TEST_CATEGORY(L"UserInput_JoinLinesInsideParagraph_WString")
	{
		struct TestCase
		{
			const wchar_t* name;
			const wchar_t* input;
			const wchar_t* expected;
			bool spaceForFlattenedLineBreak;
		};

		TestCase testCases[] = {
			{
				L"SingleLine NoLineBreaks with space",
				L"Hello World",
				L"Hello World",
				true
			},
			{
				L"SingleLine NoLineBreaks without space",
				L"Hello World",
				L"Hello World",
				false
			},
			{
				L"EmptyString with space",
				L"",
				L"",
				true
			},
			{
				L"EmptyString without space",
				L"",
				L"",
				false
			},
			{
				L"SingleLineBreak with space",
				L"\r\n",
				L" ",
				true
			},
			{
				L"SingleLineBreak without space",
				L"\r\n",
				L"",
				false
			},
			{
				L"SingleCharacter with space",
				L"X",
				L"X",
				true
			},
			{
				L"SingleCharacter without space",
				L"X",
				L"X",
				false
			},
			{
				L"TwoLines UnixFormat with space",
				L"First Line\nSecond Line",
				L"First Line Second Line",
				true
			},
			{
				L"TwoLines UnixFormat without space",
				L"First Line\nSecond Line",
				L"First LineSecond Line",
				false
			},
			{
				L"TwoLines WindowsFormat with space",
				L"First Line\r\nSecond Line",
				L"First Line Second Line",
				true
			},
			{
				L"TwoLines WindowsFormat without space",
				L"First Line\r\nSecond Line",
				L"First LineSecond Line",
				false
			},
			{
				L"TwoLines MultipleCarriageReturns with space",
				L"First Line\r\r\nSecond Line",
				L"First Line Second Line",
				true
			},
			{
				L"TwoLines MultipleCarriageReturns without space",
				L"First Line\r\r\r\nSecond Line",
				L"First LineSecond Line",
				false
			},
			{
				L"ThreeLines MixedFormats with space",
				L"Line1\nLine2\r\nLine3",
				L"Line1 Line2 Line3",
				true
			},
			{
				L"ThreeLines MixedFormats without space",
				L"Line1\r\r\nLine2\nLine3",
				L"Line1Line2Line3",
				false
			},
			{
				L"StandaloneCarriageReturns NotLineBreaks with space",
				L"Text\rWith\rCarriage\rReturns",
				L"Text\rWith\rCarriage\rReturns",
				true
			},
			{
				L"StandaloneCarriageReturns NotLineBreaks without space",
				L"Text\rWith\rCarriage\rReturns",
				L"Text\rWith\rCarriage\rReturns",
				false
			},
			{
				L"MixedStandaloneAndValidLineBreaks with space",
				L"Line1\rNot\rBreak\nLine2\r\nLine3",
				L"Line1\rNot\rBreak Line2 Line3",
				true
			},
			{
				L"MixedStandaloneAndValidLineBreaks without space",
				L"Line1\rNot\rBreak\nLine2\r\nLine3",
				L"Line1\rNot\rBreakLine2Line3",
				false
			},
			{
				L"TrailingCarriageReturn with space",
				L"Text with trailing CR\r",
				L"Text with trailing CR",
				true
			},
			{
				L"TrailingCarriageReturn without space",
				L"Text with trailing CR\r",
				L"Text with trailing CR",
				false
			},
			{
				L"ConsecutiveLineBreaks with space",
				L"Line1\n\nLine3",
				L"Line1  Line3",
				true
			},
			{
				L"ConsecutiveLineBreaks without space",
				L"Line1\r\n\r\nLine3",
				L"Line1Line3",
				false
			},
			{
				L"OnlyLineBreaks with space",
				L"\n\r\n\n",
				L"   ",
				true
			},
			{
				L"OnlyLineBreaks without space",
				L"\n\r\n\n",
				L"",
				false
			},
			{
				L"TextEndingWithLineBreaks with space",
				L"Content\n\r\n",
				L"Content  ",
				true
			},
			{
				L"TextEndingWithLineBreaks without space",
				L"Content\n\r\n",
				L"Content",
				false
			},
			{
				L"TextStartingWithLineBreaks with space",
				L"\n\r\nContent",
				L"  Content",
				true
			},
			{
				L"TextStartingWithLineBreaks without space",
				L"\n\r\nContent",
				L"Content",
				false
			}
		};

		auto xs = FromArray(testCases);
		for (auto [testCase, index] : indexed(xs))
		{
			TEST_CASE(WString::Unmanaged(testCase.name))
			{
				WString text = testCase.input;
				GuiDocumentCommonInterface::UserInput_JoinLinesInsideParagraph(text, testCase.spaceForFlattenedLineBreak);
				WString expectedText = testCase.expected;
				if (text != expectedText)
				{
					TEST_PRINT(L"Expected: " + expectedText);
					TEST_PRINT(L"Actual: " + text);
				}
				TEST_ASSERT(text == expectedText);
			});
		}
	});

	TEST_CATEGORY(L"UserInput_JoinLinesInsideParagraph_DocumentParagraphRun")
	{
		struct DocumentParagraphRunTestCase
		{
			const wchar_t* name;           // Test case description
			const wchar_t* inputXml;       // Input XML with line breaks in text runs
			const wchar_t* expectedXml;    // Expected XML after line break processing
			bool spaceForFlattenedLineBreak; // Parameter value to pass to function
		};

		DocumentParagraphRunTestCase testCases[] = {
			// Simple structure tests
			{
				L"SingleTextRun WithLineBreak",
				LR"XML(<Doc><Content><p><nop>Line1<br/>Line2</nop></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Line1</nop><sp/><nop>Line2</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"MultipleTextRuns SameLevel",
				LR"XML(<Doc><Content><p><nop>First<br/>Part</nop><nop>Second<br/>Part</nop></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>First</nop><sp/><nop>Part</nop><nop>Second</nop><sp/><nop>Part</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"EmptyTextRuns MixedWithContent",
				LR"XML(<Doc><Content><p><nop></nop><nop>Content<br/>Here</nop><nop></nop></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Content</nop><sp/><nop>Here</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			
			// Single-level container tests
			{
				L"BoldContainer WithTextRun",
				LR"XML(<Doc><Content><p><b><nop>Bold<br/>Text</nop></b></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><b><nop>Bold</nop><sp/><nop>Text</nop></b></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"ItalicContainer WithTextRun",
				LR"XML(<Doc><Content><p><i><nop>Italic<br/>Text</nop></i></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><i><nop>Italic</nop><sp/><nop>Text</nop></i></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"StyledContainer WithTextRun",
				LR"XML(<Doc><Content><p><div style="Title"><nop>Styled<br/>Text</nop></div></p></Content><Styles><Style name="Title"/></Styles></Doc>)XML",
				LR"XML(<Doc><Content><p><div style="Title"><nop>Styled</nop><sp/><nop>Text</nop></div></p></Content><Styles><Style name="Title"/></Styles></Doc>)XML",
				true
			},
			{
				L"MixedDirectAndContainer",
				LR"XML(<Doc><Content><p><nop>Direct<br/>Text</nop><b><nop>Bold<br/>Text</nop></b></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Direct</nop><sp/><nop>Text</nop><b><nop>Bold</nop><sp/><nop>Text</nop></b></p></Content><Styles/></Doc>)XML",
				true
			},
			
			// Multi-level nested container tests
			{
				L"TwoLevelNesting BoldItalic",
				LR"XML(<Doc><Content><p><b><i><nop>Bold<br/>Italic</nop></i></b></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><b><i><nop>Bold</nop><sp/><nop>Italic</nop></i></b></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"ThreeLevelNesting DivBoldItalic",
				LR"XML(<Doc><Content><p><div><b><i><nop>Deep<br/>Nested</nop></i></b></div></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><b><i><nop>Deep</nop><sp/><nop>Nested</nop></i></b></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"ComplexNesting MultipleSiblings",
				LR"XML(<Doc><Content><p><div><b><nop>Bold<br/>Part</nop></b><i><nop>Italic<br/>Part</nop></i></div></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><b><nop>Bold</nop><sp/><nop>Part</nop></b><i><nop>Italic</nop><sp/><nop>Part</nop></i></p></Content><Styles/></Doc>)XML",
				true
			},
			
			// Multiple container siblings tests
			{
				L"AdjacentContainers BoldItalic",
				LR"XML(<Doc><Content><p><b><nop>Bold<br/>Text</nop></b><i><nop>Italic<br/>Text</nop></i></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><b><nop>Bold</nop><sp/><nop>Text</nop></b><i><nop>Italic</nop><sp/><nop>Text</nop></i></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"MixedWithDirectText StartMiddleEnd",
				LR"XML(<Doc><Content><p><nop>Start<br/>Text</nop><b><nop>Bold<br/>Middle</nop></b><nop>End<br/>Text</nop></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Start</nop><sp/><nop>Text</nop><b><nop>Bold</nop><sp/><nop>Middle</nop></b><nop>End</nop><sp/><nop>Text</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			
			// Complex document element tests
			{
				L"WithImageElement",
				LR"XML(<Doc><Content><p><nop>Before<br/>Image</nop><img width="16" height="16" source="test.png"/><nop>After<br/>Image</nop></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Before</nop><sp/><nop>Image</nop><img width="16" height="16" baseline="-1" frameIndex="0" source="test.png"/><nop>After</nop><sp/><nop>Image</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"WithObjectElement",
				LR"XML(<Doc><Content><p><nop>Before<br/>Object</nop><object name="TestObject"/><nop>After<br/>Object</nop></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Before</nop><sp/><nop>Object</nop><object name="TestObject"/><nop>After</nop><sp/><nop>Object</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"WithHyperlinkElement",
				LR"XML(<Doc><Content><p><a href="http://example.com"><nop>Link<br/>Text</nop></a></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><a href="http://example.com"><nop>Link</nop><sp/><nop>Text</nop></a></p></Content><Styles/></Doc>)XML",
				true
			},
			
			// Edge cases
			{
				L"EmptyContainers WithContent",
				LR"XML(<Doc><Content><p><b></b><nop>Content<br/>After</nop></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><b/><nop>Content</nop><sp/><nop>After</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"NestedEmptyContainers",
				LR"XML(<Doc><Content><p><div><b></b></div><nop>Text<br/>Here</nop></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><b/><nop>Text</nop><sp/><nop>Here</nop></p></Content><Styles/></Doc>)XML",
				true
			},
			{
				L"OnlyContainers NoText",
				LR"XML(<Doc><Content><p><b></b><i></i></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><b/><i/></p></Content><Styles/></Doc>)XML",
				true
			},
			
			// spaceForFlattenedLineBreak = false tests
			{
				L"SimpleText WithoutSpace",
				LR"XML(<Doc><Content><p><nop>Line1<br/>Line2</nop></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Line1</nop><nop>Line2</nop></p></Content><Styles/></Doc>)XML",
				false
			},
			{
				L"NestedContainers WithoutSpace",
				LR"XML(<Doc><Content><p><b><i><nop>Deep<br/>Text</nop></i></b></p></Content><Styles/></Doc>)XML",
				LR"XML(<Doc><Content><p><b><i><nop>Deep</nop><nop>Text</nop></i></b></p></Content><Styles/></Doc>)XML",
				false
			}
		};

		auto xs = FromArray(testCases);
		for (auto [testCase, index] : indexed(xs))
		{
			TEST_CASE(WString::Unmanaged(testCase.name))
			{
				// Load document from input XML
				auto inputModel = LoadDoc(testCase.inputXml);
				TEST_ASSERT(inputModel->paragraphs.Count() == 1);
				
				// Get the first paragraph for testing
				auto paragraph = inputModel->paragraphs[0];
				
				// Apply the function under test
				GuiDocumentCommonInterface::UserInput_JoinLinesInsideParagraph(paragraph, testCase.spaceForFlattenedLineBreak);
				
				// Save processed document to XML
				auto actualXml = SaveDoc(inputModel);
				
				// Compare results
				if (actualXml != testCase.expectedXml)
				{
					TEST_PRINT(WString::Unmanaged(L"Expected XML:") + testCase.expectedXml);
					TEST_PRINT(L"Actual XML:" + actualXml);
				}
				TEST_ASSERT(actualXml == testCase.expectedXml);
			});
		}
	});

	TEST_CATEGORY(L"UserInput_FormatText_ListWString")
	{
		GuiDocumentConfig defaultConfig;
		defaultConfig.autoExpand = false;
		defaultConfig.pasteAsPlainText = false;
		defaultConfig.wrapLine = true;
		defaultConfig.paragraphMode = GuiDocumentParagraphMode::Paragraph;
		defaultConfig.paragraphPadding = true;
		defaultConfig.doubleLineBreaksBetweenParagraph = true;
		defaultConfig.spaceForFlattenedLineBreak = false;
		defaultConfig.paragraphRecycle = true;

		TEST_CATEGORY(L"Paragraph Mode")
		{
			TEST_CASE(L"Basic Behavior - No Processing")
			{
				GuiDocumentConfig config = defaultConfig;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				config.spaceForFlattenedLineBreak = false;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphs;
				paragraphs.Add(L"First paragraph\r\nwith line break");
				paragraphs.Add(L"Second paragraph\r\nwith line break");
				paragraphs.Add(L"Third paragraph");

				GuiDocumentCommonInterface::UserInput_FormatText(paragraphs, evalConfig);
				
				// Paragraph mode should leave everything unchanged
				TEST_ASSERT(paragraphs.Count() == 3);
				TEST_ASSERT(paragraphs[0] == L"First paragraph\r\nwith line break");
				TEST_ASSERT(paragraphs[1] == L"Second paragraph\r\nwith line break");
				TEST_ASSERT(paragraphs[2] == L"Third paragraph");
			});

			TEST_CASE(L"With spaceForFlattenedLineBreak True")
			{
				GuiDocumentConfig config = defaultConfig;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphs;
				paragraphs.Add(L"First paragraph\r\nwith line break");
				paragraphs.Add(L"Second paragraph\r\nwith line break");

				GuiDocumentCommonInterface::UserInput_FormatText(paragraphs, evalConfig);
				
				// Paragraph mode should still leave everything unchanged regardless of spaceForFlattenedLineBreak
				TEST_ASSERT(paragraphs.Count() == 2);
				TEST_ASSERT(paragraphs[0] == L"First paragraph\r\nwith line break");
				TEST_ASSERT(paragraphs[1] == L"Second paragraph\r\nwith line break");
			});
		});

		TEST_CATEGORY(L"Multiline Mode")
		{
			TEST_CASE(L"Basic Behavior - Line Joining Only")
			{
				GuiDocumentConfig config = defaultConfig;
				config.paragraphMode = GuiDocumentParagraphMode::Multiline;
				config.spaceForFlattenedLineBreak = false;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphs;
				paragraphs.Add(L"First paragraph\r\nwith line break");
				paragraphs.Add(L"Second paragraph\r\nwith line break");
				paragraphs.Add(L"Third paragraph");
				
				GuiDocumentCommonInterface::UserInput_FormatText(paragraphs, evalConfig);
				
				// Multiline mode should join lines within each paragraph but keep paragraphs separate
				TEST_ASSERT(paragraphs.Count() == 3);
				TEST_ASSERT(paragraphs[0] == L"First paragraphwith line break");
				TEST_ASSERT(paragraphs[1] == L"Second paragraphwith line break");
				TEST_ASSERT(paragraphs[2] == L"Third paragraph");
			});

			TEST_CASE(L"With Spaces - spaceForFlattenedLineBreak True")
			{
				GuiDocumentConfig config = defaultConfig;
				config.paragraphMode = GuiDocumentParagraphMode::Multiline;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphs;
				paragraphs.Add(L"First\r\nSecond");
				paragraphs.Add(L"Third\r\nFourth");
				
				GuiDocumentCommonInterface::UserInput_FormatText(paragraphs, evalConfig);
				
				// Should add spaces when joining lines within paragraphs
				TEST_ASSERT(paragraphs.Count() == 2);
				TEST_ASSERT(paragraphs[0] == L"First Second");
				TEST_ASSERT(paragraphs[1] == L"Third Fourth");
			});
		});

		TEST_CATEGORY(L"Singleline Mode")
		{
			TEST_CASE(L"Basic Behavior - Line and Paragraph Joining")
			{
				GuiDocumentConfig config = defaultConfig;
				config.paragraphMode = GuiDocumentParagraphMode::Singleline;
				config.spaceForFlattenedLineBreak = false;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphs;
				paragraphs.Add(L"First paragraph\r\nwith line break");
				paragraphs.Add(L"Second paragraph\r\nwith line break");
				paragraphs.Add(L"Third paragraph");
				
				GuiDocumentCommonInterface::UserInput_FormatText(paragraphs, evalConfig);
				
				// Singleline mode should join lines AND join all paragraphs into one
				TEST_ASSERT(paragraphs.Count() == 1);
				TEST_ASSERT(paragraphs[0] == L"First paragraphwith line breakSecond paragraphwith line breakThird paragraph");
			});

			TEST_CASE(L"With Spaces - spaceForFlattenedLineBreak True")
			{
				GuiDocumentConfig config = defaultConfig;
				config.paragraphMode = GuiDocumentParagraphMode::Singleline;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphs;
				paragraphs.Add(L"First\r\nSecond");
				paragraphs.Add(L"Third\r\nFourth");
				
				GuiDocumentCommonInterface::UserInput_FormatText(paragraphs, evalConfig);
				
				// Should add spaces when joining lines AND when joining paragraphs
				TEST_ASSERT(paragraphs.Count() == 1);
				TEST_ASSERT(paragraphs[0] == L"First Second Third Fourth");
			});
		});

		TEST_CATEGORY(L"Edge Cases")
		{
			TEST_CASE(L"Empty List - All Modes")
			{
				// Test Paragraph mode
				GuiDocumentConfig config1 = defaultConfig;
				config1.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig1(config1);
				collections::List<WString> emptyList1;
				GuiDocumentCommonInterface::UserInput_FormatText(emptyList1, evalConfig1);
				TEST_ASSERT(emptyList1.Count() == 0);
				
				// Test Multiline mode
				GuiDocumentConfig config2 = defaultConfig;
				config2.paragraphMode = GuiDocumentParagraphMode::Multiline;
				GuiDocumentConfigEvaluated evalConfig2(config2);
				collections::List<WString> emptyList2;
				GuiDocumentCommonInterface::UserInput_FormatText(emptyList2, evalConfig2);
				TEST_ASSERT(emptyList2.Count() == 0);
				
				// Test Singleline mode - UserInput_JoinParagraphs creates one empty string from empty list
				GuiDocumentConfig config3 = defaultConfig;
				config3.paragraphMode = GuiDocumentParagraphMode::Singleline;
				GuiDocumentConfigEvaluated evalConfig3(config3);
				collections::List<WString> emptyList3;
				GuiDocumentCommonInterface::UserInput_FormatText(emptyList3, evalConfig3);
				TEST_ASSERT(emptyList3.Count() == 1);
				TEST_ASSERT(emptyList3[0] == L"");
			});

			TEST_CASE(L"Single Paragraph Structure")
			{
				GuiDocumentConfig config = defaultConfig;
				config.paragraphMode = GuiDocumentParagraphMode::Singleline;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphs;
				paragraphs.Add(L"Only one\r\nparagraph");
				
				GuiDocumentCommonInterface::UserInput_FormatText(paragraphs, evalConfig);
				
				// Should still result in one paragraph with joined lines
				TEST_ASSERT(paragraphs.Count() == 1);
				TEST_ASSERT(paragraphs[0] == L"Only one paragraph");
			});

			TEST_CASE(L"Mixed Paragraph Content")
			{
				GuiDocumentConfig config = defaultConfig;
				config.paragraphMode = GuiDocumentParagraphMode::Singleline;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphs;
				paragraphs.Add(L"First");
				paragraphs.Add(L"");
				paragraphs.Add(L"Third\r\nWith line break");
				paragraphs.Add(L"   ");
				
				GuiDocumentCommonInterface::UserInput_FormatText(paragraphs, evalConfig);
				
				// Should join all paragraphs including empty and whitespace ones
				TEST_ASSERT(paragraphs.Count() == 1);
				TEST_ASSERT(paragraphs[0] == L"First  Third With line break    ");
			});
		});

		TEST_CATEGORY(L"Paragraph Structure Interaction")
		{
			TEST_CASE(L"Multiple Paragraphs with Multiline Mode")
			{
				GuiDocumentConfig config = defaultConfig;
				config.paragraphMode = GuiDocumentParagraphMode::Multiline;
				config.spaceForFlattenedLineBreak = false;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphs;
				paragraphs.Add(L"Para1\r\nLine2");
				paragraphs.Add(L"Para2\r\nLine2");
				paragraphs.Add(L"Para3\r\nLine2");
				paragraphs.Add(L"Para4");
				paragraphs.Add(L"Para5\r\nLine2\r\nLine3");
				
				GuiDocumentCommonInterface::UserInput_FormatText(paragraphs, evalConfig);
				
				// Should join lines within each paragraph but preserve paragraph structure
				TEST_ASSERT(paragraphs.Count() == 5);
				TEST_ASSERT(paragraphs[0] == L"Para1Line2");
				TEST_ASSERT(paragraphs[1] == L"Para2Line2");
				TEST_ASSERT(paragraphs[2] == L"Para3Line2");
				TEST_ASSERT(paragraphs[3] == L"Para4");
				TEST_ASSERT(paragraphs[4] == L"Para5Line2Line3");
			});

			TEST_CASE(L"Sequential Processing Order Verification")
			{
				GuiDocumentConfig config = defaultConfig;
				config.paragraphMode = GuiDocumentParagraphMode::Singleline;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphs;
				paragraphs.Add(L"Para1\r\nLine2");
				paragraphs.Add(L"Para2\r\nLine2");
				paragraphs.Add(L"Para3");
				
				// Create a copy for manual verification of sequential processing
				collections::List<WString> manualProcess;
				CopyFrom(manualProcess, paragraphs);
				
				// Manual step 1: Apply line joining to each paragraph
				for (vint i = 0; i < manualProcess.Count(); i++)
				{
					GuiDocumentCommonInterface::UserInput_JoinLinesInsideParagraph(manualProcess[i], true);
				}
				
				// Manual step 2: Apply paragraph joining
				GuiDocumentCommonInterface::UserInput_JoinParagraphs(manualProcess, true);
				
				// Test the actual function
				GuiDocumentCommonInterface::UserInput_FormatText(paragraphs, evalConfig);
				
				// Should match manual sequential processing
				TEST_ASSERT(paragraphs.Count() == 1);
				TEST_ASSERT(manualProcess.Count() == 1);
				TEST_ASSERT(paragraphs[0] == manualProcess[0]);
				TEST_ASSERT(paragraphs[0] == L"Para1 Line2 Para2 Line2 Para3");
			});
		});
	});

	TEST_CATEGORY(L"UserInput_FormatText_WStringToList (Single Line Breaking Mode)")
	{
		GuiDocumentConfig defaultConfig;
		defaultConfig.autoExpand = false;
		defaultConfig.pasteAsPlainText = false;
		defaultConfig.wrapLine = true;
		defaultConfig.paragraphMode = GuiDocumentParagraphMode::Paragraph;
		defaultConfig.paragraphPadding = true;
		defaultConfig.doubleLineBreaksBetweenParagraph = true;
		defaultConfig.spaceForFlattenedLineBreak = false;
		defaultConfig.paragraphRecycle = true;

		TEST_CATEGORY(L"Basic Line Breaking Tests")
		{
			TEST_CASE(L"Single Line Without Line Breaks")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Single line of text";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 1);
				TEST_ASSERT(paragraphTexts[0] == L"Single line of text");
			});

			TEST_CASE(L"Windows Line Endings (\\r\\n)")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line 1\r\nLine 2\r\nLine 3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 3);
				TEST_ASSERT(paragraphTexts[0] == L"Line 1");
				TEST_ASSERT(paragraphTexts[1] == L"Line 2");
				TEST_ASSERT(paragraphTexts[2] == L"Line 3");
			});

			TEST_CASE(L"Unix Line Endings (\\n)")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line 1\nLine 2\nLine 3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 3);
				TEST_ASSERT(paragraphTexts[0] == L"Line 1");
				TEST_ASSERT(paragraphTexts[1] == L"Line 2");
				TEST_ASSERT(paragraphTexts[2] == L"Line 3");
			});

			TEST_CASE(L"Mixed Line Endings")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line 1\r\nLine 2\nLine 3\nLine 4";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 4);
				TEST_ASSERT(paragraphTexts[0] == L"Line 1");
				TEST_ASSERT(paragraphTexts[1] == L"Line 2");
				TEST_ASSERT(paragraphTexts[2] == L"Line 3");
				TEST_ASSERT(paragraphTexts[3] == L"Line 4");
			});
		});

		TEST_CATEGORY(L"Edge Case Tests")
		{
			TEST_CASE(L"Empty Input")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 0);
			});

			TEST_CASE(L"Only Line Breaks")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"\n\n\n";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 4);
				TEST_ASSERT(paragraphTexts[0] == L"");
				TEST_ASSERT(paragraphTexts[1] == L"");
				TEST_ASSERT(paragraphTexts[2] == L"");
				TEST_ASSERT(paragraphTexts[3] == L"");
			});

			TEST_CASE(L"Text Ending With Line Break")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line 1\nLine 2\n";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 3);
				TEST_ASSERT(paragraphTexts[0] == L"Line 1");
				TEST_ASSERT(paragraphTexts[1] == L"Line 2");
				TEST_ASSERT(paragraphTexts[2] == L"");
			});

			TEST_CASE(L"Consecutive Line Breaks")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line 1\n\nLine 2\n\n\nLine 3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 6);
				TEST_ASSERT(paragraphTexts[0] == L"Line 1");
				TEST_ASSERT(paragraphTexts[1] == L"");
				TEST_ASSERT(paragraphTexts[2] == L"Line 2");
				TEST_ASSERT(paragraphTexts[3] == L"");
				TEST_ASSERT(paragraphTexts[4] == L"");
				TEST_ASSERT(paragraphTexts[5] == L"Line 3");
			});
		});

		TEST_CATEGORY(L"Paragraph Mode Tests")
		{
			TEST_CASE(L"Paragraph Mode - No Post-Processing")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line 1\nLine 2\nLine 3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 3);
				TEST_ASSERT(paragraphTexts[0] == L"Line 1");
				TEST_ASSERT(paragraphTexts[1] == L"Line 2");
				TEST_ASSERT(paragraphTexts[2] == L"Line 3");
			});

			TEST_CASE(L"Multiline Mode - With Line Joining")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Multiline;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line 1\nLine 2\nLine 3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 3);
				TEST_ASSERT(paragraphTexts[0] == L"Line 1");
				TEST_ASSERT(paragraphTexts[1] == L"Line 2");
				TEST_ASSERT(paragraphTexts[2] == L"Line 3");
			});

			TEST_CASE(L"Singleline Mode - Without Space")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Singleline;
				config.spaceForFlattenedLineBreak = false;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line 1\nLine 2\nLine 3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 1);
				TEST_ASSERT(paragraphTexts[0] == L"Line 1Line 2Line 3");
			});

			TEST_CASE(L"Singleline Mode - With Space")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = false;
				config.paragraphMode = GuiDocumentParagraphMode::Singleline;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line 1\nLine 2\nLine 3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 1);
				TEST_ASSERT(paragraphTexts[0] == L"Line 1 Line 2 Line 3");
			});
		});
	});

	TEST_CATEGORY(L"UserInput_FormatText_WStringToList (Double Line Breaking Mode)")
	{
		GuiDocumentConfig defaultConfig;
		defaultConfig.autoExpand = false;
		defaultConfig.pasteAsPlainText = false;
		defaultConfig.wrapLine = true;
		defaultConfig.paragraphMode = GuiDocumentParagraphMode::Paragraph;
		defaultConfig.paragraphPadding = true;
		defaultConfig.doubleLineBreaksBetweenParagraph = true;
		defaultConfig.spaceForFlattenedLineBreak = false;
		defaultConfig.paragraphRecycle = true;

		TEST_CATEGORY(L"Basic Double Line Break Detection")
		{
			TEST_CASE(L"Text With Single Line Breaks Only")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line 1\nLine 2\nLine 3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 1);
				TEST_ASSERT(paragraphTexts[0] == L"Line 1\r\nLine 2\r\nLine 3");
			});

			TEST_CASE(L"Text With Double Line Breaks")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Paragraph 1\n\nParagraph 2\n\nParagraph 3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 3);
				TEST_ASSERT(paragraphTexts[0] == L"Paragraph 1");
				TEST_ASSERT(paragraphTexts[1] == L"Paragraph 2");
				TEST_ASSERT(paragraphTexts[2] == L"Paragraph 3");
			});

			TEST_CASE(L"Windows vs Unix Line Endings")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				// Test Windows line endings
				collections::List<WString> paragraphTexts1;
				WString inputText1 = L"Para 1\r\n\r\nPara 2";
				GuiDocumentCommonInterface::UserInput_FormatText(inputText1, paragraphTexts1, evalConfig);
				
				// Test Unix line endings  
				collections::List<WString> paragraphTexts2;
				WString inputText2 = L"Para 1\n\nPara 2";
				GuiDocumentCommonInterface::UserInput_FormatText(inputText2, paragraphTexts2, evalConfig);
				
				TEST_ASSERT(paragraphTexts1.Count() == 2);
				TEST_ASSERT(paragraphTexts2.Count() == 2);
				TEST_ASSERT(paragraphTexts1[0] == L"Para 1");
				TEST_ASSERT(paragraphTexts2[0] == L"Para 1");
				TEST_ASSERT(paragraphTexts1[1] == L"Para 2");
				TEST_ASSERT(paragraphTexts2[1] == L"Para 2");
			});
		});

		TEST_CATEGORY(L"Consecutive Mixed Line Break Patterns")
		{
			TEST_CASE(L"Single Followed By Double Line Breaks")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\nLine2\n\nLine3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"Line1\r\nLine2");
				TEST_ASSERT(paragraphTexts[1] == L"Line3");
			});

			TEST_CASE(L"Double Followed By Single Line Breaks")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\n\nLine2\nLine3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"Line1");
				TEST_ASSERT(paragraphTexts[1] == L"Line2\r\nLine3");
			});

			TEST_CASE(L"Complex Consecutive Pattern With Multiple Empties")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\n\n\nLine2\nLine3\n\nLine4";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 3);
				TEST_ASSERT(paragraphTexts[0] == L"Line1");
				TEST_ASSERT(paragraphTexts[1] == L"\r\nLine2\r\nLine3");
				TEST_ASSERT(paragraphTexts[2] == L"Line4");
			});

			TEST_CASE(L"Alternating Pattern")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\nLine2\n\nLine3\nLine4\n\nLine5";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 3);
				TEST_ASSERT(paragraphTexts[0] == L"Line1\r\nLine2");
				TEST_ASSERT(paragraphTexts[1] == L"Line3\r\nLine4");
				TEST_ASSERT(paragraphTexts[2] == L"Line5");
			});

			TEST_CASE(L"Multiple Consecutive Empty Lines")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\n\n\n\nLine2";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 3);
				TEST_ASSERT(paragraphTexts[0] == L"Line1");
				TEST_ASSERT(paragraphTexts[1] == L"");
				TEST_ASSERT(paragraphTexts[2] == L"Line2");
			});
		});

		TEST_CATEGORY(L"Fragment Processing Edge Cases")
		{
			TEST_CASE(L"Text Ending With Empty Lines")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Content\n\n";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"Content");
				TEST_ASSERT(paragraphTexts[1] == L"");
			});

			TEST_CASE(L"Text Starting With Empty Lines")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"\n\nContent";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"");
				TEST_ASSERT(paragraphTexts[1] == L"Content");
			});

			TEST_CASE(L"Empty Text")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				// Debug the actual count to understand the behavior
				TEST_ASSERT(paragraphTexts.Count() == 0);
			});

			TEST_CASE(L"Only Empty Lines")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"\n\n\n";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"");
				TEST_ASSERT(paragraphTexts[1] == L"\r\n");
			});
		});

		TEST_CATEGORY(L"Paragraph Mode Integration")
		{
			TEST_CASE(L"Paragraph Mode - Preserves Line Breaks")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\nLine2\n\nLine3\nLine4";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"Line1\r\nLine2");
				TEST_ASSERT(paragraphTexts[1] == L"Line3\r\nLine4");
			});

			TEST_CASE(L"Multiline Mode - Removes Line Breaks Within Paragraphs")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Multiline;
				config.spaceForFlattenedLineBreak = false;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\nLine2\n\nLine3\nLine4";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"Line1Line2");
				TEST_ASSERT(paragraphTexts[1] == L"Line3Line4");
			});

			TEST_CASE(L"Multiline Mode - With Space For Flattened Line Breaks")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Multiline;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\nLine2\n\nLine3\nLine4";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"Line1 Line2");
				TEST_ASSERT(paragraphTexts[1] == L"Line3 Line4");
			});

			TEST_CASE(L"Singleline Mode - Joins All Paragraphs")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Singleline;
				config.spaceForFlattenedLineBreak = false;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Para1\nLine2\n\nPara2\nLine4";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 1);
				TEST_ASSERT(paragraphTexts[0] == L"Para1Line2Para2Line4");
			});

			TEST_CASE(L"Singleline Mode - With Space For Flattened Line Breaks")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Singleline;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Para1\nLine2\n\nPara2\nLine4";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 1);
				TEST_ASSERT(paragraphTexts[0] == L"Para1 Line2 Para2 Line4");
			});

			TEST_CASE(L"Multiline Mode With Consecutive Mixed Patterns")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Multiline;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\nLine2\n\nLine3\nLine4\n\nLine5";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 3);
				TEST_ASSERT(paragraphTexts[0] == L"Line1 Line2");
				TEST_ASSERT(paragraphTexts[1] == L"Line3 Line4");
				TEST_ASSERT(paragraphTexts[2] == L"Line5");
			});
		});

		TEST_CATEGORY(L"SubmitFragment Logic Validation")
		{
			TEST_CASE(L"Fragment Joining With Windows Line Endings")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\r\nLine2\r\n\r\nLine3";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"Line1\r\nLine2");
				TEST_ASSERT(paragraphTexts[1] == L"Line3");
			});

			TEST_CASE(L"Fragment Processing With Mixed Line Endings")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Paragraph;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Line1\r\nLine2\n\nLine3\nLine4";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"Line1\r\nLine2");
				TEST_ASSERT(paragraphTexts[1] == L"Line3\r\nLine4");
			});

			TEST_CASE(L"Ending Empty Lines With Multiline Mode")
			{
				GuiDocumentConfig config = defaultConfig;
				config.doubleLineBreaksBetweenParagraph = true;
				config.paragraphMode = GuiDocumentParagraphMode::Multiline;
				config.spaceForFlattenedLineBreak = true;
				GuiDocumentConfigEvaluated evalConfig(config);

				collections::List<WString> paragraphTexts;
				WString inputText = L"Content\n\n";
				
				GuiDocumentCommonInterface::UserInput_FormatText(inputText, paragraphTexts, evalConfig);
				
				TEST_ASSERT(paragraphTexts.Count() == 2);
				TEST_ASSERT(paragraphTexts[0] == L"Content");
				TEST_ASSERT(paragraphTexts[1] == L"");
			});
		});
	});

	TEST_CATEGORY(L"UserInput_FormatDocument")
	{
		GuiDocumentConfig defaultConfig;
		defaultConfig.autoExpand = false;
		defaultConfig.pasteAsPlainText = false;
		defaultConfig.wrapLine = true;
		defaultConfig.paragraphMode = GuiDocumentParagraphMode::Paragraph;
		defaultConfig.paragraphPadding = true;
		defaultConfig.doubleLineBreaksBetweenParagraph = true;
		defaultConfig.spaceForFlattenedLineBreak = false;
		defaultConfig.paragraphRecycle = true;

		auto MakeConfig = [&](bool pasteAsPlainText, GuiDocumentParagraphMode paragraphMode, bool spaceForFlattenedLineBreak) -> GuiDocumentConfig
		{
			GuiDocumentConfig config = defaultConfig;
			config.pasteAsPlainText = pasteAsPlainText;
			config.paragraphMode = paragraphMode;
			config.spaceForFlattenedLineBreak = spaceForFlattenedLineBreak;
			return config;
		};

		struct FormatDocumentTestCase
		{
			const wchar_t* description;
			const wchar_t* inputXml;
			const wchar_t* expectedXml;
			GuiDocumentConfig config;
		};

		FormatDocumentTestCase testCases[] = {
			// Plain text conversion tests
			{
				L"Plain Text Mode - Complex Formatting Removal",
				LR"XML(<Doc><Content><p align="Center"><div style="Title"><b><i>Complex</i></b></div></p><p><u>Formatted</u></p></Content><Styles><Style name="Title"><size>1.5x</size></Style></Styles></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Complex</nop></p><p><nop>Formatted</nop></p></Content><Styles/></Doc>)XML",
				MakeConfig(true, GuiDocumentParagraphMode::Paragraph, false)
			},
			
			// Empty document after conversion
			{
				L"Empty Document After Plain Text Conversion",
				LR"XML(<Doc><Content><p><img source="test.png"/></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p/></Content><Styles/></Doc>)XML",
				MakeConfig(true, GuiDocumentParagraphMode::Paragraph, false)
			},
			
			// Paragraph mode tests
			{
				L"Paragraph Mode - No Processing",
				LR"XML(<Doc><Content><p><nop>Line1<br/>Line2</nop></p><p><nop>Para2<br/>Line2</nop></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Line1</nop><br/><nop>Line2</nop></p><p><nop>Para2</nop><br/><nop>Line2</nop></p></Content><Styles/></Doc>)XML",
				MakeConfig(false, GuiDocumentParagraphMode::Paragraph, false)
			},
			
			{
				L"Multiline Mode - Line Joining Only",
				LR"XML(<Doc><Content><p><nop>Line1<br/>Line2</nop></p><p><nop>Para2<br/>Line2</nop></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Line1</nop><sp/><nop>Line2</nop></p><p><nop>Para2</nop><sp/><nop>Line2</nop></p></Content><Styles/></Doc>)XML",
				MakeConfig(false, GuiDocumentParagraphMode::Multiline, true)
			},
			
			{
				L"Singleline Mode - Complete Joining",
				LR"XML(<Doc><Content><p><nop>Line1<br/>Line2</nop></p><p><nop>Para2<br/>Line2</nop></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Line1</nop><sp/><nop>Line2</nop><sp/><nop>Para2</nop><sp/><nop>Line2</nop></p></Content><Styles/></Doc>)XML",
				MakeConfig(false, GuiDocumentParagraphMode::Singleline, true)
			},
			
			// Sequential processing tests
			{
				L"Sequential Processing - Plain Text Then Line Joining",
				LR"XML(<Doc><Content><p align="Center"><b><nop>Bold<br/>Text</nop></b></p><p><i><nop>Italic<br/>Text</nop></i></p></Content></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>Bold Text</nop><sp/><nop>Italic Text</nop></p></Content><Styles/></Doc>)XML",
				MakeConfig(true, GuiDocumentParagraphMode::Singleline, true)
			},
			
			// Integration tests
			{
				L"Complex Integration - Mixed Content with All Processing",
				LR"XML(<Doc><Content><p align="Right"><div style="Complex"><b><nop>Style<br/>Bold</nop></b></div></p><p><img source="test.png"/></p><p><object name="TestObj"/><nop>After<br/>Object</nop></p></Content><Styles><Style name="Complex"><size>2x</size></Style></Styles></Doc>)XML",
				LR"XML(<Doc><Content><p><nop>StyleBold</nop><nop>AfterObject</nop></p></Content><Styles/></Doc>)XML",
				MakeConfig(true, GuiDocumentParagraphMode::Singleline, false)
			}
		};

		auto xs = FromArray(testCases);
		for (auto [testCase, index] : indexed(xs))
		{
			TEST_CASE(WString::Unmanaged(testCase.description))
			{
				auto model = LoadDoc(testCase.inputXml);
				GuiDocumentCommonInterface::UserInput_FormatDocument(model, nullptr, GuiDocumentConfigEvaluated(testCase.config));
				auto actualXml = SaveDoc(model);
				
				if (testCase.expectedXml != actualXml)
				{
					TEST_PRINT(WString::Unmanaged(L"Expected: ") + testCase.expectedXml);
					TEST_PRINT(L"Actual: " + actualXml);
				}
				TEST_ASSERT(testCase.expectedXml == actualXml);
			});
		}
	});
}