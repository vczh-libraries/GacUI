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

	class IGuiRemoteProtocolElementRender : public virtual Interface
	{
	public:
		virtual IGuiGraphicsRenderer*			GetRenderer() = 0;
		virtual vint							GetID() = 0;
		virtual remoteprotocol::RendererType	GetRendererType() = 0;
		virtual bool							IsUpdated() = 0;
		virtual void							ResetUpdated() = 0;
		virtual void							SendUpdateElementMessages(bool fullContent) = 0;
		virtual bool							IsRenderedInLastBatch() = 0;

		virtual bool							NeedUpdateMinSizeFromCache() = 0;
		virtual void							TryFetchMinSizeFromCache() = 0;
		virtual void							UpdateMinSize(Size size) = 0;
		virtual void							NotifyMinSizeCacheInvalidated() = 0;
	};

	template<typename TElement, typename TRenderer, remoteprotocol::RendererType _RendererType>
	class GuiRemoteProtocolElementRenderer
		: public GuiElementRendererBase<TElement, TRenderer, GuiRemoteGraphicsRenderTarget>
		, public virtual IGuiRemoteProtocolElementRender
	{
	protected:
		vint							id = -1;
		vuint64_t						renderingBatchId = 0;
		bool							updated = true;

		void							InitializeInternal();
		void							FinalizeInternal();
		void							RenderTargetChangedInternal(GuiRemoteGraphicsRenderTarget* oldRenderTarget, GuiRemoteGraphicsRenderTarget* newRenderTarget);
	public:
		// IGuiRemoteProtocolElementRender
		IGuiGraphicsRenderer*			GetRenderer() override;
		vint							GetID() override;
		remoteprotocol::RendererType	GetRendererType() override;
		bool							IsUpdated() override;
		void							ResetUpdated() override;
		bool							IsRenderedInLastBatch() override;

		bool							NeedUpdateMinSizeFromCache() override;
		void							TryFetchMinSizeFromCache() override;
		void							UpdateMinSize(Size size) override;
		void							NotifyMinSizeCacheInvalidated() override;

		// IGuiGraphicsRenderer
		void							Render(Rect bounds) override;
		void							OnElementStateChanged() override;
	};

	class GuiFocusRectangleElementRenderer : public GuiRemoteProtocolElementRenderer<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, remoteprotocol::RendererType::FocusRectangle>
	{
		friend class GuiElementRendererBase<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiFocusRectangleElementRenderer();

		bool							IsUpdated() override;
		void							ResetUpdated() override;
		void							SendUpdateElementMessages(bool fullContent) override;
		void							OnElementStateChanged() override;
	};

	class GuiSolidBorderElementRenderer : public GuiRemoteProtocolElementRenderer<GuiSolidBorderElement, GuiSolidBorderElementRenderer, remoteprotocol::RendererType::SolidBorder>
	{
		friend class GuiElementRendererBase<GuiSolidBorderElement, GuiSolidBorderElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiSolidBorderElementRenderer();

		void							SendUpdateElementMessages(bool fullContent) override;
	};

	class Gui3DBorderElementRenderer : public GuiRemoteProtocolElementRenderer<Gui3DBorderElement, Gui3DBorderElementRenderer, remoteprotocol::RendererType::SinkBorder>
	{
		friend class GuiElementRendererBase<Gui3DBorderElement, Gui3DBorderElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		Gui3DBorderElementRenderer();

		void							SendUpdateElementMessages(bool fullContent) override;
	};

	class Gui3DSplitterElementRenderer : public GuiRemoteProtocolElementRenderer<Gui3DSplitterElement, Gui3DSplitterElementRenderer, remoteprotocol::RendererType::SinkSplitter>
	{
		friend class GuiElementRendererBase<Gui3DSplitterElement, Gui3DSplitterElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		Gui3DSplitterElementRenderer();

		void							SendUpdateElementMessages(bool fullContent) override;
	};

	class GuiSolidBackgroundElementRenderer : public GuiRemoteProtocolElementRenderer<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, remoteprotocol::RendererType::SolidBackground>
	{
		friend class GuiElementRendererBase<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiSolidBackgroundElementRenderer();

		void							SendUpdateElementMessages(bool fullContent) override;
	};

	class GuiGradientBackgroundElementRenderer : public GuiRemoteProtocolElementRenderer<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, remoteprotocol::RendererType::GradientBackground>
	{
		friend class GuiElementRendererBase<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiGradientBackgroundElementRenderer();

		void							SendUpdateElementMessages(bool fullContent) override;
	};

	class GuiInnerShadowElementRenderer : public GuiRemoteProtocolElementRenderer<GuiInnerShadowElement, GuiInnerShadowElementRenderer, remoteprotocol::RendererType::InnerShadow>
	{
		friend class GuiElementRendererBase<GuiInnerShadowElement, GuiInnerShadowElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiInnerShadowElementRenderer();

		void							SendUpdateElementMessages(bool fullContent) override;
	};

	class GuiSolidLabelElementRenderer : public GuiRemoteProtocolElementRenderer<GuiSolidLabelElement, GuiSolidLabelElementRenderer, remoteprotocol::RendererType::SolidLabel>
	{
		friend class GuiElementRendererBase<GuiSolidLabelElement, GuiSolidLabelElementRenderer, GuiRemoteGraphicsRenderTarget>;
		using MeasuringRequest = Nullable<remoteprotocol::ElementSolidLabelMeasuringRequest>;
	protected:
		FontProperties					lastFont;
		WString							lastText;
		bool							needFontHeight = false;

		MeasuringRequest				GetMeasuringRequest();
		bool							IsNeedFontHeight(MeasuringRequest request);
	public:
		GuiSolidLabelElementRenderer();

		bool							NeedUpdateMinSizeFromCache() override;
		void							TryFetchMinSizeFromCache() override;
		void							UpdateMinSize(Size size) override;
		void							NotifyMinSizeCacheInvalidated() override;

		void							SendUpdateElementMessages(bool fullContent) override;
	};

	class GuiImageFrameElementRenderer : public GuiRemoteProtocolElementRenderer<GuiImageFrameElement, GuiImageFrameElementRenderer, remoteprotocol::RendererType::ImageFrame>
	{
		friend class GuiElementRendererBase<GuiImageFrameElement, GuiImageFrameElementRenderer, GuiRemoteGraphicsRenderTarget>;
	protected:
		bool							needUpdateSize = false;

		GuiRemoteGraphicsImage*			GetRemoteImage();
		void							UpdateMinSizeFromImage(GuiRemoteGraphicsImage* image);
	public:
		GuiImageFrameElementRenderer();

		bool							NeedUpdateMinSizeFromCache() override;
		void							TryFetchMinSizeFromCache() override;
		void							SendUpdateElementMessages(bool fullContent) override;
	};

	class GuiPolygonElementRenderer : public GuiRemoteProtocolElementRenderer<GuiPolygonElement, GuiPolygonElementRenderer, remoteprotocol::RendererType::Polygon>
	{
		friend class GuiElementRendererBase<GuiPolygonElement, GuiPolygonElementRenderer, GuiRemoteGraphicsRenderTarget>;
	public:
		GuiPolygonElementRenderer();

		void							SendUpdateElementMessages(bool fullContent) override;
	};

	class GuiColorizedTextElementRenderer : public GuiRemoteProtocolElementRenderer<GuiColorizedTextElement, GuiColorizedTextElementRenderer, remoteprotocol::RendererType::UnsupportedColorizedText>, protected GuiColorizedTextElement::ICallback
	{
		friend class GuiElementRendererBase<GuiColorizedTextElement, GuiColorizedTextElementRenderer, GuiRemoteGraphicsRenderTarget>;
		using TBase = GuiRemoteProtocolElementRenderer<GuiColorizedTextElement, GuiColorizedTextElementRenderer, remoteprotocol::RendererType::UnsupportedColorizedText>;
	protected:
		void							ColorChanged() override;
		void							FontChanged() override;
		void							InitializeInternal();
		void							FinalizeInternal();
	public:
		GuiColorizedTextElementRenderer();

		void							OnElementStateChanged() override;
		void							SendUpdateElementMessages(bool fullContent) override;
	};
}

#endif