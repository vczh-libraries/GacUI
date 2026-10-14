# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Perform the change you suggested, rerun unit test and see if snapshots are updated expectedly. Commit and push once finishing.

# TEST [CONFIRMED]

- The existing `Initial file names survive asynchronous folder loading` test in `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` covers the initial filename before and after asynchronous loading, and after cancellation, for both open and save dialogs.
- The preceding source and snapshot audit confirmed that the open case constructs its filename before installing `FileSystemMock`. The committed `InitialName_Open/frame_1.json` and `frame_2.json` under `<GacUI repo>/Test/Resources/UnitTestSnapshots/Application/Dialog_File` contain a host working-directory prefix.
- Build the Debug x64 solution using `<GacUI repo>/.github/Scripts/copilotBuild.ps1`, then run the existing UnitTest project using `<GacUI repo>/.github/Scripts/copilotExecute.ps1`. No project user filter is configured, so the complete suite will run.
- Require all test files and cases to pass and no memory leak report after the test summary.
- Inspect every generated snapshot change. The open-dialog filename should become the deterministic mock value `/A/a.txt`; the save-dialog filename should remain `new.txt`. Search the entire snapshot tree for remaining checkout paths.

# PROPOSALS

- No.1 Normalize the initial filename after filesystem mock installation [CONFIRMED]

## No.1 Normalize the initial filename after filesystem mock installation

Move the `initialFileName` declaration into the GUI-main proxy, before registering its frame callbacks. `GacUIUnitTest_StartFast_WithResourceAsText` invokes the installer before this proxy, so `FilePath` uses `FileSystemMock`. Existing callbacks capture the resulting string by value and retain it safely through asynchronous folder loading.

### CODE CHANGE

- Moved the filename calculation in `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` into the registered GUI-main proxy.
- Regenerated snapshots through UnitTest. The four changed artifacts are `<GacUI repo>/Test/Resources/UnitTestSnapshots/Application/Dialog_File/InitialName_Open/frame_1.json`, `<GacUI repo>/Test/Resources/UnitTestSnapshots/Application/Dialog_File/InitialName_Open/frame_2.json`, `<GacUI repo>/Test/Resources/UnitTestSnapshots/Application/Dialog_File/InitialName_Open[commands].txt`, and `<GacUI repo>/Test/Resources/UnitTestSnapshots/Application/Dialog_File/InitialName_Open[diffs].txt`.
- Source and investigation notes are committed separately from generated snapshot changes.

### CONFIRMED

- The Debug x64 solution build completed with zero warnings and zero errors.
- The complete UnitTest run passed 93/93 test files and 1820/1820 test cases. The completed `<GacUI repo>/.github/Scripts/Execute.log` contains no skipped tests or memory leak report.
- Both open-dialog frames now contain `/A/a.txt`. The text run ends at character 8 instead of 49, the clicked caret ends at character 8, and the shorter text no longer needs the former horizontal clipping node. The rendering command and DOM diff changes are confined to that filename element and its clipping structure.
- All save-dialog snapshots remain unchanged. A search across all 8111 snapshot files found no remaining checkout names or Windows drive-qualified paths.
- The full suite also moved the date-picker highlight from October 13 to October 14 in `<GacUI repo>/Test/Resources/UnitTestSnapshots/Application/RefreshThemes/OverridesAndWindows/frame_0.json` and `<GacUI repo>/Test/Resources/UnitTestSnapshots/Application/RefreshThemes/OverridesAndWindows/frame_1.json`. The owning test creates a `GuiDatePicker` without a date mock, and its constructor uses `DateTime::LocalTime()`. Restored these unrelated date-dependent changes; only the four filename artifacts remain changed.
- Reviewed the one-line source move and every retained snapshot change. The mock is installed before the proxy executes, and the frame callbacks capture the local filename by value.
