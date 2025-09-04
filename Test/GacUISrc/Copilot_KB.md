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

### Core Modal Architecture

Modal windows are implemented through window ownership chains and event management:

**Modal Record Structure:**
```cpp
struct ShowModalRecord {
    GuiWindow* origin;   // Root window that started modal chain
    GuiWindow* current;  // Currently active modal window
};
```

**Modal Window Setup:**
- Disables owner window with `SetEnabled(false)`
- Establishes parent-child relationship via `SetParent()`
- Attaches cleanup callback to `WindowReadyToClose` event
- Maintains modal chain through `ShowModalRecord`

### Modal Window Variants

**ShowModal(owner, callback):**
- Basic modal behavior with custom completion callback
- Owner window disabled until modal closes
- Focus management and window relationships established

**ShowModalAndDelete(owner, callback):**
- Combines modal behavior with automatic cleanup
- Calls `DeleteAfterProcessingAllEvents()` after modal completion
- Provides optional separate callback for deletion completion

**ShowModalAsync(owner):**
- Returns `IFuture` for async/await pattern integration
- Enables modern asynchronous programming patterns
- Modal completion signals the future's promise

### Hosted Mode Modal Considerations

In hosted mode, modal windows receive special handling in `GuiWindow::BeforeClosing()`:
- Main window close attempts are intercepted when modal windows exist
- Active modal windows are either hidden or focused instead of allowing main window closure
- Prevents premature application termination while modal dialogs are active

## Cross-Platform Consistency

Despite platform-specific implementations, all platforms maintain consistent behavior through:

**Unified Event Processing:**
- Timer callbacks through `CallbackService.InvokeGlobalTimer()`
- Async task execution via `asyncService.ExecuteAsyncTasks()`
- Consistent event delegation patterns

**Window Lifecycle Management:**
- Same modal window API across all platforms
- Consistent parent-child window relationships
- Unified focus and activation management

**Exit Condition Handling:**
- Platform-specific triggers mapped to common exit conditions
- Graceful application shutdown across all modes
- Modal window cleanup during application termination

The architecture successfully abstracts platform differences while providing rich windowing capabilities, enabling developers to create sophisticated GUI applications that work consistently across Windows native, hosted, and remote environments.