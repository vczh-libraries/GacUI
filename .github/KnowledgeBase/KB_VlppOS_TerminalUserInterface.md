# Terminal User Interface

`vl::console::TUI`, declared in [TUI.h](../../Source/TUI/TUI.h), provides a cross-platform terminal takeover, owner-thread event loop, cell buffer, and drawing API. It is intended for applications that redraw the visible terminal as a complete user interface instead of using sequential `vl::console::Console` input and output.

All names on this page are in the `vl::console` namespace unless another namespace is shown.

## Starting and Stopping TUI

Install one or more `ITuiCallback` listeners before calling `TUI::Start`. `TUI::Start` takes control of the terminal, allocates the initial cell buffer, dispatches callbacks, runs the event loop, restores the terminal, and then returns.

```cpp
class Callback : public ITuiCallback
{
public:
	void Starting() override
	{
		TUI::StartTimer(500);
	}

	void BufferSizeChanged() override
	{
		Redraw();
	}

	void Char(const TuiCharInfo& info) override
	{
		if (info.code == L'\x1B')
		{
			TUI::Stop();
		}
	}

	void Timer() override
	{
		Redraw();
	}

	void Stopping() override
	{
		TUI::StopTimer();
	}

	void Redraw()
	{
		TUI::Clear({ 0, 0, 0 }, 0, 0, TUI::GetBufferWidth() - 1, TUI::GetBufferHeight() - 1);
		TUI::RenderBuffer();
	}
};

Callback callback;
CHECK_ERROR(TUI::InstallListener(&callback), L"Failed to install the TUI listener.");
try
{
	TUI::Start({ .colorMode = TuiColorMode::Auto });
}
catch (...)
{
	TUI::UninstallListener(&callback);
	throw;
}
CHECK_ERROR(TUI::UninstallListener(&callback), L"Failed to uninstall the TUI listener.");
```

`ITuiCallback` is non-owning. The listener object must remain alive until it is uninstalled and no active callback can use it.

### Lifecycle Order

An inactive-to-active `TUI::Start` follows this observable order:

1. Validate the requested color mode and require `Console::IsEnabled()` to be true.
2. Take over the terminal, select a color mode, query the initial visible size, disable `Console`, and allocate the buffer.
3. Set `TUI::IsInUse()` to true.
4. Invoke `ITuiCallback::Starting`.
5. Invoke the initial `ITuiCallback::BufferSizeChanged` unless a listener requested `TUI::Stop` from `Starting`.
6. Repeatedly run event-loop cycles until a stop is requested.
7. Invoke `ITuiCallback::Stopping` for a normal stop.
8. Restore the terminal and re-enable `Console` before returning.

`TUI::Start` blocks on the calling thread for the whole active lifetime. Input, resize, timer, startup, and shutdown callbacks all run synchronously on that owner thread; TUI does not create a UI worker thread.

While TUI is active, the ordinary `Console` read, write, color, and title operations are disabled rather than redirected into the TUI buffer. Do not manually call `Console::Enable()` while TUI owns the terminal.

### Stop Is a Request

`TUI::Stop` sets a stop-request flag. It does not restore the terminal or invoke `Stopping` inside the current callback.

- No later non-stopping listener for the current event, or queued input event, is dispatched after the request is observed.
- The current callback and any nested `TUI::RunOneCycle` calls unwind first.
- `TUI::IsStopRequested()` lets an adapter suppress additional higher-level callbacks after requesting a stop.
- `TUI::Stop()` and `TUI::IsStopRequested()` are harmless while TUI is inactive; they respectively do nothing and return false.
- A stop request during `Stopping` is ignored because shutdown is already in progress.

Calling `TUI::Start` again from the active owner thread is a no-op. Calling it from another thread while active violates the owner-thread contract.

### Callback Exceptions

If `Starting`, an input callback, a resize callback, or a timer callback throws, TUI stops dispatching user callbacks, skips `Stopping`, restores acquired terminal and `Console` state, and rethrows the first callback exception. Catching an exception from a nested `RunOneCycle` does not clear it or allow the outer `Start` loop to continue.

If `Stopping` throws, TUI still restores terminal and `Console` state before rethrowing.

## Color Mode Selection

`TuiStartOptions::colorMode` defaults to `TuiColorMode::Auto` and requests one of these modes:

- `TuiColorMode::Auto` lets the backend choose.
- `TuiColorMode::TrueColor` emits full RGB colors when selected.
- `TuiColorMode::Color256` quantizes colors to a 256-color palette.
- `TuiColorMode::Color16` quantizes colors to a 16-color palette.

The backend may select a lower mode than requested. Call `TUI::GetColorMode()` while active to get the selected emission and quantization mode.

The selected mode describes how TUI produces output; it is not a guarantee that the terminal, font, or display renders every requested color exactly.

## Querying Terminal Size

`TUI::TryGetConsoleSize(vint& width, vint& height)` queries the visible terminal size and returns whether the query succeeded. It can be called before `TUI::Start`.

While active:

- `TUI::GetBufferWidth()` and `TUI::GetBufferHeight()` return the allocated cell-buffer dimensions.
- `TUI::GetBuffer()` returns the row-major buffer.
- Cell `(x, y)` is `TUI::GetBuffer()[y * TUI::GetBufferWidth() + x]`.

When the visible size changes, TUI:

1. Allocates a new buffer.
2. Preserves the overlapping rectangle.
3. Initializes newly exposed cells as empty character cells with white foreground and black background.
4. Removes an invalid width-two character cut by the new right boundary.
5. Replaces the public buffer pointer.
6. Invokes `ITuiCallback::BufferSizeChanged`.

Do not retain a pointer returned by `TUI::GetBuffer()` across a resize, after requesting `TUI::Stop`, or into a later `TUI::Start`.

## Event Callbacks

All `ITuiCallback` methods have default no-op implementations, so a listener only needs to override the events it uses.

### Listener Registration

`TUI::InstallListener` and `TUI::UninstallListener` return `bool`.

- Installing `nullptr` or an already installed pointer returns false.
- Uninstalling `nullptr` or an absent pointer returns false.
- Listener order is installation order.
- Installation and removal are allowed during callbacks on the owner thread.
- Removing a listener prevents it from being called later in the current event.
- Removing and reinstalling the same pointer creates a new registration; an old event snapshot cannot invoke that new registration.
- A listener installed during an event begins with the next event. A nested `TUI::RunOneCycle` is a distinct event and sees the current registrations.

### Mouse Events

`TuiMouseInfo` reports terminal-cell coordinates, wheel information, best-effort modifier state, and current left, middle, and right button state. `TuiMouseButton` identifies the button for down, up, and double-click callbacks.

Available callbacks are:

- `MouseMove`
- `MouseDown`
- `MouseUp`
- `MouseDoubleClick`
- `MouseVerticalWheel`
- `MouseHorizontalWheel`

### Key Events

`TuiKeyInfo` carries modifier and repeat information, but `TuiKeyInfo::code` is reserved for future key-code translation. Do not depend on a portable nonzero key code. Use `ITuiCallback::Char` for text and character controls.

Modifier fields are best effort. A terminal protocol that cannot distinguish Shift or Caps Lock from the resulting character reports the unobservable fields as false.

### Character Events Use Native `wchar_t` Units

`TuiCharInfo::code` is exactly one platform-native `wchar_t` code unit, not necessarily one complete Unicode scalar.

- On Windows, `wchar_t` is UTF-16. A supplementary scalar arrives as separate high- and low-surrogate `Char` callbacks in native order. Other event types may occur between them, and a stop requested after the high surrogate suppresses the queued low surrogate.
- On Linux and macOS, `wchar_t` is UTF-32. A decoded scalar normally arrives in one `Char` callback.
- Each callback carries the modifier state associated with that native input unit.

A scalar-oriented listener owns any incremental conversion state. Use `vl::encoding::UtfConversion<wchar_t>::To32` when conversion is needed. On UTF-16, retain at most one high surrogate, combine only an adjacent low surrogate in the listener's character-unit stream, and define how unmatched units are discarded or replaced without swallowing the current valid unit or control character.

Do not pass an individual UTF-16 surrogate to scalar APIs such as `TUI::MeasureChar` or `TUI::PrintChar`.

## Timers and Reentrant Event Pumping

`TUI::StartTimer(vint milliseconds)` starts or replaces the one TUI timer. The period must be positive. `TUI::StopTimer()` disables it.

The timer:

- Uses deadlines instead of busy polling.
- Shares the owner-thread event loop with terminal input and resize events.
- Invokes `ITuiCallback::Timer` on the `TUI::Start` thread.
- Is global to the active TUI, not one timer per listener.

`TUI::RunOneCycle()` is the public owner-thread event-pump primitive used by `TUI::Start`. One call waits as necessary and dispatches one bounded unit of queued input, resize, or timer work. It returns false once a stop is requested.

`RunOneCycle` is intentionally reentrant. A same-thread modal loop may call it from a callback, and the nested call may consume already queued events. Do not cache iterators, event references, buffer pointers that could be invalidated by resize, or assumptions about listener registration across a nested cycle.

## Pixels and the Cell Buffer

`TuiPixel` represents one terminal cell. Its `glyph` selects the active union member:

- `TuiPixelGlyph::Char` stores an empty cell (`c == 0`) or one Unicode scalar.
- `TuiPixelGlyph::Mergeable` stores four independently styled box-drawing arms.
- `TuiPixelGlyph::Unmergeable` stores a rounded corner and its direction.
- `TuiPixelGlyph::WideCharContinuation` marks the second cell occupied by a width-two scalar.

Every pixel also contains logical RGB foreground and background colors.

Each arm in `TuiMergeablePixel` is independently `None`, `ThinLine`, `ThickLine`, or `DoubleLine`. `TuiUnmergeablePixel` currently represents a `RoundCorner` with a left-top, right-top, left-bottom, or right-bottom direction.

### Reading a Pixel as Text

`TuiPixel::GetChar32()` returns:

- The stored scalar for a character cell.
- The Unicode box-drawing scalar for a valid mergeable or rounded-corner cell.
- Zero for an empty cell, a continuation cell, or an unsupported raw glyph state.

`TuiPixel::GetWChar()` returns the value only when it fits in exactly one native `wchar_t`. On UTF-16 Windows, it returns zero for supplementary scalars because they require two code units.

### Character Width

`TUI::MeasureChar(char32_t code)` classifies one Unicode scalar for the current platform:

- `0` means invalid, zero-width, control, or otherwise not independently printable.
- `1` means one terminal cell.
- `2` means two terminal cells.

This API works with Unicode scalars, while `TuiCharInfo::code` works with native `wchar_t` units. Convert character input before measuring it when the native encoding can use multiple units.

The cell model intentionally does not perform grapheme shaping, combining sequences, variation sequences, emoji ZWJ sequences, bidirectional layout, or complex-script layout. Each printable scalar is measured and stored independently.

### Width-Two Invariant

A width-two scalar is represented by:

1. A leading `TuiPixelGlyph::Char` cell containing the scalar.
2. A following `TuiPixelGlyph::WideCharContinuation` cell with identical foreground and background colors.

The drawing helpers repair a previous width-two pair before overwriting either half. Code that writes directly through `TUI::GetBuffer()` must preserve the same invariant.

`TUI::RenderBuffer()` validates the complete active buffer before rendering. It rejects invalid scalars, standalone zero-width or control characters, unsupported line-arm states, invalid rounded corners, missing or orphaned continuation cells, and color mismatches within width-two pairs.

## Drawing Operations

TUI provides two forms of each drawing operation:

- Active-buffer overloads operate on `TUI::GetBuffer()` and require an active TUI on the owner thread.
- Buffer-explicit overloads begin with `TuiPixel* buffer, vint width, vint height` and can draw into a caller-owned temporary buffer without starting TUI.

The operations are:

- `TUI::PrintChar`
- `TUI::DrawLineV`
- `TUI::DrawLineH`
- `TUI::DrawRect`
- `TUI::Clear`

Buffer-explicit overloads require a non-null buffer and positive dimensions.

### Printing Characters

`TuiPrintOptions` supplies foreground and background colors.

`TUI::PrintChar`:

- Requires a valid Unicode scalar.
- Writes width-one and width-two scalars using the cell representation above.
- Does nothing for a zero-width or non-printable scalar.
- Clips an out-of-bounds leading coordinate.
- Does not write a width-two scalar unless both cells fit.
- Replaces the complete old width-two character when either destination cell intersects it.

### Lines

`TuiLineOptions` selects `ThinLine`, `ThickLine`, or `DoubleLine`, a foreground color, and an optional background color.

- `DrawLineV(options, x, y1, y2)` requires `y1 <= y2`.
- `DrawLineH(options, x1, x2, y)` requires `x1 <= x2`.
- Endpoints are inclusive.
- Every cell, including both endpoints and a one-cell line, contains both opposing arms of the selected line style.
- Coordinates outside the buffer are clipped.
- A fully clipped line is a no-op.
- An empty `backgroundColor` preserves each destination background.
- A present `backgroundColor`, including black, replaces it.

### Rectangles

`TuiRectOptions` adds `TuiRectCorner::Sharp` or `TuiRectCorner::Round`.

`DrawRect(options, x1, y1, x2, y2)` requires `x1 < x2` and `y1 < y2`. Coordinates are inclusive and clipped to the buffer without inventing new corners at a clipped edge.

Sharp rectangles support thin, thick, and double lines. Rounded rectangles require a thin line; rounded corners are unmergeable.

### Clearing

`TUI::Clear(backgroundColor, x1, y1, x2, y2)` requires ordered inclusive ranges. It clips to the buffer and replaces affected cells with empty character cells, the specified background, and white foreground.

### Line Merging

Sharp line and rectangle operations merge compatible box-drawing arms when they overlap:

- A later operation replaces any arm direction that it supplies.
- The later foreground color applies to the resulting cell.
- The optional later background only replaces the destination background when it has a value.
- If the combined arms have an exact Unicode box-drawing character, the combined geometry is kept.
- If a combination containing double lines has no exact Unicode representation, the whole cell falls back to the later operation's unmerged shape.

Do not assume that every arbitrary thin, thick, and double four-arm combination is representable. Directly written mergeable states that have no Unicode representation fail `RenderBuffer`.

## Rendering

Drawing functions only modify cells. Call `TUI::RenderBuffer()` to submit the complete active buffer to the terminal.

TUI is a retained cell buffer rather than a stream of drawing commands:

- Rendering does not invoke user callbacks.
- A caller can update many cells and render once.
- Resize preserves only overlapping cells; applications that derive layout from logical state can repaint in `BufferSizeChanged`.
- TUI validates the entire buffer on each render, so direct buffer access remains subject to all glyph and width invariants.

## Thread Affinity

TUI state is owner-thread-only.

- Active operations, including `RunOneCycle`, `Stop`, timer control, active-buffer access, rendering, color-mode access, and listener mutation, must run on the thread that called `TUI::Start`.
- Cross-thread stop requests are not supported.
- `TUI::IsInUse()` and `TUI::IsStopRequested()` return false while inactive; while active they enforce the owner-thread check.
- `TryGetConsoleSize`, listener setup while inactive, pixel conversion, `MeasureChar`, and buffer-explicit drawing do not require an active TUI, but they do not make TUI global state thread-safe.

## Deterministic Backend Injection for Tests

The public header exposes a test-only boundary in `vl::console::unittest`:

- `ITuiBackend` abstracts startup, shutdown, size query, monotonic time, event reading, and rendering.
- `TuiBackendEvent` carries resize, mouse, key, and native-unit character events.
- `ScopedTuiBackend` temporarily installs a backend and restores the previous backend at scope exit.

Create or destroy a `ScopedTuiBackend` only while TUI is inactive. The backend must be non-null. While installed, it is also used by `TUI::TryGetConsoleSize`.

This boundary enables deterministic lifecycle, callback, timer, resize, rendering, and failure tests without controlling a real terminal. Production applications should use the platform backend selected by `TUI::Start`.
