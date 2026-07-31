# AutomationService

AutomationService is the GacUI surface for programmatic inspection and input. It is exposed by `INativeAutomationService` through `GetCurrentController()->AutomationService()`. Coding agents use it to read the current UI structure and send input commands without depending on operating-system UI Automation.

The service is independent from the remote protocol. A normal Windows application, a remote protocol core application, and a remote protocol renderer can all expose an automation service, but each setup exposes the view of the UI that exists on that side.

## Service Interface

`INativeAutomationService` has one service-level availability flag and three feature groups:
- `Available`: returns false when no automation service exists for the current controller.
- `Stop`: turns off all features. Endpoint lifetime is owned separately.
- `CanDumpControlTree` and `DumpControlTree`: expose visible GacUI windows, popups, controls and compositions.
- `CanDumpDomTree` and `DumpDomTree`: expose the remote protocol renderer DOM.
- `CanRunIOCommands` and `RunIOCommand`: send a textual IO command to the main window or to a selected native window.

Feature availability is checked separately. A real service returns true from `Available` even if a specific feature is unsupported. Unsupported dump features should return false from the corresponding `Can...` function and return an empty string from the operation.

`CanRunIOCommands` returns `INativeAutomationService::IOCommandAvailability`:
- `Disabled`: `RunIOCommand` returns an empty string and endpoint layers should treat IO as unsupported.
- `Enabled`: `RunIOCommand` accepts every syntactically valid command and queues accepted work.
- `ExitOnly`: only the exact command `!Exit` is forwarded and keeps the normal `Queued` result. Every other command returns exactly `!Application stopped responding.` synchronously without queuing work.

## Windows HTTP Layer

`StartWindowsHttpAutomationService` creates a localhost HTTP wrapper around the current `INativeAutomationService`. The test-support implementation and its lifecycle scopes live in `Source/RemotingHelpers/AutomationService/Windows`, outside the ordinary `GacUI.Windows` library pair. Applications compile or consume the remoting-helper pair explicitly.

The function takes `applicationName` as a URL path fragment and `port` as the localhost port. Given `applicationName == L"Automation/MyApp"` and `port == 8888`, the listener prefix is `http://localhost:8888/Automation/MyApp/`. The service offers exactly these HTTP URLs:
- `GET http://localhost:8888/Automation/MyApp/Controls`: calls `DumpControlTree` on the UI thread when `CanDumpControlTree` is true.
- `GET http://localhost:8888/Automation/MyApp/Dom`: calls `DumpDomTree` on the UI thread when `CanDumpDomTree` is true.
- `POST http://localhost:8888/Automation/MyApp/IO`: passes the UTF-8 request body to `RunIOCommand` with no window id.
- `POST http://localhost:8888/Automation/MyApp/IO/<windowId>`: passes the UTF-8 request body to `RunIOCommand` with the window id path segment.

The window id is a path segment after `IO`, not a query parameter. All other methods, paths, and malformed `IO` suffixes return HTTP 404 with a text response. The HTTP wrapper returns 404 only when `CanRunIOCommands` returns `Disabled`; both `Enabled` and `ExitOnly` call `RunIOCommand`. Successful IO command parsing returns `Queued`; this means the command was accepted and posted to the UI thread, not that every visible effect has already finished. Syntax errors, command errors, and `ExitOnly` rejections are returned synchronously as text beginning with `!`.

## Starting The Service

Create a `WindowsAutomationServiceScope` from `GuiMain`, after the setup function has installed the current native controller and before entering the application event loop. The scope substitutes the selected service, starts the chosen HTTP endpoint, and reverses those operations in dependency order.

A normal Windows application can start the service before `GetApplication()->Run`:
```c++
#include "../../../Source/RemotingHelpers/AutomationService/Windows/WindowsAutomationService.Windows.h"

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;

void GuiMain()
{
    demo::MainWindow window;
    window.ForceCalculateSizeImmediately();
    window.MoveToScreenCenter();

    windows::WindowsAutomationServiceScope automation(
        windows::WindowsAutomationServiceType::Hosted,
        remoting::RemotingAutomationService::WindowsHttp,
        WString::Unmanaged(L"MyApp"),
        8888);
    GetApplication()->Run(&window);
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return SetupHostedWindowsDirect2DRenderer();
}
```

Only one substituted Windows automation service and one endpoint may be active in a process. Duplicate starts fail immediately.

## Setup Function Cases

The setup function decides which controller is active while `GuiMain` runs; it no longer installs test automation implicitly. Select the matching helper service explicitly:
- `SetupWindowsGDIRenderer` and `SetupWindowsDirect2DRenderer`: use `WindowsAutomationServiceType::Normal` for multi-window control tree and IO. The DOM route is normally unavailable.
- `SetupHostedWindowsGDIRenderer` and `SetupHostedWindowsDirect2DRenderer`: use `WindowsAutomationServiceType::Hosted`. IO should use `/IO` without a window id because hosted sub windows and popups are represented under the main window.
- `SetupRawWindowsGDIRenderer` and `SetupRawWindowsDirect2DRenderer`: use `WindowsAutomationServiceType::Normal` for an ordinary raw native controller. A remote protocol renderer that owns a `GuiRemoteRendererSingle` uses `WindowsAutomationServiceType::Renderer` and passes the renderer pointer to expose renderer DOM and renderer-side IO.
- `SetupRemoteNativeController`: the remote controller itself does not return an automation service. Substitute `RemoteProtocolAutomationService` around `GetApplication()->Run` when the remote core should expose the hosted control tree and core-side IO.
- `SetupGacGenNativeController`: this setup is for generation-time resource processing, not an interactive UI session. It does not provide an automation service for HTTP control.
- `SetupGtkRenderer`: do not call the Windows HTTP helper. A Gtk port must provide its own endpoint layer and automation service implementation if it needs coding-agent automation.
- `SetupWGacRenderer`: do not call the Windows HTTP helper. A Wayland port must provide its own endpoint layer and automation service implementation if it needs coding-agent automation.
- `SetupWGacHostedRenderer`: same requirement as `SetupWGacRenderer`, but the service should follow hosted-mode window-id behavior if it exposes hosted windows.

## Substituting a Service

Use `GetNativeServiceSubstitution()->Substitute(service, false)` before the automation service is first requested. The substitution layer rejects a late substitution after a service has already been used. Keep the substituted object alive until it is unsubstituted.

A remote protocol core substitutes its neutral service and owns the endpoint in a nested scope:
```c++
void GuiMain()
{
    RemoteProtocolAutomationService automationService;
    remoting::NativeAutomationServiceScope substitution(&automationService);

    {
        windows::AutomationServiceEndpointScope endpoint(
            remoting::RemotingAutomationService::WindowsHttp,
            WString::Unmanaged(L"RemoteCore"),
            8888);
        GetApplication()->Run(mainWindow);
    }
}
```

A remote protocol renderer uses `WindowsAutomationServiceScope` with `WindowsAutomationServiceType::Renderer`. This is the case where `GET /Dom` becomes meaningful.

When a remote renderer is retained after a fatal remote-protocol error, renderer automation keeps `GET /Dom` available. The DOM response is still an HTTP success containing the frozen renderer DOM, and it adds a lowercase top-level `fatalError` string with the original error. Renderer IO switches to `ExitOnly`: ordinary IO returns exactly `!Application stopped responding.`, while exact `!Exit` is still accepted so automation can close the retained renderer.

## Implementing Another Platform

A platform implementation should implement `INativeAutomationService` or reuse `AutomationServiceBase` from `Utilities/SharedServices`. Return true from `Available`, implement the supported `Can...` and operation pairs, and make `Stop` disable all future operations.

When implementing IO:
- Parse the command synchronously on the request path so malformed commands return an immediate error.
- Queue the parsed command to the UI thread instead of running arbitrary UI work on the HTTP or transport thread.
- Return `Queued` after the parsed command is accepted.
- Use `RunIOCommandOnNativeWindow` when the platform can map a window id to an `INativeWindow` and its listener list.

The Windows HTTP wrapper is only one endpoint layer. Other platforms may expose the same URL contract, a stdio protocol, an in-process function-call bridge, or another transport, as long as the transport calls the same `INativeAutomationService` operations on the correct threads.

## Operational Notes

Application-level automation depends on the GacUI UI thread. A native crash dialog, file dialog, or other modal native window can block the UI thread and keep the HTTP endpoint from answering. In that situation, inspect and operate native windows from another process using Win32 APIs, then return to the automation endpoint after the modal window is closed.

Do not use operating-system UI Automation as the fallback for GacUI automation on Windows. It can fail when the screen is locked, and it is not the contract implemented by `AutomationService`.

