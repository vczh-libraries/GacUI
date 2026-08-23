# Operating GacUI Through GacJS

This guide records how to build and start `RemotingTest_Core`, serve GacJS, and
operate the browser renderer. The feature operations, error injections, and
pass/fail observations are defined only in
[`DebugRemoteProtocolSop.md`](DebugRemoteProtocolSop.md). Use this guide to
establish each required browser-renderer topology, then execute the matching SOP
section.

Run from a monorepo root where `GacUI`, `GacJS`, and `Tools` are sibling
repositories. In the commands below, `<GacUI>` and `<GacJS>` mean those checkout
roots.

## Test Matrix

The Core application and renderer-transport columns are independent dimensions.
Test their Cartesian product with fresh processes. `/RVMT` adds five host modes:
the native C++ host over the renderer network transport, the native C++ host
auto-launched over stdio, the GacJS host inside `?rvmhost`, the independently
started GacJS Node network host, and the GacJS Node SEA auto-launched over
stdio. Do not multiply that host dimension across `/FCT` or `/RPT`.
`CppTest_Rvm` is excluded because it renders locally and needs no remote
renderer. `/Pipe` is excluded because a fetch-based browser cannot use it.

| Platform | Core application dimension | Transport dimension | Total targets |
| --- | --- | --- | --- | --- |
| Windows | `/RPT`, `/FCT`, `/RVMT` (five host modes) | `/Http`, `/MiniHttp` | 14 |
| Linux | `/RPT`, `/FCT`, `/RVMT` (five host modes) | `/MiniHttp` | 7 |
| macOS | `/RPT`, `/FCT`, `/RVMT` (five host modes) | `/MiniHttp` | 7 |

Every `/RVMT` target includes exactly one host. Native manual mode uses
`RemotingTest_RvmHost` on the same transport; native stdio mode passes that
executable to Core. The three GacJS modes instead use `?rvmhost`, the Node
network CLI, or the platform-native Node SEA. In either stdio mode, Core launches
`<path> /Cli`, so do not start the host manually. Installed Safari is a supplementary macOS
compatibility check, not another required matrix row and not a substitute for
Playwright WebKit.

The Linux/macOS `/Cli` rows describe required cross-platform code and runtime
procedure but were not runtime-verified during the Windows implementation task.

Use Playwright Chromium for Windows.
Use Playwright Firefox for Linux.
Use Playwright WebKit for macOS.

### Test Matrix Card

When instructed to create a test matrix card for recording each round of results,
the file format looks like this:

```markdown
# Test Matrix Card <DATE-TIME>

## Test Matrix

| Test Item | 1st | 2nd | ... |
| --- | --- | --- | --- |
| [Windows][`/RPT`][`/Http`] | <RESULT-1st> | <RESULT-2nd> | ... |
...

## Issues Found and Fix

### <TEST-ITEM> <x-nd> <start-time-of-the-test>

describes the failure, cause of the failure and the fix to resolve it

...
```

- When creating a new test matrix card:
  - You should know what platform you are currently in, and only copy test items for that platform, but make sure to copy the complete subset.
  - Prepare an empty 1st column.
  - Add an empty `## Issues Found and Fix` section.
- When a test item is running, the <RESULT-x-nd> is "<start-time> (running)"
- When a test item pass through the test, the <RESULT-x-nd> is "<start-time>".
- When a test item fails, the <RESULT-x-nd> is "X". After the fix is working, it becomes "<start-time> (fixed)"
- When a test item is not executed, the <RESULT-x-nd> will be blank.
- Only when a test item fail, a log to "Issues Found and Fix" is required.
  - Different x-nd of the same test item has its own section.
  - New sections is appended to the end of the file, no ordering is required.
- 1st, 2nd, 3rd means, if in the same request, the test matrix is started over, a new column is added to record the result.
  - Columns are added only when needed.

## Required Reading

Read the current repository instructions before changing or building code:

- `<GacUI>/Project.md`
- `<GacUI>/.github/copilot-instructions.md` and its linked build, run, debugging,
  and computer-use guidelines
- `<GacJS>/AGENTS.md`
- `<GacJS>/doc/Testing_Protocol.md`

`Testing_Protocol.md` and the checked-in GacJS protocol E2E harness currently
describe the Windows executable lifecycle and Playwright Chromium. Use the
platform sections below for portable core startup and static hosting.

## Core and Website Model

`RemotingTest_Core` accepts one application selector (default `/FCT`) and
requires one transport selector:

| Argument | Meaning |
| --- | --- |
| `/FCT` | FullControlTest. This is the default application. |
| `/RPT` | RemoteProtocolTest. |
| `/RVMT` | RemoteViewModelTest. Requires exactly one compatible remote view-model host; see the native and GacJS host modes below. |
| `/Http` | The Windows full-HTTP transport. |
| `/MiniHttp` | The portable async-socket MiniHTTP transport. Use this exact spelling. |
| `/Pipe` | The Windows named-pipe transport. A fetch-based browser cannot use it. |
| `/Cli:<path>` | With explicit `/RVMT`, auto-launch `<path> /Cli` on a separate stdio host server. It does not select the browser transport. |

`/FCT`, `/RPT`, and `/RVMT` are exclusive, as are the transport arguments.
`/Cli` is optional, at most once, and valid only with explicit `/RVMT` plus a
renderer transport. The path may be passed as `/Cli:<path>` or as the single
literal argument `/Cli:"<path>"`; exactly one balanced quote pair is removed,
while empty or unmatched quoted paths are rejected. Start Core before opening
`http://localhost:8896/index.html`.

The protocol endpoint is fixed at port `8888`. In MiniHTTP mode, the core also
registers its `/Automation/RemotingTest_Core/...` routes on that listener. GacJS
is a separate static website served on port `8896`; `RemotingTest_Core` does not
host it.

Only one remote renderer is active at a time. Opening another browser instance
transfers the existing application state to the new renderer and detaches the
old one. Use a fresh core process between transport or application
combinations unless renderer replacement is the scenario being tested.

## Build and Test GacJS

Synchronize, generate, build, and test from the GacJS workspace root:

```text
cd <GacJS>/Gaclib
yarn run import
yarn codegen
yarn build
yarn test
```

The website is generated in `<GacJS>/Gaclib/website/entry/lib/dist`. That
directory must be the static server's document root because the generated HTML
uses root-relative asset URLs.

Run `yarn codegen` a second time before review and require no additional diff.

On Windows this includes the checked-in protocol E2E tests. On Linux and macOS,
the website-entry package reports that the Windows-only E2E tests are skipped;
the portable renderer and remote-protocol unit-test packages still run. This
successful skip is not live browser verification.

## GacJS View-Model Host Modes

Use one of these mutually exclusive GacJS host modes for an `/RVMT` matrix row:

1. Browser host: start Core without `/Cli` or another host and navigate to
   `http://localhost:8896/index.html?rvmhost`. The static server does not process
   the query. The page first connects the generated `IViewModel` service, waits
   until Core holds it, and then creates a fresh, distinct renderer client.
2. Node network host: after Core starts, run the normal Node CLI independently:

   ```text
   node <GacJS>/Gaclib/website/rvmhost/lib/src/cli.js
   ```

   Wait for exact `GACJS_RVMHOST_READY`, then navigate to ordinary
   `index.html`. `--origin <url>` and `--base-path <path>` select the HTTP
   endpoint. `--service-only` prints only
   `GACJS_RVMHOST_SERVICE_HELD` and is used with `CppTest_Rvm`, which has no
   renderer. Do not pass Core's `/Http` or `/MiniHttp` spellings to this CLI.
3. Core-launched Node stdio host: pass the absolute native launcher emitted by
   the `rvmhost` build:

   ```text
   <GacJS>/Gaclib/website/rvmhost/lib/bin/gacjs-rvmhost.exe   # Windows
   <GacJS>/Gaclib/website/rvmhost/lib/bin/gacjs-rvmhost       # Linux/macOS
   ```

   Use that file as Core's `/Cli:<path>` value and navigate to ordinary
   `index.html` only after Core automation contains exact
   `Remote View Model Test`. Core appends exact ` /Cli`; the child reserves
   stdin/stdout for Base64 protocol lines and emits no readiness marker.

The package's npm bin and `lib/src/cli.js` are normal Node launchers, not valid
Core `/Cli:<path>` values. Do not pass `node <script>` as the path. The native
SEA is one executable and supports paths containing spaces. With a process API,
pass `/Cli:<absolute path>` as one argument without shell quoting; a literal
`/Cli:"<absolute path>"` argument is also accepted. Build and test the SEA on the
same platform where it runs.

When Core launches `/Cli:<path>` on Linux or macOS, its inherited `/bin/sh -c`
launcher handles ordinary spaces through the surrounding quotes, but paths
containing shell-special characters are not robustly supported.

For every GacJS host mode, perform the Remote View Model SOP: require
`Hello, !`, type a unique marker, and require `Hello, <marker>!`. Replace only
the renderer and require the accepted host and subsequent `Translate` calls to
remain alive. In a separate failure run, retain the renderer, terminate only the
accepted host, trigger another `Translate`, and require exact
`RemotingTest_RvmHost disconnected.` from Core. Graceful stdio verification must
POST exact `!Exit` to Core automation and require its SEA child to be reaped;
force-killing Core does not verify this shutdown path.

## Operating the Browser

Drive input through the rendered page, not through Core `/IO`. A click or key
event sent to the page and reflected in a visible state change exercises both
directions of the remote protocol.

For interactive operation:

1. Open `index.html` and wait for the application title and live main content,
   not merely an HTTP 200 response.
2. Inspect fresh visible DOM state before every action. Tabs, menus, dialogs,
   and renderer replacement change the active tree.
3. Click the enclosing interactive element rather than a child text node. Focus
   the intended editor before typing through the browser keyboard.
4. Require the expected visible state after every action and capture a
   screenshot when the result is ambiguous.

For programmatic Playwright operation:

- Register dialog, console-error, and page-error handlers before `page.goto()`.
- Register the GacJS renderer-idle and caret-blink callbacks before navigation.
- Wait for the first renderer-idle event and substantial GacUI content before
  acting. Wait for renderer idle after each click or keystroke.
- Re-query the DOM after tabs, menus, dialogs, and renderer replacement.
- Do not use arbitrary sleeps for UI synchronization.

The shared SOP defines the required renderer-replacement and application-close
results. Retain Core output and browser diagnostics while performing those
operations.

For low-level protocol diagnosis, temporarily enable `PRINT_PROTOCOL_JSON` in
`<GacUI>/Test/GacUISrc/RemotingTest_Core/CoreChannel.cpp`. Rebuild, reproduce,
and revert the logging change before committing.

## Windows

Build the GacUI solution only through the supported script:

```powershell
Set-Location <GacUI>\Test\GacUISrc
& <GacUI>\.github\Scripts\copilotBuild.ps1
```

The Core executable is
`<GacUI>\Test\GacUISrc\x64\Debug\RemotingTest_Core.exe`. For the two native
`/RVMT` host modes, the corresponding host executable is
`<GacUI>\Test\GacUISrc\x64\Debug\RemotingTest_RvmHost.exe`. The following
examples are separate runs:

```powershell
$coreExe = '<GacUI>\Test\GacUISrc\x64\Debug\RemotingTest_Core.exe'

# Portable MiniHTTP implementation
$core = Start-Process -FilePath $coreExe -ArgumentList '/MiniHttp','/FCT' -PassThru

# Full Windows HTTP implementation
$core = Start-Process -FilePath $coreExe -ArgumentList '/Http','/FCT' -PassThru
```

Substitute `/RPT` for Remote Protocol Test. For the native network-host `/RVMT`
mode, start the matching host after Core and wait until Core automation exposes
the `Remote View Model Test` window before opening the browser:

```powershell
$hostExe = '<GacUI>\Test\GacUISrc\x64\Debug\RemotingTest_RvmHost.exe'
$core = Start-Process -FilePath $coreExe -ArgumentList '/Http','/RVMT' -PassThru
$host = Start-Process -FilePath $hostExe -ArgumentList '/Http' -PassThru
```

Repeat that order with `/MiniHttp`. Exercise every application/transport target
as a separate GacJS run. Do not call MSBuild directly.

For each `/RVMT` renderer transport, also run the native `/Cli` host mode. Core
owns the child process; wait for its RVM window and then open the browser:

```powershell
$core = Start-Process -FilePath $coreExe -ArgumentList '/Http','/RVMT',('/Cli:"{0}"' -f $hostExe) -PassThru
```

Run the three GacJS-host variants separately. Browser-host mode starts only
Core and opens the query page. Network mode starts Core, then the Node CLI and
waits for its ready line. Stdio mode gives Core the SEA path and starts no host
manually:

```powershell
$nodeCli = '<GacJS>\Gaclib\website\rvmhost\lib\src\cli.js'
$sea = '<GacJS>\Gaclib\website\rvmhost\lib\bin\gacjs-rvmhost.exe'

# Browser host: then open /index.html?rvmhost
$core = Start-Process -FilePath $coreExe -ArgumentList '/Http','/RVMT' -PassThru

# Independent network host: wait for GACJS_RVMHOST_READY, then open /index.html
$core = Start-Process -FilePath $coreExe -ArgumentList '/Http','/RVMT' -PassThru
$host = Start-Process -FilePath node -ArgumentList $nodeCli -PassThru -NoNewWindow

# Core-launched SEA: wait for the RVM automation window, then open /index.html
$core = Start-Process -FilePath $coreExe -ArgumentList '/Http','/RVMT',('/Cli:"{0}"' -f $sea) -PassThru
```

Repeat all three with `/MiniHttp`. When automating `Start-Process`, retain and
parse the Node network host's redirected stdout rather than treating process
creation or `GACJS_RVMHOST_SERVICE_HELD` as renderer readiness.

After `yarn build`, start the checked-in website server:

```powershell
Set-Location <GacJS>\Gaclib\website\entry
npm run start
```

It serves `lib\dist` at `http://localhost:8896` and waits for ENTER to stop.
If IIS already owns port `8896`, the command explains that IIS may already be
hosting the website; check the URL directly. The checked-in automated protocol
harness uses the Windows Debug x64 core and Playwright Chromium.

Core automation is available at
`http://localhost:8888/Automation/RemotingTest_Core/...`. The native renderer's
automation port is selected by its optional `/port:<port>` argument and defaults
to `8889`; it is not used by GacJS.

Close the browser, stop the retained Core, and then stop a retained host if it
has not already exited:

```powershell
if ($core -and -not $core.HasExited) { Stop-Process -Id $core.Id -Force }
if ($host -and -not $host.HasExited) { Stop-Process -Id $host.Id -Force }
```

## Linux and macOS

The portable build entry and website server are shared by Linux and macOS.
Build and start the core, retaining its process identifier:

```bash
cd <GacUI>/Test/Linux/RemotingTest_Core
../../../.github/Ubuntu/build.sh
./Bin/RemotingTest_Core /MiniHttp /FCT &
core_pid=$!
```

Build the portable host before a native `/RVMT` run:

```bash
cd <GacUI>/Test/Linux/RemotingTest_RvmHost
../../../.github/Ubuntu/build.sh
```

After `yarn build` has completed, start the website in another terminal:

```bash
cd <GacJS>/Gaclib/website/entry
npm run start
```

It serves `lib/dist` at `http://localhost:8896` and waits for ENTER to stop.
Substitute `/RPT` for Remote Protocol Test. For the native network-host `/RVMT`
mode, start Core with that selector, then start the host before opening GacJS:

```bash
<GacUI>/Test/Linux/RemotingTest_RvmHost/Bin/RemotingTest_RvmHost /MiniHttp &
host_pid=$!
```

Wait until Core automation exposes the `Remote View Model Test` window before
opening the browser.

For the native `/Cli` row, start Core with an absolute host path and do not start
the host manually:

```bash
host_exe="$(realpath <GacUI>/Test/Linux/RemotingTest_RvmHost/Bin/RemotingTest_RvmHost)"
<GacUI>/Test/Linux/RemotingTest_Core/Bin/RemotingTest_Core /MiniHttp /RVMT "/Cli:$host_exe" &
core_pid=$!
```

Run the three GacJS-host variants as separate portable rows:

```bash
core_exe="<GacUI>/Test/Linux/RemotingTest_Core/Bin/RemotingTest_Core"
node_cli="<GacJS>/Gaclib/website/rvmhost/lib/src/cli.js"
sea="$(realpath <GacJS>/Gaclib/website/rvmhost/lib/bin/gacjs-rvmhost)"

# Browser host: open /index.html?rvmhost
"$core_exe" /MiniHttp /RVMT &
core_pid=$!

# Independent network host: wait for GACJS_RVMHOST_READY, then open /index.html
"$core_exe" /MiniHttp /RVMT &
core_pid=$!
node "$node_cli" &
host_pid=$!

# Core-launched SEA: wait for the RVM automation window, then open /index.html
"$core_exe" /MiniHttp /RVMT "/Cli:$sea" &
core_pid=$!
```

Build the SEA natively on Linux and macOS. Do not copy one platform's launcher
to another. The browser-host and Node-network rows use the same static output;
only the former adds `?rvmhost`.

Open `http://localhost:8896/index.html` only after the core reports that its
MiniHTTP server is waiting on port `8888`. Require the page to load
`index.html`, `global.css`, and `index.js`, and require the core to report that
a renderer connected.

After closing the browser, press ENTER in the website-server terminal. Then
stop the retained Core and any independently started host:

```bash
kill "$core_pid"
wait "$core_pid" 2>/dev/null || true
if [ -n "${host_pid:-}" ]; then
  kill "$host_pid"
  wait "$host_pid" 2>/dev/null || true
fi
unset host_pid
```

### Linux

Use the Linux build, hosting, and cleanup commands above. Firefox is the
required Playwright browser for this platform. Install it from
`<GacJS>/Gaclib` and open the page for each matrix target:

```bash
yarn playwright install firefox
yarn playwright open --browser=firefox http://localhost:8896/index.html
```

Do not substitute Chromium or WebKit in the Linux verification matrix.
These Linux instructions share the portable macOS path but were not executed
during the macOS investigation that introduced the build entry.

### macOS

WebKit is the required Playwright browser for this platform because it is
Playwright's Safari-family target. Install it from `<GacJS>/Gaclib`:

```bash
cd <GacJS>/Gaclib
yarn playwright install webkit
```

Run each required application scenario in Playwright WebKit with a fresh
`/MiniHttp` core:

```bash
yarn playwright open --browser=webkit http://localhost:8896/index.html
```

The command opens an interactive session. When automating the scenario in a
Playwright script, use the programmatic idle, blink, and error-handling rules
above and execute the matching shared SOP section. Do not add Chromium or
Firefox to the macOS verification matrix.

Playwright WebKit is the Safari-family automated target, but it is not the
installed Safari application. Real Safari remains a separate manual check after
the required Playwright run. Keep or restart the static server, stop the WebKit
core, and start a fresh `/MiniHttp` core before running:

```bash
open -a Safari http://localhost:8896/index.html
```

Run that command only when interactive Safari permissions are available.
