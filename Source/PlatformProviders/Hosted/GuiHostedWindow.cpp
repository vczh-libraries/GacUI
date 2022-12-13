#include "GuiHostedController.h"

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

		bool GuiHostedWindow::IsActivelyRefreshing()
		{
			return false;
		}

		NativeSize GuiHostedWindow::GetRenderingOffset()
		{
			CHECK_FAIL(L"Not implemented!");
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
	}
}