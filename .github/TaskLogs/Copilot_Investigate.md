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
error required by the SOP is observed without browser page errors, unexpected
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

- No.1 Measure each image after its matching browser load event

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
