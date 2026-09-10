# Using Terminal UI

`vl::console::TUI` provides terminal input, cell drawing and screen updates on Windows, Linux and macOS. Its public types are declared in `Source/TUI/TUI.h` and shipped in `VlppOS.h`. For an application built from GacUI controls, use the [GacUI terminal setup functions](.././gacui/tui.md), which manage this lower-level session for you.

## Session and callbacks

Implement `ITuiCallback`, install the listener with `TUI::InstallListener`, and call `TUI::Start(TuiStartOptions{})` on the thread that owns the session. `Start` takes over the terminal and runs until `TUI::Stop()` is requested on that thread; it restores terminal state before returning or rethrowing a callback exception. Keep the listener alive until it is uninstalled, including when startup throws.

`Starting`, `BufferSizeChanged`, keyboard and mouse callbacks, and `Timer` run on the owner thread. Use `TUI::StartTimer(milliseconds)` with a positive interval for periodic work. `Stop` requests exit; it does not immediately destroy the active session. `TUI::IsStopRequested()` distinguishes that request from `TUI::IsInUse()`, which remains true while the session is active. Active-session APIs require the owner thread.

Run with interactive terminal input and output, without redirecting them to files or pipes. Ordinary `Console` operations are disabled while TUI owns the terminal. `TuiStartOptions::colorMode` accepts `Auto`, `TrueColor`, `Color256` or `Color16` from `TuiColorMode`. Query the selected mode with `TUI::GetColorMode()`. On POSIX, automatic detection uses `COLORTERM` for true color and `TERM` for 256 colors, falling back to 16 colors.

## Cell drawing and clipping

The active buffer is available through `GetBuffer`, `GetBufferWidth` and `GetBufferHeight`. Reacquire its pointer and dimensions after `BufferSizeChanged`; resizing can replace the buffer. Draw with `PrintChar`, `DrawLineV`, `DrawLineH`, `DrawRect` and `Clear`, then call `RenderBuffer` to update the terminal. Each helper also has an overload taking `TuiPixel* buffer, vint width, vint height` for caller-owned buffers without an active terminal session.

Every drawing overload accepts a final optional `const TuiClipper* clipper = nullptr`. The clipper covers `[x1, x2)` and `[y1, y2)` and is intersected with the buffer bounds; a null clipper uses the entire buffer. Its fields default to zero, so a pointer to a default-constructed clipper selects an empty area. Inverted bounds also paint nothing; clipping does not reorder them. Line, rectangle and `Clear` drawing coordinates include their endpoints. Clipping masks the original geometry, so it does not move a rectangle's border to the clip edge.

For example, draw into a caller-owned 12 by 4 cell buffer while limiting writes to its inner area:
```C++
using namespace vl::console;

TuiPixel cells[12 * 4] = {};
TuiClipper clip{ 1, 1, 11, 3 };
TuiPrintOptions text;
TUI::Clear(cells, 12, 4, TuiColor{ 0, 0, 32 }, 0, 0, 11, 3, &clip);
TUI::PrintChar(cells, 12, 4, text, U'A', 1, 1, &clip);
```

`MeasureChar` reports a scalar's cell width. A width-two character requires both cells inside the clip; a clipped half is not drawn. Drawing over either half of an existing wide character repairs its paired cell, even if that paired cell lies outside the clip, to keep the buffer valid. Prefer the drawing helpers over directly modifying wide-character and continuation cells.

## Portable input

Keyboard, character and mouse events use the shared native input types from `vl::presentation`. Modifier fields, including `osSuper`, describe the events received from the terminal. The terminal or desktop can consume shortcuts before delivery, and POSIX terminal protocols do not always report physical key releases or distinguish every key combination. Check essential commands in the target terminal and provide alternatives for intercepted shortcuts.

