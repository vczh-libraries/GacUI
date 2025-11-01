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
		if (arguments)
		{
			for (auto id : *arguments.Obj())
			{
				availableElements.Remove(id);
				solidLabelMeasurings.Remove(id);
			}
		}
	}

	void GuiRemoteRendererSingle::RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererEndRendering(vint id)
	{
		events->RespondRendererEndRendering(id, elementMeasurings);
		elementMeasurings = {};
		fontHeightMeasurings.Clear();
	}

/***********************************************************************
* Rendering (Elemnents)
***********************************************************************/

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SolidBorder(const remoteprotocol::ElementDesc_SolidBorder& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiSolidBorderElement>();
		if (!element) return;

		element->SetColor(arguments.borderColor);
		element->SetShape(arguments.shape);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SinkBorder(const remoteprotocol::ElementDesc_SinkBorder& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<Gui3DBorderElement>();
		if (!element) return;

		element->SetColors(arguments.leftTopColor, arguments.rightBottomColor);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SinkSplitter(const remoteprotocol::ElementDesc_SinkSplitter& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<Gui3DSplitterElement>();
		if (!element) return;

		element->SetColors(arguments.leftTopColor, arguments.rightBottomColor);
		element->SetDirection(arguments.direction);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SolidBackground(const remoteprotocol::ElementDesc_SolidBackground& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiSolidBackgroundElement>();
		if (!element) return;

		element->SetColor(arguments.backgroundColor);
		element->SetShape(arguments.shape);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_GradientBackground(const remoteprotocol::ElementDesc_GradientBackground& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiGradientBackgroundElement>();
		if (!element) return;

		element->SetColors(arguments.leftTopColor, arguments.rightBottomColor);
		element->SetDirection(arguments.direction);
		element->SetShape(arguments.shape);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_InnerShadow(const remoteprotocol::ElementDesc_InnerShadow& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiInnerShadowElement>();
		if (!element) return;

		element->SetColor(arguments.shadowColor);
		element->SetThickness(arguments.thickness);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_Polygon(const remoteprotocol::ElementDesc_Polygon& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiPolygonElement>();
		if (!element) return;

		element->SetSize(arguments.size);
		element->SetBorderColor(arguments.borderColor);
		element->SetBackgroundColor(arguments.backgroundColor);

		if (arguments.points && arguments.points->Count() > 0)
		{
			element->SetPoints(&arguments.points->Get(0), arguments.points->Count());
		}
	}

/***********************************************************************
* Rendering (Elemnents -- Label)
***********************************************************************/

	void GuiRemoteRendererSingle::StoreLabelMeasuring(vint id, remoteprotocol::ElementSolidLabelMeasuringRequest request, Ptr<elements::GuiSolidLabelElement> solidLabel, Size minSize)
	{
		switch (request)
		{
		case ElementSolidLabelMeasuringRequest::FontHeight:
			{
				Pair<WString, vint> key = { solidLabel->GetFont().fontFamily,solidLabel->GetFont().size };
				if (fontHeightMeasurings.Contains(key)) return;
				fontHeightMeasurings.Add(key);

				ElementMeasuring_FontHeight response;
				response.fontFamily = key.key;
				response.fontSize = key.value;
				response.height = minSize.y;

				if (!elementMeasurings.fontHeights)
				{
					elementMeasurings.fontHeights = Ptr(new List<ElementMeasuring_FontHeight>);
				}
				elementMeasurings.fontHeights->Add(response);
			}
			break;
		case ElementSolidLabelMeasuringRequest::TotalSize:
			{
				ElementMeasuring_ElementMinSize response;
				response.id = id;
				response.minSize = minSize;

				if (!elementMeasurings.minSizes)
				{
					elementMeasurings.minSizes = Ptr(new List<ElementMeasuring_ElementMinSize>);
				}
				elementMeasurings.minSizes->Add(response);
			}
			break;
		}
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SolidLabel(const remoteprotocol::ElementDesc_SolidLabel& arguments)
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiSolidLabelElement>();
		if (!element) return;

		element->SetColor(arguments.textColor);
		element->SetAlignments(GetAlignment(arguments.horizontalAlignment), GetAlignment(arguments.verticalAlignment));
		element->SetWrapLine(arguments.wrapLine);
		element->SetWrapLineHeightCalculation(arguments.wrapLineHeightCalculation);
		element->SetEllipse(arguments.ellipse);
		element->SetMultiline(arguments.multiline);

		if (arguments.font)
		{
			element->SetFont(arguments.font.Value());
		}
		if (arguments.text)
		{
			element->SetText(arguments.text.Value());
		}

		if (arguments.measuringRequest)
		{
			SolidLabelMeasuring measuring;
			measuring.request = arguments.measuringRequest.Value();
			index = solidLabelMeasurings.Keys().IndexOf(arguments.id);
			if (solidLabelMeasurings.Keys().Contains(arguments.id))
			{
				solidLabelMeasurings.Set(arguments.id, measuring);
			}
			else
			{
				solidLabelMeasurings.Add(arguments.id, measuring);
			}

			StoreLabelMeasuring(arguments.id, measuring.request, element, element->GetRenderer()->GetMinSize());
		}
	}

/***********************************************************************
* Rendering (Elements -- Image)
***********************************************************************/

	remoteprotocol::ImageMetadata GuiRemoteRendererSingle::CreateImageMetadata(vint id, INativeImage* image)
	{
		ImageMetadata response;
		response.id = id;
		response.format = image->GetFormat();
		response.frames = Ptr(new List<ImageFrameMetadata>);
		for (vint i = 0; i < image->GetFrameCount(); i++)
		{
			auto frame = image->GetFrame(i);
			response.frames->Add({ frame->GetSize() });
		}

		return response;
	}

	remoteprotocol::ImageMetadata GuiRemoteRendererSingle::CreateImage(const remoteprotocol::ImageCreation& arguments)
	{
		arguments.imageData->SeekFromBegin(0);
		auto image = GetCurrentController()->ImageService()->CreateImageFromStream(*arguments.imageData.Obj());
		if (availableImages.Keys().Contains(arguments.id))
		{
			availableImages.Set(arguments.id, image);
		}
		else
		{
			availableImages.Add(arguments.id, image);
		}
		return CreateImageMetadata(arguments.id, image.Obj());
	}
	
	void GuiRemoteRendererSingle::RequestImageCreated(vint id, const remoteprotocol::ImageCreation& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestImageCreated(const ImageCreation&)#"
		CHECK_ERROR(!arguments.imageDataOmitted && arguments.imageData, ERROR_MESSAGE_PREFIX L"Binary content of the image is missing.");

		events->RespondImageCreated(id, CreateImage(arguments));
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteRendererSingle::RequestImageDestroyed(const vint& arguments)
	{
		availableImages.Remove(arguments);
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_ImageFrame(const remoteprotocol::ElementDesc_ImageFrame& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererUpdateElement_ImageFrame(const arguments&)#"

		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index == -1) return;
		auto element = availableElements.Values()[index].Cast<GuiImageFrameElement>();
		if (!element) return;

		element->SetAlignments(GetAlignment(arguments.horizontalAlignment), GetAlignment(arguments.verticalAlignment));
		element->SetStretch(arguments.stretch);
		element->SetEnabled(arguments.enabled);

		if (arguments.imageId && arguments.imageCreation)
		{
			CHECK_ERROR(arguments.imageId.Value() == arguments.imageCreation.Value().id, ERROR_MESSAGE_PREFIX L"imageId and imageCreation.id must be identical.");
		}

		if (arguments.imageId)
		{
			if (arguments.imageCreation && !elementMeasurings.createdImages)
			{
				elementMeasurings.createdImages = Ptr(new List<ImageMetadata>);
			}

			vint index = availableImages.Keys().IndexOf(arguments.imageId.Value());
			if (index == -1)
			{
				CHECK_ERROR(arguments.imageCreation && !arguments.imageCreation.Value().imageDataOmitted && arguments.imageCreation.Value().imageData, ERROR_MESSAGE_PREFIX L"Binary content of the image is missing.");

				auto response = CreateImage(arguments.imageCreation.Value());
				element->SetImage(availableImages[response.id], arguments.imageFrame);
				elementMeasurings.createdImages->Add(response);
			}
			else
			{
				auto image = availableImages.Values()[index];
				element->SetImage(image, arguments.imageFrame);
				if (arguments.imageCreation)
				{
					elementMeasurings.createdImages->Add(CreateImageMetadata(arguments.imageId.Value(), image.Obj()));
				}
			}
		}
#undef ERROR_MESSAGE_PREFIX
	}

/***********************************************************************
* Rendering (Elements -- Document)
***********************************************************************/

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph(vint id, const remoteprotocol::ElementDesc_DocumentParagraph& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetCaret(vint id, const remoteprotocol::GetCaretRequest& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetCaretBounds(vint id, const remoteprotocol::GetCaretBoundsRequest& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetInlineObjectFromPoint(vint id, const Point& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetNearestCaretFromTextPos(vint id, const remoteprotocol::GetCaretBoundsRequest& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_IsValidCaret(vint id, const vint& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_OpenCaret(const remoteprotocol::OpenCaretRequest& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_CloseCaret()
	{
		CHECK_FAIL(L"Not implemented.");
	}

/***********************************************************************
* Rendering (Dom)
***********************************************************************/

	void GuiRemoteRendererSingle::CheckDom()
	{
		needRefresh = true;
	}

	void GuiRemoteRendererSingle::RequestRendererRenderDom(const Ptr<remoteprotocol::RenderingDom>& arguments)
	{
		renderingDom = arguments;
		if (renderingDom)
		{
			BuildDomIndex(renderingDom, renderingDomIndex);
		}
		CheckDom();
	}

	void GuiRemoteRendererSingle::RequestRendererRenderDomDiff(const remoteprotocol::RenderingDom_DiffsInOrder& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererRenderDomDiff(const RenderingDom_DiffsInOrder&)#"
		CHECK_ERROR(renderingDom, ERROR_MESSAGE_PREFIX L"This function must be called after RequestRendererRenderDom.");

		UpdateDomInplace(renderingDom, renderingDomIndex, arguments);
		CheckDom();
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

	void GuiRemoteRendererSingle::Render(Ptr<remoteprotocol::RenderingDom> dom, elements::IGuiGraphicsRenderTarget* rt)
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
					Render(child, rt);
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

	void GuiRemoteRendererSingle::GlobalTimer()
	{
		if (!needRefresh) return;
		needRefresh = false;
		if (!window) return;
		if (!renderingDom) return;

		supressPaint = true;
		auto rt = GetGuiGraphicsResourceManager()->GetRenderTarget(window);
		rt->StartRendering();
		Render(renderingDom, rt);
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