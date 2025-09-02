#include "../ViewModel/GuiUnitTestSnapshotViewerViewModel.h"

namespace gaclib_controls
{
	class UnitTestSnapshotViewerAppWindow : public UnitTestSnapshotViewerWindow
	{
	protected:
		vl::glr::json::Parser										jsonParser;
		vl::presentation::compositions::GuiBoundsComposition*		rootComposition = nullptr;
		vl::presentation::compositions::GuiBoundsComposition*		selectedComposition = nullptr;
		vl::presentation::compositions::GuiBoundsComposition*		highlightComposition = nullptr;

		void textListFrames_SelectionChanged(vl::presentation::compositions::GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void treeViewDom_SelectionChanged(vl::presentation::compositions::GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
	
	public:
		UnitTestSnapshotViewerAppWindow(vl::Ptr<vl::presentation::unittest::UnitTestSnapshotViewerViewModel> viewModel);
	};
}