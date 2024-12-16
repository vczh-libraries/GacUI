#include "../../../Source/PlatformProviders/Remote/Protocol/FrameOperations/GuiRemoteProtocolSchema_FrameOperations.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::glr;
using namespace vl::presentation::remoteprotocol;

TEST_FILE
{
	const wchar_t* inputDomJsonSingleRoot = LR"Dom(
{
  "id":-1,
  "content":{
    "hitTestResult":null,
    "cursor":null,
    "element":null,
    "bounds":{"x1":0,"y1":1,"x2":2,"y2":3},
    "validArea":{"x1":4,"y1":5,"x2":6,"y2":7}
  },
  "children":null
}
)Dom";

	const wchar_t* inputDomJsonBinaryTree = LR"Dom(
{
  "id":-1,
  "content":{
    "hitTestResult":null,
    "cursor":null,
    "element":null,
    "bounds":{"x1":0,"y1":1,"x2":2,"y2":3},
    "validArea":{"x1":4,"y1":5,"x2":6,"y2":7}
  },
  "children":[{
    "id":0,
    "content":{
      "hitTestResult":"Client",
      "cursor":null,
      "element":null,
      "bounds":{"x1":0,"y1":0,"x2":0,"y2":0},
      "validArea":{"x1":0,"y1":0,"x2":0,"y2":0}
    },
    "children":[{
      "id":2,
      "content":{
        "hitTestResult":null,
        "cursor":"Arrow",
        "element":null,
        "bounds":{"x1":2,"y1":2,"x2":2,"y2":2},
        "validArea":{"x1":2,"y1":2,"x2":2,"y2":2}
      },
      "children":null
    },
    {
      "id":3,
      "content":{
        "hitTestResult":"Title",
        "cursor":"Help",
        "element":null,
        "bounds":{"x1":3,"y1":3,"x2":3,"y2":3},
        "validArea":{"x1":3,"y1":3,"x2":3,"y2":3}
      },
      "children":null
    }]
  },
  {
    "id":1,
    "content":{
      "hitTestResult":null,
      "cursor":null,
      "element":99,
      "bounds":{"x1":1,"y1":1,"x2":1,"y2":1},
      "validArea":{"x1":1,"y1":1,"x2":1,"y2":1}
    },
    "children":null
  }]
}
)Dom";

	json::Parser jsonParser;
	json::JsonFormatting formatting;
	formatting.spaceAfterColon = true;
	formatting.spaceAfterComma = true;
	formatting.crlf = true;
	formatting.compact = true;
	formatting.indentation = L"  ";

	TEST_CASE(L"CopyDom SingleRoot")
	{
		auto jsonNode = json::JsonParse(WString::Unmanaged(inputDomJsonSingleRoot), jsonParser);
		auto jsonFormatted = GenerateToStream([&](TextWriter& writer)
		{
			json::JsonPrint(jsonNode, writer, formatting);
		});

		Ptr<RenderingDom> domParsed;
		ConvertJsonToCustomType(jsonNode, domParsed);
		auto jsonParsed = GenerateToStream([&](TextWriter& writer)
		{
			json::JsonPrint(ConvertCustomTypeToJson(domParsed), writer, formatting);
		});

		auto domCopied = CopyDom(domParsed);
		auto jsonCopied = GenerateToStream([&](TextWriter& writer)
		{
			json::JsonPrint(ConvertCustomTypeToJson(domCopied), writer, formatting);
		});

		TEST_ASSERT(jsonFormatted == jsonParsed);
		TEST_ASSERT(jsonFormatted == jsonCopied);
	});

	TEST_CASE(L"CopyDom BinaryTree")
	{
		auto jsonNode = json::JsonParse(WString::Unmanaged(inputDomJsonBinaryTree), jsonParser);
		auto jsonFormatted = GenerateToStream([&](TextWriter& writer)
		{
			json::JsonPrint(jsonNode, writer, formatting);
		});

		Ptr<RenderingDom> domParsed;
		ConvertJsonToCustomType(jsonNode, domParsed);
		auto jsonParsed = GenerateToStream([&](TextWriter& writer)
		{
			json::JsonPrint(ConvertCustomTypeToJson(domParsed), writer, formatting);
		});

		auto domCopied = CopyDom(domParsed);
		auto jsonCopied = GenerateToStream([&](TextWriter& writer)
		{
			json::JsonPrint(ConvertCustomTypeToJson(domCopied), writer, formatting);
		});

		TEST_ASSERT(jsonFormatted == jsonParsed);
		TEST_ASSERT(jsonFormatted == jsonCopied);
	});
}