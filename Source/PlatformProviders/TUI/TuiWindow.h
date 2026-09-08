#ifndef VCZH_PRESENTATION_TUIWINDOW
#define VCZH_PRESENTATION_TUIWINDOW

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl::presentation
{
	class TuiControllerBase;

	class TuiWindow : public Object, public INativeWindow
	{
		friend class TuiControllerBase;
	protected:
		TuiControllerBase*							controller;
		collections::List<INativeWindowListener*>	listeners;
		NativeSize									clientSize;
		WString										title;
		INativeCursor*								cursor = nullptr;
		NativePoint									caret;
		Ptr<GuiImageData>							icon;
		bool										visible = false;
		bool										enabled = true;
		bool										capturing = false;
	public:
		TuiWindow(TuiControllerBase* controller);
		~TuiWindow();
		void										Dispatch(const Func<void(INativeWindowListener*)>& callback, bool duringFinalization = false);
		bool										IsActivelyRefreshing() override;
		NativeSize									GetRenderingOffset() override;
		Point										Convert(NativePoint value) override;
		NativePoint									Convert(Point value) override;
		Size										Convert(NativeSize value) override;
		NativeSize									Convert(Size value) override;
		Margin										Convert(NativeMargin value) override;
		NativeMargin								Convert(Margin value) override;
		NativeRect									GetBounds() override;
		void										SetBounds(const NativeRect& bounds) override;
		NativeSize									GetClientSize() override;
		void										SetClientSize(NativeSize size) override;
		NativeRect									GetClientBoundsInScreen() override;
		void										SuggestMinClientSize(NativeSize size) override;
		WString										GetTitle() override;
		void										SetTitle(const WString& value) override;
		INativeCursor*								GetWindowCursor() override;
		void										SetWindowCursor(INativeCursor* value) override;
		NativePoint									GetCaretPoint() override;
		void										SetCaretPoint(NativePoint value) override;
		INativeWindow*								GetParent() override;
		void										SetParent(INativeWindow* parent) override;
		WindowMode									GetWindowMode() override;
		void										EnableCustomFrameMode() override;
		void										DisableCustomFrameMode() override;
		bool										IsCustomFrameModeEnabled() override;
		NativeMargin								GetCustomFramePadding() override;
		Ptr<GuiImageData>							GetIcon() override;
		void										SetIcon(Ptr<GuiImageData> value) override;
		WindowSizeState								GetSizeState() override;
		void										Show() override;
		void										ShowDeactivated() override;
		void										ShowRestored() override;
		void										ShowMaximized() override;
		void										ShowMinimized() override;
		void										Hide(bool closeWindow) override;
		bool										IsVisible() override;
		void										Enable() override;
		void										Disable() override;
		bool										IsEnabled() override;
		void										SetActivate() override;
		bool										IsActivated() override;
		bool										IsRenderingAsActivated() override;
		bool										IsAppearedInTaskBar() override;
		bool										IsEnabledActivate() override;
		void										ShowInTaskBar() override;
		void										HideInTaskBar() override;
		void										EnableActivate() override;
		void										DisableActivate() override;
		void										SupressAlt() override;
		bool										RequireCapture() override;
		bool										ReleaseCapture() override;
		bool										IsCapturing() override;
		bool										GetMaximizedBox() override;
		void										SetMaximizedBox(bool value) override;
		bool										GetMinimizedBox() override;
		void										SetMinimizedBox(bool value) override;
		bool										GetBorder() override;
		void										SetBorder(bool value) override;
		bool										GetSizeBox() override;
		void										SetSizeBox(bool value) override;
		bool										GetIconVisible() override;
		void										SetIconVisible(bool value) override;
		bool										GetTitleBar() override;
		void										SetTitleBar(bool value) override;
		bool										GetTopMost() override;
		void										SetTopMost(bool value) override;
		bool										InstallListener(INativeWindowListener* listener) override;
		bool										UninstallListener(INativeWindowListener* listener) override;
		void										RedrawContent() override;
	};
}

#endif
