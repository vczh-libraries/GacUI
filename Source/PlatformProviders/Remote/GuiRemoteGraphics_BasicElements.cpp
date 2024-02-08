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
	IGuiGraphicsRenderer* GuiRemoteProtocolElementRenderer<TElement, TRenderer>::GetRenderer()
	{
		return this;
	}

	template<typename TElement, typename TRenderer>
	vint GuiRemoteProtocolElementRenderer<TElement, TRenderer>::GetID()
	{
		return id;
	}

	template<typename TElement, typename TRenderer>
	bool GuiRemoteProtocolElementRenderer<TElement, TRenderer>::IsUpdated()
	{
		return updated;
	}

	template<typename TElement, typename TRenderer>
	void GuiRemoteProtocolElementRenderer<TElement, TRenderer>::SetUpdated()
	{
		updated = true;
	}

	template<typename TElement, typename TRenderer>
	void GuiRemoteProtocolElementRenderer<TElement, TRenderer>::ResetUpdated()
	{
		updated = false;
	}

	template<typename TElement, typename TRenderer>
	void GuiRemoteProtocolElementRenderer<TElement, TRenderer>::Render(Rect bounds)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	template<typename TElement, typename TRenderer>
	void GuiRemoteProtocolElementRenderer<TElement, TRenderer>::OnElementStateChanged()
	{
		SetUpdated();
	}

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