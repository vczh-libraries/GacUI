# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

You are in the mono repo but you are going to debug and fix `GacUI` and `GacJS` (probably).

Release VlppOS to GacUI. `IChannelServer::WaitForClient` is removed, so `RemotingTest_Core` will have to change.
You can use a manual reset `vl::EventObject` to simulate the original blocking, but such event should be use in `StartServer` function, no extra construction is needed. There is a new `IChannelServer::Start` function, only after calling it client connections will be listened.

Run `RemotingTest_Core /RPT` with both `/Http` and `/Pipe`, make sure `RemotingTest_Renderer_Win32` works.
Verify them by debug both processes, breakpoint `RenderDom` in the renderer, and if the core and renderer runs normally, you will be able to see multiple frames of DOMs get passed to the renderer.
Kill both processes.

Follow `Tools\DebugGacUIWithBrowser.md` to start `RemotingTest_Core /Http /RPT` with `GacJS`, rebuild `GacJS` first.
Switch the the third tab and type something, observes that typed characters appear inside the text box.
Kill both processes.

It is highly possible that `RemotingTest_Renderer_Win32` and `GacJS` don't need to change, if they do, you need to pay attention as it might expose some problems in the async `IChannelServer::OnClientConnected`.

# UPDATES

# TEST [CONFIRMED]

- Regenerate `VlppOS\Release` from current `VlppOS\Source`, copy the non-`IncludeOnly` release files into `GacUI\Import`, and build GacUI to confirm stale `IChannelServer::WaitForClient` usage is exposed.
- Build and run the VlppOS unit test project, with the inter-process channel tests confirming `IChannelServer::Start` and client connection callbacks still work.
- Build and run the GacUI unit test project, because C++ source changes in GacUI require unit test verification.
- Run `RemotingTest_Core /RPT /Pipe` with `RemotingTest_Rendering_Win32 /Pipe` under debugger, set a breakpoint at `GuiRemoteRendererSingle::RenderDom`, and confirm multiple DOM frames are delivered before killing both processes.
- Run `RemotingTest_Core /RPT /Http` with `RemotingTest_Rendering_Win32 /Http` under debugger, set the same breakpoint, and confirm multiple DOM frames are delivered before killing both processes.
- Rebuild GacJS, start `RemotingTest_Core /Http /RPT` with GacJS, open the browser page, switch to the third tab, type text, and confirm the typed characters appear in the textbox before killing the processes.
- Success criteria: all builds and unit tests pass, both renderer transports reach `RenderDom` repeatedly without debugger-reported exceptions, and the GacJS browser path accepts typed text in the Remote Protocol Test UI.

# PROPOSALS

- No.1 Update GacUI to the started channel server lifecycle [CONFIRMED]

## No.1 Update GacUI to the started channel server lifecycle

### CODE CHANGE

- Regenerate and import the latest `VlppOS` release files so GacUI consumes the new `IChannelServer::Start` API and no longer sees `IChannelServer::WaitForClient`.
- Update local-channel unit test setup to call `Start()` before registering local clients.
- Update `RemotingTest_Core` to start the combined channel/network server, signal a local manual-reset `vl::EventObject` from `OnClientConnected`, and wait for the renderer client through that event instead of calling the removed `WaitForClient`.

### CONFIRMED

- `VlppOS\Release` was regenerated with `Tools\Tools\Codepack.backup.exe`, and the generated non-`IncludeOnly` release files were copied into `GacUI\Import`.
- `VlppOS` built successfully with `.github\Scripts\copilotBuild.ps1` from `VlppOS\Test\UnitTest`.
- `GacUI` built successfully with `.github\Scripts\copilotBuild.ps1` from `GacUI\Test\GacUISrc`.
- `VlppOS` unit tests passed with `.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`: 12 / 12 files and 115 / 115 cases passed, including the named pipe and HTTP channel tests.
- `GacUI` unit tests passed with `.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`: 84 / 84 files and 1686 / 1686 cases passed, including the remote unit test framework tests.
- `RemotingTest_Core /Pipe /RPT` and `RemotingTest_Rendering_Win32 /Pipe` were run under CDB at the same time. A breakpoint at `vl::presentation::remote_renderer::GuiRemoteRendererSingle::RenderDom` in the renderer was hit 200 times before both processes were killed, with no debugger stop lines left in the logs.
- `RemotingTest_Core /Http /RPT` and `RemotingTest_Rendering_Win32 /Http` were run under CDB at the same time. The same renderer `RenderDom` breakpoint was hit 200 times before both processes were killed, with no debugger stop lines left in the logs.
- `GacJS\Gaclib` was rebuilt successfully with `yarn build`. `RemotingTest_Core /Http /RPT` was then run under CDB with GacJS opened in the browser at `http://localhost:8896/index.html`; the Remote Protocol Test UI rendered and the third visible `/RPT` tab opened. That tab is a document viewer in the current UI, so the focused browser typing requirement was verified with the existing GacJS `Testing_Protocol_SimpleTyping.js` test instead, which passed 1 / 1 file and 5 / 5 tests by typing text into the remoted textbox and confirming it appeared.
- No source changes were needed in `RemotingTest_Rendering_Win32` or `GacJS`.
