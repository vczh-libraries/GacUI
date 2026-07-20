- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

You are going to add a `StartMiniHttpAutomationService(Ptr<IAsyncSocketServer>)` function, just like what `StartWindowsHttpAutomationService` is done, but it uses VlppOS's mini http server api, instead of using WinHttp.
- A dedicated cpp file in `Test/GacUISrc/RemotingTest_Core/Shared.cpp` file is created for this function, `GuiMain.cpp` in both `RemotingTest_(Core|Renderer_Win32)` will extern this function, no header file is involved.
- In `RemotingTest_Core`, when started with `/MiniHttp`, `StartMiniHttpAutomationService` is called using the `IAsyncSocketServer` used to start the remote protocol.
- `RemotingTest_Renderer_Win32` project's, since mini http server cannot share the same http port with different url prefix across processes:
  - Both `StartWindowsHttpAutomationService` and `StartMiniHttpAutomationService` will switch to use the 8889 port instead.
    - Update `Project.md` about this formation.
    - Update `../Tools/DebugGacUIRemoteProtocol.md` if this file is affected by the decision by:
      - The port changing from 8888 to 8889 in the native renderer.
      - Automation service not works when `/MiniHttp` is used.

## Verification

Firstly make sure `/MiniHttp` works with GacJS.
- You are going to follow `../Tools/Jobs/job.verifyRemoteProtocol.prompt.md` to perform verification but:
  - Only verify core with GacJS
  - Only verify `/MiniHttp`, ignore other network protocols.
This proves that `RemotingTest_Core /MiniHttp` is running as expected, reducing the complexity in the next step.

Secondly make sure `/MiniHttp` works with native renderer.
- Automation service in both core and renderer need to be accessed, making sure they actually respond.
  - If the automation service is not working, the verification instruction wont work.
- You are going to follow `../Tools/Jobs/job.verifyRemoteProtocol.prompt.md` to perform verification but:
  - Only verify core with native renderer
  - Only verify `/MiniHttp`, ignore other network protocols.
This proves that `RemotingTest_Renderer_Win32 /MiniHttp` is also running as expected, reducing the complexity in the next step.
