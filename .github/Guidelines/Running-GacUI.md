# Running a GacUI Application Project

- Go to `Windows Specific` section if you are on Windows.
- Go to `Linux Specific` section if you are on Linux or macOS.

## Windows Specific

You are strongly recommended to attach a debugger when running any GacUI application.
Because some runtime exceptions are silently consumed by Windows causing the application not to crash, covering issues if no debugger is attached.

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

## Linux Specific

NOT SUPPORTED
