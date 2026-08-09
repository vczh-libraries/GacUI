Perform the following refactor on the design in `Test/RemotingHelpers/Rvmt` and affected test apps.

- Implement stdio redirection as `INetworkProtocol(Server|Client)` in `Test/RemotingHelpers/StdioRedirection/StdioRedirection(.(Windows|Linux|macOS))?.(h|cpp)`. If any file results in being semantically empty, we could just not keep it.
  - All files added to `Test/GacUISrc/Source_RemotingHelpers` project.
  - If macOS could share the implementation with linux, then no need to have macOS specific files.
  - Let `Release/CodegenConfig.xml` properly covers new files, meanwhile `../Tools/Tools/Build.ps1 UpdateRelease` should make sure new Linux and optional macOS files get deleted to avoid appearing in `../Release/Import`.
  - Shared code will be in `StdioRedirection.(h|cpp)`, eventually test apps should be able to use the same set of class names across platform.
  - The `INetworkProtocolServer` implementation class should have one extra function `ConnectNewClient`, passing a CLI command, start the target and redirect its stdio.
    - `ConnectNewClient` should be able to call multiple times to connect multiple clients.
    - `Start` does nothing.
    - `Stop` terminates all clients by sending `!Exit` and then `ConnectNewClient` should `CHECK_ERROR` to fail after calling `Stop`.
    - Remote errors are sent via `!Error:message-in-base64`.
    - Other message are sent via base64.
    - Base64 encoding/decoding already offered in upstream repos.
  - `INetworkProtocolClient::WaitForServer` will does nothing, as stdio (aka `Console` class) is the way to communicate to the server.
  - Either a client receiving `!Exit` or a client gets terminated by any other way, is defined as a disconnection.
  - By using base64, each message could occupy a whole line, illegal format of messages are ignored, including base64 decoding failures, with non-fatal local error generated.
    - Currently when a local error occurs, only fatal local error start the termination process, non-fatal local error should just be ignored.
  - Only when technically possible, `ConnectNewClient` could add another argument `bool showClient`, which make the cli window of client also visible and everything get printed to both cli window and the server.
    - It is totally fine if this is not doable, and not to add the `showClient` argument.
- `CppTest_Rvm /Cli:"path"` would run another CLI command with stdio redirection to connect to a view model implementation.
  - During debugging, `path` should points to a `RemotingTest_RvmHost` binary, launch it with `/Cli`.
    - It means `/Cli` should be part of the string sending to `ConnectNewClient`.
  - The cli protocol will be implemented in `Test/RemotingHelpers`, per platform if necessary.
  - `CppTest_Rvm /Cli:"path"` still need start the automation service.
- `RemotingTest_Core /Cli:"path"` only hints how to connect `RemotingTest_RvmHost`, it should still need another network protocol argument to connect to renderers.
  - Update `DebugRemoteProtocol*` to mention that `RemotingTest_RvmHost` will be started by `/Cli:"path"`, but when using other network protocol, `RemotingTest_RvmHost` still need to start manually/separatedly.
  - Fix `Project.md` to catch up all details.
- To clarify the differences of `/Cli` between `CppTest_Rvm` and `RemotingTest_Core`:
  - In `CppTest_Rvm`, `/Cli` is a kind of network protocol, so `/Cli`, `/Pipe`, `/Http`, `/MiniHttp` are exclusive.
  - In `RemotingTest_Core`, `/Cli` only controls how to connect to `RemotingTest_RvmHost`, `/Pipe`, `/Http`, `/MiniHttp` are exclusive, but `/Cli` could be used with them together.
    - When `/Cli` is used, two servers might start parallelly, one to only accept renderers, another only accept rvmhost and reject renderers (`CppTest_Rvm` is using this one already).
- Now `RemotingTest_RvmHost` should be careful what to print to the screen, because with `/Cli` everything will be sent to the server.
- To verify, follow the updated `DebugRemoteProtocolWithNativeRenderer.md` but only test the `/Cli` part of the test matrix.
  - Although you are required to complete Linux/macOS code, but no need to test them as you are working on Windows.
- wGac and iGac modification is out of scope.

## DETAILS

### Stdio transport files and integration

- Put the platform-neutral declarations and shared framing/lifecycle code in `Test/RemotingHelpers/StdioRedirection/StdioRedirection.h` and, when needed, `StdioRedirection.cpp`. Put only unavoidable OS process/pipe work in `StdioRedirection.Windows.cpp`, `StdioRedirection.Linux.cpp`, and an optional `StdioRedirection.macOS.cpp`. Use the Linux implementation for macOS when the differences are small enough to guard locally, and omit any file that would have no semantic content.
- Expose the same platform-neutral server, client, and connection class names on every OS. Implement every member of `INetworkProtocolServer`, `INetworkProtocolClient`, and `INetworkProtocolConnection`, including status and callback installation. The server must remain usable as the concrete `TServerBase` of `NetworkProtocolChannelServer`; the client must remain usable as the `Ptr<INetworkProtocolClient>` composed by `NetworkProtocolChannelClient`.
- Add every created file explicitly to both `Test/GacUISrc/Source_RemotingHelpers/Source_RemotingHelpers.vcxitems` and its `.filters` file under a `StdioRedirection` filter. Keep this shared inventory unconditional: do not add wildcards, `ExcludedFromBuild`, importer-specific conditions, or generated include paths. Make platform translation units safe through the existing platform guards.
- Both portable builds already consume the shared inventory. Among the newly added stdio files, add only `StdioRedirection.Windows.cpp` to the existing `CPP_REMOVES` lists in `Test/Linux/RemotingTest_Core/vmake` and `Test/Linux/RemotingTest_RvmHost/vmake`; preserve all existing removals, and do not hand-edit generated `vmake.txt` or `makefile` files.
- Update `Release/CodegenConfig.xml` so the neutral helper category excludes `.Windows.`, `.Linux.`, and `.macOS.` files. Add platform patterns scoped specifically to `\Test\RemotingHelpers\StdioRedirection\...`, avoiding broad suffix categories that could overlap other scanned `Source`, `Import`, or `Test` files. Emit `Test.RemotingHelpers`, `Test.RemotingHelpers.Windows`, and `Test.RemotingHelpers.Linux`, grouping Linux and optional macOS inputs into the Linux pair as `VlppOS` does.
- Regenerate, rather than hand-edit, the corresponding pairs in `Release` and `Release/IncludeOnly`. Keep all test-helper code out of the ordinary `GacUI*` amalgamations.
- `../Tools/Tools/Build.ps1 UpdateRelease` dispatches the actual aggregate-import work to `../Tools/Tools/BuildRelease.ps1`. Extend the removal list there to cover `Test.RemotingHelpers.Linux.h/.cpp`, so no `Test.RemotingHelpers*` file remains in `../Release/Import` after an update.

### Wire format and connection lifecycle

- Reserve redirected stdin and stdout exclusively for the protocol. Keep diagnostics on a separately inherited stderr stream and never merge them into protocol stdout.
- Convert each `WString` to UTF-8 bytes and then standard padded Base64. Write exactly one complete line while holding the per-connection write lock:
  - An ordinary `SendString` message is the Base64 text alone.
  - A transport remote error is exact `!Error:` followed by the Base64 UTF-8 error text.
  - A shutdown request is exact `!Exit`.
- Use `Console::TryRead`, not `Console::Read`, on the stdio client so EOF is distinguishable from a valid empty line. Decode complete lines in reverse order. Exact control prefixes are unambiguous because `!` is not in the Base64 alphabet. Preserve arbitrary message lengths and partial OS pipe reads.
- `InstallCallback` supports one callback, calls `OnInstalled` immediately, and safely detaches on `nullptr`. Deliver callbacks outside internal locks; callback methods may run on reader threads.
- The stdio client is already physically connected when constructed. Its raw `WaitForServer` is a no-op, `GetStatus` reports `Connected` until terminal input or local `Stop`, and `BeginReadingLoopUnsafe` starts the blocking stdin reader. Exact `!Exit`, EOF, local `Stop`, or another terminal read/process failure transitions to `Disconnected`, produces `OnDisconnected` exactly once when applicable, and never reconnects the same connection.
- `Start` launches no process, but it still establishes the logical started state required by `INetworkProtocolServer`. Reject `ConnectNewClient` before `Start`, reject repeated `Start` or `Start` after `Stop`, make `Stop` idempotent, and make `IsStopped` reflect the terminal state.
- Each `ConnectNewClient` call owns an independent process, stdin/stdout pipe pair, connection, serialized writer, and blocking reader worker. Invoke the virtual `OnClientConnected` exactly once; if it returns `Reject`, stop and reap only that child. Repeated calls must not share connection state.
- Race process creation against `Stop` so no child can escape ownership. `Stop` first bars new launches, sends exact `!Exit` once to every live child, closes the relevant pipe ends, waits for child/read completion, drains active callbacks, and releases process/pipe handles. After it returns, no callback may touch the stopped object. Do not hold collection or write locks while invoking callbacks, waiting, or joining, and do not deadlock when `Stop` is called reentrantly from a callback.
- Fatal local I/O errors call `OnLocalError(..., true)` and then disconnect. Malformed lines call `OnLocalError(..., false)` and must honor the callback's promotion result; the unconditional-ignore requirement remains open in the review comment below.

### RVM startup and application composition

- Preserve the exact RVM host channel set `{ViewModelChannel, ViewModelReadyChannel}` and the existing post-route `Ready` barrier. Do not add a heartbeat, retry, reconnect, lease, requester-stopping message, disconnect acknowledgement, or reverse shutdown handshake.
- Do not launch the host immediately after `RemoteViewModelChannelServer::Start`, because host admission is not ready yet. Call `RemoteViewModelChannelServer::Connect(requiredServiceNames)` first; it runs `RpcServerHelpers::Start`, connects the local requester, starts the task-queue thread, and returns without acquiring the remote service. Then call `ConnectNewClient`, initialize generated RPC, and acquire the service through `RequestService`, where waiting for the host can occur. No new launch hook or `Connect` overload is required.
- Do not move `RpcServerHelpers::Start` into `RemoteViewModelChannelServer::Start`: in the existing non-CLI Core path, the Core local channel must still acquire `GacUIRemoteProtocolCoreClientId == 1` before the RVM helper local clients consume IDs.
- `CppTest_Rvm` accepts exactly one of `/Cli:<nonempty-host-path>`, `/Pipe`, `/Http`, and `/MiniHttp`. Parse a quoted path without relying on the current whole-command-line `strcmp`; quote the executable path and append literal ` /Cli` to the command passed to `ConnectNewClient`. Keep this Windows GUI application console-free.
- `CppTest_Rvm /Cli` has no MiniHTTP socket, so it continues to use Windows HTTP automation on port 8888. Correct the reversed Windows/MiniHTTP endpoint shutdown branches in `CppTest_Rvm/GuiMain.cpp`, which would otherwise stop the wrong automation service in `/Cli` mode.
- `RemotingTest_RvmHost` accepts exact `/Cli` alongside its existing exclusive transport selectors and constructs the stdio client. Preserve its setup order: connect, initialize generated RPC, register the service, send `Ready`, initialize the dispatcher, and run the task queue. Suppress its ordinary stdout banner in `/Cli` mode; any optional diagnostics must use stderr.
- `RemotingTest_Core` accepts at most one `/Cli:<nonempty-host-path>` only with `/RVMT`, and still requires exactly one of `/Pipe`, `/Http`, and `/MiniHttp` for renderers. Reject `/Cli` with default or explicit `/FCT`, with `/RPT`, with no renderer transport, or when duplicated.
- Without `/Cli`, retain the current combined network server and manually started host. With `/Cli`, own two independent servers:
  - A renderer-only `RemotingChannelServer` over the selected renderer transport, containing the Core local protocol client and renderer-replacement state.
  - A host-only `RemoteViewModelChannelServer` over stdio with renderer admission disabled, containing the RVM requester, dispatcher, and auto-launched host.
- In the `/Cli` Core path, start the renderer server and connect the Core local client first so it remains client ID 1; then start the stdio RVM server, call its RVM `Connect`, and spawn/admit the host before generated RPC initialization and `RequestService`. Stop both servers in the reverse ownership order. Route RVM host-loss exceptions through the stdio server's requester, but broadcast the resulting Core-authored fatal error through the renderer server. Renderer replacement must remain independent of host ownership.
- Correct the MiniHTTP automation cleanup branch in `RemotingTest_Rendering_Win32/GuiMain.cpp`; the `/Cli` MiniHTTP verification must stop MiniHTTP rather than call the Windows HTTP stopper again.
- Keep `showClient` optional and Windows-only if it cannot be implemented cleanly without changing the protocol streams or lifecycle.

### Documentation and scope

- Update the RVM topology and argument tables in `Project.md`, `Test/GacUISrc/README.md`, `DebugRemoteProtocolSop.md`, `DebugRemoteProtocolWithNativeRenderer.md`, `DebugRemoteProtocolWithGacJS.md`, and `DebugRemoteProtocolCrossPlatform.md`. `DebugRemoteProtocolWindows.md` can remain a delegating overview if none of its statements become stale.
- State that `/Cli:<path>` auto-launches `RemotingTest_RvmHost` with `/Cli`; do not launch the host manually in that mode. Existing `/Pipe`, `/Http`, and `/MiniHttp` host modes remain manual and keep their same-transport requirement.
- Separate the Core-to-renderer transport dimension from the Core-to-host mode. Update matrix totals and startup examples without multiplying `/Cli` across `/FCT` or `/RPT`: native Windows has `4 standalone + 12 Core = 16`, native Linux/macOS has `1 standalone + 4 Core = 5`, GacJS Windows has `8`, and GacJS Linux/macOS has `4`. Mark Linux/macOS runtime rows as unverified in this Windows task.
- Clarify that the SOP's HTTP/MiniHTTP replacement-poll timing does not apply to `/Cli` host loss, which is observed directly through stdio EOF. The externally started second-host step is a non-CLI scenario unless an explicit `ConnectNewClient` test path is provided.
- Align the test-helper release-pair statement in `.github/Learning/Learning_Coding.md` with the new Linux pair while preserving the rule that `Test/RemotingHelpers` is test-only and the shared project inventory is unconditional.
- Do not modify wGac or iGac in this task.

## VERIFICATION

- Run `git diff --check` in both GacUI and Tools. Confirm every new helper file appears exactly once in `.vcxitems` and `.filters`, the shared inventory has no wildcard/`ExcludedFromBuild`, both portable `vmake` files remove only the Windows implementation, and generated `vmake.txt`/`makefile` files were not hand-edited.
- Build `Test/GacUISrc/GacUISrc.sln` as Debug x64 and Debug Win32 through `.github/Scripts/copilotBuild.ps1`. Run the existing `UnitTest` through `.github/Scripts/copilotExecute.ps1`; require a successful summary and no memory-leak dump. No helper-specific unit test is required by `Project.md`.
- Run `& C:\Code\VczhLibraries\Tools\Tools\Build.ps1 GacUI` to regenerate the GacUI release/CodePack output, and inspect its text output because the wrapper catches and prints exceptions. In both `Release` and `Release/IncludeOnly`, require neutral code only in `Test.RemotingHelpers.*`, Windows code only in `Test.RemotingHelpers.Windows.*`, and Linux plus optional macOS code only in `Test.RemotingHelpers.Linux.*`; require no helper code in ordinary `GacUI*` pairs.
- Snapshot the `C:\Code\VczhLibraries\Release` worktree, run `& C:\Code\VczhLibraries\Tools\Tools\Build.ps1 UpdateRelease`, and inspect its text output for caught failures. Confirm no `Test.RemotingHelpers*` file remains under the aggregate `Release\Import` while GacUI's generated helper pairs remain intact, then restore only aggregate-Release changes produced by this verification.
- On Windows, follow the updated native-renderer guide for four fresh `/Cli` targets only:
  - `CppTest_Rvm /Cli:"<absolute-RemotingTest_RvmHost.exe>"`, with no renderer.
  - `RemotingTest_Core /RVMT /Pipe /Cli:"<absolute-RemotingTest_RvmHost.exe>"` with a `/Pipe` renderer.
  - `RemotingTest_Core /RVMT /Http /Cli:"<absolute-RemotingTest_RvmHost.exe>"` with an `/Http` renderer.
  - `RemotingTest_Core /RVMT /MiniHttp /Cli:"<absolute-RemotingTest_RvmHost.exe>"` with a `/MiniHttp` renderer.
- For every target, require exactly one auto-launched host whose sole selector is `/Cli`; wait for the `Remote View Model Test` UI and exact `Hello, !`; type a unique marker through the local surface or renderer-side `/IO`; require exact `Hello, <marker>!`, matching Core `Controls`/renderer `Dom` where applicable, and no malformed-line, error, or disconnect state.
- Launch every manually started requester, Core, renderer, or negative-test host asynchronously through `.github/Scripts/copilotExecute.ps1 -Mode CLI` with Debug/x64 arguments prepared in the ignored project-specific `.vcxproj.user` files. The auto-launched `/Cli` host intentionally bypasses the wrapper because that product behavior is under test.
- For standalone `/Cli`, require a nonempty response from `http://localhost:8888/Automation/CppTest_Rvm/Controls` and perform the marker interaction through that endpoint. For each Core row, probe Core `Controls` on port 8888 and renderer `Dom` on its default or explicitly selected automation port, using at least one nondefault renderer port. Use `127.0.0.1` for MiniHTTP probes.
- Close each target through its active UI. Require the requester/Core, renderer, and child host to terminate with no stale listener, orphan process, runtime-error dialog, or unexpected prompt. The child may retain its current nonzero exit on `OnDisconnected`; process cleanup, not a new exit-code contract, is under test.
- In fresh runs for both requester shapes, force-terminate the auto-launched host after a successful translation and require stdio EOF to record disconnection promptly. Then type another marker to trigger the next `Translate`, or terminate the host during an in-flight `Translate`; require the call to finish within the documented bound. `CppTest_Rvm` must then terminate nonzero; Core must send exactly one Core-authored `RemotingTest_RvmHost disconnected.` fatal package to the retained renderer and then terminate nonzero. Require no orphan or retry/reconnect loop.
- During one successful Core `/Cli` run, start a manual RVM host on the renderer transport. Require the renderer-only server to reject it without disturbing the auto-launched stdio host, then prove the original host still translates another marker.
- Smoke-test argument rejection, including a quoted host path containing spaces: reject `CppTest_Rvm` when `/Cli` is empty, duplicated, or combined with another transport; reject Core `/Cli` without `/RVMT`, with a non-RVMT selector, without a renderer transport, or when duplicated.
- Exercise `ConnectNewClient` repeatedly and malformed/remote-error framing through an explicit focused test path if one is added; otherwise document that the four application rows cover one child per server and do not claim direct runtime coverage for those lower-level requirements.
- Do not claim Linux/macOS build or runtime verification, and do not modify or test wGac/iGac.

## REVIEW COMMENTS

### MALFORMED INPUT CONFLICTS WITH CHANNEL ERROR PROMOTION

**review comment**: The task requires malformed or undecodable lines to call `OnLocalError(..., false)` and then be ignored. However, `INetworkProtocolCallback::OnLocalError` lets the callback promote a nonfatal error, and both `NetworkProtocolChannelClient` and an admitted `NetworkProtocolChannelServer` deliberately promote every post-admission local error to fatal because channel delivery is no longer reliable. All in-scope RVM consumers use that channel layer, so a malformed stdout line cannot both be reported through this callback and be unconditionally ignored without violating the current contract.

**suggested solution**: Preserve the `INetworkProtocol` contract: report malformed input with `fatal == false`, honor the callback's return, and continue only when it declines promotion. If malformed lines must always be ignored after channel admission, either ignore them without calling `OnLocalError` or explicitly change the upstream callback/channel contract and its documentation/tests; do not silently discard a promotion result in the stdio transport.

good catch, then we should just ignore malformed messages without doing local error.

### NO OPERATION SENDS A TRANSPORT REMOTE ERROR

**review comment**: The wire format reserves `!Error:<base64>` for a transport remote error, but `INetworkProtocolConnection` exposes only `SendString`; `OnReadError` is receive-only. Channel-level `BroadcastError` is not this operation: it serializes an ordinary channel package whose channel name is `!Error` and sends the whole package through `SendString`, so the stdio transport must Base64 that complete ordinary string. The task does not identify any code that can emit the raw `!Error:` control line or distinguish it from `SendString`.

**suggested solution**: Either expose a concrete stdio-connection `SendError(const WString&)` operation/shared framing helper and define its intended callers, or state that raw `!Error:` is receive-only/external in this task. Keep the transport control frame explicitly distinct from the channel-level `!Error` package.

Looks like it is IChannelServer who defines the format of errors. So the correct design should be:
- only keep !Exit as a termination signal
- others are pure base64 encoded messages, and IChannel(Server|Client) will handling error encoding and decoding.
- !Exit should just mean disconnection, it is test apps who treat disconnection as a signal to terminate. Responsibility boundary ahould be cleared.
