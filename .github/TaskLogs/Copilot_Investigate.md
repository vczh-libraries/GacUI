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
