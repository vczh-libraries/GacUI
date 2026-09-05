# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- Completed work in `../VlppOS/TODO_Task.md`, verified on Windows. The goal of this request is to make sure it also works on Linux/macOS.
- Breaking changes to handle after importing the new VlppOS:
  - GacUI: remove the moved declarations and keyboard macros from `Source/GuiTypes.h` and `Source/NativeWindow/GuiNativeWindow.h`. Use their single copy in VlppOS `Source/TUI/TUITypes.h`, still under `vl::presentation`. Regenerate GacUI releases, then import matching GacUI and VlppOS releases into wGac/iGac and rebuild.
  - Fill the new `WindowMouseInfo_::alt` in every mouse path, including buttons, movement, wheels, and generated input. Use Alt on Windows and wGac, and Option on iGac; keep Super/Command in `osSuper`. Update GacUI's Windows provider, wGac's `WGac/WGacNativeWindow.cpp` (including mouse-enter input), and iGac's `Mac/NativeWindow/OSX/CocoaWindow.mm`.
  - GacUI: carry `alt` through mouse-event copies, hosted mode, automation, reflection, and remote serialization. Regenerate protocol code and reflection metadata. Update remote cores and renderers together so their mouse messages agree.
  - Shared input fields will have defaults: flags are false, coordinates/wheel/character are zero, and key code is `VKEY::KEY_UNKNOWN` (-1). Check aggregate initializers after adding `alt`, and use normal initialization instead of clearing event structs with zero bytes.
  - `VKEY::KEY_LEFT_BRACKET` will mean `[` (0xDB), and `KEY_RIGHT_BRACKET` will mean `]` (0xDD). Check key mappings, names, shortcuts, and metadata in all three repos. Fix iGac's reversed bracket/brace tables in `Mac/NativeWindow/OSX/ServicesImpl/CocoaInputService.mm`. Keep the raw OEM key values unchanged.
  - Old TUI input types will be removed. Update any TUI callbacks and callers: `TuiMouseButton` to `NativeMouseButton`, `TuiMouseInfo` to `WindowMouseInfo`, `TuiKeyInfo` to `NativeWindowKeyInfo`, and `TuiCharInfo` to `NativeWindowCharInfo` in `vl::presentation`.
- Verification is done following these documents.
  - `VlppOS/.github/Jobs/DebugTuiPlaygroundSOP.md`
  - `GacUI/.github/Jobs/job.rpXPlat.prompt.md`
  - Based on what OS you are on:
    - On Linux, you are working on VlppOS, GacUI and wGac.
    - On macOS, you are working on VlppOS, GacUI and iGac.
  - VlppOS and GacUI are already verified on windows, but if you find any issue in the current OS during verification, fix them.
  - wGac/iGac is updated but never verified, you are going to take care of any issue in them.
- commit and push once finishing.

# TEST

- Build and run VlppOS UnitTest and TuiPlayground on macOS; exercise the production terminal according to its SOP, including input, Unicode, history, shape previews, resizing, and terminal restoration.
- Regenerate VlppOS/GacUI releases, import matching dependencies into GacUI/iGac, synchronize the three generated iGac applications, and build all native targets and portable Core/host projects.
- Run GacUI unit tests and the complete available macOS native and Playwright WebKit matrices, including independent Option/Command mouse modifiers, bracket/brace typing, renderer replacement, shutdown, and fatal errors.
- Save actual execution progress in iGac's two test cards and record failures/fixes as observed. Windows and Linux are outside this macOS run.

# PROPOSALS

- No.1 Verify the existing shared-input implementation on macOS and fix observed incompatibilities.

## No.1 Verify the existing shared-input implementation on macOS and fix observed incompatibilities

### CODE CHANGE

Regenerated the VlppOS and GacUI releases and refreshed iGac imports and application mirrors. VlppOS passed all 14 files and 263 cases. iGac, portable Core/host, and GacJS import/codegen/build/package tests pass. The repeated GacJS code generation has no semantic diff. Generated line-ending-only changes were excluded from the final changes.

The GacUI test process initially exited zero, but its text output stopped at the Unicode arrow in `Text→Inline: Same range, type changed`; subsequent console color escapes continued through process exit. The Unix test entry point did not configure the wide output stream's locale, so macOS wide console output used its initial C conversion facet and entered a failed stream state. Selecting LC_CTYPE alone did not change the stream's facet. Added environment-locale initialization to `std::wcout` before running tests and documented the installed UTF-8 locale requirement. Under `en_US.UTF-8`, the focused document-management file passed 114/114 cases and the full suite passed 88/88 files and 1729/1729 cases, with complete Unicode output and the final summary visible.

The terminal restoration probe differs only in macOS's kernel-managed PENDIN bit. A plain Python termios raw/restore round trip reproduces the same bit, including after setting the original attributes a second time. This is a verification detail, not a TUI-owned mode change; compare saved termios with that transient bit masked and separately verify original-screen/cursor/input restoration.

Production TuiPlayground runs used a macOS PTY and xterm.js/Unicode 11 in WebKit, with Menlo 14 and `en_US.UTF-8`. All ten shape styles in all four drag directions matched typed commands cell-for-cell. Unicode editing, history, modal isolation, previews/cancellation, clipping, resizing, replay and fresh-session exit passed; the normal screen and sentinel returned and all application-owned termios settings matched. The VlppOS SOP contains the detailed macOS record.

All five native macOS matrix rows passed. Coverage includes live Cocoa Option/Command input separately from renderer automation, all five mouse buttons and modifier combinations, physical bracket/brace typing, shortcuts, grid/list/editor/document workflows, renderer replacement/takeover, normal closure, the exact RPT fatal message, and both RVM host-loss timings. Standalone host loss terminated from an unhandled `RpcInjectedException`. Stdio blocked-call samples showed `PopReceivedMessage`, and EOF released the caller promptly; normal stdio shutdown reaped the owned child.

All seven Playwright WebKit matrix rows passed, including the five RVM host modes (native network, native stdio, browser, Node network and Node SEA stdio). Every host mode passed normal RPC, renderer replacement, normal shutdown and both host-loss timings; non-CLI modes also rejected a second host. Each fatal browser run received exactly one Core-authored `!Error` with the expected message, displayed the fatal mask, and produced only the matching deliberate page error. Both auto-launched host modes were reaped after exact Core `!Exit` on normal shutdown. The completed card is `iGac/TestMatrix_GacJS.md`.

Browser mouse payloads were inspected on the wire, including independent Alt/Super and `0xDB`/`0xDD` with Shift for braces. WebKit Mouse4/Mouse5 use cancelable events at the visible DOM boundary because its Playwright mouse API exposes only three buttons; native Cocoa extended-button input was tested separately. Temporary automation synchronization and proxy details are recorded in `Learning_RpJob.md`, not treated as framework fixes. No additional framework input change was needed on macOS.
