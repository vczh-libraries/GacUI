# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

in CppTest\_Tui project, after changing theme, label text becomes black and visibly disappeared from the background (but disabled label text still gray so it is visible). Find out the root cause and fix it, hopefully there is just color issues not framework issues.

# TEST [CONFIRMED]

Add regressions at the label template replacement boundary. Check the control's TextColor, the installed template's TextColor and the rendered label color before and after refreshing an unchanged default color, changing the default color, and preserving an explicit override. Exercise actual TuiSkin presets with enabled and disabled labels, including re-enabling after refresh. Compare the GUI label path and run the repository UnitTest wrapper; require passing assertions and no memory leak dump.

- Added a DarkSkin regression in `Test/GacUISrc/UnitTest/TestControls_CoreApplication_GuiLabel.cpp`, covering repeated application refresh, changed and unchanged defaults during explicit template replacement, custom color retention, and returning to the theme default.
- Extended the existing hosted TUI regression in `Test/GacUISrc/UnitTest/TestTuiProvider.cpp` with enabled, disabled and explicitly colored labels. It checks actual terminal glyphs and foreground/background RGB in the injected backend, as well as control/template/element colors, for the initial palette and all six presets, followed by re-enabling the disabled label.
- Before the fix, the Debug x64 solution built with zero warnings/errors. Separate CDB runs with `/C /F:TestControls_CoreApplication_GuiLabel.cpp` and `/C /F:TestTuiProvider.cpp` failed at the replacement template's TextColor assertion after their initial checks passed. The control's own TextColor remained correct. This reproduces the same defect in GUI and TUI themes.

# PROPOSALS

- No.1 Always synchronize the installed label template's text color [CONFIRMED]

## No.1 Always synchronize the installed label template's text color

`GuiLabel::AfterControlTemplateInstalled_` in `Source/Application/Controls/GuiLabelControls.cpp` uses the public `SetTextColor` setter to adopt the new default. The setter skips its work if the control already stores that color. A freshly constructed `GuiLabelTemplate`, however, initializes its own TextColor to black. When the old and new defaults match, the control remains correct but its new template never receives that value.

Every TuiSkin preset uses LabelText `#808080` on ControlBackground `#000000`, so each preset change reaches this exact path. The enabled SolidLabel binds to the template's TextColor and becomes black; the disabled binding uses ControlBorderDisabled `#404040` independently, explaining why disabled labels remain visible. DarkSkin also reproduces it when its default `#F1F1F1` stays unchanged. The palette definitions and terminal rendering are correct; the defect is limited to label color synchronization during template installation.

### CODE CHANGE

In `GuiLabel::AfterControlTemplateInstalled_`, adopt DefaultTextColor directly into the control's stored textColor when initializing or when the old color followed the old default. Then always set the newly installed template's TextColor to the chosen stored value. Retain the existing rule for explicit overrides and the public setter's behavior. No interface, reflection, skin resource or generated-code changes are needed.

### CONFIRMED

The installed template now receives the chosen color on every installation, even when the control's value is unchanged. Both regressions that failed before the fix now pass. Enabled TUI labels render `#808080` on black through all six palettes; disabled labels retain `#404040`, re-enabled labels return to `#808080`, and explicit colors survive refresh. The GUI regression confirms unchanged defaults, changed defaults, explicit replacement and returning to the theme default. This fixes the shared label synchronization defect without changing palettes or renderer behavior.

- The fixed Debug x64 solution builds with zero warnings/errors using `copilotBuild.ps1`.
- The complete Debug x64 `UnitTest` wrapper run passes **90/90 files and 1,756/1,756 cases**, with no assertion failures or memory-leak dump in the finished `Execute.log`.
- Existing tracked snapshots are unchanged. The new GUI regression produces one `Controls/CoreApplication/GuiLabel/RefreshThemes/frame_0.json` snapshot.
- Code review and `git diff --check` pass. No interface or reflection changes require metadata/resource regeneration.

A rebuilt `CppTest_Tui` launched through the interactive repository wrapper in an 80-column Windows pseudoterminal passed keyboard navigation to Window Manager and palette selection for Pink, Orange, Grass, Emerald, SkyBlue and Purple. Captured VT output retained `#808080` label text on `#000000` while the selected radio and accent RGB changed as expected. Returning to List/TextList retained the gray explanatory label after refreshing it while hidden. The Exit page's queued Hide action returned normally with exit code 0 and restored the terminal screen/cursor/mouse-reporting mode. These are executable input/output checks, not physical keyboard or displayed Windows Terminal checks.
