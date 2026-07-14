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
