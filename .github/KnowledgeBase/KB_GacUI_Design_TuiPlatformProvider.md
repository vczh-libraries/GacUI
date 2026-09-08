# GacUI on the terminal provider

The implementation uses VlppOS TUI as the physical renderer and event pump, with the existing hosted controller above it. Start a Windows application with `SetupTuiWindowsRenderer()` and implement the ordinary `GuiMain()`. The entry point is declared in `Source/GacUI.h`; the Windows adapter lives in `Source/PlatformProviders/Windows/TUI/TuiWindowsController.Windows.cpp`.

## Ownership and initialization

`TuiWindowsController` derives from `TuiControllerBase` in `Source/PlatformProviders/TUI/TuiController.h`. Start the ordinary Windows native controller first to supply Windows services, install the TUI callback, and call the blocking `vl::console::TUI::Start` on the application thread.

`ITuiCallback::Starting` runs after terminal takeover. It constructs a `GuiHostedController` around the TUI controller and a `GuiHostedGraphicsResourceManager` around `TuiGraphicsResourceManager`. Publish native, hosted, TUI and graphics globals before initializing the hosted controller and calling `GuiApplicationMain`. This ensures `GuiInitializeUtilities` chooses `FakeTuiDialogService`.

The nested native-window `Run` loop calls Show before pumping `TUI::RunOneCycle` on that same thread. Without Show, the hosted renderer remains correctly suppressed and the terminal stays blank. Do not call a second `TUI::Start` or move the event loop to a worker. The window constructor reads the current TUI buffer dimensions because the initial `BufferSizeChanged` callback normally follows `Starting`, which is already running the nested application. Native destruction sends both Destroying and Destroyed notifications.

On normal exit, application utilities finalize while `GetTuiApplication()` is still valid. Finalize hosted windows, uninstall the graphics listener, clear the graphics/TUI/hosted globals and restore the service controller. Request TUI stop. After the outer Start returns, uninstall the TUI callback, destroy the TUI adapter, and finally stop Windows services and COM.

`ITuiApplication::Stop()` requests owner-thread termination. Main-window `Hide` and `Close` intentionally remain no-ops; copying a GUI Exit handler would keep the loop alive. Workflow accesses the stop interface through the reflected `GuiApplication::GetTuiApplication` static function. Ordinary non-TUI startup leaves the accessor null.

## Services and the owner-thread pump

The shared adapter owns `SharedAsyncService` and `SharedCallbackService`. Its 16 ms TUI timer executes queued and delayed async work, then invokes the global timer used by GacUI rendering and caret blinking. Windows message dispatch is nonblocking and integrated into this pump. The ordinary Windows async service cannot be reused unchanged because its usual native window loop is not running.

Windows still owns its service window (GodWindow), clipboard notifications, image decoding, cursors, key names/states, global shortcut registration and executable path. The adapter forwards clipboard/global-shortcut notifications to its own callback service. That service window is not another GacUI native window. Never pass a TuiWindow to code which casts an INativeWindow to WindowsForm.

The resource adapter exposes only `TuiFont`, size 1. Font style bits are retained; family, size, DPI and antialiasing do not change cell metrics. ImageService remains available for resource decoding and the showcase's textual image objects. It does not imply that ImageFrame or document image runs have a renderer. AutomationService returns null.

Future Wayland/Cocoa adapters should reuse the shared controller, window, graphics and paragraph classes. Supply the platform native-service controller, implement `PumpPlatformEvents` and `ApplyTitle`, and keep any required native service windows/handles in that platform's adapter. Recheck the platform input/clipboard/global-shortcut service assumptions individually.

## Physical geometry and hosted windows

`TuiWindow` implements the single physical INativeWindow. Bounds have origin (0,0), conversions are identities, and one native coordinate is one terminal cell. Stored client dimensions can differ from the physical viewport: programmatic setters update the stored dimensions without resizing the console. A terminal resize replaces them and sends Moved/redraw notifications. One primary screen reports the actual buffer dimensions at scaling 1.

The physical window rejects custom frames and reports all GacUI frame options disabled. It is always activated; all Show variants enable painting, while Hide/Close do nothing. Select `ControlThemeName="SystemFrameWindow"` for the main window so TuiSkin draws only the application background and its template agrees with the physical frame contract. The theme still prefers custom frames for hosted children. They retain the existing virtual window manager, activation, capture, focus, modal/popup behavior and supported child-frame options. Size-box children use double lines; border-only children use thick lines.

`Console::SetTitle` cannot run during takeover because ordinary Console output is disabled. The Windows adapter applies stored titles with `SetConsoleTitleW`. Do not re-enable ordinary console output to set the title. Future adapters own their title implementation.

Input uses the existing VlppOS `vl::presentation` key, character and mouse types. KeyDown/KeyUp and native wchar_t Char units are forwarded separately. Text is not synthesized from KeyDown. Mouse positions are cells, wheel signs and modifiers are retained, and Alt is independent of OS Super. Future POSIX adapters may deliver fewer key-up/modifier/button combinations.

## Rendering

`TuiGraphicsRenderTarget` clears once at StartHostedRendering and presents once at StopHostedRendering. Per-window StartRendering/StopRendering calls share that buffer; clearing or presenting separately for each child would erase previous windows. Draw operations reacquire the buffer and intersect both the composition clip and current physical viewport, including after nested resize callbacks.

Registered factories are TuiBorder, SolidBorder, SolidBackground, SolidLabel and the existing renderer-independent GuiDocumentElementRenderer. ImageFrame, Polygon, 3D elements, gradients, inner shadows and FocusRectangle intentionally have no factory. Unsupported content should fail visibly instead of silently disappearing.

TUI XML instances declare `xmlns:elements="presentation::elements::*Element"` and use `<elements:TuiBorder/>`. Keep this import in the TUI resources: changing the compiler's default imports would also change existing GUI compiler snapshots.

`TuiBorderElement` is reflected with type name TuiBorder. Thin, thick and double styles use VlppOS line-merging helpers. Exclusive GacUI edges are converted to inclusive terminal endpoints once. A dimension of one creates a line; 1 by 1 and empty rectangles paint nothing. Thin rounded/ellipse geometry uses rounded corners where large enough; other cases use sharp corners. Helpers draw into a scratch copy, then only the original clipped perimeter is applied so clipping does not invent corners. Wide-cell repair remains delegated to VlppOS.

Paint alpha is composed against destination RGB. Transparent paint preserves existing cells. Text explicitly supplies the existing destination background when it has no background override; TuiPrintOptions requires a background value. Opaque backgrounds clear covered cells, including repairing width-two pairs. TuiSkin supplies opaque normal surfaces.

## Text and inline objects

`TuiTextLayout` shares scalar decoding and layout between SolidLabel and paragraphs. It decodes UTF-16 pairs on Windows, preserves native-string offsets, treats CRLF as one break and tabs as four-column stops, and gets scalar widths from TUI::MeasureChar. Text rows have height one. Wide scalars are never emitted as isolated surrogate units or clipped halves. Unsupported zero-width/nonprintable scalars occupy no standalone terminal cell.

Paragraphs implement wrapping/alignment, native-offset caret navigation/hit testing, style and foreground/background spans, inline-object ranges and caret display. Inline ranges are atomic for caret navigation. Their baseline determines line ascent/descent; stick-to-previous/next conditions keep the adjacent scalar with the object when the group fits a row. Callback-reported inline sizes invalidate layout, and render iteration uses a copy of the cells so callbacks can trigger layout/resize without invalidating an iterator. Embedded compositions remain ordinary GuiDocumentItems with their real input behavior. Labels use scalar-aware per-line ellipsis and retain natural text metrics for minimum-size calculation.

SetInlineObject must accept property reapplication to the exact same range with matching callback/image identity; document style and layout refreshes depend on this. Intersecting ranges still fail. OnRenderInlineObject receives coordinates relative to the paragraph, while drawing uses the supplied render origin. The document renderer adds paragraph offsets itself.

Document image runs are unsupported. The showcase instead decodes each selected image, retains its ImageData in a named document item, and inserts a DocumentEmbeddedObjectRun with a label such as `[Image: path, width x height]`. The normal EditRun operation supplies selection, deletion and undo semantics. Clipboard/document serialization does not turn this sample into an image-file persistence implementation.

TuiControlTest bundles no image resources. ListViewData.xml stores the original example names, categories, dimensions and filenames as text, while list/tree/grid models omit resource image lookups and bindings. The editor's explicit file-selection operation above decodes only the selected external file.

## Skin, controls and generated outputs

Authored TuiSkin XML lives in `Test/Resources/App/TuiSkin`. Its ThemeTemplates instance registers every retained control family and deliberately omits ribbon/toolbar controls. Toolstrip splitter names still map to a TUI separator because retained menu groups request those names before attachment. Install `tuiskin::CreateDefaultColorPackage()` with `tuiskin::SetColorPackage` before creating `tuiskin::TuiTheme` and controls. Palette fields are defined in Workflow's ColorPackage and accessed inside the skin through the tuiColors global. Other resources use the exported `tuiskin::TuiTheme::GetColorPackage` method. The two public C++ configuration helpers live in Generated_TuiSkin/TuiSkinConfig.h/.cpp.

Standard text/tree/detail-list item templates and default DataGrid visualizers branch on GetTuiApplication. TUI branches use cell spacing, omit image elements and use a text focus indicator. `TuiListItemBackgroundTemplate` publishes normal/disabled/selected text and selected-background colors. Helpers in `Source/Controls/ListControlPackage/TuiItemTemplates.cpp` propagate them to realized items and grid cells, including recycled cells. The existing GUI branches retain their metrics and behavior. Standard ListView view requests normalize to Detail, while DataGrid's separate editable view remains available.

Cell layouts must propagate content minimum sizes through checkbox/radio stacks and scroll-container tables. Do not split a one-cell text row between two percentage rows: rounding each half can double its height. Default grid rows reserve three cells for bordered in-place editors, and grid separator bounds have a one-cell minimum. Document controls use zero internal padding and a one-cell caret scroll margin in TUI mode; GUI pixel metrics remain unchanged.

The showcase binds document-editor shortcuts to document focus so hidden editors do not intercept another page or dialog's clipboard keys. An empty GuiToolstripCommand ShortcutBuilder unregisters its chord and clears the stored builder; reattaching a cleared command must not recreate the old shortcut.

TUI fake dialogs use `Source/Utilities/FakeServices/TuiDialogs`, six factory callbacks and their own localized-string injection. Utilities retain one FakeDialogServiceBase pointer so finalization removes the same selected instance. The fake service keeps the existing file validation, async enumeration and commit/cancel behavior.

GacUI_Compiler builds TuiSkin, TUI dialogs and TuiControlTest for both architectures, then merges dialog output into Source. Keep the original six GUI resources first in their existing order: interned resource-name ordering affects generated member/initialization order and can change recorded GUI frames. TUI instances declare their own elements namespace instead of extending the compiler's default namespaces. The shared .vcxitems inventories explicitly list controls and reflection separately. Metadata_Generate and GacUI_Host register the new dialog types; metadata also includes TuiSkin. Run the documented Debug Win32/x64 metadata sequence after generation.

Merged C++ is UTF-8 with a BOM. MBCS output uses the generator thread's code page, which can differ from the system code page used by MSVC and corrupt literal sample text. The unchanged-output check verifies both content and encoding. Keep showcase state in observable Workflow properties when a readout must track it; graphics-element properties do not necessarily expose change events. The normal TUI label template preserves explicit line breaks for file-validation messages.

The sibling Tools/Tools/ProjectGacUI.ps1 release workflow mirrors the authored skin XML/configuration to Source/Skins/TuiSkin and invokes GacGen. Release/CodegenConfig.xml routes portable TUI into GacUI, Windows adapters into GacUI.Windows, dialog reflection into GacUIReflection, and the skin into separate TuiSkin/TuiSkinReflection pairs.

MSVC consumers compiling the merged TuiSkin.cpp translation unit need `/bigobj`, as with the other large amalgamated library sources.

## Verification references

`Test/GacUISrc/UnitTest/TestTuiProvider.cpp` uses an injected backend for deterministic cell, geometry, input, async/timer and text-offset assertions. It does not create a real terminal or change existing snapshot baselines. The release build runs this with the ordinary suite in Win32 and x64.

Use [DebugTuiControlTestSop.md](../Jobs/DebugTuiControlTestSop.md) for actual Windows Terminal behavior and the page/function verification record. Use [GacUILayout.md](../../GacUILayout.md) for authoring rules and [the VlppOS TUI reference](KB_VlppOS_TerminalUserInterface.md) for backend contracts.
