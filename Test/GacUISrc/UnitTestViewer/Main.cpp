#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "DarkSkin.h"
#include "../../../Source/UnitTestUtilities/SnapshotViewer/ViewModel/GuiUnitTestSnapshotViewerViewModel.h"
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"

using namespace vl;
using namespace vl::filesystem;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::unittest;
using namespace gaclib_controls;

class MainWindow : public UnitTestSnapshotViewerWindow
{
protected:
	GuiBoundsComposition*				rootComposition = nullptr;

	GuiBoundsComposition* BuildRootComposition(const remoteprotocol::RenderingTrace& trace, const remoteprotocol::RenderingFrame& frame)
	{
		vint w = frame.windowSize.clientBounds.Width().value;
		vint h = frame.windowSize.clientBounds.Height().value;
		auto focusComposition = new GuiBoundsComposition;
		{
			focusComposition->SetAlignmentToParent(Margin(5, 5, -1, -1));
			focusComposition->SetExpectedBounds(Rect({ 0,0 }, { w + 2,h + 2 }));
			auto element = Ptr(GuiFocusRectangleElement::Create());
			focusComposition->SetOwnedElement(element);
		}
		auto canvasComposition = new GuiBoundsComposition;
		{
			focusComposition->AddChild(canvasComposition);
			canvasComposition->SetAlignmentToParent(Margin(1, 1, -1, -1));
			canvasComposition->SetExpectedBounds(Rect({ 0,0 }, { 0,0 }));
		}
		return focusComposition;
	}

	void textListFrames_SelectionChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
	{
		if (rootComposition)
		{
			SafeDeleteComposition(rootComposition);
			rootComposition = nullptr;
		}

		auto nodeObj = treeViewFileNodes->GetSelectedItem();
		if (!nodeObj.GetSharedPtr()) return;
		auto node = nodeObj.GetSharedPtr().Cast<IUnitTestSnapshotFileNode>();

		auto frameObj = textListFrames->GetSelectedItem();
		if (!frameObj.GetSharedPtr()) return;
		auto frame = frameObj.GetSharedPtr().Cast<IUnitTestSnapshotFrame>();

		if (node && node->GetNodeType() == UnitTestSnapshotFileNodeType::File && frame)
		{
			rootComposition = BuildRootComposition(GetRenderingTrace(node), GetRenderingFrame(frame));
			scRendering->GetContainerComposition()->AddChild(rootComposition);
		}
	}

public:
	MainWindow(Ptr<UnitTestSnapshotViewerViewModel> viewModel)
		: UnitTestSnapshotViewerWindow(viewModel)
	{
		textListFrames->SelectionChanged.AttachMethod(this, &MainWindow::textListFrames_SelectionChanged);
	}
};

void GuiMain()
{
	theme::RegisterTheme(Ptr(new darkskin::Theme));
	{

#if defined VCZH_MSVC
#ifdef _WIN64
		FilePath snapshotFolderPath = GetApplication()->GetExecutablePath() + L"..\\..\\..\\..\\..\\Resources\\UnitTestSnapshots";
#else
		FilePath snapshotFolderPath = GetApplication()->GetExecutablePath() + L"..\\..\\..\\..\\Resources\\UnitTestSnapshots";
#endif
#elif defined VCZH_GCC
		FilePath snapshotFolderPath = GetApplication()->GetExecutablePath() + L"../../Resources/UnitTestSnapshots";
#endif
		MainWindow window(Ptr(new UnitTestSnapshotViewerViewModel(snapshotFolderPath)));
		window.ForceCalculateSizeImmediately();
		window.MoveToScreenCenter();
		window.WindowOpened.AttachLambda([&](auto&&...)
		{
			window.ShowMaximized();
		});
		GetApplication()->Run(&window);
	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	int result = SetupWindowsDirect2DRenderer();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}