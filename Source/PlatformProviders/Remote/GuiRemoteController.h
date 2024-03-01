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
#include "GuiRemoteWindow.h"
#include "GuiRemoteGraphics.h"

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
		friend class GuiRemoteMessages;
		friend class GuiRemoteEvents;
		friend class GuiRemoteWindow;
		friend class GuiRemoteGraphicsImage;
		friend class elements::GuiRemoteGraphicsRenderTarget;
		friend class elements::GuiRemoteGraphicsResourceManager;
		using CursorMap = collections::Dictionary<INativeCursor::SystemCursorType, Ptr<INativeCursor>>;
		using HotKeyEntry = Tuple<bool, bool, bool, VKEY>;
		using HotKeySet = collections::SortedList<HotKeyEntry>;
		using HotKeyIds = collections::Dictionary<vint, HotKeyEntry>;
	protected:
		IGuiRemoteProtocol*								remoteProtocol = nullptr;
		GuiRemoteMessages								remoteMessages;
		GuiRemoteEvents									remoteEvents;
		GuiRemoteWindow									remoteWindow;
		elements::GuiRemoteGraphicsResourceManager*		resourceManager = nullptr;
		SharedCallbackService							callbackService;
		SharedAsyncService								asyncService;
		GuiRemoteGraphicsImageService					imageService;
		bool											applicationRunning = false;
		bool											connectionForcedToStop = false;
		bool											connectionStopped = false;

		remoteprotocol::FontConfig						remoteFontConfig;
		remoteprotocol::ScreenConfig					remoteScreenConfig;

		vint											usedHotKeys = (vint)NativeGlobalShortcutKeyResult::ValidIdBegins;
		HotKeySet										hotKeySet;
		HotKeyIds										hotKeyIds;

		CursorMap										cursors;
		bool											timerEnabled = false;
		bool											windowCreated = false;
		bool											windowDestroyed = false;

		collections::Dictionary<VKEY, WString>			keyNames;
		collections::Dictionary<WString, VKEY>			keyCodes;
		bool											keyInitialized = false;

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
		void							EnsureKeyInitialized();
		WString							GetKeyName(VKEY code) override;
		VKEY							GetKey(const WString& name) override;
		void							UpdateGlobalShortcutKey();
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

		// =============================================================
		// Events
		// =============================================================

		void							OnControllerConnect();
		void							OnControllerDisconnect();
		void							OnControllerRequestExit();
		void							OnControllerForceExit();
		void							OnControllerScreenUpdated(const remoteprotocol::ScreenConfig& arguments);

	public:
		GuiRemoteController(IGuiRemoteProtocol* _remoteProtocol);
		~GuiRemoteController();

		void							Initialize();
		void							Finalize();

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