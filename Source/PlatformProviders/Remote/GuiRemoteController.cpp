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
		// TODO:
		// Mark some messages (without response) and events (without request) with [@Override], means if multiple of same messages/events are sent only the last one count
		// Window's GetBounds and GetClientSize will check a dirty mark and send message asking for size if necessary
		// Using [@Override] to generate a IGuiRemoteProtocol (only for unit test and later becomes part of a standard implementation):
		//   Only send accumulated messages when Submit() is called
		//   If a batch of messages are sent, results will be sent back when both conditions are satisfies:
		//     1) All messages are processed
		//     2) All messages are responded
		//   Error will happen if new batch of messages come before sending back results of the last batch
		CHECK_ERROR(window == &remoteWindow, L"vl::presentation::GuiRemoteController::Run(INativeWindow*)#GuiHostedController should call this function with the native window.");
		window->Show();
		while (RunOneCycle());
		remoteMessages.Submit();
	}

	bool GuiRemoteController::RunOneCycle()
	{
		if (!connectionStopped)
		{
			remoteProtocol->ProcessRemoteEvents();
		}
		return connectionStopped;
	}

/***********************************************************************
GuiRemoteController
***********************************************************************/

	GuiRemoteController::GuiRemoteController(IGuiRemoteProtocol* _remoteProtocol)
		: remoteProtocol(_remoteProtocol)
		, remoteMessages(this)
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
		remoteMessages.RequestControllerConnectionStopped();
		remoteMessages.Submit();
	}

	void GuiRemoteController::OnControllerConnect()
	{
		vint idGetFontConfig = remoteMessages.RequestControllerGetFontConfig();
		vint idGetScreenConfig = remoteMessages.RequestControllerGetScreenConfig();
		remoteMessages.Submit();
		remoteFontConfig = remoteMessages.RetrieveControllerGetFontConfig(idGetFontConfig);
		remoteScreenConfig = remoteMessages.RetrieveControllerGetScreenConfig(idGetScreenConfig);
		remoteMessages.ClearResponses();
		remoteWindow.OnControllerConnect();
	}

	void GuiRemoteController::OnControllerDisconnect()
	{
		remoteWindow.OnControllerDisconnect();
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