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

- No.1 Fold the requester runtime into the specialized RVM channel server [CONFIRMED]

## No.1 Fold the requester runtime into the specialized RVM channel server

Rename `RemotingRequesterSession` to `RpcServerHelpers` and use it as a protected implementation base of `RemoteViewModelChannelServer<TServerBase>`. Keep the state machine, RPC participants, task queue, and shutdown sequencing implemented in `ViewModelHostServer.cpp`, while making the specialized server the only public application-facing object. Move the `RequesterPhase` definition into the header and make `TaskQueueThread`, `RpcBroadcastingLocalClient`, and `RpcServiceAccessLocalClient` namespace-level implementation types instead of nested classes.

Override the channel server's virtual `Start()` and `Stop()` methods in `RemoteViewModelChannelServer<TServerBase>`. `Start()` will start only the transport so `RemotingTest_Core` can retain the required GacUI local-client ID 0 before any RVM helper client connects. `RequestService()` will then prepare the broadcasting client and broker, connect and register the service-access client, and start the task queue only after both RPC identities are initialized. Keeping those helper connections contiguous preserves both the fixed Core client-ID contract and the existing requirement that queued broker traffic cannot race requester identity initialization. `Stop()` will let `RpcServerHelpers` finalize RPC on its task queue, stop the transport at the existing shutdown boundary, exit the task queue when it was started, and join its thread. Move `FinalizeRpcOnTaskQueue` into `RpcServerHelpers` as a private trivial method. This preserves the established transport/RPC shutdown order without making callers start or stop two coupled objects.

Expose `RemoteViewModelChannelServer<TServerBase>::RequestService(const WString& typeName)` as the additional RVM-specific operation, forwarding into the protected helper implementation. Service acquisition will complete the requester startup phase so renderer admission becomes available, while `Stop()` owns the stopping transition. Remove `GetSession()` and all session pointers from the GUI contexts. `CppTest_Rvm` will call the specialized server directly; generic `RemotingTest_Core` code will cast its `RemotingChannelServer<TServerBase>&` to the specialized server only in `/RVMT` mode.

Delete `ViewModelServiceName`. Pass `L"rvmt::IViewModel"` directly to `RequestService` in `CppTest_Rvm` and `RemotingTest_Core`, and directly to `GetTypeIdFromName` in `RemotingTest_RvmHost`. Reconcile the RVM ownership and startup guidance in `Project.md` and `.github/Learning/Learning_Coding.md` with the new API boundary.

### CODE CHANGE

- Replaced the public `RemotingRequesterSession` object with `RpcServerHelpers`, used as a protected implementation base of `RemoteViewModelChannelServer<TServerBase>`. Moved the complete `RequesterPhase` definition into the header and made `TaskQueueThread`, `RpcBroadcastingLocalClient`, and `RpcServiceAccessLocalClient` namespace-level implementation types.
- Made the specialized channel server the only application-facing lifetime. Its `Start()` starts the transport, `RequestService(typeName)` initializes both RPC local clients and completes the running transition, and `Stop()` finalizes RPC on the helper task queue around the base transport stop. Helper shutdown now also handles a server that was started but never requested a service.
- Preserved both startup-order contracts discovered during live verification: Core connects its fixed GacUI local client as ID 0 before the RVM local clients, while the requester task queue starts only after the service-access dispatcher has its local and server identities and has been registered with the broker.
- Removed `GetSession()` and all requester-session fields and lifecycle calls from `CppTest_Rvm` and `RemotingTest_Core`. Both applications request `L"rvmt::IViewModel"` from the specialized server, and Core uses its existing `/RVMT` discriminator for the narrow cast. Both now stop through the channel server's virtual `Stop()`.
- Deleted `ViewModelServiceName`; the two requesters pass `L"rvmt::IViewModel"` to `RequestService`, and `RemotingTest_RvmHost` passes it directly to `GetTypeIdFromName`.
- Updated `Project.md` and `.github/Learning/Learning_Coding.md` to describe the combined server/helper ownership, direct service type-name boundary, and renderer admission after service acquisition.

### CONFIRMED

The refactor proposal is confirmed by source inspection, build, and both requested application shapes:

- Static searches find the requested helper/type names and the protected helper inheritance, and find no `RemotingRequesterSession`, `GetSession(`, or `ViewModelServiceName` in `Test`. The direct `L"rvmt::IViewModel"` arguments occur at both requester boundaries and the host type lookup.
- `Test/GacUISrc/GacUISrc.sln` builds through `copilotBuild.ps1` in Debug x64 with `0 Warning(s)` and `0 Error(s)`. Per the explicit task instruction, `UnitTest` and unrelated applications were not run.
- `CppTest_Rvm /Http` plus `RemotingTest_RvmHost /Http` exposed the exact initial `Hello, !`, accepted editor input, produced exactly `Hello, CPPFINAL26!`, and shut down without a surviving target process.
- `RemotingTest_Core /Http /RVMT` plus the host exposed a 16,475-byte Core `Controls` tree before the renderer was launched. `RemotingTest_Rendering_Win32 /Http /port:8890` then exposed a 26,210-byte `Dom`. Core-side IO produced exactly `Hello, COREIMPLEMENT26!` in both surfaces, the renderer DOM contained no `fatalError`, Core-authored `!Exit` shut down all three processes, and no target process survived.

The requested renderer-side `/Http` input/exit subcheck is not green, but a detached pre-refactor comparison at `ebc8bca6d` proves that failure is unchanged and outside this proposal. In both the refactored build and the baseline build, renderer `/IO` accepted `!LeftClick` and `!Type` without changing Core `Controls` or renderer `Dom`; renderer `!Exit` also left all processes running. In the same baseline session, Core `/IO` immediately produced exactly `Hello, BASECORE26!` in both surfaces. The detached clean-build wrapper reached its six-minute bound after the three target executables had linked; those linked baseline executables were the ones used for the comparison. The temporary worktree and every comparison process were removed afterward.
