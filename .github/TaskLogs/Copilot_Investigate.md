# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Implement the current host-loss and `ViewModelHostClient` refactor specified in `TODO_Task.md`.

Plus check `DebugRemoteProtocolSop.md` to see if `Fatal Error` clicking and force terminating `RemotingTest_RvmHost` test is described in that SOP document or not and make sure it has. Commit and push all local changes on affected repos.

# UPDATES

# TEST [CONFIRMED]

- Confirm the current Workflow dispatcher cannot represent a persistent injected failure and can leave an `OnJsonRequest` or dispatcher startup wait blocked after the accepted RVM host disappears.
- Add deterministic Workflow `LibraryTest` coverage for persistent last-write-wins injection, empty messages, repeated ordinary and `BroadcastAndDrop` calls, wake-up of an in-flight response wait on its caller thread, response-versus-injection ordering, nested request checkpoints, startup waits, and the direct dispatcher implementation.
- Regenerate and verify Workflow reflection baselines, generated RPC code, TypeScript RPC output, and the released `VlppWorkflowLibrary` pair through the repository workflows.
- In GacUI, statically verify that only accepted-host loss outside normal stopping injects `RemotingTest_RvmHost disconnected.` outside server locks, startup loss is latched until the dispatcher exists, `CppTest_Rvm` leaves the exception uncaught, and Core converts `vl::Exception` and `vl::Error` through one shared fatal boundary while its channels are alive.
- Verify `ViewModelHostClient` has no nested `Impl` and preserves the existing connection, initialization, ready-barrier, and shutdown ordering with direct fields.
- Build GacUI in Debug x64 and Win32, regenerate affected reflection metadata through the prescribed projects, and run `UnitTest` with no failures or memory leaks.
- Exercise the two explicit SOP regressions through the native renderer over `/Pipe`: `/RPT` clicking `Fatal Error` must deliver the exact Core-authored `This is a fatel error!`; `/RVMT` must first complete a `Translate`, then force-terminate only `RemotingTest_RvmHost`, trigger a real RPC if idle, deliver exactly one Core-authored `RemotingTest_RvmHost disconnected.`, and terminate Core nonzero without leaving target processes.
- Ensure `DebugRemoteProtocolSop.md` describes both operations, exact observable results, bounded waits, renderer fatal state, and cleanup requirements.

The source baseline confirms the problem. Workflow's `IRpcJsonMessageDispatcher` has no injection API; `RpcJsonDispatcherClient` waits through a message semaphore and separate startup events with no persistent failure predicate. GacUI's accepted-host disconnect path still invokes a terminal callback that broadcasts conditionally and calls `std::_Exit`, and `ViewModelHostClient` still stores all operational state in a nested `Impl`. `RemotingTest_Core` currently has no generic `vl::Exception`/`vl::Error` fatal conversion around requester startup and UI execution. The SOP describes ordinary `/RPT` and `/RVMT` use but does not describe clicking the visible `Fatel Error` button or force-terminating `RemotingTest_RvmHost`.

# PROPOSALS

- No.1 Add persistent dispatcher failure injection and route RVM host loss through the shared Core fatal boundary

## No.1 Add persistent dispatcher failure injection and route RVM host loss through the shared Core fatal boundary

Add `RpcInjectedException` and `IRpcJsonMessageDispatcher::InjectException` in Workflow. Store injected state separately from its message, serialize it with response commitment under a `CriticalSection`, and use a `ConditionVariable` so dispatcher-owned waits can wake and throw on their original caller thread. Keep the state persistent and last-write-wins, and propagate the interface through all implementations, reflection, release output, and documentation.

Import the released Workflow pair through GacUI's normal dependency workflow. Make `RpcServerHelpers` inject accepted-host loss after broker bookkeeping and outside its locks, latching the message until the requester dispatcher exists. Leave `CppTest_Rvm` uncaught. Restore a generic Core `vl::Exception`/`vl::Error` boundary while Core channels remain available so `/RPT` and `/RVMT` use the same single `BroadcastError` conversion. Flatten `ViewModelHostClient::Impl` into its owner without changing protocol ordering. Extend the shared SOP with the two fatal operations and their exact results.

### CODE CHANGE
