# !!!INVESTIGATE!!!
# PROBLEM DESCRIPTION

## Task 5

This task happens in `GacUI` repo.

`GuiRemoteProtocolAsyncJsonChannelSerializer` in `GuiRemoteProtocol_Channel_Async.h`
- Rename to `xxxJsonChannelRenderer` and `xxx_Async.(h|cpp)`.

`RemotingTest_Core\GuiMain.cpp`:
- `StartServer` implements `acceptMultipleRenderers` in a wrong way.
  - Now the new named pipe architecture should accept multiple renderers.
  - An renderer can always be accepted and disconnect the previous one.
  - The current implementation does not disconnect the previous one, causing it still able to sends IO events but not rendering.

`GuiRemoteProtocol_Channel_Json.h`
- `GuiRemoteProtocolCoreChannel::Write` when `rendererClientId` is -1, it should stores, and when `Submit` is called without knowing the information, it is discarded.
- `lockRendererClientId` is unnecessary, `atomic_vint` is enough.

`GuiRemoteProtocolAsyncJsonChannelRenderer`
- `SetInvokeInMainThread` should restore missed commands, or just initialize it before `WaitForServer` as an empty window would show before connection, choose one that is easier to do with clean code.
- This class could just be a `IJsonChannelReader` as almost all other members are just redirection:
  - Figure out if it actually reasonable to do that first. I don't want you to force twisting the code.

# UPDATES

# TEST

- Build `GacUI\Test\GacUISrc\GacUISrc.sln`.
- Run `GacUI` unit tests and check `Execute.log` for no memory leak dump.
- Exercise the remoting test path affected by accepting/replacing renderers.

# PROPOSALS

- No.1 Refactor async JSON channel naming and renderer handoff

## No.1 Refactor async JSON channel naming and renderer handoff

Rename the async channel serializer to its renderer-facing name, then fix the channel ownership/lifecycle details without forcing an unnecessary interface twist. Update the core channel to keep/discard writes according to renderer id availability, and update the remoting core server to disconnect stale renderers when a replacement connects.

### CODE CHANGE

- Renamed the async JSON serializer surface to the renderer naming and regenerated `Release`.
- Kept `GuiRemoteProtocolAsyncJsonChannelRenderer` as a wrapper instead of forcing it into only `IJsonChannelReader`: it still owns queued-message scheduling, connection wait state, and main-thread dispatch, so reducing it further would make the code less direct.
- Fixed async renderer initialization ordering by allowing messages to queue before the application object exists, then processing them once the main-thread invoker is available.
- Added generation-stamping to the async renderer wrapper so queued callbacks from a detached reader cannot run against a new reader after disconnect/reconnect.
- Changed `GuiRemoteProtocolCoreChannel` to use atomic renderer id ownership, ignore stale renderer messages, discard submit batches without a renderer, and expose `DetachRenderer` for server-side replacement.
- Updated `RemotingTest_Core` server replacement logic so a new renderer always becomes current, the old renderer is detached, and the old renderer receives a raw `ControllerConnectionStopped` package; if notification fails, the server falls back to disconnecting the old client.
- Updated the Win32 renderer test client to detach the async renderer channel before fatal-exit handling during server disconnect.
- Split remote controller/window connection replay so `Run()` only submits state after a pre-run controller connection, avoiding duplicate startup/DPI callbacks while still preventing an empty pre-connection window.
- Regenerated `Release/GacUI.*` and updated affected remoting snapshots.

### TEST RESULT

- `GacUI\Test\GacUISrc\GacUISrc.sln` builds successfully with 0 warnings and 0 errors.
- Named pipe `/Pipe /FCT` two-renderer smoke passes: first renderer exits after replacement, second renderer exits cleanly after close, and core exits cleanly.
- HTTP `/Http /FCT` core/renderer smoke passes: renderer exits `0`, core exits `0`.
- Focused remote tests pass:
  - `TestRemote_Startup.cpp`: 5/5.
  - `TestRemote_UnitTestFramework_Async.cpp`: 3/3.
  - `TestRemote_EmptyWindow.cpp`: 38/38.
- Full unit pass succeeds through `copilotExecute.ps1 -Mode UnitTest -Executable UnitTest -Configuration Debug -Platform x64`: 84/84 test files and 1686/1686 test cases.
- Memory leak output files from the focused and full unit runs are empty.

## Task 6

This task happens in `GacUI` and `GacJS` repo.

- Followed `Tools\DebugGacUIWithBrowser.md`.
- Built GacJS from `GacJS\Gaclib` with `yarn build`.
- Started the sibling GacUI server as `RemotingTest_Core.exe /RPT /Http`.
- Opened `http://localhost:8896/index.html` in the browser client and verified title `Remote Protocol Test`.
- Verified the browser-rendered RPT UI received and sent protocol messages:
  - Clicking `Click Me!` changed the remote UI text to `You have clicked!`.
  - Switching to `DataGrid` and clicking `Add 3 Rows` populated rows (`Nec Odio`, `Adipiscing Elit Integer`, etc.).
  - Browser console warning/error log was empty during the manual `/RPT` run.
  - Clicking `Exit`, then confirming `OK`, produced `IGacUIRenderer exited due to receiving RequestControllerConnectionStopped.` and `RemotingTest_Core.exe` exited.
- Ran focused GacJS protocol tests from `GacJS\Gaclib\website\entry`:
  - `npx vitest run Testing_Protocol_SimpleTyping.js Testing_Protocol_RendererSwitching.js`
  - Passed 2/2 test files and 11/11 tests.
- Extra full `GacJS\Gaclib` `yarn test` was attempted. It failed outside the `/RPT` manual path in existing `/FCT` browser E2E dialog scenarios:
  - `Testing_Protocol_Font.js` did not recognize the localized font dialog text (`选择字体` instead of the expected `Choose Font`), then formatting assertions stayed at `12px/#FFFFFF`.
  - `Testing_Protocol_ImageInText.js` failed to insert the expected image through the file dialog, causing later text/image assertions to cascade.
  - Lower-level packages and the focused remoting connection tests passed.
