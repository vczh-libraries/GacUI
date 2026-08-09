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
Test their Cartesian product with fresh processes. `/RVMT` adds two host modes:
manual host over the renderer network transport, and auto-launched stdio
`/Cli:<path>`. Do not multiply that host dimension across `/FCT` or `/RPT`.
`CppTest_Rvm` is excluded because it renders locally and needs no remote
renderer. `/Pipe` is excluded because a fetch-based browser cannot use it.

| Platform | GacJS renderer | Core application dimension | Transport dimension | Total targets |
| --- | --- | --- | --- | --- |
| Windows | Playwright Chromium | `/RPT`, `/FCT`, `/RVMT` (manual or `/Cli` host) | `/Http`, `/MiniHttp` | 8 |
| Linux | Playwright Firefox | `/RPT`, `/FCT`, `/RVMT` (manual or `/Cli` host) | `/MiniHttp` | 4 |
| macOS | Playwright WebKit | `/RPT`, `/FCT`, `/RVMT` (manual or `/Cli` host) | `/MiniHttp` | 4 |

Every `/RVMT` target also includes `RemotingTest_RvmHost`. In manual mode, start
Core, start the host with the same transport, wait until the application is
ready, and then open GacJS. In `/Cli` mode, pass the absolute host path to Core;
Core launches `<path> /Cli`, so do not start the host manually. Installed Safari is a supplementary macOS
compatibility check, not another required matrix row and not a substitute for
Playwright WebKit.

The Linux/macOS `/Cli` rows describe required cross-platform code and runtime
procedure but were not runtime-verified during the Windows implementation task.

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
| `/RVMT` | RemoteViewModelTest. Requires `RemotingTest_RvmHost`. |
| `/Http` | The Windows full-HTTP transport. |
| `/MiniHttp` | The portable async-socket MiniHTTP transport. Use this exact spelling. |
| `/Pipe` | The Windows named-pipe transport. A fetch-based browser cannot use it. |
| `/Cli:<path>` | With explicit `/RVMT`, auto-launch `<path> /Cli` on a separate stdio host server. It does not select the browser transport. |

`/FCT`, `/RPT`, and `/RVMT` are exclusive, as are the transport arguments.
`/Cli` is optional, at most once, and valid only with explicit `/RVMT` plus a
renderer transport. Start Core before opening `http://localhost:8896/index.html`.

The protocol endpoint is fixed at port `8888`. In MiniHTTP mode, the core also
registers its `/Automation/RemotingTest_Core/...` routes on that listener. GacJS
is a separate static website served on port `8896`; `RemotingTest_Core` does not
host it.

Only one remote renderer is active at a time. Opening another browser instance
transfers the existing application state to the new renderer and detaches the
old one. Use a fresh core process between transport or application
combinations unless renderer replacement is the scenario being tested.

## Build and Test GacJS

Build from the GacJS workspace root:

```text
cd <GacJS>/Gaclib
yarn build
```

The website is generated in `<GacJS>/Gaclib/website/entry/lib/dist`. That
directory must be the static server's document root because the generated HTML
uses root-relative asset URLs.

Run the repository test command from the same directory:

```text
yarn test
```

On Windows this includes the checked-in protocol E2E tests. On Linux and macOS,
the website-entry package reports that the Windows-only E2E tests are skipped;
the portable renderer and remote-protocol unit-test packages still run. This
successful skip is not live browser verification.

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

The executable is
`<GacUI>\Test\GacUISrc\x64\Debug\RemotingTest_Core.exe`; `/RVMT` also uses
`<GacUI>\Test\GacUISrc\x64\Debug\RemotingTest_RvmHost.exe`. The following
examples are separate runs:

```powershell
$coreExe = '<GacUI>\Test\GacUISrc\x64\Debug\RemotingTest_Core.exe'

# Portable MiniHTTP implementation
$core = Start-Process -FilePath $coreExe -ArgumentList '/MiniHttp','/FCT' -PassThru

# Full Windows HTTP implementation
$core = Start-Process -FilePath $coreExe -ArgumentList '/Http','/FCT' -PassThru
```

Substitute `/RPT` for Remote Protocol Test. For `/RVMT`, start the matching host
after Core and wait until Core automation exposes the
`Remote View Model Test` window before opening the browser:

```powershell
$hostExe = '<GacUI>\Test\GacUISrc\x64\Debug\RemotingTest_RvmHost.exe'
$core = Start-Process -FilePath $coreExe -ArgumentList '/Http','/RVMT' -PassThru
$host = Start-Process -FilePath $hostExe -ArgumentList '/Http' -PassThru
```

Repeat that order with `/MiniHttp`. Exercise every application/transport target
as a separate GacJS run. Do not call MSBuild directly.

For each `/RVMT` renderer transport, also run the `/Cli` host mode. Core owns
the child process; wait for its RVM window and then open the browser:

```powershell
$core = Start-Process -FilePath $coreExe -ArgumentList '/Http','/RVMT',('/Cli:"{0}"' -f $hostExe) -PassThru
```

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

Build the portable host before an `/RVMT` run:

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
Substitute `/RPT` for Remote Protocol Test. For `/RVMT`, start Core with that
selector, then start the host before opening GacJS:

```bash
<GacUI>/Test/Linux/RemotingTest_RvmHost/Bin/RemotingTest_RvmHost /MiniHttp &
host_pid=$!
```

Wait until Core automation exposes the `Remote View Model Test` window before
opening the browser.

For the additional `/Cli` row, start Core with an absolute host path and do not
start the host manually:

```bash
host_exe="$(realpath <GacUI>/Test/Linux/RemotingTest_RvmHost/Bin/RemotingTest_RvmHost)"
<GacUI>/Test/Linux/RemotingTest_Core/Bin/RemotingTest_Core /MiniHttp /RVMT "/Cli:$host_exe" &
core_pid=$!
```

Open `http://localhost:8896/index.html` only after the core reports that its
MiniHTTP server is waiting on port `8888`. Require the page to load
`index.html`, `global.css`, and `index.js`, and require the core to report that
a renderer connected.

After closing the browser, press ENTER in the website-server terminal. Then
stop only the retained core process:

```bash
kill "$core_pid"
wait "$core_pid" 2>/dev/null || true
if [ -n "${host_pid:-}" ]; then
  kill "$host_pid"
  wait "$host_pid" 2>/dev/null || true
fi
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
