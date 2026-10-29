This is a continuation of previous works:
- TODO_Injection.md
- TODO_Injection2.md

When you are on Linux, you are working on `wGac` repo.
When you are on macOS, you are working on `iGac` repo.
According to previous works, make sure new features work on `wGac` or `iGac`:
- Make sure `./import.sh` and `./syncProj.sh` properly updates source files.
- `./test.sh --app:renderer` with `/remoteDT`, `/remoteLocale`, `/remoteFS`, `/corehost`.
- As described in TODO_Injection.md, verification could be done by adding temporary fake injection implementation on the renderer side, but it should not be committed.

commit and push all local changes once finishing.

## DETAILS

### Scope and source ownership

- This task verifies the completed features from `<GacUI repo>/TODO_Injection.md` and `<GacUI repo>/TODO_Injection2.md` on the current native platform. Fix defects exposed by verification in their owning sources and synchronize consumers; do not repeat the original implementation tasks. GacJS and Windows-only transports/UIA verification are outside this task.
- Below, `<platform repo>` means `<wGac repo>` on Linux and `<iGac repo>` on macOS. The shared procedure applies to both; completing it on one platform does not establish that the other passed.
- Read `<platform repo>/AGENTS.md` and the platform's build/run guidance before execution. The feature contracts are documented in `<GacUI repo>/.github/KnowledgeBase/KB_GacUI_Design_RemoteProtocolCoreArchitecture.md` and `<GacUI repo>/.github/KnowledgeBase/KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md`.
- `<platform repo>/import.sh` recreates the imported framework and test-helper snapshots from `<GacUI repo>/Import` and `<GacUI repo>/Release`. It does not publish changes from upstream authored sources. If an upstream fix is necessary, run its prescribed release/import workflow first; do not repair imported or generated copies manually.
- `<platform repo>/syncProj.sh` regenerates application resources/sources and copies the authored FullControlTest event handlers, `<GacUI repo>/Test/GacUISrc/SharedArguments.h`, and `<GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp`. The platform renderer entry points, `<wGac repo>/RemotingTest_Rendering_Wayland/Main.cpp` and `<iGac repo>/RemotingTest_Rendering_macOS/Main.cpp`, remain separately maintained. Check their parsing as well as the synchronized shared composition.

### Shared launch procedure

The slash options in the original task describe executable arguments. Use these wrapper options from the selected platform repository root:

| Purpose | Wrapper option | Forwarded executable option |
| --- | --- | --- |
| DateTime injection | `-remoteDT` | `/remoteDT` |
| Locale injection | `-remoteLocale` | `/remoteLocale` |
| Filesystem injection | `-remoteFS` | `/remoteFS` |
| Renderer destination | `--corehost:<host>` | `/corehost:<host>` |
| Renderer automation port | `--port:8889` | `/port:8889` |

1. Run `./import.sh`, then `./syncProj.sh`, then `./build.sh` from `<platform repo>`. Use the repository wrappers rather than invoking their underlying build tools directly.
2. Start Core, then start the renderer in a second interactive session. An all-enabled local run is:

   ```bash
   ./test_core.sh --app:fct --protocol:minihttp -remoteDT -remoteLocale -remoteFS
   ```

   ```bash
   ./test.sh --app:renderer --corehost:localhost --port:8889 -remoteDT -remoteLocale -remoteFS
   ```

3. Both wrappers support `--unblock`; verify foreground and background forwarding. `<platform repo>/test_core.sh` rebuilds the sibling GacUI Core on each invocation. `<platform repo>/test.sh` uses an already-built renderer, so rebuild it after changing instrumentation or imported sources.
4. Both native renderer wrappers select `/MiniHttp`. Protocol traffic always uses Core port 8888; renderer `--port` only selects its local automation listener. Keep automation ports distinct for processes on the same computer. Core is the server and does not accept `--corehost`.
5. Use Core `http://localhost:8888/Automation/RemotingTest_Core/Controls` and `/IO`, and renderer `http://localhost:8889/Automation/RemotingTest_Rendering_Native/Dom` and `/IO`. For a remote Core, query its endpoint on that machine. Prefer these automation services for interaction, then the platform's available UI tools; record any desktop-only coverage that remains unavailable.

### Observable feature contracts

- Core configuration defaults all three features to false. The library renderer configuration defaults to true, but both demo entry points explicitly start with all three false and enable only supplied flags. Apply the flags independently at both ends.
- DateTime remotes timezone conversions, retaining Core's UTC clock, construction and arithmetic. Exchange calendar fields, including milliseconds, rather than native OS timestamp representations. Locale remotes defaults, enumeration, format lists, day/month names and case mapping; formatting, comparison and searching remain on Core's predecessor implementation.
- The current native Unix locale implementation uses `EnUsLocaleImpl`, shared by Linux and macOS. Changing the host OS locale is not sufficient evidence of remote routing. Use distinguishable temporary renderer results and compare the final native run against the renderer's public APIs.
- FullControlTest and `GuiApplication` cache some date/time and locale values. Transition checks require fresh API calls or recreated controls; unchanged labels do not prove fallback. The fixed locale choices in `<GacUI repo>/Test/Resources/App/FullControlTest/LocalizedComponents.xml` do not verify locale enumeration.
- Enabled but unavailable remote filesystem injection exposes an empty read-only `/` root, never Core storage. Buffered readers retain their bytes after disconnection, but closing any still-open stream after disconnect or renderer replacement intentionally fails, including read-only streams. Native `WriteOnly` and `ReadWrite` opens truncate; buffered writeback finishes at successful close. Test intentional failures separately from normal UI sessions.
- Install temporary fake implementations through the public injection APIs in an authored renderer source that is actually compiled, after synchronization. Keep their expected values and native baseline distinguishable. Temporary fresh-query probes may be needed for cached UI values. Remove only these temporary changes before final synchronization, rebuild and native verification; never commit fake implementations, probes or their generated/imported derivatives.

### Details (wGac specific)

- Run in a Wayland session with `WAYLAND_DISPLAY` and `XDG_RUNTIME_DIR` available. wGac requires libdecor and a real runtime decoration plugin. Follow `<wGac repo>/README.md` and, when dependencies are missing on Ubuntu, its documented `<wGac repo>/build-prerequisites-ubuntu.sh` procedure.

### Details (iGac specific)

- Use Apple Command Line Tools and the dependencies documented in `<iGac repo>/readme.md` and `<iGac repo>/Brewfile`. In particular, Homebrew `coreutils` supplies `grealpath` used by the upstream synchronization tools.
- Prefer the normal build during verification. `<iGac repo>/build.sh --rebuild` runs repository-wide `git clean -Xdf`, so preserve needed ignored fixtures and evidence before using it; it is not limited to deleting the build directory.

## VERIFICATION

### Synchronization and native build

1. Record the platform, architecture and relevant repository revisions, along with pre-existing local changes. Run the import/synchronization/build sequence above. Confirm the imported GacUI injection and image-export implementations and VlppOS host-aware socket APIs match the intended upstream snapshots, respecting each repository's line endings.
2. Check shared parser/renderer copies against their canonical sources, platform entry-point option handling, FullControlTest event-handler inclusion, and the absence of stale `FullControlTestPalette` references in active inventories. Repeat synchronization to check that it retains the required sources and produces a usable build; a successful copy alone does not prove the native provider works.
3. Run relevant existing native GacUI tests from `<GacUI repo>/Test/Linux/UnitTest`, using its prescribed build/run workflow and an installed UTF-8 locale. Focused coverage includes `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Startup.cpp`, `<GacUI repo>/Test/GacUISrc/UnitTest/TestDocumentConfig.cpp`, and `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_Ribbon_Gallery.cpp`. Require the actual test summary. Apply the owning repository's additional generation/tests if a fix changes source; mock protocol tests supplement the real renderer checks below.

### Options, routing and fallback

Exercise all four flag combinations for each feature separately, then all three enabled together:

| Core flag | Renderer flag | Expected DateTime / Locale | Expected filesystem |
| --- | --- | --- | --- |
| off | off | Core predecessor | Core predecessor |
| off | on | Core predecessor | Core predecessor |
| on | off | Core predecessor | Empty read-only root |
| on | on | Specified remote/local method split | Renderer filesystem |

1. Check both wrapper launch modes, actual process arguments and observable behavior. Include option reordering, duplicate injection switches, and malformed/empty/duplicate host values; invalid executable options must fail before listeners start. Check that `--corehost` is rejected for non-renderer wrapper targets.
2. Use temporary renderer timezone/locale/filesystem results that differ from Core, and request counts or equivalent diagnostic evidence where identical values could hide incorrect routing. Do not infer success solely from an opened window.
3. DateTime: check both conversion directions, milliseconds, date rollover and dates with different timezone offsets. Verify `LocalTime` converts Core UTC rather than reading the renderer's clock. Locale: check defaults, enumeration, format lists, names and case conversions, while confirming formatting/comparison/search still use Core's implementation.
4. Check before initial connection, after disconnect, after reconnect with changed flags/results, and after replacing a still-connected renderer. Keep Core alive and use its automation endpoint for fresh fallback observations. DateTime/Locale use their predecessor when unavailable; enabled filesystem injection stays empty. Close ordinary streams before lifecycle checks; run the expected stale-stream-close failure separately. Check pending filesystem enumeration during renderer loss for hangs or stale responses.

### Filesystem and document persistence

1. Use an isolated renderer fixture directory and a distinguishable Core-side sentinel. For same-machine routing checks, map the fake renderer filesystem to different backing storage so the same logical path cannot silently access the same native file on both sides. Check path joining/relative paths, nested enumeration, names containing spaces and Unicode, create/rename/copy/delete, metadata, and missing-file/open failures. Include empty files, arbitrary binary bytes and content larger than a transport read buffer. Verify the actual renderer bytes after close, with no same-named Core file read or modified when remote filesystem injection is enabled.
2. Exercise read-only, write-only and read/write access, seek/position/size and close/writeback behavior. Verify native truncation at open, zero-byte writes, buffered-read snapshots and no duplicate writeback after explicit close plus destruction. When remote filesystem support is unavailable, root enumeration succeeds empty and normal open/mutation operations fail without falling through to Core storage.
3. Reuse `<GacUI repo>/Test/TestDocument.bin`. In both ribbon and toolstrip editors, load, edit, save privately to a separate destination, change the document, then reload. Cross-load between editors with different recognizable contents to verify handler ownership. Compare text, formatting, paragraph alignment, both embedded images and the complete style catalog, including unused styles. Verify private save clears modified state only after persistence succeeds; RTF/HTML export preserves it, and canceled dialogs change neither document nor output files.
4. Inspect exported RTF/HTML contents and images, not just file existence. Include a non-PNG image to exercise native PNG encoding for RTF. Repeat image export with remote filesystem enabled and with Core-local output to establish that image encoding is independent of filesystem injection. In a fresh process, load the private fixture while its original image paths are unavailable in the isolated fixture environment; do not delete repository assets.
5. Verify the previous ribbon-popup fix using actual popup items in both wide and collapsed layouts, both style groups and an already-selected item. Check resulting document formatting and preserved selection, then repeat after reopening, resizing and theme refresh. Also exercise selection copy/paste and undo/redo, and confirm private save/reload retains unused styles. An inline gallery selection or changed highlight alone does not verify the popup fix.

### Host selection and lifecycle

1. Test omitted host, explicit `localhost`, `127.0.0.1`, the actual resolvable machine name, a non-loopback address, and available IPv6 addresses, including bracketed input. Pass one quoted `--corehost:<host>` argument; it accepts a host/address without URL, path or separate port. Combine host selection with injection flags and a nondefault renderer automation port to verify they remain independent.
2. For each successful destination, inspect Core `Controls` and renderer `Dom`, perform IO through both endpoints, and verify matching UI state and a real document read/write. Protocol traffic remains on 8888, including the additional MiniHTTP connections; a TCP connection alone is insufficient evidence.
3. When another computer is available, repeat an all-enabled run against its Core and verify renderer-owned file output. Same-machine hostname/IP tests do not establish cross-machine access. Record unavailable remote-machine or IPv6 coverage explicitly instead of marking it passed.
4. Verify renderer disconnect leaves Core available, a replacement renderer uses its own capabilities, and Core exit reaches the renderer's normal disconnected state without a hang. Preserve the existing lifecycle contract; no new disconnect acknowledgement or recovery protocol is required.

### Final native run and delivery

1. Remove temporary injection/query instrumentation, rerun synchronization and rebuild the actual native targets. Repeat an all-enabled run, a disabled-feature baseline, a document save/reload/export and disconnect/reconnect using the real public native implementations.
2. Record exact commands, revisions, expected/observed results and any unavailable coverage in the task's execution log. Distinguish static inspection, mock tests and real native runs; this review itself is not evidence that either port has passed runtime verification.
3. Inspect final diffs for temporary hooks, machine-specific paths and generated fake artifacts. Preserve unrelated local work and keep authored changes, generated/imported outputs and any snapshot changes in the owning repositories' required separate commits. Commit and push all intended local changes, reporting any remaining verification gaps.

## REVIEW COMMENTS

No unresolved review comments. The shared details and verification procedure resolve the identified gaps; the two platform subsections contain only platform-specific setup or build behavior.
