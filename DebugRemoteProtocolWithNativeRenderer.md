# Operating GacUI With a Native Remote Renderer

This guide records how to build and start the native test applications, inspect
a native remote renderer, and send renderer-side input. The feature operations,
error injections, and pass/fail observations are defined only in
[`DebugRemoteProtocolSop.md`](DebugRemoteProtocolSop.md). Use this guide to
establish each required process topology, then execute the matching SOP section.

## Test Matrix

The application and renderer-transport columns are independent dimensions.
Test their Cartesian product. `/RVMT` adds a Core-to-host mode dimension:
manual host over the selected network transport, or auto-launched stdio
`/Cli:<path>`. Do not multiply that host dimension across `/FCT` or `/RPT`.
Use fresh processes for each target.

| Platform | Standalone RVM requester targets | Remote Core application dimension | Transport dimension | Total targets |
| --- | --- | --- | --- | --- |
| Windows | `CppTest_Rvm`: manual `/Pipe`, `/Http`, `/MiniHttp`, or auto `/Cli:<path>` | `RemotingTest_Core` + `RemotingTest_Rendering_Win32`: `/RPT`, `/FCT`, and `/RVMT` in manual or `/Cli` host mode | `/Pipe`, `/Http`, `/MiniHttp` | 4 standalone + 12 Core = 16 |
| Linux | wGac `Test_CppTest_Rvm` + manual host | `RemotingTest_Core` + `RemotingTest_Rendering_Wayland`: `/RPT`, `/FCT`, and `/RVMT` in manual or `/Cli` host mode | `/MiniHttp` | 1 standalone + 4 Core = 5 |
| macOS | iGac `Test_CppTest_Rvm` + manual host | `RemotingTest_Core` + `RemotingTest_Rendering_macOS`: `/RPT`, `/FCT`, and `/RVMT` in manual or `/Cli` host mode | `/MiniHttp` | 1 standalone + 4 Core = 5 |

Every `/RVMT` target includes `RemotingTest_RvmHost`. In manual mode, start it
after the requester/Core with the same network selector and before the renderer.
In `/Cli` mode, pass its absolute executable path to the requester/Core; the
application quotes the path, appends exact `/Cli`, and launches the host itself.
Do not also start a manual host in that mode. Standalone requesters do not use a
remote renderer. The wGac and iGac standalone launchers remain fixed to manual
`/MiniHttp`.

The Linux `/Cli` row has been runtime-verified, including normal shutdown and
both host-loss variants in the fatal regression addendum. The macOS `/Cli` row
describes the required code and runtime procedure but still requires runtime
verification on macOS.

For a Core target, the Core must start before the renderer. The application
selector belongs only to Core, while the renderer receives the transport
selector and optional `/port:<port>` automation-listener argument. Omitting the
port keeps the default `8889`. Run one target at a time and retain every process
identifier for cleanup.

## Windows

Read `GacUI/Project.md`, `GacUI/.github/copilot-instructions.md`, and the linked
build, run, debugging, and computer-use guidelines before starting. Build only
through the supported script:

```powershell
Push-Location GacUI\Test\GacUISrc
try {
    & ..\..\.github\Scripts\copilotBuild.ps1
}
finally {
    Pop-Location
}
```

The executables are:

```text
GacUI\Test\GacUISrc\x64\Debug\CppTest_Rvm.exe
GacUI\Test\GacUISrc\x64\Debug\RemotingTest_Core.exe
GacUI\Test\GacUISrc\x64\Debug\RemotingTest_Rendering_Win32.exe
GacUI\Test\GacUISrc\x64\Debug\RemotingTest_RvmHost.exe
```

Start a pair from the monorepo root. These examples use `/RPT`; substitute
`/FCT` or `/RVMT` for the other Core application targets.

```powershell
$bin = (Resolve-Path GacUI\Test\GacUISrc\x64\Debug).Path

# Full Windows HTTP implementation
$core = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Core.exe') -ArgumentList '/Http','/RPT' -PassThru
$renderer = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Rendering_Win32.exe') -ArgumentList '/Http','/port:8890' -PassThru

# Async-socket MiniHTTP implementation
$core = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Core.exe') -ArgumentList '/MiniHttp','/RPT' -PassThru
$renderer = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Rendering_Win32.exe') -ArgumentList '/MiniHttp','/port:8890' -PassThru

# Named pipe implementation
$core = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Core.exe') -ArgumentList '/Pipe','/RPT' -PassThru
$renderer = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Rendering_Win32.exe') -ArgumentList '/Pipe','/port:8890' -PassThru
```

For a non-CLI Core `/RVMT` target, start the three processes in this order, replacing
`/Http` with the selected transport. Wait until Core automation exposes the
`Remote View Model Test` window before starting the renderer:

```powershell
$core = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Core.exe') -ArgumentList '/Http','/RVMT' -PassThru
$host = Start-Process -FilePath (Join-Path $bin 'RemotingTest_RvmHost.exe') -ArgumentList '/Http' -PassThru
$renderer = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Rendering_Win32.exe') -ArgumentList '/Http','/port:8890' -PassThru
```

For a Core `/Cli` target, Core owns and auto-launches the host. Keep the selected
renderer transport, pass the host path only to Core, wait for the RVM window,
and then start the renderer:

```powershell
$hostExe = (Join-Path $bin 'RemotingTest_RvmHost.exe')
$core = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Core.exe') -ArgumentList '/Http','/RVMT',('/Cli:"{0}"' -f $hostExe) -PassThru
$renderer = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Rendering_Win32.exe') -ArgumentList '/Http','/port:8890' -PassThru
```

For a standalone RVM requester target, omit Core and the renderer. Start the
requester before its host, again using the same selected transport in both:

```powershell
$requester = Start-Process -FilePath (Join-Path $bin 'CppTest_Rvm.exe') -ArgumentList '/Http' -PassThru
$host = Start-Process -FilePath (Join-Path $bin 'RemotingTest_RvmHost.exe') -ArgumentList '/Http' -PassThru
```

The standalone `/Cli` target is one process command; it auto-launches the host
and continues to expose Windows HTTP automation on port 8888:

```powershell
$hostExe = (Join-Path $bin 'RemotingTest_RvmHost.exe')
$requester = Start-Process -FilePath (Join-Path $bin 'CppTest_Rvm.exe') -ArgumentList ('/Cli:"{0}"' -f $hostExe) -PassThru
```

The examples are separate runs, not one script. Start the selected core, wait
for its server-created message, and only then start the matching renderer. A
successful connection prints:

```text
> Waiting for a renderer ...
> Renderer connected: 2
```

HTTP creation reports
`http://localhost:8888/GacUIRemoteProtocolHttp`; MiniHTTP reports the same URL
with `Mini HTTP server created`; named pipe reports
`GacUIRemoteProtocolNamedPipe`.

### HTTP Automation

Automation endpoints are available in every Windows transport mode:

```text
GET  http://localhost:8888/Automation/RemotingTest_Core/Controls
POST http://localhost:8888/Automation/RemotingTest_Core/IO
GET  http://localhost:8890/Automation/RemotingTest_Rendering_Native/Dom
POST http://localhost:8890/Automation/RemotingTest_Rendering_Native/IO
```

Standalone `CppTest_Rvm`, including `/Cli`, uses
`http://localhost:8888/Automation/CppTest_Rvm/Controls` and `/IO`.

During `/Http` and `/Pipe` runs, the projects use the Windows HTTP automation
service. During a `/MiniHttp` run, `RemotingTest_Core` registers its automation
prefix with the exact same `IAsyncSocketServer` that hosts the remote protocol
on port `8888`; it does not create another listener. The renderer is a separate
process and cannot share that server instance, so it starts a separate MiniHTTP
automation server on the port selected by `/port:<port>`. The examples select
`8890`; without `/port:`, the renderer uses `8889`.

`Controls` describes logical GacUI controls; `Dom` describes what the native
renderer received. Search the latest JSON for the visible text, walk upward to
the nearest enclosing interactive object with bounds, and click the integer
center:

```text
x = floor((x1 + x2) / 2)
y = floor((y1 + y2) / 2)
```

Post commands as `application/json; charset=utf8`:

```powershell
Invoke-WebRequest `
  -UseBasicParsing `
  -Method Post `
  -Uri http://localhost:8890/Automation/RemotingTest_Rendering_Native/IO `
  -ContentType 'application/json; charset=utf8' `
  -SkipHeaderValidation `
  -Body '!LeftClick:<integer-x>,<integer-y>'
```

Use renderer `/IO`, or actual native mouse and keyboard input, for the shared
verification scenario. Renderer-side input crosses the network to the core;
Core `/IO` alone bypasses that half of the path. Core `/IO` remains useful for
diagnosis and for deliberately checking the opposite direction.

Re-read both trees after opening a tab, menu, or dialog. A response of `Queued`
only means the input was accepted; require the expected state change. The
renderer response can retain hidden entries in its `Elements` catalog, so also
require the matching element to be reachable from the active `Dom` tree.

### Debugging and Cleanup

Keep both consoles available when investigating connection or shutdown errors.
If a debugger is needed, use the scripts in `GacUI/.github/Scripts` as directed
by `GacUI/.github/Guidelines/Debugging.md`.

The shared SOP defines the required close and fatal-error observations. For
diagnosis, a fatal local channel error such as a pipe closure or HTTP 404 enters
the ordinary disconnected state directly; only a Core-authored `!Error` package
presents the fatal prompt or overlay.

Clean up only the processes retained for the run:

```powershell
if ($renderer -and -not $renderer.HasExited) { Stop-Process -Id $renderer.Id -Force }
if ($requester -and -not $requester.HasExited) { Stop-Process -Id $requester.Id -Force }
if ($core -and -not $core.HasExited) { Stop-Process -Id $core.Id -Force }
if ($host -and -not $host.HasExited) { Stop-Process -Id $host.Id -Force }
```

If process identifiers were lost, first inspect all matching processes and their
start times before using a name-based fallback.

## Linux Specific

The Linux native renderer is `RemotingTest_Rendering_Wayland` in the sibling
`wGac` repository. It uses `/MiniHttp` only; `/Http` and `/Pipe` are not part of
the Linux contract. Its automation port defaults to `8889`. The wGac launcher
accepts `--port:<port>` and forwards it as the renderer's `/port:` automation
option; this changes only the renderer automation listener, not the Core
connection on port 8888.

From the monorepo root, build the portable Core and host and the wGac native
applications:

```bash
(
  cd GacUI/Test/Linux/RemotingTest_Core
  ../../../.github/Ubuntu/build.sh
)
(
  cd GacUI/Test/Linux/RemotingTest_RvmHost
  ../../../.github/Ubuntu/build.sh
)
(
  cd wGac
  ./build.sh
)
```

For the standalone RVM requester target, start the wGac application first and
then its portable host:

```bash
wGac/test.sh --app:rvmt --unblock
GacUI/Test/Linux/RemotingTest_RvmHost/Bin/RemotingTest_RvmHost /MiniHttp
```

For a Core target, run Core in the first terminal. Replace `/RPT` with `/FCT`
or `/RVMT` for the other application targets:

```bash
GacUI/Test/Linux/RemotingTest_Core/Bin/RemotingTest_Core /MiniHttp /RPT
```

For `/RVMT`, start the host in another terminal and wait until Core automation
exposes the `Remote View Model Test` window before starting the renderer:

```bash
GacUI/Test/Linux/RemotingTest_RvmHost/Bin/RemotingTest_RvmHost /MiniHttp
```

For the additional Core `/Cli` row, use an absolute host path and do not start
the host manually:

```bash
host_exe="$(realpath GacUI/Test/Linux/RemotingTest_RvmHost/Bin/RemotingTest_RvmHost)"
GacUI/Test/Linux/RemotingTest_Core/Bin/RemotingTest_Core /MiniHttp /RVMT "/Cli:$host_exe"
```

Wait for `Waiting for a renderer ...`, then run the renderer in another
terminal:

```bash
wGac/test.sh --app:renderer
```

For example, a concurrent takeover renderer can use a second automation port:

```bash
wGac/test.sh --app:renderer --port:8890
```

The corresponding automation endpoints are:

```text
GET  http://localhost:8888/Automation/RemotingTest_Core/Controls
POST http://localhost:8888/Automation/RemotingTest_Core/IO
GET  http://localhost:8889/Automation/RemotingTest_Rendering_Native/Dom
POST http://localhost:8889/Automation/RemotingTest_Rendering_Native/IO
```

Post renderer commands with `curl` as shown in the macOS example below. Use
renderer `/IO` for the shared SOP so both input and rendering cross the remote
protocol. Re-read core `Controls` and renderer `Dom` after every state change.

To replace the renderer, stop only the renderer process and start it again on
port 8889 while keeping the core alive. To test takeover, keep that renderer
running and start another one with `--port:8890`; inspect the new renderer at
the same automation prefix on port 8890. Follow the shared SOP for all state
continuity, detached-renderer, and application-close observations.

The raw Wayland renderer has no `GuiApplication` in which to display a fatal
message dialog. A Core-authored fatal error is retained directly in the renderer
DOM and exposed as `fatalError`; exact `!Exit` remains available for cleanup.

## macOS Specific

The macOS native renderer is `RemotingTest_Rendering_macOS` in the sibling
`iGac` repository. It uses `/MiniHttp` only; `/Http` and `/Pipe` are not part of
the macOS contract. Its automation port defaults to `8889`; the iGac launcher
accepts `--port:<port>` and forwards it as the renderer's `/port:` automation
option without changing the Core connection on port 8888.

From the monorepo root, build the portable Core and host and the iGac native
applications:

```bash
(
  cd GacUI/Test/Linux/RemotingTest_Core
  ../../../.github/Ubuntu/build.sh
)
(
  cd GacUI/Test/Linux/RemotingTest_RvmHost
  ../../../.github/Ubuntu/build.sh
)
(
  cd iGac
  ./build.sh
)
```

For the standalone RVM requester target, start the iGac application first and
then its portable host:

```bash
iGac/test.sh --app:rvmt --unblock
GacUI/Test/Linux/RemotingTest_RvmHost/Bin/RemotingTest_RvmHost /MiniHttp
```

For a Core target, run Core in the first terminal. Replace `/RPT` with `/FCT`
or `/RVMT` for the other application targets:

```bash
GacUI/Test/Linux/RemotingTest_Core/Bin/RemotingTest_Core /MiniHttp /RPT
```

For `/RVMT`, start the host in another terminal and wait until Core automation
exposes the `Remote View Model Test` window before starting the renderer:

```bash
GacUI/Test/Linux/RemotingTest_RvmHost/Bin/RemotingTest_RvmHost /MiniHttp
```

For the additional Core `/Cli` row, use an absolute host path and do not start
the host manually:

```bash
host_exe="$(realpath GacUI/Test/Linux/RemotingTest_RvmHost/Bin/RemotingTest_RvmHost)"
GacUI/Test/Linux/RemotingTest_Core/Bin/RemotingTest_Core /MiniHttp /RVMT "/Cli:$host_exe"
```

Wait for `Waiting for a renderer ...`, then run the renderer in another
terminal:

```bash
iGac/test.sh --app:renderer
```

For example, a concurrent takeover renderer can use a second automation port:

```bash
iGac/test.sh --app:renderer --port:8890
```

The corresponding automation endpoints are:

```text
GET  http://localhost:8888/Automation/RemotingTest_Core/Controls
POST http://localhost:8888/Automation/RemotingTest_Core/IO
GET  http://localhost:8889/Automation/RemotingTest_Rendering_Native/Dom
POST http://localhost:8889/Automation/RemotingTest_Rendering_Native/IO
```

Post renderer commands with `curl`, for example:

```bash
curl \
  -X POST \
  -H 'Content-Type: application/json; charset=utf8' \
  --data-binary '!LeftClick:<integer-x>,<integer-y>' \
  http://localhost:8889/Automation/RemotingTest_Rendering_Native/IO
```

Use renderer `/IO` for the shared SOP so both input and rendering cross the
remote protocol. Re-read core `Controls` and renderer `Dom` after every state
change.

AppKit window and input coordinates are logical points. The native protocol
screen configuration therefore reports scaling `1.0` on macOS; CoreGraphics
applies the Retina backing scale separately while drawing. Do not multiply
remote bounds or input coordinates by the backing scale. If the complete UI
appears in only the top-left quarter of a window and pointer coordinates behave
as if divided by two, inspect the remote screen scaling rather than changing the
ordinary Cocoa rendering path.

To replace the renderer, stop only the renderer process and start it again on
port 8889 while keeping the core alive. To test takeover, keep that renderer
running and start another one with `--port:8890`; inspect the new renderer at
the same automation prefix on port 8890. Follow the shared SOP for all state
continuity, detached-renderer, and application-close observations.
