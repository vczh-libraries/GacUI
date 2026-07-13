Hosted Mode Window Management

Hosted mode is a GacUI operating mode in which the entire GUI application runs inside a single OS native window. Instead of creating one native window per `GuiWindow`, all sub-windows, dialogs, popups, and menus are rendered as graphics within the sole native window. The hosted mode implements its own window manager to handle z-ordering, hit testing, focus, activation, dragging, resizing, and input dispatching — functions normally provided by the OS.

The implementation lives in `Source/PlatformProviders/Hosted/` with these files:
- `GuiHostedController.h/.cpp` — the core controller wrapping a native controller
- `GuiHostedWindow.h/.cpp` — virtual window implementing `INativeWindow`
- `GuiHostedWindowManager.h` — template-based window management algorithm
- `GuiHostedGraphics.h/.cpp` — graphics resource manager wrapper
- `GuiHostedApplication.h/.cpp` — global hosted application accessor
- `GuiHostedWindowProxy_Main.cpp` — proxy for the main window
- `GuiHostedWindowProxy_NonMain.cpp` — proxy for non-main windows
- `GuiHostedWindowProxy_Placeholder.cpp` — proxy for windows before role assignment

## Entry Points and Activation

Hosted mode is activated through specific setup functions:
- `SetupHostedWindowsDirect2DRenderer()` calls `SetupWindowsDirect2DRendererInternal(true, false)`
- `SetupHostedWindowsGDIRenderer()` calls `SetupWindowsGDIRendererInternal(true, false)`
- `SetupRemoteNativeController(protocol)` creates a `GuiHostedController` wrapping `GuiRemoteController` — remote mode inherently requires hosted mode

The internal setup functions (`SetupWindowsDirect2DRendererInternal`, `SetupWindowsGDIRendererInternal`) take a `bool hosted` parameter. When `hosted` is true:
1. `StartWindowsNativeController(hInstance)` creates the real native controller
2. A `GuiHostedController` is created wrapping the native controller
3. `SetNativeController(hostedController)` replaces the global native controller
4. `SetHostedApplication(hostedController->GetHostedApplication())` sets the hosted application global
5. `RendererMainDirect2D` / `RendererMainGDI` is called with the hosted controller
6. Inside the renderer main, `GuiHostedGraphicsResourceManager` wraps the native resource manager
7. `hostedController->Initialize()` creates the single native window
8. `GuiApplicationMain()` runs the application
9. `hostedController->Finalize()` tears down

For remote mode in `SetupRemoteNativeController`:
1. `GuiRemoteController` wraps the protocol
2. `GuiHostedController` wraps the `GuiRemoteController`
3. Both remote and hosted resource managers are created in a chain
4. The same initialize/run/finalize sequence applies

## GuiHostedController Architecture

`GuiHostedController` is the central class of hosted mode. It inherits from:
- `Object` — the GacUI base class
- `hosted_window_manager::WindowManager<GuiHostedWindow*>` — the window management algorithm
- `INativeWindowListener` — to listen to events on the single real native window
- `INativeControllerListener` — to receive global timer, clipboard, and shortcut events
- `INativeController` — to replace the real native controller for the rest of GacUI
- `INativeAsyncService` — to redirect async operations through the real native controller
- `INativeScreenService` / `INativeScreen` — to provide a virtual single screen
- `INativeWindowService` — to create, destroy, and manage virtual windows
- `IGuiHostedApplication` — to expose the native window host

### Service Delegation

The controller delegates most services to the underlying native controller:
- `ResourceService()` → `nativeController->ResourceService()`
- `ClipboardService()` → `nativeController->ClipboardService()`
- `ImageService()` → `nativeController->ImageService()`
- `InputService()` → `nativeController->InputService()`
- `DialogService()` → returns `nullptr` (replaced by `FakeDialogServiceBase`)

Services it implements itself:
- `CallbackService()` → local `SharedCallbackService` instance
- `AsyncService()` → itself (delegates internally to native async service, translates window references)
- `ScreenService()` → itself (provides a single virtual screen)
- `WindowService()` → itself (manages virtual hosted windows)

### Virtual Screen

`GuiHostedController` acts as both `INativeScreenService` and `INativeScreen`:
- `GetScreenCount()` returns 1
- `GetScreen()` returns itself
- Screen bounds are derived from the native window's client size
- DPI scaling is forwarded from the real screen

### Native Window Lifecycle

`Initialize()` creates one real native OS window via `nativeController->WindowService()->CreateNativeWindow()` and installs itself as `INativeWindowListener` on it.

`Finalize()` destroys the native window and uninstalls listeners.

### Window Frame Configuration

`GuiHostedController` provides two frame configs:
- `GetMainWindowFrameConfig()` — delegates to the native controller (the main window gets the real OS frame)
- `GetNonMainWindowFrameConfig()` — returns a static config with `MaximizedBoxOption = AlwaysFalse`, `MinimizedBoxOption = AlwaysFalse`, `CustomFrameEnabled = AlwaysTrue` (non-main windows are always custom-frame rendered)

## Window Manager Template

### WindowManager<T> (GuiHostedWindowManager.h)

The core window management algorithm is template class `hosted_window_manager::WindowManager<T>` parameterized by window ID type. It maintains:
- `registeredWindows` — dictionary of all registered windows
- `ordinaryWindowsInOrder` — z-ordered list of normal windows (front to back)
- `topMostedWindowsInOrder` — z-ordered list of top-most windows (front to back)
- `mainWindow` — the root window
- `activeWindow` — currently focused window
- `needRefresh` — dirty flag for rendering

Pure virtual callbacks used by `GuiHostedController`:
- `OnOpened` / `OnClosed` — fire `Opened` / `Closed` events on hosted window listeners
- `OnEnabled` / `OnDisabled` — fire `Enabled` / `Disabled` events
- `OnGotFocus` / `OnLostFocus` — fire `GotFocus` / `LostFocus` events
- `OnActivated` / `OnDeactivated` — fire `Activated` / `Deactivated` events

### Window<T>

Each `Window<T>` holds:
- `parent` — parent window pointer forming a tree structure
- `children` — child windows list
- `bounds` — position and size in the parent's client coordinate space
- `topMost`, `visible`, `enabled`, `active`, `renderedAsActive` — state flags

Key operations:
- `SetParent(value)` — reparents the window; null parent defaults to main window; updates z-order via `FixWindowInOrder`
- `SetVisible(bool)` — shows or hides; triggers `FixWindowInOrder` and `OnOpened`/`OnClosed` callbacks
- `SetTopMost(bool)` — toggles top-most status; moves window between ordinary and top-most z-order lists
- `SetEnabled(bool)` — enables or disables; triggers callbacks
- `BringToFront()` — moves window and its visible subtree to the front of z-order within its priority level
- `Activate()` — sets focus; finds common parent with previously active window; updates `renderedAsActive` chain; calls `BringToFront`
- `Deactivate()` — walks up parent chain to find next enabled window to activate
- `Show()` — shorthand for `SetVisible(true)` + `Activate()`

### Z-Ordering

Two separate lists (`ordinaryWindowsInOrder` and `topMostedWindowsInOrder`) track window order. `IsEventuallyTopMost()` checks if a window or any ancestor is top-most and visible. `FixWindowInOrder` ensures windows are placed in the correct list based on their top-most status. `EnsureChildrenMovedInFrontOf` maintains the invariant that children appear before (in front of) their parent in the z-order list.

### HitTest

`WindowManager::HitTest(position)` iterates `topMostedWindowsInOrder` then `ordinaryWindowsInOrder`, returning the first visible window whose bounds contain the position — giving top-most windows priority.

### Start and Stop

`WindowManager::Start(mainWindow)` makes orphan windows children of the main window. `WindowManager::Stop()` clears all state and fires deactivation/close events.

## GuiHostedWindow — Virtual INativeWindow

`GuiHostedWindow` implements `INativeWindow` and inherits from `GuiHostedWindowData`, which holds all window properties (title, icon, cursor, size state, border options, etc.). Each `GuiHostedWindow` contains a `hosted_window_manager::Window<GuiHostedWindow*> wmWindow` embedded in its data.

### Proxy Pattern

`GuiHostedWindow` delegates behavior to an `IGuiHostedWindowProxy`. There are three proxy types:
1. **PlaceholderProxy** (`GuiHostedWindowProxy_Placeholder.cpp`) — initial no-op proxy for newly created windows before role assignment
2. **MainProxy** (`GuiHostedWindowProxy_Main.cpp`, `GuiMainHostedWindowProxy`) — for the main window; delegates title, icon, size, border properties, maximized/minimized/show/hide to the real native window
3. **NonMainProxy** (`GuiHostedWindowProxy_NonMain.cpp`, `GuiNonMainHostedWindowProxy`) — for all other windows; operates purely through the window manager; disallows maximized/minimized boxes; enforces custom frame mode when system borders are used

`BecomeMainWindow()` and `BecomeNonMainWindow()` switch the proxy. This happens in `SettingHostedWindowsBeforeRunning()` when the main window is known, and in `CreateNativeWindow()` for windows created after the main window.

### Coordinate System

- `GetRenderingOffset()` returns the window's top-left position in the native window's client space as `NativeSize`
- `GetBounds()` returns `wmWindow.bounds` directly (position is in parent's client space)
- `GetClientSize()` equals `GetBounds().GetSize()` (no frame for hosted windows)
- `GetClientBoundsInScreen()` equals `GetBounds()` (screen = client area of native window)
- DPI conversion functions delegate to the real native window

### Capture and Cursor

- `RequireCapture()` sets `controller->capturingWindow` and calls `nativeWindow->RequireCapture()`
- `ReleaseCapture()` clears `controller->capturingWindow` and calls `nativeWindow->ReleaseCapture()`
- `SetWindowCursor()` only updates the real native window cursor if this is the currently hovering window
- `SetCaretPoint()` adjusts by `GetRenderingOffset()` and sets on the real native window if this is the active window

## Window Creation and Destruction

### CreateNativeWindow(windowMode)

1. Creates a `GuiHostedWindow` with placeholder proxy
2. Adds to `createdWindows` sorted list
3. Registers with `wmManager`
4. Fires `InvokeNativeWindowCreated` callback
5. If main window already exists, calls `BecomeNonMainWindow()`

### DestroyNativeWindow(window)

1. Clears references (`enteringWindow`, `hoveringWindow`, `capturingWindow`, and WM-tracking pointers)
2. Fires `Destroying` on listeners
3. Fires `InvokeNativeWindowDestroying` callback
4. Unregisters from `wmManager` (which reparents children)
5. Removes from `createdWindows`

### Run(window)

1. Sets `mainWindow`
2. Calls `SettingHostedWindowsBeforeRunning()` which:
   - Switches all windows to their proper proxies (main/non-main)
   - Centers native window on screen
   - Calls `wmManager->Start(&mainWindow->wmWindow)`
3. Delegates to `nativeController->WindowService()->Run(nativeWindow)` for the message loop
4. Handles exceptions in unit test mode
5. Calls `DestroyHostedWindowsAfterRunning()` to tear down all hosted windows

## Input Event Dispatching

### Mouse Events

`GuiHostedController` listens to mouse events on the single native window. Mouse dispatching uses a template-based system:

`HandleMouseCallback<PreAction, GetSelectedWindow, PostAction, Callback>` is the core template:
1. Calls `PreAction` (may trigger window manager operations)
2. If not in a window-manager operation (`wmWindow == nullptr`), calls `GetSelectedWindow` to find the target
3. Adjusts mouse coordinates by subtracting the target window's `wmWindow.bounds` top-left
4. Dispatches to the target window's listeners
5. Calls `PostAction`

Three `GetSelectedWindow` strategies:
- `GetSelectedWindow_MouseDown` — closes popup windows not in the hovering chain, returns `capturingWindow` or `hoveringWindow`
- `GetSelectedWindow_MouseMoving` — updates `hoveringWindow` via `UpdateHoveringWindow` and `enteringWindow` via `UpdateEnteringWindow`
- `GetSelectedWindow_Other` — returns `capturingWindow` or `hoveringWindow`

Individual mouse event methods (`LeftButtonDown`, `MouseMoving`, etc.) are wired to the right combination of PreAction/GetSelectedWindow/PostAction via `IMPLEMENT_MOUSE_CALLBACK` macros.

### Window Manager Dragging and Resizing

`PreAction_LeftButtonDown` checks non-main enabled windows for hit test results (Title, BorderLeft, BorderRight, etc.). If a border/title hit is detected:
- Sets `wmOperation` to the appropriate `WindowManagerOperation` enum value
- Stores `wmWindow` pointer and `wmRelative` offset
- Captures the native window

`PreAction_MouseMoving` handles ongoing drag/resize when `wmWindow` is set: recalculates bounds based on mouse position and operation type, calls `Moving` and `Moved` listeners.

`PostAction_LeftButtonUp` checks for `ButtonClose` hit test to close windows, and ends window manager operations.

### Mouse Hovering and Entering

- `UpdateHoveringWindow(location)` stores mouse position and calls `HitTestInClientSpace` to find the window under cursor
- `UpdateEnteringWindow(window)` fires `MouseLeaved` on the old entering window and `MouseEntered` on the new one
- `MouseLeaved` on the native window calls `UpdateEnteringWindow(nullptr)`

### Mouse-Down Activation

`PreAction_MouseDown` activates the hovering window when a mouse button is pressed (if the window is enabled and activation-enabled).

### Popup Closure on Click

`GetSelectedWindow_MouseDown` implements the pattern of closing popup windows (non-`Normal` mode windows) that are not ancestors of the clicked window.

### Keyboard Events

`HandleKeyboardCallback` dispatches keyboard events (`KeyDown`, `KeyUp`, `Char`) to the active window's listeners.

## Rendering Pipeline

### Hosted Rendering in GlobalTimer

`GuiHostedController::GlobalTimer()` drives the rendering cycle. On each global timer tick:
1. Skip if the native window is not visible or already in hosted rendering
2. Check all visible windows' listeners for `NeedRefresh()` — if any returns true, set `needRefresh`
3. If no refresh is needed and nothing was updated last frame, skip rendering
4. Enter rendering loop:
   - Call `renderTarget->StartHostedRendering()` on the native resource manager's render target
   - Iterate ordinary windows (back to front, reversed list order) then top-most windows
   - For each window, call each listener's `ForceRefresh(false, updated, failureByResized, failureByLostDevice)`
   - Call `renderTarget->StopHostedRendering()`
   - Handle failures: `LostDevice` → `RecreateRenderTarget`; `ResizeWhileRendering` → `ResizeRenderTarget`
   - On success, call `nativeWindow->RedrawContent()` and break

### Hosted Rendering Protocol (GuiGraphicsRenderTarget)

`GuiGraphicsRenderTarget` (base class for D2D/GDI render targets) supports hosted rendering:
- `StartHostedRendering()` sets `hostedRendering = true` and calls `StartRenderingOnNativeWindow()` once
- During hosted rendering, individual `StartRendering()` / `StopRendering()` pairs do NOT call `StartRenderingOnNativeWindow()` / `StopRenderingOnNativeWindow()` — they just toggle the `rendering` flag
- `StopHostedRendering()` calls `StopRenderingOnNativeWindow()` once
- This means all hosted windows render within a single begin/end rendering session on the native render target

### Per-Window Rendering Offset (GuiGraphicsHost)

`GuiGraphicsHost::Render()` (called from `ForceRefresh`) applies the rendering offset:
1. `hostRecord.renderTarget->StartRendering()`
2. `auto nativeOffset = hostRecord.nativeWindow->GetRenderingOffset()` — for hosted windows this returns the window position
3. `auto localOffset = hostRecord.nativeWindow->Convert(nativeOffset)` — converts to logical pixels
4. `windowComposition->Render(localOffset)` — renders the composition tree at the offset
5. `hostRecord.renderTarget->StopRendering()`

The rendering offset mechanism is the key: in non-hosted mode, `GetRenderingOffset()` returns `(0,0)` because each window has its own render target. In hosted mode, it returns the window's position in the shared native window coordinate space, so all window content is rendered at the correct position within the single render target.

### GuiHostedGraphicsResourceManager

`GuiHostedGraphicsResourceManager` wraps the native resource manager:
- `GetRenderTarget(window)` always returns the native render target for the single native window, regardless of which hosted window asks
- `RecreateRenderTarget(window)` and `ResizeRenderTarget(window)` are no-ops (managed by the global timer loop)
- All other methods (element registration, renderer factories, layout provider) delegate to the native manager

## Native Window Event Forwarding

`GuiHostedController` as `INativeWindowListener` on the single real native window forwards events to hosted windows:
- `HitTest(location)` — if main window is enabled and cursor is on it, performs hit test via main window's listeners
- `Moving(bounds)` — adjusts bounds to main window's coordinate space and forwards to main window's listeners
- `Moved()` — syncs main window bounds to native window client size
- `DpiChanged(preparing)` — recreates render target and broadcasts to all hosted windows
- `GotFocus()` — reactivates last focused window (or main window)
- `LostFocus()` — stores last focused window and deactivates all
- `BeforeClosing(cancel)` / `AfterClosing()` — forwards to main window's listeners
- `Opened()` — forwards to main window's listeners

## Dialog Service

`GuiHostedController::DialogService()` returns `nullptr`, causing GacUI to use `FakeDialogServiceBase` — GacUI-implemented dialogs rendered inside the hosted environment. This is critical because OS-native dialogs would create separate windows outside the hosted environment.

`FakeDialogServiceBase` provides view models (`IMessageBoxDialogViewModel`, `IColorDialogViewModel`, `IFontDialogViewModel`, `IOpenFileDialogViewModel`, `ISaveFileDialogViewModel`) and creates corresponding GacUI windows that appear as hosted sub-windows.

## Remote Mode Integration

Remote mode (`SetupRemoteNativeController`) always uses hosted mode:
- `GuiRemoteController` implements `INativeController` through protocol communication
- `GuiHostedController` wraps `GuiRemoteController` just as it wraps Windows native controllers
- `GuiRemoteGraphicsResourceManager` wraps `GuiRemoteController`, then `GuiHostedGraphicsResourceManager` wraps that
- The single native window in remote mode is the `GuiRemoteWindow`, which sends rendering commands through the protocol rather than to a real screen
