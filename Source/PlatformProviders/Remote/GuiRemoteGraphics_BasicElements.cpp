#include "GuiRemoteGraphics_BasicElements.h"

namespace vl::presentation::elements_remoteprotocol
{
/***********************************************************************
GuiSolidBorderElementRenderer
***********************************************************************/

	void GuiFocusRectangleElementRenderer::InitializeInternal()
	{
	}

	void GuiFocusRectangleElementRenderer::FinalizeInternal()
	{
	}

	void GuiFocusRectangleElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	void GuiFocusRectangleElementRenderer::Render(Rect bounds)
	{
	}

	void GuiFocusRectangleElementRenderer::OnElementStateChanged()
	{
	}

/***********************************************************************
GuiSolidBorderElementRenderer
***********************************************************************/

	void GuiSolidBorderElementRenderer::InitializeInternal()
	{
	}

	void GuiSolidBorderElementRenderer::FinalizeInternal()
	{
	}

	void GuiSolidBorderElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	void GuiSolidBorderElementRenderer::Render(Rect bounds)
	{
	}

	void GuiSolidBorderElementRenderer::OnElementStateChanged()
	{
		// Color
		// Shape
	}

/***********************************************************************
Gui3DBorderElementRenderer
***********************************************************************/

	void Gui3DBorderElementRenderer::InitializeInternal()
	{
	}

	void Gui3DBorderElementRenderer::FinalizeInternal()
	{
	}

	void Gui3DBorderElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	void Gui3DBorderElementRenderer::Render(Rect bounds)
	{
	}

	void Gui3DBorderElementRenderer::OnElementStateChanged()
	{
		// Color1
		// Color2
	}

/***********************************************************************
Gui3DSplitterElementRenderer
***********************************************************************/

	void Gui3DSplitterElementRenderer::InitializeInternal()
	{
	}

	void Gui3DSplitterElementRenderer::FinalizeInternal()
	{
	}

	void Gui3DSplitterElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	void Gui3DSplitterElementRenderer::Render(Rect bounds)
	{
	}

	void Gui3DSplitterElementRenderer::OnElementStateChanged()
	{
		// Color1
		// Color2
		// Direction
	}

/***********************************************************************
GuiSolidBackgroundElementRenderer
***********************************************************************/

	void GuiSolidBackgroundElementRenderer::InitializeInternal()
	{
	}

	void GuiSolidBackgroundElementRenderer::FinalizeInternal()
	{
	}

	void GuiSolidBackgroundElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	void GuiSolidBackgroundElementRenderer::Render(Rect bounds)
	{
	}

	void GuiSolidBackgroundElementRenderer::OnElementStateChanged()
	{
		// Color
		// Shape
	}

/***********************************************************************
GuiGradientBackgroundElementRenderer
***********************************************************************/

	void GuiGradientBackgroundElementRenderer::InitializeInternal()
	{
	}

	void GuiGradientBackgroundElementRenderer::FinalizeInternal()
	{
	}

	void GuiGradientBackgroundElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	void GuiGradientBackgroundElementRenderer::Render(Rect bounds)
	{
		// Color1
		// Color2
		// Direction
		// Shape
	}

	void GuiGradientBackgroundElementRenderer::OnElementStateChanged()
	{
	}

/***********************************************************************
GuiSolidLabelElementRenderer
***********************************************************************/

	void GuiInnerShadowElementRenderer::InitializeInternal()
	{
	}

	void GuiInnerShadowElementRenderer::FinalizeInternal()
	{
	}

	void GuiInnerShadowElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	GuiInnerShadowElementRenderer::GuiInnerShadowElementRenderer()
	{
	}

	void GuiInnerShadowElementRenderer::Render(Rect bounds)
	{
	}

	void GuiInnerShadowElementRenderer::OnElementStateChanged()
	{
		// Color
		// Thickness
	}

/***********************************************************************
GuiSolidLabelElementRenderer
***********************************************************************/

	void GuiSolidLabelElementRenderer::InitializeInternal()
	{
	}

	void GuiSolidLabelElementRenderer::FinalizeInternal()
	{
	}

	void GuiSolidLabelElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	GuiSolidLabelElementRenderer::GuiSolidLabelElementRenderer()
	{
	}

	void GuiSolidLabelElementRenderer::Render(Rect bounds)
	{
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
		// Color
		// Font
		// Text
		// HorizontalAlignment
		// VerticalAlignment
		// WrapLine
		// Ellipse
		// Multiline
		// WrapLineHeightCalculation
	}

/***********************************************************************
GuiImageFrameElementRenderer
***********************************************************************/

	void GuiImageFrameElementRenderer::InitializeInternal()
	{
	}

	void GuiImageFrameElementRenderer::FinalizeInternal()
	{
	}

	void GuiImageFrameElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	GuiImageFrameElementRenderer::GuiImageFrameElementRenderer()
	{
	}

	void GuiImageFrameElementRenderer::Render(Rect bounds)
	{
	}

	void GuiImageFrameElementRenderer::OnElementStateChanged()
	{
		// Image
		// FrameIndex
		// HorizontalAlignment
		// VerticalAlignment
		// Stretch
		// Enabled
		// UpdateMinSize(Stretch ? {0,0} : frame->GetSize())
	}

/***********************************************************************
GuiPolygonElementRenderer
***********************************************************************/

	void GuiPolygonElementRenderer::InitializeInternal()
	{
	}

	void GuiPolygonElementRenderer::FinalizeInternal()
	{
	}

	void GuiPolygonElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	GuiPolygonElementRenderer::GuiPolygonElementRenderer()
	{
	}

	GuiPolygonElementRenderer::~GuiPolygonElementRenderer()
	{
	}

	void GuiPolygonElementRenderer::Render(Rect bounds)
	{
	}

	void GuiPolygonElementRenderer::OnElementStateChanged()
	{
		// Size
		// Points
		// BorderColor
		// BackgroundColor
		// UpdateMinSize(element->GetSize())
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
		element->SetCallback(this);
	}

	void GuiColorizedTextElementRenderer::FinalizeInternal()
	{
		element->SetCallback(nullptr);
	}

	void GuiColorizedTextElementRenderer::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
	}

	void GuiColorizedTextElementRenderer::Render(Rect bounds)
	{
	}

	void GuiColorizedTextElementRenderer::OnElementStateChanged()
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
	}
}