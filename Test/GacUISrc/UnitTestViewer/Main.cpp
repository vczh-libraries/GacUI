#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "DarkSkin.h"
#include "../../../Source/UnitTestUtilities/SnapshotViewer/ViewModel/GuiUnitTestSnapshotViewerViewModel.h"
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"

using namespace vl;
using namespace vl::collections;
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

	static Alignment GetAlignment(remoteprotocol::ElementHorizontalAlignment alignment)
	{
		switch (alignment)
		{
		case remoteprotocol::ElementHorizontalAlignment::Left: return Alignment::Left;
		case remoteprotocol::ElementHorizontalAlignment::Right: return Alignment::Right;
		default: return Alignment::Center;
		}
	}

	static Alignment GetAlignment(remoteprotocol::ElementVerticalAlignment alignment)
	{
		switch (alignment)
		{
		case remoteprotocol::ElementVerticalAlignment::Top: return Alignment::Top;
		case remoteprotocol::ElementVerticalAlignment::Bottom: return Alignment::Bottom;
		default: return Alignment::Center;
		}
	}

	static void InstallDom(
		const remoteprotocol::RenderingTrace& trace,
		const remoteprotocol::RenderingFrame& frame,
		GuiGraphicsComposition* container,
		vint x,
		vint y,
		Ptr<remoteprotocol::RenderingDom> dom)
	{
		auto bounds = new GuiBoundsComposition;
		container->AddChild(bounds);

		bounds->SetExpectedBounds(Rect({ dom->bounds.x1 - x,dom->bounds.y1 - y }, { dom->bounds.Width(),dom->bounds.Height() }));

		if (dom->cursor)
		{
			bounds->SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(dom->cursor.Value()));
		}

		if (dom->element)
		{
			switch (trace.createdElements->Get(dom->element.Value()))
			{
			case remoteprotocol::RendererType::FocusRectangle:
				{
					auto element = Ptr(GuiFocusRectangleElement::Create());
					bounds->SetOwnedElement(element);
				}
				break;
			case remoteprotocol::RendererType::SolidBorder:
				{
					auto element = Ptr(GuiSolidBorderElement::Create());
					bounds->SetOwnedElement(element);
					auto& desc = frame.elements->Get(dom->element.Value()).Get<remoteprotocol::ElementDesc_SolidBorder>();

					element->SetColor(desc.borderColor);
					element->SetShape(desc.shape);
				}
				break;
			case remoteprotocol::RendererType::SinkBorder:
				{
					auto element = Ptr(Gui3DBorderElement::Create());
					bounds->SetOwnedElement(element);
					auto& desc = frame.elements->Get(dom->element.Value()).Get<remoteprotocol::ElementDesc_SinkBorder>();

					element->SetColors(desc.leftTopColor, desc.rightBottomColor);
				}
				break;
			case remoteprotocol::RendererType::SinkSplitter:
				{
					auto element = Ptr(Gui3DSplitterElement::Create());
					bounds->SetOwnedElement(element);
					auto& desc = frame.elements->Get(dom->element.Value()).Get<remoteprotocol::ElementDesc_SinkSplitter>();

					element->SetColors(desc.leftTopColor, desc.rightBottomColor);
					element->SetDirection(desc.direction);
				}
				break;
			case remoteprotocol::RendererType::SolidBackground:
				{
					auto element = Ptr(GuiSolidBackgroundElement::Create());
					bounds->SetOwnedElement(element);
					auto& desc = frame.elements->Get(dom->element.Value()).Get<remoteprotocol::ElementDesc_SolidBackground>();

					element->SetColor(desc.backgroundColor);
					element->SetShape(desc.shape);
				}
				break;
			case remoteprotocol::RendererType::GradientBackground:
				{
					auto element = Ptr(GuiGradientBackgroundElement::Create());
					bounds->SetOwnedElement(element);
					auto& desc = frame.elements->Get(dom->element.Value()).Get<remoteprotocol::ElementDesc_GradientBackground>();

					element->SetColors(desc.leftTopColor, desc.rightBottomColor);
					element->SetDirection(desc.direction);
					element->SetShape(desc.shape);
				}
				break;
			case remoteprotocol::RendererType::InnerShadow:
				{
					auto element = Ptr(GuiInnerShadowElement::Create());
					bounds->SetOwnedElement(element);
					auto& desc = frame.elements->Get(dom->element.Value()).Get<remoteprotocol::ElementDesc_InnerShadow>();

					element->SetColor(desc.shadowColor);
					element->SetThickness(desc.thickness);
				}
				break;
			case remoteprotocol::RendererType::SolidLabel:
				{
					auto element = Ptr(GuiSolidLabelElement::Create());
					bounds->SetOwnedElement(element);
					auto& desc = frame.elements->Get(dom->element.Value()).Get<remoteprotocol::ElementDesc_SolidLabel>();

					element->SetColor(desc.textColor);
					element->SetAlignments(GetAlignment(desc.horizontalAlignment), GetAlignment(desc.verticalAlignment));
					element->SetWrapLine(desc.wrapLine);
					element->SetWrapLineHeightCalculation(desc.wrapLineHeightCalculation);
					element->SetEllipse(desc.ellipse);
					element->SetMultiline(desc.multiline);
					element->SetFont(desc.font.Value());
					element->SetText(desc.text.Value());
				}
				break;
			case remoteprotocol::RendererType::Polygon:
				{
					auto element = Ptr(GuiPolygonElement::Create());
					bounds->SetOwnedElement(element);
					auto& desc = frame.elements->Get(dom->element.Value()).Get<remoteprotocol::ElementDesc_Polygon>();

					element->SetSize(desc.size);
					element->SetBorderColor(desc.borderColor);
					element->SetBackgroundColor(desc.backgroundColor);

					if (desc.points && desc.points->Count() > 0)
					{
						element->SetPoints(&desc.points->Get(0), desc.points->Count());
					}
				}
				break;
			case remoteprotocol::RendererType::ImageFrame:
				{
					auto element = Ptr(GuiImageFrameElement::Create());
					bounds->SetOwnedElement(element);
					auto& desc = frame.elements->Get(dom->element.Value()).Get<remoteprotocol::ElementDesc_ImageFrame>();
				}
				break;
			default:
				CHECK_FAIL(L"This element is not supported yet.");
				break;
			}
		}

		if(dom->children)
		{
			for (auto child : *dom->children.Obj())
			{
				InstallDom(trace, frame, bounds, dom->bounds.x1, dom->bounds.y1, child);
			}
		}
	}

	static GuiBoundsComposition* BuildRootComposition(const remoteprotocol::RenderingTrace& trace, const remoteprotocol::RenderingFrame& frame)
	{
		vint w = frame.windowSize.clientBounds.Width().value;
		vint h = frame.windowSize.clientBounds.Height().value;
		auto focusComposition = new GuiBoundsComposition;
		{
			focusComposition->SetExpectedBounds(Rect({ 5,5 }, { w + 2,h + 2 }));
			auto element = Ptr(GuiFocusRectangleElement::Create());
			focusComposition->SetOwnedElement(element);
		}
		auto canvasComposition = new GuiBoundsComposition;
		{
			focusComposition->AddChild(canvasComposition);
			canvasComposition->SetExpectedBounds(Rect({ 1,1 }, { w,h }));
		}

		if (frame.root && frame.root->children)
		{
			for (auto child : *frame.root->children.Obj())
			{
				InstallDom(trace, frame, canvasComposition, 0, 0, child);
			}
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