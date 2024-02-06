/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTEGRAPHICS_BASICELEMENTS
#define VCZH_PRESENTATION_GUIREMOTEGRAPHICS_BASICELEMENTS

#include "../../GraphicsElement/GuiGraphicsTextElement.h"
#include "../../GraphicsElement/GuiGraphicsDocumentElement.h"
#include "GuiRemoteGraphics.h"

namespace vl::presentation::elements_remoteprotocol
{
	using namespace elements;

	template<typename TElement, typename TRenderer>
	class GuiRemoteProtocolElementRenderer : public GuiElementRendererBase<TElement, TRenderer, GuiRemoteGraphicsRenderTarget>
	{
	protected:
		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
	};

	class GuiFocusRectangleElementRenderer : public GuiRemoteProtocolElementRenderer<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer>
	{
		friend class GuiElementRendererBase<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiFocusRectangleElementRenderer();

		void							OnElementStateChanged() override;
	};

	class GuiSolidBorderElementRenderer : public GuiRemoteProtocolElementRenderer<GuiSolidBorderElement, GuiSolidBorderElementRenderer>
	{
		friend class GuiElementRendererBase<GuiSolidBorderElement, GuiSolidBorderElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiSolidBorderElementRenderer();

		void							OnElementStateChanged() override;
	};

	class Gui3DBorderElementRenderer : public GuiRemoteProtocolElementRenderer<Gui3DBorderElement, Gui3DBorderElementRenderer>
	{
		friend class GuiElementRendererBase<Gui3DBorderElement, Gui3DBorderElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		Gui3DBorderElementRenderer();

		void							OnElementStateChanged() override;
	};

	class Gui3DSplitterElementRenderer : public GuiRemoteProtocolElementRenderer<Gui3DSplitterElement, Gui3DSplitterElementRenderer>
	{
		friend class GuiElementRendererBase<Gui3DSplitterElement, Gui3DSplitterElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		Gui3DSplitterElementRenderer();

		void							OnElementStateChanged() override;
	};

	class GuiSolidBackgroundElementRenderer : public GuiRemoteProtocolElementRenderer<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer>
	{
		friend class GuiElementRendererBase<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiSolidBackgroundElementRenderer();

		void							OnElementStateChanged() override;
	};

	class GuiGradientBackgroundElementRenderer : public GuiRemoteProtocolElementRenderer<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer>
	{
		friend class GuiElementRendererBase<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiGradientBackgroundElementRenderer();

		void							OnElementStateChanged() override;
	};

	class GuiInnerShadowElementRenderer : public GuiRemoteProtocolElementRenderer<GuiInnerShadowElement, GuiInnerShadowElementRenderer>
	{
		friend class GuiElementRendererBase<GuiInnerShadowElement, GuiInnerShadowElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiInnerShadowElementRenderer();

		void							OnElementStateChanged() override;
	};

	class GuiSolidLabelElementRenderer : public GuiRemoteProtocolElementRenderer<GuiSolidLabelElement, GuiSolidLabelElementRenderer>
	{
		friend class GuiElementRendererBase<GuiSolidLabelElement, GuiSolidLabelElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiSolidLabelElementRenderer();

		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiImageFrameElementRenderer : public GuiRemoteProtocolElementRenderer<GuiImageFrameElement, GuiImageFrameElementRenderer>
	{
		friend class GuiElementRendererBase<GuiImageFrameElement, GuiImageFrameElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiImageFrameElementRenderer();

		void							OnElementStateChanged() override;
	};

	class GuiPolygonElementRenderer : public GuiRemoteProtocolElementRenderer<GuiPolygonElement, GuiPolygonElementRenderer>
	{
		friend class GuiElementRendererBase<GuiPolygonElement, GuiPolygonElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiPolygonElementRenderer();

		void							OnElementStateChanged() override;
	};

	class GuiColorizedTextElementRenderer : public GuiRemoteProtocolElementRenderer<GuiColorizedTextElement, GuiColorizedTextElementRenderer>, protected GuiColorizedTextElement::ICallback
	{
		friend class GuiElementRendererBase<GuiColorizedTextElement, GuiColorizedTextElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:
		void							ColorChanged() override;
		void							FontChanged() override;
		void							InitializeInternal();
		void							FinalizeInternal();
	public:
		GuiColorizedTextElementRenderer();

		void							OnElementStateChanged() override;
	};
}

#endif