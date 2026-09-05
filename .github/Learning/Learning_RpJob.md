# Important Notes of Remote Protocol Manual Testing

## Shared

- Locate input targets again from the current rendered tree after text or tab changes. An idle notification from an earlier mouse move can precede the button result; wait for the exact expected visible text before judging the next transition.
- Wait for a dialog's control bounds to settle before clicking its confirmation button. Its text can appear before the final centered layout. FullControlTest uses the visible localized confirmation label `确定`; RemoteProtocolTest uses `OK`.
- Group DataGrid cells by their vertical centers when checking rows. Different cell templates can give text in the same row different top and bottom bounds.
- Automation `DbClick` already performs down/up, double-click, and the final up. Sending an additional up violates the automation input-state contract and correctly raises an error.

## Windows

- The standalone RVM application's custom caption close button uses native hit testing. Clicking its composition through local automation does not exercise that caption path; exact `!Exit` on the standalone application's native automation endpoint performs its normal application shutdown.
- Retain an operating-system process handle until termination when recording an exit code. Looking up a PID after it disappears loses the original exit status.
- During a fresh renderer connection, treat an automation HTTP connection reset as a readiness observation: inspect native dialogs and process health, then retry the read within the startup bound while the requester remains alive. One `/RVMT /Http` startup read reset; a fresh run completed the entire blocked-call fatal check. A failed diagnostic read alone does not establish an application protocol failure.
- A nonzero standalone host-loss exit can surface as a Windows breakpoint status outside a debugger. CDB first- and second-chance exception inspection confirmed the originating unhandled `vl::rpc_controller::RpcInjectedException` from `RpcJsonDispatcherClient::ThrowInjectedExceptionLocked`, with `RemotingTest_RvmHost disconnected.`. Do not infer the exception type from the exit status alone.
- Real Windows global hotkeys require an interactive input desktop. If `GetForegroundWindow` is null and the renderer cannot acquire focus, keep that coverage pending; renderer automation does not prove the global registration and dispatch path.
- Maximize the native RPT renderer before the document walkthrough. At the initial small window size, the document's inline `RIGHT NOW` button can fall below the visible page after layout; inspect current visible controls instead of using stale measured content.
- In the FCT ribbon, identify the Search editor by its position beside `Search: ` (including the label's trailing space in native Controls output). Several other `SinglelineTextBox` controls edit color components. The large rich editor appears as `DocumentViewer` in Controls output.
- Mouse automation accepts `super` as a modifier, but keyboard commands resolve names through the native input service. Do not assume the display name `Win` is an accepted native key name; use actual Windows input for the Super shortcut checks.
- When Windows resolves `localhost` slowly for an IPv4 MiniHTTP endpoint, use `127.0.0.1` for temporary automation reads and commands. One measured Controls request took 2.062 seconds with `localhost` and 0.016 seconds with the explicit loopback address. Keep `localhost` for the Windows HTTP automation endpoints used with `/Pipe` and `/Http`: their registered hostname rejects the IPv4 Host header with HTTP 400. Removing the MiniHTTP delay exposed a renderer-takeover race, so a slow harness must not be treated as a synchronization guarantee.
- Preserve stalled Core and renderer processes for CDB before cleanup. A repeated `/RVMT /Http` replacement stall showed Core awaiting a paragraph response whose outgoing request remained queued beside an available Windows HTTP poll. This identified a missing delivery in VlppOS `HttpServerConnection::SubmitResponse`; a deterministic real-transport test reproduced it. Earlier `/RPT /Http` document-dialog timeouts may have shared that cause, but no debugger state was captured for those incidents. A passing rerun alone does not establish the cause of an earlier timeout.

## Linux (wGac repo specific)

## macOS (iGac repo specific)
