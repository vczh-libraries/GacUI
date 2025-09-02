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
		void rootComposition_MouseEnter(vl::presentation::compositions::GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void rootComopsition_MouseLeave(vl::presentation::compositions::GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void rootComposition_MouseMove(vl::presentation::compositions::GuiGraphicsComposition* sender, vl::presentation::compositions::GuiMouseEventArgs& arguments);
		void OnKeyUp(vl::presentation::compositions::GuiGraphicsComposition* sender, vl::presentation::compositions::GuiKeyEventArgs& arguments);

	public:
		UnitTestSnapshotViewerAppWindow(vl::Ptr<vl::presentation::unittest::UnitTestSnapshotViewerViewModel> viewModel);
	};
}