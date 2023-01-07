#include "GuiHostedController.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiHostedWindow
***********************************************************************/

		void GuiHostedWindow::SyncWindowProperties()
		{
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
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::BecomeMainWindow()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::BecomeNonMainWindow()
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void GuiHostedWindow::BecomeFocusedWindow()
		{
			CHECK_ERROR(this == controller->focusedWindow, L"vl::presentation::GuiHostedWindow::BecomeFocusedWindow()#Wrong timing to call this function.");
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
		}

		GuiHostedWindow::~GuiHostedWindow()
		{
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

		void GuiHostedWindow::SetTitle(WString title)
		{
			windowTitle = title;
			proxy->UpdateTitle();
		}

		INativeCursor* GuiHostedWindow::GetWindowCursor()
		{
			return windowCursor;
		}

		void GuiHostedWindow::SetWindowCursor(INativeCursor* cursor)
		{
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
			windowCaretPoint = point;
			if (this == controller->focusedWindow)
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
			wmWindow.SetParent(&dynamic_cast<GuiHostedWindow*>(parent)->wmWindow);
		}

		INativeWindow::WindowMode GuiHostedWindow::GetWindowMode()
		{
			return windowMode;
		}

		void GuiHostedWindow::EnableCustomFrameMode()
		{
			proxy->EnableCustomFrameMode();
			windowCustomFrameMode = true;
		}

		void GuiHostedWindow::DisableCustomFrameMode()
		{
			proxy->DisableCustomFrameMode();
			windowCustomFrameMode = false;
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
			return windowIcon;
		}

		void GuiHostedWindow::SetIcon(Ptr<GuiImageData> icon)
		{
			windowIcon = icon;
			proxy->UpdateIcon();
		}

		INativeWindow::WindowSizeState GuiHostedWindow::GetSizeState()
		{
			return windowSizeState;
		}

		void GuiHostedWindow::Show()
		{
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
			wmWindow.SetEnabled(true);
			proxy->UpdateEnabled();
		}

		void GuiHostedWindow::Disable()
		{
			wmWindow.SetEnabled(false);
			proxy->UpdateEnabled();
		}

		bool GuiHostedWindow::IsEnabled()
		{
			return wmWindow.enabled;
		}

		void GuiHostedWindow::SetFocus()
		{
			wmWindow.Activate();
			proxy->SetFocus();
		}

		bool GuiHostedWindow::IsFocused()
		{
			return wmWindow.active;
		}

		void GuiHostedWindow::SetActivate()
		{
			SetFocus();
		}

		bool GuiHostedWindow::IsActivated()
		{
			return wmWindow.renderedAsActive;
		}

		void GuiHostedWindow::ShowInTaskBar()
		{
			windowShowInTaskBar = true;
			proxy->UpdateShowInTaskBar();
		}

		void GuiHostedWindow::HideInTaskBar()
		{
			windowShowInTaskBar = false;
			proxy->UpdateShowInTaskBar();
		}

		bool GuiHostedWindow::IsAppearedInTaskBar()
		{
			return windowShowInTaskBar;
		}

		void GuiHostedWindow::EnableActivate()
		{
			windowEnabledActivate = true;
			proxy->UpdateEnabledActivate();
		}

		void GuiHostedWindow::DisableActivate()
		{
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
			windowMaximizedBox = visible;
			proxy->UpdateMaximizedBox();
		}

		bool GuiHostedWindow::GetMinimizedBox()
		{
			return windowMinimizedBox;
		}

		void GuiHostedWindow::SetMinimizedBox(bool visible)
		{
			windowMinimizedBox = visible;
			proxy->UpdateMinimizedBox();
		}

		bool GuiHostedWindow::GetBorder()
		{
			return windowBorder;
		}

		void GuiHostedWindow::SetBorder(bool visible)
		{
			windowBorder = visible;
			proxy->UpdateBorderVisible();
		}

		bool GuiHostedWindow::GetSizeBox()
		{
			return windowSizeBox;
		}

		void GuiHostedWindow::SetSizeBox(bool visible)
		{
			windowSizeBox = visible;
			proxy->UpdateSizeBox();
		}

		bool GuiHostedWindow::GetIconVisible()
		{
			return windowIconVisible;
		}

		void GuiHostedWindow::SetIconVisible(bool visible)
		{
			windowIconVisible = visible;
			proxy->UpdateIconVisible();
		}

		bool GuiHostedWindow::GetTitleBar()
		{
			return windowTitleBar;
		}

		void GuiHostedWindow::SetTitleBar(bool visible)
		{
			windowTitleBar = visible;
			proxy->UpdateTitleBar();
		}

		bool GuiHostedWindow::GetTopMost()
		{
			return wmWindow.topMost;
		}

		void GuiHostedWindow::SetTopMost(bool topmost)
		{
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
			controller->nativeWindow->RedrawContent();
		}
	}
}