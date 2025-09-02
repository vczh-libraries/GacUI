#include "GuiUnitTestSnapshotViewerApp.h"
#include "../../../PlatformProviders/Remote/GuiRemoteProtocol.h"

namespace gaclib_controls
{
	using namespace vl;
	using namespace vl::collections;
	using namespace vl::filesystem;
	using namespace vl::glr::json;
	using namespace vl::presentation;
	using namespace vl::presentation::elements;
	using namespace vl::presentation::compositions;
	using namespace vl::presentation::controls;
	using namespace vl::presentation::unittest;
	using namespace gaclib_controls;

	class DomProp : public Object
	{
	public:
		static const WString				PropertyName;

		Ptr<remoteprotocol::RenderingDom>	dom;
	};

	const WString DomProp::PropertyName = WString::Unmanaged(L"DomID");

	/***********************************************************************
	Helper Functions
	***********************************************************************/

	Alignment GetAlignment(remoteprotocol::ElementHorizontalAlignment alignment)
	{
		switch (alignment)
		{
		case remoteprotocol::ElementHorizontalAlignment::Left: return Alignment::Left;
		case remoteprotocol::ElementHorizontalAlignment::Right: return Alignment::Right;
		default: return Alignment::Center;
		}
	}

	Alignment GetAlignment(remoteprotocol::ElementVerticalAlignment alignment)
	{
		switch (alignment)
		{
		case remoteprotocol::ElementVerticalAlignment::Top: return Alignment::Top;
		case remoteprotocol::ElementVerticalAlignment::Bottom: return Alignment::Bottom;
		default: return Alignment::Center;
		}
	}

	void InstallDom(
		const remoteprotocol::UnitTest_RenderingTrace& trace,
		const remoteprotocol::UnitTest_RenderingFrame& frame,
		GuiGraphicsComposition* container,
		vint x,
		vint y,
		Ptr<remoteprotocol::RenderingDom> dom,
		vint& cursorCounter)
	{
		auto domProp = Ptr(new DomProp);
		domProp->dom = dom;
		auto bounds = new GuiBoundsComposition;
		bounds->SetInternalProperty(DomProp::PropertyName, domProp);
		container->AddChild(bounds);

		bounds->SetExpectedBounds(Rect({ dom->content.bounds.x1 - x,dom->content.bounds.y1 - y }, { dom->content.bounds.Width(),dom->content.bounds.Height() }));

		auto cursor = dom->content.cursor;
		if (dom->content.cursor)
		{
			cursorCounter++;
		}

		if (dom->content.hitTestResult && cursorCounter == 0)
		{
			switch (dom->content.hitTestResult.Value())
			{
			case INativeWindowListener::BorderLeft:
			case INativeWindowListener::BorderRight:
				cursor = INativeCursor::SizeWE;
				break;
			case INativeWindowListener::BorderTop:
			case INativeWindowListener::BorderBottom:
				cursor = INativeCursor::SizeNS;
				break;
			case INativeWindowListener::BorderLeftTop:
			case INativeWindowListener::BorderRightBottom:
				cursor = INativeCursor::SizeNWSE;
				break;
			case INativeWindowListener::BorderRightTop:
			case INativeWindowListener::BorderLeftBottom:
				cursor = INativeCursor::SizeNESW;
				break;
			case INativeWindowListener::Icon:
			case INativeWindowListener::ButtonMinimum:
			case INativeWindowListener::ButtonMaximum:
			case INativeWindowListener::ButtonClose:
				cursor = INativeCursor::Hand;
				break;
			case INativeWindowListener::Title:
				cursor = INativeCursor::SizeAll;
				break;
			default:;
			}
		}

		if (cursor)
		{
			bounds->SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(cursor.Value()));
		}

		if (dom->content.element)
		{
			switch (trace.createdElements->Get(dom->content.element.Value()))
			{
			case remoteprotocol::RendererType::FocusRectangle:
			{
				auto element = Ptr(GuiFocusRectangleElement::Create());
				bounds->SetOwnedElement(element);
			}
			break;
			case remoteprotocol::RendererType::Raw:
				// Do Nothing
				break;
			case remoteprotocol::RendererType::SolidBorder:
			{
				auto element = Ptr(GuiSolidBorderElement::Create());
				bounds->SetOwnedElement(element);
				auto& desc = frame.elements->Get(dom->content.element.Value()).Get<remoteprotocol::ElementDesc_SolidBorder>();

				element->SetColor(desc.borderColor);
				element->SetShape(desc.shape);
			}
			break;
			case remoteprotocol::RendererType::SinkBorder:
			{
				auto element = Ptr(Gui3DBorderElement::Create());
				bounds->SetOwnedElement(element);
				auto& desc = frame.elements->Get(dom->content.element.Value()).Get<remoteprotocol::ElementDesc_SinkBorder>();

				element->SetColors(desc.leftTopColor, desc.rightBottomColor);
			}
			break;
			case remoteprotocol::RendererType::SinkSplitter:
			{
				auto element = Ptr(Gui3DSplitterElement::Create());
				bounds->SetOwnedElement(element);
				auto& desc = frame.elements->Get(dom->content.element.Value()).Get<remoteprotocol::ElementDesc_SinkSplitter>();

				element->SetColors(desc.leftTopColor, desc.rightBottomColor);
				element->SetDirection(desc.direction);
			}
			break;
			case remoteprotocol::RendererType::SolidBackground:
			{
				auto element = Ptr(GuiSolidBackgroundElement::Create());
				bounds->SetOwnedElement(element);
				auto& desc = frame.elements->Get(dom->content.element.Value()).Get<remoteprotocol::ElementDesc_SolidBackground>();

				element->SetColor(desc.backgroundColor);
				element->SetShape(desc.shape);
			}
			break;
			case remoteprotocol::RendererType::GradientBackground:
			{
				auto element = Ptr(GuiGradientBackgroundElement::Create());
				bounds->SetOwnedElement(element);
				auto& desc = frame.elements->Get(dom->content.element.Value()).Get<remoteprotocol::ElementDesc_GradientBackground>();

				element->SetColors(desc.leftTopColor, desc.rightBottomColor);
				element->SetDirection(desc.direction);
				element->SetShape(desc.shape);
			}
			break;
			case remoteprotocol::RendererType::InnerShadow:
			{
				auto element = Ptr(GuiInnerShadowElement::Create());
				bounds->SetOwnedElement(element);
				auto& desc = frame.elements->Get(dom->content.element.Value()).Get<remoteprotocol::ElementDesc_InnerShadow>();

				element->SetColor(desc.shadowColor);
				element->SetThickness(desc.thickness);
			}
			break;
			case remoteprotocol::RendererType::SolidLabel:
			{
				auto element = Ptr(GuiSolidLabelElement::Create());
				bounds->SetOwnedElement(element);
				auto& desc = frame.elements->Get(dom->content.element.Value()).Get<remoteprotocol::ElementDesc_SolidLabel>();

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
				auto& desc = frame.elements->Get(dom->content.element.Value()).Get<remoteprotocol::ElementDesc_Polygon>();

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
				auto& desc = frame.elements->Get(dom->content.element.Value()).Get<remoteprotocol::ElementDesc_ImageFrame>();

				element->SetAlignments(GetAlignment(desc.horizontalAlignment), GetAlignment(desc.verticalAlignment));
				element->SetStretch(desc.stretch);
				element->SetEnabled(desc.enabled);

				if (desc.imageId)
				{
					vint index = trace.imageCreations->Keys().IndexOf(desc.imageId.Value());
					if (index != -1)
					{
						auto binary = trace.imageCreations->Values()[index].imageData;
						binary->SeekFromBegin(0);
						element->SetImage(GetCurrentController()->ImageService()->CreateImageFromStream(*binary.Obj()), desc.imageFrame);
					}
				}
			}
			break;
			default:
				CHECK_FAIL(L"This element is not supported yet.");
				break;
			}
		}

		if (dom->children)
		{
			for (auto child : *dom->children.Obj())
			{
				InstallDom(trace, frame, bounds, dom->content.bounds.x1, dom->content.bounds.y1, child, cursorCounter);
			}
		}

		if (dom->content.cursor)
		{
			cursorCounter--;
		}
	}

	GuiBoundsComposition* BuildRootComposition(const remoteprotocol::UnitTest_RenderingTrace& trace, const remoteprotocol::UnitTest_RenderingFrame& frame)
	{
		vint w = frame.windowSize.clientBounds.Width().value;
		vint h = frame.windowSize.clientBounds.Height().value;
		auto focusComposition = new GuiBoundsComposition;
		{
			focusComposition->SetExpectedBounds(Rect({ 5,5 }, { w + 4,h + 4 }));
		}
		auto canvasComposition = new GuiBoundsComposition;
		{
			focusComposition->AddChild(canvasComposition);
			canvasComposition->SetExpectedBounds(Rect({ 2,2 }, { w,h }));
		}

		if (frame.root && frame.root->children)
		{
			vint cursorCounter = 0;
			for (auto child : *frame.root->children.Obj())
			{
				InstallDom(trace, frame, canvasComposition, 0, 0, child, cursorCounter);
			}
		}
		return focusComposition;
	}

	/***********************************************************************
	UnitTestSnapshotViewerAppWindow
	***********************************************************************/

	void UnitTestSnapshotViewerAppWindow::textListFrames_SelectionChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
	{
		if (rootComposition)
		{
			SafeDeleteComposition(rootComposition);
			rootComposition = nullptr;
			selectedComposition = nullptr;
			highlightComposition = nullptr;
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

			rootComposition->GetEventReceiver()->mouseEnter.AttachMethod(this, &UnitTestSnapshotViewerAppWindow::rootComposition_MouseEnter);
			rootComposition->GetEventReceiver()->mouseLeave.AttachMethod(this, &UnitTestSnapshotViewerAppWindow::rootComopsition_MouseLeave);
			rootComposition->GetEventReceiver()->mouseMove.AttachMethod(this, &UnitTestSnapshotViewerAppWindow::rootComposition_MouseMove);
		}
	}

	void UnitTestSnapshotViewerAppWindow::treeViewDom_SelectionChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
	{
		if (rootComposition)
		{
			if (auto domNode = treeViewDom->GetSelectedItem().GetSharedPtr().Cast<IUnitTestSnapshotDomNode>())
			{
				remoteprotocol::RenderingDom renderingDom;
				remoteprotocol::ConvertJsonToCustomType(JsonParse(domNode->GetDomAsJsonText(), jsonParser), renderingDom);
				if (!selectedComposition)
				{
					selectedComposition = new GuiBoundsComposition;
					auto element = Ptr(GuiSolidBorderElement::Create());
					element->SetColor(Color(255, 255, 0));
					selectedComposition->SetOwnedElement(element);
					rootComposition->AddChild(selectedComposition);
				}
				selectedComposition->SetExpectedBounds(Rect(
					renderingDom.content.bounds.x1 + 1,
					renderingDom.content.bounds.y1 + 1,
					renderingDom.content.bounds.x2 + 3,
					renderingDom.content.bounds.y2 + 3
				));
			}
			else
			{
				if (selectedComposition)
				{
					SafeDeleteComposition(selectedComposition);
					selectedComposition = nullptr;
				}
			}
		}
	}

	void UnitTestSnapshotViewerAppWindow::rootComposition_MouseEnter(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
	{
	}

	void UnitTestSnapshotViewerAppWindow::rootComopsition_MouseLeave(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
	{
		if (highlightComposition)
		{
			SafeDeleteComposition(highlightComposition);
			highlightComposition = nullptr;
		}
	}

	void UnitTestSnapshotViewerAppWindow::rootComposition_MouseMove(GuiGraphicsComposition* sender, GuiMouseEventArgs& arguments)
	{
		if (!arguments.ctrl) return;
		auto domSource = arguments.compositionSource;
		while (domSource)
		{
			auto domProp = domSource->GetInternalProperty(DomProp::PropertyName).Cast<DomProp>();
			if (domProp)
			{
				if (!highlightComposition)
				{
					highlightComposition = new GuiBoundsComposition;
					auto element = Ptr(GuiSolidBorderElement::Create());
					element->SetColor(Color(255, 0, 0));
					highlightComposition->SetOwnedElement(element);
					rootComposition->AddChild(highlightComposition);
				}
				highlightComposition->SetExpectedBounds(Rect(
					domProp->dom->content.bounds.x1 + 1,
					domProp->dom->content.bounds.y1 + 1,
					domProp->dom->content.bounds.x2 + 3,
					domProp->dom->content.bounds.y2 + 3
				));
				return;
			}
			domSource = domSource->GetParent();
		}
	}

	void UnitTestSnapshotViewerAppWindow::OnKeyUp(GuiGraphicsComposition* sender, GuiKeyEventArgs& arguments)
	{
		if (arguments.code == VKEY::KEY_CONTROL && highlightComposition)
		{
			SafeDeleteComposition(highlightComposition);
			highlightComposition = nullptr;
		}
	}

	UnitTestSnapshotViewerAppWindow::UnitTestSnapshotViewerAppWindow(Ptr<UnitTestSnapshotViewerViewModel> viewModel)
		: UnitTestSnapshotViewerWindow(viewModel)
	{
		textListFrames->SelectionChanged.AttachMethod(this, &UnitTestSnapshotViewerAppWindow::textListFrames_SelectionChanged);
		treeViewDom->SelectionChanged.AttachMethod(this, &UnitTestSnapshotViewerAppWindow::treeViewDom_SelectionChanged);

		SetFocusableComposition(GetBoundsComposition());
		GetFocusableComposition()->GetEventReceiver()->keyUp.AttachMethod(this, &UnitTestSnapshotViewerAppWindow::OnKeyUp);
	}
}