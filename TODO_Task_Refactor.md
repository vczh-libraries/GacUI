# Refactor RVM Requester Composition

Commit `e1926f204` completed the stdio RVM host transport. Apply the following
focused cleanup without changing the transport, RPC protocol, renderer
behavior, or command-line contract.

## Investigation Result

Both proposed refactors are valid.

- `TerminatingViewModel` in `Test/GacUISrc/CppTest_Rvm/GuiMain.cpp` is
  unnecessary. Host loss is already injected into the requester as a
  `rpc_controller::RpcInjectedException`; the blocked or next
  `rvmt::IViewModel::Translate` call throws it. `CppTest_Rvm` is a test app, so
  letting that exception remain unhandled and crash the process is the desired
  fail-fast behavior.
- The free `RequestViewModel` helper in
  `Test/GacUISrc/RemotingTest_Core/GuiMain.cpp` has two mutually exclusive
  source call sites, but the `requestViewModel` callback passed to `StartServer`
  is invoked exactly once in each RVM run. The acquisition sequence can execute
  directly in `StartServer` through a typed RVM-server argument or a tightly
  scoped overload; a one-shot callback returning the view model is not needed.

## Required Changes

### Remove `TerminatingViewModel` from `CppTest_Rvm`

In `Test/GacUISrc/CppTest_Rvm/GuiMain.cpp`:

- Remove the `<cstdlib>` include, `RvmGuiContext::fatalHostLoss`, the complete
  `TerminatingViewModel` class, and the final `std::_Exit(1)` branch.
- Store the acquired `rvmt::IViewModel` proxy directly in `RvmGuiContext`, as
  the code did before `e1926f204`.
- Do not replace the wrapper with another `try`/`catch`, window-close adapter,
  flag, exit-code shim, or recovery path. If a UI-time
  `rvmt::IViewModel::Translate` call throws after host loss, let the exception
  escape and crash `CppTest_Rvm`.
- Keep normal application shutdown unchanged. A normal close still reaches
  `server.Stop()`, whose existing stopping phase prevents the requested host
  shutdown from being reclassified as host loss.
- Do not change the two startup `Translate` checks. They already exercise the
  real proxy directly.

This cleanup applies to standalone `/Pipe`, `/Http`, `/MiniHttp`, and `/Cli`
modes. A Windows runtime-error dialog during the deliberate crash test is an
observable crash signal, not a reason to restore graceful recovery; handle the
dialog according to the repository's computer-use guidance.

### Inline Core view-model acquisition into `StartServer`

In `Test/GacUISrc/RemotingTest_Core/GuiMain.cpp`:

- Delete the free `RequestViewModel` function.
- Remove the `Func<Ptr<rvmt::IViewModel>()> requestViewModel` parameter and its
  two wrapper lambdas. Remove the one-shot `stopRvmServer` callback as well;
  typed RVM-server ownership should express whether `StartServer` must stop a
  separate server.
- Give `StartServer`, or a small typed overload around it, direct access to the
  applicable `RemoteViewModelChannelServer` and whether that RVM server is the
  separately owned stdio server. An optional synchronous callback may remain
  solely for `ConnectNewClient(command)` in `/Cli` mode.
- Perform the view-model acquisition sequence directly inside the existing
  `StartServer` exception boundary:
  1. Start the separate stdio RVM server only in `/Cli` mode.
  2. Create the required-service list containing exactly
     `rvmt::IViewModel`.
  3. Call the RVM server's `Connect`.
  4. In `/Cli` mode, call `ConnectNewClient` only after `Connect` has made host
     admission ready.
  5. Call `RemoteViewModelTestInitialize::InitializeRpc` with the returned
     requester client ID.
  6. Request and cast `rvmt::IViewModel` into `CoreGuiContext::viewModel`.
- Preserve the complete startup order:
  renderer server start, Core local client registration as
  `GacUIRemoteProtocolCoreClientId == 1`, optional separate stdio RVM server
  start, RVM `Connect`, optional child launch, generated RPC initialization,
  service request, and remote controller setup.
- Preserve both topologies:
  - Non-CLI `/RVMT` uses the same `RemoteViewModelChannelServer` for renderer
    and host traffic. Do not start or stop it twice.
  - `/Cli` uses a renderer-only server plus a separate
    `RemoteViewModelChannelServer<StdioRedirectionServer>`. Clear Core channels
    on the renderer server, stop the RVM server first, and then stop the
    renderer server.
- Skip all RVM setup for `/FCT` and `/RPT`. A compile-time no-RVM template path
  or a clear overload is preferred over a dummy server.
- Do not restore the pre-`e1926f204` `dynamic_cast` assumption that the renderer
  server is always the RVM server; it is false for the split `/Cli` topology.
- Keep Core's current exception boundary and one-shot
  `CoreGuiContext::broadcastFatalError`. Unlike standalone `CppTest_Rvm`, Core
  must send its exact Core-authored fatal `ErrorChannel` package to the active
  renderer before terminating.

## Documentation Contract

Keep the accompanying `DebugRemoteProtocolSop.md` clarification: a nonzero
crash from an unhandled `rpc_controller::RpcInjectedException` is the required
direct termination for `CppTest_Rvm`. Do not change the SOP back to requiring
an application-layer fatal handler. Core's distinct fatal-package requirement
remains unchanged.

## Out of Scope

- No changes to `Test/RemotingHelpers/Rvmt`, stdio framing or process ownership,
  generated RemoteViewModelTest files, reflection, project inventories, release
  CodePack output, command-line parsing, renderer behavior, or test matrices.
- Do not add retry, reconnect, graceful recovery, a shutdown acknowledgement,
  a heartbeat, or defensive cleanup for a crashing test app.
- Do not modify wGac or iGac.

## Verification

- Run `git diff --check` and confirm that `TerminatingViewModel`,
  `fatalHostLoss`, the CppTest-only `std::_Exit(1)`, the free
  `RequestViewModel`, and the one-shot `requestViewModel` callback are gone.
  Confirm that no replacement catch/recovery wrapper was added.
- Build `Test/GacUISrc/GacUISrc.sln` as Debug x64 and Debug Win32 through
  `.github/Scripts/copilotBuild.ps1`. Inspect `.github/Scripts/Build.log` for a
  successful build with zero warnings and zero errors.
- Run the existing Debug x64 `UnitTest` through
  `.github/Scripts/copilotExecute.ps1`. Require the complete passing summary and
  no memory-leak dump. No new unit test or generated file is required for this
  test-app-only refactor.
- For standalone `CppTest_Rvm`, exercise `/Pipe`, `/Http`, `/MiniHttp`, and
  `/Cli` with the matching host setup from the native-renderer guide. In every
  mode, require successful service acquisition, exact `Hello, <marker>!`
  translation, and clean application-controlled shutdown with no orphan host or
  stale listener.
- Repeat standalone host-loss verification in every mode. After one successful
  translation, force-terminate the accepted host and trigger an in-flight or
  next `Translate` as appropriate for the transport. Require `CppTest_Rvm` to
  terminate nonzero from the unhandled `rpc_controller::RpcInjectedException`,
  with no retry, recovery, or graceful-close adapter. If a Windows crash/runtime
  dialog appears, record and dismiss it, and leave no process or listener
  behind.
- For `RemotingTest_Core /RVMT`, exercise the combined manual-host topology over
  `/Pipe`, `/Http`, and `/MiniHttp`, and the split `/Cli:<host-path>` topology
  with each of those three renderer transports. Require exact
  `Hello, <marker>!` translation and clean shutdown in every row.
- Force host loss in the Core rows according to `DebugRemoteProtocolSop.md`.
  Require exactly one Core-authored `ErrorChannel` package containing
  `RemotingTest_RvmHost disconnected.`, renderer-visible fatal state, and a
  nonzero Core exit. This proves that inlining did not move acquisition outside
  Core's exception/fatal-delivery boundary.
- Smoke-test at least one `/FCT` and one `/RPT` Core run to prove that the
  no-RVM `StartServer` path still starts, operates, and stops without attempting
  view-model setup.
- Launch and operate test applications only through the repository's documented
  execution wrappers. Bound waits and confirm that no requester, Core, host,
  renderer, listener, native prompt, or crash dialog remains after each run.
