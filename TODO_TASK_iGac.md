# Verify remote clipboard sharing on macOS

## What GacUI added

Remote clipboard sharing is enabled by default at both endpoints. Core subscribes once on an allowed connection. The renderer sends its current clipboard and subsequent changes as complete snapshots that can contain Unicode text, a styled GacUI document with embedded images, and a standalone PNG together. Core updates its in-memory copy before notifying the editors. Paste reads from memory. Copy and Cut wait for the renderer's OS write result. Apps on an independent Core desktop keep their own clipboard.

GacUI now takes a platform clipboard as the headless Core's fallback. iGac builds its own headless `RemotingTest_Core`. Its local clipboard is the macOS general pasteboard, or private memory with the Core-only `--fakeClipboard` switch. Reads, writes and notifications use the fallback before the current renderer's first snapshot, after disconnect, or while either endpoint denies sharing. A first snapshot with no supported formats still selects the renderer.

Win32 and Chromium renderers passed all eight configurations against a Windows Core. Windows checks also covered the iGac launch arguments and packed imports. macOS pasteboard transfers still need a native run.

## What to verify

### Build and launch

1. In a macOS desktop session, run `<iGac repo>/import.sh`, `<iGac repo>/syncProj.sh` and `<iGac repo>/build.sh` in order. Verify the synchronized Core entry points, shared argument parser, FCT/RPT/RVMT resources, editor helpers and RPC initializer link once. The public `SetupRemoteNativeController` in `<iGac repo>/Mac/NativeWindow/OSX/CocoaRemoteApplication.mm` must link against the packed `SetupRemoteNativeControllerInternal` from `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteControllerSetup.cpp`.
2. Start `<iGac repo>/test.sh --app:core --unblock`, then `<iGac repo>/test.sh --app:renderer --port:8889`. Expect `<iGac repo>/build/RemotingTest_Core/bin/RemotingTest_Core` with `/MiniHttp /FCT` on port 8888 and the `RemotingTest_Rendering_macOS` app bundle on automation port 8889. The renderer connects without a GacUI sharing prompt. Use renderer automation for editor commands.
3. In foreground and `--unblock` modes, check the launcher passes `/fakeClipboard /denyClipboard` to Core for `--fakeClipboard --denyClipboard` and `/denyClipboard` to the renderer for `--denyClipboard`. Expect a nonzero exit before launch for `--app:renderer --fakeClipboard`, and before connection for the renderer executable with `/MiniHttp /fakeClipboard`. Check that `<iGac repo>/test_core.sh` selects the platform Core for FCT/RPT/RVMT, preserves RVMT network and stdio host startup, and selects the standalone requester for `cpptest_rvm`.

### Eight clipboard configurations

Start a fresh FullControlTest Core for each row. “On” adds the named switch. In both **Document Editor (Ribbon)** and **Document Editor (Toolstrip)**, select Editable mode and use the visible editing commands. Select All on a known document provides a repeatable Toolstrip selection. The shared rows show the source after the first snapshot.

| Core `--fakeClipboard` | Core `--denyClipboard` | Renderer `--denyClipboard` | Active clipboard |
| --- | --- | --- | --- |
| Off | Off | Off | Renderer pasteboard |
| Off | Off | On | Core pasteboard |
| Off | On | Off | Core pasteboard |
| Off | On | On | Core pasteboard |
| On | Off | Off | Renderer pasteboard |
| On | Off | On | Core fake |
| On | On | Off | Core fake |
| On | On | On | Core fake |

- In **OS-backed rows**, seed the active pasteboard with text before connecting. From another process replace it with an unsupported-only pasteboard type, then with different Unicode text. Watch Paste go from enabled to disabled to enabled before invoking it. Paste must insert the new text. Verify that the headless Core event loop and the renderer's Cocoa timer each detect an external change to the pasteboard's `changeCount` through `<iGac repo>/Mac/NativeWindow/OSX/ServicesImpl/CocoaClipboardService.mm`.
- In **Core fake rows**, both editors must start with Paste disabled and ignore external copies. Cut a known selection, change the OS pasteboard again and paste. The editor must restore the selection and the external client must retain its own value.
- In **every row**, check Cut removes exactly the selected range and one Paste restores it at a new caret. Repeat with styled paragraphs and embedded images from `<GacUI repo>/Test/TestDocument.bin` and exercise a standalone image. On both OS routes exchange Unicode with TextEdit or `pbcopy`/`pbpaste`, a patterned image with Preview or a pasteboard client, and rich content with another GacUI editor, in both directions. Test PNG-only and TIFF-only input. A single-image copy must offer both `com.gaclib.document` and PNG. Compare paragraph alignment, styles, image dimensions, alpha and visible pixels.
- **Trace the connection.** Expect one `InjectionRequestClipboard` per shared connection, then an initial `InjectionUpdateClipboardFromRenderer` and ordered updates. Paste checks and repeated pastes send zero clipboard requests. Each Cut sends one `InjectionUpdateClipboardFromCore`. After a matching success reply, immediate Paste sees the submitted contents or a newer snapshot. The observed OS change must produce no further Core write. Denied rows carry zero clipboard messages.

### Handoff, isolation and write failure

- Keep a fake-fallback Core alive. Replace a sharing renderer with a denying one, then a sharing one. Use `--port:8890` for a concurrent takeover. Even without a preceding disconnect, listeners must see the fallback until the new renderer supplies data. Exercise a delayed first snapshot. A write and a local change during the gap must use the fallback. An empty snapshot must disable Paste, a superseded writer or late update must have no effect, and explicit disconnect must restore the value written during the gap. The focused cases in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Startup.cpp` cover this timing, present empty text and image-only fake clipboard input.
- For the all-Off row use two independent pasteboards with clipboard synchronization disabled. Seed Core with `core-local` and the renderer with `renderer-before`. Connect using `--corehost:<host>` and paste `renderer-before` in the editor. An external renderer-side copy of `renderer-after` must reach the editor. Copy `remote-copy` in the editor and paste it in another renderer-side app. Core must still hold `core-local`. Change Core to `core-new`. The editor must keep `remote-copy` until a denying renderer reconnects, then paste `core-new`. With one shared desktop, trace zero Core fallback reads and writes during sharing and record independent-pasteboard coverage as pending.
- In a separate shared session, make the renderer's pasteboard write return `false` during Cut. Match the reply to the write request ID and verify Core reports the clipboard failure and exits nonzero before deleting the selection. If a native rejection cannot be induced, use a controlled failing write and identify it as simulated in the results.

Record the build, launch arguments, results for both editors in each row, external pasteboard contents and protocol evidence in `<iGac repo>/TestMatrix_NativeRenderer.md`. Identify any check the desktop could not exercise and why.

## Out of scope

Windows and Wayland runs, GacJS, Wasm, website or knowledge-base publication, other transports and DateTime/Locale/filesystem injection. Standalone, TUI and view-model regression suites, unrelated showcase pages, native touch and the known Toolstrip shortcut conflict are outside this clipboard check. A standalone editor serves only as a clipboard peer.
