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

	void Char(const vl::presentation::NativeWindowCharInfo& info) override
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

### Shared Input Declarations

[Source/TUI/TUITypes.h](../../Source/TUI/TUITypes.h) is the single declaration owner. These types remain in `vl::presentation`, depend only on Vlpp, and are consumed by GacUI and GacJS's generated protocol. GacUI owns reflection and platform key-name tables. Do not duplicate the declarations or add GacUI/reflection dependencies to VlppOS.

| Declaration | Meaning and defaults |
| --- | --- |
| `GuiCoordinate` | `vint` alias for logical GUI coordinates; TUI mouse coordinates are terminal cells. |
| `NativeCoordinate` | Strong `vint` wrapper for native coordinates, initially zero, with arithmetic and defaulted comparison. |
| `CompareCoordinate` | Orders coordinates by their underlying integer values. |
| `VKEY` | Virtual-key identifiers independent of text; UNKNOWN=-1 and MAXIMUM=255. |
| `NativeMouseButton` | Left, Middle, Right, Mouse4, Mouse5; identifies the button for down/up/double-click. |
| `WindowMouseInfo_<T>` | `x/y`, signed `wheel`, `ctrl/shift/alt/osSuper`, held `left/middle/right`, and `nonClient`; all zero/false. |
| `WindowMouseInfo` | `WindowMouseInfo_<GuiCoordinate>`; TUI callback payload. |
| `NativeWindowMouseInfo` | `WindowMouseInfo_<NativeCoordinate>`; native window payload. |
| `NativeWindowKeyInfo` / `WindowKeyInfo` | Struct and alias: `code=KEY_UNKNOWN`; `ctrl/shift/alt/osSuper/capslock/autoRepeatKeyDown=false`. |
| `NativeWindowCharInfo` / `WindowCharInfo` | Struct and alias: one native `wchar_t code=0`; `ctrl/shift/alt/osSuper/capslock=false`. |

Alt/terminal Meta and OS Super (Windows/Command/Super) are independent. Unobservable fields remain false. TUI's `nonClient` is always false. Held-button fields cover three buttons even though the shared event-button enum has five.

The complete keyboard macro chain (`GUI_DEFINE_KEYBOARD_CODE_BASIC`, `GUI_DEFINE_KEYBOARD_CODE_ADDITIONAL`, `GUI_DEFINE_KEYBOARD_CODE`, enum-item macro and its `#undef`) and VKEY bit operators live with the enum. The corrected aliases are `KEY_LEFT_BRACKET=0xDB` ([, OEM_4) and `KEY_RIGHT_BRACKET=0xDD` (], OEM_6); other numeric values are unchanged.

### Mouse Events

`MouseMove`, `MouseDown`, `MouseUp`, `MouseDoubleClick`, `MouseVerticalWheel` and `MouseHorizontalWheel` receive `WindowMouseInfo`. Coordinates are zero-based visible cells, positive X right and Y down. Wheel deltas use 120 per detent, positive vertical up and horizontal right. Held flags describe the state after the transition. DoubleClick replaces the second Down.

Windows consumes `MOUSE_EVENT_RECORD`, subtracts the viewport origin and decodes the signed high word for both wheel axes. Ctrl/Shift/Alt and the three primary buttons are preserved. A native double-click selects the changed button while retaining other held buttons. [Microsoft mouse record contract](https://learn.microsoft.com/en-us/windows/console/mouse-event-record-str).

POSIX enables all-motion mode 1003 and SGR mode 1006. Coordinates must be positive before conversion. Cb bits 4/8/16 are Shift/Alt/Ctrl and 32 is motion. Base 0/1/2 mean left/middle/right; motion base 3 explicitly clears held state. Final M presses/moves, m releases. Base 64/65 gives vertical +120/-120; 66/67 gives horizontal -120/+120. Extra/overflowing parameters, impossible button codes, zero coordinates and wheel releases are discarded. A same-button down at the same cell within 500 monotonic milliseconds becomes DoubleClick and resets the candidate. [xterm input reference](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html).

### Key Events and Production Decoders

Use `KeyDown/KeyUp` for key actions and `Char` for text. Printable input may produce both; consumers must not act twice.

Windows translates `wVirtualKeyCode` in 1..255 directly, otherwise UNKNOWN. Each `wRepeatCount` unit emits KeyDown followed by its nonzero native character. The first press is not a repeat; later units and down records while held are repeats. One KeyUp clears held state without text. Start/Stop clears decoder state. Ctrl/Shift/Alt/Caps Lock come from the record. [Microsoft key record contract](https://learn.microsoft.com/en-us/windows/console/key-event-record-str).

Windows Terminal extends `dwControlKeyState` with right Win `0x0200` and left Win `0x0400`, as defined in its [ControlKeyStates.hpp](https://github.com/microsoft/terminal/blob/v1.24.11911.0/src/cascadia/TerminalCore/ControlKeyStates.hpp). The Windows decoder maps either bit to `osSuper` in key and mouse records and copies it into each accompanying Char event. These flags are host extensions, absent from the public console flag list. Native Windows Terminal 1.24.11911.0 tracing observed Q with Ctrl+Alt+left Win as `0x040A` and right Win as `0x020A`, without separate Win-key transitions. Read each event's bits directly; asynchronous key polling would substitute current state for queued event state. Hosts that omit these bits continue to report false; the decoder does not synthesize missing Win transitions or make intercepted OS chords available.

The production POSIX decoder in [TUI.Input.cpp](../../Source/TUI/TUI.Input.cpp) retains incomplete bytes and decoded events across reads:

- ASCII letters/digits/space/punctuation, Tab, Enter, Backspace and Escape map to shared keys. Inferable control bytes map to Ctrl plus their key. Uppercase/shifted punctuation does not imply observable Shift.
- CSI/SS3 arrows, Home/End, Insert/Delete, PageUp/PageDown, F1..F20 forms, Shift-Tab and SS3 application keypad forms translate to VKEY. Legacy modifier parameters 1..16 preserve Shift/Ctrl and terminal Alt/Meta; legacy Meta maps to Alt, never OS Super.
- The POSIX backend pushes Kitty disambiguation mode (`CSI >1u`) and queries its flags. A supported-mode response enables Kitty modifiers for functional keys; CSI-u keys use that encoding directly. Shift/Alt/Ctrl/Super and Caps Lock remain independent; Kitty Meta maps to Alt. Modified printable CSI-u keys produce KeyDown without inventing committed text. Escape, Enter, Tab and Backspace retain Char semantics. Unsupported enhancements and invalid scalars are discarded. The backend pops the mode before leaving the alternate screen. See the [Kitty keyboard protocol](https://sw.kovidgoyal.net/kitty/keyboard-protocol/).
- Text/control keys emit KeyDown then Char; special keys without text emit only KeyDown. POSIX DEL Backspace and LF Enter are normalized to Char backspace (U+0008) and carriage return (U+000D), matching their native key identities and preventing a second edit after KeyDown. POSIX does not synthesize KeyUp.
- Lone Escape has a 30 ms monotonic deadline. An ordinary scalar following before expiry becomes Alt-prefixed input. Incomplete UTF-8/CSI/SS3/SGR remains pending; read boundaries do not complete events.
- Numeric fields validate syntax, ranges and overflow before translation. Unsupported complete CSI/SS3 is consumed without suffix leakage. Overlong incomplete sequences discard through their final byte. OSC/DCS/SOS/PM/APC strings discard through their terminator; a new Escape can resynchronize.
- Invalid UTF-8 produces replacement units without dropping later valid text. Buffered events/bytes drain before polling. The earliest timer or Escape deadline bounds blocking, and interrupted poll returns to the owner loop to recompute deadlines.

### Terminal Limits on OS Super

Every input payload initializes `osSuper` to false. Windows fills it from each key or mouse record, including mouse movement, buttons, double-clicks and both wheel axes, and copies each key event's flag to accompanying Char events. Linux/macOS fill it from Kitty keyboard modifiers and preserve it in accompanying Char events.

Standard SGR mouse reports encode only Shift, Alt and Ctrl; their `osSuper` remains false even in Kitty. Legacy text has no independent Super bit either. Do not infer mouse Super from the last keyboard event: the current keyboard mode does not report standalone modifier releases, so that state could remain stale indefinitely.

The terminal application matters independently of the operating system. GNOME Terminal 3.52.0 with VTE 0.76.0 does not transmit Super. Its [key mapper](https://github.com/GNOME/vte/blob/0.76.0/src/keymap.cc) removes unsupported modifiers. A probe of that installed VTE widget produced identical bytes with and without Super: Ctrl+Alt+Q emitted hex `1b 11`, and Ctrl+Alt+Shift+F8 emitted `ESC [19;8~`. VlppOS cannot reconstruct a missing modifier from identical input. Local Super shortcuts require a terminal that reports it through the Kitty keyboard protocol. Native global shortcuts are handled by the downstream platform's separate input service.

### Character Events Use Native `wchar_t` Units

`vl::presentation::NativeWindowCharInfo::code` is exactly one platform-native `wchar_t` code unit, not necessarily one complete Unicode scalar.

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

- `TuiPixelGlyph::Char` selects `character`, a `TuiCharPixel` containing an empty cell (`character.c == 0`) or one Unicode scalar and its `TuiTextStyle style`.
- `TuiPixelGlyph::Mergeable` stores four independently styled box-drawing arms.
- `TuiPixelGlyph::Unmergeable` stores a rounded corner and its direction.
- `TuiPixelGlyph::WideCharContinuation` marks the second cell occupied by a width-two scalar.

Every pixel also contains logical RGB foreground and background colors.

`TuiTextStyle` contains `bold`, `italic`, `underline`, and `strikeline`, all false by default. Only nonempty character glyphs use these flags, including literal spaces. Empty cells and geometric glyphs render with all four effects disabled. A wide character's leading cell owns its style; continuation cells do not emit text or style changes. Direct cell users must migrate the former `TuiPixel::c` access to `TuiPixel::character.c`.

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

This API works with Unicode scalars, while `vl::presentation::NativeWindowCharInfo::code` works with native `wchar_t` units. Convert character input before measuring it when the native encoding can use multiple units.

Windows uses `GetStringTypeW(CT_CTYPE1/CT_CTYPE3)`: invalid scalars, controls and nonspacing/vowel marks return 0; halfwidth returns 1; supplementary scalars and fullwidth/ideograph/Hiragana/Katakana return 2; other scalars return 1. `C3_DIACRITIC` alone does not imply zero width: spacing ASCII circumflex and grave accent carry that flag and occupy one cell. This approximates console layout.

Linux/macOS use `wcwidth` under a cached environment `newlocale(LC_CTYPE_MASK, "", nullptr)`, temporarily selected using `uselocale` and restored on the calling thread. Negative widths become 0. No process-global `setlocale` or generated Unicode-width table is used. Results depend on platform/locale and may differ from the terminal/font.

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

Buffer-explicit overloads require a non-null buffer and non-negative dimensions. A zero-sized buffer paints nothing, while invalid drawing arguments still fail validation.

Both overload families append `const TuiClipper* clipper = nullptr`. `TuiClipper` in `Source/TUI/TUI.h` contains `vint x1, y1, x2, y2` with half-open bounds `[x1, x2) x [y1, y2)`. Null selects the whole current buffer. Every operation normalizes the clip against that buffer; empty, inverted and disjoint intersections paint nothing. Drawing endpoints remain inclusive and retain their original geometry, so an interior clip cannot invent rectangle edges or corners. Argument validation precedes clipping.

A new width-two character requires both cells inside the normalized clip before either cell changes. Overwriting an existing lead or continuation may clear its partner immediately outside the clip; this is the only repair spill, preserves the partner background and never changes unrelated cells. Active-buffer operations reacquire the current buffer after resize rather than retaining its previous pointer or dimensions.

`TuiLineOptions::foregroundColorBlending` and `TuiRectOptions::foregroundColorBlending` optionally transform the destination foreground for each accepted painted cell, before wide-cell repair. When absent, `foregroundColor` is used directly. The transform performs synchronous, pure color calculation; it must not mutate or pump TUI state. This lets clients retain destination-dependent RGB policies without duplicating raster clipping or copying a buffer. No transform runs for clipped cells or rectangle interiors, and its work is proportional to painted cells. TUI colors remain RGB, without a GUI alpha dependency.

### Printing Characters

`TuiPrintOptions` supplies foreground and background colors plus a default-disabled `TuiTextStyle style`. `PrintChar` copies the complete style into the leading character cell, including when replacing a geometric glyph or differently styled text.

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

### Exact Glyph Selection

`GetMergeableChar` in [TUI.cpp](../../Source/TUI/TUI.cpp) maps all 80 nonempty none/thin/thick combinations exactly; all-none returns zero. Supported double/thin-double states use U+2550..U+256C; unsupported states return zero. Rounded corners use U+256D top-left, U+256E top-right, U+256F bottom-right and U+2570 bottom-left.

This is the exact lookup. Arms are up/down/left/right, with 0=None, 1=Thin, 2=Thick, 3=Double.

| Arms | Unicode | Arms | Unicode |
| --- | --- | --- | --- |
| 0011 | U+2500 | 0022 | U+2501 |
| 1100 | U+2502 | 2200 | U+2503 |
| 0101 | U+250C | 0102 | U+250D |
| 0201 | U+250E | 0202 | U+250F |
| 0110 | U+2510 | 0120 | U+2511 |
| 0210 | U+2512 | 0220 | U+2513 |
| 1001 | U+2514 | 1002 | U+2515 |
| 2001 | U+2516 | 2002 | U+2517 |
| 1010 | U+2518 | 1020 | U+2519 |
| 2010 | U+251A | 2020 | U+251B |
| 1101 | U+251C | 1102 | U+251D |
| 2101 | U+251E | 1201 | U+251F |
| 2201 | U+2520 | 2102 | U+2521 |
| 1202 | U+2522 | 2202 | U+2523 |
| 1110 | U+2524 | 1120 | U+2525 |
| 2110 | U+2526 | 1210 | U+2527 |
| 2210 | U+2528 | 2120 | U+2529 |
| 1220 | U+252A | 2220 | U+252B |
| 0111 | U+252C | 0121 | U+252D |
| 0112 | U+252E | 0122 | U+252F |
| 0211 | U+2530 | 0221 | U+2531 |
| 0212 | U+2532 | 0222 | U+2533 |
| 1011 | U+2534 | 1021 | U+2535 |
| 1012 | U+2536 | 1022 | U+2537 |
| 2011 | U+2538 | 2021 | U+2539 |
| 2012 | U+253A | 2022 | U+253B |
| 1111 | U+253C | 1121 | U+253D |
| 1112 | U+253E | 1122 | U+253F |
| 2111 | U+2540 | 1211 | U+2541 |
| 2211 | U+2542 | 2121 | U+2543 |
| 2112 | U+2544 | 1221 | U+2545 |
| 1212 | U+2546 | 2122 | U+2547 |
| 1222 | U+2548 | 2221 | U+2549 |
| 2212 | U+254A | 2222 | U+254B |
| 0033 | U+2550 | 3300 | U+2551 |
| 0103 | U+2552 | 0301 | U+2553 |
| 0303 | U+2554 | 0130 | U+2555 |
| 0310 | U+2556 | 0330 | U+2557 |
| 1003 | U+2558 | 3001 | U+2559 |
| 3003 | U+255A | 1030 | U+255B |
| 3010 | U+255C | 3030 | U+255D |
| 1103 | U+255E | 3301 | U+255F |
| 3303 | U+2560 | 1130 | U+2561 |
| 3310 | U+2562 | 3330 | U+2563 |
| 0133 | U+2564 | 0311 | U+2565 |
| 0333 | U+2566 | 1033 | U+2567 |
| 3011 | U+2568 | 3033 | U+2569 |
| 1133 | U+256A | 3311 | U+256B |
| 3333 | U+256C | 0010 | U+2574 |
| 1000 | U+2575 | 0001 | U+2576 |
| 0100 | U+2577 | 0020 | U+2578 |
| 2000 | U+2579 | 0002 | U+257A |
| 0200 | U+257B | 0012 | U+257C |
| 1200 | U+257D | 0021 | U+257E |
| 2100 | U+257F |  |  |

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


### Emission and Colors

Pixels retain logical RGB. TrueColor emits RGB SGR. Color16 chooses from these canonical ANSI entries in index order:

`000000 800000 008000 808000 000080 800080 008080 C0C0C0 808080 FF0000 00FF00 FFFF00 0000FF FF00FF 00FFFF FFFFFF`.

Color256 adds a 6×6×6 cube with levels `0,95,135,175,215,255` at `16+36*r+6*g+b`, then grays `8+10*n` at `232+n` for n=0..23. Quantization minimizes squared RGB distance, choosing the lowest index on ties. Windows Color16 uses the saved active palette, reordered between ANSI/Windows bits, when queryable. Customized palettes may differ from canonical approximations.

VT output skips continuation cells, renders empty cells as spaces, positions each row, coalesces equal colors and styles, and emits UTF-16 on Windows/UTF-8 on POSIX. Both renderers enable bold/italic/underline/strikeline using SGR 1/3/4/9 and disable them using 22/23/24/29. Style changes are independent of colors and each frame ends with an SGR reset. Actual appearance depends on the terminal and font; bold can follow the terminal's intensity preference. Writes handle partial progress; POSIX retries EINTR. Frames are not guaranteed atomic.

Windows development targets Windows 10 or newer. The backend enables `ENABLE_VIRTUAL_TERMINAL_PROCESSING`; Auto chooses TrueColor when available and emits RGB SGR (`38;2;r;g;b` / `48;2;r;g;b`) without palette quantization. Otherwise an owned classic buffer selects Color16 even for a higher request. Classic CHAR_INFO output omits text styles and writes one physical entry per cell: width-two or supplementary leading scalars degrade to ASCII ?, and continuation cells become spaces with copied colors. No legacy DBCS flags are used as generic Unicode-width markers.

POSIX requires an interactive UTF-8 xterm-compatible terminal. Auto chooses TrueColor for COLORTERM containing truecolor/24bit, Color256 for TERM containing 256color, otherwise Color16. Explicit modes override the heuristic.

#### Windows Terminal Requirement

Windows Terminal is the required Windows host for this project's visual verification of bold, italic, underline and strikeline. The user manually confirmed all four effects in Windows Terminal on both Windows 10 and Windows 11. Windows 11 is not required: the terminal host supplies the visual rendering. The earlier underline-only result belongs to the tested built-in console host (`conhost.exe`), not Windows 10 generally. Linux text styles were also manually confirmed. See the [verification results by terminal host](../Jobs/DebugTuiPlaygroundSOP.md#manual-text-style-verification-by-terminal-host).

Install the current stable Windows Terminal app using `winget install --id Microsoft.WindowsTerminal -e`, open it with `wt`, and launch the playground from a shell tab inside it. Windows Terminal 1.11 or later provides all four effects with [configurable intense-text formatting](https://devblogs.microsoft.com/commandline/windows-terminal-preview-1-11-release/). Its profile [intense-text formatting](https://learn.microsoft.com/en-us/windows/terminal/customize-settings/profile-appearance#intense-text-formatting) defaults to `bright`, which does not request a heavier font. Set `"intenseTextStyle": "bold"` or `"all"` at profile level when testing the `bold` flag. Font choice still affects the result. The current Windows Terminal [OS requirement and installation instructions](https://github.com/microsoft/terminal#installing-and-running-windows-terminal) specify Windows 10 version 2004 (build 19041) or newer, including Windows 11; this is separate from VlppOS's Windows 10 development baseline.

The shell (PowerShell or cmd), VT parsing, ConPTY attribute transport, and the terminal's visible renderer are separate parts. Enabling `ENABLE_VIRTUAL_TERMINAL_PROCESSING` establishes VT processing, not per-style drawing support. `TUI::GetColorMode` reports color emission, not font capabilities. TUI emits standard SGR attributes; it cannot make a host draw an unsupported effect. Headless decoding of ConPTY output verifies attributes, not the visible host's glyph rendering.

Microsoft added [extended-attribute transport through ConPTY](https://github.com/microsoft/terminal/pull/2917) separately from [strikethrough rendering](https://github.com/microsoft/terminal/pull/7143) and [italic rendering](https://github.com/microsoft/terminal/pull/8580). [Bold font rendering in the GDI renderer](https://github.com/microsoft/terminal/pull/19441) followed much later; older intensity behavior need not change explicitly selected RGB colors. These changes explain why the tested older inbox conhost can transport attributes that its own renderer does not display. An upstream implementation does not establish availability in an installed host.

### Platform Takeover and Restoration

Windows validates console input/output, saves modes, cursor information, screen geometry and queryable palette/attributes. Raw record input disables line/echo/processed/Quick Edit/VT input and enables mouse/window events. VT uses alternate-screen 1049 and hides the cursor; classic uses an owned screen buffer. Active dimensions match the viewport at origin (0,0), without scrollback. Resize records trigger a fresh viewport query.

Stop exits the alternate screen or reactivates the original buffer, restores buffer/window geometry, cursor position/visibility/size, attributes and modes, and closes the classic buffer. Verify original scrollback and sentinel content immediately after the application returns, before wrapper/prompt output touches the restored console.

POSIX saves termios, applies cfmakeraw with VMIN/VTIME zero using TCSANOW, and saves/replaces SIGWINCH. A nonblocking close-on-exec process-lifetime self-pipe wakes poll; its handler only preserves errno and writes a byte. Size queries and callbacks stay on the owner thread. Stop disables mouse modes, resets SGR/cursor/alternate screen and restores termios, the prior signal action and owner-thread signal mask. Private modes are not portably queryable; inverse sequences are best effort under exclusive ownership.

### Playground Contract

[DebugTuiPlaygroundSOP.md](../Jobs/DebugTuiPlaygroundSOP.md) owns executable verification. The playground rebuilds each frame from semantic state.

- Row 0 is exactly ` Canvas ` (8 cells), ` History ` (9), ` Shapes ` (8), clipped on narrow screens. Text is FFFFFF, selected background 000080, unselected 808080.
- Canvas/History are persistent pages. Shapes is a transient flat menu. Tab/Shift-Tab cycle Canvas/History/Shapes; menu arrows clamp at the first/last of ten entries. Enter accepts; Escape dismisses. Navigation uses KeyDown; Enter/Backspace/Escape use Char once. Tab Char is ignored.
- Canvas's double border starts on row 1; paper (0,0) is terminal (1,2). The bottom command box has background 404040, wraps complete scalars and grows to at most height-1. Width-two scalars survive one-column screens until they fit. Drafts survive navigation; disabling typing discards incomplete UTF-16.
- One chronological list stores each parsed command and exact submitted text for both replay and History. Successful drawing/color/style/clear/type commands append once. `FS [B][I][U][S]` replaces all four current text flags; omitted flags are false and `FS` alone resets them. Flags are case-insensitive and order/duplicates do not matter. Only subsequent `TYPE` drawing consumes this style; geometric drawing and playground UI remain unstyled. HELP/EXIT/errors/canceled previews do not enter History.
- History wraps exact text by display width below the header and starts at newest content. Vertical wheel scrolls three visual rows per 120 units, retaining partial deltas. Position persists through page switches and clamps on resize. New records do not move a user scrolled above the end. Horizontal wheel does nothing.
- The ten styles are LINEV THIN/THICK/DOUBLE, LINEH THIN/THICK/DOUBLE, RECT THIN/THICK/DOUBLE/ROUND. Accept switches to Canvas and arms drawing, retaining draft, changing its text to 808080, and hiding the cursor. The selecting mouse gesture is consumed.
- Only Left acts. Down (or DoubleClick replacing Down) inside paper starts a drag. Each preview replays committed commands plus the same parsed drawing operation, preserving merging/colors/clipping/wide-character repair without ghosts/history changes.
- Release clamps to paper and commits exactly one canonical command. Lines use the anchor's fixed row/column with inclusive normalized endpoints; one-cell lines are valid. Rectangles need two distinct rows and columns; degenerate release stays armed without committing.
- Escape, header navigation, Tab or resize cancels preview/armed mode. During a drag, motion showing left released cancels it. Middle/Right never begin, commit or cancel.
- HELP/errors are modal rounded overlays with opaque black border/interior backgrounds and left-aligned text. Lines wrap only when they exceed the available paper width excluding the overlay border. Without wrapping, the text width is the longest original line; with wrapping, the box fills the available width. The whole box is centered and its layout is recalculated on every frame, including resize. Only Enter dismisses, without submitting. EXIT is the only application exit command; q/Q are ordinary text.
