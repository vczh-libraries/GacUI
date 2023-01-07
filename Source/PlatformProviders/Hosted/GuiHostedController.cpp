#include "GuiHostedController.h"

namespace vl
{
	namespace presentation
	{

// =============================================================
// GuiHostedController::WindowManager<GuiHostedWindow*>
// =============================================================

		void GuiHostedController::OnOpened(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
		}

		void GuiHostedController::OnClosed(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
		}

		void GuiHostedController::OnEnabled(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
		}

		void GuiHostedController::OnDisabled(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
		}

		void GuiHostedController::OnGotFocus(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
		}

		void GuiHostedController::OnLostFocus(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
		}

		void GuiHostedController::OnActivated(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
		}

		void GuiHostedController::OnDeactivated(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
		}


/***********************************************************************
GuiHostedController::INativeWindowListener
***********************************************************************/

		INativeWindowListener::HitTestResult GuiHostedController::HitTest(NativePoint location)
		{
			return INativeWindowListener::HitTestResult::NoDecision;
		}

		void GuiHostedController::Moving(NativeRect& bounds, bool fixSizeOnly, bool draggingBorder)
		{
		}

		void GuiHostedController::Moved()
		{
		}

		void GuiHostedController::DpiChanged()
		{
		}

		void GuiHostedController::Enabled()
		{
		}

		void GuiHostedController::Disabled()
		{
		}

		void GuiHostedController::GotFocus()
		{
		}

		void GuiHostedController::LostFocus()
		{
		}

		void GuiHostedController::Activated()
		{
		}

		void GuiHostedController::Deactivated()
		{
		}

		void GuiHostedController::Opened()
		{
		}

		void GuiHostedController::Closing(bool& cancel)
		{
		}

		void GuiHostedController::Closed()
		{
		}

		void GuiHostedController::Paint()
		{
		}

		void GuiHostedController::Destroying()
		{
		}

		void GuiHostedController::Destroyed()
		{
		}

		void GuiHostedController::LeftButtonDown(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::LeftButtonUp(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::LeftButtonDoubleClick(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::RightButtonDown(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::RightButtonUp(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::RightButtonDoubleClick(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::MiddleButtonDown(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::MiddleButtonUp(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::MiddleButtonDoubleClick(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::HorizontalWheel(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::VerticalWheel(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::MouseMoving(const NativeWindowMouseInfo& info)
		{
		}

		void GuiHostedController::MouseEntered()
		{
		}

		void GuiHostedController::MouseLeaved()
		{
		}

		void GuiHostedController::KeyDown(const NativeWindowKeyInfo& info)
		{
		}

		void GuiHostedController::KeyUp(const NativeWindowKeyInfo& info)
		{
		}

		void GuiHostedController::SysKeyDown(const NativeWindowKeyInfo& info)
		{
		}

		void GuiHostedController::SysKeyUp(const NativeWindowKeyInfo& info)
		{
		}

		void GuiHostedController::Char(const NativeWindowCharInfo& info)
		{
		}

/***********************************************************************
GuiHostedController::INativeControllerListener
***********************************************************************/

		void GuiHostedController::GlobalTimer()
		{
			for (auto listener : listeners)
			{
				listener->GlobalTimer();
			}

			if (hostedResourceManager)
			{
				// TODO: call all visible INativeWindowListener::NeedRefresh
				auto renderTarget = hostedResourceManager->nativeManager->GetRenderTarget(nativeWindow);
				renderTarget->StartHostedRendering();
				// TODO: call all visible INativeWindowListener::ForceRefresh
				renderTarget->StopRendering();
			}
		}

		void GuiHostedController::ClipboardUpdated()
		{
			for (auto listener : listeners)
			{
				listener->ClipboardUpdated();
			}
		}

/***********************************************************************
GuiHostedController::INativeController
***********************************************************************/

		INativeCallbackService* GuiHostedController::CallbackService()
		{
			return this;
		}

		INativeResourceService* GuiHostedController::ResourceService()
		{
			return nativeController->ResourceService();
		}

		INativeAsyncService* GuiHostedController::AsyncService()
		{
			return this;
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
			return this;
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

/***********************************************************************
GuiHostedController::INativeCallbackService
***********************************************************************/

		bool GuiHostedController::InstallListener(INativeControllerListener* listener)
		{
			if (listeners.Contains(listener))
			{
				return false;
			}
			else
			{
				listeners.Add(listener);
				return true;
			}
		}

		bool GuiHostedController::UninstallListener(INativeControllerListener* listener)
		{
			if (listeners.Contains(listener))
			{
				listeners.Remove(listener);
				return true;
			}
			else
			{
				return false;
			}
		}

/***********************************************************************
GuiHostedController::INativeAsyncService
***********************************************************************/

		bool GuiHostedController::IsInMainThread(INativeWindow* window)
		{
			CHECK_ERROR(nativeWindow, L"vl::presentation::GuiHostedController::IsInMainThread(INativeWindow*)#The underlying native window has not been created.");
			return nativeController->AsyncService()->IsInMainThread(nativeWindow);
		}

		void GuiHostedController::InvokeAsync(const Func<void()>& proc)
		{
			return nativeController->AsyncService()->InvokeAsync(proc);
		}

		void GuiHostedController::InvokeInMainThread(INativeWindow* window, const Func<void()>& proc)
		{
			CHECK_ERROR(nativeWindow, L"vl::presentation::GuiHostedController::InvokeInMainThread(INativeWindow*, ...)#The underlying native window has not been created.");
			return nativeController->AsyncService()->InvokeInMainThread(nativeWindow, proc);
		}

		bool GuiHostedController::InvokeInMainThreadAndWait(INativeWindow* window, const Func<void()>& proc, vint milliseconds)
		{
			CHECK_ERROR(nativeWindow, L"vl::presentation::GuiHostedController::InvokeInMainThreadAndWait(INativeWindow*, ...)#The underlying native window has not been created.");
			return nativeController->AsyncService()->InvokeInMainThreadAndWait(nativeWindow, proc, milliseconds);
		}

		Ptr<INativeDelay> GuiHostedController::DelayExecute(const Func<void()>& proc, vint milliseconds)
		{
			return nativeController->AsyncService()->DelayExecute(proc, milliseconds);
		}

		Ptr<INativeDelay> GuiHostedController::DelayExecuteInMainThread(const Func<void()>& proc, vint milliseconds)
		{
			return nativeController->AsyncService()->DelayExecuteInMainThread(proc, milliseconds);
		}

/***********************************************************************
GuiHostedController::INativeDialogService
***********************************************************************/

		INativeDialogService::MessageBoxButtonsOutput GuiHostedController::ShowMessageBox(INativeWindow* window, const WString& text, const WString& title, MessageBoxButtonsInput buttons, MessageBoxDefaultButton defaultButton, MessageBoxIcons icon, MessageBoxModalOptions modal)
		{
			CHECK_ERROR(nativeWindow, L"vl::presentation::GuiHostedController::ShowMessageBox(INativeWindow*, ...)#The underlying native window has not been created.");
			return nativeController->DialogService()->ShowMessageBox(nativeWindow, text, title, buttons, defaultButton, icon, modal);
		}

		bool GuiHostedController::ShowColorDialog(INativeWindow* window, Color& selection, bool selected, ColorDialogCustomColorOptions customColorOptions, Color* customColors)
		{
			CHECK_ERROR(nativeWindow, L"vl::presentation::GuiHostedController::ShowColorDialog(INativeWindow*, ...)#The underlying native window has not been created.");
			return nativeController->DialogService()->ShowColorDialog(nativeWindow, selection, selected, customColorOptions, customColors);
		}

		bool GuiHostedController::ShowFontDialog(INativeWindow* window, FontProperties& selectionFont, Color& selectionColor, bool selected, bool showEffect, bool forceFontExist)
		{
			CHECK_ERROR(nativeWindow, L"vl::presentation::GuiHostedController::ShowFontDialog(INativeWindow*, ...)#The underlying native window has not been created.");
			return nativeController->DialogService()->ShowFontDialog(nativeWindow, selectionFont, selectionColor, selected, showEffect, forceFontExist);
		}

		bool GuiHostedController::ShowFileDialog(INativeWindow* window, collections::List<WString>& selectionFileNames, vint& selectionFilterIndex, FileDialogTypes dialogType, const WString& title, const WString& initialFileName, const WString& initialDirectory, const WString& defaultExtension, const WString& filter, FileDialogOptions options)
		{
			CHECK_ERROR(nativeWindow, L"vl::presentation::GuiHostedController::ShowFileDialog(INativeWindow*, ...)#The underlying native window has not been created.");
			return nativeController->DialogService()->ShowFileDialog(nativeWindow, selectionFileNames, selectionFilterIndex, dialogType, title, initialFileName, initialDirectory, defaultExtension, filter, options);
		}

/***********************************************************************
GuiHostedController::INativeScreenService
***********************************************************************/

		vint GuiHostedController::GetScreenCount()
		{
			return 1;
		}

		INativeScreen* GuiHostedController::GetScreen(vint index)
		{
			return this;
		}

		INativeScreen* GuiHostedController::GetScreen(INativeWindow* window)
		{
			return this;
		}

/***********************************************************************
GuiHostedController::INativeScreen
***********************************************************************/

		NativeRect GuiHostedController::GetBounds()
		{
			EnsureNativeWindowCreated();
			if (nativeWindow->IsCustomFrameModeEnabled())
			{
				return { {},nativeWindow->GetBounds().GetSize() };
			}
			else
			{
				return { {},nativeWindow->GetClientSize() };
			}
		}

		NativeRect GuiHostedController::GetClientBounds()
		{
			EnsureNativeWindowCreated();
			return { {},nativeWindow->GetClientSize() };
		}

		WString GuiHostedController::GetName()
		{
			return WString::Unmanaged(L"GacUI Virtual Screen");
		}

		bool GuiHostedController::IsPrimary()
		{
			return true;
		}

		double GuiHostedController::GetScalingX()
		{
			EnsureNativeWindowCreated();
			return nativeController->ScreenService()->GetScreen(nativeWindow)->GetScalingX();
		}

		double GuiHostedController::GetScalingY()
		{
			EnsureNativeWindowCreated();
			return nativeController->ScreenService()->GetScreen(nativeWindow)->GetScalingY();
		}

/***********************************************************************
GuiHostedController::INativeWindowService
***********************************************************************/

		INativeWindow* GuiHostedController::CreateNativeWindow(INativeWindow::WindowMode windowMode)
		{
			auto hostedWindow = Ptr(new GuiHostedWindow(this, windowMode));
			createdWindows.Add(hostedWindow);
			wmManager->RegisterWindow(&hostedWindow->wmWindow);

			for (auto listener : listeners)
			{
				listener->NativeWindowCreated(hostedWindow.Obj());
			}
			return hostedWindow.Obj();
		}

		void GuiHostedController::DestroyNativeWindow(INativeWindow* window)
		{
			auto hostedWindow = dynamic_cast<GuiHostedWindow*>(window);
			CHECK_ERROR(!hostedWindow, L"vl::presentation::GuiHostedController::DestroyNativeWindow(INativeWindow*)#The window is not created by GuiHostedController.");
			vint index = createdWindows.IndexOf(hostedWindow);
			CHECK_ERROR(index != -1, L"vl::presentation::GuiHostedController::DestroyNativeWindow(INativeWindow*)#The window has been destroyed.");

			for (auto listener : listeners)
			{
				listener->NativeWindowDestroying(hostedWindow);
			}

			wmManager->UnregisterWindow(&hostedWindow->wmWindow);
			createdWindows.RemoveAt(index);
		}

		INativeWindow* GuiHostedController::GetMainWindow()
		{
			return mainWindow;
		}

		INativeWindow* GuiHostedController::GetWindow(NativePoint location)
		{
			auto wmWindow = wmManager->HitTest(location);
			return wmWindow ? wmWindow->id : nullptr;
		}

		void GuiHostedController::Run(INativeWindow* window)
		{
			CHECK_ERROR(!mainWindow, L"vl::presentation::GuiHostedController::Run(INativeWindow*)#This function has been called.");
			auto hostedWindow = dynamic_cast<GuiHostedWindow*>(window);
			CHECK_ERROR(!hostedWindow, L"vl::presentation::GuiHostedController::Run(INativeWindow*)#The window is not created by GuiHostedController.");
			mainWindow = hostedWindow;
			focusedWindow = hostedWindow;

			for (auto window : createdWindows)
			{
				if (window == mainWindow)
				{
					window->BecomeMainWindow();
				}
				else
				{
					window->BecomeNonMainWindow();
				}
			}

			if (auto screen = nativeController->ScreenService()->GetScreen(nativeWindow))
			{
				auto screenBounds = screen->GetClientBounds();
				auto windowSize = nativeWindow->GetBounds().GetSize();
				nativeWindow->SetBounds({
					{
						screenBounds.Left() + (screenBounds.Width() - windowSize.x) / 2,
						screenBounds.Top() + (screenBounds.Height() - windowSize.y) / 2
					},
					windowSize
					});
			}

			wmManager->Start(&mainWindow->wmWindow);
			nativeController->WindowService()->Run(nativeWindow);
			wmManager->Stop();

			mainWindow = nullptr;
			focusedWindow = nullptr;
			capturingWindow = nullptr;
			hoveringWindow = nullptr;
			EnsureNativeWindowDestroyed();
		}

/***********************************************************************
GuiHostedController
***********************************************************************/

		void GuiHostedController::EnsureNativeWindowCreated()
		{
			CHECK_ERROR(!nativeWindowDestroyed, L"vl::presentation::GuiHostedController()::EnsureNativeWindowCreated()#The underlying native window has been destroyed.");
			if (!nativeWindow)
			{
				nativeWindow = nativeController->WindowService()->CreateNativeWindow(INativeWindow::WindowMode::Normal);
				nativeWindow->InstallListener(this);
			}
		}

		void GuiHostedController::EnsureNativeWindowDestroyed()
		{
			if (nativeWindow)
			{
				nativeWindow->UninstallListener(this);
				nativeController->WindowService()->DestroyNativeWindow(nativeWindow);
				nativeWindow = nullptr;
				nativeWindowDestroyed = true;
			}
		}

		GuiHostedController::GuiHostedController(INativeController* _nativeController)
			: nativeController(_nativeController)
		{
			wmManager = this;
			nativeController->CallbackService()->InstallListener(this);
		}

		GuiHostedController::~GuiHostedController()
		{
			EnsureNativeWindowDestroyed();
			nativeController->CallbackService()->UninstallListener(this);
		}
	}
}