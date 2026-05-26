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
