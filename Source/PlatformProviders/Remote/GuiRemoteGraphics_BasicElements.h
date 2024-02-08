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

	class IGuiRemoteProtocolElementRenderBase : public virtual Interface
	{
	public:
		virtual IGuiGraphicsRenderer*	GetRenderer() = 0;
		virtual vint					GetID() = 0;
		virtual bool					IsUpdated() = 0;
		virtual void					SetUpdated() = 0;
		virtual void					ResetUpdated() = 0;
		virtual void					SendUpdateElementMessages() = 0;
	};

	template<typename TElement, typename TRenderer>
	class GuiRemoteProtocolElementRenderer
		: public GuiElementRendererBase<TElement, TRenderer, GuiRemoteGraphicsRenderTarget>
		, public virtual IGuiRemoteProtocolElementRenderBase
	{
	protected:
		vint							id = -1;
		bool							updated = true;

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:

		IGuiGraphicsRenderer*			GetRenderer() override;
		vint							GetID() override;
		bool							IsUpdated() override;
		void							SetUpdated() override;
		void							ResetUpdated() override;

		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiFocusRectangleElementRenderer : public GuiRemoteProtocolElementRenderer<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer>
	{
		friend class GuiElementRendererBase<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiFocusRectangleElementRenderer();

		bool							IsUpdated() override;
		void							SetUpdated() override;
		void							ResetUpdated() override;
		void							SendUpdateElementMessages() override;
		void							OnElementStateChanged() override;
	};

	class GuiSolidBorderElementRenderer : public GuiRemoteProtocolElementRenderer<GuiSolidBorderElement, GuiSolidBorderElementRenderer>
	{
		friend class GuiElementRendererBase<GuiSolidBorderElement, GuiSolidBorderElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiSolidBorderElementRenderer();

		void							SendUpdateElementMessages() override;
	};

	class Gui3DBorderElementRenderer : public GuiRemoteProtocolElementRenderer<Gui3DBorderElement, Gui3DBorderElementRenderer>
	{
		friend class GuiElementRendererBase<Gui3DBorderElement, Gui3DBorderElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		Gui3DBorderElementRenderer();

		void							SendUpdateElementMessages() override;
	};

	class Gui3DSplitterElementRenderer : public GuiRemoteProtocolElementRenderer<Gui3DSplitterElement, Gui3DSplitterElementRenderer>
	{
		friend class GuiElementRendererBase<Gui3DSplitterElement, Gui3DSplitterElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		Gui3DSplitterElementRenderer();

		void							SendUpdateElementMessages() override;
	};

	class GuiSolidBackgroundElementRenderer : public GuiRemoteProtocolElementRenderer<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer>
	{
		friend class GuiElementRendererBase<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiSolidBackgroundElementRenderer();

		void							SendUpdateElementMessages() override;
	};

	class GuiGradientBackgroundElementRenderer : public GuiRemoteProtocolElementRenderer<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer>
	{
		friend class GuiElementRendererBase<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiGradientBackgroundElementRenderer();

		void							SendUpdateElementMessages() override;
	};

	class GuiInnerShadowElementRenderer : public GuiRemoteProtocolElementRenderer<GuiInnerShadowElement, GuiInnerShadowElementRenderer>
	{
		friend class GuiElementRendererBase<GuiInnerShadowElement, GuiInnerShadowElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiInnerShadowElementRenderer();

		void							SendUpdateElementMessages() override;
	};

	class GuiSolidLabelElementRenderer : public GuiRemoteProtocolElementRenderer<GuiSolidLabelElement, GuiSolidLabelElementRenderer>
	{
		friend class GuiElementRendererBase<GuiSolidLabelElement, GuiSolidLabelElementRenderer, GuiRemoteGraphicsRenderTarget>;
		using TBase = GuiRemoteProtocolElementRenderer<GuiSolidLabelElement, GuiSolidLabelElementRenderer>;
	public:
		GuiSolidLabelElementRenderer();

		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
		void							SendUpdateElementMessages() override;
	};

	class GuiImageFrameElementRenderer : public GuiRemoteProtocolElementRenderer<GuiImageFrameElement, GuiImageFrameElementRenderer>
	{
		friend class GuiElementRendererBase<GuiImageFrameElement, GuiImageFrameElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiImageFrameElementRenderer();

		void							SendUpdateElementMessages() override;
	};

	class GuiPolygonElementRenderer : public GuiRemoteProtocolElementRenderer<GuiPolygonElement, GuiPolygonElementRenderer>
	{
		friend class GuiElementRendererBase<GuiPolygonElement, GuiPolygonElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiPolygonElementRenderer();

		void							SendUpdateElementMessages() override;
	};

	class GuiColorizedTextElementRenderer : public GuiRemoteProtocolElementRenderer<GuiColorizedTextElement, GuiColorizedTextElementRenderer>, protected GuiColorizedTextElement::ICallback
	{
		friend class GuiElementRendererBase<GuiColorizedTextElement, GuiColorizedTextElementRenderer, GuiRemoteGraphicsRenderTarget>;
		using TBase = GuiRemoteProtocolElementRenderer<GuiColorizedTextElement, GuiColorizedTextElementRenderer>;
	protected:
		void							ColorChanged() override;
		void							FontChanged() override;
		void							InitializeInternal();
		void							FinalizeInternal();
	public:
		GuiColorizedTextElementRenderer();

		void							OnElementStateChanged() override;
		void							SendUpdateElementMessages() override;
	};
}

#endif