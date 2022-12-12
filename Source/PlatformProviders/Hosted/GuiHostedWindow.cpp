#include "GuiHostedWindow.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiHostedController
***********************************************************************/

		GuiHostedController::GuiHostedController(INativeController* _nativeController)
			: nativeController(_nativeController)
		{
		}

		GuiHostedController::~GuiHostedController()
		{
		}

		INativeCallbackService* GuiHostedController::CallbackService()
		{
			return nativeController->CallbackService();
		}

		INativeResourceService* GuiHostedController::ResourceService()
		{
			return nativeController->ResourceService();
		}

		INativeAsyncService* GuiHostedController::AsyncService()
		{
			return nativeController->AsyncService();
		}

		INativeClipboardService* GuiHostedController::ClipboardService()
		{
			return nativeController->ClipboardService();
		}

		INativeImageService* GuiHostedController::ImageService()
		{
			return nativeController->ImageService();
		}

		INativeInputService* GuiHostedController::InputService()
		{
			return nativeController->InputService();
		}

		INativeDialogService* GuiHostedController::DialogService()
		{
			return nativeController->DialogService();
		}

		WString GuiHostedController::GetExecutablePath()
		{
			return nativeController->GetExecutablePath();
		}
		
		INativeScreenService* GuiHostedController::ScreenService()
		{
			return this;
		}

		INativeWindowService* GuiHostedController::WindowService()
		{
			return this;
		}

		vint GuiHostedController::GetScreenCount()
		{
			return 1;
		}

		INativeScreen* GuiHostedController::GetScreen(vint index)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		INativeScreen* GuiHostedController::GetScreen(INativeWindow* window)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		INativeWindow* GuiHostedController::CreateNativeWindow(INativeWindow::WindowMode windowMode)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedController::DestroyNativeWindow(INativeWindow* window)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		INativeWindow* GuiHostedController::GetMainWindow()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		INativeWindow* GuiHostedController::GetWindow(NativePoint location)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedController::Run(INativeWindow* window)
		{
			CHECK_FAIL(L"Not implemented!");
		}
	}
}