/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Direct2D Provider for Windows Implementation::Renderer

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSRENDERERSWINDOWSDIRECT2D
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSRENDERERSWINDOWSDIRECT2D

#include "GuiGraphicsWindowsDirect2D.h"

namespace vl
{
	namespace presentation
	{
		namespace elements_windows_d2d
		{
			using namespace elements;

			typedef collections::Pair<Color, Color> ColorPair;

			template<typename TElement, typename TRenderer>
			class GuiDirect2DElementRendererBase : public GuiElementRendererBase<TElement, TRenderer, IWindowsDirect2DRenderTarget>
			{
			protected:
				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
			public:
			};

			template<typename TElement, typename TRenderer, typename TBrush>
			class GuiSolidBrushElementRendererBase : public GuiDirect2DElementRendererBase<TElement, TRenderer>
			{
			protected:
				Color					oldColor;
				TBrush*					brush = nullptr;

				void					CreateBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					DestroyBrush(IWindowsDirect2DRenderTarget* _renderTarget);
			public:

				void					OnElementStateChanged()override;
			};

			template<typename TElement, typename TRenderer, typename TBrush>
			class GuiGradientBrushElementRendererBase : public GuiDirect2DElementRendererBase<TElement, TRenderer>
			{
			protected:
				ColorPair				oldColor;
				TBrush*					brush = nullptr;

				void					CreateBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					DestroyBrush(IWindowsDirect2DRenderTarget* _renderTarget);
			public:

				void					OnElementStateChanged()override;
			};

/***********************************************************************
Renderers
***********************************************************************/

			class GuiFocusRectangleElementRenderer : public GuiElementRendererBase<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, IWindowsDirect2DRenderTarget>
			{
				friend class GuiElementRendererBase<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, IWindowsDirect2DRenderTarget>;
			protected:
				ID2D1Effect*			focusRectangleEffect = nullptr;

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
			public:

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiSolidBorderElementRenderer : public GuiSolidBrushElementRendererBase<GuiSolidBorderElement, GuiSolidBorderElementRenderer, ID2D1SolidColorBrush>
			{
				friend class GuiElementRendererBase<GuiSolidBorderElement, GuiSolidBorderElementRenderer, IWindowsDirect2DRenderTarget>;
				friend class GuiDirect2DElementRendererBase<GuiSolidBorderElement, GuiSolidBorderElementRenderer>;
			public:
				GuiSolidBorderElementRenderer();

				void					Render(Rect bounds)override;
			};

			class Gui3DBorderElementRenderer : public GuiElementRendererBase<Gui3DBorderElement, Gui3DBorderElementRenderer, IWindowsDirect2DRenderTarget>
			{
				friend class GuiElementRendererBase<Gui3DBorderElement, Gui3DBorderElementRenderer, IWindowsDirect2DRenderTarget>;
			protected:
				Color					oldColor1;
				Color					oldColor2;
				ID2D1SolidColorBrush*	brush1 = nullptr;
				ID2D1SolidColorBrush*	brush2 = nullptr;

				void					CreateBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					DestroyBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
			public:
				Gui3DBorderElementRenderer();

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class Gui3DSplitterElementRenderer : public GuiElementRendererBase<Gui3DSplitterElement, Gui3DSplitterElementRenderer, IWindowsDirect2DRenderTarget>
			{
				friend class GuiElementRendererBase<Gui3DSplitterElement, Gui3DSplitterElementRenderer, IWindowsDirect2DRenderTarget>;
			protected:
				Color					oldColor1;
				Color					oldColor2;
				ID2D1SolidColorBrush*	brush1 = nullptr;
				ID2D1SolidColorBrush*	brush2 = nullptr;

				void					CreateBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					DestroyBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
			public:
				Gui3DSplitterElementRenderer();

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiSolidBackgroundElementRenderer : public GuiSolidBrushElementRendererBase<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, ID2D1SolidColorBrush>
			{
				friend class GuiElementRendererBase<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, IWindowsDirect2DRenderTarget>;
				friend class GuiDirect2DElementRendererBase<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer>;
			public:
				GuiSolidBackgroundElementRenderer();

				void					Render(Rect bounds)override;
			};

			class GuiGradientBackgroundElementRenderer : public GuiGradientBrushElementRendererBase<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, ID2D1LinearGradientBrush>
			{
				friend class GuiElementRendererBase<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, IWindowsDirect2DRenderTarget>;
				friend class GuiDirect2DElementRendererBase<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer>;
			public:
				GuiGradientBackgroundElementRenderer();

				void					Render(Rect bounds)override;
			};

			class GuiInnerShadowElementRenderer : public GuiElementRendererBase<GuiInnerShadowElement, GuiInnerShadowElementRenderer, IWindowsDirect2DRenderTarget>
			{
				friend class GuiElementRendererBase<GuiInnerShadowElement, GuiInnerShadowElementRenderer, IWindowsDirect2DRenderTarget>;
			protected:
				Color							oldColor;
				Color							transparentColor;
				ID2D1LinearGradientBrush*		linearBrush = nullptr;
				ID2D1RadialGradientBrush*		radialBrush = nullptr;

				void					CreateBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					DestroyBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
			public:
				GuiInnerShadowElementRenderer();

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiSolidLabelElementRenderer : public GuiElementRendererBase<GuiSolidLabelElement, GuiSolidLabelElementRenderer, IWindowsDirect2DRenderTarget>
			{
				friend class GuiElementRendererBase<GuiSolidLabelElement, GuiSolidLabelElementRenderer, IWindowsDirect2DRenderTarget>;
			protected:
				Color							oldColor;
				FontProperties					oldFont;
				WString							oldText;
				ID2D1SolidColorBrush*			brush = nullptr;
				Direct2DTextFormatPackage*		textFormat = nullptr;
				IDWriteTextLayout*				textLayout = nullptr;
				vint							oldMaxWidth = -1;

				void					CreateBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					DestroyBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					CreateTextFormat(IWindowsDirect2DRenderTarget* _renderTarget);
				void					DestroyTextFormat(IWindowsDirect2DRenderTarget* _renderTarget);
				void					CreateTextLayout();
				void					DestroyTextLayout();
				void					UpdateMinSize();

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
			public:
				GuiSolidLabelElementRenderer();

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiImageFrameElementRenderer : public GuiElementRendererBase<GuiImageFrameElement, GuiImageFrameElementRenderer, IWindowsDirect2DRenderTarget>
			{
				friend class GuiElementRendererBase<GuiImageFrameElement, GuiImageFrameElementRenderer, IWindowsDirect2DRenderTarget>;
			protected:
				ComPtr<ID2D1Bitmap>		bitmap;

				void					UpdateBitmap(IWindowsDirect2DRenderTarget* renderTarget);

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
			public:
				GuiImageFrameElementRenderer();

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiPolygonElementRenderer : public GuiElementRendererBase<GuiPolygonElement, GuiPolygonElementRenderer, IWindowsDirect2DRenderTarget>
			{
				friend class GuiElementRendererBase<GuiPolygonElement, GuiPolygonElementRenderer, IWindowsDirect2DRenderTarget>;
			protected:
				Color							oldBorderColor;
				Color							oldBackgroundColor;
				ID2D1SolidColorBrush*			borderBrush = nullptr;
				ID2D1SolidColorBrush*			backgroundBrush = nullptr;
				collections::Array<Point>		oldPoints;
				ComPtr<ID2D1PathGeometry>		geometry;

				void							CreateGeometry();
				void							DestroyGeometry();
				void							FillGeometry(Point offset);
				void							RecreateResource(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
				void							InitializeInternal();
				void							FinalizeInternal();
				void							RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
			public:
				GuiPolygonElementRenderer();

				void							Render(Rect bounds)override;
				void							OnElementStateChanged()override;
			};

			class GuiColorizedTextElementRenderer : public GuiElementRendererBase<GuiColorizedTextElement, GuiColorizedTextElementRenderer, IWindowsDirect2DRenderTarget>, protected GuiColorizedTextElement::ICallback
			{
				friend class GuiElementRendererBase<GuiColorizedTextElement, GuiColorizedTextElementRenderer, IWindowsDirect2DRenderTarget>;

			public:
				struct ColorItemResource
				{
					Color						text;
					ID2D1SolidColorBrush*		textBrush;
					Color						background;
					ID2D1SolidColorBrush*		backgroundBrush;
				};

				struct ColorEntryResource
				{
					ColorItemResource			normal;
					ColorItemResource			selectedFocused;
					ColorItemResource			selectedUnfocused;

					std::partial_ordering		operator<=>(const ColorEntryResource&) const { return std::partial_ordering::unordered; }
					bool						operator==(const ColorEntryResource& value) const { return false; }
				};

				typedef collections::Array<ColorEntryResource>			ColorArray;
			protected:
				FontProperties					oldFont;
				Direct2DTextFormatPackage*		textFormat;
				ColorArray						colors;
				Color							oldCaretColor;
				ID2D1SolidColorBrush*			caretBrush;
				
				void					CreateTextBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					DestroyTextBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					CreateCaretBrush(IWindowsDirect2DRenderTarget* _renderTarget);
				void					DestroyCaretBrush(IWindowsDirect2DRenderTarget* _renderTarget);

				void					ColorChanged();
				void					FontChanged();
				text::CharMeasurer*		GetCharMeasurer();

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
			public:
				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiDirect2DElementRenderer : public GuiElementRendererBase<GuiDirect2DElement, GuiDirect2DElementRenderer, IWindowsDirect2DRenderTarget>
			{
				friend class GuiElementRendererBase<GuiDirect2DElement, GuiDirect2DElementRenderer, IWindowsDirect2DRenderTarget>;

			protected:

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget);
			public:
				GuiDirect2DElementRenderer();
				~GuiDirect2DElementRenderer();

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};
		}
	}
}

#endif