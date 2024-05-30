#include "../ViewModel/GuiUnitTestSnapshotViewerViewModel.h"

namespace gaclib_controls
{
	class UnitTestSnapshotViewerAppWindow : public UnitTestSnapshotViewerWindow
	{
	protected:
		vl::presentation::compositions::GuiBoundsComposition*		rootComposition = nullptr;
	
		void textListFrames_SelectionChanged(vl::presentation::compositions::GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
	
	public:
		UnitTestSnapshotViewerAppWindow(vl::Ptr<vl::presentation::unittest::UnitTestSnapshotViewerViewModel> viewModel);
	};
}