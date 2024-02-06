/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::GDI Provider for Windows Implementation::Renderer

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSRENDERERSWINDOWSGDI
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSRENDERERSWINDOWSGDI

#include "GuiGraphicsWindowsGDI.h"

namespace vl
{
	namespace presentation
	{
		namespace elements_windows_gdi
		{
			using namespace elements;

/***********************************************************************
Renderers
***********************************************************************/

			class GuiFocusRectangleElementRenderer : public GuiElementRendererBase<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<GuiFocusRectangleElement, GuiFocusRectangleElementRenderer, IWindowsGDIRenderTarget>;
			protected:
				Ptr<windows::WinPen>	pen;
				Ptr<windows::WinBrush>	brush;

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiSolidBorderElementRenderer : public GuiElementRendererBase<GuiSolidBorderElement, GuiSolidBorderElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<GuiSolidBorderElement, GuiSolidBorderElementRenderer, IWindowsGDIRenderTarget>;
			protected:
				Color					oldColor;
				Ptr<windows::WinPen>	pen;
				Ptr<windows::WinBrush>	brush;

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class Gui3DBorderElementRenderer : public GuiElementRendererBase<Gui3DBorderElement, Gui3DBorderElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<Gui3DBorderElement, Gui3DBorderElementRenderer, IWindowsGDIRenderTarget>;
			protected:
				Color					oldColor1;
				Color					oldColor2;
				Ptr<windows::WinPen>	pen1;
				Ptr<windows::WinPen>	pen2;

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class Gui3DSplitterElementRenderer : public GuiElementRendererBase<Gui3DSplitterElement, Gui3DSplitterElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<Gui3DSplitterElement, Gui3DSplitterElementRenderer, IWindowsGDIRenderTarget>;
			protected:
				Color					oldColor1;
				Color					oldColor2;
				Ptr<windows::WinPen>	pen1;
				Ptr<windows::WinPen>	pen2;

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiSolidBackgroundElementRenderer : public GuiElementRendererBase<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<GuiSolidBackgroundElement, GuiSolidBackgroundElementRenderer, IWindowsGDIRenderTarget>;
			protected:
				Color					oldColor;
				Ptr<windows::WinPen>	pen;
				Ptr<windows::WinBrush>	brush;

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiGradientBackgroundElementRenderer : public GuiElementRendererBase<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<GuiGradientBackgroundElement, GuiGradientBackgroundElementRenderer, IWindowsGDIRenderTarget>;
			protected:
				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiInnerShadowElementRenderer : public GuiElementRendererBase<GuiInnerShadowElement, GuiInnerShadowElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<GuiInnerShadowElement, GuiInnerShadowElementRenderer, IWindowsGDIRenderTarget>;
			protected:
				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				GuiInnerShadowElementRenderer();

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiSolidLabelElementRenderer : public GuiElementRendererBase<GuiSolidLabelElement, GuiSolidLabelElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<GuiSolidLabelElement, GuiSolidLabelElementRenderer, IWindowsGDIRenderTarget>;
			protected:
				FontProperties			oldFont;
				Ptr<windows::WinFont>	font;
				vint					oldMaxWidth;

				void					UpdateMinSize();

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				GuiSolidLabelElementRenderer();

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiImageFrameElementRenderer : public GuiElementRendererBase<GuiImageFrameElement, GuiImageFrameElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<GuiImageFrameElement, GuiImageFrameElementRenderer, IWindowsGDIRenderTarget>;
			protected:
				Ptr<windows::WinBitmap>		bitmap;

				void					UpdateBitmap();

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				GuiImageFrameElementRenderer();

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiPolygonElementRenderer : public GuiElementRendererBase<GuiPolygonElement, GuiPolygonElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<GuiPolygonElement, GuiPolygonElementRenderer, IWindowsGDIRenderTarget>;
			protected:
				POINT*							points;
				vint								pointCount;
				Color							oldPenColor;
				Color							oldBrushColor;
				Ptr<windows::WinPen>			pen;
				Ptr<windows::WinBrush>			brush;

				void							InitializeInternal();
				void							FinalizeInternal();
				void							RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				GuiPolygonElementRenderer();
				~GuiPolygonElementRenderer();

				void							Render(Rect bounds)override;
				void							OnElementStateChanged()override;
			};

			class GuiColorizedTextElementRenderer : public GuiElementRendererBase<GuiColorizedTextElement, GuiColorizedTextElementRenderer, IWindowsGDIRenderTarget>, protected GuiColorizedTextElement::ICallback
			{
				friend class GuiElementRendererBase<GuiColorizedTextElement, GuiColorizedTextElementRenderer, IWindowsGDIRenderTarget>;

			public:
				struct ColorItemResource
				{
					Color						text;
					Color						background;
					Ptr<windows::WinBrush>		backgroundBrush;
				};

				struct ColorEntryResource
				{
					ColorItemResource			normal;
					ColorItemResource			selectedFocused;
					ColorItemResource			selectedUnfocused;

					std::partial_ordering		operator<=>(const ColorEntryResource&) const { return std::partial_ordering::unordered; }
					bool						operator==(const ColorEntryResource& value){return false;}
				};

				typedef collections::Array<ColorEntryResource>			ColorArray;
			protected:
				FontProperties			oldFont;
				Ptr<windows::WinFont>	font;
				ColorArray				colors;
				Color					oldCaretColor;
				Ptr<windows::WinPen>	caretPen;

				void					DestroyColors();
				void					ColorChanged();
				void					FontChanged();

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};

			class GuiGDIElementRenderer : public GuiElementRendererBase<GuiGDIElement, GuiGDIElementRenderer, IWindowsGDIRenderTarget>
			{
				friend class GuiElementRendererBase<GuiGDIElement, GuiGDIElementRenderer, IWindowsGDIRenderTarget>;

			protected:

				void					InitializeInternal();
				void					FinalizeInternal();
				void					RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget);
			public:
				GuiGDIElementRenderer();
				~GuiGDIElementRenderer();

				void					Render(Rect bounds)override;
				void					OnElementStateChanged()override;
			};
		}
	}
}

#endif