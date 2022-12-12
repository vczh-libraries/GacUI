/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Hosted Window

Interfaces:
  GuiHostedController
  GuiHostedScreen
  GuiHostedMainWindow
  GuiHostedWindow

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIHOSTEDWINDOW
#define VCZH_PRESENTATION_GUIHOSTEDWINDOW

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiHostedController
***********************************************************************/

		class GuiHostedController : public Object, public virtual INativeController, public virtual INativeScreenService, public virtual INativeWindowService
		{
		protected:
			INativeController*				nativeController = nullptr;
		public:
			GuiHostedController(INativeController* _nativeController);
			~GuiHostedController();

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

			virtual vint					GetScreenCount() override;
			virtual INativeScreen*			GetScreen(vint index) override;
			virtual INativeScreen*			GetScreen(INativeWindow* window) override;

			INativeWindow*					CreateNativeWindow(INativeWindow::WindowMode windowMode) override;
			void							DestroyNativeWindow(INativeWindow* window) override;
			INativeWindow*					GetMainWindow() override;
			INativeWindow*					GetWindow(NativePoint location) override;
			void							Run(INativeWindow* window) override;
		};
	}
}

#endif