- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

- `GuiRemoteProtocolRendererSingle` when receives a fatal error:
  - Display a system message box to ask if the user wants to close the application.
  - Yes: goes the old way.
  - No:
    - displays remote errors on a half transparent white background instead of system message box.
    - `/Dom` returns error.
    - Being able to exit by `!Exit` or closing the window.
    - Already disconnected from GacUI Core.

## DETAILS

- There is no `GuiRemoteProtocolRendererSingle` type. The renderer is `vl::presentation::remote_renderer::GuiRemoteRendererSingle`, while transport errors are received by `RemotingTestChannelClient` in `Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp`.
- The current fatal-error path queues an OK-only native message box and then calls `GuiRemoteRendererSingle::ForceExitByFatelError`, which marks the renderer as disconnecting and posts a close. `RemotingTestChannelClient::OnDisconnected` detaches incoming renderer messages and deliberately skips that exit call after a fatal error.
- Keeping the renderer visible requires a distinct terminal state that marks it disconnected without immediately hiding the window. Otherwise both the title-bar close and automation `!Exit` are cancelled by `GuiRemoteRendererSingle::BeforeClosing` and forwarded to the disconnected core.
- A fatal error can arrive before `SetupRawWindowsDirect2DRenderer` creates the native controller/window, before the first rendering DOM, or while a frame has suppressed refreshing. The retained-error path cannot assume an existing window, DOM, or completed frame.

## REVIEW COMMENTS

### Clarify The Target, Error Sources, And Yes Branch

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

### Define Startup, Rendering, And Overlay Behavior

**review comment**: `WaitForServer` starts receiving before the native controller and main window exist, while the current fatal callback immediately accesses both. Normal renderer packages are cached until a UI-thread invoker exists, but fatal errors bypass that cache. Please require fatal errors to be stored safely until the window is registered, with all dialog/renderer mutation performed once on the UI thread. Also define the overlay acceptance criteria: whether it freezes the last completed frame underneath, covers the full client area with 50%-transparent white, how multiline error text is laid out, and that it must still render with no DOM, during an interrupted frame, after resize/DPI changes, and after render-target recreation.

covered above.

### Define The `/Dom` Error Contract

**review comment**: “`/Dom` returns error” is not testable yet. Today it returns an HTTP success containing the renderer DOM JSON, while thrown errors become HTTP 404. Please specify the exact HTTP status, content type, and response body/schema; whether it contains the original fatal error as one string or a list; whether `CanDumpDomTree()` remains true; and whether the frozen DOM/locally drawn overlay is omitted from the response. A stable exact response is needed for automation clients and verification.

covered above.

### Require End-To-End Verification

**review comment**: The required unit test cannot cover this behavior by itself because `Project.md` explicitly excludes `GuiRemoteRendererSingle` from unit-test coverage. Please add debugger-attached two-process verification for both named-pipe and HTTP modes: fatal error + Yes exits by the old path; fatal error + No leaves a readable overlay over the frozen window; `/Dom` returns the specified error; the core is already disconnected; and both a title-bar close and POST `/IO` with `!Exit` terminate the retained renderer cleanly. Include a fatal error before the first DOM/frame if that lifecycle is supported, handle the native prompt from another process before issuing HTTP automation requests, and explicitly repeat the existing `/Pipe` Fatal Error repro from `ToDo/KnownIssues.md` to prove the intermittent hang is gone.

You are going to operate `RemotingTest_Rendering_Win32` by yourself with `/RPT`, which has a `Fatal Error` button for you to click and let GacUI core crash. No test automation will cover this scenario.
