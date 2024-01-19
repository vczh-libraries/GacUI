#include "GuiRemoteController.h"

namespace vl::presentation
{
/***********************************************************************
GuiRemoteWindow
***********************************************************************/

	GuiRemoteWindow::GuiRemoteWindow(GuiRemoteController* _remote)
		: remote(_remote)
		, remoteProtocol(_remote->remoteProtocol)
		, remoteEvents(_remote->remoteEvents)
	{
	}

	GuiRemoteWindow::~GuiRemoteWindow()
	{
	}

	void GuiRemoteWindow::OnControllerConnect()
	{
		vint idGetBounds = remoteEvents.RequestWindowGetBounds();
		remoteProtocol->Submit();
		OnWindowBoundsUpdated(remoteEvents.RetrieveWindowGetBounds(idGetBounds));
		remoteEvents.ClearResponses();
	}

	void GuiRemoteWindow::OnControllerDisconnect()
	{
	}

	void GuiRemoteWindow::OnControllerExit()
	{
	}

	void GuiRemoteWindow::OnControllerScreenUpdated(const remoteprotocol::ScreenConfig& arguments)
	{
		dpiX = (vint)(arguments.scalingX * 96);
		dpiY = (vint)(arguments.scalingY * 96);
	}

	void GuiRemoteWindow::OnWindowBoundsUpdated(const remoteprotocol::WindowSizingConfig& arguments)
	{
		remoteWindowSizingConfig = arguments;
		// TODO: fire events
	}

	bool GuiRemoteWindow::IsActivelyRefreshing()
	{
		return true;
	}

	NativeSize GuiRemoteWindow::GetRenderingOffset()
	{
		return { 0,0 };
	}

	Point GuiRemoteWindow::Convert(NativePoint value)
	{
		return Point((vint)value.x.value * 96 / dpiX, (vint)value.y.value * 96 / dpiY);
	}

	NativePoint GuiRemoteWindow::Convert(Point value)
	{
		return NativePoint(value.x * dpiX / 96, value.y * dpiY / 96);
	}

	Size GuiRemoteWindow::Convert(NativeSize value)
	{
		return Size((vint)value.x.value * 96 / dpiX, (vint)value.y.value * 96 / dpiY);
	}

	NativeSize GuiRemoteWindow::Convert(Size value)
	{
		return NativeSize(value.x * dpiX / 96, value.y * dpiY / 96);
	}

	Margin GuiRemoteWindow::Convert(NativeMargin value)
	{
		return Margin(
			(vint)value.left.value * 96 / dpiX,
			(vint)value.top.value * 96 / dpiY,
			(vint)value.right.value * 96 / dpiX,
			(vint)value.bottom.value * 96 / dpiY
		);
	}

	NativeMargin GuiRemoteWindow::Convert(Margin value)
	{
		return NativeMargin(
			(vint)value.left * dpiX / 96,
			(vint)value.top * dpiY / 96,
			(vint)value.right * dpiX / 96,
			(vint)value.bottom * dpiY / 96
		);
	}

	NativeRect GuiRemoteWindow::GetBounds()
	{
		return remoteWindowSizingConfig.bounds;
	}

	void GuiRemoteWindow::SetBounds(const NativeRect& bounds)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	NativeSize GuiRemoteWindow::GetClientSize()
	{
		return remoteWindowSizingConfig.clientBounds.GetSize();
	}

	void GuiRemoteWindow::SetClientSize(NativeSize size)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	NativeRect GuiRemoteWindow::GetClientBoundsInScreen()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	WString GuiRemoteWindow::GetTitle()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetTitle(const WString& title)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	INativeCursor* GuiRemoteWindow::GetWindowCursor()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetWindowCursor(INativeCursor* cursor)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	NativePoint GuiRemoteWindow::GetCaretPoint()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetCaretPoint(NativePoint point)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	INativeWindow* GuiRemoteWindow::GetParent()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetParent(INativeWindow* parent)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	INativeWindow::WindowMode GuiRemoteWindow::GetWindowMode()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::EnableCustomFrameMode()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::DisableCustomFrameMode()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::IsCustomFrameModeEnabled()
	{
		return true;
	}

	NativeMargin GuiRemoteWindow::GetCustomFramePadding()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Ptr<GuiImageData> GuiRemoteWindow::GetIcon()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetIcon(Ptr<GuiImageData> icon)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	INativeWindow::WindowSizeState GuiRemoteWindow::GetSizeState()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::Show()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::ShowDeactivated()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::ShowRestored()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::ShowMaximized()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::ShowMinimized()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::Hide(bool closeWindow)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::IsVisible()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::Enable()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::Disable()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::IsEnabled()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetActivate()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::IsActivated()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::IsRenderingAsActivated()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::ShowInTaskBar()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::HideInTaskBar()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::IsAppearedInTaskBar()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::EnableActivate()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::DisableActivate()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::IsEnabledActivate()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::RequireCapture()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::ReleaseCapture()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::IsCapturing()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::GetMaximizedBox()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetMaximizedBox(bool visible)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::GetMinimizedBox()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetMinimizedBox(bool visible)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::GetBorder()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetBorder(bool visible)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::GetSizeBox()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetSizeBox(bool visible)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::GetIconVisible()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetIconVisible(bool visible)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::GetTitleBar()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetTitleBar(bool visible)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::GetTopMost()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SetTopMost(bool topmost)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteWindow::SupressAlt()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::InstallListener(INativeWindowListener* listener)
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

	bool GuiRemoteWindow::UninstallListener(INativeWindowListener* listener)
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

	void GuiRemoteWindow::RedrawContent()
	{
		CHECK_FAIL(L"Not Implemented!");
	}
}