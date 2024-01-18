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
#include "GuiRemoteProtocol.h"
#include "GuiRemoteEvents.h"
#include "GuiRemoteWindow.h"

namespace vl::presentation
{
/***********************************************************************
GuiRemoteController
***********************************************************************/

	class GuiRemoteController
		: public Object
		, public INativeController
		, protected INativeResourceService
		, protected INativeInputService
		, protected INativeScreenService
		, protected INativeScreen
		, protected INativeWindowService
	{
		friend class GuiRemoteEvent;
		friend class GuiRemoteWindow;
		using CursorMap = collections::Dictionary<INativeCursor::SystemCursorType, Ptr<INativeCursor>>;
	protected:
		IGuiRemoteProtocol*											remoteProtocol;
		GuiRemoteEvents												remoteEvent;
		GuiRemoteWindow												remoteWindow;

		SharedCallbackService										callbackService;
		SharedAsyncService											asyncService;
		CursorMap													cursors;

		// =============================================================
		// INativeResourceService
		// =============================================================

		INativeCursor*					GetSystemCursor(INativeCursor::SystemCursorType type) override;
		INativeCursor*					GetDefaultSystemCursor() override;
		FontProperties					GetDefaultFont() override;
		void							SetDefaultFont(const FontProperties& value) override;
		void							EnumerateFonts(collections::List<WString>& fonts) override;

		// =============================================================
		// INativeInputService
		// =============================================================

		void							StartTimer() override;
		void							StopTimer() override;
		bool							IsTimerEnabled() override;
		bool							IsKeyPressing(VKEY code) override;
		bool							IsKeyToggled(VKEY code) override;
		WString							GetKeyName(VKEY code) override;
		VKEY							GetKey(const WString& name) override;
		vint							RegisterGlobalShortcutKey(bool ctrl, bool shift, bool alt, VKEY key) override;
		bool							UnregisterGlobalShortcutKey(vint id) override;

		// =============================================================
		// INativeScreenService
		// =============================================================

		vint							GetScreenCount() override;
		INativeScreen*					GetScreen(vint index) override;
		INativeScreen*					GetScreen(INativeWindow* window) override;

		// =============================================================
		// INativeScreen
		// =============================================================

		NativeRect						GetBounds() override;
		NativeRect						GetClientBounds() override;
		WString							GetName() override;
		bool							IsPrimary() override;
		double							GetScalingX() override;
		double							GetScalingY() override;

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
		GuiRemoteController(IGuiRemoteProtocol* _remoteProtocol);
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

#endif