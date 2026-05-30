# Running a GacUI Application Project

- Go to `Windows Specific` section if you are on Windows.
- Go to `Linux Specific` section if you are on Linux or macOS.

## Windows Specific

You are strongly recommended to attach a debugger when running any GacUI application.
Because some runtime exceptions are silently consumed by Windows causing the application not to crash, covering issues if no debugger is attached.

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

## Linux Specific

NOT SUPPORTED
