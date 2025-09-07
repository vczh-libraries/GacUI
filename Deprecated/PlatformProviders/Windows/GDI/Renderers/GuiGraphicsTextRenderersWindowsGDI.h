/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::GDI Provider for Windows Implementation::Renderer

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSTEXTRENDERERSWINDOWSGDI
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSTEXTRENDERERSWINDOWSGDI

#include "GuiGraphicsRenderersWindowsGDI.h"

namespace vl
{
	namespace presentation
	{
		namespace elements_windows_gdi
		{

/***********************************************************************
Renderers
***********************************************************************/

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
		}
	}
}

#endif