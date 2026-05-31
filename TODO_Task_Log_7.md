# TODO Task Log 7

## Task

Verify automation through remote protocol using both:

- `RemotingTest_Core /Http /FCT`
- `RemotingTest_Rendering_Win32 /Http`

The requirement was to perform the Task 5 keyboard verification through each process and confirm both processes report the same UI changes.

## Setup

Both processes were launched under separate `cdb` sessions.

Core endpoint:

```text
http://localhost:8888/Automation/RemotingTest_Core
```

Renderer endpoint:

```text
http://localhost:8888/Automation/RemotingTest_Rendering_Win32
```

Core dump:

```text
GET /Controls
```

Renderer dump:

```text
GET /Dom
```

Core IO:

```text
POST /Automation/RemotingTest_Core/IO
```

Renderer IO:

```text
POST /Automation/RemotingTest_Rendering_Win32/IO
```

The renderer does not expose `/Controls`; it exposes `/Dom`.

## Important Final Protocol Behavior

`/IO` now returns:

```text
Queued
```

This means:

- the HTTP endpoint accepted the command
- the command has been queued to the UI thread
- the UI operation may not be finished yet

During verification I waited for state changes and did not treat the `/IO` response as operation completion.

## Source Locations I Investigated

- `Source/PlatformProviders/Windows/WinNativeWindow.cpp`
  - HTTP automation dispatch.
  - Changed `/IO` to use `InvokeInMainThread`.
  - Fixed `/IO/<windowId>` prefix matching.
- `Source/Utilities/SharedServices/GuiSharedAutomationService.cpp`
  - Shared IO command execution.
  - Target native window activation.
- `Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.cpp`
  - Remote protocol automation service.
- `Source/PlatformProviders/Hosted/GuiHostedController.cpp`
  - Hosted keyboard routing.
  - Fixed pure keyboard startup by activating the main hosted window if key input arrives before any hosted child is active.
- `Source/PlatformProviders/Remote`
  - Used to understand core/renderer event forwarding and DOM reporting.

## Problems Found While Testing

### 1. Core and renderer pure keyboard needed a mouse primer

Before the hosted keyboard fix, a fresh remote launch did not respond to:

```text
!KeyPress:Alt
!KeyPress:L
!KeyPress:3
```

until a mouse click activated a hosted child window.

The cause was that `GuiHostedController::HandleKeyboardCallback` only forwarded key input when `wmManager->activeWindow` was already set. A fresh hosted remote window could receive keyboard input before any hosted child was active.

The fix:

```text
if no hosted window is active and mainWindow exists:
    activate mainWindow before forwarding keyboard input
```

After this change, fresh core `/IO` and fresh renderer `/IO` both showed ALT labels without any mouse input.

### 2. `/IO` returning synchronously blocked on modal operations

The user requested that `/IO` queue work via `INativeAsyncService::InvokeInMainThread`.

The final HTTP behavior is:

```text
POST /IO -> Queued
```

This avoids blocking the HTTP handler on automation operations. It also means verification must poll or wait for actual UI state instead of assuming completion.

### 3. Dumping core `/Controls` while the color dialog was open can time out

During the full remote-core Task 5 workflow, opening the modal color dialog and then immediately dumping core `/Controls` could time out.

The final verification avoided this incorrect assumption:

- send keyboard commands through the modal dialog
- wait for the dialog to close
- then dump `/Controls`

This matches the final queued IO contract.

## Initial Remote Observations

Core `/Controls` reported:

```text
WindowManagement = HostedRemoteProtocol
MainWindow.title = Complete Control Showcase
```

Renderer `/Dom` returned a large DOM JSON tree:

```text
RendererDomChars approximately 1.48M to 1.59M depending on state
```

After pressing ALT through core `/IO`, core `/Controls` reported ALT labels such as:

```text
[L]1
[L]2
[L]3
[M]
[W]
[X]0
[L]0
[L]4
```

After:

```text
!KeyPress:L
!KeyPress:3
```

core `/Controls` reported:

```text
Document element count = 8
```

After resetting and sending the same commands through renderer `/IO`, core `/Controls` again reported:

```text
Document element count = 8
```

This verified pure keyboard navigation through both processes.

## Full Task 5 Through Core IO

The full command flow through `RemotingTest_Core /IO` was:

```text
!KeyPress:Alt
!KeyPress:L
!KeyPress:3
!KeyPress:Alt
!KeyPress:D
!KeyPress:2
!Type:ABCDEFG
!KeyPress:Left
!KeyPress:Left
!KeyDown:Shift
!KeyPress:Left
!KeyPress:Left
!KeyPress:Left
!KeyUp:Shift
!KeyPress:Alt
!KeyPress:T
!KeyPress:1
!KeyPress:C
!KeyPress:Alt
!KeyPress:R
!KeyPress:Ctrl+A
!Type:0
!KeyPress:Alt
!KeyPress:G
!KeyPress:Ctrl+A
!Type:255
!KeyPress:Alt
!KeyPress:B
!KeyPress:Ctrl+A
!Type:0
!KeyPress:Alt
!KeyPress:O
!KeyPress:Alt
!KeyPress:D
!KeyPress:2
!KeyPress:End
```

Final core `/Controls` document XML contained:

```xml
<nop>AB</nop><font color="#00FF00"><nop>CDE</nop></font><nop>FG</nop>
```

Renderer `/Dom` also contained:

```text
ABCDEFG
00FF00
```

Observed renderer DOM size:

```text
RendererDomChars = 1594835
```

## Full Task 5 Through Renderer IO

For renderer `/IO`, I used the same command flow with longer waits and explicit synchronization before opening the modal color dialog, because renderer IO goes through the renderer process and then remote protocol events to the core.

I verified after typing:

```xml
<nop>ABCDEFG</nop>
```

Then I completed the same selection and color-dialog workflow.

Final core `/Controls` document XML contained:

```xml
<nop>AB</nop><font color="#00FF00"><nop>CDE</nop></font><nop>FG</nop>
```

Renderer `/Dom` also contained:

```text
ABCDEFG
00FF00
```

Observed renderer DOM size:

```text
RendererDomChars = 1594835
```

## Exit Handling

For remote protocol, I sent:

```text
POST /Automation/RemotingTest_Rendering_Win32/IO
!Exit
```

This follows the requirement that remote protocol exit should be sent to the renderer.

When necessary after verification, I killed leftover `cdb`, `RemotingTest_Core`, and `RemotingTest_Rendering_Win32` processes to avoid leaving debugger sessions behind.

## What Changed For This Task

- HTTP `/IO` now queues with `InvokeInMainThread`.
- `/IO/<windowId>` path matching was fixed.
- Shared IO receives the target native window and activates it before input.
- Hosted keyboard routing activates the main hosted window on first keyboard input if no hosted child is active.
- Remote verification now respects that `/IO` returning `Queued` is not a completion signal.

## Verification Result

Task 7 is verified. Both core `/IO` and renderer `/IO` can perform the Task 5 keyboard editing workflow, and both core `/Controls` and renderer `/Dom` report the final green `CDE` document edit.
