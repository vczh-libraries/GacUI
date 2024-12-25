#include "../Source/GuiUnitTestSnapshotViewer.h"

namespace vl::presentation::remoteprotocol
{
	struct UnitTest_RenderingFrame;
	struct UnitTest_RenderingTrace;
}

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

	extern const remoteprotocol::UnitTest_RenderingTrace&	GetRenderingTrace(Ptr<gaclib_controls::IUnitTestSnapshotFileNode> node);
	extern const remoteprotocol::UnitTest_RenderingFrame&	GetRenderingFrame(Ptr<gaclib_controls::IUnitTestSnapshotFrame> frame);
}