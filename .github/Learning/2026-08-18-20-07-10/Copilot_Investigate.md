# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

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

# UPDATES

# TEST [CONFIRMED]

The problem is a structural regression with executable-boundary consequences,
so it is confirmed by source inspection plus the existing build and runtime
coverage rather than by adding a new unit-test-only seam.

- Source inspection confirms that `CppTest_Rvm/GuiMain.cpp` owns the complete
  `TerminatingViewModel` catch/close wrapper, its `fatalHostLoss` flag, and the
  post-loop `std::_Exit(1)`. The real view-model proxy is already used by both
  startup `Translate` checks, and `ViewModelHostServer.cpp` already injects
  `RemoteViewModelHostDisconnectedError` into the requester dispatcher.
- Source inspection confirms that `RemotingTest_Core/GuiMain.cpp` invokes its
  `requestViewModel` callback once inside `StartServer`, while two mutually
  exclusive caller branches only reconstruct the same Connect/initialize/
  request sequence through `RequestViewModel`.
- A baseline Debug x64 build of `Test/GacUISrc/GacUISrc.sln` passed with zero
  warnings and zero errors. The baseline Debug x64 `UnitTest` run passed all
  88 files and all 1713 cases with no memory-leak dump. This establishes that
  the requested work is a focused composition cleanup, not a repair for an
  existing build or unit-test failure.

Success after the change requires the obsolete symbols and recovery path to be
absent, Debug x64 and Win32 builds to stay warning/error free, all existing unit
tests to pass without leaks, and the documented standalone/Core transport
matrices to preserve service acquisition, translation, shutdown, and distinct
host-loss behavior.

# PROPOSALS

- No.1 Remove the requester wrappers and make Core acquisition type-directed [CONFIRMED]

## No.1 Remove the requester wrappers and make Core acquisition type-directed

Use the real `rvmt::IViewModel` proxy directly in standalone `CppTest_Rvm`.
Deleting the local wrapper lets the already-injected
`rpc_controller::RpcInjectedException` escape the UI-time `Translate` call and
reach the existing process-terminating boundary, while normal GUI shutdown
still calls `server.Stop()` in the same order.

Generalize Core's existing `StartServer` with an optional typed RVM-server
pointer whose template argument defaults to `void`. A compile-time no-RVM path
keeps `/FCT` and `/RPT` free of RVM operations. For `/RVMT`, acquire the service
directly inside the existing exception boundary. The server types also encode
lifetime ownership: the combined topology has identical renderer/RVM server
types and therefore uses the already-started server once, while split `/Cli`
has different server types and starts/stops its stdio RVM server inside
`StartServer`. Keep only the optional `ConnectNewClient(command)` callback,
after `Connect`, because launching the child is the one operation that is
specific to `/Cli` composition.

This preserves Core client id 1, renderer setup, error broadcasting, shared
server single-start/single-stop behavior, split-server reverse shutdown, and
the generated RPC initialization order without a `dynamic_cast`, dummy RVM
server, request callback, or stop callback.

### CODE CHANGE

- In `CppTest_Rvm/GuiMain.cpp`, remove `<cstdlib>`, `fatalHostLoss`,
  `TerminatingViewModel`, proxy wrapping, and the final `_Exit`; initialize
  `RvmGuiContext` with the acquired proxy directly.
- In `RemotingTest_Core/GuiMain.cpp`, remove `RequestViewModel` and both
  one-shot callbacks. Add `<type_traits>` and make `StartServer` perform typed
  RVM start/connect/launch/InitializeRpc/RequestService and typed split-server
  stop operations with `if constexpr`; update the three caller branches to
  pass the real RVM server and, only for `/Cli`, the child-launch callback.

### CONFIRMED

The implementation matches the proposal. Standalone `CppTest_Rvm` now stores
the generated `rvmt::IViewModel` proxy directly, so the existing requester
dispatcher remains the only host-loss mechanism and no application-layer
catch, window-close adapter, exit flag, or `_Exit` shim remains. Core now runs
the required-service list, `Connect`, optional child launch, generated RPC
initialization, and service request directly inside `StartServer`'s existing
exception boundary. Template types distinguish the combined server from the
split stdio RVM server, preserving single start/stop for the former and
RVM-before-renderer shutdown for the latter. The compile-time `void` path
performs no view-model work for `/FCT` or `/RPT`.

Verification completed as follows:

- `git diff --check` passed, and source searches confirmed that
  `TerminatingViewModel`, `fatalHostLoss`, the CppTest `_Exit`,
  `RequestViewModel`, `requestViewModel`, and `stopRvmServer` are absent from
  the changed applications. No replacement recovery wrapper was introduced.
- Post-change Debug x64 and Debug Win32 full-solution builds completed with
  zero warnings and zero errors. The final Debug x64 `UnitTest` run passed all
  88 files and all 1713 cases with no memory-leak dump. An initial post-change
  run encountered a transient snapshot-file write failure after generated
  snapshot cleanup; an unchanged rerun completed fully, and the generated
  locale-sensitive snapshots were restored out of the working tree.
- Standalone normal runs over `/Pipe`, `/Http`, `/MiniHttp`, and `/Cli`
  acquired the service, produced the exact `Hello, <marker>!` result, and shut
  down without leftovers. Forced host loss in every mode made the next RPC
  show the expected Windows debug-runtime crash signal and terminate
  `CppTest_Rvm` nonzero; each dialog was dismissed and no retry or recovery
  occurred.
- Core `/RVMT` normal and forced-loss runs covered combined manual-host and
  split `/Cli` topologies with `/Pipe`, `/Http`, and `/MiniHttp` renderer
  transports. Every normal row produced the exact translated marker and shut
  down cleanly. Every forced-loss row exited Core nonzero and displayed the
  exact `RemotingTest_RvmHost disconnected.` prompt; choosing `No` left a DOM
  whose `fatalError` contained that message exactly once, after which the
  renderer accepted `!Exit` and ended cleanly.
- No-RVM `/Pipe /FCT` and `/Http /RPT` smoke runs started and accepted input
  without launching a view-model host, then both Core and renderer processes
  exited with code zero. The final process and listener audit found no
  requester, Core, host, renderer, native prompt, crash dialog, or listeners
  on ports 8888 or 8890.

These results confirm both requested simplifications while retaining Core's
distinct one-shot renderer fatal delivery and all current transport
topologies.
