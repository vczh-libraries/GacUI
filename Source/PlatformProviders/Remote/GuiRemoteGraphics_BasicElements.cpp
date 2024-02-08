#include "GuiRemoteGraphics_BasicElements.h"
#include "GuiRemoteController.h"

namespace vl::presentation::elements_remoteprotocol
{
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
	void RENDERER_CLASS_TYPE::SetUpdated()
	{
		updated = true;
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
		arguments.clipper = this->renderTarget->GetClipper();
		this->renderTarget->GetRemoteMessages().RequestRendererRenderElement(arguments);
	}

	RENDERER_TEMPLATE_HEADER
	void RENDERER_CLASS_TYPE::OnElementStateChanged()
	{
		SetUpdated();
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

	void GuiFocusRectangleElementRenderer::SetUpdated()
	{
		// nothing to update
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
		// Color
		// Shape
	}

/***********************************************************************
Gui3DBorderElementRenderer
***********************************************************************/

	Gui3DBorderElementRenderer::Gui3DBorderElementRenderer()
	{
	}

	void Gui3DBorderElementRenderer::SendUpdateElementMessages()
	{
		// Color1
		// Color2
		CHECK_FAIL(L"Not Implemented!");
	}

/***********************************************************************
Gui3DSplitterElementRenderer
***********************************************************************/

	Gui3DSplitterElementRenderer::Gui3DSplitterElementRenderer()
	{
	}

	void Gui3DSplitterElementRenderer::SendUpdateElementMessages()
	{
		// Color1
		// Color2
		// Direction
		CHECK_FAIL(L"Not Implemented!");
	}

/***********************************************************************
GuiSolidBackgroundElementRenderer
***********************************************************************/

	GuiSolidBackgroundElementRenderer::GuiSolidBackgroundElementRenderer()
	{
	}

	void GuiSolidBackgroundElementRenderer::SendUpdateElementMessages()
	{
		// Color
		// Shape
		CHECK_FAIL(L"Not Implemented!");
	}

/***********************************************************************
GuiGradientBackgroundElementRenderer
***********************************************************************/

	GuiGradientBackgroundElementRenderer::GuiGradientBackgroundElementRenderer()
	{
	}

	void GuiGradientBackgroundElementRenderer::SendUpdateElementMessages()
	{
		// Color1
		// Color2
		// Direction
		// Shape
		CHECK_FAIL(L"Not Implemented!");
	}

/***********************************************************************
GuiInnerShadowElementRenderer
***********************************************************************/

	GuiInnerShadowElementRenderer::GuiInnerShadowElementRenderer()
	{
	}

	void GuiInnerShadowElementRenderer::SendUpdateElementMessages()
	{
		// Color
		// Thickness
		CHECK_FAIL(L"Not Implemented!");
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
		// Size
		// Points
		// BorderColor
		// BackgroundColor
		// UpdateMinSize(element->GetSize())
		CHECK_FAIL(L"Not Implemented!");
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