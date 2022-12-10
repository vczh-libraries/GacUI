/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Direct2D Provider for Windows Implementation::Renderer

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSLAYOUTPROVIDERWINDOWSDIRECT2D
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSLAYOUTPROVIDERWINDOWSDIRECT2D

#include "..\..\..\..\GraphicsElement\GuiGraphicsDocumentInterfaces.h"

namespace vl
{
	namespace presentation
	{
		namespace elements_windows_d2d
		{
			class WindowsDirect2DLayoutProvider : public Object, public elements::IGuiGraphicsLayoutProvider
			{
			public:
				 Ptr<elements::IGuiGraphicsParagraph>		CreateParagraph(const WString& text, elements::IGuiGraphicsRenderTarget* renderTarget, elements::IGuiGraphicsParagraphCallback* callback)override;
			};
		}
	}
}

#endif