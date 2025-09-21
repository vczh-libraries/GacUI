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
		// Tests for plain text conversion functionality
		// Will be implemented in subsequent task
	});

	TEST_CATEGORY(L"UserInput_JoinParagraphs_ListWString")
	{
		// Tests for string list paragraph joining  
		// Will be implemented in subsequent task
	});

	TEST_CATEGORY(L"UserInput_JoinParagraphs_DocumentModel")
	{
		// Tests for DocumentModel paragraph joining
		// Will be implemented in subsequent task
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