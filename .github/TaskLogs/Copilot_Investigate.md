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

- No.1 REGISTER AUTOMATION PREFIXES WITH THE MINIHTTP DISPATCHER

## No.1 REGISTER AUTOMATION PREFIXES WITH THE MINIHTTP DISPATCHER

Implement the automation endpoint as `MiniHttpAutomationService`, derived from VlppOS `SocketHttpServerApi`, in the required `Test/GacUISrc/RemotingTest_Core/Shared.cpp`. Its public startup entry point will keep the required one-argument signature and inspect the currently substituted automation service: a controls-only service selects `/Automation/RemotingTest_Core`, while a DOM-only service selects `/Automation/RemotingTest_Rendering_Win32`. Requiring exactly one capability preserves the existing endpoint names without coupling the shared source file to project macros, executable names, or port numbers.

The MiniHTTP handler will mirror the Windows automation layer: `GET /Controls` and `GET /Dom` invoke tree dumps synchronously on the UI thread, while `POST /IO` and `POST /IO/<window-id>` decode the UTF-8 command and call `RunIOCommand`. Successful responses use the remote protocol JSON content type, and unsupported methods, paths, capabilities, or invalid bodies receive HTTP 404. A matching `StopMiniHttpAutomationService` will explicitly stop and drain the API before the substituted automation service is destroyed; the derived destructor will also call the idempotent `Stop` method to satisfy the MiniHTTP callback-lifetime contract.

For Core `/MiniHTTP`, retain the exact `Ptr<IAsyncSocketServer>` used to construct `MiniHttpRemotingChannelServer` and pass that same object to the automation service. VlppOS will therefore register both `/GacUIRemoteProtocolHttp` and `/Automation/RemotingTest_Core` in one process-local dispatcher on port 8888. For the native renderer `/MiniHTTP`, retain the protocol client on port 8888 but create a distinct `IAsyncSocketServer` on port 8889 for `/Automation/RemotingTest_Rendering_Win32`, because listeners cannot be shared across processes. Move the renderer's Windows HTTP automation endpoint to port 8889 as well, making the renderer automation port independent of transport.

Compile the dedicated shared source into both remoting projects through their `.vcxproj` and `.filters` files, with no header or solution change. Update `GacUI/Project.md`, `Tools/DebugGacUIWithRemoteProtocol.md`, and the related MiniHTTP note in `Tools/DebugGacUIWithBrowser.md` so the documented topology and automation availability match the implementation.

### CODE CHANGE
