/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Hosted Window

Interfaces:
  GuiHostedWindow

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIHOSTEDWINDOW
#define VCZH_PRESENTATION_GUIHOSTEDWINDOW

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		class GuiHostedController;

/***********************************************************************
GuiHostedWindow
***********************************************************************/

		class GuiHostedWindow
			: public Object
			, public INativeWindow
		{
			friend class GuiHostedController;
		protected:
			GuiHostedController*							controller = nullptr;
			INativeWindow::WindowMode						windowMode = INativeWindow::WindowMode::Normal;
			collections::List<INativeWindowListener*>		listeners;

		public:
			GuiHostedWindow(GuiHostedController* _controller, INativeWindow::WindowMode _windowMode);
			~GuiHostedWindow();

			// =============================================================
			// INativeWindowListener
			// =============================================================

			bool							IsActivelyRefreshing() override;
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
			void							SetTitle(WString title) override;
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
			void							SetFocus() override;
			bool							IsFocused() override;
			void							SetActivate() override;
			bool							IsActivated() override;
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
}

#endif