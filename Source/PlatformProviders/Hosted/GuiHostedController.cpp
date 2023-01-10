#include "GuiHostedController.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiHostedController
***********************************************************************/

		NativePoint GuiHostedController::GetPointInClientSpace(NativePoint location)
		{
			auto bounds = nativeWindow->GetBounds();
			auto clientBounds = nativeWindow->GetClientBoundsInScreen();
			location.x.value += bounds.x1.value - clientBounds.x1.value;
			location.y.value += bounds.y1.value - clientBounds.y1.value;
			return location;
		}

		GuiHostedWindow* GuiHostedController::HitTestInClientSpace(NativePoint location)
		{
			auto window = wmManager->HitTest(location);
			return window ? window->id : nullptr;
		}

		void GuiHostedController::UpdateEnteringWindow(GuiHostedWindow* window)
		{
			if (enteringWindow != window)
			{
				if (enteringWindow)
				{
					for (auto listener : enteringWindow->listeners)
					{
						listener->MouseLeaved();
					}
				}
				enteringWindow = window;
				if (enteringWindow)
				{
					for (auto listener : enteringWindow->listeners)
					{
						listener->MouseEntered();
					}
				}
			}
		}

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
			if (mainWindow)
			{
				auto point = GetPointInClientSpace(location);
				auto window = HitTestInClientSpace(point);
				if (window == mainWindow)
				{
					return PerformHitTest(From(mainWindow->listeners), point);
				}
			}
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

#define IMPLEMENT_MOUSE_CALLBACK(NAME)													\
		void GuiHostedController::NAME(const NativeWindowMouseInfo& info)				\
		{																				\
			auto selectedWindow = capturingWindow ? capturingWindow : hoveringWindow;	\
			if (selectedWindow)															\
			{																			\
				auto adjustedInfo = info;												\
				adjustedInfo.x.value -= selectedWindow->wmWindow.bounds.x1.value;		\
				adjustedInfo.y.value -= selectedWindow->wmWindow.bounds.y1.value;		\
																						\
				for (auto listener : selectedWindow->listeners)							\
				{																		\
					listener->NAME(adjustedInfo);										\
				}																		\
			}																			\
		}																				\

		IMPLEMENT_MOUSE_CALLBACK(LeftButtonDown)
		IMPLEMENT_MOUSE_CALLBACK(LeftButtonUp)
		IMPLEMENT_MOUSE_CALLBACK(LeftButtonDoubleClick)
		IMPLEMENT_MOUSE_CALLBACK(RightButtonDown)
		IMPLEMENT_MOUSE_CALLBACK(RightButtonUp)
		IMPLEMENT_MOUSE_CALLBACK(RightButtonDoubleClick)
		IMPLEMENT_MOUSE_CALLBACK(MiddleButtonDown)
		IMPLEMENT_MOUSE_CALLBACK(MiddleButtonUp)
		IMPLEMENT_MOUSE_CALLBACK(MiddleButtonDoubleClick)
		IMPLEMENT_MOUSE_CALLBACK(HorizontalWheel)
		IMPLEMENT_MOUSE_CALLBACK(VerticalWheel)

#undef IMPLEMENT_MOUSE_CALLBACK

		void GuiHostedController::MouseMoving(const NativeWindowMouseInfo& info)
		{
			hoveringWindow = HitTestInClientSpace({ info.x,info.y });
			auto selectedWindow = capturingWindow ? capturingWindow : hoveringWindow;
			UpdateEnteringWindow(selectedWindow);

			if (selectedWindow)
			{
				auto adjustedInfo = info;
				adjustedInfo.x.value -= selectedWindow->wmWindow.bounds.x1.value;
				adjustedInfo.y.value -= selectedWindow->wmWindow.bounds.y1.value;

				for (auto listener : selectedWindow->listeners)
				{
					listener->MouseMoving(adjustedInfo);
				}
			}
		}

		void GuiHostedController::MouseEntered()
		{
		}

		void GuiHostedController::MouseLeaved()
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiHostedController::MouseLeaved()#"
			CHECK_ERROR(!capturingWindow, ERROR_MESSAGE_PREFIX L"This callback is not supposed to be called when capturing.");
			UpdateEnteringWindow(nullptr);
			hoveringWindow = nullptr;
#undef ERROR_MESSAGE_PREFIX
		}

#define IMPLEMENT_KEY_CALLBACK(NAME, TYPE)										\
		void GuiHostedController::NAME(const TYPE& info)						\
		{																		\
			if (wmManager->activeWindow)										\
			{																	\
				auto hostedWindow = wmManager->activeWindow->id;				\
				for (auto listener : hostedWindow->listeners)					\
				{																\
					listener->NAME(info);										\
				}																\
			}																	\
		}																		\

		IMPLEMENT_KEY_CALLBACK(KeyDown, NativeWindowKeyInfo)
		IMPLEMENT_KEY_CALLBACK(KeyUp, NativeWindowKeyInfo)
		IMPLEMENT_KEY_CALLBACK(SysKeyDown, NativeWindowKeyInfo)
		IMPLEMENT_KEY_CALLBACK(SysKeyUp, NativeWindowKeyInfo)
		IMPLEMENT_KEY_CALLBACK(Char, NativeWindowCharInfo)

#undef IMPLEMENT_KEY_CALLBACK

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
			return nativeController->AsyncService()->IsInMainThread(nativeWindow);
		}

		void GuiHostedController::InvokeAsync(const Func<void()>& proc)
		{
			return nativeController->AsyncService()->InvokeAsync(proc);
		}

		void GuiHostedController::InvokeInMainThread(INativeWindow* window, const Func<void()>& proc)
		{
			return nativeController->AsyncService()->InvokeInMainThread(nativeWindow, proc);
		}

		bool GuiHostedController::InvokeInMainThreadAndWait(INativeWindow* window, const Func<void()>& proc, vint milliseconds)
		{
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
			return nativeController->DialogService()->ShowMessageBox(nativeWindow, text, title, buttons, defaultButton, icon, modal);
		}

		bool GuiHostedController::ShowColorDialog(INativeWindow* window, Color& selection, bool selected, ColorDialogCustomColorOptions customColorOptions, Color* customColors)
		{
			return nativeController->DialogService()->ShowColorDialog(nativeWindow, selection, selected, customColorOptions, customColors);
		}

		bool GuiHostedController::ShowFontDialog(INativeWindow* window, FontProperties& selectionFont, Color& selectionColor, bool selected, bool showEffect, bool forceFontExist)
		{
			return nativeController->DialogService()->ShowFontDialog(nativeWindow, selectionFont, selectionColor, selected, showEffect, forceFontExist);
		}

		bool GuiHostedController::ShowFileDialog(INativeWindow* window, collections::List<WString>& selectionFileNames, vint& selectionFilterIndex, FileDialogTypes dialogType, const WString& title, const WString& initialFileName, const WString& initialDirectory, const WString& defaultExtension, const WString& filter, FileDialogOptions options)
		{
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
			CHECK_ERROR(index == 0, L"vl::presentation::GuiHostedController::GetScreen(vint)#Index out of range.");
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
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiHostedController::DestroyNativeWindow(INativeWindow*)#"
			auto hostedWindow = dynamic_cast<GuiHostedWindow*>(window);
			CHECK_ERROR(!hostedWindow, ERROR_MESSAGE_PREFIX L"The window is not created by GuiHostedController.");
			vint index = createdWindows.IndexOf(hostedWindow);
			CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"The window has been destroyed.");

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
				capturingWindow->ReleaseCapture();
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
#undef ERROR_MESSAGE_PREFIX
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
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiHostedController::Run(INativeWindow*)#"
			CHECK_ERROR(!mainWindow, ERROR_MESSAGE_PREFIX L"This function has been called.");
			auto hostedWindow = dynamic_cast<GuiHostedWindow*>(window);
			CHECK_ERROR(hostedWindow, ERROR_MESSAGE_PREFIX L"The window is not created by GuiHostedController.");
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
#undef ERROR_MESSAGE_PREFIX
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