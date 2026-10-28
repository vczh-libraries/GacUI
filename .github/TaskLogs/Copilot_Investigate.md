# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- DateTime/Locale/FileSystem in remote protocol
  - When starting GacUI on core side, `SetupRemoteNativeController` accepts an optional struct with 3 options default to false:
    - `injectDateTime`, allowing remote renderer to implement date time.
    - `injectLocale`, allowing remote renderer to implement locale.
    - `injectFileSystem`, allowing remote renderer to implement file system.
    - For each `inject*` is true, an implementation is immediately injected by `GuiRemoteController`. In each injection, it either connects remote side implementation or fallbacks to local implementation via `Get(DateTime|Locale)Injection`.
      - `GuiRemoteControllerInjection.(h|cpp)` stores all injections, `GuiRemoteController` friends all injection classes and send `this` to their constructors, so that injections could use remote protocl or check if the connection still active.
      - In header file on top of each class, a comment block is required to describe which functions are remote and which functions are local, and other important design decision.
  - `Protocol_Injection.txt` will be created for injection related remote protocol requjests, this file should also be added to `Source_GacUI_Core.vcxitems`.
  - DateTime defines client side time zone, by offering utc<->local conversion functions, core side still querying utc at core side.
    - When allowing injection:
      - Remote renderer does not implement it, switch to default implementation.
      - Remote renderer disconnects, switch to default implementation.
    - To keep a good performance, `IDateTimeImpl`'s `LocalToUtcTime`, `UtcToLocalTime` are implemented via remote protocol, `LocalTime` uses `UtcTime` and `UtcToLocalTime`, other functions redirects to `GetDateTimeInjection`.
  - Locale defines client side default language (font list already implemented), by offering local names and format lists.
    - When allowing injection:
      - Remote renderer does not implement it, switch to default implementation.
      - Remote renderer disconnects, switch to default implementation.
    - To keep a good performance, `ILocaleImpl`'s `Format*`, `Compare*`, `Find*`, `*With` redirects to `GetLocaleInjection`. Other functions are implemented via remote protocol.
  - FileSystem, by offering a replacement file system.
    - When allowing injection:
      - Remote renderer does not implement it, switch to empty implementation.
      - Remote renderer disconnects, switch to empty implementation.
      - Empty implementation means a read only file system with nothing.
    - When core side needs to read a file, remote protocol sends the complete content to the core side at once, no streaming is implemented.
    - When core side needs to write a file, remote protocol receives the complete content from the core side at once, no streaming is implemented.
      - `FileSystem::Close` or its destructor will triggers `IFileSystemImpl` with complete file content prepared.
      - When remote protocol disconnects, all active `IFileStreamImpl` stays available, as it is implemented by having a `MemoryStream` inside for buffering. `IFileStreamImpl::Close` would `CHECK_ERROR` to make sure the remote protocol connection still available.
  - `ControllerGlobalConfig` for `ControllerConnect` needs 3 explicit options:
    - `injectDateTime`, remote renderer implements date time.
    - `injectLocale`, remote renderer implements locale.
    - `injectFileSystem`, remote renderer implements file system.
    - Only when each `inject*` at core and renderer are true, core will redirect implementation to renderer via remote protocol.
  - In `GuiRemoteProtocolSingle`:
    - Constructor requires an optional `GuiRemoteProtocolSingle::GlobalConfig` parameter, has all `inject*` field default to `true`, it will be copied to a field called `globalConfig`.
      - In `GuiRemoteRendererSingle::Opened`, `ControllerGlobalConfig` copies `inject*` from `globalConfig` field.
    - When implementing date time, locale and file system:
      - It should respond remote protocol by calling `DateTime`, `Locale`, `filesystem::*`, `FileStream`.
      - It should not respond remote protocol by calling `GetFileSystemInjection`, `GetLocaleInjection`, `GetDateTimeInjection`.
  - `RemotingTest_Core` has optional `/remoteDT`, `/remoteLocale` and `/remoteFS` argument, when any is used, `inject*` becomes true.
  - `RemotingTest_Win32_Renderer` has optional `/remoteDT`, `/remoteLocale` and `/remoteFS` argument, when any is used, `inject*` becomes true.
    - `test.sh` in `wGac` and `iGac` needs to be updated to receive `-remoteDT`, `-remoteLocale` and `-remoteFS`.
  - Update `Project.md` and `(wGac|iGac)/AGENTS.md` to describe these new arguments, but no need to update any `GacUI/.github/Jobs`, they are not included in standard manual verification process.
  - Update KB pages in GacUI to simply introduce these new features.
- Update to `FullControlTest`:
  - There are two document test page, one is ribbon and another is toolstrip. Their save feature is not implemented.
  - You are going to check out related tutorials in `Release` repo and copy their implementation.
  - Features are in `DocumentEditorBase::(LoadAsPrivateFormat|SaveAsPrivateFormat|SaveAsRTF|SaveAsHTML)`.
  - But you are not going to update generated files, instead you need to change the XML so that these 4 functions becomes 4 events:
    - Event handlers are stored in `FullControlTestPalette.(h|cpp)`, rename this file to `FullControlTestEventHandlers.(h|cpp)` since it is no longer dedicated to palette.
    - Both tabs need to subscribe 4 events.
      - Although child controls are "protected", but each instance root (e.g. the main window) offers a way to request for a control by name, use that function to access both tabs with `dynamic_cast` to make events accessible.
    - You can add a new function `demo::AttachFctEvents` to `FullControlTestEventHandlers.(h|cpp)` so that all main functions could call this function instead of subscribe 9 events everywhere.
      - Except `GacUI_Host` project as reflection specific code is needed, manually subscribe events there.
  - At the end, you need to create a document with rich content, including headers, different font/size/color, paragraph alignment, images, and save a private format file to `GacUI/Test/TestDocument.*` (I forget the extension name, it is hardcoded in `FullControlTest`).
    - You can insert `Gaclib.png` and `Video.png` to the document, images will be saved inside the private format, loading the private format won't need original images to exist.
    - This file could be then used in verification, for example, the fake file system implementation has a copy of this file, and see if it could be loaded to the document editor.
- Before running `RemotingTest_Win32_Renderer` to verify, local changes are recommended to push first, in order to verify:
  - You can change `GuiRemoteProtocolSingle` to offer fake implementation, for example, a different locale and format list, a different time zone, a fake file system, and see if they correctly affects dedicated pages in `FullControlTest`. Since local changes are already pushed, you can easily identify code that offers fake implementations and revert them once they are no longer needed. DO NOT check in fake implementations, they are meant to be discarded after finishing this task.
  - You need to turn options in core/renderer on and off and make sure core picks the right implementation.
  - When a renderer disconnects, you can still use core side automation service to read the UI and see if fallback works.
- GacJS is not included in this task. wGac and iGac verification is not included in this task.

## DETAILS

### Existing names and integration points

- The existing renderer class is `GuiRemoteRendererSingle`, declared in `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h`. References above to `GuiRemoteProtocolSingle` mean this class, including the new nested `GlobalConfig`. Preserve the existing `enabledAutomation` constructor argument when adding the optional configuration.
- The Windows renderer project is `RemotingTest_Rendering_Win32`, under `<GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32`. The close/destructor operation described above belongs to `vl::stream::FileStream`, not `FileSystem`.
- Add `Protocol_Injection` to `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocols.txt`. `<GacUI repo>/Test/GacUISrc/Metadata_UpdateProtocol/Main.cpp` reads this manifest explicitly; registering the new file in the project alone does not include it in generation.
- Register the injection header/source, renderer implementation sources and protocol file in the owning project inventories and applicable filters. Run `Metadata_UpdateProtocol` to regenerate the protocol schema, metadata and generated methods; do not edit generated outputs manually. Update handwritten protocol implementations and test doubles affected by the generated interfaces.
- The relevant design guidance is in `<GacUI repo>/.github/KnowledgeBase/KB_GacUI_Design_RemoteProtocolCoreArchitecture.md`, `<GacUI repo>/.github/KnowledgeBase/KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md`, and `<GacUI repo>/.github/KnowledgeBase/KB_Vlpp_Design_ImplementingInjectableFeature.md`. DateTime, locale, filesystem and stream contracts are documented in the corresponding Vlpp/VlppOS API pages linked from the knowledge-base index.

### Configuration and fallback

- Keep the library renderer configuration defaults `true`, as requested. Each renderer demo must explicitly initialize all three flags to `false`, then enable only the command-line switches present. Core configuration defaults remain `false`. This makes omission of a demo switch actually disable that capability.
- Store core preferences separately from the renderer's advertised capabilities. Re-evaluate each feature independently on every connection, including renderer replacement. The effective behavior is:

| Core option | Renderer connected and advertising the option | DateTime / Locale | FileSystem |
| --- | --- | --- | --- |
| false | either | Previous implementation; no wrapper installed | Previous implementation; no wrapper installed |
| true | false | Previous implementation | Empty read-only implementation |
| true | true | Requested remote/local method split | Renderer filesystem |

- Before the first connection, use the same fallback as after disconnection. Do not issue a synchronous injection request while the renderer is absent or does not advertise that feature.
- Initialize all three new `ControllerGlobalConfig` capability fields at every construction site, including unit-test/custom renderer configurations. Generated structs currently have no member initializers, and assigning only the old fields would leave the new booleans indeterminate. Test renderers advertise `false` unless a case implements the feature. Received messages that omit these fields must also leave them `false`. Keep this distinct from the native renderer's default configuration. Verify deserialization of a connect message lacking the new fields; updating GacJS or promising compatibility between a new renderer and an older core is outside this task.
- Add the exact slash switches to the native parsers in `<wGac repo>/RemotingTest_Rendering_Wayland/Main.cpp` and `<iGac repo>/RemotingTest_Rendering_macOS/Main.cpp`, and pass the configuration through to renderer construction. The shell scripts alone cannot enable a feature that the executable rejects.
- In `<wGac repo>/test.sh` and `<iGac repo>/test.sh`, forward the requested single-dash options to the corresponding slash switches in both foreground and `--unblock` renderer launch paths. Keep them renderer-specific and preserve existing transport/port handling. Their renderer `GuiMain.cpp` is copied from the Windows renderer by each repository's `syncProj.sh`; update the canonical source and use that synchronization workflow. Native execution in those repositories remains excluded.

### Injection lifetime, dispatch and connection changes

- Derive wrappers from the existing `feature_injection::FeatureImpl<T>` mechanism and delegate local/fallback methods through `Previous()`. `GetDateTimeInjection().Get()` and `GetLocaleInjection().Get()` return the active wrapper after injection, so calling them from that wrapper would recurse. The predecessor may itself be an application or unit-test injection; do not replace it with a direct OS lookup.
- Let the controller own its wrappers, install each enabled wrapper once, and explicitly eject it before destruction. Connection changes select the route within a wrapper rather than modifying the global injection chain. Preserve the prior chain after controller teardown and repeated setup/teardown.
- Synchronous remote work must run on the core UI thread. This cannot become a restriction that rejects all worker-thread callers: `FileDialogTaskQueue` in `<GacUI repo>/Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp` already enumerates folders/files through `InvokeAsync`. Execute directly on the UI thread; otherwise marshal through the controller's async service and wait for completion. Recheck connection/capability inside the dispatched operation. Do not hold locks across that wait or let workers modify `GuiRemoteMessages`.
- Account for startup before an event loop is available and teardown after it stops: fallback must remain usable, and outstanding dispatched calls must finish or be released before the controller/injections are destroyed. A buffered stream must never dereference a destroyed controller. Keep injection/ejection outside concurrent feature use.
- Reuse `GuiRemoteMessages` request/submit/retrieve and the existing channel cancellation mechanism. `GuiRemoteProtocolFilter::Submit` rejects recursive submission, and the async channel allows only one pending request group. Core-side serialization and path/conversion helpers must not recursively call the remote wrapper they are implementing.
- Check the `disconnected` result before retrieving any response. A connection can disappear during a request even after the initial capability check. DateTime/Locale then use their predecessor; filesystem queries use the empty behavior, while an attempted stream commit fails as required. Do not retry a mutation on a replacement renderer.
- Bind each opened remote stream to its original connection identity, not merely to a boolean saying some renderer is connected. Replacement invalidates its right to write back, even when the replacement advertises filesystem support. Reconnect must not reuse stale capability-dependent results or deliver an old response to a new request.

### DateTime and Locale contracts

- Date/time values on the wire must be portable. Windows and Unix use different `DateTime::osInternal` epochs/units; do not send an opaque value from one platform and interpret it as native on the other. Use calendar fields or an explicitly portable representation, reconstruct with the destination's public `DateTime` API, and return values in the core's representation. Preserve milliseconds and apply the renderer's timezone rules for the requested date, including daylight-saving transitions, rather than caching one current offset.
- Keep `UtcTime`, construction and arithmetic on the predecessor. Implement `LocalTime` from the core's UTC clock and the selected UTC-to-local conversion; it must not substitute the renderer's clock.
- The remote part of `ILocaleImpl` includes `Invariant`, `SystemDefault`, `UserDefault`, locale enumeration, the date/time format lists, day/month names, and `ToLower`, `ToUpper`, `ToLinguisticLower`, `ToLinguisticUpper`. The requested local part includes all `Format*`, `Compare*`, `Find*`, `StartsWith` and `EndsWith` methods. Preserve list ordering and the locale name supplied to each operation. This split does not promise that the core formatter supports every locale available on a different renderer OS.
- The renderer implements these operations through the public `DateTime`, `Locale`, filesystem and stream APIs, as requested. Core fallback uses the predecessor. Keep these two delegation rules distinct in the required class comments.
- Switching injections changes subsequent API calls. It does not rewrite previously constructed `DateTime`, `Locale` or `FilePath` values, or silently replace an application-selected locale. `GuiApplication` captures `Locale::UserDefault()` in its constructor, and FullControlTest captures other values; verification must make fresh calls when checking a transition.

### Filesystem and buffered streams

- Cover the entire `IFileSystemImpl` contract in `<GacUI repo>/Import/VlppOS.h`: path delimiter, initialization, joining and relative paths; file/folder queries and mutations; enumeration; `GetFileInfo`; `FileCopy`; and stream creation. Paths must use the renderer's rules while connected. Do not normalize a renderer path using the core OS or accidentally access core storage while implementing an injected method.
- The empty implementation needs a usable virtual root and deterministic structural path operations. Root enumeration succeeds with empty lists; ordinary files/folders do not exist; mutation/open operations fail using their normal return contracts. Define its root/delimiter behavior before first connection and after loss of a renderer, without falling through to the core filesystem. Previously created paths must not gain access to core storage through fallback.
- `FileStream` immediately calls `GetFileStreamImpl(...)->Open()`. An unavailable filesystem must return a non-null stream implementation whose `Open()` fails, not a null pointer. Keep ordinary missing-file/access failures as false/unavailable results; preserve the existing assertion contract for metadata queries on nonexistent entries.
- Preserve all `FileInfo` fields, including optional UTC timestamps and flags. Use portable timestamp serialization here too, and do not truncate file sizes or counts in the protocol representation. Implement renderer-side copy through the public metadata-preserving `File::CopyToFile` contract rather than replacing it with text or plain byte copying.
- Whole-file payloads are bytes, not `WString` text. Reuse the existing `Ptr<stream::MemoryStream>` protocol serialization, which encodes binary content with Base64, and distinguish a successful empty file from a failed open/read. Keep encoding/BOM handling in the existing higher-level APIs.
- Preserve the current native `FileStream` access-mode behavior unless implementation reveals a contract that needs a further review: `ReadOnly` snapshots the complete file; `WriteOnly` and native `ReadWrite` start empty because the native implementation opens them with truncation. The OPFS implementation preserves existing bytes for `ReadWrite`, so it is not a drop-in behavioral reference. Document the selected native behavior in the buffered implementation's class comment.
- Validate opening and access rights before reporting an available stream. The remote design defers persistence to close; specify that buffered writes become visible to newly opened readers after a successful close, while already opened readers retain their snapshot. Test creation, truncation and errors at the appropriate open/commit boundary.
- Delegate local read/write/seek/peek/position/size operations to the buffer while enforcing the original access rights. Writable close uploads the complete buffer once, including an empty buffer that creates/truncates a file. Read-only close never uploads. Explicit close followed by destruction must not upload twice; closing an already closed stream is harmless. Check writeback success rather than silently losing the buffered data.
- Preserve the requested failure on closing a still-open remote stream after its connection is lost, including read-only streams; buffer availability does not grant permission to close against a different renderer. Destruction invokes the same close path, so test this intentional failure separately from ordinary successful shutdown. No streaming, retry-on-reconnect or shutdown acknowledgement is introduced.

### FullControlTest document persistence

- The private format extension is `.bin`, from the dialogs in `<GacUI repo>/Test/Resources/App/FullControlTest/DocumentEditorBase.xml`. The requested committed fixture is `<GacUI repo>/Test/TestDocument.bin`. Use `<Release repo>/Tutorial/GacUI_Controls/DocumentEditor/UI/Source/DocumentEditorBase.cpp` as the implementation reference; the APIs are declared in `<GacUI repo>/Source/Resources/GuiDocumentClipboard.h`.
- Replace the four `@cpp:UserImpl` methods with public filename-taking events in the authored XML, preserving their names and the existing invocation sites. Remove their placeholder bodies and the unused `dialogNotImpl`. Regenerate both architectures and reflection metadata rather than editing generated methods or registrations.
- Use `GuiInstanceRootObject::GetNamedObject`, declared in `<GacUI repo>/Source/Application/Controls/GuiInstanceRootObject.h`, to find `editorRibbon` and `editorToolstrip` from the main window. Obtain their raw objects and use `dynamic_cast` to the generated editor type as requested. Look up each editor's own `document` viewer within that editor's instance scope. Each event subscription must use its corresponding viewer, so the two tabs can hold and save independent documents.
- Attach once per main-window instance: four events per editor plus `PaletteSelected`. Keep the existing deferred palette/theme refresh behavior, but perform document load/save synchronously on the UI thread. `SaveDocument()` marks the document saved immediately after the private-save event returns, so serialization and stream close, including remote writeback, must finish before returning. RTF/HTML export must retain the current behavior of leaving the private document modified. `CancelWindowClose()` has the same synchronous-save assumption, but FullControlTest currently does not call it; adding a new main-window close-confirmation flow is outside this change.
- Keep the shared persistence implementations independent of generated FullControlTest types: take the public `GuiDocumentViewer*` and filename. `GacUI_Host` compiles this helper source but loads the UI from Workflow binary, so putting generated editor casts in the common `.cpp` would introduce generated-class RTTI/link dependencies. A suitable arrangement is a header template for `demo::AttachFctEvents`, parameterized by the generated editor type and instantiated only by generated-code entry points. `GacUI_Host` uses named object values and reflection `AttachEvent` for all nine subscriptions, calling the same native persistence functions. No reflection-mode conditional is needed in the shared handlers.
- Follow the tutorial's full-document snapshot approach (`SelectAll` and `GetSelectionModel`) before serializing. For private files, call `ModifyDocumentForClipboard` and `SaveDocumentToClipboardStream`; load with `LoadDocumentFromClipboardStream` and then `SetDocument`. For RTF, use `SaveDocumentToRtf`; for HTML, write the header/content/footer byte strings from `SaveDocumentToHtmlUtf8` in order. Do not serialize the live model directly: clipboard preparation removes embedded-control runs, and private serialization rewrites image source names.
- Check stream availability, write completion and the loaded model using the repository's fail-fast conventions. A failed load must not pass null to `SetDocument`, which clears undo/modified state and replaces the current document. A failed write or disconnected commit must not return as though the save succeeded. Canceled dialogs do not invoke persistence handlers.
- Remote image persistence is required supporting work. `GuiRemoteGraphicsImage::SaveToStream` in `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteGraphics_ImageService.cpp` currently always fails, while all three document exporters call it for image runs. Extend the existing image protocol in `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_Renderer_ImageFrame.txt` to request encoded bytes for an image ID and target format, and implement the native renderer response using `INativeImage::SaveToStream`. Ensure the image exists on the current renderer, respect request cancellation before retrieving/writing a response, and fail an interrupted export rather than emitting missing image data. RTF specifically requests PNG, so retained original bytes are only a valid shortcut when their encoding matches the requested format. This image operation must also work when remote filesystem injection is disabled and the output file belongs to the core filesystem.
- Rename the manually maintained `<GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestPalette.h` and `.cpp` to `FullControlTestEventHandlers.h` and `.cpp`; despite their parent folder name, these two files are authored sources. Update includes, project/filter entries, the separate `GacUI_Host` inventory, native entry points and documentation references. Also update the Linux core and WasmFCT source inventories and the shared Wasm entry point within GacUI, plus wGac/iGac synchronization scripts and native FullControlTest callers. Their runtime exclusions and the GacJS exclusion remain in place; the rename must not leave consumers pointing at nonexistent files.
- Build the rich fixture with the completed editor/serializer, including recognizable heading text, varied font face/size/color, multiple paragraph alignments and both images. Existing image inputs are `<GacUI repo>/Test/Resources/App/Gaclib.png` and `<GacUI repo>/Test/Resources/App/FullControlTest/ResponsiveImages/Video.png`. The ribbon's Header 1/2/3 styles provide heading examples. Private serialization embeds image bytes and uses internal `res://Image_n` references; commit the `.bin` fixture without machine-specific external image dependencies. Use separate output files for mutation tests so the committed fixture remains stable.

## VERIFICATION

### Generation, build and regression checks

1. Use the repository build/execute wrappers from `<GacUI repo>/Test/GacUISrc` according to `<GacUI repo>/Project.md`. Run `Metadata_UpdateProtocol` after schema/manifest changes and rebuild Debug x64. Inspect the generated injection methods, capability fields and metadata; verify all new source files are compiled through their owning inventories.
2. Run ordinary `UnitTest` coverage for the core injection contracts, serialization, startup and async connection behavior. Keep existing relevant file filters and ensure new tests actually run. Check the final test summary and Debug memory-leak output. Use the non-rendering protocol helpers where appropriate; do not create empty or meaningless rendering snapshots for API assertions.
3. The required FullControlTest XML changes make `GacUI_Compiler` generation, the Win32/x64 `Metadata_Generate` sequence, x64 `Metadata_Test`, and updates/runs of both FullControlTest UIA verification wrappers mandatory, as described in `<GacUI repo>/Project.md`. Confirm the four events replace the former generated methods and reflection registrations. Rebuild all affected entry points, including `GacUI_Host`, after regenerating.
4. Keep authored changes, generated outputs and any changed snapshots in their required separate commits. Update the requested project/platform documentation and relevant KB pages through the established shared-guidance workflow. Do not modify standard remote-protocol verification jobs or add GacJS work. Record wGac/iGac source synchronization and option forwarding as static inspection, not runtime verification.

### Deterministic API and protocol coverage

1. For each feature, exercise all four core/renderer flag combinations in the table, plus no renderer yet, disconnect, reconnect with changed capabilities, and replacement while the old renderer is still active. Cover individual flags and all three together. Assert remote request counts as well as returned values so identical local/remote environments cannot hide incorrect routing.
2. Install distinguishable predecessor implementations and verify local methods/fallback delegate to them without recursion. Check repeated controller setup/teardown restores the prior chain. Cover a connect message with omitted capability fields and cancellation between request submission and response retrieval.
3. DateTime: use a known core UTC clock and a different renderer timezone/clock; verify local time uses only the core clock. Cover both conversion directions, milliseconds, day/month/year rollover and dates with different daylight-saving offsets. Test portable serialization independently of the Windows-only end-to-end run.
4. Locale: distinguish renderer defaults, enumeration, format lists, day/month names and case conversions from the predecessor; verify formatting, comparison, searching and prefix/suffix operations remain local. Test fallback with fresh calls rather than previously cached locale objects or labels.
5. Filesystem: verify renderer path rules, root enumeration, nested directories, metadata/copy results and ordinary error returns. Cover empty files, embedded NUL/arbitrary binary bytes, a payload larger than a transport read buffer, each access mode, seek/peek/position/size, denied opens, missing parents, zero-write close, destructor writeback and explicit-close idempotence. Confirm no core-side file is read or changed when remote filesystem injection is enabled but unavailable.
6. Keep a stream open across disconnect and verify buffered operations still work. Separately verify the specified close/destructor failure and rejection of writeback after a replacement connects. Disconnect during a pending read/query/commit; ensure waiters terminate and no response is retrieved after cancellation. Exercise worker-thread folder enumeration concurrently with normal UI work, and shut down with a dispatched operation pending to expose deadlocks or lifetime errors.

### Windows end-to-end verification

1. Establish a clean committed/pushed implementation checkpoint before temporary renderer overrides. Use distinguishable timezone, locale lists/formats and filesystem contents, recording expected values before each run. Keep these temporary overrides out of commits; permanent unit-test fixtures belong only in test code.
2. Launch `RemotingTest_Core` with `/FCT` and the Windows renderer with a matching transport. Use core `/AsPort:8888` and renderer `/AsPort:8889`; retain the fixed protocol endpoint rules. Perform the complete flag/fallback matrix on one transport and a focused all-enabled/disconnect/reconnect smoke check on the other supported Windows transports (`/Pipe`, `/Http`, `/MiniHttp`).
3. Inspect core `Controls` and renderer `Dom`, and exercise IO through both endpoints. Use the DatePicker and Localization pages for calendar names/formats and the fake Open/Save FileDialog pages for directory browsing. The standalone file-dialog page returns selected paths; use the newly implemented document-editor commands for actual content-transfer checks as detailed below, alongside the API tests above.
4. Make fresh queries for every transition. `<GacUI repo>/Test/Resources/App/FullControlTest/LocalizedStringsTabPage.xml` captures local time once, `<GacUI repo>/Test/Resources/App/FullControlTest/LocalizedComponents.xml` hardcodes two locales, and `GuiApplication` captures the default locale. Recreate/requery appropriate controls or provide a focused query action that displays newly obtained results; merely reading the unchanged UI is insufficient. Verify locale enumeration directly as well.
5. Disconnect the renderer while leaving the core running. Through core automation, trigger fresh DateTime/Locale queries and reopen/refresh filesystem browsing, then assert predecessor results or empty listings. Reconnect with different flags/values and repeat, checking that the core remains responsive and uses the new capabilities. Include a fake file-dialog enumeration in progress during loss of the renderer.
6. Remove only the temporary renderer overrides, rebuild, and run a final smoke check against the public native implementations. Review the final diff before committing/pushing the completed implementation. Record any unavailable Windows UI coverage explicitly; wGac/iGac runtime verification and GacJS remain excluded.

### FullControlTest persistence and fixture verification

1. Exercise all four commands through both the ribbon and toolstrip UI, first with different recognizable content in each tab. Verify one operation per invocation and that each handler acts on the correct editor. Repeat after palette/theme refresh to catch duplicate subscriptions. Cover generated no-reflection, metadata-only and full-reflection applications, plus `GacUI_Host` with its reflection-only event attachment.
2. Save private files, change the current editor contents, and reload. Compare text and paragraph order, effective heading/font/size/color styles, paragraph alignment and images. Load the ribbon's saved document into the toolstrip editor and vice versa. Confirm private save clears modified state only after successful persistence, whereas RTF/HTML export preserves it. Cancel both open and save dialogs and verify neither the document nor an existing output file changes.
3. Inspect exported RTF/HTML for representative Unicode text, formatting and image payloads supported by the existing serializers; file existence alone is insufficient. Cover a non-PNG image as well as the PNG fixture images to prove the remote RTF path actually produces PNG data. Exercise remote image export both with renderer-backed file output and with core-local file output, before an image is first rendered and after renderer replacement. The current `EnsureMetadata()` returns silently on disconnect, so explicitly test that this path cannot be mistaken for successful image creation/export. Use deterministic protocol responses in ordinary tests and real native codecs in the Windows end-to-end checks.
4. Create and commit `<GacUI repo>/Test/TestDocument.bin`, then reload it in a fresh process with only that fixture available in the fake renderer filesystem. Make the original image paths unavailable in that isolated filesystem without deleting repository assets. Verify both embedded images still decode and render, so neither prior caches nor access to the core's image files can conceal missing embedded data.
5. Enable `/remoteFS` at both ends, load the fixture, edit it, save to a distinct renderer-side destination, complete close/writeback, then reopen and compare rich content. Verify the destination bytes are on the renderer and no same-named core file was touched. Repeat the applicable disabled-capability and disconnected cases from the filesystem matrix; do not interpret an unavailable open/save or failed image export as success. Test cancellation during image encoding and during file commit separately.
6. Extend `<GacUI repo>/Test/UIA_CppTest.ps1`, `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1` and their shared assertions with actual load/save/export/cancel operations for both editors, plus output-content checks. Retain current editing, formatting, resize and palette assertions. Use automation for the remote pair and verify the corresponding local controls through UIA; a remote renderer does not transport a semantic UIA tree.
7. Search all affected inventories, includes and synchronization scripts for stale `FullControlTestPalette` references. Verify regenerated sources and metadata have the expected event signatures, Host has no generated FullControlTest link dependency, and the committed fixture is present. Use ordinary unit tests for serializer/event-routing/image-protocol assertions; any added rendering snapshot must capture useful rich-document output.

## REVIEW COMMENTS

No unresolved review comments. The missing details above have reasonable resolutions based on the existing APIs and repository conventions.

# TEST [CONFIRMED]

The initial Debug x64 build succeeded with zero warnings/errors. The ordinary `TestRemote_Startup.cpp` case `Remote image export supports document persistence` reproduced the `SaveToStream` assertion, `This function should not be called`, without issuing a renderer request. This established the missing image-export path needed by document persistence.

Success requires the capability/fallback matrix, predecessor restoration, portable time and metadata values, complete filesystem and buffered-stream contracts, cancellation and worker dispatch, and document event routing to pass ordinary tests with clean Debug leak output. It also requires generated metadata, both native UIA All suites, persistence in the reflection variants, an isolated rich fixture round trip, Windows transport transitions, and removal of temporary renderer overrides. Transport notification limits and excluded platform runtime coverage are recorded explicitly below.

# PROPOSALS

- No.1 Controller-owned injection chain with connection-bound protocol operations [CONFIRMED]

## No.1 Controller-owned injection chain with connection-bound protocol operations

Install each enabled wrapper once, delegate DateTime/Locale local methods through its predecessor, and select remote routing from the current renderer capabilities. Exchange calendar fields instead of OS timestamps. Use shared dispatch state to marshal worker calls, release queued work at shutdown, and invalidate buffered streams when their original connection ends. An unavailable remote filesystem exposes a deterministic empty virtual root. Renderer handlers use public APIs, and buffered writeback completes synchronously before a document save returns.

Replace the four document placeholder methods with events, attach each editor to its own viewer, and keep shared persistence independent of generated classes. Encode exported images through the renderer independently of the selected filesystem. Regenerate owned outputs and verify the rich fixture, UIA interactions and transport matrix.

### CODE CHANGE

- Added `Protocol_Injection` and registered its schema, generated interfaces and source inventories. Core configuration defaults to disabled; native renderer library capabilities default to enabled. Demo arguments independently opt in to `/remoteDT`, `/remoteLocale` and `/remoteFS`. Older connect messages without the capability fields deserialize them as false.
- Added controller-owned DateTime, Locale and FileSystem wrappers in `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteControllerInjection.h/.cpp`. Class comments describe remote/local routing, fallback, worker dispatch and buffered stream behavior. DateTime keeps the core UTC clock and predecessor construction/arithmetic; Locale preserves the requested remote/local method split. Controller teardown restores the prior injection chain.
- Implemented all filesystem operations using renderer path rules, portable optional UTC metadata and decimal representations of 64-bit values. Empty byte payloads deserialize as available empty streams. Unavailable filesystems never fall through to core storage. Read-only streams hold snapshots; native write/read-write opens validate access and truncate, and writable close uploads once, including empty files. Every open stream remains bound to its original connection, including the specified close/destructor failure after disconnection.
- Implemented native handlers through public DateTime, Locale, FilePath, File, Folder and FileStream APIs. Added the public `FilePath::GetCompatibleDelimiters` accessor in VlppOS. Renderer read/commit responses reject incomplete transfers; ordinary tests force short reads/writes and verify successful empty commits and stream closure.
- Fixed asynchronous channel detachment to cancel waiters, deliver disconnection on the event thread, reject stale responses and close the waiter-registration race. Shared injection state releases pending workers when the UI loop stops and avoids dereferencing a destroyed controller.
- Implemented renderer image encoding for remote `SaveToStream`, including conversion to PNG for RTF, connection/cancellation checks, frame-count metadata revalidation and creation bytes before first paint. Shared document writes check the actual copied byte count before closing.
- Converted the four FullControlTest persistence methods into filename-bearing events, renamed the shared helper to `FullControlTestEventHandlers`, and attached each editor once through `AttachFctEvents`. Host uses reflection event attachment and `SafeAggregationCast<GuiInstanceRootObject>` to resolve its Workflow aggregate's native viewer without a generated-class dependency. Added `<GacUI repo>/Test/TestDocument.bin`, containing six rich paragraphs and two embedded images.
- Extended shared UIA verification with both editors' private round trips, cross-editor loading, RTF/HTML content, canceled dialogs and palette refresh. The harness waits for completed document operations and retired providers, retries only idempotent menu expansion, scopes notification subscriptions to their assertions, uses rebuild-aware tab selection, and waits for normal process exit after Window.Close. Existing editing, formatting, resize, notification and palette assertions remain.
- Regenerated protocol/resource bindings, metadata and release sources using their owning generators. Updated source inventories, native platform parsers, foreground/background launcher forwarding, project documentation and canonical Tools knowledge pages, then synchronized platform sources/imports and repository knowledge copies. Generated changes were committed separately from authored changes. Temporary verification providers, query buttons and debugger probes were removed from native entry points.

### CONFIRMED

Final verification passed with the retained implementation:

| Verification | Result |
| --- | --- |
| GacUI Debug x64 and Win32 full solution builds | Zero warnings and errors |
| GacUI x64 ordinary tests | 272/272 cases, 14/14 files; clean Debug leak output |
| GacUI Win32 ordinary tests | 252/252 cases, 12/12 files; clean Debug leak output |
| VlppOS Debug x64 | Zero warnings/errors; 311/311 cases, 16/16 files; clean leak output |
| Protocol/resource generation | Metadata_UpdateProtocol and GacUI_Compiler completed successfully |
| Required metadata sequence | Win32 Metadata_Generate, x64 rebuild, x64 Metadata_Generate and Metadata_Test completed successfully |
| CppTest UIA All | 15,464 assertions; normal shutdown and endpoint release |
| CppTest_Metaonly UIA All | 15,430 assertions; normal shutdown and endpoint release |
| CppTest_Reflection Persistence | 363 assertions; normal shutdown and endpoint release |
| GacUI_Host Persistence | 363 assertions; normal shutdown and endpoint release |
| Pipe capability matrix | All 64 core/renderer flag combinations, queried through both automation endpoints: 128 passing checks |
| Clean public-native final smoke | Rich fixture load, styled RTF with two PNG payloads, native filesystem dialogs, both automation endpoints and DatePicker passed |

The matrix confirms independent feature selection, including core-disabled features, renderer-disabled features and unavailable filesystem behavior. Distinguishable temporary core clocks and renderer timezones/locales demonstrated that LocalTime uses the core UTC clock with renderer conversion, while fresh fallback calls use the predecessor. Reconnection selected the new renderer values. DatePicker rebuilding displayed renderer month names; Localization refresh used renderer date formats while retaining its captured timestamp and predecessor formatting behavior.

The isolated renderer filesystem loaded the committed fixture with its original image paths unavailable. Core Controls exposed all six rich paragraphs and both internal image references; renderer Dom exposed both image frames. Load, edit, private save/reopen and styled HTML/RTF export succeeded. Native JPEG-to-PNG conversion passed before first painting and after renderer replacement, with both renderer-backed and core-local file output. Disabled filesystem capability reported unavailable persistence and an empty root. The final clean native pair repeated fixture loading and verified two PNG payloads in its RTF output after all temporary overrides were removed.

CDB confirmed pending file-dialog enumeration reached the renderer through public `Folder::GetFiles`. Killing the pipe renderer there released the core operation; fresh queries returned predecessor time/locale, persistence became unavailable, and a reopened dialog listed `/` with no files. A separate UI-loop shutdown check observed a queued injection call and waiting worker, then completed after continuation. The disposable destructor probe reached the required failed close through `FileStream::~FileStream`; ordinary tests cover explicit close, replacement rejection, worker cancellation, stale responses and surviving stream state.

HTTP and MiniHTTP all-enabled queries, persistence, server-initiated disconnection through the public channel-server API, fallback and replacement all passed. Arbitrary abrupt HTTP peer loss has a remaining transport detection limitation: one HTTP hard-kill run passed, but another HTTP run and MiniHTTP hard-kill runs timed out. CDB showed the raw HTTP server retaining its logical client with an idle pending poll, no outgoing messages or acknowledgement timeout, and no disconnect callback. Injection cancellation works once disconnection is delivered; these runs do not establish reliable silent-peer-loss detection. No heartbeat or shutdown acknowledgement was added, consistent with repository guidance.

All final UIA logs contain no FAIL or UNVERIFIED result. The persistence checks cover both editors before and after palette refresh, output contents, private modified-state reset, export state preservation and cancellation. Host's aggregation correction passed the same scenario. The two full All suites additionally retain the existing control coverage and notification checks, and both close normally with their automation endpoints released.

Final evidence is retained locally under `<GacUI repo>/Test/Output/Injection/`: `unit-transfer-check-x64.log`, `unit-transfer-check-win32.log`, `uia-cpptest-final.log`, `uia-metaonly-final.log`, `uia-reflection-final.log`, `uia-host-final.log`, `remote-matrix.log`, `remote-pipe.log`, `remote-local-output.log`, `remote-disabled-filesystem.log`, `remote-http-server-disconnect.log`, `remote-minihttp-server-disconnect.log` and `public-smoke-final.log`. Metadata driver and release-generation logs are in the same ignored output folder.

Final review found no stale `FullControlTestPalette` references or temporary injection providers in native entry points. Platform shell syntax, parser/argument forwarding, synchronization and generated imports were checked; wGac/iGac native execution remains excluded as requested. GacJS was not changed. The implementation, fixture, generated artifacts and knowledge updates were committed and pushed in GacUI, VlppOS, wGac, iGac and Tools.
