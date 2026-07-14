# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

commit and push all local changes

## Task 1
This task is for VlppOS repo
Http(Server|Client)(Api)?.Windows.(h|cpp)  using `vl::inter_process::windows_http` namespace.
NamedPipe.Windows.(h|cpp) using `vl::inter_process::named_pipe` namespace.
update knowledge base in VlppOS to reflect this change

## Task 2
Release VlppOS to Workflow and fix build breaks due to namespace changing
To verify, you need to follow the SOP to run chatbot test projects.

## Task 3
Release VlppOS to GacUI and fix build breaks due to namespace changing.
To verify, you need to:
- follow the SOP to run RemotingTest_(Core|Rendering_Win32) and make sure `/RCP /HTTP` works
- Run GacJS against RemotingTest_Core and make sure `/RCP /HTTP` works

## Task 4
Update document website to reflect the namespace changing, if anything actually updates, publish it

## Task 5
Run [job.Windows.copilotInitAll.prompt.md](Tools/Jobs/job.Windows.copilotInitAll.prompt.md) but skip learning.

# UPDATES

# TEST [CONFIRMED]

Run `Tools\Tools\Build.ps1 GacUI` before changing GacUI source. The prescribed import step must copy every non-`IncludeOnly` root VlppOS release artifact into `GacUI\Import`. The initial build must then fail at existing GacUI consumers that still name Windows HTTP and named-pipe concrete types directly under `vl::inter_process`, confirming that the regenerated dependency is being compiled and locating the complete downstream migration surface.

After the fix, build Debug x64 through GacUI's wrapper and run `UnitTest` through the supported unit-test wrapper, requiring 0 compiler errors and no unit-test or memory-leak failure. Follow `job.Windows.verifyGacUIe23.prompt.md` for both native transports: run `RemotingTest_Core /RPT /Http` with `RemotingTest_Rendering_Win32 /Http`, then repeat with `/RPT /Pipe` and `/Pipe`; exercise the Remote Protocol Test File/close/confirmation path through the HTTP automation endpoints and require both processes to exit as specified by the SOP.

The request's `/RCP /HTTP` spellings are interpreted as the Remote Protocol Test over HTTP requested by the SOP. The actual case-sensitive command-line switches implemented by both programs and specified by the SOP are `/RPT` and `/Http`; `/RCP` is not a scenario switch and `/HTTP` is not a transport switch.

Build GacJS, require its generated index page to be reachable, and use a visible browser against `RemotingTest_Core /RPT /Http`. Require the `Remote Protocol Test` UI, perform File > `self.Close() (InvokeInMainThread)` > OK, and require the exact rendered shutdown text `IGacUIRenderer exited due to receiving RequestControllerConnectionStopped.` Finally run the full GacJS package and protocol E2E suite, require every current `Testing_Protocol_*.js` suite to pass without skips, and clean every process and test-owned listener.

## Confirmation

`Tools\Tools\Build.ps1 GacUI` imported all five root VlppOS release artifacts and stopped in the first Release Win32 rebuild. Repeating that configuration through GacUI's logging wrapper produced 57 compiler errors and 0 warnings. The first error occurs in all three GacUI library variants at `WinNativeWindow.cpp:2269`: `vl::inter_process::HttpServerApi` no longer exists, so the automation service's base class and inherited methods cascade into further errors. This proves the regenerated dependency is being consumed.

A source scan finds the rest of the compile migration surface that will become reachable after fixing that first shared source: `RemotingTest_Core` has three `NamedPipeServer` references and two `HttpServer` references, while `RemotingTest_Rendering_Win32` constructs one `NamedPipeClient` and one `HttpClient`. These are exactly the concrete Windows transport types moved by VlppOS; no other stale concrete transport reference exists under GacUI `Source` or `Test`.

# PROPOSALS

- No.1 Qualify concrete Windows transport types at GacUI composition boundaries [CONFIRMED]

## No.1 Qualify concrete Windows transport types at GacUI composition boundaries

Keep the VlppOS namespace separation intact and migrate only GacUI's concrete transport composition points. Change `HttpAutomationService` to inherit from `vl::inter_process::windows_http::HttpServerApi`. In `RemotingTest_Core`, bind the HTTP server wrapper to `vl::inter_process::windows_http::HttpServer` and the named-pipe wrapper plus its type comparison to `vl::inter_process::named_pipe::NamedPipeServer`. In `RemotingTest_Rendering_Win32`, construct `vl::inter_process::windows_http::HttpClient` and `vl::inter_process::named_pipe::NamedPipeClient`.

Do not add compatibility aliases in `vl::inter_process` and do not import either concrete namespace globally. The explicit qualifications preserve the visible transport boundary while generic `INetworkProtocolClient`, channel, wait-result and status APIs remain in their existing parent namespace.

The first successful Debug x64 build can expose an existing MSVC C4297 warning in `RemotingTest_Rendering_Win32/Main.cpp`: the Windows entry point directly expands `CHECK_FAIL` to a throw for an invalid transport argument, even though `WinMain` is assumed not to throw. Meet the SOP's zero-warning gate by returning the function's already initialized `-1` result from that invalid-argument branch. This matches `RemotingTest_Core`, which validates bad switches by returning nonzero instead of throwing across the process entry boundary, and does not alter either valid `/Pipe` or `/Http` path.

After the source migration, build Debug x64 and run GacUI `UnitTest`, then complete every native HTTP, native named-pipe, browser RPT and GacJS E2E gate documented under `# TEST`. Regenerate GacUI's Release through the prescribed monorepo build only after all source behavior is verified.

### CODE CHANGE

Updated the shared Windows automation provider so `HttpAutomationService` inherits from `inter_process::windows_http::HttpServerApi`. Updated `RemotingTest_Core` to use `inter_process::windows_http::HttpServer` and `inter_process::named_pipe::NamedPipeServer`, including its server-type comparison. Updated `RemotingTest_Rendering_Win32` to construct `inter_process::windows_http::HttpClient` and `inter_process::named_pipe::NamedPipeClient`.

The first successful Debug x64 rebuild exposed MSVC C4297 at the renderer's `WinMain` invalid-argument branch. Replaced the throwing `CHECK_FAIL` there with `return result`; `result` is initialized to `-1`, while both valid transport branches are unchanged. The subsequent wrapper build completed with 0 warnings and 0 errors.

Regenerated `Release/GacUI.Windows.cpp` with the prescribed monorepo build. The merged release now contains the same nested `windows_http::HttpServerApi` qualification as its source; no Release file was edited directly.

### CONFIRMED

The migration is exhaustive under GacUI `Source`, `Test`, and the generated Windows Release: no concrete `HttpServerApi`, `HttpServer`, `HttpClient`, `NamedPipeServer`, or `NamedPipeClient` reference remains directly under `inter_process`. An independent diff review confirmed all eight downstream transport references use the intended nested namespaces, the imported VlppOS files match the upstream Release, and the invalid-argument warning fix does not affect `/Pipe` or `/Http`.

Debug x64 rebuilt with 0 warnings and 0 errors. The supported `UnitTest` wrapper passed 85/85 test files and 1690/1690 test cases, left no unfinished-log sentinel, and reported no memory leak. Native Remote Protocol Test verification passed for both `/RPT /Http` with renderer `/Http` and `/RPT /Pipe` with renderer `/Pipe`: both automation endpoints returned the expected `Remote Protocol Test` controls/DOM, two-way File-menu synchronization was exercised, the exit confirmation was handled, both processes exited, and port 8888 was released. The optional named-pipe error dialog did not appear.

GacJS built all five packages and served the generated `index.html` and `index.js` at HTTP 200. In the browser, `/FCT /Http` rendered `Complete Control Showcase`, opened the Control tab, and round-tripped the unique Search value `ANAMESPACECHECK20260714`. With `/RPT /Http`, File > `self.Close() (InvokeInMainThread)` > OK exited the core and rendered exactly `IGacUIRenderer exited due to receiving RequestControllerConnectionStopped.` The complete current protocol inventory then passed all 6 files and 44 tests without skips or leaked processes; the other GacJS package suites passed 1/1 and 87/87 tests.

Finally, `Tools\Tools\Build.ps1 GacUI` completed end to end with exit code 0 after Release Win32/x64 rebuilds, metadata generation and tests on both platforms, UnitTest on both platforms, Codepack release generation, GacGen, DarkSkin regeneration, and the final Codepack pass. File-dialog snapshot changes produced by the test runs were confirmed unrelated timing churn and excluded from this namespace change.
