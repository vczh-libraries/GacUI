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
