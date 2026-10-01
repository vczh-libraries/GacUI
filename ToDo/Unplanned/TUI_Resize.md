# TUI Window Resizing and State Control

Research and implementation guidance, checked on 2026-09-05. This document describes possible future functionality; it does not add window-control APIs to VlppOS.

## What having a GUI environment enables

Resizing, maximizing, minimizing and restoring are possible when we control the GUI window displaying the TUI, or when its terminal host exposes a suitable control interface. Merely running under Windows, a Wayland desktop, or macOS does not give a terminal application ownership of that window.

There are three different objects to consider:

1. **The TUI cell buffer:** application content measured in columns and rows.
2. **The terminal session:** console or PTY state describing the current cell grid.
3. **The GUI window:** a native window owned by the terminal emulator or by our own GUI frontend. It may contain multiple tabs and panes.

Changing the session's reported grid size does not necessarily resize the GUI window. Similarly, changing a GUI window can affect several terminal sessions at once. A window-control implementation needs an explicit association between the current session and the intended host window.

The existing implementation exposes `vl::console::TUI::TryGetConsoleSize` and `vl::console::ITuiCallback::BufferSizeChanged` in [TUI.h](Source/TUI/TUI.h). It observes terminal dimensions and reallocates the cell buffer. The Windows backend's console-geometry management is part of terminal takeover/restoration, not an existing public API for controlling the surrounding GUI window. See the [TUI specification](.github/KnowledgeBase/KB_VlppOS_TerminalUserInterface.md).

## Practical support by environment

| Actual environment | Resize / maximize / minimize / restore | Enforce our own minimum during user resizing |
| --- | --- | --- |
| Windows local console with a real console HWND | Native window operations are possible, subject to the host's rules. | No ordinary console API installs an application-specific minimum on the host's window. |
| Windows Terminal or another ConPTY frontend | Requires a supported host interface or an explicitly supplied real GUI HWND; the pseudoconsole handle is insufficient. | Requires cooperation from the frontend. |
| Existing terminal on Wayland | Requires terminal cooperation or an explicitly supported compositor integration. | The child TUI cannot install constraints on another client's surface. |
| Existing terminal on macOS | Terminal-specific scripting or accessibility automation may expose some operations. | Requires terminal support; manipulating the current frame does not install a persistent minimum. |
| GUI frontend that we own | Win32 and AppKit provide direct mechanisms. Wayland provides requests governed by compositor policy. | Win32/AppKit support user-resize constraints. Wayland constraints are advisory. |

The platform mechanisms and limits behind this table are detailed below. An ordinary terminal backend should report unavailable host control instead of assuming that a desktop session makes every operation available.

## Windows 10 and newer

### A local console window versus a pseudoconsole

`GetConsoleWindow()` can identify a traditional local console window. Inside a pseudoconsole, however, Microsoft documents the returned HWND as serving message-queue purposes; it is not the visible terminal frontend. Therefore, finding a non-null HWND is insufficient to claim control of Windows Terminal. Do not locate a target by assuming that the foreground window or the parent process is the terminal window. [Microsoft: GetConsoleWindow](https://learn.microsoft.com/en-us/windows/console/getconsolewindow)

For a known, controllable GUI HWND:

| Operation | Native mechanism |
| --- | --- |
| Resize the outer window | `SetWindowPos`, normally with `SWP_NOMOVE \| SWP_NOZORDER \| SWP_NOACTIVATE`. |
| Maximize | `ShowWindow(hwnd, SW_MAXIMIZE)`. |
| Minimize | `ShowWindow(hwnd, SW_MINIMIZE)`. |
| Restore | `ShowWindow(hwnd, SW_RESTORE)`; preserve window placement when exact restoration is required. |

`SetWindowPos` reports native call failure through its return value and `GetLastError`. `ShowWindow` is different: its return value describes **previous visibility**, not success. Verify the resulting state instead of treating a zero return as failure. `GetWindowPlacement` exposes the show state and normal placement for observation/restoration. [SetWindowPos](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos), [ShowWindow](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow), [GetWindowPlacement](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowplacement)

`SetConsoleWindowInfo` changes the console viewport in character coordinates within its screen buffer, with buffer and display restrictions. It is not a general terminal-frontend window API. `ResizePseudoConsole` changes the pseudoconsole's cell dimensions and is called by the host holding its HPCON; a successful call does not prove that a GUI window changed size. [SetConsoleWindowInfo](https://learn.microsoft.com/en-us/windows/console/setconsolewindowinfo), [ResizePseudoConsole](https://learn.microsoft.com/en-us/windows/console/resizepseudoconsole)

### Minimum size

For a window we own, handle `WM_GETMINMAXINFO` in its window procedure and supply `MINMAXINFO::ptMinTrackSize`. This constrains interactive border resizing. Also validate our own programmatic resize requests; a tracking constraint should not be advertised as universal protection against every placement API. [WM_GETMINMAXINFO](https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-getminmaxinfo)

Convert the desired minimum cell grid into a minimum content size, then account for window decorations and current DPI. `AdjustWindowRectExForDpi` supports that conversion for applicable Windows versions; it requires Windows 10 version 1607 or later, so targeting the original Windows 10 releases would require an appropriate older-API path. [AdjustWindowRectExForDpi](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-adjustwindowrectexfordpi)

A normal console application does not own conhost's window procedure. Installing our procedure into another process is not an ordinary subclassing solution: Microsoft's documentation says applications should not subclass a window class created by another process. Therefore, use an owned frontend or a documented host extension for custom minimum-size enforcement. [SetWindowLongPtr](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowlongptra)

## Wayland

The terminal emulator owns its Wayland surfaces. A separate TUI process cannot obtain authority over them merely by opening its own Wayland connection. The core object model explicitly prevents clients from accessing other clients' surfaces. A terminal IPC interface or compositor-specific extension would be a separate, optional integration. [Wayland protocol and object model](https://wayland.freedesktop.org/docs/book/Protocol.html)

For an owned frontend using stable xdg-shell:

| Operation | Mechanism |
| --- | --- |
| Programmatic size change | Commit appropriately sized content/window geometry when the configured state permits; no universal force-size request. |
| Begin a user drag | `xdg_toplevel.resize`, requiring a valid input-event serial. |
| Maximize / unmaximize | `set_maximized` / `unset_maximized`; observe configure state. |
| Minimize | `set_minimized`; no standard minimized-state notification or `unset_minimized`. |
| Minimum size | `set_min_size(width, height)`, followed by a surface commit. |

Sizes use window-geometry coordinates. Minimum constraints can be ignored. Maximize requests may produce a different configured state. Acknowledge configuration before committing the corresponding content. Where negotiated xdg-shell version 5 or newer is available, `wm_capabilities` advertises supported maximize/minimize operations; older versions lack that advertisement. Neither sending a request nor completing a display round trip proves that it took effect. [XDG shell protocol](https://wayland.app/protocols/xdg-shell)

Restoring a minimized surface may involve compositor UI or an optional activation integration. XDG activation uses tokens, and the compositor can decline activation. It must not be presented as a guaranteed unminimize operation. [XDG activation protocol](https://wayland.app/protocols/xdg-activation-v1)

## Cocoa / AppKit on macOS

### A window owned by our frontend

Use AppKit on the GUI thread, with access to the actual `NSWindow`:

| Operation | Mechanism |
| --- | --- |
| Resize content | [NSWindow.setContentSize(_:)](https://developer.apple.com/documentation/appkit/nswindow/setcontentsize(_:)), in window coordinate units, normally points. |
| Resize or restore an explicit frame | `NSWindow.setFrame(_:display:)`, using a saved frame when restoring. |
| Native zoom / unzoom | [NSWindow.zoom(_:)](https://developer.apple.com/documentation/appkit/nswindow/zoom(_:)); this toggles between standard and user sizes. |
| Minimize | [NSWindow.miniaturize(_:)](https://developer.apple.com/documentation/appkit/nswindow/miniaturize(_:)). |
| Restore from minimization | [NSWindow.deminiaturize(_:)](https://developer.apple.com/documentation/appkit/nswindow/deminiaturize(_:)). |
| Minimum content size | [NSWindow.contentMinSize](https://developer.apple.com/documentation/appkit/nswindow/contentminsize). |

AppKit zoom is a best-fit operation and is not identical to Windows maximization or entering fullscreen. Define the intended cross-platform behavior explicitly. For a "fill the available work area" operation, an owned frontend can save the previous frame and apply the screen's usable frame; keep this policy distinct from native zoom. Calling a toggle twice is not an idempotent maximize request. [AppKit zoom semantics](https://developer.apple.com/documentation/appkit/nswindow/zoom(_:))

`contentMinSize` constrains user resizing and `setContentSize(_:)`. Apple explicitly excludes `setFrame(_:display:)` and its animated variant from this enforcement. Clamp frame-based requests ourselves, including restoration after font, scale, or minimum-size changes. [AppKit minimum-size enforcement](https://developer.apple.com/documentation/appkit/nswindow/contentminsize)

These setters do not provide a uniform Boolean success result. Observe the resulting window frame/state, accounting for AppKit's event processing and transitions, before reporting a confirmed result.

### A TUI inside Terminal or another terminal application

The TUI does not own the terminal application's `NSWindow`. Calling AppKit in the TUI process does not turn a window in another process into a locally controlled object.

Possible adapters include the terminal's scripting interface or macOS Accessibility APIs, depending on what the target exposes and what access the user has granted. Accessibility attribute writes can report unsupported attributes or communication failures; availability must be checked for the actual target. These are host integrations, not a portable property of a macOS TTY. They also do not imply the ability to set the terminal's persistent minimum-size constraint. [Apple: AXUIElementSetAttributeValue](https://developer.apple.com/documentation/applicationservices/1460434-axuielementsetattributevalue)

## Terminal escape sequences and PTY size changes

Some terminals implement xterm's optional window operations. In the following examples, `ESC` means byte 0x1B:

| Request | Sequence |
| --- | --- |
| Request 120 columns by 40 rows | `ESC [ 8 ; 40 ; 120 t` |
| Maximize / restore from maximization | `ESC [ 9 ; 1 t` / `ESC [ 9 ; 0 t` |
| Minimize / deiconify | `ESC [ 2 t` / `ESC [ 1 t` |

Spaces above separate notation tokens; omit them in the emitted bytes. These extensions can be disabled, are not supported by every terminal, and provide no universal persistent minimum-size operation or per-request success acknowledgment. A successful output write proves only that bytes were written. A size/state query may help observe the result when that query is supported. [Xterm control sequences: XTWINOPS](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html)

On Linux, `ioctl(TIOCSWINSZ)` updates the terminal's stored dimensions and generates `SIGWINCH` for its foreground process group when they change. It does not command a Wayland window to resize. Use it in a cooperating terminal host after calculating the actual grid, rather than treating it as GUI control. [Linux TIOCSWINSZ documentation](https://man7.org/linux/man-pages/man2/TIOCSWINSZ.2const.html)

## Reporting unsupported operations and failures

**Yes: an API can explicitly report unsupported operations. It cannot always distinguish an ignored request from an unsupported environment using only generic terminal I/O.**

The following is a proposed contract for future VlppOS work, not an existing API:

| Result | Meaning |
| --- | --- |
| `Unsupported` | The selected backend has no usable mechanism for this operation, or a negotiated capability explicitly excludes it. |
| `InvalidArgument` | Invalid dimensions or incompatible constraints; reject before issuing a native request. |
| `PermissionDenied` | The host or platform explicitly denies the required access. |
| `Failed` | A native call or host response explicitly reports failure; retain its error details. |
| `Requested` | A request was dispatched, but the requested outcome has not been verified. |
| `Applied` | The requested postcondition was observed or authoritatively confirmed. |
| `TimedOutUnknown` | The observation deadline elapsed without confirmation; this is not proof of unsupported functionality. |

Report capability separately for each operation, including whether a minimum is **advisory** or **enforced during user resizing**. Missing capability information means unknown, not supported. An implementation with a strict "confirmed changes only" policy can decline a request whose result cannot be observed, explaining that limitation before issuing it.

Suggested processing:

1. Resolve the intended window and a usable adapter. Environment variables such as `TERM`, `WAYLAND_DISPLAY`, or terminal branding are hints, not authorization or proof of supported operations.
2. Validate the request and consult current capabilities. Return `Unsupported` immediately when no control path exists.
3. Issue the request through the appropriate owner-thread/event-loop mechanism. Preserve native failures without translating every failure into `Unsupported`.
4. Observe the relevant postcondition. For grid resizing, this is the actual column/row count; for maximizing or minimizing, a window-state observation is required. Unchanged dimensions alone do not establish failure or success.
5. Deliver the outcome asynchronously where necessary. Keep `Requested` distinct from `Applied`, and do not block the TUI event loop waiting for events that it must itself dispatch.

Treat restoring from minimization and returning from maximization to a normal frame as distinct operations. A minimized window can have a previously maximized state, and restoring visibility need not produce a normal-sized window. Specify which postcondition the caller requested.

## Recommended direction for this repository

Keep normal TUI rendering and resize observation usable without window-control support. Add optional host adapters only where the window association, capabilities, and result semantics are known. For consistent Win32/AppKit minimum constraints, use a frontend we own or a cooperating terminal host. For Wayland, expose the advisory nature of constraints and retain layout behavior for sizes below the requested minimum.

Express the public requested grid in columns and rows. The GUI host can calculate:

```text
minimum content width  = minimum columns * current cell width  + horizontal padding
minimum content height = minimum rows    * current cell height + vertical padding
```

Use the host's coordinate units and actual font metrics. Account for decorations, tabs, panes, and DPI/scale changes separately. The current terminal grid is the rendering limit even if a requested minimum is larger; render a compact layout or a clipped "window too small" message instead of assuming the request succeeded.

A future implementation should verify these cases on each supported host: successful operations, already-satisfied requests, unsupported hosts, permission rejection, ignored requests/timeouts, resizing across font/DPI changes, attempted dragging below the minimum, restoring from minimized/maximized states, and operating in a tab or split pane. This research did not execute those native window-control operations.
