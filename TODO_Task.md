- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

You are in the mono repo but you are going to debug and fix `GacUI` and `GacJS` (probably).

Release VlppOS to GacUI. `IChannelServer::WaitForClient` is removed, so `RemotingTest_Core` will have to change.
You can use a manual reset `vl::EventObject` to simulate the original blocking, but such event should be use in `StartServer` function, no extra construction is needed.

Run `RemotingTest_Core /RPT` with both `/Http` and `/Pipe`, make sure `RemotingTest_Renderer_Win32` works.
Verify them by debug both processes, breakpoint `RenderDom` in the renderer, and if the core and renderer runs normally, you will be able to see multiple frames of DOMs get passed to the renderer.
Kill both processes.

Follow `Tools\DebugGacUIWithBrowser.md` to start `RemotingTest_Core /Http /RPT` with `GacJS`.
Switch the the third tab and type something, observes that typed characters appear inside the text box.
Kill both processes.

It is highly possible that `RemotingTest_Renderer_Win32` and `GacJS` don't need to change, if they do, you need to pay attention as it might expose some problems in the async `IChannelServer::OnClientConnected`.
