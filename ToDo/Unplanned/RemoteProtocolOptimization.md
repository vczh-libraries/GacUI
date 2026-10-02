# Remote Protocol Optimization

- Is it able to pass text diff? Current `IGuiGraphidsParagraph` recreates so some implementation must be changed in the `GuiDocumentElement`.
- `RemotingTest_Win32_Core /FCT` typing performance issue.
  - Half way improved.
  - Will fast typing cause core with HTTP receiving characters in different order?

## Renderer Switching

- A new `DebugRemoteProtocolWithMultipleRenderers.md`, declare how to test renderer switching, including native, gacjs, both.
  - Custom automation service port on native renderer is required so that two native renderers could exist at the same time, although only one is working.
- During switching renderers, remote protocol core might send old requests/messages to the new renderer. Figure out how to handle it.
  - Core is running single threaded, so remembering the client id might help.
  - Client id from `VlppOS`'s channel will be used to check if an underlying new `ControllerConnect` event is sent from a new renderer, as this part might be parallel, and `Submit` would say disconnected in this case.
- Make sure `DebugRemoteProtocolWithMultipleRenderers.md` works with `wGac` and `iGac`.

## Refactor RemoteViewModelChannelServer

To remove dependency on `TaskQueue` in `RemoteViewModelChannelServer`:
- The async service is needed as a replacement.
- But creating `IViewModel` instance already needs a running task queue, but it is created before the main window, therefore before calling `Run(mainWindow)`, which then starts the async service.
- This is a bootstrap deadlock, need to carefully rethink the process.
- Or maybe the task queue is still not needed because creating `IViewModel` results in a blocking `OnJsonRequest` which handles callback internally.

## Binary Protocol

- Workflow RPC
- GacUI Remote Protocol