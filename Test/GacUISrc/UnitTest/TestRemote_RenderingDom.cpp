#include "../../../Source/PlatformProviders/Remote/Protocol/FrameOperations/GuiRemoteProtocolSchema_FrameOperations.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::glr;
using namespace vl::presentation::remoteprotocol;

static void AssertDomsEqual(Ptr<RenderingDom> dom1, Ptr<RenderingDom> dom2)
{
	TEST_ASSERT(dom1->id == dom2->id);
	TEST_ASSERT(dom1->content.hitTestResult == dom2->content.hitTestResult);
	TEST_ASSERT(dom1->content.cursor == dom2->content.cursor);
	TEST_ASSERT(dom1->content.element == dom2->content.element);
	TEST_ASSERT(dom1->content.bounds == dom2->content.bounds);
	TEST_ASSERT(dom1->content.validArea == dom2->content.validArea);

	vint count1 = dom1->children ? dom1->children->Count() : 0;
	vint count2 = dom2->children ? dom2->children->Count() : 0;
	TEST_ASSERT(count1 == count2);

	if (count1 > 0)
	{
		for (auto [child, i] : indexed(*dom1->children.Obj()))
		{
			AssertDomsEqual(child, dom2->children->Get(i));
		}
	}
};

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

	auto runCopyDom = [&](Ptr<json::JsonNode> jsonNode)
	{
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
	};

	TEST_CASE(L"CopyDom SingleRoot")
	{
		auto jsonNode = json::JsonParse(WString::Unmanaged(inputDomJsonSingleRoot), jsonParser);
		runCopyDom(jsonNode);
	});

	TEST_CASE(L"CopyDom BinaryTree")
	{
		auto jsonNode = json::JsonParse(WString::Unmanaged(inputDomJsonBinaryTree), jsonParser);
		runCopyDom(jsonNode);
	});

	TEST_CASE(L"BuildDomIndex SingleRoot")
	{
		auto jsonNode = json::JsonParse(WString::Unmanaged(inputDomJsonSingleRoot), jsonParser);

		Ptr<RenderingDom> dom;
		ConvertJsonToCustomType(jsonNode, dom);

		DomIndex index;
		BuildDomIndex(dom, index);

		TEST_ASSERT(index.Count() == 1);
		TEST_ASSERT(index[0].id == -1);
		TEST_ASSERT(index[0].dom == dom);
	});

	TEST_CASE(L"BuildDomIndex BinaryTree")
	{
		auto jsonNode = json::JsonParse(WString::Unmanaged(inputDomJsonBinaryTree), jsonParser);

		Ptr<RenderingDom> dom;
		ConvertJsonToCustomType(jsonNode, dom);

		DomIndex index;
		BuildDomIndex(dom, index);

		TEST_ASSERT(index.Count() == 5);
		TEST_ASSERT(index[0].id == -1);
		TEST_ASSERT(index[0].parentId == -1);
		TEST_ASSERT(index[0].dom == dom);
		TEST_ASSERT(index[1].id == 0);
		TEST_ASSERT(index[1].parentId == -1);
		TEST_ASSERT(index[1].dom == dom->children->Get(0));
		TEST_ASSERT(index[2].id == 1);
		TEST_ASSERT(index[2].parentId == -1);
		TEST_ASSERT(index[2].dom == dom->children->Get(1));
		TEST_ASSERT(index[3].id == 2);
		TEST_ASSERT(index[3].parentId == 0);
		TEST_ASSERT(index[3].dom == dom->children->Get(0)->children->Get(0));
		TEST_ASSERT(index[4].id == 3);
		TEST_ASSERT(index[4].parentId == 0);
		TEST_ASSERT(index[4].dom == dom->children->Get(0)->children->Get(1));
	});

	auto runDiffDom = [&](Ptr<RenderingDom> domFrom, Ptr<RenderingDom> domTo, RenderingDom_DiffsInOrder& diffs)
	{
		DomIndex indexFrom, indexTo;
		BuildDomIndex(domFrom, indexFrom);
		BuildDomIndex(domTo, indexTo);
		DiffDom(domFrom, indexFrom, domTo, indexTo, diffs);
		UpdateDomInplace(domFrom, indexFrom, diffs);
		AssertDomsEqual(domFrom, domTo);

		DomIndex indexFrom2;
		BuildDomIndex(domFrom, indexFrom2);
		TEST_ASSERT(CompareEnumerable(indexFrom, indexFrom2) == 0);
	};

	TEST_CASE(L"Diff SingleRoot -> SingleRoot")
	{
		Ptr<RenderingDom> dom;
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonSingleRoot), jsonParser), dom);

		RenderingDom_DiffsInOrder diffs;
		runDiffDom(dom, dom, diffs);
		TEST_ASSERT(diffs.diffsInOrder->Count() == 0);
	});

	TEST_CASE(L"Diff BinaryTree -> BinaryTree")
	{
		Ptr<RenderingDom> dom;
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonBinaryTree), jsonParser), dom);

		RenderingDom_DiffsInOrder diffs;
		runDiffDom(dom, dom, diffs);
		TEST_ASSERT(diffs.diffsInOrder->Count() == 0);
	});

	TEST_CASE(L"Diff SingleRoot with content changed")
	{
		Ptr<RenderingDom> domFrom, domTo;
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonSingleRoot), jsonParser), domFrom);
		domTo = CopyDom(domFrom);

		domTo->content.element = 0;

		RenderingDom_DiffsInOrder diffs;
		runDiffDom(domFrom, domTo, diffs);
		TEST_ASSERT(diffs.diffsInOrder->Count() == 1);
	});

	TEST_CASE(L"Diff BinaryTree with root content changed")
	{
		Ptr<RenderingDom> domFrom, domTo;
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonBinaryTree), jsonParser), domFrom);
		domTo = CopyDom(domFrom);

		domTo->content.element = 0;

		RenderingDom_DiffsInOrder diffs;
		runDiffDom(domFrom, domTo, diffs);
		TEST_ASSERT(diffs.diffsInOrder->Count() == 1);
	});

	TEST_CASE(L"Diff BinaryTree with leaves content changed")
	{
		Ptr<RenderingDom> domFrom, domTo;
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonBinaryTree), jsonParser), domFrom);
		domTo = CopyDom(domFrom);

		domTo->children->Get(0)->children->Get(0)->content.element = 0;
		domTo->children->Get(0)->children->Get(1)->content.element = 0;
		domTo->children->Get(1)->content.element = 0;

		RenderingDom_DiffsInOrder diffs;
		runDiffDom(domFrom, domTo, diffs);
		TEST_ASSERT(diffs.diffsInOrder->Count() == 3);
	});

	TEST_CASE(L"Diff SingleRoot with children null -> []")
	{
		Ptr<RenderingDom> domFrom, domTo;
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonSingleRoot), jsonParser), domFrom);
		domTo = CopyDom(domFrom);

		TEST_ASSERT(!domFrom->children);
		TEST_ASSERT(!domTo->children);
		domTo->children = Ptr(new List<Ptr<RenderingDom>>);

		RenderingDom_DiffsInOrder diffs;
		runDiffDom(domFrom, domTo, diffs);
		TEST_ASSERT(diffs.diffsInOrder->Count() == 0);
	});

	TEST_CASE(L"Diff SingleRoot with children [] -> null")
	{
		Ptr<RenderingDom> domFrom, domTo;
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonSingleRoot), jsonParser), domFrom);
		domTo = CopyDom(domFrom);

		TEST_ASSERT(!domFrom->children);
		TEST_ASSERT(!domTo->children);
		domFrom->children = Ptr(new List<Ptr<RenderingDom>>);

		RenderingDom_DiffsInOrder diffs;
		runDiffDom(domFrom, domTo, diffs);
		TEST_ASSERT(diffs.diffsInOrder->Count() == 0);
	});

	TEST_CASE(L"Diff SingleRoot -> BinaryTree with children null")
	{
	});

	TEST_CASE(L"Diff SingleRoot -> BinaryTree with children []")
	{
	});

	TEST_CASE(L"Diff SingleRoot -> BinaryTree")
	{
	});

	TEST_CASE(L"Diff BinaryTree -> SingleRoot")
	{
	});
}