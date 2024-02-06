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

	class GuiFocusRectangleElementRenderer : GuiElementRendererBase<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, GuiRemoteGraphicsRenderTarget>
	{
		friend class GuiElementRendererBase<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiSolidBorderElementRenderer : GuiElementRendererBase<GuiSolidBorderElement, GuiSolidBorderElementRenderer, GuiRemoteGraphicsRenderTarget>
	{
		friend class GuiElementRendererBase<GuiSolidBorderElement, GuiSolidBorderElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class Gui3DBorderElementRenderer : GuiElementRendererBase<Gui3DBorderElement, Gui3DBorderElementRenderer, GuiRemoteGraphicsRenderTarget>
	{
		friend class GuiElementRendererBase<Gui3DBorderElement, Gui3DBorderElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class Gui3DSplitterElementRenderer : GuiElementRendererBase<Gui3DSplitterElement, Gui3DSplitterElementRenderer, GuiRemoteGraphicsRenderTarget>
	{
		friend class GuiElementRendererBase<Gui3DSplitterElement, Gui3DSplitterElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiSolidBackgroundElementRenderer : GuiElementRendererBase<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget>
	{
		friend class GuiElementRendererBase<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiGradientBackgroundElementRenderer : GuiElementRendererBase<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget>
	{
		friend class GuiElementRendererBase<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiInnerShadowElementRenderer : GuiElementRendererBase<GuiInnerShadowElement, GuiInnerShadowElementRenderer, GuiRemoteGraphicsRenderTarget>
	{
		friend class GuiElementRendererBase<GuiInnerShadowElement, GuiInnerShadowElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		GuiInnerShadowElementRenderer();

		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiSolidLabelElementRenderer : GuiElementRendererBase<GuiSolidLabelElement, GuiSolidLabelElementRenderer, GuiRemoteGraphicsRenderTarget>
	{
		friend class GuiElementRendererBase<GuiSolidLabelElement, GuiSolidLabelElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		GuiSolidLabelElementRenderer();

		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiImageFrameElementRenderer : GuiElementRendererBase<GuiImageFrameElement, GuiImageFrameElementRenderer, GuiRemoteGraphicsRenderTarget>
	{
		friend class GuiElementRendererBase<GuiImageFrameElement, GuiImageFrameElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		GuiImageFrameElementRenderer();

		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiPolygonElementRenderer : GuiElementRendererBase<GuiPolygonElement, GuiPolygonElementRenderer, GuiRemoteGraphicsRenderTarget>
	{
		friend class GuiElementRendererBase<GuiPolygonElement, GuiPolygonElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		GuiPolygonElementRenderer();
		~GuiPolygonElementRenderer();

		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiColorizedTextElementRenderer : GuiElementRendererBase<GuiColorizedTextElement, GuiColorizedTextElementRenderer, GuiRemoteGraphicsRenderTarget>, protected GuiColorizedTextElement::ICallback
	{
		friend class GuiElementRendererBase<GuiColorizedTextElement, GuiColorizedTextElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:
		void					ColorChanged() override;
		void					FontChanged() override;

		void					InitializeInternal();
		void					FinalizeInternal();
		void					RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void					Render(Rect bounds) override;
		void					OnElementStateChanged() override;
	};
}

#endif