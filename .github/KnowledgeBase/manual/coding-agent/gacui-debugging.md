# AutomationService and Basic Computer Use

GacUI applications need an application-level automation path so a coding agent can inspect and operate the UI while debugging. On Windows, the recommended path is to start the GacUI AutomationService HTTP endpoint from the application during startup. This lets the agent read the control tree or remote-renderer DOM and send IO commands without using operating-system UI Automation.

From the agent user's perspective, this means:
- The application should expose a stable localhost automation URL while it is running.
- The agent should use the automation endpoint for normal GacUI windows and controls.
- The agent should not rely on UI Automation as a fallback, because it can fail when the screen is locked.
- The user can keep using the computer because the agent does not need to drive the physical pointer for normal GacUI operations.

See [AutomationService](.././gacui/coding-agent/automation-service.md) for the service setup, exact HTTP URLs, setup-function cases, and platform implementation details.

## When the Agent Gets Blocked

Application-level automation depends on the application UI thread. A native crash dialog, message box, file dialog, color dialog, or another modal native window can block that thread. When this happens, the agent should stop polling the application automation endpoint and inspect native windows from another process.

The copied `.github/Guidelines/Running-ComputerUse.md` file explains the unblocking workflow:
- Identify the target process.
- Enumerate top-level and child windows with Win32 APIs.
- Read class names, window text, control ids, and rectangles.
- Prefer direct control messages such as `BM_CLICK`, `WM_SETTEXT`, and combo-box messages.
- Capture a screenshot only when visual confirmation is needed.
- Close or answer the native dialog before returning to AutomationService.

## Project Preparation

If the application contains GacUI UI work, mention its automation endpoint in `Project.md`. Include the executable project name, port, URL prefix, whether hosted mode is used, and any native dialogs that commonly appear. This gives the agent enough context to choose AutomationService first and native-window handling only when the application is blocked.

