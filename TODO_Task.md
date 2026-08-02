Perform the following refactoring inside `Source/RemotingHelpers` and fix test apps accordingly:
- Add `Test/GacUISrc/Source_RemotingHelpers/Source_RemotingHelpers.vcxitems` to index the whole `Source/RemotingHelpers`:
  - Use them in all test apps under `GacUISrc` which needs them, instead of adding actual source files to each project files.
  - No need to care if any source files is not used in any specific test app.
  - If `UnitTest` has any test cases testing against `Source/RemotingHelpers`, just delete them.
- Keep automation service API simple:
  - In `WindowsAutomationService.Windows.(h|cpp)` remove `WindowsAutomationServiceType` and `*Scope`.
  - In `MiniHttpAutomationService.(h|cpp)` remove `*Scope`.
  - In `RemotingTest_Core`, remove `NativeAutomationServiceScope`, we can just call `Unsubstitute` directly. Because when `GuiMain` crashes, nothing is going to recover so ensuring such `Unsubstitute` call is just making the code complex without gaining any benefit.
    - The same rule applies to all test apps under `GacUISrc` solution.
    - Just like `Unsubstitute`, `Stop(Windows|Mini)HttpAutomationService` does not need protection, as the http service will automatically stops after the app is crashed.
  - Everything in `AutomationServiceHost` is not useful too, delete them.
  - The reason is that, only the test app knows what actual service to create. For example, for a trivial GacUI application, there are:
    - `WindowsAutomationService` for Windows
    - `WGacAutomationService` for Linux, and the source code is not in this repo.
    - `CocoaAutomationService` for macOS, and the source code is not in this repo.
    - And there is a `RemoteProtocolAutomationService` for GacUI remote protocol core application.
    - So any test app is going to create their own automation service as a value type and do substitution, and than starts either WindowsHttp or MiniHttp to expose the automation service via HTTP.
    - Since all test app is going to decide which automation service to substitute and which http service to start, so any "guiding code" in `Source/RemotingHelpers` is not useful. Just call specific function in test apps directly.
  - `../Tools/Copilot/Guidelines/Running-GacUI.md` needs to be updated to say, different test apps under each mode/platform need to start different `*AutomationService` with different Start/Stop functions, but all of them work in the same way.
- Fix `Test/Linux` test apps' `vmake` accordingly but there is no way to test them on Windows, the testing part will be done separatedly.
- `CppTest_Rvm`:
  - Remove `Main.Linux.cpp` and `Test/Linux/CppTest_Rvm` as this could not possibly be done in `GacUI` repo.
  - In `GuiMain`, when `/MinHttp` is used, it should call `StartMiniHttpAutomationService`:
    - `StartMiniHttpAutomationService` is not Linux/macOS specific, it is designed to pair with `/MiniHttp` in Windows/Linux/macOS.
    - `Start(NamedPipe|Http|MiniHttp)Server` is too similar, just extract most of the code into a template function, and keep very simple `Start(NamedPipe|Http|MiniHttp)Server` functions call it like `StartServer<named_pipe::NamedPipeServer>`:
      - Use `SetupHostedWindowsDirect2DRenderer` directly in `StartServer` first, this part will be fixed when Linux/macOS starts to run it.
- `RemotingTest_RvmHost`:
  - No need to split `main` into `Transport` and `ParseTransport`, we can inline `ParseTransport` in `main`. And when it is not Windows, just don't try to see if the argument is `/Http` or `/Pipe`, no need to create error messages to tell that they are Windows only -- as this is a test app, such information is expected to be read from the source code.
- `RemotingTest_Core` and `RemotingTest_Renderer_Win32`:
  - Similar issues as in `CppTest_Rvm` and `RemotingTest_RvmHost`, applies here.
- Fix `Project.md` if anything is affected.
- One more code review on `Source/RemotingHelpers` and affected test apps:
  - Theoridically we prefer those `*Scope` class than try-catch, but if a finalization just don't need to be done as the exception will crash the app directly, the `*Scope` pattern itself could be omitted, and call the finalization function directly at the very end, pretending the exception is not going to happen.
  - No need to detect if the network connection is alive, because if any connection actually breaks, the underlying API (http.sys/WinHttp/TCP socket) will tell you at the next call. Knowing ahead the status of the network doesn't bring any benefit.
    - Remove all constructions around heart beats. Heart beats is unnecessary in the whole test apps organization, as all test apps are supposed to be running in the same computer, where the network quality is not an issue.
    - Including all other means, not limited to heart beats.
    - Some messages seem to be no longer used or useful, clean up the code in configuration and constants.
  - In test apps, all messages are for business purpose only, which means, if a construct is invented only to increase network reliability, those are not needed too.
  - Something might happen during multiple apps tearing down:
    - The library and test app logic already offer enough signal, for example, when a remote protocol core sends `ControllerDisconnected`, the renderer know everything starts to tear down. In this case, there is no need to care about if other processes still alive, no need to care about responding messages, no need to care about network protocol issues, after this point. The renderer can just ignore all negative signal, and make sure itself could exit directly.
    - The idea applies to all other test apps.
  - I would like to see significant amount of code to be deleted without introducing too many others.

## Details

(This section is for `One more code review on `Source/RemotingHelpers` and affected test apps` only, delete this line when this section is filled)
