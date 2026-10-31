Main Window and Modal Window Management

GacUI's window management system provides a sophisticated multi-layered architecture that enables consistent main window and modal window behavior across all supported platforms while maintaining platform-specific optimizations.

## Application Loop Architecture

The application loop operates through a hierarchical delegation pattern from high-level application management down to platform-specific message processing.

### GuiApplication Layer

`GuiApplication` serves as the primary application framework entry point with two essential methods:

- `Run(GuiWindow* mainWindow)`: Establishes the main window, delegates to the native controller's window service, and manages application lifecycle
- `RunOneCycle()`: Processes a single iteration of the message loop by delegating to the platform-specific window service

The application layer maintains the main window reference and coordinates with the underlying native controller through `GetCurrentController()->WindowService()`.

### INativeWindowService Abstraction

The `INativeWindowService` interface provides platform-agnostic window lifecycle management:

- `Run(INativeWindow* window)`: Initiates the blocking message loop until the window closes
- `RunOneCycle()`: Processes one iteration of events and returns continuation status

This abstraction enables consistent behavior across Windows native, hosted mode, and remote mode implementations.

## Platform-Specific Implementations

### Windows Native Platform

The Windows implementation follows traditional Win32 message pump patterns:
- Direct integration with Windows message queue using `GetMessage`/`DispatchMessage`
- Each `GuiWindow` maps to a dedicated HWND
- Application exits when the main window's HWND is destroyed

### Hosted Mode Implementation

`GuiHostedController` virtualizes multiple windows within a single native window:
- Delegates to underlying native controller while managing virtual windows through `WindowManager<GuiHostedWindow*>`
- Performs coordinate transformation for multiple virtual windows
- Provides windowing capabilities within embedded environments

### Remote Mode Implementation

`GuiRemoteController` completely abstracts platform dependencies:
- Processes events through `IGuiRemoteProtocol` instead of OS message queues
- All windows are virtualized and rendered through protocol messages
- Application exits when the virtual main window is destroyed; renderer disconnection alone allows the core to wait for reconnection

## Modal Window System

### Non-Blocking Modal Architecture

Modal windows in GacUI provide traditional blocking semantics without actually blocking the underlying event processing system. This is achieved through an event-driven callback architecture:

**Event-Driven Completion**: `ShowModal` returns immediately after setting up the modal state and relies on the `WindowReadyToClose` event to execute completion callbacks asynchronously.

**Selective Window Disabling**: Modal behavior is created by disabling the owner window (`owner->SetEnabled(false)`) while keeping the modal window and message loop fully operational.

**Continuous Message Processing**: The underlying `INativeWindowService::RunOneCycle` continues processing all events, timers, and async operations normally, ensuring the application remains responsive.

### Modal Window Variants

**ShowModal(owner, callback):**
- Basic modal behavior with custom completion callback
- Owner window disabled until modal closes
- Establishes parent-child relationship and focus management

**ShowModalAndDelete(owner, callback):**
- Combines modal behavior with automatic cleanup via `DeleteAfterProcessingAllEvents`
- Provides optional separate callback for deletion completion
- Ensures proper resource cleanup after modal completion

**ShowModalAsync(owner):**
- Returns `Ptr<reflection::description::IAsync>` for async/await integration; the implementation creates an `IFuture` internally and returns it through the async interface
- Demonstrates the truly non-blocking nature of the modal system
- Enables modern asynchronous programming patterns

### Modal Record Management

The system tracks modal window chains through `ShowModalRecord` structures:

```cpp
struct ShowModalRecord {
    GuiWindow* origin;   // Root window that started modal chain
    GuiWindow* current;  // Currently active modal window
};
```

This enables nested modal windows and proper cleanup when modal chains are established.

### Hosted Mode Modal Considerations

In hosted mode, modal windows receive special handling in `GuiWindow::BeforeClosing()`:
- Main window close attempts are intercepted when modal windows exist
- Active modal windows are either hidden or focused instead of allowing main window closure
- Prevents premature application termination while modal dialogs are active

## Cross-Platform Event Processing

### Explicitly Owned Popups

`GuiPopup::ShowPopup` resolves each control-relative owner to its native window. The native parent tree supplies the relationship; `hasPopupOwner` distinguishes an explicitly supplied owner from hosted main-window parenting. Reuse replaces this association, and the screen-position overload clears it. The application's existing `openingPopups` list supplies the popup inventory; modal records continue to represent only the modal chain.

`GuiControlHost::AfterClosing` dismisses owned popups after cancellation is resolved and before `WindowReadyToClose`. Destruction and native-window detachment dismiss them before the relationship is lost. `GuiApplication::CloseOwnedPopups` rescans the existing list after each synchronous child hide, allowing callbacks to mutate it. A popup's own accepted-close hook closes its descendants first. Unrelated normal/modal windows and popups remain outside this traversal.

Windows popup Hide sends `WM_CLOSE` synchronously; normal windows retain posted close requests. The native outside-click traversal snapshots children with disposal flags and closes descendants first, since synchronous callbacks can mutate or destroy children. Hosted and remote subwindows use the same application lifecycle hook before visibility changes or reparenting.

`GuiPopup::Closed` invalidates positioning, clears ownership, removes the popup from the opening list and detaches its native parent before public closure notifications. `GuiTooltip::Closed` first detaches temporary content, bypassing hover-based delayed dismissal. Menu closure notifications release menu-service, submenu-button and ALT state through their normal paths; dismissal does not execute commands.

Owner-driven dismissal sets `closingForOwner` before Hide. `GuiPopup::BeforeClosing` still invokes the normal closing subscribers but clears cancellation for this mandatory path; an independent popup close remains cancellable. `Closed` clears the transient flag before public notifications and subsequent reuse.

### Platform-Specific Message Handling

Despite different underlying implementations, modal windows work consistently across all platforms:

**Windows Native**: Win32 message pump continues processing `WM_*` messages while owner windows are disabled, allowing modal windows to remain interactive.

**Hosted Mode**: Virtual window manager continues processing events within the host window, with coordinate transformation handling multiple virtual windows.

**Remote Mode**: Protocol event processing continues normally, with modal state managed through the protocol abstraction layer.

### Memory Management During Modal Operations

The system handles object lifetime carefully during modal operations:
- Uses `GetDisposedFlag()` to detect window destruction during modal operation
- Employs `InvokeInMainThread` for safe event handler cleanup
- Ensures proper resource management even if modal windows are unexpectedly destroyed

### Async Operations Integration

Modal windows integrate seamlessly with GacUI's async infrastructure:
- Timer callbacks continue through `CallbackService.InvokeGlobalTimer()`
- Async task execution proceeds via `asyncService.ExecuteAsyncTasks()`
- All platforms maintain consistent async processing during modal operations

## Cross-Platform Consistency

The architecture successfully abstracts platform differences while providing rich windowing capabilities:

**Unified Modal API**: Same modal window interface across all platforms with identical semantics and behavior patterns.

**Consistent Event Processing**: All platforms maintain responsive user interfaces through continued message loop processing during modal operations.

**Platform-Optimized Implementation**: Each platform uses its optimal event processing mechanism while maintaining the same high-level behavior.

This design enables developers to create sophisticated GUI applications with complex modal dialog patterns that work consistently across Windows native, hosted, and remote environments while maintaining the responsiveness expected in modern applications.
