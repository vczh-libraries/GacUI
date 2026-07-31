# Operating GacUI With a Native Remote Renderer

This guide explains how to start a native renderer, inspect its remote UI, and
send real renderer-side input. The shared UI operations and observable results
are defined in [`DebugRemoteProtocolSop.md`](DebugRemoteProtocolSop.md). The required transport
matrix and pass/fail criteria are defined in
[`../Tools/Jobs/job.verifyRemoteProtocol.prompt.md`](../Tools/Jobs/job.verifyRemoteProtocol.prompt.md).
Use this document as the native-renderer operation entry point on every
platform; its platform section states the currently available implementation.

The core must start before the renderer. The application selector (`/FCT` or
`/RPT`) belongs only to the core, while the renderer receives the matching
transport selector. Run one core/renderer pair at a time and retain both process
identifiers for cleanup.

The native-renderer transport contract is:

| Platform | Transport arguments | Current implementation status |
| --- | --- | --- |
| Windows | `/MiniHttp`, `/Http`, `/Pipe` | Available |
| Linux | `/MiniHttp` | Native renderer not implemented yet |
| macOS | `/MiniHttp` | Available in the sibling `iGac` repository |

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
GacUI\Test\GacUISrc\x64\Debug\RemotingTest_Core.exe
GacUI\Test\GacUISrc\x64\Debug\RemotingTest_Rendering_Win32.exe
```

Start a pair from the monorepo root. These examples use `/RPT`; substitute
`/FCT` when required by the verification job.

```powershell
$bin = (Resolve-Path GacUI\Test\GacUISrc\x64\Debug).Path

# Full Windows HTTP implementation
$core = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Core.exe') -ArgumentList '/Http','/RPT' -PassThru
$renderer = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Rendering_Win32.exe') -ArgumentList '/Http' -PassThru

# Async-socket MiniHTTP implementation
$core = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Core.exe') -ArgumentList '/MiniHttp','/RPT' -PassThru
$renderer = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Rendering_Win32.exe') -ArgumentList '/MiniHttp' -PassThru

# Named pipe implementation
$core = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Core.exe') -ArgumentList '/Pipe','/RPT' -PassThru
$renderer = Start-Process -FilePath (Join-Path $bin 'RemotingTest_Rendering_Win32.exe') -ArgumentList '/Pipe' -PassThru
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
GET  http://localhost:8889/Automation/RemotingTest_Rendering_Win32/Dom
POST http://localhost:8889/Automation/RemotingTest_Rendering_Win32/IO
```

During `/Http` and `/Pipe` runs, the projects use the Windows HTTP automation
service. During a `/MiniHttp` run, `RemotingTest_Core` registers its automation
prefix with the exact same `IAsyncSocketServer` that hosts the remote protocol
on port `8888`; it does not create another listener. The renderer is a separate
process and cannot share that server instance, so it starts a separate MiniHTTP
automation server on port `8889`.

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
  -Uri http://localhost:8889/Automation/RemotingTest_Rendering_Win32/IO `
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

After a normal close, the renderer must settle without an `ERROR from GacUI
Core` dialog. A fatal local channel error, such as a pipe closure or an HTTP 404
after connection, is itself terminal: the renderer enters its ordinary
disconnected state directly and does not wait for `OnDisconnected`. Only a
Core-authored `!Error` package presents the fatal prompt or overlay.

Clean up only the processes retained for the run:

```powershell
if ($renderer -and -not $renderer.HasExited) { Stop-Process -Id $renderer.Id -Force }
if ($core -and -not $core.HasExited) { Stop-Process -Id $core.Id -Force }
```

If process identifiers were lost, first inspect all matching processes and their
start times before using a name-based fallback.

## Linux Specific

The planned native-renderer transport for Linux is `/MiniHttp`; `/Http` and
`/Pipe` are not part of the planned platform contract. Native remote rendering is
not implemented yet, so the equivalent renderer and its concrete
start/inspection procedure must be provided by another repository before the job
can run. Do not attempt to run `RemotingTest_Rendering_Win32` on Linux.

## macOS Specific

The macOS native renderer is `RemotingTest_Renderer_macOS` in the sibling
`iGac` repository. It uses `/MiniHttp` only; `/Http` and `/Pipe` are
Windows-only.

From the monorepo root, build the portable core and the iGac renderer:

```bash
(
  cd GacUI/Test/Linux/RemotingTest_Core
  ../../../.github/Ubuntu/build.sh
)
(
  cd iGac
  ./build.sh
)
```

Run the core in the first terminal. Use `/RPT` for Remote Protocol Test, or
replace it with `/FCT` for Full Control Test:

```bash
GacUI/Test/Linux/RemotingTest_Core/Bin/RemotingTest_Core /MiniHttp /RPT
```

Wait for `Waiting for a renderer ...`, then run the renderer in a second
terminal:

```bash
iGac/build/RemotingTest_Renderer_macOS/bin/RemotingTest_Renderer_macOS.app/Contents/MacOS/RemotingTest_Renderer_macOS /MiniHttp
```

The corresponding automation endpoints are:

```text
GET  http://localhost:8888/Automation/RemotingTest_Core/Controls
POST http://localhost:8888/Automation/RemotingTest_Core/IO
GET  http://localhost:8889/Automation/RemotingTest_Renderer_macOS/Dom
POST http://localhost:8889/Automation/RemotingTest_Renderer_macOS/IO
```

Post renderer commands with `curl`, for example:

```bash
curl \
  -X POST \
  -H 'Content-Type: application/json; charset=utf8' \
  --data-binary '!LeftClick:<integer-x>,<integer-y>' \
  http://localhost:8889/Automation/RemotingTest_Renderer_macOS/IO
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

To replace the renderer, stop only the renderer process and start it again
while keeping the core alive. To test takeover, start another renderer without
first stopping the active one. The new renderer must retain the application
state and the previous renderer must exit cleanly. Closing the application
through its UI must also close the active renderer without a fatal prompt or
retry loop.
