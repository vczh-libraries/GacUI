/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteController

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEWINDOW
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEWINDOW

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl::presentation
{
	class GuiRemoteController;

/***********************************************************************
GuiRemoteWindow
***********************************************************************/

	class GuiRemoteWindow : public Object, public virtual INativeWindow
	{
		friend class GuiRemoteController;
	protected:
		GuiRemoteController*								remote;
		IGuiRemoteProtocol*									remoteProtocol;
		GuiRemoteEvents&									remoteEvents;
		collections::List<INativeWindowListener*>			listeners;
		INativeWindow::WindowMode							windowMode = INativeWindow::Normal;

		remoteprotocol::WindowSizingConfig					remoteWindowSizingConfig;
		vint												dpiX = 96;
		vint												dpiY = 96;

		bool												customFrameMode = false;

	public:
		GuiRemoteWindow(GuiRemoteController* _remote);
		~GuiRemoteWindow();

		void							OnControllerConnect();
		void							OnControllerDisconnect();
		void							OnControllerExit();
		void							OnControllerScreenUpdated(const remoteprotocol::ScreenConfig& arguments);
		void							OnWindowBoundsUpdated(const remoteprotocol::WindowSizingConfig& arguments);

		// =============================================================
		// INativeWindow
		// =============================================================

		bool							IsActivelyRefreshing() override;
		NativeSize						GetRenderingOffset() override;
		Point							Convert(NativePoint value) override;
		NativePoint						Convert(Point value) override;
		Size							Convert(NativeSize value) override;
		NativeSize						Convert(Size value) override;
		Margin							Convert(NativeMargin value) override;
		NativeMargin					Convert(Margin value) override;
		NativeRect						GetBounds() override;
		void							SetBounds(const NativeRect& bounds) override;
		NativeSize						GetClientSize() override;
		void							SetClientSize(NativeSize size) override;
		NativeRect						GetClientBoundsInScreen() override;
		WString							GetTitle() override;
		void							SetTitle(const WString& title) override;
		INativeCursor*					GetWindowCursor() override;
		void							SetWindowCursor(INativeCursor* cursor) override;
		NativePoint						GetCaretPoint() override;
		void							SetCaretPoint(NativePoint point) override;
		INativeWindow*					GetParent() override;
		void							SetParent(INativeWindow* parent) override;
		WindowMode						GetWindowMode() override;
		void							EnableCustomFrameMode() override;
		void							DisableCustomFrameMode() override;
		bool							IsCustomFrameModeEnabled() override;
		NativeMargin					GetCustomFramePadding() override;
		Ptr<GuiImageData>				GetIcon() override;
		void							SetIcon(Ptr<GuiImageData> icon) override;
		WindowSizeState					GetSizeState() override;
		void							Show() override;
		void							ShowDeactivated() override;
		void							ShowRestored() override;
		void							ShowMaximized() override;
		void							ShowMinimized() override;
		void							Hide(bool closeWindow) override;
		bool							IsVisible() override;
		void							Enable() override;
		void							Disable() override;
		bool							IsEnabled() override;
		void							SetActivate() override;
		bool							IsActivated() override;
		bool							IsRenderingAsActivated() override;
		void							ShowInTaskBar() override;
		void							HideInTaskBar() override;
		bool							IsAppearedInTaskBar() override;
		void							EnableActivate() override;
		void							DisableActivate() override;
		bool							IsEnabledActivate() override;
		bool							RequireCapture() override;
		bool							ReleaseCapture() override;
		bool							IsCapturing() override;
		bool							GetMaximizedBox() override;
		void							SetMaximizedBox(bool visible) override;
		bool							GetMinimizedBox() override;
		void							SetMinimizedBox(bool visible) override;
		bool							GetBorder() override;
		void							SetBorder(bool visible) override;
		bool							GetSizeBox() override;
		void							SetSizeBox(bool visible) override;
		bool							GetIconVisible() override;
		void							SetIconVisible(bool visible) override;
		bool							GetTitleBar() override;
		void							SetTitleBar(bool visible) override;
		bool							GetTopMost() override;
		void							SetTopMost(bool topmost) override;
		void							SupressAlt() override;
		bool							InstallListener(INativeWindowListener* listener) override;
		bool							UninstallListener(INativeWindowListener* listener) override;
		void							RedrawContent() override;
	};
}

#endif