#include "TuiWindow.h"
#include "TuiController.h"

namespace vl::presentation
{
	using namespace console;

	TuiWindow::TuiWindow(TuiControllerBase* value)
		: controller(value)
		, clientSize(TUI::GetBufferWidth(), TUI::GetBufferHeight())
	{
	}

	TuiWindow::~TuiWindow()
	{
		Dispatch([](auto listener) { listener->Destroyed(); }, true);
	}

	void TuiWindow::Dispatch(const Func<void(INativeWindowListener*)>& callback, bool duringFinalization)
	{
		collections::List<INativeWindowListener*> snapshot;
		CopyFrom(snapshot, listeners);
		for (auto listener : snapshot)
		{
			if (!duringFinalization && TUI::IsStopRequested()) break;
			if (listeners.Contains(listener)) callback(listener);
		}
	}

	bool TuiWindow::IsActivelyRefreshing()
	{
		return true;
	}

	NativeSize TuiWindow::GetRenderingOffset()
	{
		return {};
	}

	Point TuiWindow::Convert(NativePoint value)
	{
		return {value.x.value, value.y.value};
	}

	NativePoint TuiWindow::Convert(Point value)
	{
		return {value.x, value.y};
	}

	Size TuiWindow::Convert(NativeSize value)
	{
		return {value.x.value, value.y.value};
	}

	NativeSize TuiWindow::Convert(Size value)
	{
		return {value.x, value.y};
	}

	Margin TuiWindow::Convert(NativeMargin value)
	{
		return {value.left.value, value.top.value, value.right.value, value.bottom.value};
	}

	NativeMargin TuiWindow::Convert(Margin value)
	{
		return {value.left, value.top, value.right, value.bottom};
	}

	NativeRect TuiWindow::GetBounds()
	{
		return {NativePoint(), clientSize};
	}

	void TuiWindow::SetBounds(const NativeRect& bounds)
	{
		SetClientSize(bounds.GetSize());
	}

	NativeSize TuiWindow::GetClientSize()
	{
		return clientSize;
	}

	void TuiWindow::SetClientSize(NativeSize size)
	{
		if (clientSize != size)
		{
			clientSize = size;
			Dispatch([](auto listener) { listener->Moved(); });
		}
	}

	NativeRect TuiWindow::GetClientBoundsInScreen()
	{
		return GetBounds();
	}

	void TuiWindow::SuggestMinClientSize(NativeSize size)
	{
	}

	WString TuiWindow::GetTitle()
	{
		return title;
	}

	void TuiWindow::SetTitle(const WString& value)
	{
		title = value;
		controller->ApplyTitle(value);
	}

	INativeCursor* TuiWindow::GetWindowCursor()
	{
		return cursor;
	}

	void TuiWindow::SetWindowCursor(INativeCursor* value)
	{
		cursor = value;
	}

	NativePoint TuiWindow::GetCaretPoint()
	{
		return caret;
	}

	void TuiWindow::SetCaretPoint(NativePoint value)
	{
		caret = value;
	}

	INativeWindow* TuiWindow::GetParent()
	{
		return nullptr;
	}

	void TuiWindow::SetParent(INativeWindow* parent)
	{
	}

	INativeWindow::WindowMode TuiWindow::GetWindowMode()
	{
		return Normal;
	}

	void TuiWindow::EnableCustomFrameMode()
	{
	}

	void TuiWindow::DisableCustomFrameMode()
	{
	}

	bool TuiWindow::IsCustomFrameModeEnabled()
	{
		return false;
	}

	NativeMargin TuiWindow::GetCustomFramePadding()
	{
		return {};
	}

	Ptr<GuiImageData> TuiWindow::GetIcon()
	{
		return icon;
	}

	void TuiWindow::SetIcon(Ptr<GuiImageData> value)
	{
		icon = value;
	}

	INativeWindow::WindowSizeState TuiWindow::GetSizeState()
	{
		return Restored;
	}

	void TuiWindow::Show()
	{
		if (!visible)
		{
			visible = true;
			Dispatch([](auto listener) { listener->Opened(); });
			Dispatch([](auto listener) { listener->GotFocus(); });
			Dispatch([](auto listener) { listener->RenderingAsActivated(); });
		}
	}

	void TuiWindow::ShowDeactivated()
	{
		Show();
	}

	void TuiWindow::ShowRestored()
	{
		Show();
	}

	void TuiWindow::ShowMaximized()
	{
		Show();
	}

	void TuiWindow::ShowMinimized()
	{
		Show();
	}

	void TuiWindow::Hide(bool closeWindow)
	{
	}

	bool TuiWindow::IsVisible()
	{
		return visible;
	}

	void TuiWindow::Enable()
	{
		enabled = true;
	}

	void TuiWindow::Disable()
	{
		enabled = false;
	}

	bool TuiWindow::IsEnabled()
	{
		return enabled;
	}

	void TuiWindow::SetActivate()
	{
	}

	bool TuiWindow::IsActivated()
	{
		return true;
	}

	bool TuiWindow::IsRenderingAsActivated()
	{
		return true;
	}

	bool TuiWindow::IsAppearedInTaskBar()
	{
		return true;
	}

	bool TuiWindow::IsEnabledActivate()
	{
		return true;
	}

	void TuiWindow::ShowInTaskBar()
	{
	}

	void TuiWindow::HideInTaskBar()
	{
	}

	void TuiWindow::EnableActivate()
	{
	}

	void TuiWindow::DisableActivate()
	{
	}

	void TuiWindow::SupressAlt()
	{
	}

	bool TuiWindow::RequireCapture()
	{
		capturing = true;
		return true;
	}

	bool TuiWindow::ReleaseCapture()
	{
		capturing = false;
		return true;
	}

	bool TuiWindow::IsCapturing()
	{
		return capturing;
	}

	bool TuiWindow::GetMaximizedBox()
	{
		return false;
	}

	void TuiWindow::SetMaximizedBox(bool value)
	{
	}

	bool TuiWindow::GetMinimizedBox()
	{
		return false;
	}

	void TuiWindow::SetMinimizedBox(bool value)
	{
	}

	bool TuiWindow::GetBorder()
	{
		return false;
	}

	void TuiWindow::SetBorder(bool value)
	{
	}

	bool TuiWindow::GetSizeBox()
	{
		return false;
	}

	void TuiWindow::SetSizeBox(bool value)
	{
	}

	bool TuiWindow::GetIconVisible()
	{
		return false;
	}

	void TuiWindow::SetIconVisible(bool value)
	{
	}

	bool TuiWindow::GetTitleBar()
	{
		return false;
	}

	void TuiWindow::SetTitleBar(bool value)
	{
	}

	bool TuiWindow::GetTopMost()
	{
		return false;
	}

	void TuiWindow::SetTopMost(bool value)
	{
	}

	bool TuiWindow::InstallListener(INativeWindowListener* listener)
	{
		if (!listener || listeners.Contains(listener)) return false;
		listeners.Add(listener);
		return true;
	}

	bool TuiWindow::UninstallListener(INativeWindowListener* listener)
	{
		return listeners.Remove(listener);
	}

	void TuiWindow::RedrawContent()
	{
		Dispatch([](auto listener) { listener->Paint(); });
	}

}
