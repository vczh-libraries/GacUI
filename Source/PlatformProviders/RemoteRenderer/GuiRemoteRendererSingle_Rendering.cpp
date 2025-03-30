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

	void GuiRemoteRendererSingle::RequestRendererDestroyed(const Ptr<collections::List<vint>>& arguments)
	{
		if (arguments)
		{
			for (auto id : *arguments.Obj())
			{
				availableElements.Remove(id);
			}
		}
	}

	void GuiRemoteRendererSingle::RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments)
	{
		elementMeasurings = {};
	}

	void GuiRemoteRendererSingle::RequestRendererEndRendering(vint id)
	{
		events->RespondRendererEndRendering(id, elementMeasurings);
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
			switch (arguments.measuringRequest.Value())
			{
			case ElementSolidLabelMeasuringRequest::FontHeight:
				CHECK_FAIL(L"Not Implemented!");
				break;
			case ElementSolidLabelMeasuringRequest::TotalSize:
				CHECK_FAIL(L"Not Implemented!");
				break;
			}
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
* Rendering (Commands)
***********************************************************************/

	void GuiRemoteRendererSingle::Render(Ptr<remoteprotocol::RenderingDom> dom, elements::IGuiGraphicsRenderTarget* rt)
	{
		if (dom->content.validArea.Width() <= 0 || dom->content.validArea.Height() <= 0)
		{
			return;
		}

		if (dom->content.element)
		{
			vint index = availableElements.Keys().IndexOf(dom->content.element.Value());
			if (index != -1)
			{
				auto element = availableElements.Values()[index];
				if (auto renderer = element->GetRenderer())
				{
					rt->PushClipper(dom->content.validArea, nullptr);
					renderer->Render(dom->content.bounds);
					rt->PopClipper(nullptr);
				}
			}
		}

		if (dom->children)
		{
			for (auto child : *dom->children.Obj())
			{
				Render(child, rt);
			}
		}
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
		supressPaint = false;

		switch (result)
		{
		case RenderTargetFailure::ResizeWhileRendering:
			GetGuiGraphicsResourceManager()->ResizeRenderTarget(window);
			needRefresh = true;
			break;
		case RenderTargetFailure::LostDevice:
			GetGuiGraphicsResourceManager()->RecreateRenderTarget(window);
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
}