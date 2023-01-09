#include "GuiHostedController.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiHostedController::WindowManager<GuiHostedWindow*>
***********************************************************************/

		void GuiHostedController::OnOpened(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->Opened();
			}
		}

		void GuiHostedController::OnClosed(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->Closed();
			}
		}

		void GuiHostedController::OnEnabled(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->Enabled();
			}
		}

		void GuiHostedController::OnDisabled(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->Disabled();
			}
		}

		void GuiHostedController::OnGotFocus(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->GotFocus();
			}
		}

		void GuiHostedController::OnLostFocus(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			window->id->BecomeFocusedWindow();
			for (auto listener : window->id->listeners)
			{
				listener->LostFocus();
			}
		}

		void GuiHostedController::OnActivated(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->RenderingAsActivated();
			}
		}

		void GuiHostedController::OnDeactivated(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->RenderingAsDeactivated();
			}
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
			// TODO: call mainWindow's Moving
		}

		void GuiHostedController::Moved()
		{
			// TODO: set mainWindow's size and call Moved
		}

		void GuiHostedController::DpiChanged()
		{
			for (auto hostedWindow : createdWindows)
			{
				for (auto listener : hostedWindow->listeners)
				{
					listener->DpiChanged();
				}
			}
		}

		void GuiHostedController::GotFocus()
		{
			if (lastFocusedWindow)
			{
				lastFocusedWindow->wmWindow.Activate();
				lastFocusedWindow = nullptr;
			}
			else if (mainWindow)
			{
				mainWindow->wmWindow.Activate();
			}
		}

		void GuiHostedController::LostFocus()
		{
			lastFocusedWindow = wmManager->activeWindow ? wmManager->activeWindow->id : nullptr;
			while (wmManager->activeWindow)
			{
				wmManager->activeWindow->Deactivate();
			}
		}

		void GuiHostedController::BeforeClosing(bool& cancel)
		{
			if (mainWindow)
			{
				for (auto listener : mainWindow->listeners)
				{
					listener->BeforeClosing(cancel);
					if (cancel) return;
				}
			}
		}

		void GuiHostedController::AfterClosing()
		{
			if (mainWindow)
			{
				for (auto listener : mainWindow->listeners)
				{
					listener->AfterClosing();
				}
			}
		}

		void GuiHostedController::Paint()
		{
			wmManager->needRefresh = true;
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

			if (hostedResourceManager && nativeWindow && nativeWindow->IsVisible())
			{
				if (wmManager->needRefresh)
				{
					wmManager->needRefresh = false;
					goto NEED_REFRESH;
				}
				for (auto hostedWindow : createdWindows)
				{
					for (auto listener : hostedWindow->listeners)
					{
						if (listener->NeedRefresh())
						{
							goto NEED_REFRESH;
						}
					}
				}
				return;

			NEED_REFRESH:
				while (true)
				{
					auto renderTarget = hostedResourceManager->nativeManager->GetRenderTarget(nativeWindow);
					renderTarget->StartHostedRendering();
					bool failureByResized = false;
					bool failureByLostDevice = false;

					for (vint i = wmManager->ordinaryWindowsInOrder.Count() - 1; i >= 0; i--)
					{
						auto hostedWindow = wmManager->ordinaryWindowsInOrder[i]->id;
						for (auto listener : hostedWindow->listeners)
						{
							listener->ForceRefresh(false, failureByResized, failureByLostDevice);
							if (failureByResized || failureByLostDevice)
							{
								goto STOP_RENDERING;
							}
						}
					}
					for (vint i = wmManager->topMostedWindowsInOrder.Count() - 1; i >= 0; i--)
					{
						auto hostedWindow = wmManager->topMostedWindowsInOrder[i]->id;
						for (auto listener : hostedWindow->listeners)
						{
							listener->ForceRefresh(false, failureByResized, failureByLostDevice);
							if (failureByResized || failureByLostDevice)
							{
								goto STOP_RENDERING;
							}
						}
					}

				STOP_RENDERING:
					renderTarget->StopHostedRendering();

					if (failureByLostDevice)
					{
						hostedResourceManager->nativeManager->RecreateRenderTarget(nativeWindow);
					}
					else if (failureByResized)
					{
						hostedResourceManager->nativeManager->ResizeRenderTarget(nativeWindow);
					}
					else
					{
						break;
					}
				}
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
			return nativeController->ScreenService()->GetScreen(nativeWindow)->GetScalingX();
		}

		double GuiHostedController::GetScalingY()
		{
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

			if (mainWindow)
			{
				hostedWindow->BecomeNonMainWindow();
			}
			return hostedWindow.Obj();
		}

		void GuiHostedController::DestroyNativeWindow(INativeWindow* window)
		{
			auto hostedWindow = dynamic_cast<GuiHostedWindow*>(window);
			CHECK_ERROR(!hostedWindow, L"vl::presentation::GuiHostedController::DestroyNativeWindow(INativeWindow*)#The window is not created by GuiHostedController.");
			vint index = createdWindows.IndexOf(hostedWindow);
			CHECK_ERROR(index != -1, L"vl::presentation::GuiHostedController::DestroyNativeWindow(INativeWindow*)#The window has been destroyed.");

			if (hostedWindow == hoveringWindow)
			{
				hoveringWindow = nullptr;
			}
			if (hostedWindow == lastFocusedWindow)
			{
				lastFocusedWindow = nullptr;
			}
			if (hostedWindow == capturingWindow)
			{
				capturingWindow = nullptr;
				nativeWindow->ReleaseCapture();
			}

			for (auto listener : hostedWindow->listeners)
			{
				listener->Destroying();
			}
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
			CHECK_ERROR(hostedWindow, L"vl::presentation::GuiHostedController::Run(INativeWindow*)#The window is not created by GuiHostedController.");
			mainWindow = hostedWindow;

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
			wmManager->needRefresh = true;
			nativeController->WindowService()->Run(nativeWindow);
			wmManager->Stop();

			for (vint i = createdWindows.Count() - 1; i >= 0; i--)
			{
				auto hostedWindow = createdWindows[i];
				DestroyNativeWindow(hostedWindow.Obj());
			}
		}

/***********************************************************************
GuiHostedController
***********************************************************************/

		GuiHostedController::GuiHostedController(INativeController* _nativeController)
			: nativeController(_nativeController)
		{
			wmManager = this;
			nativeController->CallbackService()->InstallListener(this);
		}

		GuiHostedController::~GuiHostedController()
		{
		}

		void GuiHostedController::Initialize()
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiHostedController()::Initialize()#"
			CHECK_ERROR(!nativeWindow, ERROR_MESSAGE_PREFIX L"Initialize() has been called");
			CHECK_ERROR(!nativeWindowDestroyed, ERROR_MESSAGE_PREFIX L"Finalize() has been called.");

			nativeController->CallbackService()->InstallListener(this);
			nativeWindow = nativeController->WindowService()->CreateNativeWindow(INativeWindow::WindowMode::Normal);
			nativeWindow->InstallListener(this);
#undef ERROR_MESSAGE_PREFIX
		}

		void GuiHostedController::Finalize()
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiHostedController()::Finalize()#"
			CHECK_ERROR(!nativeWindowDestroyed, ERROR_MESSAGE_PREFIX L"Finalize() has been called.");
			CHECK_ERROR(nativeWindow, ERROR_MESSAGE_PREFIX L"Initialize() has not been called");

			nativeWindow->UninstallListener(this);
			nativeController->WindowService()->DestroyNativeWindow(nativeWindow);
			nativeController->CallbackService()->UninstallListener(this);
			nativeWindow = nullptr;
			nativeWindowDestroyed = true;
#undef ERROR_MESSAGE_PREFIX
		}
	}
}