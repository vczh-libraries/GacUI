/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Direct2D Provider for Windows Implementation::Renderer

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSTEXTRENDERERSWINDOWSDIRECT2D
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSTEXTRENDERERSWINDOWSDIRECT2D

#include "GuiGraphicsRenderersWindowsDirect2D.h"

namespace vl
{
	namespace presentation
	{
		namespace elements_windows_d2d
		{

/***********************************************************************
Renderers
***********************************************************************/

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
		}
	}
}

#endif