# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

investigate repro in [MessageBox.xml](<GacUI repo>/Source/Utilities/FakeServices/TuiDialogs/MessageBox.xml) , I would like you to delete the `CreateMessageLabelTemplate` function, instead implement the control template using another `<Instance/>` in the same file. Therefore you can specify the type name directly to `ControlTemplate` instead of using `-eval` binding. Perform minimum limited test to make sure the new control template doesn't change the behavior.

# UPDATES

## UPDATE

once finishing also port the change to Release demo's tutorial and run `GacBuild.ps1` to update generated files.

# TEST [CONFIRMED]

- Reuse the existing `TUI message dialogs wrap long text inside the viewport` regression in `<GacUI repo>/Test/GacUISrc/UnitTest/TestTuiProvider.cpp`. It checks multiline text, a blank line, wrapping, viewport bounds at 80x25 and 120x40, and the Cancel result with the second default button.
- Run only the `TestTuiProvider.cpp` unit-test filter after regeneration. Check the label still uses the installed theme's default color and the dialog font.
- Regenerate the affected resources and required reflection metadata; check generator errors and build both required architectures. Run the Release tutorial's GacBuild driver and inspect its output.

# PROPOSALS

- No.1 Give the wrapping label a named template instance [CONFIRMED]

## No.1 Give the wrapping label a named template instance

The old factory enabled `WrapLine` and `WrapLineHeightCalculation` on the active theme's label so long messages did not expand beyond the terminal viewport. Preserve that themed label inside a reusable label template instance, forwarding text, font, enabled state and text color. The instance constructor assigns the current theme's label factory explicitly, preserving its wrapping settings across theme refreshes as before, then enables wrapping on the label. The message box selects the instance class directly through `ControlTemplate`.

This keeps TUI dialog resources independent of a particular skin implementation and preserves custom label palettes. The Release tutorial uses the library's built-in fake TUI dialogs rather than a separate MessageBox XML, so its library snapshot must receive the generated dialog implementation.

### CODE CHANGE

- Add `tui_controls::TuiMessageBoxLabelTemplate` to the existing MessageBox XML; remove the window's factory and use the template type name directly.
- Preserve the existing focused regression and add color/font checks there.
- Regenerate dialog C++ and metadata, update the Release tutorial's library dependency, and run its GacBuild driver.

### CONFIRMED

- Generated both architectures of TUI dialog resources with `GacUI_Compiler`; no resource error files remain. Temporarily selected only `BUILD_TUIDIALOGS` for this focused generation, then restored the compiler's original source before the final builds. Regenerated the merged dialog sources and packed the four affected library files with CodePack.
- Debug Win32 and x64 solution builds passed with zero warnings and errors. Regenerated metadata for both architectures; `Metadata_Test` passed its core/full metadata round trips for both architectures.
- Debug x64 `UnitTest` with `/F:TestTuiProvider.cpp` passed 1/1 test files and 21/21 cases, with no memory-leak report. The existing message-dialog regression passed at 80x25 and 120x40, including the new installed-palette text-color and effective-font assertions, multiline wrapping, blank lines, viewport bounds and Cancel result.
- Copied the generated GacUI/GacUIReflection pairs and current reflection metadata into `<Release repo>/Import` and `<Release repo>/Tools`. The tutorial already uses these built-in dialogs, and its authored TUI XML matches the development demo, so no duplicate dialog XML was added.
- Ran `<Release repo>/Tools/GacBuild.ps1 -FileName <Release repo>/Tutorial/GacUI.xml` successfully. Refreshed the TUI tutorial resource timestamp to select its x32/x64 regeneration; unrelated tutorial resources were skipped. Both staged architectures completed and the merged C++ and data resource were deployed. Their contents remain identical, so the Release diff contains only the four library files and two metadata binaries.
- Final source review and `git diff --check` passed in both repositories. The message box contains a direct `ControlTemplate="tui_controls::TuiMessageBoxLabelTemplate"` attribute and no `CreateMessageLabelTemplate` function.

Build and execution evidence is retained in the ignored `<GacUI repo>/Test/GacUISrc/x64/Debug/RpWindows/MessageBox*.log` files.
