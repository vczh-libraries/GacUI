# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Perform the following refactor on the design in `Test/RemotingHelpers/Rvmt` and affected test apps.

- Move `RequesterPhrase` to header.
- `RemotingRequesterSession::TaskQueueThread|BroadcastingLocalClient|RequesterLocalClient` could be moved to parallel level types.
- `RemoteViewModelChannelServer` inherits from `RemotingRequesterSession` protectedly, to keep more functions in cpp files:
  - Delete `GetSession` method.
    - `RemotingTest_Core` and `CppTest_Rvm` only needs `Start`, `Stop` and `RequestService`.
    - `Start` and `Stop` are virtual functions, they could be overrided in `RemoteViewModelChannelServer`. Therefore no need to call `Start` and `Stop` for eachclass separately.
    - `RequestService` is an extra function, `RemotingTest_Core` could cast from `RemotingChannelServer` to `RemoteViewModelChannelServer` to use it.
  - `RequestService` will be moved to `RemoteViewModelChannelServer` so nothing from `RemotingRequesterSession` needs to expose for users.
    - Delete `ViewModelServiceName` constant.
    - A type name `WString` argument will be added to `RequestService`.
    - Both `RemotingTest_Core` and `CppTest_Rvm` will call `RequestService` with a string literal directly.
  - `FinalizeRpcOnTaskQueue` moved into `RemotingRequesterSession` as trivial method.
- Rename:
  - `RemotingRequesterSession` -> `RpcServerHelpers`
  - `BroadcastingLocalClient` -> `RpcBroadcastingLocalClient`
  - `RequesterLocalClient` -> `RpcServiceAccessLocalClient`

To verify, follow `DebugRemoteProtocolWithNativeRenderer.md` to cover `CppTest_Rvm`, and `RemotingTest_(Core|Renderer_Win32)`, with only `/Http` and `/RVMT` only.
No need to run unit test and other test apps.

# UPDATES

# TEST [CONFIRMED]

- Confirm the current design problem structurally in `Test/RemotingHelpers/Rvmt` and its callers:
  - `RequesterPhase` is only forward-declared in the header and defined in the implementation file.
  - `TaskQueueThread`, `BroadcastingLocalClient`, and `RequesterLocalClient` are nested implementation types of `RemotingRequesterSession`.
  - `RemoteViewModelChannelServer` owns a separate `RemotingRequesterSession`, exposes it through `GetSession`, and makes both test applications coordinate the server and session lifecycle separately.
  - `ViewModelServiceName` is a shared fixed constant rather than a type-name argument supplied at the service-access boundary.
- After the refactor, use source inspection to require the requested type names, protected helper inheritance, single public server lifecycle, direct service type-name arguments, and absence of `GetSession`, `RemotingRequesterSession`, and `ViewModelServiceName` references.
- Build `Test/GacUISrc/GacUISrc.sln` in the default Debug x64 configuration through `copilotBuild.ps1`; require zero build errors.
- Run only the requested `/Http` and `/RVMT` application flows; do not run the unit test or unrelated test applications:
  - Start `CppTest_Rvm /Http`, then `RemotingTest_RvmHost /Http`. Require the `Remote View Model Test` window, type a unique marker into its single-line editor, and require the greeting to become exactly `Hello, <marker>!` before clean application-controlled shutdown.
  - Start `RemotingTest_Core /Http /RVMT`, then `RemotingTest_RvmHost /Http`, and start `RemotingTest_Rendering_Win32 /Http` only after Core exposes the `Remote View Model Test` window. Require nonempty Core `Controls` and renderer `Dom`, drive the editor through renderer-side `IO`, require the exact translated greeting in both surfaces, and close through renderer-side `IO` without a fatal prompt or surviving process.

The structural reproduction is confirmed: the current sources contain every coupling and symbol listed in the first group, so the acceptance checks distinguish the requested refactor from the existing design.

# PROPOSALS
