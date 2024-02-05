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

	class GuiFocusRectangleElementRenderer : public Object, public IGuiGraphicsRenderer
	{
		DEFINE_GUI_GRAPHICS_RENDERER(GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, GuiRemoteGraphicsRenderTarget)
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiSolidBorderElementRenderer : public Object, public IGuiGraphicsRenderer
	{
		DEFINE_GUI_GRAPHICS_RENDERER(GuiSolidBorderElement, GuiSolidBorderElementRenderer, GuiRemoteGraphicsRenderTarget)
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class Gui3DBorderElementRenderer : public Object, public IGuiGraphicsRenderer
	{
		DEFINE_GUI_GRAPHICS_RENDERER(Gui3DBorderElement, Gui3DBorderElementRenderer, GuiRemoteGraphicsRenderTarget)
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class Gui3DSplitterElementRenderer : public Object, public IGuiGraphicsRenderer
	{
		DEFINE_GUI_GRAPHICS_RENDERER(Gui3DSplitterElement, Gui3DSplitterElementRenderer, GuiRemoteGraphicsRenderTarget)
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiSolidBackgroundElementRenderer : public Object, public IGuiGraphicsRenderer
	{
		DEFINE_GUI_GRAPHICS_RENDERER(GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget)
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiGradientBackgroundElementRenderer : public Object, public IGuiGraphicsRenderer
	{
		DEFINE_GUI_GRAPHICS_RENDERER(GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget)
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiInnerShadowElementRenderer : public Object, public IGuiGraphicsRenderer
	{
		DEFINE_GUI_GRAPHICS_RENDERER(GuiInnerShadowElement, GuiInnerShadowElementRenderer, GuiRemoteGraphicsRenderTarget)
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		GuiInnerShadowElementRenderer();

		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiSolidLabelElementRenderer : public Object, public IGuiGraphicsRenderer
	{
		DEFINE_GUI_GRAPHICS_RENDERER(GuiSolidLabelElement, GuiSolidLabelElementRenderer, GuiRemoteGraphicsRenderTarget)
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		GuiSolidLabelElementRenderer();

		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiImageFrameElementRenderer : public Object, public IGuiGraphicsRenderer
	{
		DEFINE_GUI_GRAPHICS_RENDERER(GuiImageFrameElement, GuiImageFrameElementRenderer, GuiRemoteGraphicsRenderTarget)
	protected:

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		GuiImageFrameElementRenderer();

		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiPolygonElementRenderer : public Object, public IGuiGraphicsRenderer
	{
		DEFINE_GUI_GRAPHICS_RENDERER(GuiPolygonElement, GuiPolygonElementRenderer, GuiRemoteGraphicsRenderTarget)
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

	class GuiColorizedTextElementRenderer : public Object, public IGuiGraphicsRenderer, protected GuiColorizedTextElement::ICallback
	{
		DEFINE_GUI_GRAPHICS_RENDERER(GuiColorizedTextElement, GuiColorizedTextElementRenderer, GuiRemoteGraphicsRenderTarget)
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