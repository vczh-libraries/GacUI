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
		remoteMessages.ClearResponses();
	}

	GuiRemoteWindow::GuiRemoteWindow(GuiRemoteController* _remote)
		: remote(_remote)
		, remoteMessages(_remote->remoteMessages)
		, remoteEvents(_remote->remoteEvents)
	{
	}

	GuiRemoteWindow::~GuiRemoteWindow()
	{
	}

	void GuiRemoteWindow::OnControllerConnect()
	{
		RequestGetBounds();
		// TODO: sync styles and status
	}

	void GuiRemoteWindow::OnControllerDisconnect()
	{
		// TODO: reset styles and status if necessary
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
		if (remoteWindowSizingConfig.bounds != arguments.bounds ||
			remoteWindowSizingConfig.clientBounds != arguments.clientBounds ||
			remoteWindowSizingConfig.customFramePadding != arguments.customFramePadding)
		{
			remoteWindowSizingConfig = arguments;
			for (auto l : listeners) l->Moved();
		}
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
			vint id = remoteMessages.RequestWindowSetBounds(bounds);
			remoteMessages.Submit();
			OnWindowBoundsUpdated(remoteMessages.RetrieveWindowSetBounds(id));
			remoteMessages.ClearResponses();
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
			vint id = remoteMessages.RequestWindowSetClientSize(size);
			remoteMessages.Submit();
			OnWindowBoundsUpdated(remoteMessages.RetrieveWindowSetClientSize(id));
			remoteMessages.ClearResponses();
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
		return statusSizeState;
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
			statusActivated = true;
			remoteMessages.RequestWindowNotifyActivate();
			for (auto l : listeners) l->RenderingAsActivated();
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
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteWindow::ReleaseCapture()
	{
		CHECK_FAIL(L"Not Implemented!");
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

#undef SET_REMOTE_WINDOW_STYLE_INVALIDATE
#undef SET_REMOTE_WINDOW_STYLE
}