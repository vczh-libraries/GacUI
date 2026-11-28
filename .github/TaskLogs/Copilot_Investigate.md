# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Currently remote protocol core is using `FakeClipboardService`. This task would like it to use the real one.
This task requires to fix `GacUI` including Windows and WASM, `wGac`, `iGac` and `GacJS`.
This task requires verification to be done against `GacUI` on Windows, and `GacJS`.
This task should only verify against remote protocol related test apps.
Injection options are everywhere, like in core, renderers, scripts, GacJS url arguments. You should read how other injections are implemented and add them in the same pattern.

Verification Tips:

Both document editor test page watches for clipboard. If the clipboard has anything that is not image/text/document, the paste button will automatically becomes disabled. Vice versa. So this feature is the best suite for seeing if the whole thing properly implemented.

Only `RemotingTest_Core` accepts `fakeClipboard`. It selects `FakeClipboardService` as Core's local fallback. When both endpoints allow injection, the renderer's OS clipboard supplies the contents. Core keeps a copy in memory for the remote application. On separate machines, copying in a renderer-side app makes the item available to the remote editor. Copying in the remote editor makes the item available to other apps on the renderer machine. An unrelated app on the Core machine pastes from that machine's own OS clipboard.

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

Declare the clipboard messages and event in `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_Injection.txt`.

- After `ControllerConnect`, Core sends `InjectionRequestClipboard` once for that connection if both endpoints enable clipboard injection.
- On subscription the renderer starts observing its OS clipboard and immediately sends its current contents in `InjectionUpdateClipboardFromRenderer`. It sends a fresh snapshot after each observed OS clipboard change, including one caused by a write from Core. Deliver snapshots in observation order. Each snapshot carries optional text, document and image data.
- When the remote application submits a clipboard write, Core sends `InjectionUpdateClipboardFromCore` with the new contents. The renderer writes them to its OS clipboard and replies with the result. After success, another app on the renderer machine can paste the item.

For the core side, now each OS is going to prepare its `SetupRemoteNativeController` implementation, which calls `SetupRemoteNativeControllerInternal` with their native clipboard implementation. For Windows, it may needs to create a proc just for `WindowsClipboardService::SetOwnerHandle` so that the OS update would reach the remote protocol core. `fakeClipboard` will be added for the core side. When it is passed, the application will call `SetupRemoteNativeControllerInternal` instead of `SetupRemoteNativeController` and pass null, so that `FakeClipboardService` will take place.

For the native renderer side, everything should just be ready as `GuiRemoteRendererSingle` starts on a OS native `INativeController` provider.
`RemotingTest_Rendering_Win32` should have a new command line argument added, since `injectClipboard` is by default true, so you are going to add an `denyClipboard` to make it false, there will be no `remoteClipboard`.

`denyClipboard` will also works for `RemotingTest_Core`.

### DETAILS

- Keep the public setup signature in `<GacUI repo>/Source/GacUI.h`. The platform entry point owns the native clipboard for the duration of the internal setup call. The internal function owns the fake fallback when passed null. Have the hosted controller obtain the selected service through `GuiRemoteController::ClipboardService()`. Use the internal setup with a null fallback in the snapshot harness in `<GacUI repo>/Source/UnitTestUtilities/GuiUnitTestUtilities.cpp` and the direct callers in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote.h` and `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Startup.cpp`. This keeps their clipboard deterministic and lets the portable unit-test target link without a platform clipboard provider.
- Append `injectClipboard` to the Core and native renderer configuration structs to preserve existing positional initializers. Add the advertised field to `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_Controller.txt`. An absent advertisement means the renderer cannot supply a clipboard. Use `InjectionRequestClipboard` consistently for the subscription. Generate the C++ schema and protocol metadata with `Metadata_UpdateProtocol` after editing the injection and controller schemas.
- Both update directions carry a complete snapshot. Preserve text presence even for an empty string. Use the existing clipboard document stream helpers in `<GacUI repo>/Source/Resources/GuiDocumentClipboard.h` for styles and embedded images, and encoded PNG bytes for a standalone image. All three representations can coexist. A successful read with none of them clears the previous snapshot. Copy data out of the native reader promptly so caching it does not keep the OS clipboard open.
- `GuiRemoteClipboardInjection` owns an in-memory snapshot separate from the native or fake fallback. It starts empty on each subscription. Applying `InjectionUpdateClipboardFromRenderer` only replaces that snapshot and raises `InvokeClipboardUpdated()` after the replacement. `ReadClipboard()` serves the snapshot synchronously for `CanPaste()` and `Paste()`, with zero protocol requests or fallback accesses. Core uses its native clipboard writer and OS change notifications only while the native fallback is active.
- Only the injected writer's `Submit()` sends `InjectionUpdateClipboardFromCore`. Declare `response: bool` and wait for the renderer's OS write result matching the request ID on the same connection. After success, an immediate Paste reads the submitted contents or a newer renderer snapshot from Core memory. Use `CHECK_ERROR` in `GuiRemoteClipboardInjection` on failure, following the writeback pattern in `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteControllerInjection.cpp`. A rejected write enters Core's fatal-error path before `GuiDocumentCommonInterface::Cut()` reaches `SetSelectionText`.
- Before connection or when either endpoint denies injection, Core reads and writes its selected fallback directly. On disconnect, discard the renderer snapshot, expose the fallback's own current contents and notify listeners. On replacement, discard updates and pending completions from the old connection and subscribe to the new renderer if both endpoints enable injection.
- The headless Core event loop in `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteController.cpp` processes protocol events. A message-only HWND registered with `WindowsClipboardService::SetOwnerHandle` needs a live Windows message pump. Deliver `WM_CLIPBOARDUPDATE` to Core's UI thread while its local service is active and detach the listener before destroying its owner. The native image helpers in `<GacUI repo>/Source/PlatformProviders/Windows/ServicesImpl/WindowsImageService.cpp` cast to Windows image types. Bridge bitmap data between WIC and Core's remote image service using encoded bytes so the native fallback also handles images.
- Parse `/fakeClipboard` in `<GacUI repo>/Test/GacUISrc/RemotingTest_Core/Main.cpp`. Parse `/denyClipboard` for both endpoints in `<GacUI repo>/Test/GacUISrc/SharedArguments.h`. Forward both switches when `<GacUI repo>/Test/StartCore.ps1` launches `RemotingTest_Core`. The only clipboard switch on `<GacUI repo>/Test/StartRendrerer.ps1` is `-DenyClipboard`. Keep the existing duplicate-switch checks.

### VERIFICATION

Run the eight rows below for Win32 and GacJS. The two deny columns describe the effective endpoint settings. For a browser renderer, its URL selects the renderer column. In the Renderer OS rows, Core reads its in-memory snapshot and sends application writes to the renderer. These rows assume renderer clipboard access is available.

| Core fake | Core deny | Renderer deny | Active clipboard |
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
2. In each OS-backed row, seed the OS clipboard named in the table with known text before connecting. Confirm Paste starts enabled, then replace the clipboard from another process with an unsupported-only format and then with different Unicode text. Observe Paste disable and re-enable without an editor command, then paste and compare the new text. In a fake-backed row start with an empty fake clipboard. External changes must leave Paste disabled. Cut a known selection, change the OS clipboard again, and verify Paste still inserts the cut selection while the OS retains the external value.
3. In every row, verify Cut removes exactly the selected range and Paste restores it once at a new caret. Check styled text and an embedded image from `<GacUI repo>/Test/TestDocument.bin`, and exercise a standalone image on each clipboard backend. For an OS-backed write, check the contents in another app on the machine named in the table. Capture the clipboard messages in the Renderer OS rows. Expect one subscription on connection and an `InjectionUpdateClipboardFromRenderer` for the initial value and each change in step 2, before invoking Paste. Repeated Paste commands produce zero clipboard requests. One Cut sends one `InjectionUpdateClipboardFromCore`. The resulting OS change produces a renderer event and zero further Core writes. Expect zero clipboard messages in the fallback rows.
4. Keep a fake-fallback Core alive and replace an enabled renderer with a denying renderer, then reconnect an enabled one. Check that Paste follows each active source, accepts the fresh initial value and that updates settle after a write. Add only the first and last matrix rows to the shared SOP, including their browser URL settings.
5. In a separate session with injection active, hold the Windows clipboard open from another process while invoking Cut. Match the renderer's `false` response to that write's request ID and check Core's clipboard error and nonzero exit code.
6. Verify `RemotingTest_Rendering_Win32 /MiniHttp /fakeClipboard` exits with a nonzero status before connecting.
7. For the all-Off row, use two machines or two sessions with independent OS clipboards and clipboard redirection disabled. Seed the Core OS clipboard with `core-local`. Perform the actions in order and compare what each app pastes.

   | Action | Remote editor | Another app on the renderer machine | Unrelated app on the Core machine |
   | --- | --- | --- | --- |
   | Set the renderer clipboard to `renderer-before`, connect and wait for the initial event. | `renderer-before` | `renderer-before` | `core-local` |
   | Copy `renderer-after` in a renderer-side app and wait for the change event. | `renderer-after` | `renderer-after` | `core-local` |
   | Copy `remote-copy` in the remote editor. | `remote-copy` | `remote-copy` | `core-local` |
   | Copy `core-new` in an unrelated Core-side app. | `remote-copy` | `remote-copy` | `core-new` |
   | Reconnect with renderer injection denied. | `core-new` | `remote-copy` | `core-new` |

   On a single shared Windows desktop, verify the external paste after Copy and trace `ReadClipboard()` and `WriteClipboard()` on the native fallback in the Core process. Expect zero calls while injection is active. Both processes share the same OS clipboard in this setup.

2. GacJS

Update `GacJS` to implement clipboard. The clipboard injection check box should be by default checked, unlike others.
The URL argument will also be `remoteClipboard`, unlike `denyClipboard` for native renderers.

### DETAILS

- The existing parser in `<GacJS repo>/Gaclib/website/entry/src/connectionOptions.ts` treats any value on an injection flag as enabled. Use `remoteClipboard` as the only clipboard checkbox and URL option. An omitted option, `?remoteClipboard` and `?remoteClipboard=true` enable it. Unchecking the box writes `?remoteClipboard=false`, which stays unchecked after Refresh. Apply this to the HTTP renderer and the renderer started with `rvmhost`.
- Implement clipboard access in the shared page-side renderer. Advertise the capability when the option is enabled and the page has the Clipboard API in a secure context. Start reading when Core subscribes. Map Unicode text, PNG and the private GacUI document bytes to representations of the same clipboard item. Use a web custom format for the document where supported. Detect support for clipboard change events and refresh on focus or a permitted user gesture when needed. A denied read gives no information about the clipboard contents. Discard asynchronous results from a stopped or replaced session. [W3C Clipboard API](https://www.w3.org/TR/clipboard-apis/)
- The browser text input adapter already handles DOM `paste` and `beforeinput` in `<GacJS repo>/Gaclib/gaclib/renderer/src/textInput.ts`. Coordinate that path with the Core Paste command so a single gesture inserts once and a rich selection keeps its formatting.
- Complete the Core write request when the browser's clipboard write promise settles. Resolve the protocol response to `true` on success or `false` on rejection so Core can apply its failure policy.

### VERIFICATION

- After native schema generation, run `yarn run import`, `yarn codegen` and `yarn build` in `<GacJS repo>/Gaclib`. Run the eight rows and both editor pages against the Windows `/FCT /MiniHttp` Core using Playwright Chromium on localhost. Use the real browser clipboard with read/write permission for the OS-backed cases. Verify the checkbox, refreshed URL and advertised capability for the bare URL and explicit off setting.
- Check that the browser pushes the initial snapshot after subscription and the changed snapshot after an external copy and refocus, before the editor requests Paste. Paste rich content through the editor command and check keyboard Paste for duplicate insertion. Copy a selection in the remote editor and paste it into another app on the browser machine to confirm the browser wrote its OS clipboard. Deny a read and verify it produces no fabricated empty snapshot. Then allow access, refocus the page and check that Core receives the current contents.
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
In `<wGac repo>/test.sh`, accept `--denyClipboard` for both `RemotingTest_Core` and `RemotingTest_Rendering_Wayland`. Accept `--fakeClipboard` only with `--app:core`.
You do not have the environment to test wGac.

### DETAILS

- Add a headless `RemotingTest_Core` test app to `<wGac repo>/CMakeLists.txt`. Repack GacUI and refresh both platform imports so the packed library supplies the internal setup and each platform supplies `SetupRemoteNativeController`. Extend `<wGac repo>/syncProj.sh` to copy the main and GUI entry points from `<GacUI repo>/Test/GacUISrc/RemotingTest_Core` alongside the shared argument parser. Use the packed GacUI and test-remoting headers in the Unix build. Link the synchronized FullControlTest, RemoteProtocolTest and RemoteViewModelTest resources, editor helpers and RPC initialization once.
- In both platform launchers, `test.sh --app:core` starts the local Core with `/MiniHttp` and defaults to `/FCT`. Support `/RPT`, `/RVMT` and the existing injection options. Translate the selected app's `--` clipboard switches to `/` arguments in both foreground and `--unblock` modes. Route the existing Core launcher scripts' FCT, RPT and RVMT cases to the new binary. Their `cpptest_rvm` case still starts the standalone requester. Preserve native RVMT host startup and let GacJS supply its own network host or pass a complete `/Cli:<command>` as one argument.
- Once both platform targets and launchers use the new Core, remove `<GacUI repo>/Test/Linux/RemotingTest_Core`. The separate host still builds from `<GacUI repo>/Test/Linux/RemotingTest_RvmHost`. Update the Unix build and launch examples in `<GacUI repo>/.github/Jobs/job.rpXPlat.prompt.md`, `<GacUI repo>/.github/Jobs/DebugRemoteProtocolWithNativeRenderer.md` and `<GacUI repo>/.github/Jobs/DebugRemoteProtocolWithGacJS.md`. Update the matching project inventories, platform launcher documentation and the Core binary location in `<GacJS repo>/doc/Projects.md`.

In `<wGac repo>/WGac/Services/WGacClipboardService.cpp`, an external selection changes the offer without raising a clipboard notification. Its writer needs a compositor input serial to publish a selection. Notify after releasing the offer lock. A headless Core needs a clipboard backend it can service without focused Wayland input. The TUI adapter in `<wGac repo>/WGac/TUI/TuiWGacClipboardService.cpp` uses X11/XWayland selection ownership and currently exposes only text to other applications.

### VERIFICATION

Check the CMake target, its synchronized inputs, and the path selected by `<wGac repo>/test.sh` and `<wGac repo>/test_core.sh`. Trace both foreground and `--unblock` launches. `--app:core --fakeClipboard --denyClipboard` must pass `/fakeClipboard /denyClipboard` to Core. `--app:renderer --denyClipboard` must pass `/denyClipboard` to the renderer. Check that `--app:renderer --fakeClipboard` fails argument validation before launch and that the renderer executable treats `/fakeClipboard` as unknown. Check the updated job commands for references to the retired build directory. Native execution and external clipboard notifications are for the Ubuntu follow-up.

5. iGac

You need to update `iGac` to add the `SetupRemoteNativeController` function.
In `<iGac repo>/test.sh`, accept `--denyClipboard` for both `RemotingTest_Core` and `RemotingTest_Rendering_macOS`. Accept `--fakeClipboard` only with `--app:core`.
You do not have the environment to test iGac.

### DETAILS

Add the headless `RemotingTest_Core` target to `<iGac repo>/CMakeLists.txt` alongside `RemotingTest_Rendering_macOS`. Extend `<iGac repo>/syncProj.sh` with the same canonical Core entry points and resource inputs as wGac. Link the Cocoa setup into this executable. Apply the Core launcher and host modes above in `<iGac repo>/test.sh` and `<iGac repo>/test_core.sh`.

Use the Cocoa clipboard's callback and image-service dependencies, and arrange for `CocoaClipboardService::CheckForUpdates()` to observe external pasteboard changes on both the native fallback and renderer paths. The TUI integration in `<iGac repo>/Mac/TUI/TuiCocoaController.mm` shows how it observes the pasteboard change count.

### VERIFICATION

Check that the executable path used by `<iGac repo>/test.sh` matches the new Core target. Trace the same accepted and rejected clipboard arguments as on wGac through both launch modes and the native entry points. Check that the target has one public setup definition and a live caller for the pasteboard update check. Native execution and pasteboard transfers are for the macOS follow-up.

6. Website

Update the website to introduce this new clipboard injection thing.
Also update KB pages.
Publish the website.
When updating KB pages and website, you are going to pretend that WASM/wGac/iGac are already implemented this feature. I will follow up right away to verify them, I don't want to waste time updating KB pages and website again.

### DETAILS

Update the Core and renderer articles in `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/modes` and their canonical design pages `KB_GacUI_Design_RemoteProtocolCoreArchitecture.md` and `KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md` in `<Tools repo>/Copilot/KnowledgeBase`. Update the stale clipboard description in the GacUI KB index. Explain the initial and subsequent renderer snapshots, the in-memory reads in Core and where a user can paste a copy from the remote editor on separate machines. Document the defaults, fallback, browser access requirements and platform implementations. Keep app switches, the two SOP cases and actual platform verification results in the project and test guidance.

### VERIFICATION

Follow `<WebsiteSource repo>/job.publish.prompt.md` for the documentation publish. Verify the deployed Core and renderer pages and the synchronized Markdown describe the same routing. The website workflow publishes documentation independently of the Linux-built Wasm demo. Deploy a matching bundle and Wasm binary together when updating the runnable demo.

## REVIEW COMMENTS

### Scope of the Unix renderer's fake clipboard

**review comment**: The eight cases assign `fakeClipboard` to Core. The wGac and iGac sections also request it on their renderer executables. A renderer flag cannot select the fallback in the separately launched Core. Its effect when both sides enable injection is unspecified.

**suggested solution**: Keep `--fakeClipboard` on both Core launchers. If the requested renderer switch is intentional, let it supply a renderer-local fake clipboard when Core subscribes, with `--denyClipboard` still controlling the advertisement. If only the eight cases are intended, expose the fake switch on Core alone.

I think OS native clipboard implementation is in `wGac` and `iGac`. So from now on you have to add `RemotingTest_Core` as a test app to `wGac` and `iGac` repos, and update the `GacUI/.github/Jobs` to fix the test instructions. Both `test.sh` should now supports `--app:core`. And you can delete `GacUI/Test/Linux/RemotingTest_Core`. Both `syncProj.sh` needs to fix too, but unfortunately you do not have the environment to test them, I will verify them in Ubuntu and macOS later.

**review comment**: The new platform-owned Core gives `test.sh --app:core --fakeClipboard` a defined destination on both systems. The remaining case is the literal `test.sh --app:renderer --fakeClipboard` requested in sections 4 and 5. With injection enabled, that would substitute a second clipboard in the renderer process.

**suggested solution**: Use `--fakeClipboard` for the new Core mode alone and `--denyClipboard` for either process. Keep renderer-side simulation only if it is a separate intended feature.

To clarify, `fakeClipboard` means to use `FakeCliboardService` instead of OS native clipboard. `denyClipboard` means core won't use renderer side clipboard or renderer won't offer clipboard implementation. They have different purpose.

Only Core accepts the `fakeClipboard` argument. The renderer's only clipboard option controls whether it offers its OS clipboard.

### Cut when the renderer cannot write

**review comment**: `GuiDocumentCommonInterface::Cut()` in `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp` ignores the Boolean returned by `Submit()` and immediately deletes the selection. A renderer can fail to open the OS clipboard, and a browser can reject a write after Core sends it. A one-way update gives Core no way to report that failure before deleting the text.

**suggested solution**: Return the renderer's write result on `InjectionUpdateClipboardFromCore` and have Cut remove the selection only after a successful submit. Verify a rejected write leaves the selection intact. If delivery is intentionally one-way, define how the user can recover the cut content after a rejected write.

Messages have responses, I think it should be fine when the renderer returning failure, and it just crashes the core. `CHECK_ERROR` could just be used in `GuiRemoteClipboardInjection`.

# UPDATES

## UPDATE

Use the clipboard that accessible in the browser, this is also what "remote renderer side" means

## UPDATE

when the option is enabled using checkbox or in the URL

## UPDATE

In `GuiRemoteRendererSingle` you seems to ask user if they want to share the clipboard, but this is the test app, there is no reason to ask. Remove them in GacUI and potentially wGac and iGac.
additionally, I would like you to copy paste image into and out of document editor test page to make sure text/image/document is working with clipboard. You can run `RemotingTest_Core` + `RemotingTest_Rendering_Win32` as an app, `CppTest` as another app, for image and document. When you only select an image in the document, both document and image will appear in the clipboard. And you also want to test copy paste to and from notepad, image copy with mspaint, carefully verify if anything would break when interop with other apps.

## UPDATE

Meanwhile you edited [DocumentEditorBase.xml](Test/Resources/App/FullControlTest/DocumentEditorBase.xml) about shortcut key issues, I have stated very clearly in [Project.md](Project.md) that this issue could be ignored, please revert it. If you need to do copy pasting, you could use the UI directly, no need to use shortcut key.

## UPDATE

I think there is no need to add `TryReadClipboard`. In the injection implementation it could just call `Submit` and this function will block until a response comes. Fix GacUI/wGac/iGac so that it doesn't implement or call this function.

## UPDATE

I think I could have offer a wrong suggestion, there should be no need to wait for `TryReadClipboard` because core side is always sync with the renderer side for clipboard data when both injection are allowed. One of the proper implementation is that, between `ControllerConnect` and the immediate `InjectionUpdateClipbpardFromRenderer`, the core side clipboard should notice GacUI that clipboard data is cleared, `ReadClipboard` should return a reader that text/image/document are both empty

## UPDATE

So you are going to still remove `TryReadClipboard`, just to be clear

## UPDATE

This is not the appropriate way. I mean `InjectionUpdateClipboardFromRenderer` does not update core side OS clipboard, it only update the cached inside core's clipboard service. So there is never a actual clipboard change, so there should not make any WM_CLIPBOARDUPDATE message. You should just call `InvokeClipboardUpdated` upon receiving `InjectionUpdateClipboardFromRenderer` instead of doing these things around. What do you think?

## UPDATE

commits have been grouped but changes still. I would like you to take a look at `DispatchClipboardUpdate` and other changes to `WindowsClipboardService.(h|cpp)`. I think some changes are only valid with remote protocol, take a look add see if they could be moved to `GuiRemoteControllerInjection.(h|cpp)` or not. For example, you could add a `GuiRemoteClipboardInjection::Disconnected` to tell GacUI that remote protocol no longer serving the clipboard implementation, it could be called when `ControllerDisconnected` is called, but this event is not guaranteed to trigger, and it could also be called in `ControllerConnected` before the first clipboard data from the renderer is received. Since `GuiRemoteClipboardInjection` is the accessible `INativeClipboardService` in remote protocol core, no one is going to access `WindowsClipboardService` directly, So I guess `GuiRemoteClipboardInjection::Disconnected|UpdateData` might be sufficient and leave `WindowsClipboardService` with only bug fixes. When it is disconnected, instead of serving empty data, it should return to OS clipboard data, so an clipboard updated notification to GacUI (but not via windows message since there is no actual OS clipboard change) is sent. Now a disconnected `GuiRemoteClipboardInjection` will just redirect everything to the underlying clipboard service appropriately, until `UpdateData` is called and it goes back to "renderer mode". I think this should be enough to remove `DispatchClipboardUpdate` since that function looks wired and it is unnecessary to stay inside `WindowsClipboardService`.

by the way, `SetupRemoteNativeController` no need to do try-catch, just let it crashes, since at the moment no one is going to recover the error so `CoUninitialize` is unnecessary and this helps to preserve the scene for debugging.

# TEST [CONFIRMED]

Build the Debug x64 remote applications and use the two document editor pages in Editable mode. Seed the clipboard with a known fixture before connecting. Check Paste availability and the result of Cut and Paste using the eight routing cases above for each renderer. Verify each renderer subscription, change event, write result, replacement and denied write with the observations specified above. Check the Unix launchers and Wasm wiring statically on Windows.

On the original Debug x64 binaries, a separate Win32 process seeded the clipboard with `GacUI clipboard fixture 42`. In the Ribbon context menu and the Toolstrip Edit menu, Paste had the disabled label color and clicking it left the fixture absent from the document. Select All in the Toolstrip menu had the enabled color. These runs used an independent named-pipe Core and Win32 renderer because a pre-existing Core held the fixed MiniHTTP port. The GacJS connection-option unit test also failed on the original parser because an omitted option yielded no clipboard setting.

A controller listener recorded the reader from each connection notification. On the previous implementation, the first callback already contained the renderer’s text when its subscription handler replied immediately. The first empty-reader assertion failed.

For the revised handoff, seed the fallback with distinct text and replace a renderer while its successor delays its first snapshot. Observe the reader from inside each listener callback. The gap must allow local writes and local update notifications. An empty first snapshot must select the renderer, a writer from the old connection must be rejected, and an explicit disconnect must restore the text written during the gap.

# PROPOSALS

- No.1 Give the headless Core a platform-owned local clipboard [CONFIRMED]
- No.2 Synchronize the active renderer clipboard for the selected session [CONFIRMED]

- No.3 Remove the native clipboard dialog and verify cross-application transfers [DENIED]
- No.4 Notify listeners of the empty shared clipboard before subscribing to the renderer [DENIED]
- No.5 Deliver renderer snapshots directly to Core clipboard listeners [DENIED]
- No.6 Use the local clipboard until the current renderer supplies data [CONFIRMED]

The first two proposals supply the two clipboard routes. The first supplies the clipboard when sharing is disabled on either endpoint. The second supplies the connected renderer’s clipboard when both endpoints enable sharing.

## No.1 Give the headless Core a platform-owned local clipboard

Keep the public platform setup entry point and let it own the native service until the hosted Core has finished. The portable setup takes the selected service explicitly. A null argument selects a local fake for tests, Wasm and the Core-only switch. The hosted controller delegates to this service. Native changes notify the application on its UI thread. Image data crosses between the native and remote image services in an encoded stream.

The browser entry connects on page load to a destination taken from the URL. The renderer binds browser clipboard sharing to the selected Core for the session.

### CODE CHANGE

Route the local service through the setup and controller. Pump platform clipboard events on the Core thread, own the Windows clipboard HWND and bridge Windows bitmaps through PNG. Select the fake explicitly in the portable harness and in the Core launcher. Exercise both local backends through the two remote document editors.

### CONFIRMED

With renderer sharing disabled, a separate Windows process set text, an unsupported-only format, Unicode text and a bitmap. The two remote document editors followed Paste availability. Pasting inserted the new text and bitmap. With the fake fallback the editors started with Paste disabled, retained their own cut selections across external clipboard changes and left the external value intact. The focused Windows unit run passed all 31 remote startup tests, including a standalone image with no text or document format pasted into both Core fake-backed editors.

## No.2 Synchronize the active renderer clipboard for the selected session

Both endpoints advertise whether the renderer can share its clipboard. Core subscribes to that renderer, caches complete snapshots and waits for the renderer to acknowledge a submitted write. Changing connections invalidates the cache and outstanding writes. GacJS uses the clipboard available to its page, selected by the checkbox or URL. Browser permission applies to the page origin. When a URL selects another host, identify that Core and request session approval before advertising clipboard access.

### CODE CHANGE

Add the negotiated protocol and shared serialization for text, document streams and PNG. Connect the native renderer listener and browser adapter to their respective OS clipboards. Keep the worker on the shared browser adapter and validate local fallback independently of session sharing.

The initial implementation registered Copy, Cut and Paste shortcuts only while each document had focus. The maintainer requested the original command declarations in proposal 3. The shortcut change was reverted on Windows and in the platform mirrors. In GacJS a DOM paste uses the existing editor command, and its accompanying beforeinput event is consumed.

### CONFIRMED

The browser matrix used independent Windows and Chromium clipboards and the Windows MiniHTTP Core. All eight rows passed in both document editors. The OS-backed rows followed external text, unsupported formats, Unicode and standalone PNG. Every row restored the exact cut range at a new caret and the styled persistence fixture with both embedded images. Each renderer-backed row recorded one subscription, four writes and four matched success responses. The six fallback rows recorded zero clipboard messages.

Before the focus fix, physical Ctrl+V reached Core, but only Ribbon pasted. With the regenerated resources, both editors accepted a physical Ctrl+V exactly once and likewise inserted once for a paired DOM paste and beforeinput. Keyboard Paste in each editor restored the fixture’s original styles and both embedded images. The Win32 renderer passed the same plain and rich keyboard checks in both editors. Both full eight-row matrices then passed again with the regenerated resources. The final traces recorded 15 renderer snapshots per injected Win32 row and 11 per injected Chromium row. Across both renderers all 12 fallback rows had zero clipboard protocol messages.

Both Win32 editors passed the same eight routing rows. In the injected rows a test build recorded the real Core channel packages, renderer acknowledgements, and Core-local Windows clipboard calls. Reading Paste availability and executing Paste produced no Core clipboard request. Each Cut sent one write. Core made zero local clipboard calls during the injected editor operations. With a Windows process holding the clipboard open, request 118 received a false response, Core reported that the renderer could not write the clipboard and exited with code 3. Chromium also returned false for write request 143 after its write permission was denied, and that Core exited with code 3. The Win32 renderer rejected `/MiniHttp /fakeClipboard` before connecting and exited with code -1.

A fake-fallback Core survived live replacement with a denying and then an enabled renderer on both Windows and Chromium. Paste followed the new source and the fresh connection published its initial snapshot. Unchecking the browser checkbox wrote `remoteClipboard=false` and retained it on refresh. A URL with `remoteClipboard=true` restored sharing. With a different Core hostname and an existing browser permission grant, dismissing the destination confirmation produced zero clipboard messages. Selecting that named Core for the session enabled Paste from the browser clipboard. A separate all-enabled browser session verified the independent Windows and browser clipboard values before connection, after an external renderer copy, after Copy in the editor, after an external Core-side copy and after a denied reconnect.

The Chromium permission test reproduced an additional state transition. Granting read access in an already-focused page could leave Paste disabled. The adapter now observes changes to read permission and page visibility and retries on a permitted gesture following a read failure. A denied read emitted zero snapshots and the real contents reached Core after access was granted. A stopped read cannot hold up a replacement session.

The final Debug Win32 and x64 solutions built successfully. Win32 reported two code-page warnings in the existing remote caret test. Both metadata generators ran, the x64 metadata test loaded and compared both architectures, and all 31 remote startup tests passed again with the regenerated resources. In GacJS, import, code generation and the full build succeeded. The browser entry suite passed 83 tests, with 23 Wasm artifact tests skipped. The HTTP and renderer clipboard suites each passed all six tests.

The Windows-hosted launcher checks covered the six accepted and rejected argument traces on each Unix platform. Wasm capability wiring was checked through the page, transport handshake and worker setup. Ubuntu, macOS and Wasm runtime checks belong to the platform follow-up. Publication of the Core and renderer articles completed. Both served HTML files matched their deployed Git blobs byte for byte, and all 40 synchronized knowledge-base files matched their five canonical sources.

## No.3 Remove the native clipboard dialog and verify cross-application transfers

Use the native launcher settings for clipboard negotiation without opening a dialog. Use the menu commands in the FullControlTest editors with the original shortcut declarations. Verify Unicode text with Notepad, a patterned bitmap with Paint and a styled document with embedded images in both directions between the Win32 remote editor and CppTest. Check the clipboard formats from a single-image selection and compare dimensions, decoded pixels and document structure after pasting.

### CODE CHANGE

Restore the original FullControlTest command declarations and regenerate the resources. Remove the dialog from the native renderer and synchronize the platform imports.

Paint supplied an opaque 37 × 23 image with 851 distinct pixel positions. Copying it from the remote editor kept the pixels in both DIB representations. Paint pasted them exactly. A Windows Forms bitmap reader changed 521 pixels in that same outgoing clipboard. Changing just the DIB color-rendering intent from graphics to images produced 851 exact matches. Use the image intent for clipboard bitmaps.

CppTest also kept Paste disabled after a rich copy when another process was reading the clipboard during its change notification. Replaying the notification with the clipboard contents unchanged enabled Paste and restored both embedded images. Retry the notification on the Windows UI thread after a busy read. Supply an optional nonblocking reader to the native renderer so a temporarily busy clipboard does not replace its last snapshot with an empty one.

The saved source clipboard document included a centered first paragraph. CppTest pasted the text and both images but the first paragraph inherited the alignment of its old text, even when the entire destination had been selected. Preserve the source alignment when a rich paste supplies the entire first output paragraph. Keep the surrounding paragraph’s alignment for an inline replacement.

A 21 × 9 image with five alpha levels exposed a second Paint issue. The editor’s private document retained the alpha channel, but Paint made all 152 nonopaque pixels opaque when given the GacUI bitmap. Supplying the original PNG beside that same bitmap preserved all alpha and visible RGB values on the same Paint canvas. Paint then provided both formats to the remote editor, whose bitmap reader again lost the 152 alpha values. Publish the registered Windows PNG clipboard format alongside DIBV5 and prefer it when reading an image. Keep bitmap fallback for applications that supply only a bitmap.

### DENIED BY USER

The maintainer rejected the additional clipboard read API. Proposal 4 replaces that part of this implementation. The native dialog removal, restored editor shortcuts and verified text, bitmap and document fixes remain applicable. The following observations record the original interoperability run.

The Win32 renderer connected to a named-pipe Core using the computer’s hostname and the default clipboard settings. It opened the FullControlTest editor without a dialog. The Core used its fake fallback for this run. A copy from Notepad reached the editor, and Notepad pasted a different Unicode string from the editor. Both directions matched exactly, including the Greek and Chinese characters. All Copy and Paste operations used menu commands.

Paint copied a 37 × 23 patterned image into the remote editor. Selecting that image alone and copying it published both a Windows bitmap and `GacUI Document Format`. An independent Paint window pasted it onto a different canvas and copied it back. All 851 RGB pixels matched. CppTest and the remote editor also exchanged that single-image selection in both directions. Both apps published the private document and bitmap together, and an independent Windows Forms reader found zero pixel differences in every outgoing copy.

The remote editor loaded the private persistence fixture and copied its whole contents to CppTest. After replacing the remote document with a marker, CppTest copied its pasted document back. Across both directions all six paragraph alignments, styled text runs, image dimensions and frame indices matched. The decoded pixels of the 1250 × 500 and 256 × 256 images had identical hashes at each step. During a forced busy read the remote editor retained its Paste command. On release CppTest recovered and pasted the same rich document without a second external change.

The focused Debug x64 run passed all 177 cases across the document model, editable controls and remote startup. The XML and generated FullControlTest resources matched their versions before the shortcut change on Windows, Wayland and macOS.

With registered PNG support, a single-image selection offered the private document, PNG and DIBV5 together. Paint pasted the 21 × 9 image and copied it back with all 189 pixels’ alpha values and visible RGB intact. The remote editor pasted that Paint copy, then CppTest pasted the result. The PNG and the embedded image matched at each step, including alpha values of 0, 64, 128, 192 and 255. CppTest also pasted a PNG-only clipboard. A DIBV5-only clipboard still supplied all 851 opaque pixels to the remote editor.

The final Win32 and x64 Debug builds succeeded. Both metadata generators and the cross-architecture metadata test completed successfully. All 177 focused cases passed again. The independent CppTest UI Automation client invoked the Copy and Paste menu items and verified the clipboard text and the pasted document.

CppTest also exchanged Unicode text directly with Notepad and the 37 × 23 image directly with Paint in both directions. The receiving Paint process owned the final clipboard and supplied all 851 original RGB pixels. In a separate run using the same computer hostname and `/denyClipboard`, the fresh fake Core initially showed Paste disabled. Copying inside the remote editor enabled its own Paste command and preserved the distinct CppTest text on the Windows clipboard.

The broader `DocumentContextMenu` scenario passed its Ribbon and Toolstrip clipboard commands through automation IO, then timed out waiting for the palette UI Automation provider to be replaced. The independent menu Invoke check above passed. The November 27 pre-change CppTest binary timed out at the same palette step. Runtime interoperability was tested on Windows. The Wayland and macOS imports matched the packed sources after their normal line-ending conversion.

## No.4 Notify listeners of the empty shared clipboard before subscribing to the renderer

When both endpoints allow clipboard injection, start every connection with an empty snapshot and notify application listeners before the renderer can supply its first snapshot. Each read returns the current snapshot immediately. The renderer subscription and the acknowledged write already provide the required synchronization.

### CODE CHANGE

Remove the additional read method from the native interface and its Windows implementation. Use the ordinary clipboard reader in the native renderer. Deliver Windows change notifications while the native clipboard is readable and retry a busy notification on the existing timer. Regenerate the GacUI distributions and both Unix imports.

Observe the clipboard inside the Core change callback with immediate and deferred renderer responses. Check text, image and document availability across replacement, and count subscription requests and protocol submissions across repeated reads. Use the existing persistence test protocol for the rich clipboard setup because the embedded document requests its source image format and the standalone image requests PNG. Exercise the document editor with a Windows process holding the clipboard during a change notification.

### DENIED BY USER

The maintainer rejected manufacturing a Windows clipboard-change message on a failed read. Proposal 5 keeps the verified Core connection ordering and removes that native message.

With an immediate renderer reply, the listener received an empty reader and then a reader containing the renderer’s text. On replacement, all three formats from the previous clipboard disappeared in the first callback. Two reads during the delayed reply made zero protocol submissions. The replacement snapshot triggered the next callback. Each connection requested its clipboard once.

The Windows Debug x64 build succeeded and all 178 focused tests passed. The Win32 editor pasted a Unicode fixture present before connection and disabled Paste when the Windows clipboard was cleared. Both the remote editor and CppTest exchanged Unicode text with Notepad. A single-image selection offered both a bitmap and the private document. All 851 RGB pixels matched after pasting in Paint, after copying from Paint, and in both directions between the editors. The remote editor and CppTest also exchanged six styled paragraphs with two embedded images in both directions. The remote editor kept Paste enabled during a forced busy read. CppTest pasted the same document after release without a second external change.

GacUI, wGac and iGac have zero source references to the removed method. All four changed imports in each Unix repository matched the packed GacUI sources after line-ending normalization. Runtime verification used Windows.

## No.5 Deliver renderer snapshots directly to Core clipboard listeners

Core already stores each accepted renderer snapshot and calls `InvokeClipboardUpdated()` synchronously. Its local OS clipboard is independent. Windows supplies its own change notifications when another program actually copies something.

### CODE CHANGE

Remove the Win32 read-retry timer and the code that posts `WM_CLIPBOARDUPDATE`. Check directly in the connection test that an incoming snapshot raises one listener callback with the new cached value and causes zero accesses to the Core-local clipboard service. Exercise genuine native clipboard changes and the busy-read case separately.

With the repost removed, a Windows process copied new text and immediately reopened the clipboard for reading. The OS sequence advanced from 3392 to 3397 for the copy and stayed at 3397 after release. The remote editor lost Paste and still had it disabled after release. Remember a failed native read in the Windows provider and finish it on the existing event loop. The GUI holds the readable native clipboard while delivering that change to listeners. The headless fallback continues to drain its window independently of the injected cache.

### DENIED BY USER

The maintainer changed the handoff before the first snapshot and requested removal of the Windows callback dispatcher. Proposal 6 supersedes the connection-time empty reader. Direct delivery of renderer snapshots and recovery of genuine busy Windows reads still apply. The following observations describe the previous implementation.

The Core listener observed the replacement text and then an empty clipboard in the callbacks for the two corresponding renderer events. Each snapshot produced one synchronous callback. The instrumented Core-local service recorded zero accesses throughout sharing and retained its original contents. The cache reads and the two incoming events issued zero protocol submissions.

A Win32 renderer connected to a Core with a Windows fallback while another process held the Windows clipboard open. Paste was disabled on connection. Releasing the read lock enabled Paste and the remote editor pasted the existing text. The Windows clipboard sequence number was 3508 before connection, while locked and after the paste. The same OS text remained available to other applications.

A separate real Windows copy made while both apps were running advanced the sequence from 3513 to 3518. The remote editor kept Paste enabled while the new contents were busy. It and CppTest pasted the new text after release with the sequence still at 3518. A renderer that denied sharing also recovered the new text through Core’s native fallback after release. The single-image selection retained the private document and all 851 bitmap pixels. A separate Paint window pasted the image and copied all 851 pixels back. Both GacUI editors exchanged Unicode text in both directions with the Notepad editing control. The rich document retained six paragraph alignments, its text runs and both embedded images across the remote editor and CppTest.

The Windows Debug x64 build succeeded and all 199 focused tests passed. Every remaining production reference to `WM_CLIPBOARDUPDATE` in GacUI, wGac and iGac receives a message from Windows. Both platform imports match the generated Windows sources. Runtime checks used Windows.

## No.6 Use the local clipboard until the current renderer supplies data

The presence of a complete renderer snapshot selects the renderer route. Both connect and disconnect discard any previous snapshot before notifying listeners. A replacement that sends no disconnect therefore exposes the local clipboard immediately. The first accepted snapshot, including one with no formats, selects the new renderer. The permission to request and accept snapshots still comes from both endpoints.

### CODE CHANGE

In `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteControllerInjection.cpp`, `GuiRemoteClipboardInjection` retains an immutable reader for the last accepted snapshot. Its absence routes reads, writes and local change notifications to the fallback. Switching sources updates the reader before notifying GacUI. The writer still checks its connection and the revision of the snapshot it submitted.

The Windows GUI and TUI event loops deliver native clipboard changes while holding the readable clipboard. A failed native read stays pending until the clipboard opens. The Windows provider still handles registered PNG, DIBV5 and the WIC conversion required for native bitmap handles. Headless Core supplies the Windows image service for this conversion because its own graphics service produces remote images. Windows remote setup lets exceptions propagate and balances successful COM initialization on a normal return.

Exercise immediate and delayed snapshots, replacement without disconnect, writes during the gap, a present empty snapshot, explicit disconnect, ignored updates and fallback notifications. Check the active reader from inside each callback and count clipboard requests, writes and accesses to the fallback.

### CONFIRMED

The replacement test observed the seeded local text in the callback that retired the previous renderer. A write during the gap updated the fallback and sent zero renderer writes. A local change notified the listener during that gap. Once an empty first snapshot arrived, the application saw no supported formats despite the text still present in the fallback. The previous renderer’s writer was rejected. Disconnecting restored the text written during the gap, and a late renderer update produced no callback.

The Windows Debug x64 solution built successfully. The Core executable was linked in a separate output directory because a running MiniHTTP Core held the usual executable open. All 605 selected cases in 24 files passed, including the eight clipboard configurations and the handoff regression. No leaks were reported. The window and editor fixtures rendered identically. Thirteen generated Workflow listings reordered existing declarations or layout assignments and were excluded after comparison. CodePack regenerated the portable and Windows sources, and the four resulting files matched both Unix platform imports after line-ending normalization.

In the live Ribbon editor, an external process copied a fresh text marker and immediately held the Windows clipboard open for five seconds. The first Paste after release inserted the marker with renderer sharing enabled. Repeating the check with a renderer that denied sharing pasted the second marker through the Windows fallback. Each run used a single external copy.

With only a private, unsupported Windows format present, the open context menu showed Paste in its disabled color while Select All stayed enabled. After one external text copy, Paste changed to its enabled color without editor input. Sampling covered the five-second hold and the following interval. The first subsequent paste inserted the new text.
