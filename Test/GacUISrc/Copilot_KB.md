# !!!KNOWLEDGE BASE!!!

# DOCUMENT REQUEST

## TOPIC

When running GacUI on any platform that brings up `GuiApplication`, we must call `GuiApplication::Run` with the main window to start an application. And `Run` exits when the window is closed.

Read the code and analysis the how following functions and features are co-operate together:
- GuiApplication::Run and RunOneCycle
- INativeWindowService::Run and RunOneCycle
- GuiWindow::ShowModal and ShowModalAndDelete and ShowModalAsync

And analysis if there is any differences between each supported platform, including Windows, HostedMode on Windows and HostedMode on RemoteMode, on INativeWindowService::Run and RunOneCycle

### Insight

The GacUI application loop and modal window system operates through a multi-layered architecture that abstracts platform-specific differences while providing consistent functionality across all supported platforms. Here's the detailed analysis:

## Application Loop Architecture

### GuiApplication Layer
`GuiApplication::Run(GuiWindow* _mainWindow)` serves as the high-level entry point that:
- Sets the main window as a member variable
- Delegates to the native controller's window service: `GetCurrentController()->WindowService()->Run(mainWindow->GetNativeWindow())`
- Clears the main window reference when the application exits

`GuiApplication::RunOneCycle()` provides a single iteration of the message loop by directly calling `GetCurrentController()->WindowService()->RunOneCycle()`.

### INativeWindowService Platform Abstraction
The `INativeWindowService` interface provides platform-agnostic window lifecycle management through two key methods:
- `Run(INativeWindow* window)`: Starts the main message loop and blocks until the window closes
- `RunOneCycle()`: Processes one iteration of events and returns false when the application should exit

## Platform-Specific Implementations

### Windows Native Platform
The Windows implementation follows the traditional Windows message loop pattern:
- `Run()` calls the Windows `GetMessage`/`DispatchMessage` loop
- `RunOneCycle()` processes pending messages with `PeekMessage` and returns immediately
- Exit condition is triggered when the main window receives `WM_CLOSE` or is destroyed

### Hosted Mode Implementation
`GuiHostedController` provides a virtualized window environment running within a single native window:

**Run Implementation:**
```cpp
void GuiHostedController::Run(INativeWindow* window) {
    SettingHostedWindowsBeforeRunning();
    nativeController->WindowService()->Run(nativeWindow);
    DestroyHostedWindowsAfterRunning();
}
```

**RunOneCycle Implementation:**
```cpp
bool GuiHostedController::RunOneCycle() {
    return nativeController->WindowService()->RunOneCycle();
}
```

The hosted controller delegates to the underlying native controller while managing virtual windows through `WindowManager<GuiHostedWindow*>`.

### Remote Mode Implementation  
`GuiRemoteController` abstracts away all platform dependencies:

**Run Implementation:**
```cpp
void GuiRemoteController::Run(INativeWindow* window) {
    applicationRunning = true;
    window->Show();
    while (RunOneCycle());
    applicationRunning = false;
}
```

**RunOneCycle Implementation:**
```cpp
bool GuiRemoteController::RunOneCycle() {
    if (!connectionStopped) {
        processor->ProcessRemoteEvents();
        remoteMessages.Submit(disconnected);
        callbackService.InvokeGlobalTimer();
        asyncService.ExecuteAsyncTasks();
    }
    return !connectionStopped;
}
```

## Modal Window System

### Core Modal Architecture
GacUI implements modal windows through window ownership and event loop nesting:

**ShowModal Structure:**
- Creates `ShowModalRecord` to track modal window chain
- Disables owner window with `owner->SetEnabled(false)`
- Sets window parent relationship with `GetNativeWindow()->SetParent(owner->GetNativeWindow())`
- Attaches cleanup callback to `WindowReadyToClose` event

**Modal Window Chain Management:**
```cpp
struct ShowModalRecord {
    GuiWindow* origin = nullptr;   // Root window that started modal chain
    GuiWindow* current = nullptr;  // Currently active modal window
};
```

### Modal Variants

**ShowModal(owner, callback):** Basic modal behavior with custom callback
**ShowModalAndDelete(owner, callback):** Modal + automatic cleanup via `DeleteAfterProcessingAllEvents`
**ShowModalAsync(owner):** Returns `IFuture` for async/await pattern integration

### Hosted Mode Modal Considerations
In hosted mode, modal windows have special handling in `GuiWindow::BeforeClosing`:
- When the main window in hosted mode tries to close, it searches for active modal windows
- If a modal window exists, it either hides or focuses that window instead of closing
- This prevents the hosted application from terminating while modal dialogs are active

## Platform Differences Analysis

### Event Processing Differences
- **Windows Native:** Direct OS message pump integration with Windows message queue
- **Hosted Mode:** Virtualized through `WindowManager` that manages multiple virtual windows within one native window
- **Remote Mode:** Protocol-based event processing where events come through `IGuiRemoteProtocol`

### Window Lifecycle Differences
- **Windows Native:** Each GuiWindow maps to a dedicated HWND
- **Hosted Mode:** Multiple GuiHostedWindows share a single native window with coordinate transformation
- **Remote Mode:** All windows are virtualized and rendered through protocol messages

### Exit Condition Differences
- **Windows Native:** Application exits when main window's HWND is destroyed
- **Hosted Mode:** Application exits when the host native window closes or `connectionStopped` is set
- **Remote Mode:** Application exits when `connectionStopped` becomes true (protocol disconnection)

### Timer and Async Processing
All platforms converge on the same async processing model:
- Global timer callbacks through `CallbackService.InvokeGlobalTimer()`
- Async task execution through `asyncService.ExecuteAsyncTasks()`
- Event processing delegation to the appropriate platform layer

The key insight is that while the platform-specific `INativeWindowService` implementations handle low-level event processing differently, the high-level `GuiApplication` and modal window behaviors remain consistent across all platforms through careful abstraction and delegation patterns.

## IMPROVE

Explain why `ShowModal` can block without blocking user operation

### Insight

The `ShowModal` function creates an apparent "blocking" behavior for the calling code while maintaining full user interaction capabilities through a sophisticated event-driven architecture. Here's how this is achieved:

## Modal Window Non-Blocking Mechanism

### Event-Driven Callback System
`GuiWindow::ShowModal` does not actually block the calling thread. Instead, it:
- Immediately shows the modal window and returns control to the message loop
- Uses the `WindowReadyToClose` event to execute the completion callback asynchronously
- The calling code appears "blocked" because execution continues only when the callback is invoked

### Owner Window Disabling Pattern
The modal behavior is achieved through selective window disabling:
- `owner->SetEnabled(false)` disables user interaction with the owner window
- The modal window remains fully interactive through `Show()`
- Parent-child relationship is established via `GetNativeWindow()->SetParent(owner->GetNativeWindow())`
- This creates visual modal behavior while maintaining event processing

### Message Loop Continuation
The underlying message loop (`INativeWindowService::RunOneCycle`) continues processing:
- User input events are still processed normally
- Timer events and async operations continue executing
- Only the disabled owner window rejects user interaction
- The modal window and all other enabled windows remain responsive

### Callback-Based Completion
When the modal window closes, the cleanup process is entirely asynchronous:
```cpp
container->handler = WindowReadyToClose.AttachLambda([=, this](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
{
    callback();  // Execute user callback
    GetNativeWindow()->SetParent(nullptr);
    owner->SetEnabled(true);  // Re-enable owner
    owner->SetFocused();
    // Cleanup modal records
});
```

### Async Operations Integration
`ShowModalAsync` demonstrates the true non-blocking nature:
- Returns an `IFuture` immediately
- The future is completed when the modal closes
- Enables modern async/await patterns in client code
- Proves that no actual thread blocking occurs

### Platform-Specific Event Processing
The modal system works seamlessly across all platforms because:
- **Windows Native**: Win32 message pump continues processing `WM_*` messages
- **Hosted Mode**: Virtual window manager continues processing events within the host window
- **Remote Mode**: Protocol event processing continues normally

### Memory Management During Modal Operations
The system handles object lifetime carefully during modal operations:
- Uses `GetDisposedFlag()` to detect if the window is destroyed during modal operation
- Employs `InvokeInMainThread` to safely detach event handlers
- Ensures proper cleanup even if the modal window is deleted unexpectedly

This architecture allows `ShowModal` to provide traditional blocking semantics to application code while maintaining the responsiveness and event-driven nature required for modern GUI applications.

## DRAFT

The title would be "Main window and modal window"

# DRAFT-LOCATION

This will be a new topic under the `Design Explanation` section of the GacUI project in `Index.md` of the knowledge base.

# DRAFT-TITLE

Main Window and Modal Window Management

# DRAFT-CONTENT

# Main Window and Modal Window Management

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
- Application exits when protocol connection terminates

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
- Returns `IFuture` for async/await pattern integration
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