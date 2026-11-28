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

### DETAILS

- Keep the public setup signature in `<GacUI repo>/Source/GacUI.h`. The platform entry point owns the native clipboard for the duration of the internal setup call. The internal function owns the fake fallback when passed null. Have the hosted controller obtain the selected service through `GuiRemoteController::ClipboardService()`. Use the internal setup with a null fallback in the snapshot harness in `<GacUI repo>/Source/UnitTestUtilities/GuiUnitTestUtilities.cpp` and the direct callers in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote.h` and `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Startup.cpp`. This keeps their clipboard deterministic and lets the portable unit-test target link without a platform clipboard provider.
- Append `injectClipboard` to the Core and native renderer configuration structs to preserve existing positional initializers. Add the advertised field to `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_Controller.txt`. An absent advertisement means the renderer cannot supply a clipboard. Use `InjectionRequestClipboard` consistently for the subscription. Generate the C++ schema and protocol metadata with `Metadata_UpdateProtocol` after editing the injection and controller schemas.
- Both update directions carry a complete snapshot. Preserve text presence even for an empty string. Use the existing clipboard document stream helpers in `<GacUI repo>/Source/Resources/GuiDocumentClipboard.h` for styles and embedded images, and encoded PNG bytes for a standalone image. All three representations can coexist. A successful read with none of them clears the previous snapshot. Copy data out of the native reader promptly so caching it does not keep the OS clipboard open.
- Declare `InjectionUpdateClipboardFromCore` with `response: bool`. The renderer replies to the request ID with the outcome of its clipboard write. In the injected writer's `Submit()`, wait for that response from the same connection and use `CHECK_ERROR` in `GuiRemoteClipboardInjection` on failure, following the writeback pattern in `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteControllerInjection.cpp`. A successful write returns to the editor. A rejected write enters Core's fatal-error path before `GuiDocumentCommonInterface::Cut()` reaches `SetSelectionText`.
- Before a renderer connects, Core uses its selected fallback. When both ends enable injection, request the renderer's current snapshot and start with an empty remote reader until it arrives. Install the snapshot before raising `InvokeClipboardUpdated()` so Paste sees the new value. Only an application write sends an update back to the renderer. On disconnect, restore the fallback and notify listeners. On replacement, discard the old snapshot and pending completions, negotiate again and request the new renderer's initial value.
- The headless Core event loop in `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteController.cpp` processes protocol events. A message-only HWND registered with `WindowsClipboardService::SetOwnerHandle` needs a live Windows message pump. Deliver `WM_CLIPBOARDUPDATE` to Core's UI thread while its local service is active and detach the listener before destroying its owner. The native image helpers in `<GacUI repo>/Source/PlatformProviders/Windows/ServicesImpl/WindowsImageService.cpp` cast to Windows image types. Bridge bitmap data between WIC and Core's remote image service using encoded bytes so the native fallback also handles images.
- Route `/fakeClipboard` to Core's fallback selection and `/denyClipboard` to each endpoint's capability through `<GacUI repo>/Test/GacUISrc/SharedArguments.h` and the two app entry points. Forward the applicable switches through `<GacUI repo>/Test/StartCore.ps1` and `<GacUI repo>/Test/StartRendrerer.ps1`. Keep the existing duplicate-switch checks.

### VERIFICATION

Use the eight rows below for each renderer. The two deny columns describe the effective endpoint settings. For a browser renderer, its URL selects the renderer column. The renderer OS rows assume clipboard access is available.

| Core fake | Core deny | Renderer deny | Clipboard used for reads and writes |
| --- | --- | --- | --- |
| Off | Off | Off | Renderer OS |
| Off | Off | On | Core OS |
| Off | On | Off | Core OS |
| Off | On | On | Core OS |
| On | Off | Off | Renderer OS |
| On | Off | On | Core fake |
| On | On | Off | Core fake |
| On | On | On | Core fake |

1. Build the Windows Debug x64 remote apps with the repository wrapper. Start a fresh `RemotingTest_Core /FCT /MiniHttp` and `RemotingTest_Rendering_Win32 /MiniHttp` for each row, with distinct automation ports. Exercise both Document Editor (Ribbon) and Document Editor (Toolstrip) in Editable mode. Use the Toolstrip commands and its point-selection procedure from section 3.2 of `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md`.
2. Seed the OS clipboard with known text before connecting. In each OS-backed row, confirm Paste starts enabled, then replace the clipboard from another process with an unsupported-only format and then with different Unicode text. Observe Paste disable and re-enable without an editor command, then paste and compare the new text. In a fake-backed row start with an empty fake clipboard. External changes must leave Paste disabled. Cut a known selection, change the OS clipboard again, and verify Paste still inserts the cut selection while the OS retains the external value.
3. In every row, verify Cut removes exactly the selected range and Paste restores it once at a new caret. Check styled text and an embedded image from `<GacUI repo>/Test/TestDocument.bin`, and exercise a standalone image on each clipboard backend. For OS-backed writes, inspect the clipboard from outside the editor. On one Windows desktop the two processes share an OS clipboard, so also capture the protocol messages. Expect a subscription, an initial renderer update and a Core write in the renderer rows, and zero clipboard messages in the fallback rows.
4. Keep a fake-fallback Core alive and replace an enabled renderer with a denying renderer, then reconnect an enabled one. Check that Paste follows each active source, accepts the fresh initial value and that updates settle after a write. Add only the first and last matrix rows to the shared SOP, including their browser URL settings.
5. In a separate session with injection active, hold the Windows clipboard open from another process while invoking Cut. Match the renderer's `false` response to that write's request ID and check Core's clipboard error and nonzero exit code.

2. GacJS

Update `GacJS` to implement clipboard. The clipboard injection check box should be by default checked, unlike others.
The URL argument will also be `remoteClipboard`, unlike `denyClipboard` for native renderers.

### DETAILS

- The existing parser in `<GacJS repo>/Gaclib/website/entry/src/connectionOptions.ts` treats any value on an injection flag as enabled. Give the new default-on option a round trip for both states. An omitted option, `?remoteClipboard` and `?remoteClipboard=true` enable it. Unchecking the box writes `?remoteClipboard=false`, which stays unchecked after Refresh. Apply this to the HTTP renderer and the renderer started with `rvmhost`.
- Implement clipboard access in the shared page-side renderer. Advertise the capability when the option is enabled and the page has the Clipboard API in a secure context. Start reading when Core subscribes. Map Unicode text, PNG and the private GacUI document bytes to representations of the same clipboard item. Use a web custom format for the document where supported. Detect support for clipboard change events and refresh on focus or a permitted user gesture when needed. A denied read gives no information about the clipboard contents. Discard asynchronous results from a stopped or replaced session. [W3C Clipboard API](https://www.w3.org/TR/clipboard-apis/)
- The browser text input adapter already handles DOM `paste` and `beforeinput` in `<GacJS repo>/Gaclib/gaclib/renderer/src/textInput.ts`. Coordinate that path with the Core Paste command so a single gesture inserts once and a rich selection keeps its formatting.
- Complete the Core write request when the browser's clipboard write promise settles. Resolve the protocol response to `true` on success or `false` on rejection so Core can apply its failure policy.

### VERIFICATION

- After native schema generation, run `yarn run import`, `yarn codegen` and `yarn build` in `<GacJS repo>/Gaclib`. Run the eight rows and both editor pages against the Windows `/FCT /MiniHttp` Core using Playwright Chromium on localhost. Use the real browser clipboard with read/write permission for the OS-backed cases. Verify the checkbox, refreshed URL and advertised capability for the bare URL and explicit off setting.
- Exercise the initial clipboard read, an external change followed by refocus, a browser clipboard write visible to another application, and rich Paste through the editor command. Check keyboard Paste for duplicate insertion. Deny a read and verify it produces no fabricated empty snapshot. Then allow access, refocus the page and check that Core receives the current contents.
- In a fresh session, deny a browser clipboard write after subscription. Invoke Cut and observe the matching `false` response and Core's fatal exit.

3. WASM

WASM is going to use browser's clipboard feature anyway, so `injectClipboard` from core should be `true`.
Update the `wasm_fct/index.html` properly.
You do not have the environment to test WASM.

### DETAILS

Enable clipboard in the shared Core startup in `<GacUI repo>/Test/Linux/Wasm/WasmApplication.cpp` and in the renderer handshake in `<GacJS repo>/Gaclib/website/remote-protocol-wasm/src/index.ts`. The worker can use the internal setup with a null fallback. Browser clipboard access belongs to the page. The FCT entry is `<GacJS repo>/Gaclib/website/entry/assets/wasm-fct/index.html` and shares its connection code with RPT and RVMT. The published showcase has its own entry at `<WebsiteSource repo>/packages/website/assets/wasm-fct/index.html`.

### VERIFICATION

Trace the page's capability into `ControllerConnect` and the worker's Core flag into the decision to subscribe on receipt. Check that the Wasm target links the neutral setup and that the three existing injection settings keep their defaults. Record runtime verification as pending for the platform follow-up.

4. wGac

You need to update `wGac` to add the `SetupRemoteNativeController` function.
You are going to add the `--denyClipboard` and `--fakeClipboard` to `RemotingTest_Rendering_Wayland` and `test.sh`.
You do not have the environment to test wGac.

### DETAILS

- Add a headless `RemotingTest_Core` test app to `<wGac repo>/CMakeLists.txt`. Repack GacUI and refresh both platform imports so the packed library supplies the internal setup and each platform supplies `SetupRemoteNativeController`. Extend `<wGac repo>/syncProj.sh` to copy the main and GUI entry points from `<GacUI repo>/Test/GacUISrc/RemotingTest_Core` alongside the shared argument parser. Use the packed GacUI and test-remoting headers in the Unix build. Link the synchronized FullControlTest, RemoteProtocolTest and RemoteViewModelTest resources, editor helpers and RPC initialization once.
- In both platform launchers, `test.sh --app:core` starts the local Core with `/MiniHttp` and defaults to `/FCT`. Support `/RPT`, `/RVMT` and the existing injection options. Forward `--fakeClipboard` and `--denyClipboard` on the Core command, and `--denyClipboard` on the renderer command, in foreground and `--unblock` modes. Route the existing Core launcher scripts' FCT, RPT and RVMT cases to the new binary. Their `cpptest_rvm` case still starts the standalone requester. Preserve native RVMT host startup and let GacJS supply its own network host or pass a complete `/Cli:<command>` as one argument.
- Once both platform targets and launchers use the new Core, remove `<GacUI repo>/Test/Linux/RemotingTest_Core`. The separate host still builds from `<GacUI repo>/Test/Linux/RemotingTest_RvmHost`. Update the Unix build and launch examples in `<GacUI repo>/.github/Jobs/job.rpXPlat.prompt.md`, `<GacUI repo>/.github/Jobs/DebugRemoteProtocolWithNativeRenderer.md` and `<GacUI repo>/.github/Jobs/DebugRemoteProtocolWithGacJS.md`. Update the matching project inventories, platform launcher documentation and the Core binary location in `<GacJS repo>/doc/Projects.md`.

In `<wGac repo>/WGac/Services/WGacClipboardService.cpp`, an external selection changes the offer without raising a clipboard notification. Its writer needs a compositor input serial to publish a selection. Notify after releasing the offer lock. A headless Core needs a clipboard backend it can service without focused Wayland input. The TUI adapter in `<wGac repo>/WGac/TUI/TuiWGacClipboardService.cpp` uses X11/XWayland selection ownership and currently exposes only text to other applications.

### VERIFICATION

Check the CMake target, its synchronized inputs, and the path selected by `<wGac repo>/test.sh` and `<wGac repo>/test_core.sh`. Trace `--app:core --fakeClipboard` and `--app:renderer --denyClipboard` through both foreground and `--unblock` branches. Check the updated job commands for references to the retired build directory. Native execution and external clipboard notifications are for the Ubuntu follow-up.

5. iGac

You need to update `iGac` to add the `SetupRemoteNativeController` function.
You are going to add the `--denyClipboard` and `--fakeClipboard` to `RemotingTest_Rendering_Cocoa` and `test.sh`.
You do not have the environment to test iGac.

### DETAILS

Add the headless `RemotingTest_Core` target to `<iGac repo>/CMakeLists.txt` alongside `RemotingTest_Rendering_macOS`. Extend `<iGac repo>/syncProj.sh` with the same canonical Core entry points and resource inputs as wGac. Link the Cocoa setup into this executable. Apply the Core launcher and host modes above in `<iGac repo>/test.sh` and `<iGac repo>/test_core.sh`.

Use the Cocoa clipboard's callback and image-service dependencies, and arrange for `CocoaClipboardService::CheckForUpdates()` to observe external pasteboard changes on both the native fallback and renderer paths. The TUI integration in `<iGac repo>/Mac/TUI/TuiCocoaController.mm` shows how it observes the pasteboard change count.

### VERIFICATION

Check that the executable path used by `<iGac repo>/test.sh` matches the new Core target, that both launch modes forward each switch to the intended process, and that the target has one public setup definition and a live caller for the pasteboard update check. Native execution and pasteboard transfers are for the macOS follow-up.

6. Website

Update the website to introduce this new clipboard injection thing.
Also update KB pages.
Publish the website.
When updating KB pages and website, you are going to pretend that WASM/wGac/iGac are already implemented this feature. I will follow up right away to verify them, I don't want to waste time updating KB pages and website again.

### DETAILS

Update the Core and renderer articles in `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/modes` and their canonical design pages `KB_GacUI_Design_RemoteProtocolCoreArchitecture.md` and `KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md` in `<Tools repo>/Copilot/KnowledgeBase`. Update the stale clipboard description in the GacUI KB index. Document defaults, which machine owns the active clipboard, browser access requirements and the platform implementations. Keep app switches, the two SOP cases and actual platform verification results in the project and test guidance.

### VERIFICATION

Follow `<WebsiteSource repo>/job.publish.prompt.md` for the documentation publish. Verify the deployed Core and renderer pages and the synchronized Markdown describe the same routing. The website workflow publishes documentation independently of the Linux-built Wasm demo. Deploy a matching bundle and Wasm binary together when updating the runnable demo.

## REVIEW COMMENTS

### Scope of the Unix renderer's fake clipboard

**review comment**: The eight cases assign `fakeClipboard` to Core. The wGac and iGac sections also request it on their renderer executables. A renderer flag cannot select the fallback in the separately launched Core. Its effect when both sides enable injection is unspecified.

**suggested solution**: Keep `--fakeClipboard` on both Core launchers. If the requested renderer switch is intentional, let it supply a renderer-local fake clipboard when Core subscribes, with `--denyClipboard` still controlling the advertisement. If only the eight cases are intended, expose the fake switch on Core alone.

I think OS native clipboard implementation is in `wGac` and `iGac`. So from now on you have to add `RemotingTest_Core` as a test app to `wGac` and `iGac` repos, and update the `GacUI/.github/Jobs` to fix the test instructions. Both `test.sh` should now supports `--app:core`. And you can delete `GacUI/Test/Linux/RemotingTest_Core`. Both `syncProj.sh` needs to fix too, but unfortunately you do not have the environment to test them, I will verify them in Ubuntu and macOS later.

**review comment**: The new platform-owned Core gives `test.sh --app:core --fakeClipboard` a defined destination on both systems. The remaining case is the literal `test.sh --app:renderer --fakeClipboard` requested in sections 4 and 5. With injection enabled, that would substitute a second clipboard in the renderer process.

**suggested solution**: Use `--fakeClipboard` for the new Core mode alone and `--denyClipboard` for either process. Keep renderer-side simulation only if it is a separate intended feature.

### Cut when the renderer cannot write [CLOSED]

**review comment**: `GuiDocumentCommonInterface::Cut()` in `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp` ignores the Boolean returned by `Submit()` and immediately deletes the selection. A renderer can fail to open the OS clipboard, and a browser can reject a write after Core sends it. A one-way update gives Core no way to report that failure before deleting the text.

**suggested solution**: Return the renderer's write result on `InjectionUpdateClipboardFromCore` and have Cut remove the selection only after a successful submit. Verify a rejected write leaves the selection intact. If delivery is intentionally one-way, define how the user can recover the cut content after a rejected write.

Messages have responses, I think it should be fine when the renderer returning failure, and it just crashes the core. `CHECK_ERROR` could just be used in `GuiRemoteClipboardInjection`.
