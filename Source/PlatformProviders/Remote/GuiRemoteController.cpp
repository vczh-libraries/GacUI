#include "GuiRemoteController.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
GuiRemoteController::INativeScreenService
***********************************************************************/

		vint GuiRemoteController::GetScreenCount()
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		INativeScreen* GuiRemoteController::GetScreen(vint index)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		INativeScreen* GuiRemoteController::GetScreen(INativeWindow* window)
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
			CHECK_FAIL(L"Not Implemented!");
		}

		INativeWindow* GuiRemoteController::GetWindow(NativePoint location)
		{
			CHECK_FAIL(L"Not Implemented!");
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

		GuiRemoteController::GuiRemoteController()
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
			CHECK_FAIL(L"Not Implemented!");
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
			CHECK_FAIL(L"Not Implemented!");
		}

		INativeDialogService* GuiRemoteController::DialogService()
		{
			// Use FakeDialogServiceBase
			return nullptr;
		}

		WString GuiRemoteController::GetExecutablePath()
		{
			CHECK_FAIL(L"Not Implemented!");
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
}

/***********************************************************************
SetupRemoteNativeController
***********************************************************************/

int SetupRemoteNativeController(vl::presentation::IGuiRemoteProtocol* protocol)
{
	vl::presentation::GuiRemoteController();
	return 0;
}