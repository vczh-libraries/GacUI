# !!!INVESTIGATE!!!
# PROBLEM DESCRIPTION

## Task 4

This task happens in `GacUI` repo.
Release `VlppOS`, import into `GacUI`.
Make sure `RemotingTest_Core` and `RemotingTest_Win32_Renderer` works with `/Http /FCT`.
To verify that, you should launch both processes with the debugger, so that you are able to know the renderer actually communicate commands correctly with core.
You may need to write a piece of temporary powershell script to close the process in a gentle way:
  - You can use whatever way you like, including calling Windows API, to close the renderer gently.
  - Currently `RemotingTest_Win32_Core` and `RemotingTest_Win32_Renderer` crashes during exiting, figure out why and fix that.
    - I don't remember which one crashes, or maybe both, figure out by yourself.
  - After the renderer exits, ensure that core will be notified and exits. If the code has no problem core should already been working that way.
  - Delete that script before commiting.
- Ensure `RemotingTest_*` call `IChannelServer::Stop`, and this function should already been returning after all pending callbacks end, so that you might be able to remove unnecessary blocking code in main/WinMain/GuiMain.

# UPDATES

- `VlppOS` was released after the Windows HTTP client long-poll request was changed to use an infinite receive timeout. The generated non-`IncludeOnly` release files were copied into `GacUI\Import`.
- Debugging `/Http /FCT` found two shutdown issues after import:
  - Pausing under the debugger could make the renderer's HTTP `/Request` long-poll hit WinHTTP timeout `12002`; the `VlppOS` HTTP client API now exposes per-request timeouts and `HttpClient` disables the receive timeout for long-poll requests.
  - Renderer shutdown could finish before the final `RequestControllerConnectionStopped` packet arrived; the remoting test renderer now exits on transport disconnect and explicitly stops the network connection before stack-owned channel wrappers are destroyed.
- `GuiRemoteProtocolAsyncJsonChannelRenderer` no longer calls `Initialize(nullptr)` in its destructor. `IChannel::Initialize` is install-only, so calling it with `nullptr` caused the runtime dialog/assertion seen during renderer shutdown.
- The full GacUI unit test run exposed two unrelated test-lifetime leaks. `GacUIUnitTest_StartAsync` now waits for and deletes its helper thread, and the `NodeItemProvider` multiple-callback test detaches stack callbacks before leaving scope.

# TEST

- Ran `VlppOS` unit tests: 12/12 files and 115/115 cases passed, with no memory leak dump.
- Regenerated `VlppOS\Release` and copied `VlppOS.h`, `VlppOS.cpp`, `VlppOS.Windows.h`, `VlppOS.Windows.cpp`, and `VlppOS.Linux.cpp` into `GacUI\Import`.
- Built `GacUI\Test\GacUISrc\GacUISrc.sln`: build succeeded with 0 warnings and 0 errors.
- Direct `/Http /FCT` run: started `RemotingTest_Core.exe /Http /FCT` and `RemotingTest_Rendering_Win32.exe /Http`, posted one normal `SC_CLOSE` message to the renderer window, and observed renderer exit code 0 and core exit code 0.
- Debugger `/Http /FCT` run: launched both processes under CDB, set `GuiRemoteRendererSingle::RenderDom` as a renderer-side breakpoint with automatic `gc`, observed repeated `RENDERDOM_HIT` entries, posted one normal close message to the renderer, and observed no remaining `cdb`, core, or renderer processes.
- Ran `GacUI` unit tests after the fixes: 84/84 files and 1686/1686 cases passed. `Execute.log` ended at the pass counts and contained no `Detected memory leaks!` dump.
- Regenerated `GacUI\Release` after GacUI source changes.

# PROPOSALS

- No.1 Update remoting lifecycle for explicit channel-server stop

## No.1 Update remoting lifecycle for explicit channel-server stop

Import the regenerated `VlppOS` release first, then build and debug `RemotingTest_Core` and `RemotingTest_Win32_Renderer` to locate stale channel lifecycle or shutdown assumptions. Fix the narrowest GacUI-side lifecycle code so the tests explicitly stop channel servers and no longer keep redundant blocking code after `IChannelServer::Stop`.

### CODE CHANGE

- `VlppOS\Source\InterProcess\Windows\HttpClientApi.Windows.(h|cpp)`: added timeout fields to `HttpRequest` and applies them with `WinHttpSetTimeouts`.
- `VlppOS\Source\InterProcess\Windows\HttpClient.Windows.cpp`: sets `/Request` receive timeout to `0` so long-poll requests survive debugger pauses and idle periods.
- `GacUI\Import`: imported the regenerated `VlppOS` release files.
- `GacUI\Source\PlatformProviders\Remote\GuiRemoteProtocol_Channel_AsyncRenderer.cpp`: destructor now clears queued async state instead of trying to uninstall the channel reader with `Initialize(nullptr)`.
- `GacUI\Test\GacUISrc\RemotingTest_Rendering_Win32\GuiMain.cpp`: added a small channel-client subclass that exits the renderer on transport disconnect, clears the renderer pointer before stop, and explicitly stops the network connection after the renderer loop returns.
- `GacUI\Source\UnitTestUtilities\GuiUnitTestUtilities.cpp`: async unit-test helper thread is now created as non-self-deleting, waited, and deleted explicitly.
- `GacUI\Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp`: the multiple-callback test detaches all stack callbacks before scope exit.
- `GacUI\Release`: regenerated generated release files for the source changes.
