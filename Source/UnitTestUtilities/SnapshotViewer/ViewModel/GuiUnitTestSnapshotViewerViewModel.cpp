#include "GuiUnitTestSnapshotViewerViewModel.h"
#include "../../../PlatformProviders/Remote/GuiRemoteProtocol.h"

namespace vl::presentation::unittest
{
	using namespace collections;
	using namespace filesystem;
	using namespace gaclib_controls;
	using namespace glr::json;
	using namespace vl::presentation::remoteprotocol;

/***********************************************************************
UnitTestSnapshotFrame
***********************************************************************/

	class UnitTestSnapshotFrame : public Object, public virtual IUnitTestSnapshotFrame
	{
	protected:
		vint				index;
		RenderingFrame		frame;
		WString				elements;
		WString				commands;
		WString				dom;
		JsonFormatting		formatting;

		friend const remoteprotocol::RenderingFrame& GetRenderingFrame(Ptr<IUnitTestSnapshotFrame> frame)
		{
			return frame.Cast<UnitTestSnapshotFrame>()->frame;
		}

	public:
		UnitTestSnapshotFrame(vint _index, RenderingFrame _frame)
			: index(_index)
			, frame(_frame)
		{
			formatting.spaceAfterColon = true;
			formatting.spaceAfterComma = true;
			formatting.crlf = true;
			formatting.compact = true;
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
				elements = JsonToString(ConvertCustomTypeToJson(frame.elements), formatting);
			}
			return elements;
		}

		WString GetCommandsAsJsonText() override
		{
			if (commands == L"")
			{
				commands = JsonToString(ConvertCustomTypeToJson(frame.commands), formatting);
			}
			return commands;
		}

		WString GetDomAsJsonText() override
		{
			if (dom == L"")
			{
				dom = JsonToString(ConvertCustomTypeToJson(frame.root), formatting);
			}
			return dom;
		}
	};

/***********************************************************************
UnitTestSnapshotFileNode
***********************************************************************/

	class UnitTestSnapshotFileNode : public Object, public virtual IUnitTestSnapshotFileNode
	{
	protected:
		File								file;
		Ptr<RenderingTrace>					renderingTrace;
		List<Ptr<UnitTestSnapshotFrame>>	frames;

		friend const remoteprotocol::RenderingTrace& GetRenderingTrace(Ptr<gaclib_controls::IUnitTestSnapshotFileNode> node)
		{
			return *node.Cast<UnitTestSnapshotFileNode>()->renderingTrace.Obj();
		}

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
				renderingTrace = Ptr(new RenderingTrace);
				ConvertJsonToCustomType(jsonNode, *renderingTrace.Obj());

				frames.Clear();
				if (renderingTrace->frames)
				{
					for (auto [frame, index] : indexed(*renderingTrace->frames.Obj()))
					{
						frames.Add(Ptr(new UnitTestSnapshotFrame(index, frame)));
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