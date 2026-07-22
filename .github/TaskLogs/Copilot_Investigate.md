# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

When `RemotingTest_Renderer_Win32` is connecting `RemotingTest_Core` using `/MiniHttp`, and during a normal exiting process, when core is gone it might result in `/Response` returning 404. This is normal, but 404 will also become a local fatal error injecting into the native renderer.

The current behavior is that, if a local fatal error happens, it will prompt an error message and then exit or render the error according to user selection. But if core is exiting normally, it should send the `ControllerConnectionStopped` message to the native renderer. Once 404 happens after receiving `ControllerConnectionStopped`, it should be ignored, and the native renderer should just close itself.

This behavior doesn't always repro because such 404 is not stably generated, it depends on whether a `/Response` is pending or not. In order to increase the probability, you could use `/FCT`, because it is big, exiting takes time, meanwhile `/RPT` is small and everything is happening too fast, lowering the repro rate.

You are going to repro this and fix the issue, I think you can always ignore any fatal error after receiving `ControllerConnectionStopped`, as errors after core exiting doesn't really matter anyway. And run `/MiniHttp`, `/Http`, `/Pipe` multiple times (reasonably amount of time that could trigger the issue, you will have a sense during repro-ing), goes to `Exit` tab to click a button to trigger the normal exit process, or send `/Exit` through automation service, make sure the fix is stable.

# UPDATES

# TEST [CONFIRMED]

Build the complete Debug x64 solution, then repeatedly launch `RemotingTest_Core /MiniHttp /FCT` and `RemotingTest_Rendering_Win32 /MiniHttp` through the repository wrappers. After the renderer has connected, post exact `!Exit` to the core automation `/IO` endpoint. Confirm that the core exits normally and inspect the renderer process, native windows, renderer `/Dom` endpoint, TCP state, and debugger breakpoints around `GuiRemoteRendererSingle::RequestControllerConnectionStopped`, `RemotingTestChannelClient::OnLocalError`, and the MiniHTTP API shutdown boundary.

The pre-fix Debug x64 solution built with 0 warnings and 0 errors. Live FullControlTest runs confirmed the reported timing dependency: after `!Exit` returned `Queued` and the core exited, the renderer's next connection to port 8888 varied between an absent request and a new `/Response` connection attempt. Debugger pauses around `SocketHttpServerApi::Impl::StopInternal` showed that the core first stops its automation API, finalizes the remote controller (which submits `ControllerConnectionStopped`), and then unregisters the remoting API; whether another renderer poll is already accepted during the very small unregister/listener-stop window determines whether the renderer sees 404. Runs without that accepted poll naturally cannot display the reported prompt, which is the negative half of the intermittent reproduction described in the problem.

The policy defect itself is deterministic once that transport outcome occurs. `GuiRemoteRendererSingle::RequestControllerConnectionStopped` calls `DisconnectFromCore`, setting the renderer's terminal `disconnectingFromCore` state. `RemotingTestChannelClient::OnLocalError` nevertheless treats every fatal local error as claimable: `ClaimFatalError` only checks `triggeredFatalError`, then `OnLocalError` requests a core force-exit and queues the native Yes/No prompt. There is no check for the already-received normal-stop state, so a 404 delivered after `ControllerConnectionStopped` necessarily enters the fatal UI path.

After the fix, the same normal-exit runs must close the renderer without a fatal prompt whenever `ControllerConnectionStopped` has been processed, regardless of any later read/local/disconnection callback. Genuine fatal errors before the normal-stop message must retain the existing prompt and Yes/No behavior. Verification must cover `/MiniHttp`, `/Http`, and `/Pipe` with `/FCT` repeatedly, plus the complete unit-test project and a clean Debug x64 build.

# PROPOSALS

- No.1 IGNORE FATAL CHANNEL ERRORS AFTER RENDERER DISCONNECTION

## No.1 IGNORE FATAL CHANNEL ERRORS AFTER RENDERER DISCONNECTION

Expose `GuiRemoteRendererSingle`'s existing `disconnectingFromCore` terminal state through an `IsDisconnectedFromCore()` query and make the state atomic because transport error callbacks and protocol message dispatch run on different threads. Keep the state owned by `GuiRemoteRendererSingle`: it already controls suppression of core-bound events and local closing after `ControllerConnectionStopped`.

In `RemotingTestChannelClient::ClaimFatalError`, reject the claim when its renderer exists and reports that it is disconnected from the core. This single gate applies equally to fatal packages from the core and local transport failures, so any error arriving after normal controller shutdown is ignored before it can request another core exit or queue the native fatal prompt. Errors arriving before disconnection preserve the current first-error-wins behavior and application-specific Yes/No policy.

### CODE CHANGE

- Change `GuiRemoteRendererSingle::disconnectingFromCore` to `atomic_vint` and add `GuiRemoteRendererSingle::IsDisconnectedFromCore()` as a public read-only query.
- Update `RemotingTestChannelClient::ClaimFatalError` to claim only when no fatal error has already been triggered and the renderer has not disconnected from the core.
