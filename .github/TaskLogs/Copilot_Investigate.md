# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Perform the following refactor on the design in `Source/RemotingHelpers` and test apps.

1) Interface re-organization

I notice that `CreateCallbacks` functions in test apps (meaning `CppTest_Rvm` and `RemotingTest_*` in this task) is creating incorrect patterns of the code.
They are just wired code simulating interface and inheritance.
`RemotingChannelServerCallbacks` should actually be a list of virtual function in `RemotingChannelServer`, and test apps calling `CreateCallbacks` should instead inherit from this class to implement these functions.

When implementing these virtual functions, do not introduce too many unnecessary layers of redirection like the current only.
Currently the server calls `callbacks`, which redirects to `RemoteViewModelRequesterSession` by `CreateCallbacks`, which redirects to `RemotingRequesterSession`, but the only differences between `RemotingTest_Core` and `CppTest_Rvm` is just about wanting to accept renderer clients or not. I would say just:
- Just inline everything to `RemotingChannelServer`, removing unnecessary virtual functions during translating `RemotingChannelServerCallbacks`
- `acceptRenderer` as a constructor argument is already enough.

Example of reason to not make certain virtual functions:
- `canAcceptLocalClient` should be removed. As `ConnectLocalClient` is called by the process creating the server, if a local client cannot be accepted, just don't call the `ConnectLocalClient` instead of creating a barrier. This is unnecessary dynamic check that can be done by static analysis (e.g. type checking, or review the code more carefully).
- `canAdmitRenderer` is just monitoring a state. It is acceptable to have a dedicated function, but since this is not supposed to be a behavior which can be modified, making it virtual does not make sense.
It might ends up no or just a few virtual fucntions survive, which is acceptable.

After doing this, `Start*Server` in `CppTest_Rvm` and `RemotingTest_Core` no longer need to pass constructor arguments `StartServer<>`, as there will be no objedts to create before starting the server. The only exception might be the `parser`, it is acceptable to create them in each `Start*Server` as this is just single line, and pass `parser` via argument to `StartServer<>`.

2) MISC

`RemotingCoreChannel` should rename. The original class `GuiRemoteProtocolCoreChannel` actually says, this is a channel for `GuiRemoteProtocol`, and it is for the "core" side. Naming its sub class `RemotingCoreChannel` is just making people confusing. Since the only purpose of making this class is to add a `getTransportRendererClientId` guard:
- Turn `getTransportRendererClientId` to be a virtual function in the base class `IsCorrectRendererClientId`, returning true by default.
- `RemotingTest_Core` is going to inherit this class and name it after `SwitchableRenderersCoreChannel`.
- Delete `RemotingCoreChannel`.

# UPDATES

## UPDATE

you need to run remote protocol test apps verification, although the task did not include this, to make sure your change is valid. Since this is a refactoring instead of feature work, and the verification was done before, any breaking will because of your work.

## UPDATE

there is a native error dialog blocking the execution. That is what you need to pay attention to, as you might need to constantly pick and see if the UI is blocked by such thing.

## UPDATE

Please also update [Running-GacUI.md](.github/Guidelines/Running-GacUI.md) (but not in this repo but in Tools repo) to talk about it.

## UPDATE

The think the best way to check is the "Microsoft Visual C++ Runtime Library" title. You accidentally stopped, please continue to finish `investigate repro [TODO_Task.md](TODO_Task.md) `, not to start a new task but to continue working on it, with the current [Copilot_Investigate.md](.github/TaskLogs/Copilot_Investigate.md)

# TEST [CONFIRMED]

The problem is an architectural source-shape regression rather than a missing runtime behavior. It is confirmed by the current implementation:

- `Source/RemotingHelpers/RemotingServer/RemotingChannelServer.h` declares `RemotingChannelServerCallbacks`, stores it in `RemotingChannelServer<TServerBase>`, dynamically checks `canAcceptLocalClient` and `canAdmitRenderer`, and contains `RemotingCoreChannel` solely to compare `GetRendererClientId()` with `getTransportRendererClientId()`.
- `Test/GacUISrc/CppTest_Rvm/GuiMain.cpp` and `Test/GacUISrc/RemotingTest_Core/GuiMain.cpp` each declare `CreateCallbacks`. Their callbacks only recognize the RVM-host channel and forward admission/disconnection operations through `RemoteViewModelRequesterSession` into `RemotingRequesterSession`.
- Both applications forward transport constructor arguments through variadic `StartServer<TServer, TArgs...>` functions rather than constructing the concrete channel server in `StartNamedPipeServer`, `StartHttpServer`, or `StartMiniHttpServer`.

The refactor succeeds when all of the following are true:

- Static inspection finds no `RemotingChannelServerCallbacks`, `CreateCallbacks`, or `RemotingCoreChannel`; local clients are accepted without a dynamic callback barrier; renderer admission is concrete state rather than an overridable policy; and only genuine remote-client admission/disconnection extension points survive on `RemotingChannelServer<TServerBase>`.
- `GuiRemoteProtocolCoreChannel::IsCorrectRendererClientId(vint)` returns `true` by default and is used by `Submit`; `RemotingTest_Core` supplies `SwitchableRenderersCoreChannel` to compare the protocol renderer with the transport server's current renderer.
- Debug x64 `GacUISrc.sln` builds with zero warnings and errors, and the complete Debug x64 `UnitTest` run passes without a memory-leak dump.
- The real remote-protocol applications are exercised after the refactor, not merely compiled. For `/Pipe`, `/Http`, and `/MiniHttp`, `RemotingTest_Core /RPT` with `RemotingTest_Rendering_Win32` must expose healthy Core `Controls` and renderer `Dom`, accept representative IO, preserve renderer replacement/state continuity, and shut down normally without a fatal prompt or surviving process.
- The affected RVM path is exercised after the refactor. For `/Pipe`, `/Http`, and `/MiniHttp`, `CppTest_Rvm` with `RemotingTest_RvmHost` must acquire `rvmt::IViewModel`, complete a live `Translate`, accept exact `!Exit`, and leave no surviving process. `RemotingTest_Core /RVMT` with `RemotingTest_RvmHost` and the native renderer must acquire the same service, render a healthy window, complete a live `Translate`, and shut down normally.
- `git diff --check` is clean, and static searches confirm that the refactor leaves no stale callback adapter or misleading channel class behind.

The pre-change behavioral baseline does not need to be rerun: the user confirms that the remote-protocol verification passed before this refactor, so any post-change breakage is attributable to this work.

# PROPOSALS

- No.1 Replace callback bundles with server inheritance and move the renderer guard to the protocol channel [CONFIRMED]

## No.1 Replace callback bundles with server inheritance and move the renderer guard to the protocol channel

Delete `RemotingChannelServerCallbacks`. Make `RemotingChannelServer<TServerBase>` accept every local client because only the owning process can call `ConnectLocalClient`. Give it one protected virtual remote-admission operation whose default implementation owns the existing renderer acceptance/replacement behavior. A generated-app-specific `RemoteViewModelChannelServer<TServerBase>` will override that operation to admit the exact RVM-host channel and consult the concrete requester session phase before delegating renderer channels to the base. It will override the already-virtual disconnect notification to update the requester session after the base has handled renderer detachment. No virtual `canAcceptLocalClient`, channel-recognition predicate, renderer-admission state query, or renderer-change notification will be introduced.

Make `RemoteViewModelRequesterSession` directly inherit `RemotingRequesterSession` and retain only generated-module construction plus the typed `rvmt::IViewModel` result. `RemoteViewModelChannelServer<TServerBase>` will own that session, so both `CppTest_Rvm` and `RemotingTest_Core /RVMT` use the same server class; the existing `acceptRenderer` constructor argument remains their only server-policy difference. Its terminal action will broadcast the RVM-host failure only when renderer support is enabled, then fail fast. This removes the callback-to-wrapper-to-session redirection while preserving the post-`Ready` renderer admission barrier.

Construct each named-pipe, Windows HTTP, or MiniHTTP concrete channel server in its corresponding `Start*Server` function. Pass the constructed server and JSON parser into the shared run function instead of forwarding a variadic transport-constructor argument pack through `StartServer<TServerBase>`.

Add protected virtual `GuiRemoteProtocolCoreChannel::IsCorrectRendererClientId(vint)`, returning `true` by default, and have `Submit` evaluate it before the existing default `-1` handling. Delete `RemotingCoreChannel`. Define `SwitchableRenderersCoreChannel<TServerBase>` in `RemotingTest_Core`; it will override the guard, reject `-1`, and compare the protocol channel's selected renderer with `RemotingChannelServer<TServerBase>::GetRendererClientId()`.

Update the remote-protocol renderer/serialization design document so its demo composition, server naming, renderer replacement, transport modes, and shutdown description match the resulting implementation.

### CODE CHANGE

- Removed `RemotingChannelServerCallbacks` and its six optional delegates. `RemotingChannelServer<TServerBase>` now accepts caller-created local clients directly and exposes only `OnRemoteClientConnected`, whose default implementation owns renderer admission and replacement. Derived servers use the existing virtual `OnClientDisconnected` when they have additional remote-client state.
- Removed `RemotingRequesterSession::CanAcceptLocalClient`; its two internal local clients are connected only by `RemotingRequesterSession::Start` in the required broker-then-requester order.
- Changed `RemoteViewModelRequesterSession` from a forwarding owner into a direct `RemotingRequesterSession` subclass with only generated RPC construction and typed service acquisition. Added `RemoteViewModelChannelServer<TServerBase>`, which owns the session, admits the exact RVM host, checks the concrete running phase for renderer admission, forwards disconnection to the session, and selects fatal broadcast solely from `acceptRenderer`.
- Reworked `CppTest_Rvm` and `RemotingTest_Core` so each transport entry point constructs its concrete server before calling the shared run function. Removed both `CreateCallbacks` functions, the variadic transport-constructor forwarding, the Core terminal server pointer, and the edited files' anonymous namespaces.
- Added default-true `GuiRemoteProtocolCoreChannel::IsCorrectRendererClientId(vint)` and applied it before the base channel's default no-renderer branch in `Submit`. Deleted `RemotingCoreChannel`; `RemotingTest_Core` now owns `SwitchableRenderersCoreChannel<TServerBase>`, rejects renderer id `-1`, and directly compares other ids against its channel server.
- Updated the GacUI remote renderer/serialization design document and its index entry to describe the current server inheritance, RVM admission barrier, switchable renderer guard, all three transports, and shared renderer client.

### CONFIRMED

Static review confirms that the callback-shaped abstraction and its forwarding layers are gone: there are no remaining `RemotingChannelServerCallbacks`, `CreateCallbacks`, `RemotingCoreChannel`, or `CanAcceptLocalClient` symbols in the affected source and applications. Local clients are accepted directly, `RemotingChannelServer<TServerBase>` owns the normal renderer behavior, and `RemoteViewModelChannelServer<TServerBase>` adds only the RVM-host admission and requester-session lifecycle required by the generated RVM applications. Each transport entry point now constructs its concrete server and passes it to the shared run function.

The required executable-level verification found an implementation regression that compilation and the first unit-test pass did not expose. The first guard implementation performed the base channel's `receiverClientId == -1` shortcut before calling `IsCorrectRendererClientId`. During Core startup this reported `disconnected = false` for absent responses, allowing `GuiRemoteProtocolFilter::ProcessResponses` to process missing results and throw. Moving the virtual guard before the shortcut and having `SwitchableRenderersCoreChannel` reject `-1` restores the old switchable-channel contract without changing the default channel contract. A debugger confirmed that a separate native runtime dialog encountered during the first automation probe came from sending the wrong HTTP content type to `HttpServerApi::GetUtf8Body`; all subsequent probes used the required `application/json; charset=utf8` and continuously checked for a top-level window titled exactly `Microsoft Visual C++ Runtime Library`.

The post-fix Debug x64 solution build completed with zero warnings and errors. The complete Debug x64 `UnitTest` suite passed 89/89 files and 1714/1714 cases without a memory-leak dump. For each of `/Pipe`, `/Http`, and `/MiniHttp`, `CppTest_Rvm` acquired the service from `RemotingTest_RvmHost`, completed a live `Translate`, accepted exact `!Exit`, and left no process behind. For the same three transports, `RemotingTest_Core /RPT` and `RemotingTest_Rendering_Win32` exposed healthy automation trees, processed a real button click, replaced a connected renderer while preserving the changed UI state, and shut down cleanly. Finally, `RemotingTest_Core /RVMT`, `RemotingTest_RvmHost`, and the native renderer completed live translations and clean shutdown for all three transports. Every verification loop checked for the exact runtime-dialog title, and none appeared after the corrected HTTP probe. Static searches and `git diff --check` are clean. The canonical Tools-repository `Copilot/Guidelines/Running-GacUI.md` now documents both repeated exact-title checks and the required automation POST content type.
