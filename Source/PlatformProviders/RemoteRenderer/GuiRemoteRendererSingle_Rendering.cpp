#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace collections;
	using namespace elements;
	using namespace remoteprotocol;

	Alignment GuiRemoteRendererSingle::GetAlignment(remoteprotocol::ElementHorizontalAlignment alignment)
	{
		switch (alignment)
		{
		case remoteprotocol::ElementHorizontalAlignment::Left: return Alignment::Left;
		case remoteprotocol::ElementHorizontalAlignment::Right: return Alignment::Right;
		default: return Alignment::Center;
		}
	}

	Alignment GuiRemoteRendererSingle::GetAlignment(remoteprotocol::ElementVerticalAlignment alignment)
	{
		switch (alignment)
		{
		case remoteprotocol::ElementVerticalAlignment::Top: return Alignment::Top;
		case remoteprotocol::ElementVerticalAlignment::Bottom: return Alignment::Bottom;
		default: return Alignment::Center;
		}
	}

/***********************************************************************
* Rendering
***********************************************************************/

	void GuiRemoteRendererSingle::RequestRendererCreated(const Ptr<collections::List<remoteprotocol::RendererCreation>>& arguments)
	{
		supressRefresh = true;
		if (arguments)
		{
			for (auto&& rc : *arguments.Obj())
			{
				Ptr<IGuiGraphicsElement> element;
				switch (rc.type)
				{
				case RendererType::FocusRectangle:
					element = Ptr(GuiFocusRectangleElement::Create());
					break;
				case RendererType::Raw:
					// Do Nothing
					break;
				case RendererType::SolidBorder:
					element = Ptr(GuiSolidBorderElement::Create());
					break;
				case RendererType::SinkBorder:
					element = Ptr(Gui3DBorderElement::Create());
					break;
				case RendererType::SinkSplitter:
					element = Ptr(Gui3DSplitterElement::Create());
					break;
				case RendererType::SolidBackground:
					element = Ptr(GuiSolidBackgroundElement::Create());
					break;
				case RendererType::GradientBackground:
					element = Ptr(GuiGradientBackgroundElement::Create());
					break;
				case RendererType::InnerShadow:
					element = Ptr(GuiInnerShadowElement::Create());
					break;
				case RendererType::SolidLabel:
					element = Ptr(GuiSolidLabelElement::Create());
					break;
				case RendererType::Polygon:
					element = Ptr(GuiPolygonElement::Create());
					break;
				case RendererType::ImageFrame:
					element = Ptr(GuiImageFrameElement::Create());
					break;
				case RendererType::DocumentParagraph:
					element = CreateRemoteDocumentParagraphElement(rc.id);
					break;
				default:;
				}

				if (element)
				{
					element->GetRenderer()->SetRenderTarget(GetGuiGraphicsResourceManager()->GetRenderTarget(window));

					if (availableElements.Keys().Contains(rc.id))
					{
						availableElements.Set(rc.id, element);
					}
					else
					{
						availableElements.Add(rc.id, element);
					}
				}
			}
		}
	}

	void GuiRemoteRendererSingle::RequestRendererDestroyed(const Ptr<collections::List<vint>>& arguments)
	{
		supressRefresh = true;
		if (arguments)
		{
			for (auto id : *arguments.Obj())
			{
				focusedParagraphElements.Remove(id);
				availableElements.Remove(id);
				solidLabelMeasurings.Remove(id);
			}
		}
	}

	void GuiRemoteRendererSingle::RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments)
	{
		supressRefresh = true;
		if (arguments.updatedElements)
		{
			for (auto&& desc : *arguments.updatedElements.Obj())
			{
				desc.Apply(Overloading(
					[&](const remoteprotocol::ElementDesc_SolidBorder& d) { RequestRendererUpdateElement_SolidBorder(d); },
					[&](const remoteprotocol::ElementDesc_SinkBorder& d) { RequestRendererUpdateElement_SinkBorder(d); },
					[&](const remoteprotocol::ElementDesc_SinkSplitter& d) { RequestRendererUpdateElement_SinkSplitter(d); },
					[&](const remoteprotocol::ElementDesc_SolidBackground& d) { RequestRendererUpdateElement_SolidBackground(d); },
					[&](const remoteprotocol::ElementDesc_GradientBackground& d) { RequestRendererUpdateElement_GradientBackground(d); },
					[&](const remoteprotocol::ElementDesc_InnerShadow& d) { RequestRendererUpdateElement_InnerShadow(d); },
					[&](const remoteprotocol::ElementDesc_Polygon& d) { RequestRendererUpdateElement_Polygon(d); },
					[&](const remoteprotocol::ElementDesc_SolidLabel& d) { RequestRendererUpdateElement_SolidLabel(d); },
					[&](const remoteprotocol::ElementDesc_ImageFrame& d) { RequestRendererUpdateElement_ImageFrame(d); }
				));
			}
		}
	}

	void GuiRemoteRendererSingle::RequestRendererEndRendering(vint id)
	{
		events->RespondRendererEndRendering(id, elementMeasurings);
		elementMeasurings = {};
		fontHeightMeasurings.Clear();
		supressRefresh = false;
		if (needRefresh)
		{
			needRefresh = false;
			ForceRender();
		}
	}

/***********************************************************************
* Rendering (Dom)
***********************************************************************/

	void GuiRemoteRendererSingle::RequestRendererRenderDom(const Ptr<remoteprotocol::RenderingDom>& arguments)
	{
		renderingDom = arguments;
		if (renderingDom)
		{
			BuildDomIndex(renderingDom, renderingDomIndex);
		}
		needRefresh = true;
	}

	void GuiRemoteRendererSingle::RequestRendererRenderDomDiff(const remoteprotocol::RenderingDom_DiffsInOrder& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererRenderDomDiff(const RenderingDom_DiffsInOrder&)#"
		CHECK_ERROR(renderingDom, ERROR_MESSAGE_PREFIX L"This function must be called after RequestRendererRenderDom.");

		UpdateDomInplace(renderingDom, renderingDomIndex, arguments);
		needRefresh = true;
#undef ERROR_MESSAGE_PREFIX
	}

/***********************************************************************
* Rendering (Commands)
***********************************************************************/

	void GuiRemoteRendererSingle::RequestRendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererBeginBoundary(const ElementBoundary&)#"
		CHECK_FAIL(ERROR_MESSAGE_PREFIX L"The current implementation require dom-diff enabled in core side.");
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteRendererSingle::RequestRendererEndBoundary()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererEndBoundary()#"
		CHECK_FAIL(ERROR_MESSAGE_PREFIX L"The current implementation require dom-diff enabled in core side.");
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteRendererSingle::RequestRendererRenderElement(const remoteprotocol::ElementRendering& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererRenderElement(const ElementRendering&)#"
		CHECK_FAIL(ERROR_MESSAGE_PREFIX L"The current implementation require dom-diff enabled in core side.");
#undef ERROR_MESSAGE_PREFIX
	}

/***********************************************************************
* Rendering (INativeWindow)
***********************************************************************/

	void GuiRemoteRendererSingle::UpdateRenderTarget(elements::IGuiGraphicsRenderTarget* rt)
	{
		for (auto element : availableElements.Values())
		{
			element->GetRenderer()->SetRenderTarget(rt);
		}
	}

	void GuiRemoteRendererSingle::RenderDom(Ptr<remoteprotocol::RenderingDom> dom, elements::IGuiGraphicsRenderTarget* rt)
	{
		if (dom->content.element)
		{
			vint index = availableElements.Keys().IndexOf(dom->content.element.Value());
			if (index != -1)
			{
				auto element = availableElements.Values()[index];
				rt->PushClipper(dom->content.validArea, nullptr);
				element->GetRenderer()->Render(dom->content.bounds);
				rt->PopClipper(nullptr);

				if (auto solidLabel = element.Cast<GuiSolidLabelElement>())
				{
					index = solidLabelMeasurings.Keys().IndexOf(dom->content.element.Value());
					if (index != -1)
					{
						auto& measuring = const_cast<SolidLabelMeasuring&>(solidLabelMeasurings.Values()[index]);
						auto minSize = element->GetRenderer()->GetMinSize();

						bool measuringChanged = false;
						if (!measuring.minSize)
						{
							measuringChanged = true;
						}
						else switch (measuring.request)
						{
						case ElementSolidLabelMeasuringRequest::FontHeight:
							if (measuring.minSize.Value().y != minSize.y)
							{
								measuringChanged = true;
							}
							break;
						case ElementSolidLabelMeasuringRequest::TotalSize:
							if (measuring.minSize.Value() != minSize)
							{
								measuringChanged = true;
							}
							break;
						}

						measuring.minSize = minSize;
						if (measuringChanged)
						{
							StoreLabelMeasuring(dom->content.element.Value(), measuring.request, solidLabel, minSize);
						}
					}
				}
			}
		}

		if (dom->children)
		{
			for (auto child : *dom->children.Obj())
			{
				if (child->content.validArea.Width() > 0 && child->content.validArea.Height()> 0)
				{
					RenderDom(child, rt);
				}
			}
		}
	}

	void GuiRemoteRendererSingle::HitTestInternal(Ptr<remoteprotocol::RenderingDom> dom, Point location, Nullable<INativeWindowListener::HitTestResult>& hitTestResult, Nullable<INativeCursor::SystemCursorType>& cursorType)
	{
		if (dom->children)
		{
			for (auto child : *dom->children.Obj())
			{
				if (child->content.validArea.Contains(location))
				{
					HitTestInternal(child, location, hitTestResult, cursorType);

					if (!hitTestResult && child->content.hitTestResult)
					{
						hitTestResult = child->content.hitTestResult;
					}
					if (!cursorType && child->content.cursor)
					{
						cursorType = child->content.cursor;
					}
				}
			}
		}
	}

	void GuiRemoteRendererSingle::HitTest(Ptr<remoteprotocol::RenderingDom> dom, Point location, INativeWindowListener::HitTestResult& hitTestResult, INativeCursor*& cursor)
	{
		Nullable<INativeWindowListener::HitTestResult> hitTestResultNullable;
		Nullable<INativeCursor::SystemCursorType> cursorTypeNullable;
		HitTestInternal(dom, location, hitTestResultNullable, cursorTypeNullable);
		hitTestResult = hitTestResultNullable ? hitTestResultNullable.Value() : INativeWindowListener::NoDecision;
		cursor = cursorTypeNullable ? GetCurrentController()->ResourceService()->GetSystemCursor(cursorTypeNullable.Value()) : GetCurrentController()->ResourceService()->GetDefaultSystemCursor();
	}

	void GuiRemoteRendererSingle::ForceRender()
	{
		supressPaint = true;
		auto rt = GetGuiGraphicsResourceManager()->GetRenderTarget(window);
		rt->StartRendering();
		RenderDom(renderingDom, rt);
		auto result = rt->StopRendering();
		window->RedrawContent();
		supressPaint = false;

		switch (result)
		{
		case RenderTargetFailure::ResizeWhileRendering:
			GetGuiGraphicsResourceManager()->ResizeRenderTarget(window);
			needRefresh = true;
			break;
		case RenderTargetFailure::LostDevice:
			UpdateRenderTarget(nullptr);
			GetGuiGraphicsResourceManager()->RecreateRenderTarget(window);
			UpdateRenderTarget(GetGuiGraphicsResourceManager()->GetRenderTarget(window));
			needRefresh = true;
			break;
		default:;
		}
	}

	void GuiRemoteRendererSingle::GlobalTimer()
	{
		DateTime now = DateTime::UtcTime();
		if (now.osMilliseconds - lastCaretTime >= CaretInterval)
		{
			lastCaretTime = now.osMilliseconds;
			OnCaretNotify();
		}
		SendAccumulatedMessages();

		// Between any element destroying, element creating, start rendering
		// and end rendering
		// no refreshing is allowed to avoid flashing issue
		if (supressRefresh) return;
		if (!needRefresh) return;
		needRefresh = false;
		if (!window) return;
		if (!renderingDom) return;
		ForceRender();
	}

	void GuiRemoteRendererSingle::Paint()
	{
		if (!supressPaint)
		{
			needRefresh = true;
		}
	}

	INativeWindowListener::HitTestResult GuiRemoteRendererSingle::HitTest(NativePoint location)
	{
		INativeWindowListener::HitTestResult hitTestResult = INativeWindowListener::NoDecision;
		INativeCursor* cursor = nullptr;
		if (renderingDom)
		{
			HitTest(renderingDom, window->Convert(location), hitTestResult, cursor);
		}
		return hitTestResult;
	}
}
