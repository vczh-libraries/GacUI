/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteController

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTEGRAPHICS_DOCUMENT
#define VCZH_PRESENTATION_GUIREMOTEGRAPHICS_DOCUMENT

#include "../../GraphicsElement/GuiGraphicsResourceManager.h"

namespace vl::presentation
{
	class GuiRemoteController;
}

namespace vl::presentation::elements
{
	class GuiRemoteGraphicsResourceManager;
	class GuiRemoteGraphicsRenderTarget;

/***********************************************************************
GuiRemoteGraphicsParagraph
***********************************************************************/

	class GuiRemoteGraphicsParagraph : public IGuiGraphicsParagraph
	{
	protected:
		WString									text;
		GuiRemoteController*					remote = nullptr;
		GuiRemoteGraphicsResourceManager*		resourceManager = nullptr;
		GuiRemoteGraphicsRenderTarget*			renderTarget = nullptr;
		IGuiGraphicsParagraphCallback*			callback = nullptr;

	public:
		GuiRemoteGraphicsParagraph(const WString& _text, GuiRemoteController* _remote, GuiRemoteGraphicsResourceManager* _resourceManager, GuiRemoteGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _callback);
		~GuiRemoteGraphicsParagraph();
	};
}

#endif