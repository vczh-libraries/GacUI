# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

In [TODO_Injection2.md](TODO_Injection2.md) an issue is fixed around CppTest's document style ribbon dropdown, verify if `Release\Tutorial\GacUI_Controls\DocumentEditorRibbon` has the same issue and fix it. commit and push all local changes

# TEST [CONFIRMED]

- Inspect the authored tutorial XML, its generated handler and the library actually linked by its project.
- Build `<Release repo>/Tutorial/GacUI_Controls/GacUI_Controls.sln` in Debug x64 using the repository wrapper.
- Run the standalone DocumentEditorRibbon tutorial. Use automation Controls/IO to click actual popup item captions in wide and collapsed layouts, then check effective formatting and retained text selection through UIA. Cover both groups, all eight styles, repeated application and reopening after resizing. Retain an inline application check.
- Run the existing GacUI gallery/document regression tests with the repository wrappers. Record any UIA operation limitations separately.

## Results

- Both `<Release repo>/Tutorial/GacUI_Controls/GacUI_Controls.sln` and `<GacUI repo>/Test/GacUISrc/GacUISrc.sln` built successfully in Debug x64 through their repository wrappers, each with zero warnings and zero errors.
- The standalone tutorial was launched through the Release execution wrapper from its project directory, which resolves its existing relative resource paths. Its automation endpoint exposed the expected editor, style catalog and actual separate popup window. Clicking the popup's Header 1 caption applied the named style to the selected test text in the dumped document model.
- A windowless MTA probe reused the existing `<GacUI repo>/Test/UIA_CppTest_Shared.cs` helpers in memory, without changing application or test source. It passed 405 assertions against the standalone tutorial: inline Header 1 through UIA SelectionItem, then all eight actual popup captions at window widths 1400, 650 and 1250. The 650-wide layout used the collapsed Style button. Each width included reopening and applying Header 1 again while already selected.
- The probe verified exact effective font size, foreground color, weight, italic, underline and strikethrough through UIA TextPattern, and verified that the complete selected test text remained selected after every application. Both Headers and Content groups passed.
- UIA successfully located the actual popup's eight background controls and scrolled items into view. Those controls expose Toggle without Invoke, so their actual application clicks used automation IO on the popup window. This is the existing UIA limitation documented by the showcase tests, not a failure of the tutorial's mouse-click path. Physical desktop input was not exercised.
- The existing Debug x64 unit-test selection passed all 235 cases across seven files, including gallery child-caption clicks, exactly-once application, already-selected items, selection-only synchronization, dismissal, inline clicks, theme refresh and document style/copy regressions. The completed execution log contains no memory-leak dump. Test snapshots remained unchanged.
- The tutorial had no runtime-error dialog and exited with code zero through UIA Window.Close followed by No on its save prompt, discarding only the test text.

# PROPOSALS

- No.1 Verify the already-published shared gallery fix before changing the tutorial [CONFIRMED]

## No.1 Verify the already-published shared gallery fix before changing the tutorial

The tutorial lives in the sibling Release repository. Its authored XML already handles `ItemApplied`, and its GacUILite dependency compiles `<Release repo>/Import/GacUI.cpp`. That imported implementation already accepts mouse events from popup item-template child controls, dispatches each valid `ApplyItem` once even for an already-selected item, and preserves selection-only synchronization. These are the fixes described in `<GacUI repo>/TODO_Injection2.md` and the gallery learning guidance.

### CODE CHANGE

No application source change was needed. Keep this investigation in the GacUI repository as the task's primary record; the Release repository remained clean.

### CONFIRMED

The original dropdown bug does not reproduce in the current standalone tutorial. Release commit `056ff4fe` already published the shared gallery fix in its imported library. The tutorial's authored and generated handlers already apply document styles from `ItemApplied`, so rebuilding the tutorial picks up the fix without changing its XML or regenerating its resources. Direct automation and the standalone UIA formatting checks confirmed the behavior in wide and collapsed ribbons, while the existing unit tests confirmed the shared event contract and theme-refresh behavior.
