/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteController

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER

#include "../../Utilities/SharedServices/GuiSharedCallbackService.h"
#include "../../Utilities/SharedServices/GuiSharedAsyncService.h"

namespace vl
{
	namespace presentation
	{
/***********************************************************************
IGuiRemoteProtocol
***********************************************************************/

		class IGuiRemoteProtocol : public virtual Interface
		{
		public:
		};

/***********************************************************************
GuiRemoteController
***********************************************************************/

		class GuiRemoteController
			: public Object
			, public INativeController
			, protected INativeScreenService
			, protected INativeWindowService
		{
		protected:
			SharedCallbackService										callbackService;
			SharedAsyncService											asyncService;

			// =============================================================
			// INativeScreenService
			// =============================================================

			vint							GetScreenCount() override;
			INativeScreen*					GetScreen(vint index) override;
			INativeScreen*					GetScreen(INativeWindow* window) override;

			// =============================================================
			// INativeWindowService
			// =============================================================
			
			const NativeWindowFrameConfig&	GetMainWindowFrameConfig() override;
			const NativeWindowFrameConfig&	GetNonMainWindowFrameConfig() override;
			INativeWindow*					CreateNativeWindow(INativeWindow::WindowMode windowMode) override;
			void							DestroyNativeWindow(INativeWindow* window) override;
			INativeWindow*					GetMainWindow() override;
			INativeWindow*					GetWindow(NativePoint location) override;
			void							Run(INativeWindow* window) override;
			bool							RunOneCycle() override;
		public:
			GuiRemoteController();
			~GuiRemoteController();

			// =============================================================
			// INativeController
			// =============================================================

			INativeCallbackService*			CallbackService() override;
			INativeResourceService*			ResourceService() override;
			INativeAsyncService*			AsyncService() override;
			INativeClipboardService*		ClipboardService() override;
			INativeImageService*			ImageService() override;
			INativeInputService*			InputService() override;
			INativeDialogService*			DialogService() override;
			WString							GetExecutablePath() override;
			
			INativeScreenService*			ScreenService() override;
			INativeWindowService*			WindowService() override;
		};
	}
}

#endif