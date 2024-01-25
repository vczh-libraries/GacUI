/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Hosted Window

Interfaces:
  GuiHostedWindow

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIHOSTEDCONTROLLER_GUIHOSTEDWINDOW
#define VCZH_PRESENTATION_GUIHOSTEDCONTROLLER_GUIHOSTEDWINDOW

#include "../../NativeWindow/GuiNativeWindow.h"
#include "GuiHostedWindowManager.h"

namespace vl
{
	namespace presentation
	{
		class GuiHostedWindow;
		class GuiHostedController;

/***********************************************************************
Proxy
***********************************************************************/

		struct GuiHostedWindowData
		{
			hosted_window_manager::Window<GuiHostedWindow*>		wmWindow;
			GuiHostedController*								controller = nullptr;
			INativeWindow::WindowMode							windowMode = INativeWindow::WindowMode::Normal;
			collections::List<INativeWindowListener*>			listeners;

			WString												windowTitle;
			INativeCursor*										windowCursor = nullptr;
			NativePoint											windowCaretPoint;
			Ptr<GuiImageData>									windowIcon;

			bool												windowMaximizedBox = true;
			bool												windowMinimizedBox = true;
			bool												windowBorder = true;
			bool												windowSizeBox = true;
			bool												windowIconVisible = true;
			bool												windowTitleBar = true;

			INativeWindow::WindowSizeState						windowSizeState = INativeWindow::Restored;
			bool												windowShowInTaskBar = true;
			bool												windowEnabledActivate = true;
			bool												windowCustomFrameMode = true;

			GuiHostedWindowData(GuiHostedController* _controller, GuiHostedWindow* _window, INativeWindow::WindowMode _windowMode)
				: wmWindow(_window)
				, controller(_controller)
				, windowMode(_windowMode)
			{
			}
		};

		class IGuiHostedWindowProxy
			: public virtual Interface
		{
		public:
			virtual void			CheckAndSyncProperties() = 0;

			virtual NativeRect		FixBounds(const NativeRect& bounds) = 0;
			virtual void			UpdateBounds() = 0;
			virtual void			UpdateTitle() = 0;
			virtual void			UpdateIcon() = 0;
			virtual void			UpdateEnabled() = 0;
			virtual void			UpdateTopMost() = 0;

			virtual void			UpdateMaximizedBox() = 0;
			virtual void			UpdateMinimizedBox() = 0;
			virtual void			UpdateBorderVisible() = 0;
			virtual void			UpdateSizeBox() = 0;
			virtual void			UpdateIconVisible() = 0;
			virtual void			UpdateTitleBar() = 0;

			virtual void			UpdateShowInTaskBar() = 0;
			virtual void			UpdateEnabledActivate() = 0;
			virtual void			UpdateCustomFrameMode() = 0;

			virtual void			Show() = 0;
			virtual void			ShowDeactivated() = 0;
			virtual void			ShowRestored() = 0;
			virtual void			ShowMaximized() = 0;
			virtual void			ShowMinimized() = 0;
			virtual void			Hide() = 0;
			virtual void			Close() = 0;
			virtual void			SetFocus() = 0;
		};

		extern Ptr<IGuiHostedWindowProxy>		CreatePlaceholderHostedWindowProxy(GuiHostedWindowData* data);
		extern Ptr<IGuiHostedWindowProxy>		CreateMainHostedWindowProxy(GuiHostedWindowData* data, INativeWindow* nativeWindow);
		extern Ptr<IGuiHostedWindowProxy>		CreateNonMainHostedWindowProxy(GuiHostedWindowData* data, INativeWindow* nativeWindow);

/***********************************************************************
GuiHostedWindow
***********************************************************************/

		class GuiHostedWindow
			: public Object
			, public INativeWindow
			, protected GuiHostedWindowData
		{
			friend class GuiHostedController;
		protected:
			Ptr<IGuiHostedWindowProxy>		proxy;

			void							BecomeMainWindow();
			void							BecomeNonMainWindow();
			void							BecomeFocusedWindow();
			void							BecomeHoveringWindow();

		public:
			GuiHostedWindow(GuiHostedController* _controller, INativeWindow::WindowMode _windowMode);
			~GuiHostedWindow();

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
}

#endif