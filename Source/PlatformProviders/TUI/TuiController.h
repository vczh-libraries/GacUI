#ifndef VCZH_PRESENTATION_TUICONTROLLER
#define VCZH_PRESENTATION_TUICONTROLLER

#include "TuiApplication.h"
#include "TuiWindow.h"
#include "../../Utilities/SharedServices/GuiSharedCallbackService.h"
#include "../../Utilities/SharedServices/GuiSharedAsyncService.h"

namespace vl::presentation
{
	class TuiControllerBase
		: public Object
		, public INativeController
		, public ITuiApplication
		, public console::ITuiCallback
		, protected INativeScreenService
		, protected INativeScreen
		, protected INativeWindowService
	{
	protected:
		SharedCallbackService			callbackService;
		SharedAsyncService				asyncService;
		Ptr<TuiWindow>					window;
		NativeWindowFrameConfig			frameConfig;

		virtual void					PumpPlatformEvents() = 0;
		void							Starting() override;
	public:
		TuiControllerBase();
		~TuiControllerBase();
		ITuiApplication*				GetTuiApplication();
		virtual void					ApplyTitle(const WString& title) = 0;
		INativeCallbackService*			CallbackService() override;
		INativeAsyncService*			AsyncService() override;
		INativeDialogService*			DialogService() override;
		INativeAutomationService*		AutomationService() override;
		INativeScreenService*			ScreenService() override;
		INativeWindowService*			WindowService() override;
		vint							GetScreenCount() override;
		INativeScreen*					GetScreen(vint index) override;
		INativeScreen*					GetScreen(INativeWindow* window) override;
		NativeRect						GetBounds() override;
		NativeRect						GetClientBounds() override;
		WString							GetName() override;
		bool							IsPrimary() override;
		double							GetScalingX() override;
		double							GetScalingY() override;
		const NativeWindowFrameConfig&	GetMainWindowFrameConfig() override;
		const NativeWindowFrameConfig&	GetNonMainWindowFrameConfig() override;
		INativeWindow*					CreateNativeWindow(INativeWindow::WindowMode windowMode) override;
		void							DestroyNativeWindow(INativeWindow* value) override;
		INativeWindow*					GetMainWindow() override;
		INativeWindow*					GetWindow(NativePoint location) override;
		void							Run(INativeWindow* value) override;
		bool							RunOneCycle() override;
		void							Stop() override;
		void							BufferSizeChanged() override;
		void							Timer() override;
		void							ClipboardUpdated();
		void							GlobalShortcutKeyActivated(vint id);
		void							KeyDown(const NativeWindowKeyInfo& info) override;
		void							KeyUp(const NativeWindowKeyInfo& info) override;
		void							Char(const NativeWindowCharInfo& info) override;
		void							MouseMove(const WindowMouseInfo& info) override;
		void							MouseDown(NativeMouseButton button, const WindowMouseInfo& info) override;
		void							MouseUp(NativeMouseButton button, const WindowMouseInfo& info) override;
		void							MouseDoubleClick(NativeMouseButton button, const WindowMouseInfo& info) override;
		void							MouseVerticalWheel(const WindowMouseInfo& info) override;
		void							MouseHorizontalWheel(const WindowMouseInfo& info) override;
	};
}

#endif
