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

The key insight is that while the platform-specific `INativeWindowService` implementations handle low-level event processing differently, the high-level `GuiApplication` and modal window behaviors remain consistent across all platforms through careful abstraction and delegation patterns