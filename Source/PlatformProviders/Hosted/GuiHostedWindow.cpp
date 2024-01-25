#include "GuiHostedController.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiHostedWindow
***********************************************************************/

		void GuiHostedWindow::BecomeMainWindow()
		{
			proxy = CreateMainHostedWindowProxy(this, controller->nativeWindow);
			proxy->CheckAndSyncProperties();
		}

		void GuiHostedWindow::BecomeNonMainWindow()
		{
			proxy = CreateNonMainHostedWindowProxy(this, controller->nativeWindow);
			proxy->CheckAndSyncProperties();
		}

		void GuiHostedWindow::BecomeFocusedWindow()
		{
			CHECK_ERROR(&wmWindow == controller->wmManager->activeWindow, L"vl::presentation::GuiHostedWindow::BecomeFocusedWindow()#Wrong timing to call this function.");
			controller->nativeWindow->SetCaretPoint(windowCaretPoint + GetRenderingOffset());
		}

		void GuiHostedWindow::BecomeHoveringWindow()
		{
			CHECK_ERROR(this == controller->hoveringWindow, L"vl::presentation::GuiHostedWindow::BecomeFocusedWindow()#Wrong timing to call this function.");
			controller->nativeWindow->SetWindowCursor(windowCursor);
		}

		GuiHostedWindow::GuiHostedWindow(GuiHostedController* _controller, INativeWindow::WindowMode _windowMode)
			: GuiHostedWindowData(_controller, this, _windowMode)
		{
			wmWindow.bounds = { {0,0},{1,1} };
			proxy = CreatePlaceholderHostedWindowProxy(this);
			proxy->CheckAndSyncProperties();
		}

		GuiHostedWindow::~GuiHostedWindow()
		{
			for (auto listener : listeners)
			{
				listener->Destroyed();
			}
		}

		bool GuiHostedWindow::IsActivelyRefreshing()
		{
			return false;
		}

		NativeSize GuiHostedWindow::GetRenderingOffset()
		{
			auto pos = wmWindow.bounds.LeftTop();
			return { pos.x,pos.y };
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
			return wmWindow.bounds;
		}

		void GuiHostedWindow::SetBounds(const NativeRect& bounds)
		{
			auto fixedBounds = proxy->FixBounds(bounds);
			if (wmWindow.bounds == fixedBounds) return;
			wmWindow.SetBounds(fixedBounds);
			proxy->UpdateBounds();
		}

		NativeSize GuiHostedWindow::GetClientSize()
		{
			return GetBounds().GetSize();
		}

		void GuiHostedWindow::SetClientSize(NativeSize size)
		{
			SetBounds({ GetBounds().LeftTop(),size });
		}

		NativeRect GuiHostedWindow::GetClientBoundsInScreen()
		{
			return GetBounds();
		}

		WString GuiHostedWindow::GetTitle()
		{
			return windowTitle;
		}

		void GuiHostedWindow::SetTitle(const WString& title)
		{
			if (windowTitle == title) return;
			windowTitle = title;
			proxy->UpdateTitle();
		}

		INativeCursor* GuiHostedWindow::GetWindowCursor()
		{
			return windowCursor;
		}

		void GuiHostedWindow::SetWindowCursor(INativeCursor* cursor)
		{
			if (windowCursor == cursor) return;
			windowCursor = cursor;
			if (this == controller->hoveringWindow)
			{
				controller->nativeWindow->SetWindowCursor(windowCursor);
			}
		}

		NativePoint GuiHostedWindow::GetCaretPoint()
		{
			return windowCaretPoint;
		}

		void GuiHostedWindow::SetCaretPoint(NativePoint point)
		{
			if (windowCaretPoint == point) return;
			windowCaretPoint = point;
			if (&wmWindow == controller->wmManager->activeWindow)
			{
				controller->nativeWindow->SetCaretPoint(windowCaretPoint + GetRenderingOffset());
			}
		}

		INativeWindow* GuiHostedWindow::GetParent()
		{
			return wmWindow.parent ? wmWindow.parent->id : nullptr;
		}

		void GuiHostedWindow::SetParent(INativeWindow* parent)
		{
			auto hostedWindow = dynamic_cast<GuiHostedWindow*>(parent);
			CHECK_ERROR(!parent || hostedWindow, L"vl::presentation::GuiHostedWindow::SetParent(INativeWindow*)#The window is not created by GuiHostedController.");
			auto parentWindow = hostedWindow ? &hostedWindow->wmWindow : nullptr;
			if (wmWindow.parent == parentWindow) return;
			wmWindow.SetParent(parentWindow);
		}

		INativeWindow::WindowMode GuiHostedWindow::GetWindowMode()
		{
			return windowMode;
		}

		void GuiHostedWindow::EnableCustomFrameMode()
		{
			if (windowCustomFrameMode) return;
			windowCustomFrameMode = true;
			proxy->UpdateCustomFrameMode();
		}

		void GuiHostedWindow::DisableCustomFrameMode()
		{
			if (!windowCustomFrameMode) return;
			windowCustomFrameMode = false;
			proxy->UpdateCustomFrameMode();
		}

		bool GuiHostedWindow::IsCustomFrameModeEnabled()
		{
			return windowCustomFrameMode;
		}

		NativeMargin GuiHostedWindow::GetCustomFramePadding()
		{
			return controller->nativeWindow->GetCustomFramePadding();
		}

		Ptr<GuiImageData> GuiHostedWindow::GetIcon()
		{
			if (windowIcon) return windowIcon;
			if (controller->nativeWindow)return controller->nativeWindow->GetIcon();
			return nullptr;
		}

		void GuiHostedWindow::SetIcon(Ptr<GuiImageData> icon)
		{
			if (windowIcon == icon) return;
			windowIcon = icon;
			proxy->UpdateIcon();
		}

		INativeWindow::WindowSizeState GuiHostedWindow::GetSizeState()
		{
			return windowSizeState;
		}

		void GuiHostedWindow::Show()
		{
			EnableActivate();
			proxy->Show();
		}

		void GuiHostedWindow::ShowDeactivated()
		{
			proxy->ShowDeactivated();
		}

		void GuiHostedWindow::ShowRestored()
		{
			proxy->ShowRestored();
		}

		void GuiHostedWindow::ShowMaximized()
		{
			proxy->ShowMaximized();
		}

		void GuiHostedWindow::ShowMinimized()
		{
			proxy->ShowMinimized();
		}

		void GuiHostedWindow::Hide(bool closeWindow)
		{
			if (!wmWindow.visible) return;

			if (this != controller->mainWindow)
			{
				// for main window, the underlying INativeWindow will run the process
				bool cancel = false;
				for (auto listener : listeners)
				{
					listener->BeforeClosing(cancel);
					if (cancel) return;
				}
				for (auto listener : listeners)
				{
					listener->AfterClosing();
				}
			}

			if (closeWindow)
			{
				proxy->Close();
			}
			else
			{
				proxy->Hide();
			}
		}

		bool GuiHostedWindow::IsVisible()
		{
			return wmWindow.visible;
		}

		void GuiHostedWindow::Enable()
		{
			if (wmWindow.enabled) return;
			wmWindow.SetEnabled(true);
			proxy->UpdateEnabled();
		}

		void GuiHostedWindow::Disable()
		{
			if (!wmWindow.enabled) return;
			wmWindow.SetEnabled(false);
			proxy->UpdateEnabled();
		}

		bool GuiHostedWindow::IsEnabled()
		{
			return wmWindow.enabled;
		}

		void GuiHostedWindow::SetActivate()
		{
			EnableActivate();
			proxy->SetFocus();
		}

		bool GuiHostedWindow::IsActivated()
		{
			return wmWindow.active;
		}

		bool GuiHostedWindow::IsRenderingAsActivated()
		{
			return wmWindow.renderedAsActive;
		}

		void GuiHostedWindow::ShowInTaskBar()
		{
			if (windowShowInTaskBar) return;
			windowShowInTaskBar = true;
			proxy->UpdateShowInTaskBar();
		}

		void GuiHostedWindow::HideInTaskBar()
		{
			if (!windowShowInTaskBar) return;
			windowShowInTaskBar = false;
			proxy->UpdateShowInTaskBar();
		}

		bool GuiHostedWindow::IsAppearedInTaskBar()
		{
			return windowShowInTaskBar;
		}

		void GuiHostedWindow::EnableActivate()
		{
			if (windowEnabledActivate) return;
			windowEnabledActivate = true;
			proxy->UpdateEnabledActivate();
		}

		void GuiHostedWindow::DisableActivate()
		{
			if (!windowEnabledActivate) return;
			windowEnabledActivate = false;
			proxy->UpdateEnabledActivate();
		}

		bool GuiHostedWindow::IsEnabledActivate()
		{
			return windowEnabledActivate;
		}

		bool GuiHostedWindow::RequireCapture()
		{
			if (controller->capturingWindow) return false;
			controller->capturingWindow = this;
			controller->nativeWindow->RequireCapture();
			return true;
		}

		bool GuiHostedWindow::ReleaseCapture()
		{
			if (controller->capturingWindow != this) return false;
			controller->capturingWindow = nullptr;
			controller->nativeWindow->ReleaseCapture();
			controller->UpdateEnteringWindow(controller->hoveringWindow);
			return true;
		}

		bool GuiHostedWindow::IsCapturing()
		{
			return controller->capturingWindow == this;
		}

		bool GuiHostedWindow::GetMaximizedBox()
		{
			return windowMaximizedBox;
		}

		void GuiHostedWindow::SetMaximizedBox(bool visible)
		{
			if (windowMaximizedBox == visible) return;
			windowMaximizedBox = visible;
			proxy->UpdateMaximizedBox();
		}

		bool GuiHostedWindow::GetMinimizedBox()
		{
			return windowMinimizedBox;
		}

		void GuiHostedWindow::SetMinimizedBox(bool visible)
		{
			if (windowMinimizedBox == visible) return;
			windowMinimizedBox = visible;
			proxy->UpdateMinimizedBox();
		}

		bool GuiHostedWindow::GetBorder()
		{
			return windowBorder;
		}

		void GuiHostedWindow::SetBorder(bool visible)
		{
			if (windowBorder == visible) return;
			windowBorder = visible;
			proxy->UpdateBorderVisible();
		}

		bool GuiHostedWindow::GetSizeBox()
		{
			return windowSizeBox;
		}

		void GuiHostedWindow::SetSizeBox(bool visible)
		{
			if (windowSizeBox == visible) return;
			windowSizeBox = visible;
			proxy->UpdateSizeBox();
		}

		bool GuiHostedWindow::GetIconVisible()
		{
			return windowIconVisible;
		}

		void GuiHostedWindow::SetIconVisible(bool visible)
		{
			if (windowIconVisible == visible) return;
			windowIconVisible = visible;
			proxy->UpdateIconVisible();
		}

		bool GuiHostedWindow::GetTitleBar()
		{
			return windowTitleBar;
		}

		void GuiHostedWindow::SetTitleBar(bool visible)
		{
			if (windowTitleBar == visible) return;
			windowTitleBar = visible;
			proxy->UpdateTitleBar();
		}

		bool GuiHostedWindow::GetTopMost()
		{
			return wmWindow.topMost;
		}

		void GuiHostedWindow::SetTopMost(bool topmost)
		{
			if (wmWindow.topMost == topmost) return;
			wmWindow.SetTopMost(topmost);
			proxy->UpdateTopMost();
		}

		void GuiHostedWindow::SupressAlt()
		{
			controller->nativeWindow->SupressAlt();
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
		}
	}
}