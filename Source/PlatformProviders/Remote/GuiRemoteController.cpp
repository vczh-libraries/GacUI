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
			CHECK_FAIL(L"Not Implemented!");
		}

		bool GuiRemoteController::IsKeyToggled(VKEY code)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		WString GuiRemoteController::GetKeyName(VKEY code)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		VKEY GuiRemoteController::GetKey(const WString& name)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		vint GuiRemoteController::RegisterGlobalShortcutKey(bool ctrl, bool shift, bool alt, VKEY key)
		{
			CHECK_FAIL(L"Not Implemented!");
		}
			
		bool GuiRemoteController::UnregisterGlobalShortcutKey(vint id)
		{
			CHECK_FAIL(L"Not Implemented!");
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
		CHECK_ERROR(!windowCreated, L"vl::presentation::GuiRemoteController::CreateNativeWindow(INativeWindow::WindowMode)#GuiHostedController is not supposed to call this function twice.");
		windowCreated = true;
		callbackService.InvokeNativeWindowCreated(&remoteWindow);
		return &remoteWindow;
	}

	void GuiRemoteController::DestroyNativeWindow(INativeWindow* window)
	{
		CHECK_ERROR(!windowDestroyed, L"vl::presentation::GuiRemoteController::CreateNativeWindow(INativeWindow::WindowMode)#GuiHostedController is not supposed to call this function twice.");
		windowDestroyed = true;

		for (auto listener : remoteWindow.listeners)
		{
			listener->Destroying();
		}
		callbackService.InvokeNativeWindowDestroying(&remoteWindow);
		for (auto listener : remoteWindow.listeners)
		{
			listener->Destroyed();
		}
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
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteController::RunOneCycle()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

/***********************************************************************
GuiRemoteController
***********************************************************************/

	GuiRemoteController::GuiRemoteController(IGuiRemoteProtocol* _remoteProtocol)
		: remoteProtocol(_remoteProtocol)
		, remoteEvents(this)
		, remoteWindow(this)
	{
	}

	GuiRemoteController::~GuiRemoteController()
	{
	}

	void GuiRemoteController::Initialize()
	{
		remoteProtocol->Initialize(&remoteEvents);
	}

	void GuiRemoteController::Finalize()
	{
	}

	void GuiRemoteController::OnControllerConnect()
	{
		vint idGetFontConfig = remoteEvents.RequestControllerGetFontConfig();
		vint idGetScreenConfig = remoteEvents.RequestControllerGetScreenConfig();
		remoteProtocol->Submit();
		remoteFontConfig = remoteEvents.RetrieveControllerGetFontConfig(idGetFontConfig);
		remoteScreenConfig = remoteEvents.RetrieveControllerGetScreenConfig(idGetScreenConfig);
		remoteEvents.ClearResponses();
		remoteWindow.OnControllerConnect();
	}

	void GuiRemoteController::OnControllerDisconnect()
	{
		remoteWindow.OnControllerDisconnect();
	}

	void GuiRemoteController::OnControllerExit()
	{
		remoteWindow.OnControllerExit();
	}

	void GuiRemoteController::OnControllerScreenUpdated(const remoteprotocol::ScreenConfig& arguments)
	{
		remoteScreenConfig = arguments;
	}

/***********************************************************************
GuiRemoteController::INativeController
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
		CHECK_FAIL(L"Not Implemented!");
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