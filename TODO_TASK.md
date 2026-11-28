Currently remote protocol core is using `FakeClipboardService`. This task would like it to use the real one.
This task requires to fix `GacUI` including Windows and WASM, `wGac`, `iGac` and `GacJS`.
This task requires verification to be done against `GacUI` on Windows, and `GacJS`.
This task should only verify against remote protocol related test apps.
Injection options are everywhere, like in core, renderers, scripts, GacJS url arguments. You should read how other injections are implemented and add them in the same pattern.

Verification Tips:

Both document editor test page watches for clipboard. If the clipboard has anything that is not image/text/document, the paste button will automatically becomes disabled. Vice versa. So this feature is the best suite for seeing if the whole thing properly implemented.

`fakeClipboard` passed to core will make `RemotingTest_Core` only reads a clipboard simulation inside the process. So when there is no `denyClipboard` for both core and renderer, core will access the real renderer clipboard, therefore it begins to sense OS clipboard changing.

Combining `fakeClipboard` and both `denyClipboard` there will be 8 combinations:
- Each combination should test the paste button auto relation, cut and paste features from document editor test pages.
- Add to `DebugRemoteProtocolSop.md` for only two combination:
  - `fakeClipboard` is turned on and both `denyClipboard` is used, so OS clipboard does not affect it.
  - `fakeClipboard` is turned off and both `denyClipboard` is not used, so OS clipboard does affect it.
  - GacJS option will be in the URL.

1. Windows

`GuiRemoteControllerConfig::injectClipboard` should have default value `true`.
`GacUI.h` still keeps `SetupRemoteNativeController` forward declaration but in `GuiRemoteControllerSetup.cpp` that function should be updated to be:
```C++
int SetupRemoteNativeControllerInternal(vl::presentation::IGuiRemoteProtocol* protocol, const GuiRemoteControllerConfig& config, INativeClipboardService* clipboardService);
```
and forward declare at the end of `GuiRemoteController.h`.

The `clipboardService` here will be the implementation when `injectClipboard` is false, or when `injectClipboard` is true but `ControllerGlobalConfig::injectClipboard` passed from the remote renderer is false.

The value will be passed to `GuiRemoteController` and then to `GuiRemoteClipboardInjection` in `GuiRemoteControllerInjection.h`.
`GuiRemoteClipboardInjection` inherits from `INativeClipboardService` so that it talks to the remote renderer when it could.

If `clipboardService` is null, a `FakeClipboardService` will be used herer. Since `FakeClipboardService` is almost no performance overhead, this value could be just declared inside `SetupRemoteNativeControllerInternal` and use its address when needed.

You are going to update the remote protocol so that when remote renderer offers a clipboard implementation:
- After `ControllerConnect` is processed a `InjectionRequestClipboard` message will be sent from core when both `injectClipboard` is true, so that the renderer knows its clipboard implementation is in use.
- Right after `InjectRequestClipboard`, or when clipboard value from renderer side is changed by whatever program, an `InjectionUpdateClipboardFromRenderer` event is sent to the core with clipboard values. The value has optional text, document and image.
- If the core side wants to update the clipboard value proactively, an `InjectionUpdateClipboardFromCore` message will be sent from core with the same argument. The renderer should update the OS's clipboard.
- Declare protocols in `Protocol_Injection.txt`.

For the core side, now each OS is going to prepare its `SetupRemoteNativeController` implementation, which calls `SetupRemoteNativeControllerInternal` with their native clipboard implementation. For Windows, it may needs to create a proc just for `WindowsClipboardService::SetOwnerHandle` so that the OS update would reach the remote protocol core. `fakeClipboard` will be added for the core side. When it is passed, the application will call `SetupRemoteNativeControllerInternal` instead of `SetupRemoteNativeController` and pass null, so that `FakeClipboardService` will take place.

For the native renderer side, everything should just be ready as `GuiRemoteRendererSingle` starts on a OS native `INativeController` provider.
`RemotingTest_Rendering_Win32` should have a new command line argument added, since `injectClipboard` is by default true, so you are going to add an `denyClipboard` to make it false, there will be no `remoteClipboard`.

`denyClipboard` will also works for `RemotingTest_Core`.

2. GacJS

Update `GacJS` to implement clipboard. The clipboard injection check box should be by default checked, unlike others.
The URL argument will also be `remoteClipboard`, unlike `denyClipboard` for native renderers.

3. WASM

WASM is going to use browser's clipboard feature anyway, so `injectClipboard` from core should be `true`.
Update the `wasm_fct/index.html` properly.
You do not have the environment to test WASM.

4. wGac

You need to update `wGac` to add the `SetupRemoteNativeController` function.
You are going to add the `--denyClipboard` and `--fakeClipboard` to `RemotingTest_Rendering_Wayland` and `test.sh`.
You do not have the environment to test wGac.

5. iGac

You need to update `iGac` to add the `SetupRemoteNativeController` function.
You are going to add the `--denyClipboard` and `--fakeClipboard` to `RemotingTest_Rendering_Cocoa` and `test.sh`.
You do not have the environment to test iGac.

6. Website

Update the website to introduce this new clipboard injection thing.
Also update KB pages.
Publish the website.
When updating KB pages and website, you are going to pretend that WASM/wGac/iGac are already implemented this feature. I will follow up right away to verify them, I don't want to waste time updating KB pages and website again.
