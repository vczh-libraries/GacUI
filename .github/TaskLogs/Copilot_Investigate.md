# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

## Task 1

This task happens in `VlppOS` repo.

`IChannelClient::OnError` is renamed to `IChannelClient::OnReadError`. This is a callback that only listen to `IChannelServer::BroadcastError`.
`IChannelClient::OnLocalError` is added. Other errors go here.
- For example, currently `HttpClient` calls `RaiseErrorUnsafe` when there is any connection error happens, it should call `OnLocalError` instead.
- When `/Connect` or `/Respond` fails, retry for 3 times. Each failure calls `OnLocalError`, non fatal except for the last retry
- When `/Request` fails, just retry, because it would timeout when the http server does not send any request, no `OnLocalError` is needed.
- When the client already stopped or disconnected, no retry is issued, and in this case, no `OnLocalError` is needed.
- For named pipe, any error causing the pipe to close would issue `OnLocalError` with fatal.
- Only after calling `OnLocalError` with fatal, disconnect the client.
`HttpServerApi::SendResponse` should group the last 4 arguments into a struct `HttpServerResponse`.

## Task 2

This task happens in `VlppParser2` repo.
Release `VlppOS` to `VlppParser2`.

Move `JsonNodeListSerializer` from `GuiRemoteProtocol_Channel_Json.h` in `GacUI` to `GlrJson.h` in `VlppParser2`, put it in namespace `vl::glr::json`.
Add a similar `XmlElementListSerializer` in `GlrXml.h`.
- When serializing, group all elements in an `<Array>` element, build an element (not document) and `XmlPrint`.
- When deserializing, just use all elements in the root element, no need to care about the name or attributes of the root element. Use `XmlParseElement`.

## Task (Final Validation)

This task happens in `GacUI` and `GacJS` repo.
Release `VlppOS` and `VlppParser2 to `GacUI`.

### GacUI

Make sure `RemotingTest_Core` and `RemotingTest_Win32_Renderer` works with `/Http /FCT`.
- Since `IChannelClient::OnError` is renamed, the renderer will build break, just rename the function too, but no need to handle `OnLocalError`.
- Delete `JsonNodeListSerializer` as it is moved to `VlppParser2`.

To verify that, you should launch both processes with the debugger, so that you are able to know the renderer actually communicate commands correctly with core.
You may need to write a piece of temporary powershell script to close the process in a gentle way:
  - You can use whatever way you like, including calling Windows API, to close the renderer gently.
  - After the renderer exits, ensure that core will be notified and exits. If the code has no problem core should already been working that way.
  - Delete that script before commiting.

Release `GacUI`.

### GacJS

Follows `Tools\DebugGacUIWithBrowser.md` to run `RemotingTest_Core` with `/Http /RPT` and make sure `GacJS` could launch and operate the UI.
Make sure test cases work. Half of tests fail in `Gaclib\website\entry`, figure out why and fix all of them.
- Hint: it works on an en-US machine which is faster.
- You are now running on an zh-CN machine which is slower.
- Some cases fail because of localization, make sure it works on both zh-CN and en-US.
- Machine performance might not be a factor, make your own judgement.

# UPDATES

# TEST [CONFIRMED]

Validate the downstream remoting handoff:

- Copy updated `VlppOS` and `VlppParser2` release outputs into `GacUI\Import` without `IncludeOnly`.
- Build `Test\GacUISrc\GacUISrc.sln` through `copilotBuild.ps1`.
- Run `RemotingTest_Core.exe /Http /FCT` and `RemotingTest_Rendering_Win32.exe /Http` under CDB. The renderer executable accepts `/Http` or `/Pipe`; the `/FCT` selection belongs to core.
- Observe protocol traffic with breakpoints or logs on the core/renderer channel path, close the renderer gently, and confirm the core notices the renderer disconnect and exits.
- Regenerate `GacUI\Release` with CodePack.

Confirmed with:

- `& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1` from `Test\GacUISrc`: build succeeded with 0 errors and 1 warning.
- CDB-launched `RemotingTest_Core.exe /Http /FCT` and `RemotingTest_Rendering_Win32.exe /Http` with breakpoints on core/renderer read/write paths.
- Observed protocol traffic: `CoreOnReadHits=37`, `CoreWriteHits=133`, `RendererOnReadHits=128`, `RendererWriteHits=37`.
- Posted `WM_CLOSE` only to the visible renderer window (`Complete Control Showcase`); renderer exited and core exited afterward.
- Temporary CDB command files and close-helper code were created under `%TEMP%` and deleted after validation.
- `Tools\Tools\CodePack.backup.exe Release\CodegenConfig.xml` regenerated `GacUI\Release`.

# PROPOSALS [CONFIRMED]

- No.1 Import updated dependencies and adapt remoting channel serialization

## No.1 Import updated dependencies and adapt remoting channel serialization

Refresh GacUI imports from the updated `VlppOS` and `VlppParser2` releases. Remove the local `JsonNodeListSerializer` from `GuiRemoteProtocol_Channel_Json.*` and use the new `vl::glr::json::JsonNodeListSerializer` as the serializer type for the JSON remoting channels. Rename the renderer's `OnError` override to `OnReadError` so it matches the new `IChannelClient` API; no renderer behavior is needed for `OnLocalError` in this task.

### CODE CHANGE

- Refreshed `Import\VlppOS.*` and `Import\VlppGlrParser*` from upstream release outputs.
- Removed GacUI's local `JsonNodeListSerializer` implementation from `GuiRemoteProtocol_Channel_Json.*`.
- Switched JSON channel typedefs to `vl::glr::json::JsonNodeListSerializer`.
- Renamed the renderer callback override from `OnError` to `OnReadError`.
- Regenerated `Release\GacUI.h` and `Release\GacUI.cpp`.

### CONFIRMED

GacUI builds with the imported dependency releases, the `/Http /FCT` remoting pair exchanges commands under the debugger, and a gentle renderer window close causes both renderer and core processes to exit.
