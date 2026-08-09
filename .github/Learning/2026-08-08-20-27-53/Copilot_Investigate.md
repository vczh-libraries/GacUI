# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Follow GacUI/DebugRemoteProtocolWithGacJS.md to make sure GacJS works in Linux against the complete test matrix. commit and push all local changes. This has been verified on Windows so I assume GacJS and GacUI should be both fine.

# UPDATES

# TEST [CONFIRMED]

Run the complete Linux matrix from `DebugRemoteProtocolWithGacJS.md` with fresh
processes for each target: `/RPT /MiniHttp`, `/FCT /MiniHttp`, and
`/RVMT /MiniHttp` with `RemotingTest_RvmHost`. Build GacJS, run its repository
tests, build both portable GacUI executables through the supported Linux build
script, and execute the matching feature, renderer-replacement, normal-close,
and error-injection operations from `DebugRemoteProtocolSop.md` in Playwright
Firefox.

The matrix succeeds only when every visible state transition and exact fatal
error required by the SOP is observed without unexpected browser page errors,
console errors, protocol hangs, or surviving Core/host processes. The browser
must load the generated GacJS website assets, connect to Core, and drive input
through the rendered page in both protocol directions.

The problem is confirmed in the first renderer-replacement exercise. A freshly
built `/FCT /MiniHttp` Core rendered normally in Playwright Firefox, but after
that page closed, a replacement Firefox page connected to the same application
session and Core rejected its image metadata with
`New metadata should be identical to the last one.`. GacJS displayed that text
in its fatal error mask and raised one matching page error; Core terminated
nonzero. The replacement renderer could not drive the application, so the Linux
matrix does not currently satisfy the shared SOP.

# PROPOSALS

- No.1 Measure each image after its matching browser load event [CONFIRMED]

## No.1 Measure each image after its matching browser load event

The two Firefox renderer connections received byte-for-byte identical image
data, including the PNG whose Core image id was 55. The first renderer reported
that image as 32 by 32, while the replacement reported `Unknown` and 1 by 1.
Browser diagnostics showed why: Firefox intermittently rejected
`HTMLImageElement.decode()` with `EncodingError: Invalid image request` before
the newly assigned data URL finished loading. The shared measuring element still
held the previous image's 16 by 16 natural size at that point. GacJS treated any
such rejection as a permanent decode failure and fabricated the generic
`Unknown` 1 by 1 metadata, making a valid PNG inconsistent across connections.

Use a fresh `HTMLImageElement` for each measurement and register its `load` and
`error` handlers before assigning `src`. Report the natural dimensions only
after the matching `load` event; retain the existing `Unknown` 1 by 1 fallback
only for an actual `error` event. Add a renderer unit regression that controls
the image events and verifies both paths, and update the DOM measurement
documentation to describe the event-driven behavior. A diagnostic-only browser
override using this algorithm already allowed the first, replacement, and
takeover Firefox renderers to connect without an error.

### CODE CHANGE

In `GacJS`, `ElementHTMLMeasurer` now creates a fresh image element for every
queued image measurement. It installs `load` and `error` handlers before
assigning the data URL, waits for that matching event, and reads natural image
dimensions only from the successful `load` path. The existing `Unknown` 1 by 1
fallback remains limited to the actual `error` path. The obsolete shared image
element and the call to `decode()` were removed.

`TestElementMeasurer.ts` supplies controlled fake image elements and covers both
event paths. The success regression deliberately makes `decode()` reject while
the image's `load` event provides PNG dimensions, proving the fixed code no
longer mistakes Firefox's early decode rejection for an invalid image. The
error regression verifies the fallback. `GacJS/doc/DOM.md` now documents this
event-driven measurement contract.

### CONFIRMED

The focused renderer regression passes, and the complete GacJS test command
passes 94 tests: one remote-protocol test, 89 renderer tests, and four HTTP
tests. `yarn build` also succeeds. Both GacUI portable executables build from a
clean state through their supported Linux build scripts.

The complete required Linux matrix passes in Playwright Firefox with a fresh
Core for every application target and the generated website served on port
8896:

- `/RPT /MiniHttp` completed the feature interaction, DataGrid add/clear,
  document-dialog, renderer replacement, third-renderer takeover with retained
  state, normal renderer terminal state, and application close operations. A
  separate fatal-error run displayed exact `This is a fatel error!`, raised
  only the SOP's one matching page error, and terminated Core nonzero.
- `/FCT /MiniHttp` added two groups of ten list items, cleared them, exercised
  Search and the rich document editor, preserved both typed markers across the
  `List` and `Control` tabs, and force-exited normally.
- `/RVMT /MiniHttp` completed translated input in both directions, rejected a
  second host without disturbing the accepted host, translated another marker,
  and exited normally. Fresh-process idle-next-call host loss displayed exact
  `RemotingTest_RvmHost disconnected.`, raised only the one matching page
  error, and terminated Core nonzero. The delivery-acknowledgement-loss variant
  stopped the accepted host at
  `SocketHttpClient::Impl::SubmitReceivePoll`, after the second RPC was
  delivered through the pending request and before the replacement request;
  the browser displayed that same exact error and Core terminated nonzero
  within the five-second bound.

All normal replacement pages received identical image metadata and remained
interactive. Every run loaded the built page and its CSS/JavaScript assets,
had no unexpected dialog, console error, page error, retry loop, or protocol
hang, and left no Core, host, or port-8888 listener behind. This confirms that
waiting for the image's own load/error event fixes the Firefox-only metadata
inconsistency without weakening invalid-image handling.
