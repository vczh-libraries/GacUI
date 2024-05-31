#include "GuiRemoteGraphics_BasicElements.h"
#include "GuiRemoteController.h"

namespace vl::presentation::elements_remoteprotocol
{
	using namespace collections;
	using namespace remoteprotocol;

/***********************************************************************
GuiSolidBorderElementRenderer
***********************************************************************/

#define RENDERER_TEMPLATE_HEADER	template<typename TElement, typename TRenderer, remoteprotocol::RendererType _RendererType>
#define RENDERER_CLASS_TYPE			GuiRemoteProtocolElementRenderer<TElement, TRenderer, _RendererType>

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::InitializeInternal()
	{
	}

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::FinalizeInternal()
	{
		if (this->renderTarget && id != -1)
		{
			this->renderTarget->UnregisterRenderer(this);
			id = -1;
		}
	}

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
		if (oldRenderTarget == newRenderTarget) return;
		if (oldRenderTarget && id != -1)
		{
			oldRenderTarget->UnregisterRenderer(this);
			id = -1;
		}
		if (newRenderTarget)
		{
			id = newRenderTarget->AllocateNewElementId();
			newRenderTarget->RegisterRenderer(this);
		}
	}

	RENDERER_TEMPLATE_HEADER
	IGuiGraphicsRenderer* RENDERER_CLASS_TYPE::GetRenderer()
	{
		return this;
	}

	RENDERER_TEMPLATE_HEADER
	vint RENDERER_CLASS_TYPE::GetID()
	{
		return id;
	}

	RENDERER_TEMPLATE_HEADER
	remoteprotocol::RendererType RENDERER_CLASS_TYPE::GetRendererType()
	{
		return _RendererType;
	}

	RENDERER_TEMPLATE_HEADER
	bool RENDERER_CLASS_TYPE::IsUpdated()
	{
		return updated;
	}

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::ResetUpdated()
	{
		updated = false;
	}

	RENDERER_TEMPLATE_HEADER
	bool RENDERER_CLASS_TYPE::IsRenderedInLastBatch()
	{
		return this->renderTarget && this->renderTarget->renderingBatchId == renderingBatchId;
	}

	RENDERER_TEMPLATE_HEADER
	bool RENDERER_CLASS_TYPE::NeedUpdateMinSizeFromCache()
	{
		return false;
	}

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::TryFetchMinSizeFromCache()
	{
		CHECK_FAIL(L"vl::presentation::elements_remoteprotocol::GuiRemoteProtocolElementRenderer<TElement, TRenderer, RendererType>::TryUpdateFromCache()#This function should not be called.");
	}

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::UpdateMinSize(Size size)
	{
		CHECK_FAIL(L"vl::presentation::elements_remoteprotocol::GuiRemoteProtocolElementRenderer<TElement, TRenderer, RendererType>::UpdateMinSize(Size)#This function should not be called.");
	}

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::NotifyMinSizeCacheInvalidated()
	{
	}

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::Render(Rect bounds)
	{
		remoteprotocol::ElementRendering arguments;
		arguments.id = id;
		arguments.bounds = bounds;
		arguments.areaClippedByParent = this->renderTarget->GetClipperValidArea();
		this->renderTarget->GetRemoteMessages().RequestRendererRenderElement(arguments);
		renderingBatchId = this->renderTarget->renderingBatchId;
	}

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::OnElementStateChanged()
	{
		updated = true;
	}


#undef RENDERER_CLASS_TYPE
#undef RENDERER_TEMPLATE_HEADER

/***********************************************************************
GuiSolidBorderElementRenderer
***********************************************************************/

	GuiFocusRectangleElementRenderer::GuiFocusRectangleElementRenderer()
	{
	}
	
	bool GuiFocusRectangleElementRenderer::IsUpdated()
	{
		// there is no properties for this element
		return false;
	}

	void GuiFocusRectangleElementRenderer::ResetUpdated()
	{
		// nothing to update
	}

	void GuiFocusRectangleElementRenderer::OnElementStateChanged()
	{
		// nothing to update
	}

	void GuiFocusRectangleElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		// nothing to update
	}

/***********************************************************************
GuiSolidBorderElementRenderer
***********************************************************************/

	GuiSolidBorderElementRenderer::GuiSolidBorderElementRenderer()
	{
	}

	void GuiSolidBorderElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		ElementDesc_SolidBorder arguments;
		arguments.id = id;
		arguments.borderColor = element->GetColor();
		arguments.shape = element->GetShape();
		renderTarget->GetRemoteMessages().RequestRendererUpdateElement_SolidBorder(arguments);
	}

/***********************************************************************
Gui3DBorderElementRenderer
***********************************************************************/

	Gui3DBorderElementRenderer::Gui3DBorderElementRenderer()
	{
	}

	void Gui3DBorderElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		ElementDesc_SinkBorder arguments;
		arguments.id = id;
		arguments.leftTopColor = element->GetColor1();
		arguments.rightBottomColor = element->GetColor2();
		renderTarget->GetRemoteMessages().RequestRendererUpdateElement_SinkBorder(arguments);
	}

/***********************************************************************
Gui3DSplitterElementRenderer
***********************************************************************/

	Gui3DSplitterElementRenderer::Gui3DSplitterElementRenderer()
	{
	}

	void Gui3DSplitterElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		ElementDesc_SinkSplitter arguments;
		arguments.id = id;
		arguments.leftTopColor = element->GetColor1();
		arguments.rightBottomColor = element->GetColor2();
		arguments.direction = element->GetDirection();
		renderTarget->GetRemoteMessages().RequestRendererUpdateElement_SinkSplitter(arguments);
	}

/***********************************************************************
GuiSolidBackgroundElementRenderer
***********************************************************************/

	GuiSolidBackgroundElementRenderer::GuiSolidBackgroundElementRenderer()
	{
	}

	void GuiSolidBackgroundElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		ElementDesc_SolidBackground arguments;
		arguments.id = id;
		arguments.backgroundColor = element->GetColor();
		arguments.shape = element->GetShape();
		renderTarget->GetRemoteMessages().RequestRendererUpdateElement_SolidBackground(arguments);
	}

/***********************************************************************
GuiGradientBackgroundElementRenderer
***********************************************************************/

	GuiGradientBackgroundElementRenderer::GuiGradientBackgroundElementRenderer()
	{
	}

	void GuiGradientBackgroundElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		ElementDesc_GradientBackground arguments;
		arguments.id = id;
		arguments.leftTopColor = element->GetColor1();
		arguments.rightBottomColor = element->GetColor2();
		arguments.direction = element->GetDirection();
		arguments.shape = element->GetShape();
		renderTarget->GetRemoteMessages().RequestRendererUpdateElement_GradientBackground(arguments);
	}

/***********************************************************************
GuiInnerShadowElementRenderer
***********************************************************************/

	GuiInnerShadowElementRenderer::GuiInnerShadowElementRenderer()
	{
	}

	void GuiInnerShadowElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		ElementDesc_InnerShadow arguments;
		arguments.id = id;
		arguments.shadowColor = element->GetColor();
		arguments.thickness = element->GetThickness();
		renderTarget->GetRemoteMessages().RequestRendererUpdateElement_InnerShadow(arguments);
	}

/***********************************************************************
GuiSolidLabelElementRenderer
***********************************************************************/

	GuiSolidLabelElementRenderer::MeasuringRequest GuiSolidLabelElementRenderer::GetMeasuringRequest()
	{
		if (element->GetEllipse())
		{
			return ElementSolidLabelMeasuringRequest::FontHeight;
		}
		else if (element->GetWrapLine() && !element->GetWrapLineHeightCalculation())
		{
			return {};
		}
		else
		{
			return ElementSolidLabelMeasuringRequest::TotalSize;
		}
	}

	bool GuiSolidLabelElementRenderer::IsNeedFontHeight(MeasuringRequest request)
	{
		return request && request.Value() == ElementSolidLabelMeasuringRequest::FontHeight;
	}

	GuiSolidLabelElementRenderer::GuiSolidLabelElementRenderer()
	{
		minSize = { 1,1 };
	}

	bool GuiSolidLabelElementRenderer::NeedUpdateMinSizeFromCache()
	{
		return needFontHeight;
	}

	void GuiSolidLabelElementRenderer::TryFetchMinSizeFromCache()
	{
		if (needFontHeight)
		{
			vint index = renderTarget->fontHeights.Keys().IndexOf({ lastFont.fontFamily,lastFont.size });
			if (index != -1)
			{
				needFontHeight = false;
				vint size = renderTarget->fontHeights.Values()[index];
				UpdateMinSize({ size,size });
			}
		}
	}

	void GuiSolidLabelElementRenderer::UpdateMinSize(Size size)
	{
		minSize = size;
		if (minSize.x < 1)minSize.x = 1;
		if (minSize.y < 1)minSize.y = 1;
	}

	void GuiSolidLabelElementRenderer::NotifyMinSizeCacheInvalidated()
	{
		OnElementStateChanged();
		auto request = GetMeasuringRequest();
		needFontHeight = IsNeedFontHeight(request);
	}

	void GuiSolidLabelElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		ElementDesc_SolidLabel arguments;
		arguments.id = id;
		arguments.textColor = element->GetColor();
		arguments.wrapLine = element->GetWrapLine();
		arguments.wrapLineHeightCalculation = element->GetWrapLineHeightCalculation();
		arguments.ellipse = element->GetEllipse();
		arguments.multiline = element->GetMultiline();

		switch (element->GetHorizontalAlignment())
		{
		case Alignment::Left:
			arguments.horizontalAlignment = ElementHorizontalAlignment::Left;
			break;
		case Alignment::Right:
			arguments.horizontalAlignment = ElementHorizontalAlignment::Right;
			break;
		default:
			arguments.horizontalAlignment = ElementHorizontalAlignment::Center;
		}

		switch (element->GetVerticalAlignment())
		{
		case Alignment::Top:
			arguments.verticalAlignment = ElementVerticalAlignment::Top;
			break;
		case Alignment::Bottom:
			arguments.verticalAlignment = ElementVerticalAlignment::Bottom;
			break;
		default:
			arguments.verticalAlignment = ElementVerticalAlignment::Center;
		}

		auto elementFont = element->GetFont();
		auto elementText = element->GetText();
		if (elementFont.fontFamily == WString::Empty)
		{
			elementFont = GetCurrentController()->ResourceService()->GetDefaultFont();
		}

		if (fullContent || lastFont != elementFont)
		{
			arguments.font = elementFont;
		}

		if (fullContent || lastText != elementText)
		{
			arguments.text = elementText;
		}

		lastFont = elementFont;
		lastText = elementText;
		arguments.measuringRequest = GetMeasuringRequest();
		if ((needFontHeight = IsNeedFontHeight(arguments.measuringRequest)))
		{
			TryFetchMinSizeFromCache();
			if (!needFontHeight)
			{
				arguments.measuringRequest = {};
			}
		}

		renderTarget->GetRemoteMessages().RequestRendererUpdateElement_SolidLabel(arguments);
	}

/***********************************************************************
GuiImageFrameElementRenderer
***********************************************************************/

	GuiRemoteGraphicsImage* GuiImageFrameElementRenderer::GetRemoteImage()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements_remoteprotocol::GuiImageFrameElementRenderer::GetRemoteImage()#"
		if (element && element->GetImage())
		{
			auto image = dynamic_cast<GuiRemoteGraphicsImage*>(element->GetImage().Obj());
			CHECK_ERROR(image, ERROR_MESSAGE_PREFIX L"Only INativeImage that created from GetCurrentController()->ImageService() is supported.");
			return image;
		}
		return nullptr;
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiImageFrameElementRenderer::UpdateMinSizeFromImage(GuiRemoteGraphicsImage* image)
	{
		needUpdateSize = true;
		if (!image || element->GetStretch())
		{
			minSize = { 0,0 };
			needUpdateSize = false;
		}
		else if (image->status == GuiRemoteGraphicsImage::MetadataStatus::Retrived)
		{
			if (0 <= element->GetFrameIndex() && element->GetFrameIndex() < image->GetFrameCount())
			{
				minSize = image->GetFrame(element->GetFrameIndex())->GetSize();
			}
			else
			{
				minSize = { 0,0 };
			}
			needUpdateSize = false;
		}
	}

	GuiImageFrameElementRenderer::GuiImageFrameElementRenderer()
	{
	}

	bool GuiImageFrameElementRenderer::NeedUpdateMinSizeFromCache()
	{
		return needUpdateSize;
	}

	void GuiImageFrameElementRenderer::TryFetchMinSizeFromCache()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements_remoteprotocol::GuiImageFrameElementRenderer::TryFetchMinSizeFromCache()#"
		auto image = GetRemoteImage();
		if (image)
		{
			CHECK_ERROR(image->status == GuiRemoteGraphicsImage::MetadataStatus::Retrived, ERROR_MESSAGE_PREFIX L"The expected metadata of an image does not exist.");
		}
		UpdateMinSizeFromImage(image);
		needUpdateSize = false;
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiImageFrameElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		auto image = GetRemoteImage();
		if (image)
		{
			needUpdateSize = true;
			if (fullContent && image->status == GuiRemoteGraphicsImage::MetadataStatus::Retrived)
			{
				image->status = GuiRemoteGraphicsImage::MetadataStatus::Uninitialized;
			}
			if (image->status == GuiRemoteGraphicsImage::MetadataStatus::Retrived)
			{
				UpdateMinSizeFromImage(image);
			}
		}

		remoteprotocol::ElementDesc_ImageFrame arguments;
		arguments.id = id;
		if (image) arguments.imageId = image->id;
		arguments.imageFrame = element->GetFrameIndex();
		arguments.stretch = element->GetStretch();
		arguments.enabled = element->GetEnabled();

		switch (element->GetHorizontalAlignment())
		{
		case Alignment::Left:
			arguments.horizontalAlignment = ElementHorizontalAlignment::Left;
			break;
		case Alignment::Right:
			arguments.horizontalAlignment = ElementHorizontalAlignment::Right;
			break;
		default:
			arguments.horizontalAlignment = ElementHorizontalAlignment::Center;
		}

		switch (element->GetVerticalAlignment())
		{
		case Alignment::Top:
			arguments.verticalAlignment = ElementVerticalAlignment::Top;
			break;
		case Alignment::Bottom:
			arguments.verticalAlignment = ElementVerticalAlignment::Bottom;
			break;
		default:
			arguments.verticalAlignment = ElementVerticalAlignment::Center;
		}

		if (needUpdateSize && image)
		{
			arguments.imageCreation = image->GenerateImageCreation();
		}

		renderTarget->GetRemoteMessages().RequestRendererUpdateElement_ImageFrame(arguments);
	}

/***********************************************************************
GuiPolygonElementRenderer
***********************************************************************/

	GuiPolygonElementRenderer::GuiPolygonElementRenderer()
	{
	}

	void GuiPolygonElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		minSize = element->GetSize();

		ElementDesc_Polygon arguments;
		arguments.id = id;
		arguments.size = element->GetSize();
		arguments.borderColor = element->GetBorderColor();
		arguments.backgroundColor = element->GetBackgroundColor();
		arguments.points = Ptr(new List<Point>);
		CopyFrom(*arguments.points.Obj(), element->GetPointsArray());
		renderTarget->GetRemoteMessages().RequestRendererUpdateElement_Polygon(arguments);
	}

/***********************************************************************
GuiColorizedTextElementRenderer
***********************************************************************/

	void GuiColorizedTextElementRenderer::ColorChanged()
	{
	}

	void GuiColorizedTextElementRenderer::FontChanged()
	{
	}

	void GuiColorizedTextElementRenderer::InitializeInternal()
	{
		TBase::InitializeInternal();
		element->SetCallback(this);
	}

	void GuiColorizedTextElementRenderer::FinalizeInternal()
	{
		element->SetCallback(nullptr);
		TBase::FinalizeInternal();
	}

	GuiColorizedTextElementRenderer::GuiColorizedTextElementRenderer()
	{
	}

	void GuiColorizedTextElementRenderer::OnElementStateChanged()
	{
		TBase::OnElementStateChanged();
	}

	void GuiColorizedTextElementRenderer::SendUpdateElementMessages(bool fullContent)
	{
		// Lines
		// Colors
		// Font
		// PasswordChar
		// ViewPosition
		// VisuallyEnabled
		// Focused
		// CaretBegin
		// CaretEnd
		// CaretVisible
		// CaretColor
		CHECK_FAIL(L"Not Implemented!");
	}
}