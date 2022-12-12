#include "GuiHostedWindow.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiHostedWindow
***********************************************************************/

		GuiHostedWindow::GuiHostedWindow(GuiHostedController* _controller, INativeWindow::WindowMode _windowMode)
			: controller(_controller)
			, windowMode(_windowMode)
		{
		}

		GuiHostedWindow::~GuiHostedWindow()
		{
		}

		Point GuiHostedWindow::Convert(NativePoint value)
		{
			return controller->nativeWindow->Convert(value);
		}

		NativePoint GuiHostedWindow::Convert(Point value)
		{
			return controller->nativeWindow->Convert(value);
		}

		Size GuiHostedWindow::Convert(NativeSize value)
		{
			return controller->nativeWindow->Convert(value);
		}

		NativeSize GuiHostedWindow::Convert(Size value)
		{
			return controller->nativeWindow->Convert(value);
		}

		Margin GuiHostedWindow::Convert(NativeMargin value)
		{
			return controller->nativeWindow->Convert(value);
		}

		NativeMargin GuiHostedWindow::Convert(Margin value)
		{
			return controller->nativeWindow->Convert(value);
		}

		NativeRect GuiHostedWindow::GetBounds()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetBounds(const NativeRect& bounds)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		NativeSize GuiHostedWindow::GetClientSize()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetClientSize(NativeSize size)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		NativeRect GuiHostedWindow::GetClientBoundsInScreen()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		WString GuiHostedWindow::GetTitle()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetTitle(WString title)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		INativeCursor* GuiHostedWindow::GetWindowCursor()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetWindowCursor(INativeCursor* cursor)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		NativePoint GuiHostedWindow::GetCaretPoint()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetCaretPoint(NativePoint point)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		INativeWindow* GuiHostedWindow::GetParent()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetParent(INativeWindow* parent)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		INativeWindow::WindowMode GuiHostedWindow::GetWindowMode()
		{
			return windowMode;
		}

		void GuiHostedWindow::EnableCustomFrameMode()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::DisableCustomFrameMode()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::IsCustomFrameModeEnabled()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		NativeMargin GuiHostedWindow::GetCustomFramePadding()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		Ptr<GuiImageData> GuiHostedWindow::GetIcon()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetIcon(Ptr<GuiImageData> icon)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		INativeWindow::WindowSizeState GuiHostedWindow::GetSizeState()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::Show()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::ShowDeactivated()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::ShowRestored()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::ShowMaximized()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::ShowMinimized()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::Hide(bool closeWindow)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::IsVisible()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::Enable()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::Disable()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::IsEnabled()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetFocus()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::IsFocused()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetActivate()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::IsActivated()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::ShowInTaskBar()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::HideInTaskBar()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::IsAppearedInTaskBar()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::EnableActivate()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::DisableActivate()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::IsEnabledActivate()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::RequireCapture()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::ReleaseCapture()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::IsCapturing()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::GetMaximizedBox()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetMaximizedBox(bool visible)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::GetMinimizedBox()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetMinimizedBox(bool visible)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::GetBorder()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetBorder(bool visible)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::GetSizeBox()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetSizeBox(bool visible)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::GetIconVisible()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetIconVisible(bool visible)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::GetTitleBar()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetTitleBar(bool visible)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::GetTopMost()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SetTopMost(bool topmost)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::SupressAlt()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		bool GuiHostedWindow::InstallListener(INativeWindowListener* listener)
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

		bool GuiHostedWindow::UninstallListener(INativeWindowListener* listener)
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

		void GuiHostedWindow::RedrawContent()
		{
			CHECK_FAIL(L"Not implemented!");
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
GuiHostedController::INativeController
***********************************************************************/

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
			return hostedWindow.Obj();
		}

		void GuiHostedController::DestroyNativeWindow(INativeWindow* window)
		{
			auto hostedWindow = dynamic_cast<GuiHostedWindow*>(window);
			CHECK_ERROR(!hostedWindow, L"vl::presentation::GuiHostedController::DestroyNativeWindow(INativeWindow*)#The window is not created by GuiHostedController.");
			vint index = createdWindows.IndexOf(hostedWindow);
			CHECK_ERROR(index != -1, L"vl::presentation::GuiHostedController::DestroyNativeWindow(INativeWindow*)#The window has been destroyed.");
			createdWindows.RemoveAt(index);
		}

		INativeWindow* GuiHostedController::GetMainWindow()
		{
			return mainWindow;
		}

		INativeWindow* GuiHostedController::GetWindow(NativePoint location)
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedController::Run(INativeWindow* window)
		{
			CHECK_ERROR(!mainWindow, L"vl::presentation::GuiHostedController::Run(INativeWindow*)#This function has been called.");
			auto hostedWindow = dynamic_cast<GuiHostedWindow*>(window);
			CHECK_ERROR(!hostedWindow, L"vl::presentation::GuiHostedController::Run(INativeWindow*)#The window is not created by GuiHostedController.");
			mainWindow = hostedWindow;

			// TODO:
			//   sync window properties to nativeWindow
			//   check main window properties
			//     no parent
			//   check non-main window properties
			//     CustomFrameMode should be true to render the frame using templates
			//     for normal windows, parent should be either null or the main window
			//       if it is null, it is treated to be the main window
			//     for other windows, parent should be non-null
			//     ensure parent is partial ordered in realtime
			//   sync non-main window window-management properties
			//     changing activated or focused etc before calling Run() are ignored

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

			nativeController->WindowService()->Run(nativeWindow);
			mainWindow = nullptr;
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
		}

		GuiHostedController::~GuiHostedController()
		{
			EnsureNativeWindowDestroyed();
		}
	}
}