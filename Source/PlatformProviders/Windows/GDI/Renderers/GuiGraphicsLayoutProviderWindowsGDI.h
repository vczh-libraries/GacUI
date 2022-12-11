/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::GDI Provider for Windows Implementation::Renderer

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSLAYOUTPROVIDERWINDOWSGDI
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSLAYOUTPROVIDERWINDOWSGDI

#include "..\..\..\..\GraphicsElement\GuiGraphicsDocumentInterfaces.h"

namespace vl
{
	namespace presentation
	{
		namespace elements_windows_gdi
		{
			class WindowsGDILayoutProvider : public Object, public elements::IGuiGraphicsLayoutProvider
			{
			public:
				 Ptr<elements::IGuiGraphicsParagraph>		CreateParagraph(const WString& text, elements::IGuiGraphicsRenderTarget* renderTarget, elements::IGuiGraphicsParagraphCallback* callback)override;
			};
		}
	}
}

#endif