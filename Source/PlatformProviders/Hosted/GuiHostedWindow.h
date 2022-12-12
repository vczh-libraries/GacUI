/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Hosted Window

Interfaces:
  GuiHostedController

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

/***********************************************************************
GuiHostedController
***********************************************************************/

		class GuiHostedController
			: public Object
			, protected INativeWindowListener
			, public INativeController
			, protected INativeAsyncService
			, protected INativeDialogService
			, protected INativeScreenService
			, protected INativeScreen
			, protected INativeWindowService
		{
			friend class GuiHostedWindow;
		protected:
			INativeController*								nativeController = nullptr;
			INativeWindow*									nativeWindow = nullptr;
			bool											nativeWindowDestroyed = false;
			GuiHostedWindow*								mainWindow = nullptr;
			collections::SortedList<Ptr<GuiHostedWindow>>	createdWindows;

			// =============================================================
			// INativeWindowListener
			// =============================================================

			HitTestResult					HitTest(NativePoint location);
			void							Moving(NativeRect& bounds, bool fixSizeOnly, bool draggingBorder);
			void							Moved();
			void							DpiChanged();
			void							Enabled();
			void							Disabled();
			void							GotFocus();
			void							LostFocus();
			void							Activated();
			void							Deactivated();
			void							Opened();
			void							Closing(bool& cancel);
			void							Closed();
			void							Paint();
			void							Destroying();
			void							Destroyed();

			void							LeftButtonDown(const NativeWindowMouseInfo& info);
			void							LeftButtonUp(const NativeWindowMouseInfo& info);
			void							LeftButtonDoubleClick(const NativeWindowMouseInfo& info);
			void							RightButtonDown(const NativeWindowMouseInfo& info);
			void							RightButtonUp(const NativeWindowMouseInfo& info);
			void							RightButtonDoubleClick(const NativeWindowMouseInfo& info);
			void							MiddleButtonDown(const NativeWindowMouseInfo& info);
			void							MiddleButtonUp(const NativeWindowMouseInfo& info);
			void							MiddleButtonDoubleClick(const NativeWindowMouseInfo& info);
			void							HorizontalWheel(const NativeWindowMouseInfo& info);
			void							VerticalWheel(const NativeWindowMouseInfo& info);
			void							MouseMoving(const NativeWindowMouseInfo& info);
			void							MouseEntered();
			void							MouseLeaved();

			void							KeyDown(const NativeWindowKeyInfo& info);
			void							KeyUp(const NativeWindowKeyInfo& info);
			void							SysKeyDown(const NativeWindowKeyInfo& info);
			void							SysKeyUp(const NativeWindowKeyInfo& info);
			void							Char(const NativeWindowCharInfo& info);

			// =============================================================
			// INativeController
			// =============================================================

			INativeCallbackService*			CallbackService() override;
			INativeResourceService*			ResourceService() override;
			INativeAsyncService*			AsyncService() override;
			INativeClipboardService*		ClipboardService() override;
			INativeImageService*			ImageService() override;
			INativeInputService*			InputService() override;
			INativeDialogService*			DialogService() override;
			WString							GetExecutablePath() override;
			
			INativeScreenService*			ScreenService() override;
			INativeWindowService*			WindowService() override;

			// =============================================================
			// INativeAsyncService
			// =============================================================

			bool							IsInMainThread(INativeWindow* window) override;
			void							InvokeAsync(const Func<void()>& proc) override;
			void							InvokeInMainThread(INativeWindow* window, const Func<void()>& proc) override;
			bool							InvokeInMainThreadAndWait(INativeWindow* window, const Func<void()>& proc, vint milliseconds) override;
			Ptr<INativeDelay>				DelayExecute(const Func<void()>& proc, vint milliseconds) override;
			Ptr<INativeDelay>				DelayExecuteInMainThread(const Func<void()>& proc, vint milliseconds) override;

			// =============================================================
			// INativeDialogService
			// =============================================================

			MessageBoxButtonsOutput			ShowMessageBox(INativeWindow* window, const WString& text, const WString& title, MessageBoxButtonsInput buttons, MessageBoxDefaultButton defaultButton, MessageBoxIcons icon, MessageBoxModalOptions modal) override;
			bool							ShowColorDialog(INativeWindow* window, Color& selection, bool selected, ColorDialogCustomColorOptions customColorOptions, Color* customColors) override;
			bool							ShowFontDialog(INativeWindow* window, FontProperties& selectionFont, Color& selectionColor, bool selected, bool showEffect, bool forceFontExist) override;
			bool							ShowFileDialog(INativeWindow* window, collections::List<WString>& selectionFileNames, vint& selectionFilterIndex, FileDialogTypes dialogType, const WString& title, const WString& initialFileName, const WString& initialDirectory, const WString& defaultExtension, const WString& filter, FileDialogOptions options) override;

			// =============================================================
			// INativeScreenService
			// =============================================================

			vint							GetScreenCount() override;
			INativeScreen*					GetScreen(vint index) override;
			INativeScreen*					GetScreen(INativeWindow* window) override;

			// =============================================================
			// INativeScreen
			// =============================================================

			NativeRect						GetBounds() override;
			NativeRect						GetClientBounds() override;
			WString							GetName() override;
			bool							IsPrimary() override;
			double							GetScalingX() override;
			double							GetScalingY() override;

			// =============================================================
			// INativeWindowService
			// =============================================================

			INativeWindow*					CreateNativeWindow(INativeWindow::WindowMode windowMode) override;
			void							DestroyNativeWindow(INativeWindow* window) override;
			INativeWindow*					GetMainWindow() override;
			INativeWindow*					GetWindow(NativePoint location) override;
			void							Run(INativeWindow* window) override;

		protected:

			void							EnsureNativeWindowCreated();
			void							EnsureNativeWindowDestroyed();
		public:
			GuiHostedController(INativeController* _nativeController);
			~GuiHostedController();
		};
	}
}

#endif