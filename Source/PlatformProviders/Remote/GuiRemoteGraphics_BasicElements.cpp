#include "GuiRemoteGraphics_BasicElements.h"

namespace vl::presentation::elements_remoteprotocol
{
/***********************************************************************
GuiSolidBorderElementRenderer
***********************************************************************/

	template<typename TElement, typename TRenderer>
	void GuiRemoteProtocolElementRenderer<TElement, TRenderer>::InitializeInternal()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	template<typename TElement, typename TRenderer>
	void GuiRemoteProtocolElementRenderer<TElement, TRenderer>::FinalizeInternal()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	template<typename TElement, typename TRenderer>
	void GuiRemoteProtocolElementRenderer<TElement, TRenderer>::RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	template<typename TElement, typename TRenderer>
	void GuiRemoteProtocolElementRenderer<TElement, TRenderer>::Render(Rect bounds)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

/***********************************************************************
GuiSolidBorderElementRenderer
***********************************************************************/

	GuiFocusRectangleElementRenderer::GuiFocusRectangleElementRenderer()
	{
	}

	void GuiFocusRectangleElementRenderer::OnElementStateChanged()
	{
	}

/***********************************************************************
GuiSolidBorderElementRenderer
***********************************************************************/

	GuiSolidBorderElementRenderer::GuiSolidBorderElementRenderer()
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

	Gui3DBorderElementRenderer::Gui3DBorderElementRenderer()
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

	Gui3DSplitterElementRenderer::Gui3DSplitterElementRenderer()
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

	GuiSolidBackgroundElementRenderer::GuiSolidBackgroundElementRenderer()
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

	GuiGradientBackgroundElementRenderer::GuiGradientBackgroundElementRenderer()
	{
	}

	void GuiGradientBackgroundElementRenderer::OnElementStateChanged()
	{
	}

/***********************************************************************
GuiInnerShadowElementRenderer
***********************************************************************/

	GuiInnerShadowElementRenderer::GuiInnerShadowElementRenderer()
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

	GuiSolidLabelElementRenderer::GuiSolidLabelElementRenderer()
	{
	}

	void GuiSolidLabelElementRenderer::Render(Rect bounds)
	{
		GuiRemoteProtocolElementRenderer<GuiSolidLabelElement, GuiSolidLabelElementRenderer>::Render(bounds);
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

	GuiImageFrameElementRenderer::GuiImageFrameElementRenderer()
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

	GuiPolygonElementRenderer::GuiPolygonElementRenderer()
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
		GuiRemoteProtocolElementRenderer<GuiColorizedTextElement, GuiColorizedTextElementRenderer>::InitializeInternal();
		element->SetCallback(this);
	}

	void GuiColorizedTextElementRenderer::FinalizeInternal()
	{
		element->SetCallback(nullptr);
		GuiRemoteProtocolElementRenderer<GuiColorizedTextElement, GuiColorizedTextElementRenderer>::FinalizeInternal();
	}

	GuiColorizedTextElementRenderer::GuiColorizedTextElementRenderer()
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