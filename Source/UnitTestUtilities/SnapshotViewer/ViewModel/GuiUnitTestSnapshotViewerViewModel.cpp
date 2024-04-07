#include "GuiUnitTestSnapshotViewerViewModel.h"

namespace vl::presentation::unittest
{
	using namespace collections;
	using namespace filesystem;
	using namespace gaclib_controls;

/***********************************************************************
UnitTestSnapshotFileNode
***********************************************************************/

	class UnitTestSnapshotFileNode : public Object, public virtual IUnitTestSnapshotFileNode
	{
	protected:
		File				file;

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
			return file.GetFilePath().GetName();
		}

		LazyList<Ptr<IUnitTestSnapshotFileNode>> GetChildren() override
		{
			return {};
		}

		WString LoadContent() override
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

		WString LoadContent() override
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