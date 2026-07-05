# Running a GacUI Application Project

- Go to `Windows Specific` section if you are on Windows.
- Go to `Linux Specific` section if you are on Linux.
- Go to `macOS Specific` section if you are on macOS.

When the model `gpt-5.3-codex-spark` is available:
- If multiple steps need to be done to operate the GUI:
  - Summarize what to look for and what to do.
  - Start a sub agent with this model to perform all GUI operations.
  - If the target application has bugs, it is normal that steps can't be performed to the end. The sub agent should summarize what is going wrong to prevent steps to be done.
  - This model is fast, it significantly improves the performance of operating the GUI.

## Windows Specific

- You are strongly recommended to attach a debugger when running any GacUI application.
- Because some runtime exceptions are silently consumed by Windows causing the application not to crash, covering issues if no debugger is attached.
- GacUI applications could end up in dead loop or dead locks, so DO NOT JUST wait for the process to exit.
  - When it is crashed, sometimes (but not always) a native dialog would show and block the process.
  - Native dialogs could be proactivately called from a GacUI application, even when `FakeDialogService` is not used.
  - If you believe the processing is blocked or is running too long, you are going to check out `Running-ComputerUse.md` and deal with it.

### Automation Service via HTTP

This is a very useful way for coding agent to debug GacUI applications.
Computer use via UI Automation may not work when the computer screen is locked.

When `StartWindowsHttpAutomationService` is used during startup up a GacUI application:
- It listens to `http://localhost:<port>/Automation/<applicationName>/...`.
- GET `.../Controls`, for GacUI applications, exposing all visible windows and popups.
  - Read comment for `DumpWindowClientArea` for the schema.
- GET `.../Dom`, for remote protocol renderer, exposing the DOM tree.
  - Read comment for DumpRemoteProtocolRenderingDom` for the schema.
- POST `.../IO` or `IO/<WINDOW-ID>`
  - IRead comment for `RunIOCommandOnNativeWindow` for the schema.
  - `<WINDOW-ID>` is the window id returning from `.../Controls`.
  - The window ID can be comitted for the main window.
  - The window ID must be omitted for GacUI applications with hosted mode or remote protocol core side.
    - In this case all sub windows or popups behaves like controls in the main window.

When remote protocol is in use:
- Core side exposes UI in window-control tree concept.
- Renderer side exposes UI in DOM tree concept.
- Both supports IO operations:
  - When performing IO via renderer, remote protocol events are used to pass IO operations to core.
  - When performing IO via core, renderer only receives UI updates and redraw.
  - Core and renderer should sync in the same UI state afterwards.
  - Performaning IO via no matter renderer or core should result in the same UI state.
- If GacJS connects to the core side, automation service only works on core.

### UI Automation

GacUI does not support UI Automation so far, but this situation will be changed very soon.

UI Automation does not work when the screen is locked. Calling any UIA tools in this case will just fail.

## Linux Specific

(to be editing...)

## macOS Specific

(to be editing...)
