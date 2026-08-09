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
