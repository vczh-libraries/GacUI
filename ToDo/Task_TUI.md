# GacUI on TUI

## Authority and plan composition

The GacUI TUI plan has two parts:

1. Implement the cross-platform terminal, buffer, drawing, and input foundation in [`../../VlppOS/TODO_Task.md`](../../VlppOS/TODO_Task.md), using the verified platform details in [`../../VlppOS/Task_TUI.md`](../../VlppOS/Task_TUI.md).
2. Implement the GacUI native-controller and renderer adapter described in this file.

The two VlppOS files are therefore Phase 1 of this GacUI plan, not an unrelated task. Read all three files before execution. If this file conflicts with `VlppOS/TODO_Task.md`, the VlppOS file has priority.

Do not reimplement a platform abstraction, terminal buffer, box-drawing lookup, color quantizer, character-width table, input parser, or resize loop in GacUI. Those are owned by `vl::console::TUI`.

## Goal

GacUI rendered in a terminal should:

- Run in Windows console hosts launched from cmd.exe or PowerShell, and in supported xterm-compatible terminals on Linux and macOS.
- Use Hosted Mode so one real terminal-backed native window contains all supported GacUI windows, popups, menus, and dialogs. Supply a compact cell-scale, image-free TUI theme and matching dialog resources; the pixel-scaled DarkSkin and current image-creating fake-dialog resources cannot be reused unchanged.
- Follow terminal resizing and reflow the UI to the new cell dimensions.
- Render foreground and background colors. A GacUI color with `a == 0` is skipped; any positive alpha is treated as opaque. `vl::console::TUI` selects truecolor, 256-color, or 16-color output and performs fallback quantization.
- Render with Unicode box-drawing and block characters, not ASCII art. Use ASCII-only C++ source spelling such as `U'\u2500'` where a literal code point is needed.
- Draw into the `TuiPixel` buffer supplied by VlppOS and submit it through `TUI::RenderBuffer`.
- Follow the VlppOS merge rule: merge representable `TuiMergeablePixel` geometry regardless of color, let the later foreground color win, and replace with the later cell when the combined arm state has no Unicode glyph.
- Treat one terminal cell as one GacUI coordinate unit. The actual terminal font is controlled by the terminal; “1” is a logical cell size, not a physical font size.
- Measure and print independent Unicode scalar values through TUI. Combining sequences, variation sequences, emoji ZWJ sequences, bidirectional layout, and grapheme shaping are deferred because Phase 1 stores one `char32_t` per leading cell.
- Route character and mouse input to GacUI without terminal echo. Keyboard events remain placeholders until a future VlppOS keyboard-translation phase.
- Keep all TUI, native-window, rendering, input, resize, and global-timer callbacks on the thread that calls `TUI::Start`, normally the thread running `main`.
- Leave `GuiImageFrameElement` unsupported. Because the renderer factory is intentionally absent, creating it fails through GacUI's existing `CHECK_ERROR` for an unsupported renderer; this is not described as an uncontrolled crash.

## Plan

- Complete Phase 1 in VlppOS and update the generated release/import boundary.
- Add a TUI-backed `INativeController` and the ten current native services.
- Bridge `ITuiCallback` to GacUI's native-window, timer, character, and mouse callbacks.
- Add a TUI graphics render target and resource manager without creating another character buffer.
- Add the supported GacUI element renderers and document every coarse-cell approximation.
- Add a compact, cell-scale, image-free TUI theme and TUI-safe fake-dialog resources.
- Add `SetupTuiRenderer()` through the existing Hosted Mode initialization pattern.
- Add unit and integration coverage for lifecycle, rendering, input, resize, and color fallback.
- Run `ToDo/CheckLinks.ps1 Task_TUI.md` and all builds/tests required by the files changed during implementation.

## Phase 1: VlppOS foundation

Phase 1 is exactly the work in:

- [`VlppOS/TODO_Task.md`](../../VlppOS/TODO_Task.md): public API, ownership, single-threaded lifecycle, buffer model, drawing semantics, tests, and playground.
- [`VlppOS/Task_TUI.md`](../../VlppOS/Task_TUI.md): Windows, Linux, and macOS terminal implementation details.

Important consequences for GacUI:

- TUI belongs to VlppOS, not Vlpp or GacUI.
- `TUI::Start` calls `Console::Disable()` and cleanup calls `Console::Enable()`. Disabled Console `Read`, `Write`, and `SetColor` calls fail through `CHECK_ERROR`; they are not redirected to GacUI callbacks.
- GacUI receives `TuiPixel*` from `TUI::GetBuffer()` and uses TUI drawing helpers.
- `TUI::Start` is the real native event loop and blocks until a same-thread callback requests `TUI::Stop`.
- `TUI::RunOneCycle` supplies the reentrant owner-thread pump required by GacUI modal dialogs.
- `TUI::IsStopRequested` lets the bridge suppress later higher-level callbacks that it controls within the current TUI callback.
- `TUI::TryGetConsoleSize` is available before `Start`.
- `TUI::Timer` supplies a periodic owner-thread callback even when no input arrives.
- `TUI::Char` supplies a decoded `TuiCharInfo`.
- Keyboard event structures exist only as placeholders in this phase.

## TUI native controller

Keep the native/graphics provider in the Core layer under `Source/PlatformProviders/TUI`. Suggested Core files:

- `GuiTuiController.h` / `GuiTuiController.cpp`
- `GuiTuiWindow.h` / `GuiTuiWindow.cpp`
- `GuiTuiGraphics.h` / `GuiTuiGraphics.cpp`
- `GuiTuiGraphicsRenderers.h` / `GuiTuiGraphicsRenderers.cpp`
- `GuiTuiGraphicsLayoutProvider.h` / `GuiTuiGraphicsLayoutProvider.cpp`

Do not introduce an upward dependency from `Source_GacUI_Core` to controls or `FakeDialogServiceBase`. Put `GuiTuiDialogService` and its generated image-free dialog resources in the controls-capable utilities layer beside the existing fake-dialog implementation. Put the TUI `ThemeTemplates` and generated theme resources in a controls-capable full-GacUI/generated-resource layer. Compile the higher-level `GuiTuiControllerSetup.cpp` where it can instantiate both the Core provider and `GuiTuiDialogService`.

Terminal acquisition, input, output, color, resize, and restoration remain entirely in VlppOS. The only GacUI-side OS branch is the executable-path query required by `INativeController`; keep it in a small guarded helper or matching `.Windows.cpp`, `.Linux.cpp`, and `.macOS.cpp` files.

### Controller services

The current `INativeController` in `Source/NativeWindow/GuiNativeWindow.h` exposes ten services, plus `GetExecutablePath`. Implement them as follows:

- `INativeCallbackService`: reuse `SharedCallbackService`.
- `INativeResourceService`: provide one logical monospace font with cell size 1. `EnumerateFonts` returns that family and `SetDefaultFont` accepts/coerces requests to it. Cache a non-null no-op `INativeCursor` for every `INativeCursor::SystemCursorType` from zero through `INativeCursor::SystemCursorCount - 1`, and return the arrow cursor as default; the objects do not change the terminal cursor.
- `INativeAsyncService`: reuse `SharedAsyncService`, and construct the controller/service on the same thread that later calls `TUI::Start` so its recorded main-thread id is correct. Its queued main-thread work is drained from the TUI timer callback. `InvokeAsync` and non-main-thread delayed work retain their existing thread-pool semantics when requested by application/framework code; they are not used to implement TUI input or rendering.
- `INativeClipboardService`: return null and let `GuiInitializeUtilities` install `FakeClipboardService`.
- `INativeImageService`: provide a non-null stub because image service is not substitutable. All three `CreateImageFromFile`, `CreateImageFromMemory`, and `CreateImageFromStream` operations return null; it exposes no loadable frames for this renderer.
- `INativeScreenService`: expose one named primary screen whose bounds and client bounds equal the terminal's current cell rectangle and whose scaling is 1.0. Return it for the terminal window and index zero; return null for every other index or window.
- `INativeWindowService`: expose one real terminal-backed native window. Hosted Mode creates all other windows virtually.
- `INativeInputService`: implement the global timer bridge described below. `IsKeyPressing` and `IsKeyToggled` return false until key translation exists; `GetKeyName` returns empty; `GetKey` returns `(VKEY)-1`; global-shortcut registration returns `NativeGlobalShortcutKeyResult::NotSupported`; unregistration returns false.
- `INativeDialogService`: the Core `GuiTuiController` returns null, as does the current `GuiHostedController` wrapper. The higher-level setup installs a TUI dialog service derived from `FakeDialogServiceBase` as a non-optional service substitution. Its generated cell-scale resources never construct `GuiImageFrameElement`. The existing `FakeDialogService` is not suitable because its current resources create image elements.
- `INativeAutomationService`: return null and let `GuiInitializeUtilities` install `INativeAutomationService::UnavailableService()`.
- `GetExecutablePath`: there is no existing cross-platform provider helper. Use [`GetModuleFileNameW`](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew) on Windows, `readlink("/proc/self/exe", ...)` as described by [`proc_pid_exe(5)`](https://man7.org/linux/man-pages/man5/proc_pid_exe.5.html) on Linux, and [`_NSGetExecutablePath`](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html) on macOS. Handle growable buffers, missing termination, and UTF-8/UTF-16 conversion explicitly.

Clipboard, Dialog, and Automation are the substitutable service categories listed by `GUI_SUBSTITUTABLE_SERVICES`. The setup-installed non-optional Dialog substitution wins over the later optional `FakeDialogService` installed by `GuiInitializeUtilities`; it must be uninstalled during setup cleanup. Callback, Resource, Async, Image, Screen, Window, and Input must be non-null on the Core provider.

### Size before `TUI::Start`

`GuiHostedController::Run` calls `SettingHostedWindowsBeforeRunning` before it enters the underlying `INativeWindowService::Run`. Screen/window bounds must therefore be valid before `TUI::Start`.

Use `TUI::TryGetConsoleSize` when the screen or terminal window is created. If the size cannot be queried, fail setup cleanly rather than inventing permanent bounds. Once TUI starts, its initial and subsequent `BufferSizeChanged` callbacks update the window bounds and notify listeners.

The one terminal native window has a fixed origin `(0, 0)`, scaling `1.0`, and a client size owned by the terminal. `SetBounds` and `SetClientSize` must not replace the queried cell size with an application's pixel-oriented requested size during `SettingHostedWindowsBeforeRunning`; retain the terminal size and let the initial `Moved` notification resize the Hosted Mode main window. Size changes come only from `BufferSizeChanged`.

### Window-service run loop

The underlying TUI window service owns exactly one terminal window:

1. Install the `ITuiCallback` bridge before running.
2. Call `window->Show()` to record the visible/pending-open state expected by Hosted Mode, but defer the listener's `Opened` notification until `Starting`.
3. Call the blocking `TUI::Start` from `INativeWindowService::Run`.
4. Implement `INativeWindowService::RunOneCycle` by delegating to reentrant `TUI::RunOneCycle`; this is required by `FakeDialogServiceBase` modal loops and stays on the owner thread.
5. Let `INativeWindow::Hide(true)` synchronously invoke `BeforeClosing`. If it directly requested TUI stop, return and let the forced-close path finish regardless of the cancel flag; otherwise honor cancellation and remain open, or invoke `AfterClosing` and request `TUI::Stop`.
6. After `TUI::Start` returns, emit `Closed` once and uninstall the bridge. Later `DestroyNativeWindow` emits `Destroying`, `INativeControllerListener::NativeWindowDestroying`, and `Destroyed` in that order.

Do not call `TUI::Start` from `SetupTuiRenderer`, controller initialization, or a background thread. It replaces the normal platform message loop specifically inside `INativeWindowService::Run`.

`Stopping` is too late for a cancelable close. It only marks the bridge as shutting down and must not repeat `BeforeClosing` or `AfterClosing`. A direct Phase-1 `TUI::Stop` that was not initiated by `Hide(true)` is a forced terminal shutdown: skip the cancelable pair and still deliver `Closed` after `Start` returns.

### Callback bridge

Implement one `ITuiCallback` owned by the native controller/window service:

- `Starting`: synchronize the initial size internally, start one 16 ms TUI timer for the whole native run, and emit `Moved` if needed. If `TUI::IsStopRequested()` is still false, complete the pending show by emitting `Opened` exactly once. Starting the timer before any native callback lets a `Moved` or `Opened` handler enter a modal `RunOneCycle` loop safely.
- If `TUI::IsStopRequested()` is still false after `Opened`, mark the sole terminal window activated and emit `GotFocus` exactly once. Treat it as permanently focused while active; terminal focus-in/focus-out tracking is outside this phase.
- `Stopping`: prevent new rendering/input work, emit `LostFocus` once if the window is active, and stop the TUI timer. Do not emit the cancelable close pair here.
- `BufferSizeChanged`: update screen/window bounds, invalidate any render-target frame binding, and invoke `Moved` only when the bounds actually changed. If `TUI::IsStopRequested()` is still false, request the paint/full Hosted Mode refresh even when the dimensions were unchanged. The initial callback must not duplicate a `Moved` already emitted by `Starting` for the same dimensions.
- Mouse callbacks: copy zero-based cell coordinates into `NativeCoordinate`, set `nonClient = false`, copy known modifier/button state, normalize wheel deltas to 120 units per detent without collapsing multi-detent values, and invoke the matching `INativeWindowListener` method.
- `Char(const TuiCharInfo&)`: convert `code` to native `wchar_t` units and copy its best-effort `ctrl`, `shift`, `alt`, and `capslock` fields. On Windows, a supplementary scalar becomes up to two `NativeWindowCharInfo` callbacks because GacUI's current character event stores one UTF-16 code unit; after the high-surrogate callback, check `TUI::IsStopRequested()` and suppress the low-surrogate callback if shutdown was requested. On Linux/macOS it normally becomes one callback. Phase 1 sets unobservable POSIX modifiers deterministically false.
- Keyboard callbacks: leave unraised in this task. Key-state queries return the placeholder values listed above.
- `Timer`: drain `SharedAsyncService::ExecuteAsyncTasks`, then check `TUI::IsStopRequested()` before allowing Hosted Mode to perform requested refreshes, and check again before invoking `SharedCallbackService::InvokeGlobalTimer` while the input service's global-timer flag is enabled.

`INativeInputService::StartTimer` and `StopTimer` only set the global-timer flag; `IsTimerEnabled` returns it. The underlying 16 ms TUI timer stays active for the entire `Run`, even when that flag is false, because `SharedAsyncService` has no separate event-loop wake primitive and delayed/main-thread work must continue to make progress.

Every callback above, every native-window/render callback, and every main-thread completion drained by `ExecuteAsyncTasks` runs on the `TUI::Start` thread. No GacUI-side input, rendering, or timer thread is added. `SharedAsyncService::InvokeAsync` remains an explicit background-work API: application code and the existing `FakeDialogServiceBase` file enumeration can use the existing thread pool, then marshal UI completion back through `InvokeInMainThread`. That unrelated framework facility does not move any TUI callback or platform event-loop work off the owner thread.

When one TUI-triggered native event fans out to multiple `INativeWindowListener` calls, check `TUI::IsStopRequested()` after each call and suppress the remaining calls for that event. `Stopping` is the deliberate exception: it may emit the one required `LostFocus` during shutdown.

## Graphics resource manager and render target

### Resource manager

Derive `GuiTuiGraphicsResourceManager` from `GuiGraphicsResourceManager` and implement every remaining `IGuiGraphicsResourceManager` operation:

- `GetRenderTarget`
- `RecreateRenderTarget`
- `ResizeRenderTarget`
- `GetLayoutProvider`
- `CreateRawElement`

There is only one native render target because Hosted Mode renders all virtual windows into the one terminal. `ResizeRenderTarget` invalidates any per-frame binding and requests a full redraw; it must not retain the old `TUI::GetBuffer()` pointer. `RecreateRenderTarget` rebuilds only GacUI-side state; TUI owns terminal recovery.

`GetLayoutProvider` returns the TUI paragraph/layout provider required below so text editors and rich documents are usable. `CreateRawElement` still needs a valid TUI raw-element implementation or an explicit supported no-op element; it cannot be omitted from the class.

### Render target lifecycle

Derive `GuiTuiGraphicsRenderTarget` from the existing `GuiGraphicsRenderTarget` rather than reimplementing `IGuiGraphicsRenderTarget`. The base class already:

- Ensures `StartRenderingOnNativeWindow` and `StopRenderingOnNativeWindow` wrap a complete Hosted Mode frame.
- Allows multiple per-window `StartRendering`/`StopRendering` pairs inside that frame.
- Maintains hosted-rendering state and the clipper stack.
- Implements `HostedRenderingIdle` and `IsClipperCoverWholeTarget`.

Override the protected backend hooks:

- `StartRenderingOnNativeWindow`: reacquire `TUI::GetBuffer()` and the current dimensions, bind them only for this frame, and clear the buffer once for the entire hosted frame.
- `StopRenderingOnNativeWindow`: validate and submit the final frame through `TUI::RenderBuffer` once.
- `GetCanvasSize`: return current TUI width/height.
- The four clipper-transition hooks: update any cached valid drawing rectangle required by the TUI renderers.

Never clear in each `StartRendering` or flush in each `StopRendering`; doing so erases or submits partial virtual-window frames. `BufferSizeChanged` occurs on the same thread between callbacks, invalidates the frame binding through the resource manager, and requests refresh. A renderer must not cache the raw `TuiPixel*`; every write goes through a render-target accessor that verifies the bound pointer/dimensions/generation before dereferencing it. If a nested cycle resizes during rendering, mark the frame invalid, skip all later writes from that frame, and return `RenderTargetFailure::ResizeWhileRendering` so Hosted Mode retries with the new target size.

GacUI rectangles use an exclusive `x2/y2` boundary, while the Phase 1 drawing helpers use inclusive endpoints. Convert a nonempty GacUI rectangle to `[x1, y1, x2 - 1, y2 - 1]` after applying the current clipper.

## Element renderers

Each supported renderer follows the existing `GuiElementRendererBase<TElement, TRenderer, TRenderTarget>` pattern and writes only through `GuiTuiGraphicsRenderTarget`/TUI helpers.

### Focus rectangle

Approximate the focus rectangle with alternating thin one-cell segments or explicit box-drawing characters. Phase 1 has no dashed merge style, so do not pretend dashed Unicode arms participate in its merge table.

### Solid border

- `Rectangle`: draw a thin TUI rectangle when both dimensions exceed one; use a horizontal line, vertical line, or deterministic one-cell point glyph for 1xN, Nx1, or 1x1 bounds.
- `RoundRect`: map positive `ElementShape::radiusX/radiusY` to a thin one-cell rounded-corner rectangle; zero/degenerate radii fall back to `Rectangle`. For bounds too small to contain four distinct corners, fall back deterministically to the rectangle/line/point representation that fits.
- `Ellipse`: approximate the outline on the cell grid with an ellipse midpoint/implicit-equation mask, falling back to a rounded rectangle only for dimensions too small to distinguish.

Alpha zero skips the operation. The border foreground uses the element RGB value.

### Solid and gradient backgrounds

Fill selected cells with spaces whose background is the requested color:

- Rectangle uses the clipped bounds.
- RoundRect and Ellipse use a cell-center mask so corners outside the shape are not filled.
- Gradient computes the element's interpolation at each selected cell and writes that RGB as the background. TUI performs terminal palette fallback.

Do not use shade characters merely to simulate a gradient when a per-cell background color already represents it.

### Solid label

Decode `WString` into Unicode scalar values with the existing Vlpp conversion utilities. Use `TUI::MeasureChar` for minimum size, alignment, trimming, and wrapping, and `TUI::PrintChar` for output.

Respect width-two continuation cells. Do not split a scalar at the right clip edge. Scalar width zero, combining sequences, and grapheme shaping follow the Phase 1 limitations rather than receiving a second inconsistent implementation in GacUI.

Define every coarse-cell property:

- Normalize CRLF/CR to LF. When `multiline` is false, treat line breaks as spaces; otherwise they create one-cell-high logical lines.
- `wrapLine` wraps at scalar boundaries without splitting a width-two scalar. `wrapLineHeightCalculation` includes the resulting line count in minimum height only when enabled.
- Horizontal and vertical alignment operate on measured cell extents. When `ellipse` is enabled and clipping is required, reserve the last fitting cell for U+2026 (or a documented ASCII fallback if it is unavailable).
- Font family, pixel size, bold, italic, underline, strikeline, antialias, and vertical-antialias properties cannot be represented by `TuiPixel`; coerce them to the one logical terminal font instead of pretending they affect metrics.
- One rendered line has height one. Minimum-size precedence is explicit: `ellipse = true` forces minimum width to zero so clipping can occur; otherwise a non-wrapping label uses the maximum measured explicit-line width. A wrapping label reports width zero and includes wrapped-content height only when `wrapLineHeightCalculation` is true; when it is false, wrapped content contributes no minimum height. Preserve a one-cell font height for an empty non-wrapping label and while height-calculating wrap has no usable width yet.

### 3D border and splitter

`Gui3DBorderElement` is not a Unicode double-line rectangle. Match the existing visual meaning:

- Draw top/left edges with `color1`.
- Draw bottom/right edges with `color2`.
- Use adjacent thin lines where the two-cell thickness is visible.

Draw `Gui3DSplitterElement` as two adjacent horizontal or vertical thin lines using `color1` and `color2`.

### Inner shadow

Approximate the inner shadow with one or more clipped cell bands along the required edges. Use the shadow RGB as the affected foreground/background according to the selected block character. Document that sub-cell blur and alpha blending are unavailable.

### Polygon

Use `GuiPolygonElement::GetSize()` as the renderer minimum size. Keep its local point set unscaled and center it in `bounds` with offsets `(bounds.Width() - element->GetSize().x) / 2` and `(bounds.Height() - element->GetSize().y) / 2`, matching the existing renderers. Rasterize the interior with a scanline or cell-center inclusion test and rasterize every edge—including diagonals—with a deterministic cell-line algorithm; use mergeable TUI lines where an axis-aligned edge is exact. Apply `element->GetBackgroundColor().a` and `element->GetBorderColor().a` independently, using each positive-alpha RGB value for its own fill/outline. Do not replace every polygon with its bounding rectangle.

### Rich document and image

`GuiDocumentElementRenderer` does not delegate to solid-label and solid-border renderers. It requires a real `IGuiGraphicsLayoutProvider` and `IGuiGraphicsParagraph` implementation for runs, caret positions, wrapping, inline objects, and rendering.

Implement `GuiTuiGraphicsLayoutProvider::CreateParagraph` and a `GuiTuiGraphicsParagraph` that covers every `IGuiGraphicsParagraph` operation:

- Return the creating provider and associated render target from `GetProvider` and `GetRenderTarget`.
- Decode `WString` to scalars while retaining a map to native `wchar_t` offsets. Every offset from zero through `WString::Length()` is a valid text position, including the interior UTF-16 offset of a Windows surrogate pair. Valid carets are scalar boundaries only; `GetNearestCaretFromTextPos(textPos, frontSide)` maps an interior text position to the requested boundary and never leaves a caret inside a surrogate pair.
- Use `TUI::MeasureChar`, one-cell line height, scalar-boundary wrapping, max width, and paragraph alignment to build deterministic line/caret tables.
- Validate every `(start, length)` range and implement font, size, style, foreground, background, `SetInlineObject`, and `ResetInlineObject` mutations. Accept font family/size/style runs for API compatibility but use the same fixed terminal metrics. Apply foreground/background run colors; document that bold, italic, underline, and strikeline have no cell-style representation.
- Implement `IsValidTextPos`, `IsValidCaret`, `GetNearestCaretFromTextPos`, first/last, line-first/last, left/right/up/down caret movement, point hit testing, `GetInlineObjectFromPoint`, logical zero-width caret bounds, enable/disable/blink, and rendering from the same line table.
- Render the coarse-cell caret by inverting the nearest character cell on the requested front/back side while retaining logical zero-width caret bounds; invert both cells of a width-two scalar and select the only valid side at a line edge.
- Reserve rectangular cell ranges for inline objects; honor their `baseline`, break condition, background color, and optional background element. Invoke `IGuiGraphicsParagraphCallback::OnRenderInlineObject` for valid callback ids and store/use the returned `Size` for subsequent layout. An inline background element is rendered only when its element renderer is supported.
- Rebuild cached layout when text, runs, wrapping, max width, inline objects, or target dimensions change.

Only after this provider passes paragraph/caret tests, register the existing generic `GuiDocumentElementRenderer`. This is required for GacUI text editors; it is not implemented by reusing the solid-label renderer.

Do not register `GuiImageFrameElement`. Creating one directly, or from an incompatible application/theme resource, fails through the existing unsupported-renderer `CHECK_ERROR`.

## Cell-scale theme and dialogs

The current DarkSkin is not a usable TUI default: its pixel-sized fonts, title-bar buttons, margins, and other preferred sizes become huge when one GacUI unit is one terminal cell, and several templates construct `GuiImageFrameElement` even when an icon value is null.

Create and register a compact TUI theme whose control templates:

- Express fonts, padding, borders, title bars, scroll bars, menu rows, and buttons in cell-scale values.
- Use supported labels, borders, backgrounds, polygons, and document elements only.
- Represent optional icons, menu checks, submenu arrows, and window buttons with text/box-drawing glyphs; never instantiate `GuiImageFrameElement`.
- Keep focus, hover, pressed, disabled, popup, and modal-owner states visibly distinct using the selected terminal palette.
- Set `ThemeTemplates::PreferCustomFrameWindow` to true and provide a complete compact `CustomFrameWindow` template. The TUI window service's main `NativeWindowFrameConfig` forces `CustomFrameEnabled = BoolOption::AlwaysTrue` (and disables unsupported maximize/minimize boxes), while the terminal native window reports zero custom-frame padding.

Create matching generated resources for the TUI dialog service's message box, color, font, open-file, and save-file windows. They may coarsen previews and swatches to cells, but must not reuse the existing image-creating fake-dialog resources. Their modal wait delegates through Hosted Mode to `TUI::RunOneCycle`, so input, painting, timers, and async main-thread work continue on the `Start` thread.

`theme::InitializeTheme()` runs inside `GuiApplicationMain` before `GuiMain`, so setup cannot register the theme earlier. Expose `bool RegisterTuiTheme()` and return the result of registering the generated TUI `ThemeTemplates`. A TUI application's `GuiMain` must call it successfully to register the complete TUI theme as its last/highest-priority theme, before constructing any control, window, or dialog, and must not register an incompatible pixel/image theme such as the current DarkSkin.

## Setup entry point

Follow the verified structure of `Source/PlatformProviders/Remote/GuiRemoteControllerSetup.cpp`:

```C++
extern void GuiApplicationMain();

int SetupTuiRenderer()
{
	GuiTuiController tuiController;
	GuiHostedController hostedController(&tuiController);
	GuiTuiDialogService tuiDialogService;

	GuiTuiGraphicsResourceManager tuiResourceManager(&tuiController, &hostedController);
	GuiHostedGraphicsResourceManager hostedResourceManager(&hostedController, &tuiResourceManager);

	SetNativeController(&hostedController);
	auto serviceSubstitution = GetNativeServiceSubstitution();
	serviceSubstitution->Substitute(&tuiDialogService, false);
	SetGuiGraphicsResourceManager(&hostedResourceManager);
	SetHostedApplication(hostedController.GetHostedApplication());

	tuiController.Initialize();
	tuiResourceManager.Initialize();
	hostedController.Initialize();
	GuiApplicationMain();
	hostedController.Finalize();
	tuiResourceManager.Finalize();
	tuiController.Finalize();

	SetHostedApplication(nullptr);
	SetGuiGraphicsResourceManager(nullptr);
	serviceSubstitution->Unsubstitute(&tuiDialogService);
	SetNativeController(nullptr);
	return 0;
}
```

`GuiApplicationMain` eventually enters the underlying TUI window service's `Run`, which is where `TUI::Start` blocks. `GuiHostedController::DialogService()` currently returns null, so the explicit non-optional substitution above is required; `GuiInitializeUtilities`' later optional fake-dialog substitution cannot replace it. Use scope-based cleanup around setup so the dialog substitution and controller/resource/global state are reset in reverse order if initialization or user code throws.

## Project and release integration

- Enumerate every Core TUI provider/header explicitly in `Test/GacUISrc/Source_GacUI_Core/Source_GacUI_Core.vcxitems`.
- Enumerate the controls-dependent TUI dialog/setup files in the appropriate utilities-controls item group, and enumerate theme/generated-resource files in their controls-capable project/item groups. Do not put these dependencies in `Source_GacUI_Core` and do not use wildcards.
- Declare the public `SetupTuiRenderer()` and `RegisterTuiTheme()` entry points in `Source/GacUI.h` beside the existing platform setup declarations.
- Add all tests and resource-generation inputs to their `.vcxproj`, `.vcxitems`, and `.filters` files. Regenerate resource C++ with the established GacGen workflow instead of hand-editing generated output.
- Complete VlppOS Phase 1 and copy its regenerated release sources, excluding `IncludeOnly`, into `GacUI/Import` before compiling Phase 2; never fix imported source in place.
- After implementation and tests, regenerate `GacUI/Release` from `Release/CodegenConfig.xml` and run every build/test required by `Project.md`.

## Verification

### VlppOS phase

Complete all helper, lifecycle, Windows integration, and playground coverage required by `VlppOS/TODO_Task.md`. Verify Windows during the initial delivery; run Linux and macOS runtime verification later on their target environments.

### GacUI unit coverage

Add deterministic tests that do not require a physical terminal:

- Render into an explicit in-memory `TuiPixel` buffer.
- Verify rectangle (including 1xN, Nx1, and 1x1 fallbacks), rounded rectangle, ellipse approximation, fills, gradients, labels, width-two text, 3D edges, splitter, clipping, polygon, and unsupported-element behavior.
- Verify every solid-label property and all paragraph layout, wrapping, range validation, provider/target access, style/color runs, `SetInlineObject`/`ResetInlineObject`, baseline and callback-size updates, `GetInlineObjectFromPoint`, text-position-to-caret conversion, hit testing, and caret operations/visuals.
- Compile representative TUI theme and dialog resources and assert that none constructs `GuiImageFrameElement`.
- Verify GacUI exclusive rectangles are converted correctly to TUI inclusive endpoints.
- Verify one clear and one `RenderBuffer` submission per complete Hosted Mode frame containing multiple virtual windows.
- Verify a resize between frames never reuses the invalidated old TUI buffer pointer.
- Verify resize during rendering produces `ResizeWhileRendering` and a full retry.
- Verify alpha-zero skip and positive-alpha opaque behavior.
- Verify TUI merge geometry and latest-foreground behavior without reintroducing color-dependent merge logic.

### Controller and event coverage

Use the test-only backend hook exported in the codepacked VlppOS Release under `vl::console::unittest` to install a scoped injectable/fake TUI backend and verify:

- Pre-`Start` terminal size populates screen/window bounds.
- Pixel-oriented `SetBounds`/`SetClientSize` requests do not replace the fixed terminal cell bounds, and screen scaling remains 1.0.
- Initial and resized buffer notifications update Hosted Mode bounds.
- Mouse and `TuiCharInfo` events, modifiers, coordinates, `nonClient`, and normalized wheel deltas map correctly.
- Supplementary scalar conversion produces native `wchar_t` event units correctly on each platform.
- A stop request from the first Windows UTF-16 unit suppresses the second unit, and stop requests during other fanned-out native events suppress their remaining listeners/work.
- The always-on TUI timer drains `SharedAsyncService` while the input timer is disabled; `GlobalTimer` fires only while it is enabled.
- Startup emits `Opened` then `GotFocus` once. A canceled `Hide(true)` stops after `BeforeClosing` and remains visible/focused. An accepted close continues with `AfterClosing`, `LostFocus`, `Closed`, `Destroying`, controller `NativeWindowDestroying`, and `Destroyed`, in that order and without duplicates.
- A direct `TUI::Stop` follows the documented forced-close path and returns from `Run`.
- A TUI-safe message box enters nested `INativeWindowService::RunOneCycle`, remains responsive, exits, and leaves every native/UI callback on the `Start` thread.
- Every TUI/native-window/render/timer callback and every main-thread async completion executes on the thread that called `TUI::Start`; explicitly requested `InvokeAsync` work may execute in the existing thread pool.
- Every placeholder `INativeInputService` operation returns its documented value, and keyboard callbacks remain explicitly absent.
- `GetExecutablePath` handles long paths and returns the current executable on Windows, Linux, and macOS.

### Interactive coverage

After unit tests pass, run a small TUI GacUI application on:

- Windows conhost and Windows Terminal, launched from cmd.exe and PowerShell.
- Common xterm-compatible Linux terminals.
- macOS Terminal and iTerm2.

Check the compact TUI theme, resize, mouse hover/buttons/double-click/wheels, character insertion, mouse/programmatic caret placement and blinking, color fallback, Hosted Mode popups/dialogs, clean exit, and restoration of the previous terminal screen. Keyboard-driven navigation, Backspace/Delete, Enter, and menu-key behavior wait for the future key-translation phase. Also verify that supplying an incompatible image-creating theme fails with the documented unsupported-renderer assertion.

Finally run:

```PowerShell
& "<absolute-repo-path>\ToDo\CheckLinks.ps1" Task_TUI.md
```
