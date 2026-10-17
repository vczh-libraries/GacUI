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
		remote->EnsureControllerConnected();
		vint idGetBounds = remoteMessages.RequestWindowGetBounds();
		bool disconnected = false;
		remoteMessages.Submit(disconnected);
		if (disconnected) return;
		sizingConfigInvalidated = false;
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
		remote->EnsureControllerConnected();
		if (!statusVisible || remoteWindowSizingConfig.sizeState != sizeState)
		{
			remoteprotocol::WindowShowing windowShowing;
			windowShowing.activate = activate;
			windowShowing.sizeState = sizeState;
			remoteMessages.RequestWindowNotifyShow(windowShowing);
			bool disconnected = false;
			remoteMessages.Submit(disconnected);
			// there is no result from this request, assuming succeeded

			remoteWindowSizingConfig.sizeState = sizeState;
			Opened();
			SetActivated(activate);
		}
		else if (!statusActivated && activate)
		{
			SetActivate();
		}
	}

	void GuiRemoteWindow::SubmitStateAfterControllerConnect()
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
		if (suggestedMinClientSize != NativeSize{ {0},{0} })
		{
			// The renderer calculates its outer minimum from the current
			// native frame. Apply frame styles first so reconnecting from a
			// platform-default window does not include stale decorations.
			remoteMessages.RequestWindowNotifyMinSize(suggestedMinClientSize);
		}
		if (statusCapturing)
		{
			remoteMessages.RequestIORequireCapture();
		}
		else
		{
			remoteMessages.RequestIOReleaseCapture();
		}
		bool disconnected = false;
		remoteMessages.Submit(disconnected);
		// there is no result from these requests, assuming succeeded
	}

/***********************************************************************
GuiRemoteWindow (events)
***********************************************************************/


	bool GuiRemoteWindow::AcceptPointRecord(const WString& sequenceId, vint eventIndex)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteWindow::AcceptPointRecord#"
		if (controllerDisconnected || wirePointSequenceId == 0 || remoteprotocol::DecodePointUnsigned(sequenceId) != wirePointSequenceId) return false;
		if (eventIndex < nextPointEventIndex) return false;
		CHECK_ERROR(eventIndex == nextPointEventIndex, ERROR_MESSAGE_PREFIX L"Point event index gap.");
		CHECK_ERROR(eventIndex < 0x7FFFFFFF, ERROR_MESSAGE_PREFIX L"Point event indices exhausted.");
		return true;
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteWindow::OnPointStarted(const remoteprotocol::IOPointAction& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteWindow::OnPointStarted#"
		if (controllerDisconnected || pointState.sequenceId != 0) return;
		auto info = remoteprotocol::DecodePointInfo(arguments.info);
		if (completedPointSequences.Contains(info.sequenceId)) return;
		CHECK_ERROR(info.sequenceId != 0 && arguments.eventIndex == 0 && info.pointId == 0 && info.activePointCount == 1, ERROR_MESSAGE_PREFIX L"Invalid Started record.");
		wirePointSequenceId = info.sequenceId;
		completedPointSequences.Add(wirePointSequenceId);
		nextPointEventIndex = 1;
		BeginNativePointSequence(pointState, listeners, info);
		firstPointSample = info;
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteWindow::OnPointAction(NativePointEvent event, const remoteprotocol::IOPointAction& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteWindow::OnPointAction#"
		if (!AcceptPointRecord(arguments.info.sequenceId, arguments.eventIndex)) return;
		auto info = remoteprotocol::DecodePointInfo(arguments.info);
		info.sequenceId = pointState.sequenceId;
		bool starting = pointState.nextPointId == 0;
		CHECK_ERROR(!starting || (event == NativePointEvent::Down && info == firstPointSample), ERROR_MESSAGE_PREFIX L"First Down must match Started.");
		CHECK_ERROR(info.pointId >= 0, ERROR_MESSAGE_PREFIX L"Invalid contact index.");
		if (event == NativePointEvent::Down)
		{
			CHECK_ERROR(info.pointId == pointState.nextPointId, ERROR_MESSAGE_PREFIX L"Contact indices must follow Down order.");
		}
		else
		{
			CHECK_ERROR(pointState.contacts.Keys().Contains((vuint64_t)info.pointId), ERROR_MESSAGE_PREFIX L"Contact is not down.");
		}
		auto activeCount = pointState.contacts.Count() + (event == NativePointEvent::Down ? 1 : event == NativePointEvent::Up ? -1 : 0);
		CHECK_ERROR(info.activePointCount == activeCount, ERROR_MESSAGE_PREFIX L"Invalid active contact count.");
		UpdateNativePointInput(pointState, listeners, event, (vuint64_t)info.pointId, info);
		if (!pointState.accepted) ProcessIOPointFallback(pointCommandState, listeners, event, (vuint64_t)info.pointId, info, starting);
		nextPointEventIndex++;
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteWindow::OnPointEnded(const remoteprotocol::IOPointEnd& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteWindow::OnPointEnded#"
		if (!AcceptPointRecord(arguments.sequenceId, arguments.eventIndex)) return;
		auto timestamp = remoteprotocol::DecodePointUnsigned(arguments.timestamp);
		CHECK_ERROR(arguments.reason != PointSequenceEndReason::Succeeded || (pointState.nextPointId > 0 && pointState.contacts.Count() == 0), ERROR_MESSAGE_PREFIX L"Successful End requires all contacts Up.");
		EndNativePointSequence(pointState, listeners, arguments.reason, timestamp);
		if (arguments.reason != PointSequenceEndReason::Succeeded) CancelIOPointFallback(pointCommandState, listeners);
		ResetCompletedNativePointSequence(pointState);
		wirePointSequenceId = 0;
		nextPointEventIndex = 0;
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteWindow::CancelPointInput(PointSequenceEndReason reason)
	{
		CancelNativePointSequence(pointState, listeners, reason, GetNativePointTimestamp());
		CancelIOPointFallback(pointCommandState, listeners);
		ResetCompletedNativePointSequence(pointState);
		wirePointSequenceId = 0;
		nextPointEventIndex = 0;
	}

	void GuiRemoteWindow::OnControllerConnect()
	{
		CancelPointInput(PointSequenceEndReason::Canceled_CaptureLost);
		completedPointSequences.Clear();
		if (controllerDisconnected)
		{
			controllerDisconnected = false;
		}

		scalingX = remote->remoteScreenConfig.scalingX;
		scalingY = remote->remoteScreenConfig.scalingY;

		auto bounds = remoteWindowSizingConfig.bounds;
		if (remote->applicationRunning)
		{
			// A replacement renderer creates its native window with platform
			// defaults. Restore the application frame style before applying
			// the saved bounds so client-size calculations use the right frame.
			// Style changes report the renderer's temporary bounds back to the
			// core, so keep the saved bounds in a local variable.
			SubmitStateAfterControllerConnect();
		}

		sizingConfigInvalidated = true;
		remoteMessages.RequestWindowNotifySetBounds(bounds);
		RequestGetBounds();

		// TODO:
		//   This is a workaround to call GuiWindow::UpdateCustomFramePadding
		//   Refactor to make it more elegant.
		for (auto l : listeners) l->DpiChanged(true);
		for (auto l : listeners) l->DpiChanged(false);
	}

	void GuiRemoteWindow::OnControllerDisconnect()
	{
		controllerDisconnected = true;
		CancelPointInput(PointSequenceEndReason::Canceled_CaptureLost);
		completedPointSequences.Clear();
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
			auto x1 = remoteWindowSizingConfig.clientBounds.x1 - remoteWindowSizingConfig.bounds.x1;
			auto y1 = remoteWindowSizingConfig.clientBounds.y1 - remoteWindowSizingConfig.bounds.y1;
			auto x2 = remoteWindowSizingConfig.clientBounds.x2 - remoteWindowSizingConfig.bounds.x2;
			auto y2 = remoteWindowSizingConfig.clientBounds.y2 - remoteWindowSizingConfig.bounds.y2;
			remoteWindowSizingConfig.bounds = bounds;
			remoteWindowSizingConfig.clientBounds = {
				x1 + remoteWindowSizingConfig.bounds.x1,
				y1 + remoteWindowSizingConfig.bounds.y1,
				x2 + remoteWindowSizingConfig.bounds.x2,
				y2 + remoteWindowSizingConfig.bounds.y2
			};
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
			auto x1 = remoteWindowSizingConfig.bounds.x1 - remoteWindowSizingConfig.clientBounds.x1;
			auto y1 = remoteWindowSizingConfig.bounds.y1 - remoteWindowSizingConfig.clientBounds.y1;
			auto x2 = remoteWindowSizingConfig.bounds.x2 - remoteWindowSizingConfig.clientBounds.x2;
			auto y2 = remoteWindowSizingConfig.bounds.y2 - remoteWindowSizingConfig.clientBounds.y2;
			remoteWindowSizingConfig.clientBounds = { remoteWindowSizingConfig.clientBounds.LeftTop(),size };
			remoteWindowSizingConfig.bounds = {
				x1 + remoteWindowSizingConfig.clientBounds.x1,
				y1 + remoteWindowSizingConfig.clientBounds.y1,
				x2 + remoteWindowSizingConfig.clientBounds.x2,
				y2 + remoteWindowSizingConfig.clientBounds.y2
			};
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

	void GuiRemoteWindow::SuggestMinClientSize(NativeSize size)
	{
		if (suggestedMinClientSize != size)
		{
			suggestedMinClientSize = size;
			remoteMessages.RequestWindowNotifyMinSize(suggestedMinClientSize);
		}
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
		remoteMessages.RequestWindowNotifySetCaret(point);
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

		remote->AsyncService()->InvokeInMainThread(this, [this]()
		{
			remote->DestroyNativeWindow(this);
		});
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
			bool disconnected = false;
			remoteMessages.Submit(disconnected);
			// there is no result from this request, assuming succeeded
		}
		return true;
	}

	bool GuiRemoteWindow::ReleaseCapture()
	{
		if (statusCapturing)
		{
			statusCapturing = false;
			if (controllerDisconnected) return true;
			remoteMessages.RequestIOReleaseCapture();
			bool disconnected = false;
			remoteMessages.Submit(disconnected);
			// there is no result from this request, assuming succeeded
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
