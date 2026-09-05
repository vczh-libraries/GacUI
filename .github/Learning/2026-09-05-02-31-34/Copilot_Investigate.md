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

- Build and run VlppOS UnitTest and TuiPlayground on Linux, then exercise the production terminal according to its SOP, including special keys, mouse, wheel, Unicode, resizing, and terminal restoration.
- Regenerate the upstream releases, refresh wGac imports and mirrored applications, and build the portable GacUI projects and all wGac applications.
- Run the GacUI unit tests and the complete available Linux native/GacJS matrices in wGac's two test cards, observing renderer input and the exact results required by the shared SOP.
- Record actual execution and failures as they occur. macOS is outside this Linux run.

# PROPOSALS

- No.1 Verify the existing shared-input implementation on Linux and fix observed incompatibilities.

## No.1 Verify the existing shared-input implementation on Linux and fix observed incompatibilities

### CODE CHANGE

Preserved the existing wGac Alt/bracket changes and regenerated VlppOS/GacUI releases. Refreshed wGac imports and generated all three application mirrors with `syncProj.sh`; all wGac targets and portable Core/host build successfully.

The Linux VlppOS build regenerated the tracked `makefile` and `vmake.txt` inventories to include the new `TUI.Input.cpp` translation unit. Its 14 test files and 263 cases pass. The production VTE 0.76 terminal run covers Unicode and bracket typing, Tab/Shift-Tab, HELP/errors, all ten shapes in all four drag directions against typed replay, history and both wheel axes, resizing, repeated starts, and zero-exit terminal restoration including the original sentinel and unchanged termios.

Corrected the playground SOP/specification's stale menu-wrapping wording to match the original task, implementation, and existing tests: Up/Down clamp at the first/last shape.

GacJS import, code generation, build and package tests pass on Linux. A second generation is semantically unchanged; normalized the copied snapshot line endings back to their committed LF form. The complete live Firefox matrix passes, including both application shortcuts, grid/list operations, bracket and brace typing, all five buttons and double clicks, both wheel axes with independent Alt/Super, replacement/takeover, normal close, and exact Core-authored fatal messages. All five RVM host modes passed normal ownership/shutdown and both host-loss timings.

The native Wayland matrix exposed an early-automation startup failure. Immediate renderer `/Dom` polling threw while serializing the uninitialized cached `WindowSizingConfig::sizeState`. GDB identified the generated enum serializer via `AutomationServiceRenderer::DumpDomTreeInternal`. Initialize the full cached configuration from the native window during `GuiRemoteRendererSingle::RegisterMainWindow`, before installing callbacks or exposing automation. A deterministic regression in `TestAutomationService.cpp` seeds an invalid stale size state, registers the window, and compares the complete pre-frame DOM dump with its native sizing configuration. It failed before the fix and passes afterward (8/8 focused automation cases). Regenerated GacUI release files and wGac imports; rebuilt all wGac targets. Immediate native startup polling and retained-fatal checks pass. The final full unit run and remaining native matrix rows are in progress.

All five native matrix rows subsequently passed: RPT/FCT feature workflows, replacement/takeover and clean shutdown; standalone RVM and Core RVM with network/stdio hosts; host ownership, child cleanup, and both fatal timings. The full suite after the startup fix passed 88 files and 1,727 cases.

The additional standard local FullControlTest exposed a second shared automation defect after closing a shortcut dialog. `AutomationService::DumpControlTreeInternal` used control-level `GetVisible()` for subwindows, which can remain true after the native window is destroyed and before deferred C++ deletion. Changed the filter to `GetOpening()`, which checks actual native visibility and handles a missing native window. Preserved wGac's strict native-window pointer validation. A deterministic multiwindow regression covers hidden, open, hidden-again, and native-destroyed windows. It failed before the visibility fix and passes afterward; all 10 focused automation cases pass.

Final verification completed: all 88 GacUI test files and 1,729 cases passed; the portable Core and every wGac target rebuilt successfully against the regenerated releases. Fresh standard and hosted local FullControlTest runs both passed the full input/editor/list workflow, both shortcut dialogs, and clean `self.Close()` shutdown. The two wGac test cards record all five native and seven GacJS Linux rows and their coverage. Native input was injected through renderer MiniHTTP automation, not physical hardware; Windows-only global hotkeys and macOS are outside this Linux run.

Removed only test-generated file-dialog snapshot ordering churn from the working tree; no snapshot expectations were changed by these fixes. GacJS has no semantic source changes to commit. The extended production-terminal checks, including widths 1 through 4, lost-release cancellation, mixed-color/wide-character overlaps, modal isolation, and terminal restoration, are recorded in VlppOS's SOP.
