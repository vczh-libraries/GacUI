# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

You are going to add a `StartMiniHttpAutomationService(Ptr<IAsyncSocketServer>)` function, just like what `StartWindowsHttpAutomationService` is done, but it uses VlppOS's mini http server api, instead of using WinHttp.
- A dedicated cpp file in `Test/GacUISrc/RemotingTest_Core/Shared.cpp` file is created for this function, `GuiMain.cpp` in both `RemotingTest_(Core|Renderer_Win32)` will extern this function, no header file is involved.
- In `RemotingTest_Core`, when started with `/MiniHttp`, `StartMiniHttpAutomationService` is called using the `IAsyncSocketServer` used to start the remote protocol.
- `RemotingTest_Renderer_Win32` project's, since mini http server cannot share the same http port with different url prefix across processes:
  - Both `StartWindowsHttpAutomationService` and `StartMiniHttpAutomationService` will switch to use the 8889 port instead.
    - Update `Project.md` about this formation.
    - Update `../Tools/DebugGacUIRemoteProtocol.md` if this file is affected by the decision by:
      - The port changing from 8888 to 8889 in the native renderer.
      - Automation service not works when `/MiniHttp` is used.

## Verification

Firstly make sure `/MiniHttp` works with GacJS.
- You are going to follow `../Tools/Jobs/job.verifyRemoteProtocol.prompt.md` to perform verification but:
  - Only verify core with GacJS
  - Only verify `/MiniHttp`, ignore other network protocols.
This proves that `RemotingTest_Core /MiniHttp` is running as expected, reducing the complexity in the next step.

Secondly make sure `/MiniHttp` works with native renderer.
- Automation service in both core and renderer need to be accessed, making sure they actually respond.
  - If the automation service is not working, the verification instruction wont work.
- You are going to follow `../Tools/Jobs/job.verifyRemoteProtocol.prompt.md` to perform verification but:
  - Only verify core with native renderer
  - Only verify `/MiniHttp`, ignore other network protocols.
This proves that `RemotingTest_Renderer_Win32 /MiniHttp` is also running as expected, reducing the complexity in the next step.

# UPDATES

# TEST [CONFIRMED]

The behavior is verified at the executable boundary because the defect concerns two HTTP prefixes sharing one injected socket listener in the core process and a separate listener in the renderer process. No new unit-test source file is needed; the existing GacUI unit-test project provides regression coverage for the shared UI and automation implementations.

1. Build `Test/GacUISrc/GacUISrc.sln` in Debug x64 through `copilotBuild.ps1`, then run the existing `UnitTest` project through `copilotExecute.ps1`. The build and unit test must succeed without warnings, errors, assertion failures, crashes, or a final memory-leak report.
2. Before implementing a proposal, start `RemotingTest_Core /MiniHTTP /RPT`, connect GacJS, and prove the current defect by requesting `GET http://localhost:8888/Automation/RemotingTest_Core/Controls`. The remote UI should render through `/GacUIRemoteProtocolHttp`, while the automation URL is unavailable because no MiniHTTP automation prefix is registered.
3. After implementing a proposal, repeat `RemotingTest_Core /MiniHTTP /RPT` with GacJS. Require the core automation `Controls` and `IO` URLs on port 8888 to respond while the browser renderer remains connected. Perform the complete `/RPT` browser operations from `Tools/DebugGacUISop.md`, including visible state changes, renderer replacement, state continuity, and normal application exit. Start a fresh `RemotingTest_Core /MiniHTTP /FCT` session and perform the complete-control-showcase operations from the same SOP.
4. Start a fresh `RemotingTest_Core /MiniHTTP /RPT` with `RemotingTest_Rendering_Win32 /MiniHTTP`. Require both automation services to respond simultaneously: core `Controls` and `IO` on port 8888, renderer `Dom` and `IO` on port 8889. Send at least one state-changing IO command through each service and confirm both trees converge on the same UI state. Perform the complete `/RPT` native-renderer operations from `Tools/DebugGacUISop.md`, including renderer replacement, state continuity, and normal application exit. Start a fresh `RemotingTest_Core /MiniHTTP /FCT` plus native-renderer session and perform the complete-control-showcase operations from the same SOP.
5. Confirm that `/Http` and `/Pipe` still use the Windows HTTP automation layer, with the renderer endpoints moved to port 8889 as required, and that all started core, renderer, browser/server, and debugger processes are cleaned up after verification.

Success requires the baseline probe to reproduce the missing endpoint, the final build and unit tests to pass, both `/MiniHTTP` end-to-end scenarios to complete, both automation services to be directly accessed in the native-renderer scenario, and the documented port assignments to match the implementation.

The baseline was confirmed on 2026-07-20 with the unmodified Debug x64 executables. `copilotBuild.ps1` completed with 0 warnings and 0 errors, and `copilotExecute.ps1` reported 85/85 passed test files and 1690/1690 passed test cases with no memory-leak dump. `RemotingTest_Core /MiniHTTP /RPT` listened on port 8888, GacJS rendered the Remote Protocol Test, and the core logged `Renderer connected: 2`. While that renderer was connected, `GET http://localhost:8888/Automation/RemotingTest_Core/Controls` returned HTTP 404 `NotFound` with body `{}`, confirming that the shared MiniHTTP listener had no automation prefix.

# PROPOSALS

- No.1 REGISTER AUTOMATION PREFIXES WITH THE MINIHTTP DISPATCHER [CONFIRMED]

## No.1 REGISTER AUTOMATION PREFIXES WITH THE MINIHTTP DISPATCHER

Implement the automation endpoint as `MiniHttpAutomationService`, derived from VlppOS `SocketHttpServerApi`, in the required `Test/GacUISrc/RemotingTest_Core/Shared.cpp`. Its public startup entry point will keep the required one-argument signature and inspect the currently substituted automation service: a controls-only service selects `/Automation/RemotingTest_Core`, while a DOM-only service selects `/Automation/RemotingTest_Rendering_Win32`. Requiring exactly one capability preserves the existing endpoint names without coupling the shared source file to project macros, executable names, or port numbers.

The MiniHTTP handler will mirror the Windows automation layer: `GET /Controls` and `GET /Dom` invoke tree dumps synchronously on the UI thread, while `POST /IO` and `POST /IO/<window-id>` decode the UTF-8 command and call `RunIOCommand`. Successful responses use the remote protocol JSON content type, and unsupported handler paths, capabilities, or invalid bodies receive HTTP 404. The MiniHTTP dispatcher retains its built-in handling for preflight and unsupported wire methods. A matching `StopMiniHttpAutomationService` will explicitly stop and drain the API before the substituted automation service is destroyed; the derived destructor will also call the idempotent `Stop` method to satisfy the MiniHTTP callback-lifetime contract.

For Core `/MiniHTTP`, retain the exact `Ptr<IAsyncSocketServer>` used to construct `MiniHttpRemotingChannelServer` and pass that same object to the automation service. VlppOS will therefore register both `/GacUIRemoteProtocolHttp` and `/Automation/RemotingTest_Core` in one process-local dispatcher on port 8888. For the native renderer `/MiniHTTP`, retain the protocol client on port 8888 but create a distinct `IAsyncSocketServer` on port 8889 for `/Automation/RemotingTest_Rendering_Win32`, because listeners cannot be shared across processes. Move the renderer's Windows HTTP automation endpoint to port 8889 as well, making the renderer automation port independent of transport.

Compile the dedicated shared source into both remoting projects through their `.vcxproj` and `.filters` files, with no header or solution change. Update `GacUI/Project.md`, `Tools/DebugGacUIWithRemoteProtocol.md`, and the related MiniHTTP note in `Tools/DebugGacUIWithBrowser.md` so the documented topology and automation availability match the implementation.

### CODE CHANGE

- Add `Test/GacUISrc/RemotingTest_Core/Shared.cpp` and compile it into both remoting projects. Implement a capability-selected `SocketHttpServerApi` for core `Controls` and renderer `Dom` GET requests plus `IO` POST requests. Reject missing or empty UTF-8 IO bodies, run tree dumps on the UI thread, use the shared remote-protocol content type, and stop the singleton API before its substituted automation service is destroyed.
- In `RemotingTest_Core`, retain the port-8888 MiniHTTP socket server, pass that exact object to both the remote-protocol channel and `StartMiniHttpAutomationService`, and tear down the selected Windows or MiniHTTP automation endpoint plus native-service substitution on both normal and exceptional exits.
- In `RemotingTest_Rendering_Win32`, keep the remote-protocol client on port 8888, move Windows automation to port 8889, create a separate port-8889 MiniHTTP socket server for renderer automation, and make automation/substitution cleanup exception-safe.
- Add the shared compilation unit to both `.vcxproj` and `.filters` files without adding a header or changing the solution.
- Update `GacUI/Project.md`, `Tools/DebugGacUIWithRemoteProtocol.md`, and `Tools/DebugGacUIWithBrowser.md` to document core automation on 8888, renderer automation on 8889, and MiniHTTP automation availability and socket sharing.

### CONFIRMED

The proposal was confirmed on Windows in Debug x64. `copilotBuild.ps1` built the full `GacUISrc.sln` with 0 warnings and 0 errors. Both `RemotingTest_Core` and `RemotingTest_Rendering_Win32` compiled and linked `Shared.obj`, proving that the dedicated implementation is shared by the two projects without a header. After the empty-body and exception-safe-cleanup review changes, the full solution was rebuilt with the same 0-warning, 0-error result. A focused post-review run returned HTTP 404 for empty IO bodies on both services and HTTP 200 `Queued` for a valid renderer IO command.

With `RemotingTest_Core /MiniHTTP /RPT` and GacJS, the remote protocol and core automation prefix responded concurrently through port 8888. `GET /Automation/RemotingTest_Core/Controls` returned HTTP 200 with `WindowManagement` equal to `HostedRemoteProtocol`, while malformed IO returned the automation parser diagnostic rather than a missing-prefix response. The complete RPT scenario passed: the Click Me state changed, the three fixed DataGrid rows were added and cleared, the RIGHT NOW dialog opened and closed, a second browser page inherited state, a third page took over while the second renderer detached, and the application exited normally through the File menu. A fresh `/MiniHTTP /FCT` run also passed both ten-item lists, clear, Control/Ribbon search and rich-editor markers, List-to-Control round-trip state preservation, and cleanup.

With a fresh core `/MiniHTTP /RPT` and native renderer `/MiniHTTP`, the core process owned port 8888 and returned HTTP 200 from `Controls`, while the renderer process independently owned port 8889 and returned HTTP 200 from `Dom`. A core IO click and a renderer IO `Ctrl+Q` command each changed visible UI state. Renderer IO then switched to DataGrid, added all three fixed rows, and cleared them; fresh core and renderer trees agreed before and after the clear. The Document dialog opened and was dismissed. Force-stopping the first renderer left the core and port 8888 alive; a second renderer acquired port 8889 and restored the Click Me state. Starting a third renderer while the second remained alive detached the second, transferred port 8889 to the third, and preserved the same state. The application then exited normally through its File-menu confirmation.

A fresh native `/MiniHTTP /FCT` pair returned HTTP 200 from both automation services with the expected separate listener ownership. Renderer IO added two complete `0` through `9` sequences and cleared both; both the active renderer DOM and core control tree showed the same results. Renderer access-key IO entered `Native-MiniHTTP` in the Ribbon search box and `Native-Rich-Mini` in the rich editor. After a List-to-Control round trip, both marker elements remained active in the renderer DOM and both values remained in the core control tree.

Source inspection confirmed that `/Http` and `/Pipe` still select `StartWindowsHttpAutomationService`, with the renderer using the new port-8889 constant, while `/MiniHTTP` selects the VlppOS service. The project and Tools documentation now describe core automation on 8888, renderer automation on 8889, the exact shared socket in the core, and the renderer's separate socket. All retained core and renderer processes were stopped, ports 8888 and 8889 were released, and all GacJS browser pages were closed after verification.
