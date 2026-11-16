#include "../../../Source/PlatformProviders/Remote/Protocol/FrameOperations/GuiRemoteProtocolSchema_FrameOperations.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol_DomDiff.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::glr;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;

static void AssertDomsEqual(Ptr<RenderingDom> dom1, Ptr<RenderingDom> dom2)
{
	TEST_ASSERT(dom1->id == dom2->id);
	TEST_ASSERT(dom1->content.hitTestResult == dom2->content.hitTestResult);
	TEST_ASSERT(dom1->content.cursor == dom2->content.cursor);
	TEST_ASSERT(dom1->content.element == dom2->content.element);
	TEST_ASSERT(dom1->content.bounds == dom2->content.bounds);
	TEST_ASSERT(dom1->content.validArea == dom2->content.validArea);
	TEST_ASSERT(bool(dom1->content.editableAttribute) == bool(dom2->content.editableAttribute));
	if (dom1->content.editableAttribute)
	{
		TEST_ASSERT(dom1->content.editableAttribute.Value().focused == dom2->content.editableAttribute.Value().focused);
		TEST_ASSERT(dom1->content.editableAttribute.Value().editable == dom2->content.editableAttribute.Value().editable);
	}

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

namespace rendering_dom_tests
{
	class RecordingProtocol : public GuiRemoteProtocolCombinator_PassingThrough<void>
	{
	public:
		IGuiRemoteProtocolEvents* receiver = nullptr;
		Ptr<RenderingDom> dom;
		DomIndex index;
		RenderingDom_DiffsInOrder lastDiff;
		vint fullCount = 0;
		vint diffCount = 0;

		RecordingProtocol() : GuiRemoteProtocolCombinator_PassingThrough<void>(nullptr) {}
		void Initialize(IGuiRemoteProtocolEvents* value) override { receiver = value; }
		void RequestRendererBeginRendering(const ElementBeginRendering&) override {}
		void RequestRendererEndRendering(vint) override {}
		void RequestRendererRenderDom(const Ptr<RenderingDom>& value) override
		{
			ConvertJsonToCustomType(ConvertCustomTypeToJson(value), dom);
			BuildDomIndex(dom, index);
			fullCount++;
		}
		void RequestRendererRenderDomDiff(const RenderingDom_DiffsInOrder& value) override
		{
			ConvertJsonToCustomType(ConvertCustomTypeToJson(value), lastDiff);
			UpdateDomInplace(dom, index, lastDiff);
			diffCount++;
		}
	};

	class ConnectionEvents : public GuiRemoteEventCombinator_PassingThrough
	{
	public:
		void OnControllerConnect(const ControllerGlobalConfig&) override {}
	};
}

TEST_FILE
{
	TEST_CASE(L"Every nullable editing metadata transition survives JSON, copy and diff")
	{
		Nullable<ElementEditableAttribute> states[] = { {},{ {false,false} },{ {false,true} },{ {true,false} },{ {true,true} } };
		for (vint from = 0; from < 5; from++)
		for (vint to = 0; to < 5; to++)
		{
			auto oldDom = Ptr(new RenderingDom);
			oldDom->id = -1;
			auto owner = Ptr(new RenderingDom);
			owner->id = 6;
			owner->content.bounds = owner->content.validArea = Rect(0, 0, 100, 100);
			owner->content.editableAttribute = states[from];
			oldDom->children = Ptr(new List<Ptr<RenderingDom>>);
			oldDom->children->Add(owner);
			auto newDom = CopyDom(oldDom);
			newDom->children->Get(0)->content.editableAttribute = states[to];
			Ptr<RenderingDom> jsonDom;
			ConvertJsonToCustomType(ConvertCustomTypeToJson(newDom), jsonDom);
			AssertDomsEqual(newDom, jsonDom);
			AssertDomsEqual(newDom, CopyDom(newDom));
			DomIndex oldIndex, newIndex;
			BuildDomIndex(oldDom, oldIndex);
			BuildDomIndex(newDom, newIndex);
			RenderingDom_DiffsInOrder diff, recovered;
			DiffDom(oldDom, oldIndex, newDom, newIndex, diff);
			ConvertJsonToCustomType(ConvertCustomTypeToJson(diff), recovered);
			TEST_ASSERT(recovered.diffsInOrder->Count() == (from == to ? 0 : 1));
			if (from != to)
			{
				auto change = recovered.diffsInOrder->Get(0);
				TEST_ASSERT(change.id == 6 && change.diffType == RenderingDom_DiffType::Modified);
				TEST_ASSERT(change.content && !change.children);
				TEST_ASSERT(bool(change.content.Value().editableAttribute) == (to != 0));
			}
			auto reconstructed = CopyDom(oldDom);
			DomIndex reconstructedIndex;
			BuildDomIndex(reconstructed, reconstructedIndex);
			UpdateDomInplace(reconstructed, reconstructedIndex, recovered);
			AssertDomsEqual(reconstructed, newDom);
		}
	});

	TEST_CASE(L"A children-only diff leaves the editing attribute unchanged")
	{
		auto oldDom = Ptr(new RenderingDom);
		oldDom->id = -1;
		oldDom->children = Ptr(new List<Ptr<RenderingDom>>);
		auto owner = Ptr(new RenderingDom);
		owner->id = 6;
		owner->content.editableAttribute = ElementEditableAttribute{ true,false };
		oldDom->children->Add(owner);
		auto newDom = CopyDom(oldDom);
		auto child = Ptr(new RenderingDom);
		child->id = 8;
		newDom->children->Get(0)->children = Ptr(new List<Ptr<RenderingDom>>);
		newDom->children->Get(0)->children->Add(child);
		DomIndex oldIndex, newIndex;
		BuildDomIndex(oldDom, oldIndex);
		BuildDomIndex(newDom, newIndex);
		RenderingDom_DiffsInOrder diff, recovered;
		DiffDom(oldDom, oldIndex, newDom, newIndex, diff);
		ConvertJsonToCustomType(ConvertCustomTypeToJson(diff), recovered);
		TEST_ASSERT(recovered.diffsInOrder->Count() == 2);
		auto modified = recovered.diffsInOrder->Get(0);
		TEST_ASSERT(modified.id == 6 && modified.diffType == RenderingDom_DiffType::Modified);
		TEST_ASSERT(!modified.content && modified.children);
		UpdateDomInplace(oldDom, oldIndex, recovered);
		AssertDomsEqual(oldDom, newDom);
	});

	TEST_CASE(L"Editing boundaries preserve siblings, equal nested owners and clipping wrappers")
	{
		RenderingDomBuilder builder;
		builder.RequestRendererBeginRendering();
		auto begin = [&](vint id, Rect bounds, Rect area, bool otherAttributes)
		{
			ElementBoundary command;
			command.id = id;
			command.bounds = bounds;
			command.areaClippedBySelf = area;
			command.editableAttribute = ElementEditableAttribute{ false,false };
			if (otherAttributes)
			{
				command.cursor = INativeCursor::IBeam;
				command.hitTestResult = INativeWindowListener::Client;
			}
			ElementBoundary recovered;
			ConvertJsonToCustomType(ConvertCustomTypeToJson(command), recovered);
			TEST_ASSERT(recovered.editableAttribute && !recovered.editableAttribute.Value().focused && !recovered.editableAttribute.Value().editable);
			builder.RequestRendererBeginBoundary(recovered);
		};
		begin(1, Rect(0, 0, 100, 100), Rect(0, 0, 100, 100), false);
		begin(2, Rect(10, 10, 90, 90), Rect(10, 10, 80, 80), true);
		builder.RequestRendererEndBoundary();
		builder.RequestRendererEndBoundary();
		begin(3, Rect(110, 0, 200, 100), Rect(110, 0, 200, 100), false);
		builder.RequestRendererEndBoundary();
		auto dom = builder.RequestRendererEndRendering();
		TEST_ASSERT(dom->children->Count() == 2 && !dom->content.editableAttribute);
		auto first = dom->children->Get(0);
		auto sibling = dom->children->Get(1);
		TEST_ASSERT(first->id == 6 && sibling->id == 14 && !sibling->children);
		TEST_ASSERT(first->content.editableAttribute && sibling->content.editableAttribute);
		TEST_ASSERT(first->children->Count() == 1);
		auto clip = first->children->Get(0);
		TEST_ASSERT(clip->id == 11 && !clip->content.editableAttribute && clip->children->Count() == 1);
		auto nested = clip->children->Get(0);
		TEST_ASSERT(nested->id == 10 && nested->content.editableAttribute && !nested->children);
		TEST_ASSERT(nested->content.cursor && nested->content.hitTestResult);
		TEST_ASSERT(nested->content.cursor.Value() == INativeCursor::IBeam && nested->content.hitTestResult.Value() == INativeWindowListener::Client);
		TEST_ASSERT(nested->content.validArea == Rect(10, 10, 80, 80));
		AssertDomsEqual(dom, CopyDom(dom));
	});

	TEST_CASE(L"Converter reconnect sends current metadata and serialized diffs remove tags explicitly")
	{
		rendering_dom_tests::RecordingProtocol target;
		rendering_dom_tests::ConnectionEvents events;
		GuiRemoteProtocolDomDiffConverter converter(&target);
		converter.Initialize(&events);
		auto render = [&](Nullable<ElementEditableAttribute> value)
		{
			converter.RequestRendererBeginRendering({});
			ElementBoundary command;
			command.id = 1;
			command.bounds = command.areaClippedBySelf = Rect(0, 0, 100, 100);
			command.cursor = INativeCursor::IBeam;
			command.editableAttribute = value;
			converter.RequestRendererBeginBoundary(command);
			converter.RequestRendererEndBoundary();
			converter.RequestRendererEndRendering(0);
			RenderingDomBuilder fresh;
			fresh.RequestRendererBeginRendering();
			fresh.RequestRendererBeginBoundary(command);
			fresh.RequestRendererEndBoundary();
			AssertDomsEqual(target.dom, fresh.RequestRendererEndRendering());
		};
		render(ElementEditableAttribute{ true,true });
		TEST_ASSERT(target.fullCount == 1 && target.diffCount == 0);
		render(ElementEditableAttribute{ true,false });
		TEST_ASSERT(target.fullCount == 1 && target.diffCount == 1);
		TEST_ASSERT(target.lastDiff.diffsInOrder->Count() == 1 && target.lastDiff.diffsInOrder->Get(0).content);
		target.receiver->OnControllerConnect({});
		render(ElementEditableAttribute{ false,false });
		TEST_ASSERT(target.fullCount == 2 && target.diffCount == 1);
		render({});
		TEST_ASSERT(target.fullCount == 2 && target.diffCount == 2);
		auto change = target.lastDiff.diffsInOrder->Get(0);
		TEST_ASSERT(change.content && !change.content.Value().editableAttribute);
		render({});
		TEST_ASSERT(target.lastDiff.diffsInOrder->Count() == 0);
	});

	const wchar_t* inputDomJsonSingleRoot = LR"Dom(
{
  "id":-1,
  "content":{
    "hitTestResult":null,
    "cursor":null,
    "element":null,
    "bounds":{"x1":0,"y1":1,"x2":2,"y2":3},
    "validArea":{"x1":4,"y1":5,"x2":6,"y2":7},
    "editableAttribute":null
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
    "validArea":{"x1":4,"y1":5,"x2":6,"y2":7},
    "editableAttribute":null
  },
  "children":[{
    "id":0,
    "content":{
      "hitTestResult":"Client",
      "cursor":null,
      "element":null,
      "bounds":{"x1":0,"y1":0,"x2":0,"y2":0},
      "validArea":{"x1":0,"y1":0,"x2":0,"y2":0},
      "editableAttribute":null
    },
    "children":[{
      "id":2,
      "content":{
        "hitTestResult":null,
        "cursor":"Arrow",
        "element":null,
        "bounds":{"x1":2,"y1":2,"x2":2,"y2":2},
        "validArea":{"x1":2,"y1":2,"x2":2,"y2":2},
        "editableAttribute":null
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
        "validArea":{"x1":3,"y1":3,"x2":3,"y2":3},
        "editableAttribute":null
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
      "validArea":{"x1":1,"y1":1,"x2":1,"y2":1},
      "editableAttribute":null
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

	TEST_CASE(L"Diff SingleRoot -> BinaryTree")
	{
		Ptr<RenderingDom> domFrom, domTo;
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonSingleRoot), jsonParser), domFrom);
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonBinaryTree), jsonParser), domTo);

		RenderingDom_DiffsInOrder diffs;
		runDiffDom(domFrom, domTo, diffs);
		TEST_ASSERT(diffs.diffsInOrder->Count() == 5);
	});

	TEST_CASE(L"Diff SingleRoot -> BinaryTree with children []")
	{
		Ptr<RenderingDom> domFrom, domTo;
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonSingleRoot), jsonParser), domFrom);
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonBinaryTree), jsonParser), domTo);

		domTo->children->Get(0)->children->Get(0)->children = Ptr(new List<Ptr<RenderingDom>>);
		domTo->children->Get(0)->children->Get(1)->children = Ptr(new List<Ptr<RenderingDom>>);
		domTo->children->Get(1)->children = Ptr(new List<Ptr<RenderingDom>>);

		RenderingDom_DiffsInOrder diffs;
		runDiffDom(domFrom, domTo, diffs);
		TEST_ASSERT(diffs.diffsInOrder->Count() == 5);
	});

	TEST_CASE(L"Diff BinaryTree -> SingleRoot")
	{
		Ptr<RenderingDom> domFrom, domTo;
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonBinaryTree), jsonParser), domFrom);
		ConvertJsonToCustomType(json::JsonParse(WString::Unmanaged(inputDomJsonSingleRoot), jsonParser), domTo);

		RenderingDom_DiffsInOrder diffs;
		runDiffDom(domFrom, domTo, diffs);
		TEST_ASSERT(diffs.diffsInOrder->Count() == 5);
	});
}
