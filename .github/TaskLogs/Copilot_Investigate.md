# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

A small refactor:
- Affected files are for building test apps, no need to maintain library flexibility
- Scan markdown files, fix path references.
- Build test apps, try `RemotingTest_RvmHost` with `CppTest_Rvm` and `RemotingTest_(Core|Renderer_Win32)`. No unit test is required to run.
- commit and push after finishing

Delete `RemotingChannelClientConfiguration`:
- fatalTitle could be passed from constructor arguments of `RemotingChannelClient`
- `retainFatalError` could just be moved into `RemotingChannelClient.cpp` directly.
- Therefore no configuration is needed from `RemotingTest_Rendering_Win32`, `CreateClientConfiguration` will be no more needed.
- Rename class/file name `RemotingChannelClient` to `RemoteProtocolRendererClient`.

Delete `RemotingRpcConfiguration`
- Configurations could actually be moved from `RemoteViewModelTestShared.h` to `RemotingClient.h`, therefore `CreateConfiguration` can also be deleted. The rest of `RemotingClient.*` could just use constants directly.
- Rename `RemotingClient.(h|cpp)` to `ViewModeoHostClient.(h|cpp)`.
- Rename class `RemotingHostingClient` to `ViewModelHostClient`.

# UPDATES

# TEST [CONFIRMED]

- Build the GacUISrc test solution in Debug|x64 with the repository wrapper. Success requires zero warnings and zero errors.
- Run `CppTest_Rvm /Http` with `RemotingTest_RvmHost /Http`; require the host to declare `rvmt::IViewModel`, the requester automation control tree to expose `Remote View Model Test`, and a live `Translate` result.
- Run `RemotingTest_Core /RVMT /Http`, `RemotingTest_RvmHost /Http`, and `RemotingTest_Rendering_Win32 /Http`; require both automation endpoints to respond, the remote-view-model UI to render, and a live `Translate` result.
- Confirm no Microsoft Visual C++ Runtime Library dialog or stale process remains after either smoke test. Per the request, no unit test executable is required.
- Scan Markdown and all source/build metadata for stale configuration, class, and file names.

CodePack regenerated the ordinary and IncludeOnly `RemotingHelpers*` outputs successfully. The second incremental Debug|x64 solution build succeeded with zero warnings and zero errors after the first build encountered one diagnostic-free, transient `CL.exe` exit in the unrelated `CppTest_Metaonly` project; all requested remoting apps had already compiled and linked in that first build.

Both `/Http` smoke tests passed. With `CppTest_Rvm`, the host declared `rvmt::IViewModel`, the automation tree exposed the expected window and `Hello, !`, simulated typing changed the RPC-backed label to `Hello, Codex!`, and exact `!Exit` was queued. With `RemotingTest_Core /RVMT`, the host again declared the service, both the Core controls endpoint and Win32 renderer DOM exposed the UI, simulated typing changed both views to `Hello, Remote!`, and exact `!Exit` was queued. Core and both requester/renderers exited with code 0; the host exited with its expected requester-disconnection code 1. No runtime dialog or stale process remained. No unit test executable was run, as requested. The stale-name scans passed outside this document's verbatim problem statement and archived historical investigation logs.

# PROPOSALS

- No.1 Specialize the remoting helpers for their test-app consumers [CONFIRMED]

## No.1 Specialize the remoting helpers for their test-app consumers

Remove both configuration structs and make their fixed test-app behavior explicit in the helper implementations. Move the renderer fatal-error prompt policy into the renamed renderer client implementation, move the fixed RVM channel/service constants into the renamed view-model host helper, and update all consumers and build metadata. Use the corrected filename `ViewModelHostClient.(h|cpp)` so it agrees with the requested `ViewModelHostClient` class name.

### CODE CHANGE

- Rename `RemotingChannelClient.(h|cpp)` and `RemotingChannelClient` to `RemoteProtocolRendererClient.(h|cpp)` and `RemoteProtocolRendererClient`.
- Delete `RemotingChannelClientConfiguration` and `CreateClientConfiguration`; pass only the fatal title to the renderer client constructor and keep the platform prompt policy in its implementation.
- Rename `RemotingClient.(h|cpp)` to `ViewModelHostClient.(h|cpp)` and `RemotingHostingClient` to `ViewModelHostClient`.
- Delete `RemotingRpcConfiguration` and `CreateConfiguration`; define the fixed RVM channel, service, ready-message, disconnect-error, and invalid-client constants in the helper header and consume them directly.
- Update C++ consumers, shared-items metadata, generated CodePack outputs, and Markdown references.

### CONFIRMED

`RemoteProtocolRendererClient` now receives only its fatal-error title; its platform-specific retain/close prompt is implemented directly in the helper, so the renderer test app has no configuration factory. `ViewModelHostClient` and `RemotingRequesterSession` consume fixed RVM constants declared in `ViewModelHostClient.h`, eliminating the RPC configuration object and factory. All C++ consumers, shared-items inventories, CodePack output, IncludeOnly paths, and live Markdown references use the new names. The clean Debug|x64 build and both live-RPC smoke-test topologies confirm that the test-only specialization preserves requester/host and Core/host/renderer behavior.
