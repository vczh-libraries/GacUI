# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

You are going to test `GacJS` with `GacUI` on macOS:
- Linux/macOS's `wchar_t` is UTF-32, but JavaScript uses UTF-16, it is a good time to launch GacJS on macOS.
  - The work is done and logged in current `Copilot_Investigate.md`, that's why `GacJS` should work now theoritically.
- Prepare `RemotingTest_Core` project and start with `/MiniHttp`.
  - Make an entry in `Test/Linux/RemotingTest_Core`, make sure it builds `Bin/RemotingTest_Core` instead of `Bin/UnitTest`.
- Verify with safari running `GacJS`:
  - Update package.json to skip e2e test in `npm run test` when the current platform is not Windows.
  - Run `GacJS` with playwright+chrome(chromium) first, make sure the core side is working.
  - Run `GacJS` with playwright+safari(webkit), make sure `GacJS` is compatible.
  - Do not try to run the actual safari since there might be permission issue, I will test with it manually later.
- Update `Tools/DebugGacUIWithGacJS.md`:
  - Write down macOS specific instructions.
- Unlike on Windows, building `GacJS` doesn't host the website automatically, you need to figure out how to host it. Probably `Linux` is going to do the same thing, prepare instructions for Linux too but no need to actually test Linux instructions since you are currently on macOS.
  - If any instruction is shared, reorganize this file instead of duplicating contents.
- Update `Tools/Jobs/job.verifyRemoteProtocol.prompt.md` accordingly but no need to follow it to test, verification of this instruction file will be done later.
- commit and push all local changes to master.

# UPDATES

# TEST [CONFIRMED]

Use the real macOS build and browser boundary instead of adding another unit test:

- Confirm the initial state has no `Test/Linux/RemotingTest_Core` entry, so the supported `build.sh` workflow cannot build the core on macOS and no `Bin/RemotingTest_Core` exists.
- Add the entry, build it only through `.github/Ubuntu/build.sh`, and require the generated target to be exactly `Bin/RemotingTest_Core`.
- Start that binary asynchronously with `/MiniHttp`, build GacJS, and serve `Gaclib/website/entry/lib/dist` from a retained local static-server process on port `8896`.
- Run a Playwright Chromium smoke test first. Require `index.html` to reach the first renderer-idle state, render substantial GacUI text, accept renderer-side input, and leave no browser dialog or page error.
- Repeat the same smoke test with Playwright WebKit against a fresh core process. WebKit is the requested automated compatibility coverage; do not launch actual Safari.
- Run the GacJS build followed by `npm run test`. On macOS, require the portable unit-test workspaces to pass while the website-entry package exits before launching its Windows-only E2E Vitest suite.
- Run the GacUI unit-test project from `Test/Linux/UnitTest` after the build-entry/source changes and require all selected files and cases to pass.
- Review the generated `vmake.txt` and `makefile`, the platform-specific preprocessor paths in `RemotingTest_Core`, browser/core output, and all repository diffs. Ensure only retained process identifiers from this investigation are stopped.

The missing build entry confirms the initial macOS repro: `Test/Linux` contains entries for the existing CLI and unit-test projects but not `RemotingTest_Core`, so the repository-supported macOS build path cannot produce the requested executable. The GacJS website-entry package also unconditionally launches Vitest; its internal protocol-suite guard reports skipped suites only after the Windows-specific E2E harness has been loaded.

# PROPOSALS

- No.1 Add a portable RemotingTest_Core build and verify GacJS in Chromium and WebKit [CONFIRMED]

## No.1 Add a portable RemotingTest_Core build and verify GacJS in Chromium and WebKit

Create a dedicated `Test/Linux/RemotingTest_Core/vmake` based on the owning Visual C++ project and its required shared projects, generated applications, DarkSkin, and no-reflection configuration. Override the default target with `CPP_TARGET=./Bin/RemotingTest_Core`. Let the supported build script generate `vmake.txt` and `makefile`; do not hand-edit either generated file.

Make only the source portability changes exposed by that build. Keep Windows named-pipe, Windows HTTP, and Windows automation code available on Windows, but exclude those implementations and headers on non-Windows platforms. Keep `/MiniHttp` and the portable async-socket automation path available on macOS/Linux, and make unsupported transport selectors fail explicitly instead of referencing unavailable platform types.

Condition the GacJS website-entry package test so it launches Vitest only on Windows. The root command still runs the portable unit-test workspaces and records an explicit E2E skip on non-Windows platforms. Preserve the existing Windows E2E behavior and leave the checked-in Chromium harness Windows-specific; use a focused temporary Playwright smoke runner for this investigation so WebKit coverage does not rewrite the Windows test suite.

Quote the `assets/**/*` argument in the website entry build script so the glob reaches `copyfiles` unchanged. On macOS, the unquoted glob is expanded by the shell before `copyfiles` runs and matches only nested files, producing a `lib/dist` with the JavaScript bundle and snapshot JSON but no `index.html`, CSS, or favicon. The quoted glob lets `copyfiles` copy both top-level and nested assets on every platform.

Rename the existing GacJS browser-debugging guide to the requested `Tools/DebugGacUIWithGacJS.md`, update references, and reorganize shared build/serve/browser instructions ahead of concise platform sections. Document the tested macOS MiniHTTP, Python static hosting, Chromium/WebKit commands, manual Safari handoff, cleanup, and the equivalent untested Linux build/hosting flow. Update the remote-protocol verification job so GacJS is supported on macOS and planned on Linux without changing the native-renderer support matrix.

### CODE CHANGE

- Added the generated Linux/macOS `RemotingTest_Core` make entry with 176 source files and an explicit `CPP_TARGET=./Bin/RemotingTest_Core`. It includes both generated test applications and DarkSkin while excluding Windows-only Vlpp source files.
- Guarded the Windows named-pipe, http.sys, native-window, automation, CRT, and leak-reporting paths with `VCZH_MSVC`. `/MiniHttp` continues through the portable async socket server, while `/Pipe` and `/Http` now return clear Windows-only errors on non-Windows platforms.
- Registered `RemotingTest_Core` in the Linux/macOS project documentation and standardized the documented selector as `/MiniHttp`.
- Made the GacJS website-entry test launch Vitest only on Windows. On other platforms it exits successfully with the explicit `Skipping Windows-only protocol E2E tests.` message while Lerna continues to run portable packages.
- Quoted the website asset glob so macOS copies `index.html`, CSS, the favicon, and nested snapshot assets into `lib/dist`.
- Documented portable `/MiniHttp` core startup and explicit Python static hosting in GacJS, including the Windows-only scope of the checked-in protocol E2E harness.
- Renamed the Tools browser guide to `DebugGacUIWithGacJS.md`, moved shared Linux/macOS build, hosting, browser, and cleanup instructions into one section, and added the verified Chromium/WebKit order plus the separate manual Safari handoff. Updated the verification job, SOP link, monorepo guide, and synchronized root `AGENTS.md`.

### CONFIRMED

The supported macOS build generated `vmake.txt` and `makefile`, compiled the 176-source target, and produced the exact executable `Test/Linux/RemotingTest_Core/Bin/RemotingTest_Core`. A final incremental run of `.github/Ubuntu/build.sh` completed successfully, and the generated `all` target references only `./Bin/RemotingTest_Core`, never `Bin/UnitTest`.

GacJS built all five packages successfully. Quoting `assets/**/*` changed the macOS output from a partial bundle to a complete static document root containing `index.html`, `index.js`, `global.css`, the favicon, supporting pages, and the nested snapshot. `npm run test` then passed one remote-protocol case and 87 renderer cases; the website-entry package printed the expected Windows-only E2E skip without starting Vitest.

The live `/MiniHttp /FCT` Chromium run connected renderer client 2, rendered 23 initial leaf-text nodes, opened the Control tab, showed one caret in the Search editor, and rendered `Hello` from five browser keyboard events. The final page had 47 leaf-text nodes and zero dialogs, page errors, or console errors. The browser, static server, and core were stopped before the next run.

A fresh `/MiniHttp /FCT` core produced the same WebKit result: renderer client 2 connected, 23 initial and 47 final leaf-text nodes rendered, one caret appeared, `Hello` round-tripped through real input, and no dialog, page error, or console error occurred. This confirms Playwright WebKit compatibility on macOS; actual Safari was deliberately not launched.

The macOS GacUI unit-test target built successfully and its complete executable run returned success. A focused copilot-mode replay of the UTF-32-native caret cache file ended with `Passed test files: 1/1` and `Passed test cases: 3/3`. Snapshot and existing UnitTest make-metadata rewrites produced by the full runner were inspected and restored because they are unrelated test artifacts.

The final documentation review confirmed exact `/MiniHttp` casing, valid Playwright CLI syntax, one-shell PID retention and cleanup, a fresh core for Chromium, WebKit, and later Safari runs, an explicit distinction between interactive Playwright CLI sessions and programmatic idle hooks, and Linux instructions that are prepared but not reported as tested.
