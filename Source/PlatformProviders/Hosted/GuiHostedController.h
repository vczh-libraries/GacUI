/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Hosted Window

Interfaces:
  GuiHostedController

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIHOSTEDCONTROLLER
#define VCZH_PRESENTATION_GUIHOSTEDCONTROLLER

#include "GuiHostedApplication.h"
#include "GuiHostedWindow.h"
#include "GuiHostedGraphics.h"
#include "../../Utilities/SharedServices/GuiSharedCallbackService.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiHostedController
***********************************************************************/

		class GuiHostedController
			: public Object
			, protected hosted_window_manager::WindowManager<GuiHostedWindow*>
			, protected INativeWindowListener
			, protected INativeControllerListener
			, public INativeController
			, protected INativeAsyncService
			, protected INativeScreenService
			, protected INativeScreen
			, protected INativeWindowService
			, protected IGuiHostedApplication
		{
			friend class GuiHostedWindow;
			friend class elements::GuiHostedGraphicsResourceManager;
		protected:
			SharedCallbackService										callbackService;
			hosted_window_manager::WindowManager<GuiHostedWindow*>*		wmManager = nullptr;
			bool														windowsUpdatedInLastFrame = false;
			INativeController*											nativeController = nullptr;
			elements::GuiHostedGraphicsResourceManager*					hostedResourceManager = nullptr;
			collections::SortedList<Ptr<GuiHostedWindow>>				createdWindows;

			INativeWindow*												nativeWindow = nullptr;
			bool														nativeWindowDestroyed = false;

			GuiHostedWindow*											mainWindow = nullptr;
			GuiHostedWindow*											capturingWindow = nullptr;
			GuiHostedWindow*											enteringWindow = nullptr;

			NativePoint													hoveringLocation{ -1,-1 };
			GuiHostedWindow*											hoveringWindow = nullptr;
			GuiHostedWindow*											lastFocusedWindow = nullptr;

			enum class WindowManagerOperation
			{
				None,
				Title,
				BorderLeft,
				BorderRight,
				BorderTop,
				BorderBottom,
				BorderLeftTop,
				BorderRightTop,
				BorderLeftBottom,
				BorderRightBottom,
			};
			WindowManagerOperation										wmOperation = WindowManagerOperation::None;
			GuiHostedWindow*											wmWindow = nullptr;
			NativePoint													wmRelative;

			NativePoint						GetPointInClientSpace(NativePoint location);
			GuiHostedWindow*				HitTestInClientSpace(NativePoint location);
			void							UpdateHoveringWindow(Nullable<NativePoint> location);
			void							UpdateEnteringWindow(GuiHostedWindow* window);

			// =============================================================
			// WindowManager<GuiHostedWindow*>
			// =============================================================

			void							OnOpened(hosted_window_manager::Window<GuiHostedWindow*>* window) override;
			void							OnClosed(hosted_window_manager::Window<GuiHostedWindow*>* window) override;
			void							OnEnabled(hosted_window_manager::Window<GuiHostedWindow*>* window) override;
			void							OnDisabled(hosted_window_manager::Window<GuiHostedWindow*>* window) override;
			void							OnGotFocus(hosted_window_manager::Window<GuiHostedWindow*>* window) override;
			void							OnLostFocus(hosted_window_manager::Window<GuiHostedWindow*>* window) override;
			void							OnActivated(hosted_window_manager::Window<GuiHostedWindow*>* window) override;
			void							OnDeactivated(hosted_window_manager::Window<GuiHostedWindow*>* window) override;

			// =============================================================
			// INativeWindowListener
			// =============================================================

			HitTestResult					HitTest(NativePoint location) override;
			void							Moving(NativeRect& bounds, bool fixSizeOnly, bool draggingBorder) override;
			void							Moved() override;
			void							DpiChanged(bool preparing) override;
			void							GotFocus() override;
			void							LostFocus() override;
			void							Opened() override;
			void							BeforeClosing(bool& cancel) override;
			void							AfterClosing() override;
			void							Paint() override;
			
			GuiHostedWindow*				GetSelectedWindow_MouseDown(const NativeWindowMouseInfo& info);
			GuiHostedWindow*				GetSelectedWindow_MouseMoving(const NativeWindowMouseInfo& info);
			GuiHostedWindow*				GetSelectedWindow_Other(const NativeWindowMouseInfo& info);

			void							PreAction_LeftButtonDown(const NativeWindowMouseInfo& info);
			void							PreAction_MouseDown(const NativeWindowMouseInfo& info);
			void							PreAction_MouseMoving(const NativeWindowMouseInfo& info);
			void							PreAction_Other(const NativeWindowMouseInfo& info);

			void							PostAction_LeftButtonUp(GuiHostedWindow* selectedWindow, const NativeWindowMouseInfo& info);
			void							PostAction_Other(GuiHostedWindow* selectedWindow, const NativeWindowMouseInfo& info);

			template<
				void (GuiHostedController::* PreAction)(const NativeWindowMouseInfo&),
				GuiHostedWindow* (GuiHostedController::* GetSelectedWindow)(const NativeWindowMouseInfo&),
				void (GuiHostedController::* PostAction)(GuiHostedWindow*, const NativeWindowMouseInfo&),
				void (INativeWindowListener::* Callback)(const NativeWindowMouseInfo&)
				>
			void							HandleMouseCallback(const NativeWindowMouseInfo& info);

			template<
				typename TInfo,
				void (INativeWindowListener::* Callback)(const TInfo&)
			>
			void							HandleKeyboardCallback(const TInfo& info);

			void							LeftButtonDown(const NativeWindowMouseInfo& info) override;
			void							LeftButtonUp(const NativeWindowMouseInfo& info) override;
			void							LeftButtonDoubleClick(const NativeWindowMouseInfo& info) override;
			void							RightButtonDown(const NativeWindowMouseInfo& info) override;
			void							RightButtonUp(const NativeWindowMouseInfo& info) override;
			void							RightButtonDoubleClick(const NativeWindowMouseInfo& info) override;
			void							MiddleButtonDown(const NativeWindowMouseInfo& info) override;
			void							MiddleButtonUp(const NativeWindowMouseInfo& info) override;
			void							MiddleButtonDoubleClick(const NativeWindowMouseInfo& info) override;
			void							HorizontalWheel(const NativeWindowMouseInfo& info) override;
			void							VerticalWheel(const NativeWindowMouseInfo& info) override;
			void							MouseMoving(const NativeWindowMouseInfo& info) override;
			void							MouseEntered() override;
			void							MouseLeaved() override;

			void							KeyDown(const NativeWindowKeyInfo& info) override;
			void							KeyUp(const NativeWindowKeyInfo& info) override;
			void							Char(const NativeWindowCharInfo& info) override;

			// =============================================================
			// INativeControllerListener
			// =============================================================

			void							GlobalTimer() override;
			void							ClipboardUpdated() override;
			void							GlobalShortcutKeyActivated(vint id) override;
			void							NativeWindowDestroying(INativeWindow* window) override;

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
			
			const NativeWindowFrameConfig&	GetMainWindowFrameConfig() override;
			const NativeWindowFrameConfig&	GetNonMainWindowFrameConfig() override;
			INativeWindow*					CreateNativeWindow(INativeWindow::WindowMode windowMode) override;
			void							DestroyNativeWindow(INativeWindow* window) override;
			INativeWindow*					GetMainWindow() override;
			INativeWindow*					GetWindow(NativePoint location) override;

			void							SettingHostedWindowsBeforeRunning();
			void							DestroyHostedWindowsAfterRunning();
			void							Run(INativeWindow* window) override;
			bool							RunOneCycle() override;

			// =============================================================
			// IGuiHostedApplication
			// =============================================================

			INativeWindow*					GetNativeWindowHost() override;
		public:
			GuiHostedController(INativeController* _nativeController);
			~GuiHostedController();

			IGuiHostedApplication*			GetHostedApplication();
			void							Initialize();
			void							Finalize();
			void							RequestRefresh();

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
		};
	}
}

#endif