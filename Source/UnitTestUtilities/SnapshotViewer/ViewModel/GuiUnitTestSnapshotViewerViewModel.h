#include "../Source/GuiUnitTestSnapshotViewer.h"

namespace vl::presentation::unittest
{
	class UnitTestSnapshotViewerViewModel
		: public Object
		, public virtual gaclib_controls::IUnitTestSnapshotViewerViewModel
	{
	protected:
		Ptr<gaclib_controls::IUnitTestSnapshotFileNode>		rootNode;

	public:
		UnitTestSnapshotViewerViewModel(filesystem::FilePath snapshotFolderPath);
		~UnitTestSnapshotViewerViewModel();

		Ptr<gaclib_controls::IUnitTestSnapshotFileNode>		GetRootNode() override;
	};
}