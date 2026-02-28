# GacUI on CLI

# Goal

GacUI rendered on CLI should be able to:
- Adapt Windows CMD/Powershell, Linux, macOS.
  - Win32API should be enough for Windows.
  - Before introducing third-party libraries for Linux or macOS, its license should be considered, it should be compatible with Apache 2.0, no GPL family license is allowed.
- When the CLI window resizes, the UI should know and automatically adjust its rendering.
  - Part of it could be done with Hosted Mode, in this mode the main window always follow the size of the native window, all other windows are rendered in the same native window as well.
  - GacUI on CLI will offer an `INativeController` implemention to the Hosted Mode implementation, just like how GDI/Direct2D/RemoteProtocol is working with Hosted Mode today.
- Background and foreground colors should be supported. If 24bit true color is not supported, the renderer should automatically find the nearest color.
  - When alpha byte is zero for any element, it is not rendered. Any positive renders with the color, just like how GDI renderer is currently implemented.
- ASCII Art should be used to render elements. For example, when a rectangle is rendered, user should actually see a rectangle consist with precisely selected ASCII characters.
  - The source file should be compatible to UTF-7 so ASCII art characters should use its byte code instead of directly writing it in any char/string literal.
  - Consecutive drawing on the same position should be handled correctly:
    - Drawing a vertical line followed by a horizontal line crossing a certain position, a cross character should be used.
      - Unless two different color is used, then overriding always happen, a horizontal line is rendered at the position.
      - If 24bit true color is not supported, "different colors" here means two converted colors are different, not the original color.
    - Drawing anything followed by text, or drawing text followed by line, will override each other.
    - Drawing a line is different from drawling a text but happen to use a ASCII art character. So extra information should be stored instead of just looking at the current character in the buffer.
- Size of a character matters.
  - Font size will always be 1.
  - One letter might not always occupy one position:
    - Some Chinese characters will always take the space of two letters, measuring is necessary.
    - No LTR/RTL rendering is supported, but code point combination might need to be considered like emoji.
    - Try not to implement this by GacUI, it should always leverage OS functionality or third-party libraries.
- `GuiImageFrameElement` is not supported, by not registering a renderer for it, creating such element crashes.
- Typing should not actually type strings to the CLI window, keyboard input should be completely handled by GacUI itself, even when it is using a text box.
- Mouse input should support.
- An abstration layer should be designed, because many logic mentioned above are cross-platform.
  - For example, always prepare a character buffer before rendering, since rendering is platform independent, and handle the buffer to platform-specific implementation later.
  - Measuring text might be platform-specific, it is fine therefore the abstraction layer should support injecting such differences, e.g. using virtual functions or anything.

## ToDo

- Need a class to handle TUI interaction.
- Need a class to draw elements, like lines, texts, blocks, etc.
- Above two could be in Vlpp, and when it starts, Console::Read/Write will be redirected to callbacks.
- Need a class to convert them to OS provider.

## References

https://en.wikipedia.org/wiki/Box-drawing_characters

## Instructions

You goal is to make a detailed plan to implement `# Goal`.
When API or third-party libraries is introduced, the plan must be detailed enough to mention how to use those functions, with supported materials in links.
Think througly as CLI handling is complex, especially when talking to the OS.
In `## Plan`, list each item in bullet-point list, with supported materials in links if applicable.
In `## Details`, each bullet-point item would be an `### Item`, providing detailed explanation.
When involving API or third-party libraries, it should be detailed enough so that even one without knowledge with them could implement the work immediately.
In the end, call `CheckLinks.ps1` to verify all linkes are available.

## Plan

- Design the cross-platform abstraction layer (`ICliController`, character buffer, render target)
- Implement the character buffer and cell model (including box-drawing state for line merging)
- Implement the box-drawing character lookup table and line merge algorithm
- Implement color handling: 24-bit true color with automatic fallback to nearest terminal-supported color
- Implement character width measurement via platform abstraction (https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-getstringtypew)
- Implement `INativeController` for CLI (`CliNativeController`) providing all 9 required services
- Implement platform-specific terminal I/O for Windows using Win32 Console API (https://learn.microsoft.com/en-us/windows/console/writeconsoleoutput, https://learn.microsoft.com/en-us/windows/console/readconsoleinput, https://learn.microsoft.com/en-us/windows/console/setconsolemode)
- Implement platform-specific terminal I/O for Linux/macOS using ANSI escape sequences and POSIX termios
- Implement keyboard input handling: raw mode, escape sequence parsing, key event translation
- Implement mouse input handling: Win32 `MOUSE_EVENT_RECORD` on Windows, SGR extended mouse mode on Linux/macOS (https://learn.microsoft.com/en-us/windows/console/mouse-event-record-str)
- Implement terminal resize detection: `WINDOW_BUFFER_SIZE_EVENT` on Windows, `SIGWINCH` on Linux/macOS (https://learn.microsoft.com/en-us/windows/console/window-buffer-size-record-str)
- Implement element renderers for CLI (SolidBorder, SolidBackground, GradientBackground, SolidLabel, FocusRectangle, 3DBorder, 3DSplitter, InnerShadow, Polygon, RichDocument)
- Implement `CliGraphicsResourceManager` and `CliGraphicsRenderTarget`
- Implement the setup entry point `SetupCliRenderer()` following the existing `SetupRemoteNativeController` pattern
- Integration testing: rendering, input, resize, color fallback

## Details

### Design the cross-platform abstraction layer

The central design principle is separating platform-independent rendering logic from platform-specific terminal I/O. This mirrors how GacUI already separates `INativeController` (OS abstraction) from element renderers (drawing logic).

The abstraction consists of three layers:

**Layer 1: `ICliPlatform` interface** (platform-specific, `*.Windows.cpp` / `*.Linux.cpp`):
```
class ICliPlatform : public virtual Interface
{
    // Terminal output
    virtual void FlushBuffer(const CliCell* buffer, vint rows, vint cols) = 0;

    // Terminal input
    virtual bool PollInputEvents(List<CliInputEvent>& events, vint timeoutMs) = 0;

    // Terminal size
    virtual void GetTerminalSize(vint& outRows, vint& outCols) = 0;

    // Character width
    virtual vint MeasureCharWidth(char32_t codepoint) = 0;

    // Color capability
    virtual CliColorCapability GetColorCapability() = 0;

    // Lifecycle
    virtual void Initialize() = 0;
    virtual void Finalize() = 0;
};
```

**Layer 2: `CliCharacterBuffer`** (platform-independent):
A 2D grid of `CliCell` structs that all element renderers draw into. After all elements are rendered, the buffer is handed to `ICliPlatform::FlushBuffer()`.

**Layer 3: Element renderers** (platform-independent):
Each GacUI element type has a CLI renderer that draws into the `CliCharacterBuffer`. These renderers follow the same `GuiElementRendererBase<TElement, TRenderer, TRenderTarget>` pattern used by GDI, Direct2D, and Remote renderers.

The `ICliPlatform` interface is injected into the `CliNativeController` at construction. Two implementations are provided: `WindowsCliPlatform` using Win32 Console API, and `PosixCliPlatform` using ANSI escape sequences with POSIX termios. Each lives in its own `*.Windows.cpp` / `*.Linux.cpp` file per the project convention.

### Implement the character buffer and cell model

The character buffer is a 2D array of `CliCell` that represents the terminal screen. Each cell stores enough information to support line merging, color handling, and text/line distinction.

Cell structure:
```
struct CliBoxDrawingState
{
    vuint8_t up    : 2;  // 0=None, 1=Light, 2=Heavy, 3=Double
    vuint8_t down  : 2;
    vuint8_t left  : 2;
    vuint8_t right : 2;
};

struct CliColor
{
    vuint8_t r;
    vuint8_t g;
    vuint8_t b;
    vuint8_t a;  // 0 = transparent (not rendered)
};

enum class CliCellKind : vuint8_t
{
    Empty,
    Text,
    Line,
    WideCharTrailing,  // second cell occupied by a wide character
};

struct CliCell
{
    wchar_t              character;
    CliColor             foreground;
    CliColor             background;
    CliCellKind          kind;
    CliBoxDrawingState   lineState;  // only meaningful when kind == Line
};
```

The buffer is allocated as `CliCell* buffer = new CliCell[rows * cols]` and reallocated when the terminal resizes. A helper class `CliCharacterBuffer` manages the allocation, provides `SetCell(row, col, ...)` and `GetCell(row, col)` methods, and handles bounds checking.

For wide characters (CJK ideographs that occupy 2 columns), the first cell stores the character and `kind = Text`, while the second cell stores `kind = WideCharTrailing` to indicate it is the continuation of the previous cell. This prevents subsequent rendering from corrupting the wide character by only overwriting one of its cells.

Before each render pass, the buffer is cleared to `Empty` cells with space character and transparent colors.

### Implement the box-drawing character lookup table and line merge algorithm

Unicode box-drawing characters (U+2500 through U+257F) are decomposed into four directional arms: Up, Down, Left, Right. Each arm has a style: None (0), Light (1), Heavy (2), or Double (3). This 4x2-bit state packs into a single `vuint8_t` key:

```
vuint8_t key = (up << 6) | (down << 4) | (left << 2) | right;
```

A 256-entry lookup table maps each key to a `wchar_t` code point. Invalid combinations map to `L' '` (space). All codepoints are written as hex literals `L'\x2500'` to keep source files UTF-7 compatible.

Key entries in the lookup table:

| State (U,D,L,R) | Key   | Codepoint | Character |
|--|--|--|--|
| (0,0,1,1) | 0x05 | 0x2500 | Light horizontal |
| (1,1,0,0) | 0x50 | 0x2502 | Light vertical |
| (0,1,0,1) | 0x15 | 0x250C | Down-right light corner |
| (0,1,1,0) | 0x14 | 0x2510 | Down-left light corner |
| (1,0,0,1) | 0x41 | 0x2514 | Up-right light corner |
| (1,0,1,0) | 0x44 | 0x2518 | Up-left light corner |
| (1,1,0,1) | 0x51 | 0x251C | Vertical-right T-junction |
| (1,1,1,0) | 0x54 | 0x2524 | Vertical-left T-junction |
| (0,1,1,1) | 0x15 | 0x252C | Horizontal-down T-junction |
| (1,0,1,1) | 0x45 | 0x2534 | Horizontal-up T-junction |
| (1,1,1,1) | 0x55 | 0x253C | Cross (all four light) |
| (0,0,2,2) | 0x0A | 0x2501 | Heavy horizontal |
| (2,2,0,0) | 0xA0 | 0x2503 | Heavy vertical |
| (2,2,2,2) | 0xAA | 0x254B | Heavy cross |
| (0,0,3,3) | 0x0F | 0x2550 | Double horizontal |
| (3,3,0,0) | 0xF0 | 0x2551 | Double vertical |
| (3,3,3,3) | 0xFF | 0x256C | Double cross |

The full table includes all light-heavy mixed combinations (U+250D through U+254A) and all light-double mixed combinations (U+2552 through U+256B). Half-line stubs (U+2574 through U+257B) are included for line termination aesthetics.

**Merge algorithm**: When a renderer draws a line at cell (r, c):
1. Check the existing cell's `kind`.
2. If `kind == Line` and the existing cell's foreground color matches the new line's color (after quantization if 24-bit is not supported), merge the `CliBoxDrawingState` by taking `max(existing, incoming)` per direction.
3. If `kind == Line` but colors differ, override the cell entirely (new line replaces old).
4. If `kind == Text` or `Empty`, override the cell entirely.
5. After merging, look up the resulting character from the lookup table.

Text always overrides lines and lines always override text. Merging only happens for line-on-line with the same color.

### Implement color handling

GacUI elements specify colors as RGBA. The CLI renderer must handle three scenarios:

**24-bit true color** (modern terminals): Use colors directly. On Windows, this requires Virtual Terminal Processing (`ENABLE_VIRTUAL_TERMINAL_PROCESSING` via `SetConsoleMode`). On Linux/macOS, emit SGR sequences `\x1b[38;2;r;g;bm` for foreground, `\x1b[48;2;r;g;bm` for background. Detect capability on Windows by checking if `SetConsoleMode` succeeds with the VT flag. On Linux/macOS, check the `COLORTERM` environment variable for values `truecolor` or `24bit`.

**256-color** (common on Linux): Convert RGB to the nearest color in the xterm 256-color palette. The palette consists of:
- Indices 0-15: standard 16 ANSI colors.
- Indices 16-231: a 6x6x6 color cube where index = 16 + 36*r_idx + 6*g_idx + b_idx (r_idx, g_idx, b_idx each 0-5, mapped from 0-255 via `round(val / 255.0 * 5.0)`).
- Indices 232-255: a 24-step grayscale ramp from rgb(8,8,8) to rgb(238,238,238).

Conversion algorithm: compute the nearest cube color and the nearest grayscale color, then pick the one with the smallest Euclidean distance in RGB space.

**16-color** (Windows legacy): Map RGB to the nearest of 16 console colors using `FOREGROUND_RED|GREEN|BLUE|INTENSITY` and `BACKGROUND_RED|GREEN|BLUE|INTENSITY` attributes. Build a predefined table of the 16 console RGB values and find the nearest match by Euclidean distance.

**Alpha handling**: The GDI renderer convention is followed: alpha = 0 means the element is not rendered (transparent). Any alpha > 0 means the color is rendered as-is (no blending). The CLI renderer should skip rendering for cells where both foreground and background alpha are zero.

The `CliColorConverter` class encapsulates color mapping:
```
class CliColorConverter
{
    CliColorCapability  capability;
    // Convert CliColor (RGBA) to the terminal representation
    // Returns a terminal-native color index or RGB triplet
    CliTermColor Convert(CliColor color);
    // Compare two colors post-conversion (for line merge decision)
    bool ColorsMatch(CliColor a, CliColor b);
};
```

The "different colors" check for line merging uses the **converted** colors, not the original RGBA values, as specified in the Goal section.

### Implement character width measurement

Terminal cells are fixed-width. Most characters occupy 1 cell, but CJK ideographs and fullwidth forms occupy 2 cells. The renderer must know each character's display width for correct layout.

**Windows**: Use `GetStringTypeW` with `CT_CTYPE3` (https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-getstringtypew):
```
WORD charType = 0;
GetStringTypeW(CT_CTYPE3, &ch, 1, &charType);
if (charType & C3_NONSPACING) return 0;     // combining mark
if (charType & (C3_FULLWIDTH | C3_IDEOGRAPH)) return 2; // wide character
return 1; // normal width
```
The flags `C3_FULLWIDTH` (0x0080) and `C3_IDEOGRAPH` (0x0100) indicate 2-column characters. `C3_NONSPACING` (0x0001) indicates zero-width combining marks.

**Linux/macOS**: Use `wcwidth()` from `<wchar.h>`:
```
#include <wchar.h>
#include <locale.h>
setlocale(LC_ALL, "");
int width = wcwidth((wchar_t)codepoint);
// Returns -1 (non-printable), 0, 1, or 2
```

macOS's `wcwidth()` implementation may return incorrect values for some emoji. If more accuracy is needed, a custom `wcwidth` implementation using Unicode East Asian Width property tables can be bundled. The implementation uses a binary search on a sorted array of `{lo, hi}` codepoint ranges:
- Characters in Wide (W) and Fullwidth (F) ranges return 2.
- Characters with `UCHAR_EMOJI_PRESENTATION` property return 2.
- All others return 1 (or 0 for combining marks).

This is exposed through `ICliPlatform::MeasureCharWidth(char32_t codepoint)` so the platform-specific implementation is injected, and the renderers remain platform-independent.

### Implement INativeController for CLI

`CliNativeController` implements `INativeController` (defined in `Source/NativeWindow/GuiNativeWindow.h:1700`) and provides 9 services. Since GacUI on CLI uses Hosted Mode, many services can be simplified:

**`INativeCallbackService`**: Manages `INativeControllerListener` objects. Straightforward implementation with a `List<INativeControllerListener*>`.

**`INativeResourceService`**: Returns a default monospace font descriptor with size 1. Cursor types can be no-ops or minimal implementations since CLI cursors are limited.

**`INativeAsyncService`**: Delegates to the existing async infrastructure. Uses `ThreadPoolLite::QueueLambda` for background work and maintains a queue of callbacks to execute on the main thread, pumped during the event loop.

**`INativeClipboardService`**: Platform-specific. On Windows, use Win32 clipboard API (`OpenClipboard`, `GetClipboardData`, `SetClipboardData`). On Linux, integrate with `xclip` or `xsel` via process spawning, or use OSC 52 escape sequence for terminal clipboard access: `\x1b]52;c;BASE64DATA\x07`.

**`INativeImageService`**: Returns a stub implementation since `GuiImageFrameElement` is not supported. Attempting to use it should indicate unsupported operation.

**`INativeScreenService`**: Reports a single screen with dimensions matching the terminal size.

**`INativeWindowService`**: Creates a single `INativeWindow` that represents the terminal. In Hosted Mode, `GuiHostedController` manages all virtual windows on top of this single native window.

**`INativeInputService`**: Reports keyboard state. Timer functionality is critical for GacUI's animation and event system. The timer can be implemented using a high-resolution clock polled during the event loop, firing callbacks at the requested interval.

**`INativeDialogService`**: Since Hosted Mode replaces OS dialogs with GacUI-rendered dialogs, this can return stub implementations.

The setup follows the pattern from `SetupRemoteNativeController` (`Source/PlatformProviders/Remote/GuiRemoteControllerSetup.cpp:14`):
```
CliNativeController cliController(platform);
GuiHostedController hostedController(&cliController);
// ... resource managers, initialization, GuiApplicationMain(), finalization
```

### Implement platform-specific terminal I/O for Windows

The Windows implementation `WindowsCliPlatform` uses the Win32 Console API.

**Initialization** (https://learn.microsoft.com/en-us/windows/console/setconsolemode):
```
HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

// Input: enable window resize events, mouse, disable quick-edit and line input
DWORD inMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
SetConsoleMode(hIn, inMode);

// Output: attempt VT processing for 24-bit color
DWORD outMode = ENABLE_PROCESSED_OUTPUT
    | ENABLE_WRAP_AT_EOL_OUTPUT
    | ENABLE_VIRTUAL_TERMINAL_PROCESSING
    | DISABLE_NEWLINE_AUTO_RETURN
    | ENABLE_LVB_GRID_WORLDWIDE;
if (!SetConsoleMode(hOut, outMode)) {
    // Fallback: no VT, use classic 16-color CHAR_INFO attributes
    outMode = ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
    SetConsoleMode(hOut, outMode);
}
```

**Screen output** (https://learn.microsoft.com/en-us/windows/console/writeconsoleoutput): Use `WriteConsoleOutputW` to write the entire screen buffer atomically. Build a `CHAR_INFO` array from the `CliCharacterBuffer`:
```
CHAR_INFO ci;
ci.Char.UnicodeChar = cell.character;
ci.Attributes = foregroundAttr | backgroundAttr;
// For wide characters: first cell gets COMMON_LVB_LEADING_BYTE (0x0100)
// second cell gets COMMON_LVB_TRAILING_BYTE (0x0200)
```
Call `WriteConsoleOutputW(hOut, buffer, bufferSize, {0,0}, &writeRegion)`.

If VT processing is enabled and 24-bit color is needed, use `WriteConsoleW` with embedded SGR sequences instead. Position the cursor with `\x1b[row;colH`, set colors with `\x1b[38;2;r;g;bm` and `\x1b[48;2;r;g;bm`. Write entire screen content as one large string in a single `WriteConsoleW` call for flicker-free rendering.

**Double buffering**: Use `CreateConsoleScreenBuffer` (https://learn.microsoft.com/en-us/windows/console/createconsolescreenbuffer) and `SetConsoleActiveScreenBuffer` (https://learn.microsoft.com/en-us/windows/console/setconsoleactivescreenbuffer) to create two screen buffers and alternate between them. Write to the back buffer, then swap it to front. This eliminates flicker entirely for the classic `CHAR_INFO` approach.

**Terminal size** (https://learn.microsoft.com/en-us/windows/console/getconsolescreenbufferinfo):
```
CONSOLE_SCREEN_BUFFER_INFO csbi;
GetConsoleScreenBufferInfo(hOut, &csbi);
vint cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
vint rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
```

### Implement platform-specific terminal I/O for Linux/macOS

The POSIX implementation `PosixCliPlatform` uses ANSI escape sequences and POSIX APIs.

**Initialization**: Enter alternative screen buffer, raw mode, enable mouse:
```
// Alternative screen buffer (preserves user's terminal content)
write(STDOUT_FILENO, "\x1b[?1049h", 8);

// Hide cursor
write(STDOUT_FILENO, "\x1b[?25l", 6);

// Enable mouse button tracking + SGR extended mode
write(STDOUT_FILENO, "\x1b[?1002h\x1b[?1006h", 18);

// Raw mode via termios
struct termios raw;
tcgetattr(STDIN_FILENO, &orig_termios);
raw = orig_termios;
raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
raw.c_oflag &= ~(OPOST);
raw.c_cflag |= CS8;
raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
raw.c_cc[VMIN]  = 0;
raw.c_cc[VTIME] = 1;
tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
```

**Finalization**: Reverse all initialization in reverse order (disable mouse, show cursor, leave alt screen, restore termios).

**Screen output**: Buffer all escape sequences into a single memory buffer, then flush with one `write(STDOUT_FILENO, buf, len)` call per frame. For each cell:
- Position: `\x1b[row;colH` (1-based coordinates)
- Foreground color: `\x1b[38;2;r;g;bm` (24-bit) or `\x1b[38;5;nm` (256-color)
- Background color: `\x1b[48;2;r;g;bm` or `\x1b[48;5;nm`
- Character: UTF-8 encoded bytes

Optimize by skipping unchanged cells between frames (dirty tracking). Use `setvbuf(stdout, NULL, _IOFBF, 65536)` for full buffering if using `fprintf` instead of raw `write`.

**Color capability detection**:
```
const char* colorterm = getenv("COLORTERM");
if (colorterm && (strcmp(colorterm, "truecolor") == 0 || strcmp(colorterm, "24bit") == 0))
    return TrueColor;
const char* term = getenv("TERM");
if (term && strstr(term, "256color"))
    return Color256;
return Color16;
```

**Terminal size**: Use `ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)` to read `struct winsize { unsigned short ws_row, ws_col, ws_xpixel, ws_ypixel; }`.

### Implement keyboard input handling

**Windows** (https://learn.microsoft.com/en-us/windows/console/readconsoleinput): Use `ReadConsoleInputW` to receive `INPUT_RECORD` events. For `KEY_EVENT`:
```
KEY_EVENT_RECORD ke;
// ke.bKeyDown - true for press, false for release
// ke.wVirtualKeyCode - VK_LEFT, VK_RETURN, VK_ESCAPE, etc.
// ke.uChar.UnicodeChar - the translated character (0 for non-character keys)
// ke.dwControlKeyState - LEFT_CTRL_PRESSED, SHIFT_PRESSED, LEFT_ALT_PRESSED, etc.
```
Map (https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes) virtual key codes to GacUI's `VKEY` constants. The `dwControlKeyState` flags map to GacUI's `ctrl`, `shift`, `alt` modifiers.

Keyboard events are not echoed to the terminal because `ENABLE_ECHO_INPUT` and `ENABLE_LINE_INPUT` are not set during initialization. All text input is routed through GacUI's own input system.

**Linux/macOS**: In raw terminal mode, `read(STDIN_FILENO, buf, ...)` returns raw bytes. Regular ASCII characters produce a single byte. Special keys produce escape sequences starting with `\x1b`:

| Key | Sequence |
|--|--|
| Arrow Up/Down/Left/Right | `\x1b[A`, `\x1b[B`, `\x1b[D`, `\x1b[C` |
| Home/End | `\x1b[H`, `\x1b[F` |
| Insert/Delete | `\x1b[2~`, `\x1b[3~` |
| Page Up/Down | `\x1b[5~`, `\x1b[6~` |
| F1-F4 | `\x1bOP`, `\x1bOQ`, `\x1bOR`, `\x1bOS` |
| F5-F12 | `\x1b[15~` through `\x1b[24~` |
| Ctrl+Arrow | `\x1b[1;5A` etc. (modifier in `1;{m}` where m-1 is bitmask: 1=Shift, 2=Alt, 4=Ctrl) |

A standalone `\x1b` with no following bytes within a short timeout (e.g. 50ms) is treated as the Escape key. Use `poll()` with a timeout to distinguish Escape from escape sequences.

### Implement mouse input handling

**Windows** (https://learn.microsoft.com/en-us/windows/console/mouse-event-record-str): The `MOUSE_EVENT_RECORD` from `ReadConsoleInputW` provides:
```
COORD dwMousePosition;  // cell coordinates (column, row)
DWORD dwButtonState;    // FROM_LEFT_1ST_BUTTON_PRESSED, RIGHTMOST_BUTTON_PRESSED, etc.
DWORD dwEventFlags;     // 0 (press/release), MOUSE_MOVED, DOUBLE_CLICK, MOUSE_WHEELED
DWORD dwControlKeyState; // modifier keys
```
Mouse wheel direction is in the high word of `dwButtonState`: positive = scroll up, negative = scroll down.

Mouse must be enabled by setting `ENABLE_MOUSE_INPUT` and clearing `ENABLE_QUICK_EDIT_MODE` (done in initialization). Mouse coordinates are in character cells relative to the screen buffer, which directly maps to the `CliCharacterBuffer` coordinates.

**Linux/macOS**: SGR extended mouse mode (`\x1b[?1006h`) reports mouse events as:
- Press: `\x1b[<btn;x;yM` (capital M)
- Release: `\x1b[<btn;x;ym` (lowercase m)

Where `btn` encodes the button (0=left, 1=middle, 2=right) plus modifiers (4=Shift, 8=Alt, 16=Ctrl) and motion flag (32=motion). `x` and `y` are 1-based cell coordinates. Wheel events use `btn` values 64 (up) and 65 (down).

SGR mode is preferred over legacy X10 mode because it has no column limit (legacy breaks above column 223) and distinguishes press from release.

### Implement terminal resize detection

**Windows** (https://learn.microsoft.com/en-us/windows/console/window-buffer-size-record-str): The `WINDOW_BUFFER_SIZE_EVENT` appears in the input record stream when `ENABLE_WINDOW_INPUT` is set:
```
WINDOW_BUFFER_SIZE_RECORD wbs;
// wbs.dwSize.X = new columns, wbs.dwSize.Y = new rows
```
On receiving this event, call `GetConsoleScreenBufferInfo` (https://learn.microsoft.com/en-us/windows/console/getconsolescreenbufferinfo) to get the precise visible window dimensions from `srWindow`, then reallocate the `CliCharacterBuffer` and notify GacUI by invoking `INativeWindowListener::Moved()` and `INativeWindowListener::Paint()` on the hosted window.

**Linux/macOS**: Install a `SIGWINCH` signal handler that sets a flag:
```
volatile sig_atomic_t g_resize_pending = 0;
void sigwinch_handler(int sig) { g_resize_pending = 1; }

struct sigaction sa;
sa.sa_handler = sigwinch_handler;
sa.sa_flags = SA_RESTART;
sigemptyset(&sa.sa_mask);
sigaction(SIGWINCH, &sa, NULL);
```
In the main event loop, check `g_resize_pending`. When set, query the new size with `ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)`, reallocate the buffer, and notify GacUI.

### Implement element renderers for CLI

Each GacUI element type gets a CLI renderer following the `GuiElementRendererBase<TElement, TRenderer, TRenderTarget>` template pattern. The renderer draws into the `CliCharacterBuffer` accessed via the `CliGraphicsRenderTarget`.

**`GuiSolidBorderElementRenderer`**: Draws a rectangle outline using box-drawing characters. For a rectangle from (left,top) to (right,bottom):
- Top edge: `HorizontalLight` (0x2500)
- Bottom edge: `HorizontalLight` (0x2500)
- Left edge: `VerticalLight` (0x2502)
- Right edge: `VerticalLight` (0x2502)
- Corners: `DownRightLight` (0x250C), `DownLeftLight` (0x2510), `UpRightLight` (0x2514), `UpLeftLight` (0x2518)

Each cell is drawn with `kind = Line` and appropriate `CliBoxDrawingState`, so intersecting borders merge correctly.

**`GuiSolidBackgroundElementRenderer`**: Fills the rectangle with space characters using the element's color as the background color. Cells are drawn with `kind = Text`. Alpha = 0 means skip rendering.

**`GuiGradientBackgroundElementRenderer`**: Fills the rectangle with shade characters (`LightShade` 0x2591, `MediumShade` 0x2592, `DarkShade` 0x2593, `FullBlock` 0x2588) to simulate a gradient. Interpolate between the two gradient colors along the gradient direction, and select the shade character based on the interpolation position. Use the interpolated color as foreground on one of the gradient endpoint colors as background.

**`GuiSolidLabelElementRenderer`**: Renders text. Uses `ICliPlatform::MeasureCharWidth()` to determine how many cells each character occupies. Wide characters (CJK) occupy 2 cells. The first cell gets the character; the second cell gets `kind = WideCharTrailing`. Handles text alignment (left, center, right) and word wrapping based on the element's properties.

`GetMinSize()` returns the measured text size in cell units. This is critical for GacUI's layout system to work correctly with the fixed cell grid.

**`GuiFocusRectangleElementRenderer`**: Draws a dotted rectangle. Uses dashed box-drawing characters (e.g., `LightTripleDashHorizontal` 0x2504, `LightTripleDashVertical` 0x2506) or alternates between line and space characters to create a dashed effect.

**`Gui3DBorderElementRenderer`**: Draws a double-line border using the 3D border's two colors (`color1` for the top-left edges, `color2` for the bottom-right edges) to simulate a raised or sunken effect.

**`Gui3DSplitterElementRenderer`**: Draws horizontal or vertical lines using the splitter direction, with two colors for the 3D effect.

**`GuiInnerShadowElementRenderer`**: Draws shade characters along the inner edges of the rectangle to simulate an inner shadow effect. Uses `LightShade` (0x2591) or `MediumShade` (0x2592) characters with the shadow color.

**`GuiPolygonElementRenderer`**: Simplified: since CLI cells are coarse, polygon rendering approximates the shape. For simple polygons, render them as a filled rectangle or outlined rectangle. For complex shapes, use a simple scanline fill with block characters.

**`GuiDocumentElementRenderer`**: Reuses the existing `GuiDocumentElementRenderer::Register()` mechanism. The document element has its own rendering pipeline that ultimately calls SolidLabel and SolidBorder renderers internally.

**`GuiImageFrameElement`**: Not registered. Per the Goal specification, not registering a renderer for it means creating such an element crashes, which is the intended behavior.

### Implement CliGraphicsResourceManager and CliGraphicsRenderTarget

**`CliGraphicsRenderTarget`** implements `IGuiGraphicsRenderTarget` (defined in `Source/GraphicsElement/GuiGraphicsElementInterfaces.h:124`):

```
class CliGraphicsRenderTarget : public IGuiGraphicsRenderTarget
{
    CliCharacterBuffer*  buffer;
    List<Rect>           clipperStack;

    // Hosted rendering support
    bool IsInHostedRendering() override;
    void StartHostedRendering() override;
    RenderTargetFailure StopHostedRendering() override;

    // Per-window rendering
    void StartRendering() override;
    RenderTargetFailure StopRendering() override;

    // Clipping
    void PushClipper(Rect clipper, ...) override;
    void PopClipper(...) override;
    Rect GetClipper() override;

    // CLI-specific: access to the character buffer for renderers
    CliCharacterBuffer* GetBuffer();
};
```

`StartRendering()` clears the buffer. `StopRendering()` calls `ICliPlatform::FlushBuffer()` to output the buffer to the terminal. The clipper stack constrains rendering to rectangular regions, which renderers must respect when writing to the buffer.

**`CliGraphicsResourceManager`** extends `GuiGraphicsResourceManager` (defined in `Source/GraphicsElement/GuiGraphicsResourceManager.h:35`):

```
class CliGraphicsResourceManager : public GuiGraphicsResourceManager
{
    CliGraphicsRenderTarget  renderTarget;

    void Initialize();   // Register all CLI element renderers
    void Finalize();

    IGuiGraphicsRenderTarget* GetRenderTarget(INativeWindow* window) override;
    void RecreateRenderTarget(INativeWindow* window) override;
    void ResizeRenderTarget(INativeWindow* window) override;
    IGuiGraphicsLayoutProvider* GetLayoutProvider() override;
};
```

`Initialize()` calls `Register()` for each CLI renderer, following the same pattern as `GuiRemoteGraphicsResourceManager::Initialize()` (`Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp:456`).

### Implement the setup entry point

Following the pattern from `SetupRemoteNativeController` (`Source/PlatformProviders/Remote/GuiRemoteControllerSetup.cpp:14`):

```
extern void GuiApplicationMain();

int SetupCliRenderer()
{
    // Create platform-specific implementation
    #ifdef _WIN32
    WindowsCliPlatform platform;
    #else
    PosixCliPlatform platform;
    #endif

    // Create CLI native controller
    CliNativeController cliController(&platform);
    // Wrap with hosted mode (all windows rendered in one terminal)
    GuiHostedController hostedController(&cliController);

    // Create resource managers
    CliGraphicsResourceManager cliResourceManager(&cliController, &hostedController);
    GuiHostedGraphicsResourceManager hostedResourceManager(&hostedController, &cliResourceManager);

    // Register globally
    SetNativeController(&hostedController);
    SetGuiGraphicsResourceManager(&hostedResourceManager);
    SetHostedApplication(hostedController.GetHostedApplication());

    // Initialize in order
    cliController.Initialize();
    cliResourceManager.Initialize();
    hostedController.Initialize();

    // Run the application
    GuiApplicationMain();

    // Finalize in reverse order
    hostedController.Finalize();
    cliResourceManager.Finalize();
    cliController.Finalize();

    // Cleanup globals
    SetHostedApplication(nullptr);
    SetGuiGraphicsResourceManager(nullptr);
    SetNativeController(nullptr);
    return 0;
}
```

The platform-specific files are:
- `CliNativeController.h` (shared interface and controller)
- `CliNativeController.cpp` (shared logic)
- `CliPlatform.Windows.cpp` (`WindowsCliPlatform` using Win32 Console API)
- `CliPlatform.Linux.cpp` (`PosixCliPlatform` using ANSI/termios)
- `CliRenderers.h` (CLI element renderer declarations)
- `CliRenderers.cpp` (CLI element renderer implementations)
- `CliCharacterBuffer.h` / `CliCharacterBuffer.cpp` (buffer and box-drawing logic)
- `CliSetup.cpp` (the `SetupCliRenderer` entry point)

### Integration testing

Testing should cover:

**Rendering correctness**: Use GacUI's existing unit test framework with the Remote Protocol test infrastructure. The CLI renderer can be tested by capturing the `CliCharacterBuffer` output after rendering and comparing it against expected character grids. Test cases should include:
- Simple rectangles with borders and backgrounds
- Overlapping borders that should produce merged intersections
- Text rendering with CJK characters (verifying 2-cell width handling)
- Color fallback behavior (force 16-color mode and verify nearest-color selection)
- Nested compositions with clipping

**Input correctness**: Verify that keyboard events are correctly translated to GacUI `VKEY` constants. Verify mouse coordinates map correctly to the character grid. Verify that text input goes through GacUI's input system and not directly to the terminal.

**Resize handling**: Programmatically change the terminal size (or simulate `WINDOW_BUFFER_SIZE_EVENT` / `SIGWINCH`) and verify the UI reflows correctly.

**Platform-specific testing**: Test on Windows CMD, Windows Terminal, PowerShell, common Linux terminals (GNOME Terminal, Konsole, xterm), and macOS Terminal.app and iTerm2. Verify color capability detection and fallback behavior on each.
