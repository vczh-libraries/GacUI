#include "GuiRemoteController.h"

namespace vl::presentation
{
	using namespace collections;

/***********************************************************************
GuiRemoteCursor
***********************************************************************/

	class GuiRemoteCursor : public Object, public virtual INativeCursor
	{
	protected:
		INativeCursor::SystemCursorType								cursorType;

	public:
		GuiRemoteCursor(INativeCursor::SystemCursorType _cursorType) : cursorType(_cursorType) {}

		bool				IsSystemCursor() { return true; }
		SystemCursorType	GetSystemCursorType() { return cursorType; }
	};
		
/***********************************************************************
GuiRemoteController::INativeResourceService
***********************************************************************/

	INativeCursor* GuiRemoteController::GetSystemCursor(INativeCursor::SystemCursorType type)
	{
		vint index = cursors.Keys().IndexOf(type);
		if (index == -1)
		{
			auto cursor = Ptr(new GuiRemoteCursor(type));
			cursors.Add(type, cursor);
			return cursor.Obj();
		}
		else
		{
			return cursors.Values()[index].Obj();
		}
	}

	INativeCursor* GuiRemoteController::GetDefaultSystemCursor()
	{
		return GetSystemCursor(INativeCursor::SystemCursorType::Arrow);
	}

	FontProperties GuiRemoteController::GetDefaultFont()
	{
		return remoteFontConfig.defaultFont;
	}

	void GuiRemoteController::SetDefaultFont(const FontProperties& value)
	{
		remoteFontConfig.defaultFont = value;
	}

	void GuiRemoteController::EnumerateFonts(collections::List<WString>& fonts)
	{
		if (remoteFontConfig.supportedFonts)
		{
			CopyFrom(fonts, *remoteFontConfig.supportedFonts.Obj());
		}
	}
			
/***********************************************************************
GuiRemoteController::INativeInputService
***********************************************************************/

	void GuiRemoteController::StartTimer()
	{
		timerEnabled = true;
	}

	void GuiRemoteController::StopTimer()
	{
		timerEnabled = false;
	}

	bool GuiRemoteController::IsTimerEnabled()
	{
		return timerEnabled;
	}

	bool GuiRemoteController::IsKeyPressing(VKEY code)
	{
		vint idIsKeyPressing = remoteMessages.RequestIOIsKeyPressing(code);
		remoteMessages.Submit();
		bool result = remoteMessages.RetrieveIOIsKeyPressing(idIsKeyPressing);
		return result;
	}

	bool GuiRemoteController::IsKeyToggled(VKEY code)
	{
		vint idIsKeyToggled = remoteMessages.RequestIOIsKeyToggled(code);
		remoteMessages.Submit();
		bool result = remoteMessages.RetrieveIOIsKeyToggled(idIsKeyToggled);
		return result;
	}

	void GuiRemoteController::EnsureKeyInitialized()
	{
		if (keyInitialized) return;
		keyInitialized = true;

#define INITIALIZE_KEY_NAME(NAME, TEXT)\
		keyNames.Add(VKEY::KEY_ ## NAME, WString::Unmanaged(TEXT));\
		if (!keyCodes.Keys().Contains(WString::Unmanaged(TEXT))) keyCodes.Add(WString::Unmanaged(TEXT), VKEY::KEY_ ## NAME);\

		GUI_DEFINE_KEYBOARD_WINDOWS_NAME(INITIALIZE_KEY_NAME)
#undef INITIALIZE_KEY_NAME
	}

	WString GuiRemoteController::GetKeyName(VKEY code)
	{
		EnsureKeyInitialized();
		vint index = keyNames.Keys().IndexOf(code);
		return index == -1 ? WString::Unmanaged(L"?") : keyNames.Values()[index];
	}

	VKEY GuiRemoteController::GetKey(const WString& name)
	{
		EnsureKeyInitialized();
		vint index = keyCodes.Keys().IndexOf(name);
		return index == -1 ? VKEY::KEY_UNKNOWN : keyCodes.Values()[index];
	}

	void GuiRemoteController::UpdateGlobalShortcutKey()
	{
		auto hotKeys = Ptr(new List<remoteprotocol::GlobalShortcutKey>);
		for (auto [id, entry] : hotKeyIds)
		{
			remoteprotocol::GlobalShortcutKey key;
			key.id = id;
			key.ctrl = entry.get<0>();
			key.shift = entry.get<1>();
			key.alt = entry.get<2>();
			key.code = entry.get<3>();
			hotKeys->Add(key);
		}
		remoteMessages.RequestIOUpdateGlobalShortcutKey(hotKeys);
	}

	vint GuiRemoteController::RegisterGlobalShortcutKey(bool ctrl, bool shift, bool alt, VKEY key)
	{
		HotKeyEntry entry = { ctrl,shift,alt,key };
		if (hotKeySet.Contains(entry)) return (vint)NativeGlobalShortcutKeyResult::Occupied;

		vint id = ++usedHotKeys;
		hotKeySet.Add(entry);
		hotKeyIds.Add(id, entry);

		UpdateGlobalShortcutKey();
		remoteMessages.Submit();

		return id;
	}
		
	bool GuiRemoteController::UnregisterGlobalShortcutKey(vint id)
	{
		vint index = hotKeyIds.Keys().IndexOf(id);
		if (index == -1) return false;

		auto entry = hotKeyIds.Values()[index];
		hotKeyIds.Remove(id);
		hotKeySet.Remove(entry);

		UpdateGlobalShortcutKey();
		remoteMessages.Submit();

		return true;
	}

/***********************************************************************
GuiRemoteController::INativeScreenService
***********************************************************************/

	vint GuiRemoteController::GetScreenCount()
	{
		return 1;
	}

	INativeScreen* GuiRemoteController::GetScreen(vint index)
	{
		CHECK_ERROR(index == 0, L"vl::presentation::GuiRemoteController::GetScreen(vint)#Index out of range.");
		return this;
	}

	INativeScreen* GuiRemoteController::GetScreen(INativeWindow* window)
	{
		return this;
	}

/***********************************************************************
GuiHostedController::INativeScreen
***********************************************************************/

	NativeRect GuiRemoteController::GetBounds()
	{
		return remoteScreenConfig.bounds;
	}

	NativeRect GuiRemoteController::GetClientBounds()
	{
		return remoteScreenConfig.clientBounds;
	}

	WString GuiRemoteController::GetName()
	{
		return WString::Unmanaged(L"GacUI Virtual Remote Screen");
	}

	bool GuiRemoteController::IsPrimary()
	{
		return true;
	}

	double GuiRemoteController::GetScalingX()
	{
		return remoteScreenConfig.scalingX;
	}

	double GuiRemoteController::GetScalingY()
	{
		return remoteScreenConfig.scalingY;
	}

/***********************************************************************
GuiRemoteController::INativeWindowService
***********************************************************************/

	const NativeWindowFrameConfig& GuiRemoteController::GetMainWindowFrameConfig()
	{
		return NativeWindowFrameConfig::Default;
	}

	const NativeWindowFrameConfig& GuiRemoteController::GetNonMainWindowFrameConfig()
	{
		return NativeWindowFrameConfig::Default;
	}

	INativeWindow* GuiRemoteController::CreateNativeWindow(INativeWindow::WindowMode windowMode)
	{
		CHECK_ERROR(!windowCreated, L"vl::presentation::GuiRemoteController::CreateNativeWindow(INativeWindow::WindowMode)#GuiHostedController is not supposed to call this function for twice.");
		windowCreated = true;
		remoteWindow.windowMode = windowMode;
		callbackService.InvokeNativeWindowCreated(&remoteWindow);
		return &remoteWindow;
	}

	void GuiRemoteController::DestroyNativeWindow(INativeWindow* window)
	{
		CHECK_ERROR(!windowDestroyed, L"vl::presentation::GuiRemoteController::CreateNativeWindow(INativeWindow::WindowMode)#GuiHostedController is not supposed to call this function for twice.");
		windowDestroyed = true;

		for (auto l : remoteWindow.listeners) l->Closed();
		for (auto l : remoteWindow.listeners) l->Destroying();
		callbackService.InvokeNativeWindowDestroying(&remoteWindow);
		for (auto l : remoteWindow.listeners) l->Destroyed();
		connectionStopped = true;
	}

	INativeWindow* GuiRemoteController::GetMainWindow()
	{
		return windowCreated  && !windowDestroyed ? &remoteWindow : nullptr;
	}

	INativeWindow* GuiRemoteController::GetWindow(NativePoint location)
	{
		return GetMainWindow();
	}

	void GuiRemoteController::Run(INativeWindow* window)
	{
		CHECK_ERROR(window == &remoteWindow, L"vl::presentation::GuiRemoteController::Run(INativeWindow*)#GuiHostedController should call this function with the native window.");
		applicationRunning = true;
		window->Show();
		while (RunOneCycle());
		applicationRunning = false;
	}

	bool GuiRemoteController::RunOneCycle()
	{
		if (!connectionStopped)
		{
			remoteProtocol->ProcessRemoteEvents();
			remoteMessages.Submit();
			if (timerEnabled)
			{
				callbackService.InvokeGlobalTimer();
			}
			asyncService.ExecuteAsyncTasks();
		}
		return !connectionStopped;
	}

/***********************************************************************
GuiRemoteController (events)
***********************************************************************/

	void GuiRemoteController::OnControllerConnect()
	{
		UpdateGlobalShortcutKey();
		vint idGetFontConfig = remoteMessages.RequestControllerGetFontConfig();
		vint idGetScreenConfig = remoteMessages.RequestControllerGetScreenConfig();
		remoteMessages.Submit();
		remoteFontConfig = remoteMessages.RetrieveControllerGetFontConfig(idGetFontConfig);
		remoteScreenConfig = remoteMessages.RetrieveControllerGetScreenConfig(idGetScreenConfig);
		remoteWindow.OnControllerConnect();
		imageService.OnControllerConnect();
		resourceManager->OnControllerConnect();
	}

	void GuiRemoteController::OnControllerDisconnect()
	{
		remoteWindow.OnControllerDisconnect();
		imageService.OnControllerDisconnect();
		resourceManager->OnControllerDisconnect();
	}

	void GuiRemoteController::OnControllerRequestExit()
	{
		remoteWindow.Hide(true);
	}

	void GuiRemoteController::OnControllerForceExit()
	{
		connectionForcedToStop = true;
		remoteWindow.Hide(true);
	}

	void GuiRemoteController::OnControllerScreenUpdated(const remoteprotocol::ScreenConfig& arguments)
	{
		remoteScreenConfig = arguments;
	}

/***********************************************************************
GuiRemoteController
***********************************************************************/

	GuiRemoteController::GuiRemoteController(IGuiRemoteProtocol* _remoteProtocol)
		: remoteProtocol(_remoteProtocol)
		, remoteMessages(this)
		, remoteEvents(this)
		, remoteWindow(this)
		, imageService(this)
	{
	}

	GuiRemoteController::~GuiRemoteController()
	{
	}

	void GuiRemoteController::Initialize()
	{
		remoteProtocol->Initialize(&remoteEvents);
		imageService.Initialize();
	}

	void GuiRemoteController::Finalize()
	{
		imageService.Finalize();
		remoteMessages.RequestControllerConnectionStopped();
		remoteMessages.Submit();
	}

/***********************************************************************
GuiRemoteController (INativeController)
***********************************************************************/

	INativeCallbackService* GuiRemoteController::CallbackService()
	{
		return &callbackService;
	}

	INativeResourceService* GuiRemoteController::ResourceService()
	{
		return this;
	}

	INativeAsyncService* GuiRemoteController::AsyncService()
	{
		return &asyncService;
	}

	INativeClipboardService* GuiRemoteController::ClipboardService()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	INativeImageService* GuiRemoteController::ImageService()
	{
		return &imageService;
	}

	INativeInputService* GuiRemoteController::InputService()
	{
		return this;
	}

	INativeDialogService* GuiRemoteController::DialogService()
	{
		// Use FakeDialogServiceBase
		return nullptr;
	}

	WString GuiRemoteController::GetExecutablePath()
	{
		return remoteProtocol->GetExecutablePath();
	}
		
	INativeScreenService* GuiRemoteController::ScreenService()
	{
		return this;
	}

	INativeWindowService* GuiRemoteController::WindowService()
	{
		return this;
	}
}