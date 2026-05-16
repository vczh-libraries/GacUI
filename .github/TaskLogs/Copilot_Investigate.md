# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

## Affected Repos

This task is running in the monorepo, here are repos that expected to work on:
- VlppOS
- GacUI
- GacJS

## Background

GacUI can be launched with remote protocol, and it can be used in:
- UnitTest.
- `RemotingTest_Core /RPT /Pipe` followed by `RemotingTest_Rendering_Win32 /Pipe`.
  - Unfortunately UI Automation is not supported, but in order to test if the application can be launched, you can debug two processes, and after a few seconds if nothing bad happens, especially no uncought exception happens, kill the two processes directly.
  - You can also put a break point on `GuiRemoteRendererSingle::RenderDom` and see if it is hit multiple times with proper things in `dom`.
- `RemotingTest_Core /RPT /Http` followed by `GacJS`.
  - You can use playwright to run `http://localhost:8896` after building `GacJS`, it will refresh the website.

In order to simplify cross process message transimission, rendering commands are serialized to JSON and then string and then go through network protocol.
JSON translation is in `GuiRemoteProtocolFromJsonChannel` and `GuiRemoteJsonChannelFromProtocol`.
Connecting async channel to GacUI Core in UI thread is by `GuiRemoteProtocolAsyncChannelSerializer`.
Actual network protocol processing is in `GacUI\Test\GacUISrc\RemotingTest_Core\Shared`.
Channel and network protocol are designed and implemented to be working on 1 client to 1 server mode.

There are more features to join but the current channel and network protocol design cannot work with new ideas.
I have to refactor channel to be multiple client to 1 server mode.
Now the new channel design is in VlppOS, putting in vl::inter_process namespace.

Two main biggest breaking change besides of completely different naming:
- From 1 client to 1 server mode to multiple clients.
- Wording are changed.
  - Currently writing means from server to client and reading means from client to server, it makes writing and reading in the client side meaning the reverse thing, e.g. writing will be receiving message and reading will be sending message in client.
  - In the new design, no more such confusing thing, writing always mean sending messages away, reading always mean receiving incoming messages.
  - Since the server never ask the client for `GetExecutablePath` and `ProcessRemoteEvents` when channel is involved (in part of the unit test core and renderer talk to each other using only `IGuiRemoteProtocol`, things are different), so they are removed from the channel.

## Goal and Refactoring

Many code in `GacUI` have to be deleted and replaced by new constructions in `VlppOS`, including but not limited to:
- `GacUI\Test\GacUISrc\RemotingTest_Core\Shared`, they can be found in `VlppOS.Windows.(h|cpp)`. Be aware that the header file is a new file. Currently `VlppOS\Release` has the latest release in `VlppOS`, but `GacUI\Import` is still old. When you copy files to `GacUI`, you will need to add `VlppOS.Windows.h` to all proper position, you can find how many projects have `VlppOS.Windows.cpp` and put the header file there.
- `GuiRemoteProtocol_Channel_Shared.h`, completed replaced by `vl::inter_process::IChannel(Server|Client)`. There is a `NetworkProtocolChannel(Server|Client)` to convert existing HTTP or NamedPipe `INetworkProtocol(Server|Client)` to `IChannel(Server|Client)`.
  - The old design connects server (core) and client (rendering), and they can send messages to each other.
  - The new design makes server only doing message delivering, you are supposed to use `IChannelServer::ConnectLocalClient` to hook a core client in the same process, and use the core client to talk to the renderer client.
  - New makes each client talk to each other in multiple parallel channels, but here we only need one, we can name it `GacUIRemoteProtocol`.
- `GuiRemoteProtocol_Channel_Json.h` needs to be refactored. You can keep most code converting from `IGuiRemoteProtocol` to `IChannelClient`:
  - Name the core side `GuiRemoteProtocolCoreChannel` and the rendering side `GuiRemoteProtocolRendererChannel`.
  - You can hook `IChannel(Server|Client)<JsonNode>` to `INetworkProtocol(Server|Client)` by `NetworkProtocolChannel(Server|Client)` with a new serializer injection, to convert between `List<Ptr<JsonNode>>` to `WString`, put the serialization injection in this file too.
- `GuiRemoteProtocol_Channel_Async.h` needs to be completely rewritten.
  - In `GacUI\Test\GacUISrc\RemotingTest_Core\GuiMain.cpp` you can find remote protocol is started with the server in the current thread and run channel and UI in two new thread.
  - The new design should be:
    - Start the server, register the core channel first, wait for the renderer channel, assert that it has the `GacUIRemoteProtocol` channel (in `GuiMain.cpp`).
    - Start GacUI with remote protocol in the same thread, but register the reader to the core channel and send everything to the UI thread using `GetApplication()->InvokeInMainThread`, in the UI thread, JSON will be converted back to `IGuiRemoteProtocol` method calls.
    - Checkout out how `RemotingTest_Rendering_Win32` do the threading thing.
  - Previous async channel is universal, the new design you could make it `IChannel(Server|Client)<Ptr<JsonNode>>`.
  - The connection will be `core side IGuiRemoteProtocol` -> `JSON Serialization channel` -> `async channel` -> `network protocol` -> `JSON Serialization channel` -> `renderer side IGuiRemoteProtocol`.
    - Unit test with async channel does not go through network protocol, messages are passed directly between threads.
- Some unit test utility constructions, and anything that will be impacted.
- The new Http implementations need you to specify a URL fraction name and a port, use `GacUIRemoteProtocolHttp` and `8888`, and `GacUIRemoteProtocolNamedPipe`.

## Tips

Since the channel send commands in batch, currently multiple JSON commands are serialized to string, and then put in a JSON array and serialize again. The new design should be able to avoid double serialization, making the protocol running faster.

`ChannelPackageSemanticUnpack` is designed to let async channel tell if the current command is `ControllerConnect`. But the new design let you access JSON directly, so no such construction is needed, the async channel should work on JSON directly. But you can still make helper functions in `GuiRemoteProtocol_Channel_Json.h`, it can be used by JSON serialization itself too.

Since there will be no need to create UI and channel thread in async channel now, the code should be much simpler, close to complete rewriting.

## Verification

### VlppOS

If you need to fix anything in `VlppOS(.Windows)?.(h|cpp)`, you have to fix it in `VlppOS` repo, run the unit test, re-import the new change from `VlppOS` to `GacUI`.

### GacUI

First you need to make the unit test work on your new change.
Since this is a refactoring to channels so it should not touch any test cases that only depends on `IGuiRemoteProtocol`.
You don't have to make `RemotingTest_Core` and `RemotingTest_Rendering_Win32` build in order to run `UnitTest`.
Focus on test cases that set `UnitTestScreenConfig::useChannel` to `Sync` and `Async`, they are in `TestRemote_UnitTestFramework.cpp` and `TestRemote_UnitTestFramework_Async.cpp`, you can run only these 2 files until they pass, and run the whole thing after that.

Secondly you need to make `RemotingTest_Core` and `RemotingTest_Rendering_Win32` runs properly, checkout `## Background` for details.

### GacJS

The new Http implementation slightly changing the URL structure and JSON content, you need to fix the code to make them work.
Checkout `Tools\DebugGacUIWithBrowser.md` for how to launch the test.
Checkout `## Background` for details.

## Review Your own Work

You can checkout `TestInterProcess.cpp` in `VlppOS` to understand how `IChannel(Server|Client)` work with `INetworkProtocol(Server|Client)`.
Remove all code in `GacUI` that is no longer needed.

## Finishing

After `GacUI` repo passing `UnitTest`, you need to commit and push `VlppOS` and `GacUI` immediately.
After `GacUI` repo passing the rest of the verification, you need to commit and push `VlppOS` and `GacUI` immediately.
After fixing `GacJS`, you need to commit and push all local changes in these repos immediately.

# UPDATES

## UPDATE

Bug 1: `RemotingTest_Core /Http /RPT` should deliver the exception raised by the `Fatel Error` button through `IChannelServer::BroadcastError`, so GacJS can receive the `!Error` package and display the alert/error UI. Before the fix the core exited and the browser only observed the transport closing. The same fatal package delivery path is shared by `RemotingTest_Rendering_Win32`, so the core-side channel and transport shutdown behavior must preserve the fatal package before closing.

Bug 2: `RemotingTest_Rendering_Win32` must not mutate the renderer DOM, Direct2D paragraphs, or native window state from the network reader thread. The renderer channel now queues core-to-renderer protocol packages to the GacUI main thread before dispatching them to `GuiRemoteRendererSingle`. While building the stress harness, a separate `VlppOS` disconnect lifetime bug was found: `NetworkProtocolChannelServer::OnConnectionDisconnected` removed the last owning `Ptr<Connection>` and then signaled through the raw pointer. That has been fixed in `VlppOS`, regenerated into `VlppOS\Release`, and imported into `GacUI`.

# TEST

- Build and run the affected VlppOS unit test coverage around the new `vl::inter_process` channels, especially `TestInterProcess.cpp`.
- Build GacUI UnitTest and first run `TestRemote_UnitTestFramework.cpp` and `TestRemote_UnitTestFramework_Async.cpp`, because those are the tests using `UnitTestScreenConfig::useChannel` with synchronous and asynchronous channels.
- Run the full GacUI UnitTest after the focused remote-channel tests pass.
- Build and run `RemotingTest_Core /RPT /Pipe` with `RemotingTest_Rendering_Win32 /Pipe`; success means both processes stay alive for a short launch/render window without uncaught exceptions.
- Build GacJS, run `RemotingTest_Core /RPT /Http`, open `http://localhost:8896` with Playwright, and verify the browser client connects to the updated HTTP protocol without errors.

# PROPOSALS

- No.1 Deliver fatal channel errors before closing transports [CONFIRMED]
- No.2 Dispatch renderer protocol packages on the UI thread [CONFIRMED]

## No.1 Deliver fatal channel errors before closing transports

### CODE CHANGE

- In VlppOS, `NetworkProtocolChannelServer::BroadcastError` now leaves a short shutdown grace after sending `!Error`, so HTTP and named-pipe clients can consume the fatal package before the transport is stopped.
- In VlppOS HTTP transport, a server package produced while processing a client `/Response` POST can be returned in that POST response body. The Windows HTTP client and GacJS HTTP client both read non-empty POST response bodies as normal server packages, covering request/reply races where no long-poll receive is available yet.
- In VlppOS named-pipe transport, writes are serialized and overlapped writes wait for completion before handles are canceled or closed, preventing the fatal package from being lost during immediate shutdown.
- Regenerated VlppOS release files and copied the updated generated files into GacUI `Import`.

### CONFIRMED

- Rebuilt VlppOS and GacUI successfully.
- Rebuilt GacJS successfully.
- Reproduced Bug 1 through `RemotingTest_Core /RPT /Http` and GacJS at `http://localhost:8896/index.html`; after clicking `Fatel Error`, the page logged `Error: This is a fatel error!` from `HttpClientImpl.handleNetworkPackageText` and displayed the error mask, confirming the browser received the `!Error` package instead of only a fetch failure.
- Ran VlppOS UnitTest: passed 12/12 files and 115/115 cases.
- Ran GacUI UnitTest: passed 84/84 files and 1686/1686 cases.
- Ran GacJS `yarn build`: passed. Ran GacJS `yarn test`: package unit tests passed, but existing website E2E tests failed in font/image dialog flows unrelated to this HTTP fatal-error path.

## No.2 Dispatch renderer protocol packages on the UI thread

### CODE CHANGE

- In GacUI, `GuiRemoteProtocolRendererChannel::OnRead` now queues incoming core messages/requests and invokes their processing on the GacUI main thread when a running application is available. This prevents `GuiRemoteRendererSingle` rendering/document/window state from being touched by transport callback threads.
- In VlppOS, `NetworkProtocolChannelServer::OnConnectionDisconnected` keeps an owning `Ptr<Connection>` while signaling disconnect, avoiding a use-after-free when a renderer disconnects or the stress harness closes it.
- Regenerated `VlppOS\Release` and copied the updated generated `VlppOS.h` into `GacUI\Import`.

### CONFIRMED

- Rebuilt GacUI successfully.
- Ran the UIA stress harness against `RemotingTest_Core /FCT /Pipe` and `RemotingTest_Rendering_Win32 /Pipe`: 5 consecutive runs of 1000 typed characters passed, with both processes alive and the renderer responsive after each run.
- Ran the same UIA stress harness against `/FCT /Http` and `/Http`: 5 consecutive runs of 1000 typed characters passed, with both processes alive and the renderer responsive after each run.
- Rebuilt VlppOS and ran VlppOS UnitTest: passed 12/12 files and 115/115 cases.
- Ran GacUI UnitTest: passed 84/84 files and 1686/1686 cases.

## Implemented So Far

- Replaced the GacUI remote protocol channel layer with `vl::inter_process` channel server/client abstractions.
- Added JSON package packing/unpacking and a `List<Ptr<JsonNode>>` serializer so network batches carry one JSON array of protocol packages instead of stringifying each command twice.
- Replaced the old async channel serializer with a JSON-channel async wrapper that queues incoming events for UI-thread delivery and waits for matching response ids.
- Updated unit-test remote channel setup to use a local `IChannelServer` with core and renderer clients.
- Replaced `RemotingTest_Core` and `RemotingTest_Rendering_Win32` transport glue with `NetworkProtocolChannelServer/Client` over the VlppOS HTTP and named-pipe implementations.
- Imported regenerated VlppOS release files into GacUI, including the new `VlppOS.Windows.h`.
- Removed obsolete GacUI shared channel and old remoting transport source files.
- Fixed `NetworkProtocolChannel::BatchWrite` in VlppOS so adjacent outgoing packages with the same sender/receiver are delivered as one batch.

## Verification So Far

- `VlppOS` build: passed.
- `VlppOS` focused unit test: `TestInterProcess.cpp`, passed 4/4 cases.
- `GacUI` build: passed.
- `GacUI` focused channel unit tests: `TestRemote_UnitTestFramework.cpp` and `TestRemote_UnitTestFramework_Async.cpp`, passed 2/2 files and 13/13 cases.
- `GacUI` full unit tests: passed 84/84 files and 1686/1686 cases.
