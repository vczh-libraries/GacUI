- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

You are in the mono repo but you are going to debug and fix `GacUI` and `GacJS` (probably).

## Bug 1

Follow `Tools\DebugGacUIWithBrowser.md` to start `RemotingTest_Core /Http /RPT` with `GacJS`.
After the webpage is ready, find and click the `Fatal Error` button.
`RemotingTest_Core` is supposed to pass the exception message through `IChannelServer::BroadcastError` and the webpage will receive the error and `alert`.
But this doesn't happen, `RemotingTest_Core` just die and the webpage displays nothing.
`RemotingTest_Rendering_Win32` also does not receive the error message, so I suspect the problem is in the core side.
Fix it.

Commit and push after fixing `Bug 1`.

## Bug 2

Crashes happen at `RemotingTest_Rendering_Win32` when `RemotingTest_Core` being launched with
- `/FCT` `/Pipe` typing easy to trigger data corruption.
- `/FCT` `/Http` easy to trigger memory corruption, but GacJS is running well.
- `GacJS` seems fine

So I guess there might be race condition or something happening in `RemotingTest_Rendering_Win32`. This is just my guess, I don't know the reason.
But I offer you a way to fix it.
You can debug both `RemotingTest_Core` and `RemotingTest_Rendering_Win32` at the same time.
After launching `RemotingTest_Rendering_Win32`, you can click the `Control` tab page and then the lower half of the window will be filled by a document control.
Keep typing and `RemotingTest_Rendering_Win32` will crash.

GacUI does not support UI Automation yet so when you try to query it you will just get an empty window,
but by debugging the process you will be able to know where the `Control` tab and the document control is by reading the DOM.
And then you can follow `Tools\Experiment\UIA.md` and use `UIA_List.ps1` with `UIA_Op.ps1` to operate the renderer window.

I offer a tip here, after figuring out how to use UIA scripts, you might be able to just write a powershell script to repeat the process.
Be awared that UI being debugged might perform not well so you need some thread sleeps from starting to show the `Control` tab, but typing could be fast.
By the way, when I am typing I found I needs to type ~100 characters with random `ENTER` to trigger the crash, but there is no stable results.
Sometimes the renderer crashes, sometimes the renderer hang, but the core seems fine.

An important observation is that, running core with `GacJS` seems never crash.

Fix the issue under both `/Pipe` and `/Http`, and you need to run the script to type 1000 characters without crashing for consecutive running for 5 times, it counts as a success.

Commit and push after fixing `Bug 2`.
