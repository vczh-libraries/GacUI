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