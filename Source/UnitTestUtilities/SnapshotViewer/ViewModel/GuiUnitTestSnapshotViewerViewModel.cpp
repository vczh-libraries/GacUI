#include "GuiUnitTestSnapshotViewerViewModel.h"
#include "../../../PlatformProviders/Remote/GuiRemoteProtocol.h"

namespace vl::presentation::unittest
{
	using namespace collections;
	using namespace filesystem;
	using namespace gaclib_controls;
	using namespace glr::json;
	using namespace vl::presentation::remoteprotocol;

	JsonFormatting GetJsonFormatting()
	{
		JsonFormatting formatting;
		formatting.spaceAfterColon = true;
		formatting.spaceAfterComma = true;
		formatting.crlf = true;
		formatting.compact = true;
		return formatting;
	}

/***********************************************************************
UnitTestSnapshotDomNode
***********************************************************************/

	class UnitTestSnapshotDomNode : public Object, public virtual IUnitTestSnapshotDomNode
	{
	protected:
		UnitTest_RenderingTrace&			trace;
		UnitTest_RenderingFrame&			frame;
		Ptr<RenderingDom>					renderingDom;
		WString								name;
		WString								dom;
		WString								element;
		List<Ptr<UnitTestSnapshotDomNode>>	children;

	public:
		UnitTestSnapshotDomNode(UnitTest_RenderingTrace& _trace, UnitTest_RenderingFrame& _frame, Ptr<RenderingDom> _renderingDom)
			: trace(_trace)
			, frame(_frame)
			, renderingDom(_renderingDom)
		{
			if (renderingDom->children)
			{
				for (auto child : *renderingDom->children.Obj())
				{
					children.Add(Ptr(new UnitTestSnapshotDomNode(trace, frame, child)));
				}
			}
		}

		WString GetName() override
		{
			WString idString, typeString, hittestString, boundsString;
			{
				idString = L"[" + itow(renderingDom->id) + L"] ";
			}
			if (renderingDom->content.element && trace.createdElements)
			{
				vint index = trace.createdElements->Keys().IndexOf(renderingDom->content.element.Value());
				if (index != -1)
				{
					auto renderingType = trace.createdElements->Values()[index];
					auto typeName = ConvertCustomTypeToJson(renderingType).Cast<JsonString>()->content.value;
					typeString = L" {" + typeName + L"}";
				}
			}
			if(renderingDom->content.hitTestResult)
			{
				auto hittest = renderingDom->content.hitTestResult.Value();
				auto hittestName = ConvertCustomTypeToJson(hittest).Cast<JsonString>()->content.value;
				typeString = L" <" + hittestName + L">";
			}
			{
				auto bounds = renderingDom->content.bounds;
				boundsString = L" (" + itow(bounds.x1) + L"," + itow(bounds.y1) + L") - (" + itow(bounds.Width()) + L"x" + itow(bounds.Height()) + L")";
			}
			return idString + typeString + hittestString + boundsString;
		}

		vint GetDomID() override
		{
			return renderingDom->id;
		}

		WString GetDomAsJsonText() override
		{
			if (dom == L"")
			{
				RenderingDom copy = *renderingDom.Obj();
				copy.children = nullptr;
				dom = JsonToString(ConvertCustomTypeToJson(copy), GetJsonFormatting());
			}
			return dom;
		}

		WString GetElementAsJsonText() override
		{
			if (element == L"")
			{
				element = L"null";
				if (renderingDom->content.element && frame.elements)
				{
					vint index = frame.elements->Keys().IndexOf(renderingDom->content.element.Value());
					if (index != -1)
					{
						auto elementVariant = frame.elements->Values()[index];
						elementVariant.Apply([&](auto desc)
						{
							element = JsonToString(ConvertCustomTypeToJson(desc), GetJsonFormatting());
						});
					}
				}
			}
			return element;
		}

		LazyList<Ptr<IUnitTestSnapshotDomNode>> GetChildren() override
		{
			return From(children).Cast<IUnitTestSnapshotDomNode>();
		}
	};

/***********************************************************************
UnitTestSnapshotFrame
***********************************************************************/

	class UnitTestSnapshotFrame : public Object, public virtual IUnitTestSnapshotFrame
	{
		friend const remoteprotocol::UnitTest_RenderingFrame& GetRenderingFrame(Ptr<IUnitTestSnapshotFrame> frame);
	protected:
		vint							index;
		UnitTest_RenderingTrace&		trace;
		UnitTest_RenderingFrame			frame;
		Ptr<UnitTestSnapshotDomNode>	domRoot;
		WString							elements;
		WString							commands;
		WString							dom;

	public:
		UnitTestSnapshotFrame(vint _index, UnitTest_RenderingTrace& _trace, UnitTest_RenderingFrame _frame)
			: index(_index)
			, trace(_trace)
			, frame(_frame)
		{
		}

		WString GetName() override
		{
			if (frame.frameName)
			{
				return frame.frameName.Value();
			}
			else
			{
				return itow(index);
			}
		}

		WString GetElementsAsJsonText() override
		{
			if (elements == L"")
			{
				elements = JsonToString(ConvertCustomTypeToJson(frame.elements), GetJsonFormatting());
			}
			return elements;
		}

		WString GetDomAsJsonText() override
		{
			if (dom == L"")
			{
				dom = JsonToString(ConvertCustomTypeToJson(frame.root), GetJsonFormatting());
			}
			return dom;
		}

		Ptr<IUnitTestSnapshotDomNode> GetDom() override
		{
			if (!domRoot)
			{
				domRoot = Ptr(new UnitTestSnapshotDomNode(trace, frame, frame.root));
			}
			return domRoot;
		}
	};

	const remoteprotocol::UnitTest_RenderingFrame& GetRenderingFrame(Ptr<IUnitTestSnapshotFrame> frame)
	{
		return frame.Cast<UnitTestSnapshotFrame>()->frame;
	}

/***********************************************************************
UnitTestSnapshotFileNode
***********************************************************************/

	class UnitTestSnapshotFileNode : public Object, public virtual IUnitTestSnapshotFileNode
	{
		friend const remoteprotocol::UnitTest_RenderingTrace& GetRenderingTrace(Ptr<gaclib_controls::IUnitTestSnapshotFileNode> node);
	protected:
		File								file;
		Ptr<UnitTest_RenderingTrace>		renderingTrace;
		List<Ptr<UnitTestSnapshotFrame>>	frames;

		void EnsureLoaded()
		{
			if (!renderingTrace)
			{
				WString jsonText = file.ReadAllTextByBom();
				Ptr<JsonNode> jsonNode;
				{
					glr::json::Parser parser;
					jsonNode = JsonParse(jsonText, parser);
				}
				renderingTrace = Ptr(new UnitTest_RenderingTrace);
				ConvertJsonToCustomType(jsonNode, *renderingTrace.Obj());

				frames.Clear();
				if (renderingTrace->frames)
				{
					for (auto [frame, index] : indexed(*renderingTrace->frames.Obj()))
					{
						frames.Add(Ptr(new UnitTestSnapshotFrame(index, *renderingTrace.Obj(), frame)));
					}
				}
			}
		}
	public:
		UnitTestSnapshotFileNode(FilePath _filePath)
			: file(_filePath)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestSnapshotFileNode::UnitTestSnapshotFileNode(FilePath)#"
			CHECK_ERROR(file.Exists(), ERROR_MESSAGE_PREFIX L"File does not exist.");
#undef ERROR_MESSAGE_PREFIX
		}

		UnitTestSnapshotFileNodeType GetNodeType() override
		{
			return UnitTestSnapshotFileNodeType::File;
		}

		WString GetName() override
		{
			auto name = file.GetFilePath().GetName();
			return name.Left(name.Length() - 5);
		}

		LazyList<Ptr<IUnitTestSnapshotFileNode>> GetChildren() override
		{
			return {};
		}

		LazyList<Ptr<IUnitTestSnapshotFrame>> GetFrames() override
		{
			EnsureLoaded();
			return From(frames).Cast<IUnitTestSnapshotFrame>();
		}

		void Refresh() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	};

	const remoteprotocol::UnitTest_RenderingTrace& GetRenderingTrace(Ptr<gaclib_controls::IUnitTestSnapshotFileNode> node)
	{
		return *node.Cast<UnitTestSnapshotFileNode>()->renderingTrace.Obj();
	}

/***********************************************************************
UnitTestSnapshotFolderNode
***********************************************************************/

	class UnitTestSnapshotFolderNode : public Object, public virtual IUnitTestSnapshotFileNode
	{
	protected:
		Folder										folder;
		Ptr<List<Ptr<IUnitTestSnapshotFileNode>>>	children;

	public:
		UnitTestSnapshotFolderNode(FilePath _filePath)
			: folder(_filePath)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestSnapshotFolderNode::UnitTestSnapshotFolderNode(FilePath)#"
			CHECK_ERROR(folder.Exists(), ERROR_MESSAGE_PREFIX L"Folder does not exist.");
#undef ERROR_MESSAGE_PREFIX
		}

		UnitTestSnapshotFileNodeType GetNodeType() override
		{
			return UnitTestSnapshotFileNodeType::Folder;
		}

		WString GetName() override
		{
			return folder.GetFilePath().GetName();
		}

		LazyList<Ptr<IUnitTestSnapshotFileNode>> GetChildren() override
		{
			if (!children)
			{
				children = Ptr(new List<Ptr<IUnitTestSnapshotFileNode>>);
				{
					List<Folder> folders;
					folder.GetFolders(folders);
					for (auto name : From(folders)
						.Select([](auto&& folder) {return folder.GetFilePath().GetName(); })
						.OrderBySelf())
					{
						children->Add(Ptr(new UnitTestSnapshotFolderNode(folder.GetFilePath() / name)));
					}
				}
				{
					List<File> files;
					folder.GetFiles(files);
					for (auto name : From(files)
						.Select([](auto&& file) {return file.GetFilePath().GetName(); })
						.OrderBySelf())
					{
						if (name.Length() > 5 && name.Right(5) == L".json")
						{
							children->Add(Ptr(new UnitTestSnapshotFileNode(folder.GetFilePath() / name)));
						}
					}
				}
			}
			return children;
		}

		LazyList<Ptr<IUnitTestSnapshotFrame>> GetFrames() override
		{
			return {};
		}

		void Refresh() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	};

/***********************************************************************
UnitTestSnapshotViewerViewModel
***********************************************************************/

	UnitTestSnapshotViewerViewModel::UnitTestSnapshotViewerViewModel(FilePath snapshotFolderPath)
		: rootNode(Ptr(new UnitTestSnapshotFolderNode(snapshotFolderPath)))
	{
	}

	UnitTestSnapshotViewerViewModel::~UnitTestSnapshotViewerViewModel()
	{
	}

	Ptr<IUnitTestSnapshotFileNode> UnitTestSnapshotViewerViewModel::GetRootNode()
	{
		return rootNode;
	}
}