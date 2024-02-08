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
	void RENDERER_CLASS_TYPE::Render(Rect bounds)
	{
		remoteprotocol::ElementRendering arguments;
		arguments.id = id;
		arguments.bounds = bounds;
		arguments.clipper = this->renderTarget->GetClipperValidArea();
		this->renderTarget->GetRemoteMessages().RequestRendererRenderElement(arguments);
	}

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::OnElementStateChanged()
	{
		updated = true;
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

	void GuiFocusRectangleElementRenderer::SendUpdateElementMessages()
	{
		// nothing to update
	}

/***********************************************************************
GuiSolidBorderElementRenderer
***********************************************************************/

	GuiSolidBorderElementRenderer::GuiSolidBorderElementRenderer()
	{
	}

	void GuiSolidBorderElementRenderer::SendUpdateElementMessages()
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

	void Gui3DBorderElementRenderer::SendUpdateElementMessages()
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

	void Gui3DSplitterElementRenderer::SendUpdateElementMessages()
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

	void GuiSolidBackgroundElementRenderer::SendUpdateElementMessages()
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

	void GuiGradientBackgroundElementRenderer::SendUpdateElementMessages()
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

	void GuiInnerShadowElementRenderer::SendUpdateElementMessages()
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

	GuiSolidLabelElementRenderer::GuiSolidLabelElementRenderer()
	{
	}

	void GuiSolidLabelElementRenderer::Render(Rect bounds)
	{
		TBase::Render(bounds);
		// UpdateMinSize()
		// When text is empty, total size is the size of one space character
		//   WrapLine == true:
		//     WrapLineHeightCalculation == true -> calculate total size remotely
		//     WrapLineHeightCalculation == false -> {0,0}
		//   WrapLine == false:
		//     Ellipse == true -> {0,calculate line height remotely}
		//     Ellipse == false -> calculate total size remotely
	}

	void GuiSolidLabelElementRenderer::OnElementStateChanged()
	{
		TBase::OnElementStateChanged();
	}

	void GuiSolidLabelElementRenderer::SendUpdateElementMessages()
	{
		// Color
		// Font
		// Text
		// HorizontalAlignment
		// VerticalAlignment
		// WrapLine
		// Ellipse
		// Multiline
		// WrapLineHeightCalculation
		CHECK_FAIL(L"Not Implemented!");
	}

/***********************************************************************
GuiImageFrameElementRenderer
***********************************************************************/

	GuiImageFrameElementRenderer::GuiImageFrameElementRenderer()
	{
	}

	void GuiImageFrameElementRenderer::SendUpdateElementMessages()
	{
		// Image
		// FrameIndex
		// HorizontalAlignment
		// VerticalAlignment
		// Stretch
		// Enabled
		// UpdateMinSize(Stretch ? {0,0} : frame->GetSize())
		CHECK_FAIL(L"Not Implemented!");
	}

/***********************************************************************
GuiPolygonElementRenderer
***********************************************************************/

	GuiPolygonElementRenderer::GuiPolygonElementRenderer()
	{
	}

	void GuiPolygonElementRenderer::SendUpdateElementMessages()
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

	void GuiColorizedTextElementRenderer::SendUpdateElementMessages()
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