/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteController

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTEGRAPHICS
#define VCZH_PRESENTATION_GUIREMOTEGRAPHICS

#include "../../GraphicsElement/GuiGraphicsResourceManager.h"

namespace vl::presentation
{
	class GuiRemoteController;

	namespace elements
	{
/***********************************************************************
GuiRemoteGraphicsRenderTarget
***********************************************************************/

/***********************************************************************
GuiRemoteGraphicsResourceManager
***********************************************************************/

		class GuiRemoteGraphicsResourceManager
			: public GuiGraphicsResourceManager
		{
		protected:
			GuiRemoteController*								remote;

		public:
			GuiRemoteGraphicsResourceManager(GuiRemoteController* _remote);
			~GuiRemoteGraphicsResourceManager();

			// =============================================================
			// IGuiGraphicsResourceManager
			// =============================================================

			IGuiGraphicsRenderTarget*			GetRenderTarget(INativeWindow* window) override;
			void								RecreateRenderTarget(INativeWindow* window) override;
			void								ResizeRenderTarget(INativeWindow* window) override;
			IGuiGraphicsLayoutProvider*			GetLayoutProvider() override;
		};
	}
}

#endif