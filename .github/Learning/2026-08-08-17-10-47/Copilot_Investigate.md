# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Follow GacUI/DebugRemoteProtocolWithGacJS.md to make sure GacJS works in macOS against the complete test matrix. commit and push all local changes. This has been verified on Windows so I assume GacJS and GacUI should be both fine.

# UPDATES

# TEST [CONFIRMED]

Build the portable `RemotingTest_Core` and `RemotingTest_RvmHost` targets through
the supported macOS build script. Build and test the GacJS workspace, install
Playwright WebKit, and serve the generated website from its required document
root.

Exercise the complete macOS GacJS matrix with fresh `/MiniHttp` processes:

- `/RPT`: perform the normal Home, DataGrid, document-dialog, two-renderer-
  replacement, state-continuity, and menu-close procedure. In another fresh
  session, trigger the Core-authored fatal error and verify its exact terminal
  GacJS state.
- `/FCT`: mutate and clear both lists, type distinct markers into the search and
  rich editors, verify both markers across tab changes, and force-exit through
  GacJS.
- `/RVMT`: start Core before the accepted host, verify Workflow RPC translation,
  reject a second host without disrupting the accepted host, and close through
  the UI. In fresh sessions, run both host-loss variants from the fatal addendum
  and verify the exact Core-authored terminal error without a hang.

Each browser run must use Playwright WebKit, register idle/blink and error
handlers before navigation, send input only through the rendered page, wait on
renderer events rather than arbitrary UI sleeps, and load `index.html`,
`global.css`, and `index.js`. Success requires every stated UI transition and
exact terminal result, no unexpected dialog/page/console error, no reconnect or
retry loop, and bounded cleanup of every process and listener.

Both portable GacUI targets completed clean full rebuilds through
`.github/Ubuntu/build.sh -f`. GacJS completed `yarn build`, installed Playwright
WebKit, and passed `yarn test`: 87 renderer tests, four HTTP-client tests, and
one remote-protocol package test passed; the documented Windows-only browser
suite skipped on macOS as expected.

The live Playwright WebKit verification passed all three fresh `/MiniHttp`
matrix targets. Every page loaded `index.html`, `global.css`, and `index.js`
with HTTP 200. `/RPT` passed its Home transition, three populated DataGrid rows
and clear, document dialog, two renderer takeovers with state continuity and a
settled detached renderer, and confirmed menu close. `/FCT` added and cleared
the complete `0` through `9` sequence in both lists, preserved distinct
`webkitmini1` and `webkitmini2` markers in the search and rich editors across
tab changes, and force-exited successfully. `/RVMT` translated both typed
markers, rejected a second host without replacing the accepted host, continued
translating through the accepted host, and closed successfully. All normal
paths retained the ordinary success mask, with no unexpected dialog, page
error, console error, reconnect, or request retry. WebKit reports one expected
failed-resource console diagnostic when an outstanding long poll is ended by
renderer replacement or normal Core shutdown; the client stopped requesting
and displayed no fatal mask.

The fatal addendum also passed in fresh sessions. `/RPT` displayed exactly
`This is a fatel error!` in the error mask, emitted one matching WebKit page
error, and Core terminated by `SIGABRT`. The `/RVMT` idle-next-call host loss
displayed exactly `RemotingTest_RvmHost disconnected.`, emitted one matching
page error, released the UI without hanging, and Core terminated by `SIGABRT`.
For delivery-acknowledgement loss, LLDB stopped the accepted macOS host at
`SocketHttpClient::Impl::SubmitReceivePoll` in the exact interval after message
delivery and before the replacement poll. Killing the host there produced the
same single exact error and `SIGABRT`; the end-to-end browser observation
completed in 5.345 seconds, including the five-second acknowledgement deadline.
All Core, host, browser, debugger, and listener processes were gone afterward.

# PROPOSALS

- No.1 Retain product code after complete macOS verification [CONFIRMED]

## No.1 Retain product code after complete macOS verification [CONFIRMED]

Exercise the documented macOS matrix and shared SOP against clean rebuilt
binaries. Change GacJS, GacUI, or an owning dependency only if a product failure
is reproduced; otherwise retain the implementations already verified on
Windows and record the independent macOS result.

### CODE CHANGE

No product-code correction was required. The previous completed investigation
was archived as required, and this investigation record was created for the
macOS build and live WebKit evidence.

### CONFIRMED

The clean builds, portable unit tests, three normal matrix targets, Core-authored
RPT fatal path, and both RVM host-loss timings all passed their exact observable
criteria. Repeated normal and fatal runs remained stable after adding strict
dialog, page-error, console-error, static-asset, terminal-mask, process-exit,
and detached-renderer no-retry checks. No change to GacJS, GacUI, or a dependency
is justified by the macOS results.
