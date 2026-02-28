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

## Instructions

You goal is to make a detailed plan to implement `# Goal`.
When API or third-party libraries is introduced, the plan must be detailed enough to mention how to use those functions, with supported materials in links.
Think througly as CLI handling is complex, especially when talking to the OS.
In `## Plan`, list each item in bullet-point list, with supported materials in links if applicable.
In `## Details`, each bullet-point item would be an `### Item`, providing detailed explanation.
When involving API or third-party libraries, it should be detailed enough so that even one without knowledge with them could implement the work immediately.
In the end, call `CheckLinks.ps1` to verify all linkes are available.

## Plan

## Details
