# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

investigate repro in [MessageBox.xml](<GacUI repo>/Source/Utilities/FakeServices/TuiDialogs/MessageBox.xml) , I would like you to delete the `CreateMessageLabelTemplate` function, instead implement the control template using another `<Instance/>` in the same file. Therefore you can specify the type name directly to `ControlTemplate` instead of using `-eval` binding. Perform minimum limited test to make sure the new control template doesn't change the behavior.

# UPDATES

## UPDATE

once finishing also port the change to Release demo's tutorial and run `GacBuild.ps1` to update generated files.

## UPDATE

Using `<Label/>` in `<LabelTemplate/>` is not correct, the correct way is to use `<SolidLabel/>` with TUI theme colors. Although this makes you having to copy some TUI label template code but this is expected. Rework `tui_controls::TuiMessageBoxLabelTemplate`. Perform minimum limited as the original request required.

# TEST [CONFIRMED]

- Reuse the existing `TUI message dialogs wrap long text inside the viewport` regression in `<GacUI repo>/Test/GacUISrc/UnitTest/TestTuiProvider.cpp`. It checks multiline text, a blank line, wrapping, viewport bounds at 80x25 and 120x40, and the Cancel result with the second default button.
- Run only the `TestTuiProvider.cpp` unit-test filter after regeneration. Check the label still uses the installed theme's default color and the dialog font.
- Regenerate the affected resources and required reflection metadata; check generator errors and build both required architectures. Run the Release tutorial's GacBuild driver and inspect its output.

# PROPOSALS

- No.1 Give the wrapping label a named template instance [DENIED]
- No.2 Render the message directly with SolidLabel [CONFIRMED]

## No.1 Give the wrapping label a named template instance

The old factory enabled `WrapLine` and `WrapLineHeightCalculation` on the active theme's label so long messages did not expand beyond the terminal viewport. Preserve that themed label inside a reusable label template instance, forwarding text, font, enabled state and text color. The instance constructor assigns the current theme's label factory explicitly, preserving its wrapping settings across theme refreshes as before, then enables wrapping on the label. The message box selects the instance class directly through `ControlTemplate`.

This keeps TUI dialog resources independent of a particular skin implementation and preserves custom label palettes. The Release tutorial uses the library's built-in fake TUI dialogs rather than a separate MessageBox XML, so its library snapshot must receive the generated dialog implementation.

### CODE CHANGE

- Add `tui_controls::TuiMessageBoxLabelTemplate` to the existing MessageBox XML; remove the window's factory and use the template type name directly.
- Preserve the existing focused regression and add color/font checks there.
- Regenerate dialog C++ and metadata, update the Release tutorial's library dependency, and run its GacBuild driver.

### DENIED BY USER

The original implementation passed the limited behavioral tests below, but incorrectly nested a themed `Label` control inside `LabelTemplate`. The user requires a directly owned `SolidLabel` and explicitly accepts copying the TUI label template's rendering logic and colors. Proposal No.2 replaces this implementation.

- Generated both architectures of TUI dialog resources with `GacUI_Compiler`; no resource error files remain. Temporarily selected only `BUILD_TUIDIALOGS` for this focused generation, then restored the compiler's original source before the final builds. Regenerated the merged dialog sources and packed the four affected library files with CodePack.
- Debug Win32 and x64 solution builds passed with zero warnings and errors. Regenerated metadata for both architectures; `Metadata_Test` passed its core/full metadata round trips for both architectures.
- Debug x64 `UnitTest` with `/F:TestTuiProvider.cpp` passed 1/1 test files and 21/21 cases. The existing message-dialog regression passed at 80x25 and 120x40, including the new installed-palette text-color and effective-font assertions, multiline wrapping, blank lines, viewport bounds and Cancel result. The earlier completion report missed the deferred dialog-deletion leak in the debug log; No.2 corrects the regression's shutdown and verifies the complete log.
- Copied the generated GacUI/GacUIReflection pairs and current reflection metadata into `<Release repo>/Import` and `<Release repo>/Tools`. The tutorial already uses these built-in dialogs, and its authored TUI XML matches the development demo, so no duplicate dialog XML was added.
- Ran `<Release repo>/Tools/GacBuild.ps1 -FileName <Release repo>/Tutorial/GacUI.xml` successfully. Refreshed the TUI tutorial resource timestamp to select its x32/x64 regeneration; unrelated tutorial resources were skipped. Both staged architectures completed and the merged C++ and data resource were deployed. Their contents remain identical, so the Release diff contains only the four library files and two metadata binaries.
- Final source review and `git diff --check` passed in both repositories. The message box contains a direct `ControlTemplate="tui_controls::TuiMessageBoxLabelTemplate"` attribute and no `CreateMessageLabelTemplate` function.

Build and execution evidence is retained in the ignored `<GacUI repo>/Test/GacUISrc/x64/Debug/RpWindows/MessageBox*.log` files.

## No.2 Render the message directly with SolidLabel

Copy the TUI label template's text/font/enabled-color bindings into the named message label template. Use the built-in TUI palettes' shared neutral colors: `LabelText` is `#808080` and `ControlBorderDisabled` is `#404040`, as defined in `<GacUI repo>/Test/Resources/App/TuiSkin/TuiSkin.xml`. Set `Multiline`, `WrapLine` and `WrapLineHeightCalculation` declaratively on `SolidLabel`. Remove the nested control, factory assignment and template constructor script. The message box continues selecting the template by its type name.

### CODE CHANGE

- Simplify `<GacUI repo>/Source/Utilities/FakeServices/TuiDialogs/MessageBox.xml` to a `LabelTemplate` owning one `SolidLabel`.
- Extend the same existing message-dialog regression with rendered text/font and enabled/disabled-color checks. Keep verification limited to `/F:TestTuiProvider.cpp`, required builds/metadata generation, and the already requested Release port and tutorial generation.
- Regenerate affected dialog sources, library release files and metadata; update the Release tutorial's shared dependencies and rerun its GacBuild driver.
- The first focused run passed all 21 cases but the debug log reported leaked message-dialog objects. `ShowModalAndDelete` queues native-window destruction and then object deletion, while the regression immediately closed its main window after `ShowMessageBox` returned. Keep pumping the existing test application's event loop until only the main window remains, then close it, so the service's normal asynchronous deletion finishes. This is a correction to the existing regression's shutdown sequence; no production lifecycle change is needed.

### CONFIRMED

- The message label template owns one `SolidLabel`, with TUI neutral colors, text/font/enabled-color bindings and all three multiline/wrapping flags. There is no nested `Label`, constructor script or template factory in this instance. The message box still references its type name directly.
- Focused TUI-dialog generation passed for x86/x64. The temporary compiler selection was restored before the final builds; no compiler source change remains. Debug Win32 and x64 builds passed with zero warnings/errors. Both metadata generators and the core/full x86/x64 metadata round-trip check passed.
- After correcting the regression's shutdown, `/F:TestTuiProvider.cpp` passed 1/1 files and 21/21 cases. The complete debug log ends with the passing summary and contains no memory-leak report. Existing 80x25/120x40 wrapping, multiline text, blank lines, viewport and Cancel checks passed along with rendered text/font, disabled-color and explicit text-color checks.
- CodePack regenerated the affected library files; their Release tutorial dependency copies and both reflection binaries were updated. `<Release repo>/Tools/GacBuild.ps1 -FileName <Release repo>/Tutorial/GacUI.xml` succeeded, rebuilding only the TUI tutorial in both architectures and merging/deploying its outputs. Tutorial output contents remain unchanged; the Release diff is the three affected library files and two metadata binaries.
- Source review and `git diff --check` passed in both repositories. Verification remained limited to the existing TUI provider test file and required generation/build checks.

Evidence is retained in the ignored `<GacUI repo>/Test/GacUISrc/x64/Debug/RpWindows/SolidLabel*.log` files, including the complete final debug log `SolidLabelFinalExecute.log` and initial leak report `SolidLabelLeak.log`.
