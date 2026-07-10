- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

- `GuiRemoteRendererSingle` when its host receives a fatal remote-protocol error:
  - Display a system message box to ask if the user wants to close the application.
  - Yes: goes the old way.
  - No:
    - displays remote errors on a half transparent white background instead of system message box.
    - `/Dom` keeps returning the frozen DOM and includes the fatal error.
    - `/IO` reports that the application has stopped responding.
    - Being able to exit by `!Exit` or closing the window.
    - Already disconnected from GacUI Core.

## DETAILS

- There is no `GuiRemoteProtocolRendererSingle` type. The renderer is `vl::presentation::remote_renderer::GuiRemoteRendererSingle`, while transport errors are received by `RemotingTestChannelClient` in `Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp`.
- The current fatal-error path queues an OK-only native message box and then calls `GuiRemoteRendererSingle::ForceExitByFatelError`, which marks the renderer as disconnecting and posts a close. `RemotingTestChannelClient::OnDisconnected` detaches incoming renderer messages and deliberately skips that exit call after a fatal error.
- Keeping the renderer visible requires a distinct terminal state that marks it disconnected without immediately hiding the window. Otherwise both the title-bar close and automation `!Exit` are cancelled by `GuiRemoteRendererSingle::BeforeClosing` and forwarded to the disconnected core.
- `GuiRemoteRendererSingle` owns the reusable disconnected-but-visible state, stopped rendering, and suppression of further core events. `RemotingTest_Rendering_Win32` owns the native Yes/No interaction and passes the remote error and selected policy into the renderer.
- The retained-window behavior is required after `GuiMain` has registered the renderer window and automation service and at least one frame has completed, matching the manual `Fatel Error` scenario. Pre-registration, missing-DOM, and interrupted-frame fatal injection are outside this task. After No, the retained mask must continue repainting across resize, DPI changes, minimize/restore, and render-target recreation.
- `RemotingTestChannelClient` should use one first-error-wins fatal handler for remote `OnReadError` and fatal `OnLocalError`; nonfatal local errors remain ignored and an unexplained `OnDisconnected` retains the old immediate renderer-exit behavior. Claim the fatal state before any send or queued UI work so overlapping/disconnect/reentrant callbacks cannot show multiple dialogs or preempt the retained window.
- A remote `OnReadError` means GacUI Core is already shutting down and needs no extra event. A fatal `OnLocalError` shows the same Yes/No prompt but first makes one best-effort attempt to send the existing `ControllerForceExit` event while the channel is still connected, independently of the eventual Yes/No choice. The core already handles this event by bypassing `BeforeClosing`; no protocol schema or code generation change is required.
- Use `ERROR from GacUI Core` for remote errors and `ERROR from Renderer Transport` for fatal local errors. In both cases, the dialog body contains the original error and asks whether to close the renderer; repeated errors do not replace the first displayed/stored error.
- Yes is the default message-box choice and immediately uses the existing fatal-exit path. In one UI-thread callback, No passes the same first fatal error to both the renderer and `AutomationServiceRenderer`, keeps the renderer window alive, changes its title to `[STOPPED] <original-title>`, preserves the last renderable state, and draws the error over a full-client 50%-transparent white mask.
- Renderer automation remains available after No. Add the renderer-specific `SetFatalError(Nullable<WString>)` operation to `AutomationServiceRenderer` (and therefore `WindowsAutomationServiceRenderer`), not to the platform-wide `INativeAutomationService`. Keep access to this state synchronized across the UI and HTTP callback threads.
- Change `INativeAutomationService::CanRunIOCommands()` to return `INativeAutomationService::IOCommandAvailability`, an `enum class` with `Disabled`, `Enabled`, and `ExitOnly`. Migrate the unavailable, shared-base, remote-core, ordinary Windows, hosted Windows, and renderer Windows implementations. `WindowsAutomationServiceRenderer` returns `ExitOnly` when its synchronized `fatalError` is nonempty and `Enabled` otherwise; the ordinary and hosted Windows services and `RemoteProtocolAutomationService` remain `Enabled`, while the shared default remains `Disabled`.
- Enforce `ExitOnly` centrally in `AutomationServiceBase::RunIOCommand`, not only in the Windows HTTP wrapper: exact `!Exit` is forwarded and keeps the normal `Queued` result, while every other command synchronously returns exactly `!Application stopped responding.` without queuing work. The HTTP wrapper treats only `Disabled` as unsupported/404 and calls `RunIOCommand` for both `Enabled` and `ExitOnly`. `RunIOCommand` must re-read the synchronized mode so an `Enabled` to `ExitOnly` transition between the HTTP capability check and execution cannot queue stale IO.
- Update the `INativeAutomationService` API comments and `.github/KnowledgeBase/manual/gacui/coding-agent/automation-service.md` to document all three availability values and the exact `ExitOnly` response contract.
- A healthy `/Dom` response keeps its existing schema. After No, it remains an HTTP success containing the frozen renderer DOM, plus an exact lowercase top-level string field `fatalError` with the original remote error; the locally drawn mask is not part of the remote DOM. Omit `fatalError` while the nullable value is empty.
- The `fatalError` setter, `/Dom` snapshot, and renderer IO-availability query must copy/read the nullable string under a short lock. The `INativeAutomationService` change is not reflected, so it does not trigger metadata generation.
- Manual RemoteProtocolTest runs use `/RPT /Pipe` or `/RPT /Http` on `RemotingTest_Core`; `RemotingTest_Rendering_Win32` accepts only the matching `/Pipe` or `/Http` argument.

## VERIFICATION

- Add a focused automation-service test double covering all three IO modes: `Disabled` never invokes internal handling; `Enabled` forwards arbitrary commands; `ExitOnly` forwards exact `!Exit` but rejects `!Exit `, ordinary commands, and malformed commands with exactly `!Application stopped responding.`. Build `Test/GacUISrc/GacUISrc.sln` in the default Debug|x64 configuration through `.github/Scripts/copilotBuild.ps1`; require `Build.log` to end in build success with zero warnings and errors. Run `UnitTest` through `.github/Scripts/copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`; require all selected test files and cases to pass with no memory-leak dump.
- Perform manual two-process Windows verification with `RemotingTest_Rendering_Win32` under the debugger; do not add an automated test for the native-prompt scenario. Test core `/RPT /Pipe` with renderer `/Pipe`, then core `/RPT /Http` with renderer `/Http`, starting the core before the renderer.
- In each fresh session, confirm that the RemoteProtocolTest UI appears, record the original title and `/Dom`, and prove healthy renderer IO still works by POSTing a representative command and observing `Queued` plus the expected UI change. Then manually activate its currently labelled `Fatel Error` button. While the native modal prompt is open, do not call application HTTP automation; inspect and operate the renderer process's `#32770` dialog from a separate Win32 helper process following `.github/Guidelines/Running-ComputerUse.md`. Confirm that the fatal text and both Yes and No buttons appear and that Yes is the default.
- Yes case: accept the default Yes and confirm the old fatal-exit behavior completes, the renderer closes, the core has already stopped, neither process hangs, and the debugger reports no unexpected exception.
- No case: choose No and confirm the core is already stopped/disconnected while the renderer remains alive; the title is exactly `[STOPPED] <original-title>`; a readable fatal message appears on a full-client 50%-transparent white mask over the retained UI; and ordinary mouse/keyboard input causes no UI change. Resize and minimize/restore the window to prove the mask redraws and still covers the client.
- After No, GET `http://localhost:8888/Automation/RemotingTest_Rendering_Win32/Dom`; require HTTP success, the frozen pre-error DOM, and the exact `fatalError` string. POST a representative non-exit `/IO` command such as `!Type:x`; require HTTP success with exactly `!Application stopped responding.` rather than 404, an empty response, or `Queued`, and confirm no work is queued and the renderer stays alive. In one fresh No session POST exact `!Exit`, require `Queued`, and confirm it closes the renderer locally; in another, use the native title-bar close and confirm the same clean exit.
- Separately inject a fatal `OnLocalError` after the renderer is connected. Under the debugger, require one `ControllerForceExit` arrival at the core, no core `BeforeClosing`, one Yes/No prompt, and no preemption of the retained No window by the following `OnDisconnected`. Inject a recursive failure while sending the force-exit event and require no duplicate event/prompt or crash; if delivery fails, require the renderer path to remain safe because the event is best-effort.
- Explicitly regress `ToDo/KnownIssues.md` by completing five consecutive fresh `/Pipe` fatal-error sessions without a hang, including the default Yes path, No plus `!Exit`, and No plus title-bar close. Run Yes and both No exit paths at least once with `/Http`. After every run, confirm no stale core/renderer process remains and the next pair can bind and connect normally.

## REVIEW COMMENTS

### Clarify The Target, Error Sources, And Yes Branch [CLOSED]

**review comment**: Please confirm that the reusable state/rendering behavior belongs to `GuiRemoteRendererSingle`, with `RemotingTestChannelClient` responsible only for receiving the transport error, asking the policy question, and passing the error into the renderer. Also clarify which callbacks use this policy: a remote fatal error from `OnReadError`, a fatal `OnLocalError`, and/or an unexplained `OnDisconnected`. Remote-core errors and local transport errors currently share a misleading “ERROR from GacUI Core” title, so define their user-facing titles/bodies and whether repeated errors are accumulated or the first fatal error wins. Finally, define “Yes: goes the old way”: should the Yes/No dialog itself contain the fatal error and selecting Yes immediately call the existing exit path, and which button should be the default?

Yes `GuiRemoteRendererSingle` is going to process the actual protocol exiting stuff, and it serves as a library so that everyone could write a GacUI renderer easily.
`RemotingTest_Rendering_Win32` is a test app to use it. And it handles actual user interaction.
If the user selects No, it means the app should not close immediately while the connection should be disconnected, at that moment it could simply stop sending any new events to GacUI core.
Now the DOM tree will never change, so `GuiRemoteRendererSingle` could still use it to render the UI from the last state, and add a new transparent white rectangle covering the whole application, meaning that the app will no longer response to the user. And change the title to `[STOPPED] <original-title>`.
For the message box, YES will be default.

### Define The Disconnected-But-Visible State

**review comment**: The No branch needs an explicit UI-thread transition into a disconnected-but-visible terminal state. Besides allowing `BeforeClosing` to close locally, please confirm that this transition must wait until or ensure transport disconnection, run exactly once when fatal/disconnect callbacks overlap, release mouse capture, unregister global shortcuts, clear accumulated IO/window updates, and suppress all further events to the dead core. Define whether the overlay is a true input mask and whether renderer automation `/IO` must reject every command except `!Exit`; any input mask must still preserve native and custom-frame close-button hit testing. Without these requirements the frozen window can continue hit testing and sending mouse, keyboard, timer, shortcut, and close events through a disconnected channel. The implementation should also replace the current `Initialize(nullptr)` reader-uninstallation call with an explicit detach operation because the channel contract defines `Initialize` as install-only.

Fetal error from GacUI core means GacUI core will no longer process any new events and kill itself immediately, so `GuiRemoteRendererSingle` no need to bother about that.
Since the app already no responding, `/IO` will not make any sense at all. So `/IO` should be disabled, but instead of 404, it should return an error saying the app stops responding. A coding agent might keep sending `/IO` and keep receiving the error, which is fine. The app should not crash because of receiving `/IO` at this moment. The command returning the DOM tree should remain working and add a `fatalError` field containing the error message. The automation service should add a new method to set a `Nullable<WString>` fatalError, and use it in `RemotingTest_Rendering_Win32`.

### Define Startup, Rendering, And Overlay Behavior [CLOSED]

**review comment**: `WaitForServer` starts receiving before the native controller and main window exist, while the current fatal callback immediately accesses both. Normal renderer packages are cached until a UI-thread invoker exists, but fatal errors bypass that cache. Please require fatal errors to be stored safely until the window is registered, with all dialog/renderer mutation performed once on the UI thread. Also define the overlay acceptance criteria: whether it freezes the last completed frame underneath, covers the full client area with 50%-transparent white, how multiline error text is laid out, and that it must still render with no DOM, during an interrupted frame, after resize/DPI changes, and after render-target recreation.

covered above.

### Define The `/Dom` Error Contract [CLOSED]

**review comment**: “`/Dom` returns error” is not testable yet. Today it returns an HTTP success containing the renderer DOM JSON, while thrown errors become HTTP 404. Please specify the exact HTTP status, content type, and response body/schema; whether it contains the original fatal error as one string or a list; whether `CanDumpDomTree()` remains true; and whether the frozen DOM/locally drawn overlay is omitted from the response. A stable exact response is needed for automation clients and verification.

covered above.

### Require End-To-End Verification [CLOSED]

**review comment**: The required unit test cannot cover this behavior by itself because `Project.md` explicitly excludes `GuiRemoteRendererSingle` from unit-test coverage. Please add debugger-attached two-process verification for both named-pipe and HTTP modes: fatal error + Yes exits by the old path; fatal error + No leaves a readable overlay over the frozen window; `/Dom` returns the specified error; the core is already disconnected; and both a title-bar close and POST `/IO` with `!Exit` terminate the retained renderer cleanly. Include a fatal error before the first DOM/frame if that lifecycle is supported, handle the native prompt from another process before issuing HTTP automation requests, and explicitly repeat the existing `/Pipe` Fatal Error repro from `ToDo/KnownIssues.md` to prove the intermittent hang is gone.

You are going to operate `RemotingTest_Rendering_Win32` by yourself with `/RPT`, which has a `Fatal Error` button for you to click and let GacUI core crash. No test automation will cover this scenario.

### Define The Fatal Callback Policy And Single Source Of State

**review comment**: The reply scopes its explanation to a fatal error broadcast by GacUI Core, but current fatal `OnLocalError` delegates to `OnReadError`, an unexplained `OnDisconnected` takes a different immediate-exit path, and repeated callbacks can queue multiple dialogs. Please state which callbacks receive the Yes/No policy. The simplest policy is remote `OnReadError` only, with fatal local errors and unexplained disconnects retaining the old immediate-exit behavior. Whichever policy is chosen, the first fatal error must win, prompt at most once, and populate one synchronized stored error that drives the renderer mask/title and `AutomationServiceRenderer::fatalError`; separate renderer and automation copies can otherwise diverge.

You already see `OnLocalError` delegates when `fatal` is true, that is expected, so we still handle the message box in both function. But only native `OnReadError` means GacUI core will shutting down, so we can extract one method to do that, and extra remote protocol will be sent to Core from `OnLocalError` branch, telling the core to exit. There is a force exit event which doesn't trigger the BeforeClosing from the core.

### Complete Renderer-Local Shutdown And Channel Detachment

**review comment**: Core termination does not clean up renderer-owned OS state. A retained renderer can still own mouse capture, registered global shortcuts, accumulated IO/window updates, and native listeners that send through `events`. The disconnected-but-visible transition therefore still needs to release capture, unregister shortcuts, clear pending updates, suppress all core-bound callbacks, and permit local close while keeping paint/resize active. Also replace `GuiRemoteProtocolAsyncJsonChannelRenderer::Initialize(nullptr)` with an explicit detach operation because `IChannel::Initialize` is install-only; the wrapper's current private null convention should not leak into application code.

"Disconnect" means the remote protocol disconnecting thing, you don't need to take care of the actual remote networking disconnecting stuff.

### Make The Claimed Early And Partial-Frame Scope Verifiable [CLOSED]

**review comment**: The manual `Fatel Error` button runs only after a window and DOM are visible, so it cannot verify the existing DETAILS claim that a fatal error may arrive before setup/the first DOM or during a suppressed frame. Please either narrow the supported scope to errors after the first completed frame, or add a debugger/fault-injection procedure for each earlier lifecycle point. If those points remain supported, define whether the mask covers the last completed native frame or partially applied next-frame state and require painting to recover from suppressed refresh, resize/DPI changes, and render-target recreation without dereferencing a missing DOM.

It doesn't matter, since `RemotingTest_Rendering_Win32` are going to set the fatal error to the automation service, just add `fatalError` field to the JSON when the fatal error is set.

### Make Local Fatal Force-Exit Safe And Testable

**review comment**: `ControllerForceExit` already exists and bypasses core `BeforeClosing`, but a fatal `OnLocalError` means the same transport may already be unable to deliver it. The callback also runs on an arbitrary network thread, while the ordinary renderer-channel sender has UI-side mutable batching state, and a failed send can recursively raise `OnLocalError`. Please require an explicitly synchronized immediate-send path: atomically claim the first fatal error before sending, attempt exactly one force-exit only while the client still reports Connected, complete the attempt before `OnLocalError` returns and `OnDisconnected` runs, and treat delivery as best-effort. A delayed UI post is too late. Add a deterministic debugger/fault-injection verification distinct from the `Fatel Error` button: break at `GuiRemoteController::OnControllerForceExit`, prove core `BeforeClosing` is skipped, prove only one prompt/event occurs under a recursive send failure, and prove `OnDisconnected` does not preempt the No branch.

`OnLocalError` is supposed to be raised only when the network protocol client has problem, the server wouldn't know anything about that because the remote protocol connection is designed to be able to "disconnect and reconnect". So your concern would not happen. Just call `ControllerForceExit` when `OnLocalError` triggers it.

### Separate Renderer Cleanup From Network Shutdown

**review comment**: Agreed that `GuiRemoteRendererSingle` should not stop or coordinate the raw network transport. The remaining cleanup is renderer-local remote-protocol state, not networking: set the stopped flag so `BeforeClosing` permits local close; release native mouse capture; unregister OS global shortcuts; clear pending mouse/wheel/key/bounds updates; stop caret blinking; and guard every future `events->...` call, including resize/DPI notifications, while retaining paint, refresh, hit testing, the window listener, DOM/elements, and render target. Likewise, replacing `GuiRemoteProtocolAsyncJsonChannelRenderer::Initialize(nullptr)` with a named detach operation only clears the protocol reader and queued messages; it does not stop the transport. Without these local steps, the retained window can keep OS resources and continue producing events even though network batches are dropped.

agreed.
