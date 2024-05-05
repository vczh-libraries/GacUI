#include "GuiRemoteController.h"

namespace vl::presentation
{
/***********************************************************************
GuiRemoteWindow
***********************************************************************/

#define SET_REMOTE_WINDOW_STYLE(STYLE, VALUE)\
		if (style ## STYLE != VALUE)\
		{\
			style ## STYLE = VALUE;\
			remoteMessages.RequestWindowNotifySet ## STYLE(VALUE);\
		}\

#define SET_REMOTE_WINDOW_STYLE_INVALIDATE(STYLE, VALUE)\
		if (style ## STYLE != VALUE)\
		{\
			style ## STYLE = VALUE;\
			sizingConfigInvalidated = true;\
			remoteMessages.RequestWindowNotifySet ## STYLE(VALUE);\
		}\

	void GuiRemoteWindow::RequestGetBounds()
	{
		sizingConfigInvalidated = false;
		vint idGetBounds = remoteMessages.RequestWindowGetBounds();
		remoteMessages.Submit();
		OnWindowBoundsUpdated(remoteMessages.RetrieveWindowGetBounds(idGetBounds));
	}

	void GuiRemoteWindow::Opened()
	{
		if (!statusVisible)
		{
			statusVisible = true;
			for (auto l : listeners)l->Opened();
		}
	}

	void GuiRemoteWindow::SetActivated(bool activated)
	{
		if (statusActivated != activated)
		{
			statusActivated = activated;
			if (statusActivated)
			{
				for (auto l : listeners)l->GotFocus();
				for (auto l : listeners)l->RenderingAsActivated();
			}
			else
			{
				for (auto l : listeners)l->LostFocus();
				for (auto l : listeners)l->RenderingAsDeactivated();
			}
		}
	}

	void GuiRemoteWindow::ShowWithSizeState(bool activate, INativeWindow::WindowSizeState sizeState)
	{
		if (!statusVisible || remoteWindowSizingConfig.sizeState != sizeState)
		{
			remoteprotocol::WindowShowing windowShowing;
			windowShowing.activate = activate;
			windowShowing.sizeState = sizeState;
			remoteMessages.RequestWindowNotifyShow(windowShowing);
			remoteMessages.Submit();

			remoteWindowSizingConfig.sizeState = sizeState;
			Opened();
			SetActivated(activate);
		}
		else if (!statusActivated && activate)
		{
			SetActivate();
		}
	}

/***********************************************************************
GuiRemoteWindow (events)
***********************************************************************/

	void GuiRemoteWindow::OnControllerConnect()
	{
		if (disconnected)
		{
			disconnected = false;
		}

		sizingConfigInvalidated = true;
		RequestGetBounds();

		if (remote->applicationRunning)
		{
			remoteMessages.RequestWindowNotifySetTitle(styleTitle);
			remoteMessages.RequestWindowNotifySetEnabled(styleEnabled);
			remoteMessages.RequestWindowNotifySetTopMost(styleTopMost);
			remoteMessages.RequestWindowNotifySetShowInTaskBar(styleShowInTaskBar);
			remoteMessages.RequestWindowNotifySetCustomFrameMode(styleCustomFrameMode);
			remoteMessages.RequestWindowNotifySetMaximizedBox(styleMaximizedBox);
			remoteMessages.RequestWindowNotifySetMinimizedBox(styleMinimizedBox);
			remoteMessages.RequestWindowNotifySetBorder(styleBorder);
			remoteMessages.RequestWindowNotifySetSizeBox(styleSizeBox);
			remoteMessages.RequestWindowNotifySetIconVisible(styleIconVisible);
			remoteMessages.RequestWindowNotifySetTitleBar(styleTitleBar);
			if (statusCapturing)
			{
				remoteMessages.RequestIORequireCapture();
			}
			else
			{
				remoteMessages.RequestIOReleaseCapture();
			}
			remoteMessages.Submit();
		}
	}

	void GuiRemoteWindow::OnControllerDisconnect()
	{
		disconnected = true;
	}

	void GuiRemoteWindow::OnControllerScreenUpdated(const remoteprotocol::ScreenConfig& arguments)
	{
		if (scalingX != arguments.scalingX || scalingY != arguments.scalingY)
		{
			scalingX = arguments.scalingX;
			scalingY = arguments.scalingY;
			for (auto l : listeners) l->DpiChanged(true);
			for (auto l : listeners) l->DpiChanged(false);
		}
	}

	void GuiRemoteWindow::OnWindowBoundsUpdated(const remoteprotocol::WindowSizingConfig& arguments)
	{
		bool callMoved = false;
		if (remoteWindowSizingConfig.bounds != arguments.bounds ||
			remoteWindowSizingConfig.clientBounds != arguments.clientBounds)
		{
			callMoved = true;
		}

		remoteWindowSizingConfig = arguments;
		if(callMoved)
		{
			for (auto l : listeners) l->Moved();
		}
	}

	void GuiRemoteWindow::OnWindowActivatedUpdated(bool activated)
	{
		SetActivated(activated);
	}

/***********************************************************************
GuiRemoteWindow
***********************************************************************/

	GuiRemoteWindow::GuiRemoteWindow(GuiRemoteController* _remote)
		: remote(_remote)
		, remoteMessages(_remote->remoteMessages)
		, remoteEvents(_remote->remoteEvents)
	{
		remoteWindowSizingConfig.sizeState = INativeWindow::Restored;
	}

	GuiRemoteWindow::~GuiRemoteWindow()
	{
	}

/***********************************************************************
GuiRemoteWindow (INativeWindow)
***********************************************************************/

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
		return Point((vint)(value.x.value / scalingX), (vint)(value.y.value / scalingY));
	}

	NativePoint GuiRemoteWindow::Convert(Point value)
	{
		return NativePoint((vint)(value.x * scalingX), (vint)(value.y * scalingY));
	}

	Size GuiRemoteWindow::Convert(NativeSize value)
	{
		return Size((vint)(value.x.value / scalingX), (vint)(value.y.value / scalingY));
	}

	NativeSize GuiRemoteWindow::Convert(Size value)
	{
		return NativeSize((vint)(value.x * scalingX), (vint)(value.y * scalingY));
	}

	Margin GuiRemoteWindow::Convert(NativeMargin value)
	{
		return Margin(
			(vint)(value.left.value / scalingX),
			(vint)(value.top.value / scalingY),
			(vint)(value.right.value / scalingX),
			(vint)(value.bottom.value / scalingY)
		);
	}

	NativeMargin GuiRemoteWindow::Convert(Margin value)
	{
		return NativeMargin(
			(vint)(value.left * scalingX),
			(vint)(value.top * scalingY),
			(vint)(value.right * scalingX),
			(vint)(value.bottom * scalingY)
		);
	}

	NativeRect GuiRemoteWindow::GetBounds()
	{
		if (sizingConfigInvalidated) RequestGetBounds();
		return remoteWindowSizingConfig.bounds;
	}

	void GuiRemoteWindow::SetBounds(const NativeRect& bounds)
	{
		if (remoteWindowSizingConfig.bounds != bounds)
		{
			remoteMessages.RequestWindowNotifySetBounds(bounds);
			sizingConfigInvalidated = true;
		}
	}

	NativeSize GuiRemoteWindow::GetClientSize()
	{
		if (sizingConfigInvalidated) RequestGetBounds();
		return remoteWindowSizingConfig.clientBounds.GetSize();
	}

	void GuiRemoteWindow::SetClientSize(NativeSize size)
	{
		if (remoteWindowSizingConfig.clientBounds.GetSize() != size)
		{
			remoteMessages.RequestWindowNotifySetClientSize(size);
			sizingConfigInvalidated = true;
		}
	}

	NativeRect GuiRemoteWindow::GetClientBoundsInScreen()
	{
		auto bounds = remoteWindowSizingConfig.clientBounds;
		bounds.x1.value += remoteWindowSizingConfig.bounds.x1.value;
		bounds.y1.value += remoteWindowSizingConfig.bounds.y1.value;
		bounds.x2.value += remoteWindowSizingConfig.bounds.x1.value;
		bounds.y2.value += remoteWindowSizingConfig.bounds.y1.value;
		return bounds;
	}

	WString GuiRemoteWindow::GetTitle()
	{
		return styleTitle;
	}

	void GuiRemoteWindow::SetTitle(const WString& title)
	{
		SET_REMOTE_WINDOW_STYLE(Title, title);
	}

	INativeCursor* GuiRemoteWindow::GetWindowCursor()
	{
		if (!styleCursor)
		{
			styleCursor = remote->ResourceService()->GetDefaultSystemCursor();
		}
		return styleCursor;
	}

	void GuiRemoteWindow::SetWindowCursor(INativeCursor* cursor)
	{
		styleCursor = cursor;
	}

	NativePoint GuiRemoteWindow::GetCaretPoint()
	{
		return styleCaret;
	}

	void GuiRemoteWindow::SetCaretPoint(NativePoint point)
	{
		styleCaret = point;
	}

	INativeWindow* GuiRemoteWindow::GetParent()
	{
		return nullptr;
	}

	void GuiRemoteWindow::SetParent(INativeWindow* parent)
	{
		CHECK_FAIL(L"vl::presentation::GuiRemoteWindow::SetParent(INativeWindow*)#GuiHostedController is not supposed to call this.");
	}

	INativeWindow::WindowMode GuiRemoteWindow::GetWindowMode()
	{
		return windowMode;
	}

	void GuiRemoteWindow::EnableCustomFrameMode()
	{
		SET_REMOTE_WINDOW_STYLE_INVALIDATE(CustomFrameMode, true);
	}

	void GuiRemoteWindow::DisableCustomFrameMode()
	{
		SET_REMOTE_WINDOW_STYLE_INVALIDATE(CustomFrameMode, false);
	}

	bool GuiRemoteWindow::IsCustomFrameModeEnabled()
	{
		return styleCustomFrameMode;
	}

	NativeMargin GuiRemoteWindow::GetCustomFramePadding()
	{
		return remoteWindowSizingConfig.customFramePadding;
	}

	Ptr<GuiImageData> GuiRemoteWindow::GetIcon()
	{
		return styleIcon;
	}

	void GuiRemoteWindow::SetIcon(Ptr<GuiImageData> icon)
	{
		styleIcon = icon;
	}

	INativeWindow::WindowSizeState GuiRemoteWindow::GetSizeState()
	{
		return remoteWindowSizingConfig.sizeState;
	}

	void GuiRemoteWindow::Show()
	{
		ShowWithSizeState(true, remoteWindowSizingConfig.sizeState);
	}

	void GuiRemoteWindow::ShowDeactivated()
	{
		ShowWithSizeState(false, remoteWindowSizingConfig.sizeState);
	}

	void GuiRemoteWindow::ShowRestored()
	{
		ShowWithSizeState(true, INativeWindow::Restored);
	}

	void GuiRemoteWindow::ShowMaximized()
	{
		ShowWithSizeState(true, INativeWindow::Maximized);
	}

	void GuiRemoteWindow::ShowMinimized()
	{
		ShowWithSizeState(true, INativeWindow::Minimized);
	}

	void GuiRemoteWindow::Hide(bool closeWindow)
	{
		if (!remote->connectionForcedToStop)
		{
			bool cancel = false;
			for (auto l : listeners)
			{
				l->BeforeClosing(cancel);
				if (cancel) return;
			}
			for (auto l : listeners) l->AfterClosing();
		}
		remote->DestroyNativeWindow(this);
	}

	bool GuiRemoteWindow::IsVisible()
	{
		return statusVisible;
	}

	void GuiRemoteWindow::Enable()
	{
		if (styleEnabled != true)
		{
			styleEnabled = true;
			remoteMessages.RequestWindowNotifySetEnabled(true);
			for (auto l : listeners) l->Enabled();
		}
	}

	void GuiRemoteWindow::Disable()
	{
		if (styleEnabled != false)
		{
			styleEnabled = false;
			remoteMessages.RequestWindowNotifySetEnabled(false);
			for (auto l : listeners) l->Disabled();
		}
	}

	bool GuiRemoteWindow::IsEnabled()
	{
		return styleEnabled;
	}

	void GuiRemoteWindow::SetActivate()
	{
		if (statusActivated != true)
		{
			SetActivated(true);
			remoteMessages.RequestWindowNotifyActivate();
		}
	}

	bool GuiRemoteWindow::IsActivated()
	{
		return statusActivated;
	}

	bool GuiRemoteWindow::IsRenderingAsActivated()
	{
		return statusActivated;
	}

	void GuiRemoteWindow::ShowInTaskBar()
	{
		SET_REMOTE_WINDOW_STYLE(ShowInTaskBar, true);
	}

	void GuiRemoteWindow::HideInTaskBar()
	{
		SET_REMOTE_WINDOW_STYLE(ShowInTaskBar, false);
	}

	bool GuiRemoteWindow::IsAppearedInTaskBar()
	{
		return styleShowInTaskBar;
	}

	void GuiRemoteWindow::EnableActivate()
	{
		CHECK_FAIL(L"vl::presentation::GuiRemoteWindow::EnableActivate()#GuiHostedController is not supposed to call this.");
	}

	void GuiRemoteWindow::DisableActivate()
	{
		CHECK_FAIL(L"vl::presentation::GuiRemoteWindow::EnableActivate()#GuiHostedController is not supposed to call this.");
	}

	bool GuiRemoteWindow::IsEnabledActivate()
	{
		return true;
	}

	bool GuiRemoteWindow::RequireCapture()
	{
		if (!statusCapturing)
		{
			statusCapturing = true;
			remoteMessages.RequestIORequireCapture();
			remoteMessages.Submit();
		}
		return true;
	}

	bool GuiRemoteWindow::ReleaseCapture()
	{
		if (statusCapturing)
		{
			statusCapturing = false;
			remoteMessages.RequestIOReleaseCapture();
			remoteMessages.Submit();
		}
		return true;
	}

	bool GuiRemoteWindow::IsCapturing()
	{
		return statusCapturing;
	}

	bool GuiRemoteWindow::GetMaximizedBox()
	{
		return styleMaximizedBox;
	}

	void GuiRemoteWindow::SetMaximizedBox(bool visible)
	{
		SET_REMOTE_WINDOW_STYLE_INVALIDATE(MaximizedBox, visible);
	}

	bool GuiRemoteWindow::GetMinimizedBox()
	{
		return styleMinimizedBox;
	}

	void GuiRemoteWindow::SetMinimizedBox(bool visible)
	{
		SET_REMOTE_WINDOW_STYLE_INVALIDATE(MinimizedBox, visible);
	}

	bool GuiRemoteWindow::GetBorder()
	{
		return styleBorder;
	}

	void GuiRemoteWindow::SetBorder(bool visible)
	{
		SET_REMOTE_WINDOW_STYLE_INVALIDATE(Border, visible);
	}

	bool GuiRemoteWindow::GetSizeBox()
	{
		return styleSizeBox;
	}

	void GuiRemoteWindow::SetSizeBox(bool visible)
	{
		SET_REMOTE_WINDOW_STYLE_INVALIDATE(SizeBox, visible);
	}

	bool GuiRemoteWindow::GetIconVisible()
	{
		return styleIconVisible;
	}

	void GuiRemoteWindow::SetIconVisible(bool visible)
	{
		SET_REMOTE_WINDOW_STYLE_INVALIDATE(IconVisible, visible);
	}

	bool GuiRemoteWindow::GetTitleBar()
	{
		return styleTitleBar;
	}

	void GuiRemoteWindow::SetTitleBar(bool visible)
	{
		SET_REMOTE_WINDOW_STYLE_INVALIDATE(TitleBar, visible);
	}

	bool GuiRemoteWindow::GetTopMost()
	{
		return styleTopMost;
	}

	void GuiRemoteWindow::SetTopMost(bool topmost)
	{
		SET_REMOTE_WINDOW_STYLE(TopMost, topmost);
	}

	void GuiRemoteWindow::SupressAlt()
	{
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
	}

#undef SET_REMOTE_WINDOW_STYLE_INVALIDATE
#undef SET_REMOTE_WINDOW_STYLE
}