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
			CHECK_FAIL(L"Not Implemented (from init)!");
		}

		void GuiRemoteController::SetDefaultFont(const FontProperties& value)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void GuiRemoteController::EnumerateFonts(collections::List<WString>& fonts)
		{
			CHECK_FAIL(L"Not Implemented (from init)!");
		}
			
/***********************************************************************
GuiRemoteController::INativeInputService
***********************************************************************/

		void GuiRemoteController::StartTimer()
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void GuiRemoteController::StopTimer()
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		bool GuiRemoteController::IsTimerEnabled()
		{
			CHECK_FAIL(L"Not Implemented!");
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
		CHECK_FAIL(L"vl::presentation::GuiRemoteController::GetBounds()#GuiHostedController is not supposed to call this function.");
	}

	NativeRect GuiRemoteController::GetClientBounds()
	{
		CHECK_FAIL(L"vl::presentation::GuiRemoteController::GetClientBounds()#GuiHostedController is not supposed to call this function.");
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
		CHECK_FAIL(L"Not Implemented!");
	}

	double GuiRemoteController::GetScalingY()
	{
		CHECK_FAIL(L"Not Implemented!");
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
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteController::DestroyNativeWindow(INativeWindow* window)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	INativeWindow* GuiRemoteController::GetMainWindow()
	{
		return &remoteWindow;
	}

	INativeWindow* GuiRemoteController::GetWindow(NativePoint location)
	{
		CHECK_FAIL(L"vl::presentation::GuiRemoteController::GetWindow(NativePoint)#GuiHostedController is not supposed to call this function.");
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
		, remoteEvent(this)
		, remoteWindow(this)
	{
	}

	GuiRemoteController::~GuiRemoteController()
	{
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
		CHECK_FAIL(L"Not Implemented (from config)!");
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