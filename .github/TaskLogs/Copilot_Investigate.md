# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- `Test\GacUISrc\Generated_TuiSkin\TuiSkinConfig.(h|cpp)` should not exist, content should be moved to `Source\Skins\TuiSkin\Config\TuiSkinConfig.(h|cpp)`.
  - `Source\Skins\TuiSkin\Source\TuiSkinConfig.(h|cpp)` is the old file, they are now part of the above config, delete them.
- Rework `DarkSkin` to have color themes. Just like how `TuiSkin` implements color theme, here is the main requirement:
  - Make a `darkskin::ColorPackage` struct in darkskin XML resource.
  - Make a `darkskin::CreateDefaultColorPackage`.
  - Make a `darkskin::SetColorPackage`.
  - Current colors are defined in `Style.xml`.
  - Decide what needs to be named in `darkskin::ColorPackage`, and then replace all color literals in `Style.xml` by either `-eval` binding or referencing the global variable directly in scripts.
- I would like to keep all themes dark but changing limited colors. The current default theme is blue (general) purple (highlighted tab) green (progress bar).
  - I would like you to come out of 5 other combinations, set their name, ensure they are beautiful, you might want to find some idea of designs from the internet.
    - Since each theme are some combination of colors, using color names on them is too long, you can come out with real theme name like `DawnOnSea`.
    - `DawnOnSea` is only my example, you are not required to actually have it, it is up to you.
    - Put config in `Source\Skins\DarkSkin\Config\DarkSkinConfig.(h|cpp)`.
    - `FullControlTest` and `FakeDialogService` UI hardcoded a few other colors, and the `BaselineDocuments.xml` assume all theme are dark, so you can only have dark themes.
  - Prepare functions for them.
  - Just like `TuiControlTest`'s `Window Management` tab, put all color options to `FullControlTest`.
  - Run `CppTest` to make sure it works.

## DETAILS

### TuiSkin configuration ownership

- Move the current implementation from `Test/GacUISrc/Generated_TuiSkin`, preserving all six preset factories, `tuiskin::CreateDefaultColorPackage`, and `tuiskin::SetColorPackage`. The obsolete `Source/Skins/TuiSkin/Source` copy does not contain all six factories and must not replace the newer implementation.
- Update `Generated_TuiSkin.vcxitems`, its filters, and the includes in `CppTest_Tui/Main.cpp` and `UnitTest/TestTuiProvider.cpp`. Compile the canonical configuration implementation exactly once through the shared skin inventory for each consumer.
- Resolve the configuration header's dependencies for both development architectures and the release layout. Development consumes `Generated_TuiSkin/Source_x86` or `Source_x64`; release consumes `Source/Skins/TuiSkin/Source`. Do not accidentally include both generated definitions. Apply the same dependency discipline to the new DarkSkin configuration.
- Update the sibling `../Tools/Tools/ProjectGacUI.ps1` release workflow: its existing copy-and-rewrite loop reads the old test configuration and recreates `Source/Skins/TuiSkin/Source/TuiSkinConfig.*`. The canonical `Config` files must survive release generation, and neither obsolete pair may be recreated.
- Extend the skin categories in `Release/CodegenConfig.xml` to include each skin's `Config` folder in its own skin output. They currently match only `Source`; the generic GacUI category excludes skins. Update the configuration input and regenerate outputs through the release tools; do not hand-edit amalgamations or IncludeOnly files.
- Check release ordering as well: `ProjectGacUI.ps1` packs once before skin generation and again afterward. Newly introduced DarkSkin palette types must be available when configuration dependencies are first processed, without relying on previously refreshed generated headers.
- Update the configuration-location and ownership descriptions in `Project.md` and `.github/KnowledgeBase/KB_GacUI_Design_TuiPlatformProvider.md` when implementing the relocation.

### DarkSkin palette contract

- The authored resources for this solution are in `Test/Resources/App/DarkSkin`, including `DarkSkin.xml`, `Style.xml`, and `Index.xml`. The shorter `Test/Resources/DarkSkin` path mentioned elsewhere in `Project.md` does not exist. The `Source/Skins/DarkSkin` XML copy is maintained by the release workflow.
- Define `darkskin::ColorPackage` in the Workflow resource and keep one installed palette, following `Test/Resources/App/TuiSkin/TuiSkin.xml`. Put the public C++ configuration functions in `Source/Skins/DarkSkin/Config/DarkSkinConfig.h/.cpp`: `CreateDefaultColorPackage()`, five named `Create<Name>ColorPackage()` factories, and `SetColorPackage(const ColorPackage&)`.
- There are six choices in total: the unchanged default and five new combinations. Give the new presets concise theme names and matching factory names. Share neutral backgrounds, normal/disabled text, neutral borders, and transparency across presets; vary coordinated general accents, highlighted-tab accents, progress fills, and their related hover/pressed/selection shades. Keep each preset's concrete values in one authoritative implementation.
- Name fields by their visual roles and states. Preserve every existing default value, including alpha and subtly different blues such as `#007ACC`, `#017ACC`, `#1997EA`, and `#1C97EA`; do not merge them merely because they look similar. Cover nested colors too, including polygon fill/border, splitter colors, and text-box selection structures. All color literals in `Style.xml` must move into palette initialization.
- Keep existing state-dependent `-bind` expressions reactive to enabled, focused, hovered, pressed, selected, and submenu states; replace their literal branches with palette fields. Use `-eval` for values evaluated when templates are constructed. Replacing a state-dependent binding with `-eval` would freeze that state until another theme refresh.
- Preserve DarkSkin's existing default appearance without requiring every current caller to learn a new initialization step. The palette must be initialized before templates first read it in generated C++ applications, unit tests, and the Workflow binary host. Creating another theme or window must not reset an already selected palette.
- `SetColorPackage` installs colors for subsequent template construction. Refresh existing controls explicitly with `vl::presentation::controls::GuiApplication::RefreshThemes()` from `Source/Application/Controls/GuiApplication.h/.cpp`. Follow the lifetime contract in `.github/KnowledgeBase/KB_GacUI_Design_AddingNewControl.md`: queue palette installation and refresh together with `InvokeInMainThread` after input dispatch, capturing the selected preset by value.
- `BaselineDocuments.xml` also hardcodes a blue document-selection background outside `Style.xml`. To make editor selection follow the chosen general accent, supply palette-based baseline overrides while preserving its existing text defaults for the dark theme and all other baseline styles. Follow TuiSkin's baseline-document pattern; refresh must preserve the document model, explicit formatting, selection, and undo/redo history. Application-specific colors in FullControlTest and fake dialogs can remain fixed and must remain readable with every dark preset.

### FullControlTest integration

- Add a compact, labeled group of six mutually exclusive radio options to the Window Management page in `Test/Resources/App/FullControlTest/Resource.xml`, using TuiControlTest's layout and selection behavior as the reference. Select the default initially, process only newly selected options, and preserve the selection through refresh without emitting another selection event.
- TuiControlTest currently raises a `PaletteSelected(int)` event that is handled in `CppTest_Tui/Main.cpp`. FullControlTest is shared by `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`, `RemotingTest_Core /FCT`, and the Workflow-binary `GacUI_Host`; installing a handler only in `CppTest/Main.cpp` would leave the other selectors inert. Provide a shared integration path, including the interpreted resource path, without duplicating preset color values or editing generated code by hand.
- Palette changes apply to all existing default-themed windows and to windows created afterward. Explicitly assigned custom templates and application-owned `Color-eval` elements retain their captured values under the existing refresh contract; keep their fixed colors compatible with the shared dark neutrals.
- Include the new configuration in the appropriate shared skin inventory and release output. Regenerate resource C++, reflection, and binary outputs with the existing tools. If exposing native configuration to Workflow affects reflection, follow the registration and metadata requirements in `Project.md`.

## VERIFICATION

These are implementation acceptance checks; this review does not execute the task.

1. Build and generate through the repository wrappers. Run `GacUI_Compiler` for the changed skin/showcase resources and inspect `git status` for `*.UI.errors.txt`. After generation, perform the required sequence: build Debug Win32, run `Metadata_Generate` Win32, build Debug x64, run `Metadata_Generate` x64, then run `Metadata_Test` x64. Inspect completed logs and generated changes. Do not patch generated files to repair errors.
2. Run `UnitTest` through `copilotExecute.ps1`, following the existing filter rules. Ensure `TestApplication_Theme.cpp` and `TestTuiProvider.cpp` are included: they already cover template overrides, multiple/hidden windows, preserved editing state, TUI presets, and repeated refresh. Require passing results and no appended memory-leak report. Compare existing DarkSkin visual snapshots with the baseline; investigate rendering changes because the default palette must remain visually unchanged. Distinguish expected compiler-output changes from visual regressions.
3. Add focused DarkSkin coverage for complete default initialization, the five distinct accent combinations with shared neutral values, and a palette switch followed by refresh of representative controls. Check an existing control, a newly created control, a state-dependent binding after another input change, and document-selection colors. Restore the default palette afterward so later tests are independent; reuse the existing theme tests for general state-preservation coverage.
4. Run `CppTest` with the documented GacUI execution/automation workflow and use the applicable startup, control-interaction, and shutdown checks from the Complete Control Showcase section of `.github/Jobs/DebugRemoteProtocolSop.md`. Select all six palettes and return to default. Inspect actual rendered colors for general focus/selection, highlighted tabs, progress bars, hovered/pressed/disabled buttons, menus, list/tree/grid selections, and editor selection. Check readability of the fixed example and fake-dialog colors, and check the palette group fits when the window is resized.
5. Before switching, enter document text and establish selection/undo history, scroll a list, and open a secondary window. Switch palettes, reopen an already-created popup, and create another window. Confirm the appropriate templates recolor while text, editing history, list position, current tab, and selected palette survive. Repeat selection to catch callbacks caused by rebuilding the selector, then close normally and inspect the process result.
6. Smoke-test palette selection in `CppTest_Metaonly`, `CppTest_Reflection`, `GacUI_Host`, and one `RemotingTest_Core /FCT` native-renderer setup. Check initial default colors and a switch back and forth in each. The binary-host check must load regenerated resource binaries; the remoting check must show the new colors at the renderer.
7. After the TuiSkin move, run its existing palette tests and exercise the six choices in `CppTest_Tui` following `.github/Jobs/DebugTuiControlTestSop.md`. Confirm unchanged default colors and switching behavior. Debug Win32/x64 solution builds must resolve the relocated configuration in every compiled reflection variant.
8. Run the supported release-generation path and check both normal and IncludeOnly skin outputs contain the configuration APIs exactly once, reference the new locations, and do not recreate either obsolete TuiSkin configuration pair. Verify downstream compilation of the generated skin/configuration outputs. Keep Linux build inputs consistent with the shared inventories and regenerate `vmake.txt`/`makefile` through the supported build script when validating there.

## REVIEW COMMENTS

# TEST [CONFIRMED]

Reproduce the missing DarkSkin palette API and selector with focused theme tests. Preserve the default snapshot colors; verify six presets, refresh, input-state bindings and document selection. Follow all eight acceptance checks from the task, including both architectures, resource and metadata generation, every showcase host, TUI and release consumers.

Reproduction evidence: the baseline Style.xml has visual constants inline, Theme has no palette API, and the requested native DarkSkin factories and shared selector are absent. The initial Debug x64 build passed with zero warnings/errors. An early descriptor assertion failed under CDB, but it was insufficient evidence: this native test harness intentionally does not register DarkSkin reflection types. The final tests exercise the native palette contract directly; metadata and interpreted/compiled showcase runs validate the reflected path.

# PROPOSALS

- No.1 Share a Workflow palette across native configuration and all showcase hosts [CONFIRMED]

## No.1 Share a Workflow palette across native configuration and all showcase hosts

Keep the palette type, pure preset constructors and installed global in the DarkSkin Workflow resource. The requested public C++ factories and setter in Source/Skins/DarkSkin/Config forward to those generated functions, so C++ and the interpreted host share exactly one set of concrete values. Initialize the global once from the default factory, before template construction; constructing another Theme never resets it. This avoids native reflection dependencies on a Workflow-defined structure and keeps the binary host independent of generated native skins.

Retain every distinct default color and alpha in role/state fields. Bind state-dependent branches to palette fields, evaluate construction-only colors, and build document baseline overrides from the installed palette. Expose static Theme palette methods to the shared FullControlTest resource. Its six-radio selector captures the chosen package and queues installation plus application refresh after input dispatch.

Move the newer TuiSkin configuration intact to Config, delete obsolete copies, and compile each config once through its shared skin inventory. Use a development-only include branch hidden from CodePack and a normal release header dependency. Include Config in each skin category. Generate release skin headers before the first pack that processes new palette dependencies; retain the supported GacGen/bootstrap order and remove the old config copying loop.

### CODE CHANGE

Implemented the 40-field Workflow ColorPackage, six preset factories, one default-initialized installed value, static Theme bridge and palette-based document overrides. Every Style.xml color now comes from a role while existing state bindings remain reactive. The C++ DarkSkin config forwards to generated functions, sharing concrete values with the binary host. FullControlTest has a six-radio group and queues installation plus RefreshThemes with value capture.

Relocated the newer six-preset TuiSkin config to canonical Config, deleted both obsolete pairs, updated the shared inventories/filters, consumer includes and Linux compilation flags. Added development/release header selection hidden from CodePack, Config categories, and a release bootstrap that generates skin types before packing. Updated Project.md and the TUI provider knowledge base.

Added default-value/neutral/preset contract coverage and six repeated refresh checks for existing/new controls, reactive button input, preserved editor model/caret/undo and palette-based selection. Existing broad theme-preservation and TUI tests remain the regression coverage. Resource generation initially exposed immutable Workflow struct and inferred-global limitations; constructors now use complete struct expressions and darkColors has an explicit ColorPackage type. Validation is in progress.
Bootstrap Win32 build exposed the expected missing generated DarkSkin include in FullControlTest and a Ptr cast in the new button-color assertion. The compiler include input now supplies the skin dependency; the test uses Ptr::Cast. A focused resource compiler build regenerates the changed resources; the final source retains all normal BUILD switches.

Release-GacUI completed successfully using the existing GacGen for bootstrap, both CodePack passes and rebuilt GacGen for final generation. Separate Debug x64 downstream static-library consumers compiled both normal and IncludeOnly DarkSkin/TuiSkin outputs plus calls to every native preset factory: zero errors, two MSB8029 warnings about the deliberately temporary output directory. IncludeOnly references each canonical Config header/implementation once; obsolete Tui config paths remain absent.

The early Win32 GacUI_Host run loaded the regenerated binary resources, exposed the exact showcase title and all six selector labels, and showed original #2D2D30/#017ACC defaults. The monitor capture API failed twice with IGraphicsCaptureItemInterop.CreateForMonitor (0x80070057), so no visual screenshot pass is claimed. A 10-second GET timeout during interpreted palette refresh was followed by abort() on HttpAutomationService::OnHttpRequestReceived; CDB showed the UI thread in its message loop and the HTTP worker in its termination handler. This run was deliberately terminated and will be retried without canceling server responses.

Win32 CppTest_Tui passed all six keyboard selections at 120x40 through the wrapper's interactive PTY: Pink D99AB7, Orange E3B080, Grass B5C98A, Emerald 88C9B0, SkyBlue 87CEFA and Purple B8A4DA. Each settled VT frame had one selected radio. Shift+Tab restored SkyBlue; the Exit page's queued Close returned exit 0 and restored the console. SGR mouse injection produced no observed effect, so these are keyboard/PTY checks, not physical-terminal mouse evidence.

CDB located the slow Debug Win32 resource pass in MergeCppFileContent/ProcessCppContent on the existing 953,000-character DarkSkin.h, rather than a compiler failure. To complete clean regeneration, the verified generated-only DarkSkin*/FullControlTest* aggregate C++ and header files for both architectures were removed; every file was checked for USERIMPL/USER_CONTENT_BEGIN first. MainWindow and DocumentEditorBase user-code files were preserved. The targeted compiler is regenerating these outputs from the authored XML.

Header review found that project-reference consumers do not inherit shared-inventory preprocessor definitions. UnitTest and CppTest_Tui now explicitly define GACUI_SKIN_DEVELOPMENT for all four configurations, so including canonical config headers selects development-generated types even when no release skin headers have been refreshed. The shared libraries still compile each config implementation exactly once.

The second Win32 GacUI_Host run loaded freshly regenerated binaries and passed Default -> Aurora -> Default. Live compositions changed #007ACC/#017ACC to #168A7A/#178A7A and back; each settled state had exactly the matching radio selected. Long-running control reads completed without canceling the HTTP response. The six labels fit within the 640-wide window.

The final targeted Debug x64 GacUI_Compiler run completed with exit 0, regenerated both architectures and merged the dialog output. No UI error files or missing generated DarkSkin/FullControlTest outputs remain. The required final build/metadata/test sequence now starts from these regenerated files. The successful binary host closed through queued Close with exit 0. Win32 TUI also passed all six choices at 80x25 and restored SkyBlue before normal exit 0.

Final Debug Win32 solution build passed with 0 warnings / 0 errors (3m16s), followed by Metadata_Generate Win32 exit 0. The Debug x64 build is next in the required sequence.

Debug x64 then passed with 0 warnings / 0 errors (3m38s); Metadata_Generate x64 and Metadata_Test x64 both exited 0. Metadata_Test compared both architectures' full and core type dumps successfully. The first full UnitTest run stopped at the new reflection assertion: its native-control harness intentionally does not call LoadDarkSkinTypes. Remove that harness-inappropriate assertion and retain the native palette contract; reflection registration is covered by the successful metadata round trip and compiled/interpreted showcase paths. The original missing-descriptor reproduction was therefore insufficient by itself; the absent palette declarations and factories in the baseline are the decisive API evidence.

CDB traced the next contract-test failure to RendererEndRendering before the first idle frame. The newly added bare window omitted ClientSize; give it an explicit 400x300 client area, matching the existing native theme tests. The 40 default-value and six-factory assertions had completed before rendering reached this harness error.

With that corrected, the complete palette contract passed. The refresh test also verified the existing document's selection style, model, caret and undo state, then found its new standalone control had not yet constructed its lazy template. Materialize that template with GetControlTemplateObject before checking its baseline selection style.

The full Debug x64 suite passed 90/90 files and 1758/1758 cases with no appended leak report. A final live scroll check then exposed a separate refresh defect: a 60-item TextList scrolled to y=192 returned to zero after palette refresh, while tab navigation alone preserved its position. CDB stopped at GuiScrollView::AdjustView: the viewport was 642 high, but the temporary total height was only 240 instead of 1200. The caller was GuiListControl::OnRenderTargetChanged with renderTarget=nullptr, rebuilding rows while an ancestor template was detached. Defer row reload on the null-render-target notification until reattachment supplies a real renderer. Add a regression for an inactive tab containing a scrolled list, rerun the live check, rebuild both architectures and rerun the complete suite.

That change passed the native CppTest Default -> Aurora -> Default scroll check: item 9 remained at y=124, including the window's initial layout expansion. Both architecture builds passed again. The new remote-protocol regression exposed a second timing issue: CDB found a temporary total height of 300 against a 374-high viewport after refresh. Remote ellipsis labels start at 1x1 and fetch an already-cached font height only when sending their next rendering message. Make GetMinSize consult the existing font-height cache using the current element font, so replacement list rows can measure correctly before the next remote frame. Keep uncached measurement requests and the existing fallback behavior unchanged.

The font-height-cache experiment did not solve this additional remoting case: DefaultTextListItemTemplate uses full text measurement, not ellipsis/font-height measurement. The experiment is removed. The required CppTest scroll acceptance now passes; the separate, pre-existing asynchronous-remoting scroll limitation is recorded below, without claiming it passed or changing the remote measurement protocol in this palette task. The focused committed regression exercises the null-target lifecycle directly and proves that it neither recreates realized rows nor changes their scroll state.

Remoting follow-up reproduction (the native palette is already default; replacing SetColorPackage with no operation still reaches the same existing RefreshThemes measurement path):

```cpp
		TEST_CASE(L"DarkSkin palette refresh preserves a scrolled list on an inactive tab")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				ConnectThemeTestProtocol(protocol);
				auto skin = Ptr(new darkskin::Theme);
				theme::RegisterTheme(skin);
				{
					GuiWindow window(theme::ThemeName::Window);
					window.SetText(L"Retained list position");
					window.SetClientSize(Size(640, 480));
					auto tab = new GuiTab(theme::ThemeName::Tab);
					tab->GetBoundsComposition()->SetAlignmentToParent(Margin(5, 5, 5, 5));
					window.GetContainerComposition()->AddChild(tab->GetBoundsComposition());
					auto listPage = new GuiTabPage(theme::ThemeName::CustomControl);
					listPage->SetText(L"List");
					tab->GetPages().Add(listPage);
					auto palettePage = new GuiTabPage(theme::ThemeName::CustomControl);
					palettePage->SetText(L"Palette");
					tab->GetPages().Add(palettePage);
					auto list = new GuiTextList(theme::ThemeName::TextList);
					list->GetBoundsComposition()->SetAlignmentToParent(Margin(5, 5, 5, 5));
					listPage->GetContainerComposition()->AddChild(list->GetBoundsComposition());
					for (vint i = 0; i < 60; i++) list->GetItems().Add(Ptr(new TextItem(itow(i), false)));
					Point position;
					protocol->OnNextIdleFrame(L"Scroll list", [&]()
					{
						list->SetSelected(3, true);
						list->SetViewPosition(Point(0, 192));
					});
					protocol->OnNextIdleFrame(L"Hide scrolled tab", [&]()
					{
						position = list->GetViewPosition();
						TEST_ASSERT(position.y == 192);
						tab->SetSelectedPage(palettePage);
					});
					for (vint preset = 0; preset < 6; preset++)
					{
						protocol->OnNextIdleFrame(L"Refresh palette " + itow(preset), [&, preset]()
						{
							TEST_ASSERT(list->GetViewPosition() == position);
							darkskin::SetColorPackage(darkskin::Theme::CreateColorPackage(preset));
							GetApplication()->RefreshThemes();
						});
						protocol->OnNextIdleFrame(L"Check hidden list " + itow(preset), [&]()
						{
							TEST_ASSERT(list->GetViewPosition() == position);
							TEST_ASSERT(list->GetSelected(3) && list->GetItems().Count() == 60);
							TEST_ASSERT(tab->GetSelectedPage() == palettePage);
						});
					}
					protocol->OnNextIdleFrame(L"Show retained list", [&]()
					{
						tab->SetSelectedPage(listPage);
					});
					protocol->OnNextIdleFrame(L"Restore default", [&]()
					{
						TEST_ASSERT(list->GetViewPosition() == position);
						darkskin::SetColorPackage(darkskin::CreateDefaultColorPackage());
						GetApplication()->RefreshThemes();
						window.Hide();
					});
					GetApplication()->Run(&window);
				}
				theme::UnregisterTheme(skin->Name);
			});
			GacUIUnitTest_Start(L"Application/DarkSkin/InactiveListScroll");
		});

```

### CONFIRMED

The selected implementation supplies one authoritative Workflow palette across native and interpreted hosts, preserving all 40 default values and all 40 existing reactive expression bodies. Default, Aurora, Ember, Moonstone, Lagoon and Rosewood share 28 neutral roles and have 12 distinct accent roles each. The native configuration forwards to those factories; there is no second palette-value table. The selector queues a value-captured package with installation and RefreshThemes in the same callback. Creating another theme does not reset it.

Resource generation completed for Win32 and x64 with no UI error files. Immutable Workflow struct construction and explicit global typing were corrected in authored XML. The large generated-only DarkSkin/FullControlTest aggregates were regenerated after checking they contained no user-code markers; MainWindow and DocumentEditorBase user-code files were preserved. No generated output was hand-patched, and all normal compiler BUILD switches are restored.

The required Debug Win32 build / Metadata_Generate Win32 / Debug x64 build / Metadata_Generate x64 / Metadata_Test x64 sequence passed. Both metadata dumps match their round trips; core metadata hashes are unchanged. Both architectures rebuilt after the native list fix and final test correction with zero warnings or errors (x64 10.87 seconds; Win32 7.89 seconds). The final wrapper-run x64 suite exited 0 with 90/90 test files and 1759/1759 test cases passed, including TestApplication_Theme and TestTuiProvider. The completed Execute.log ends at those counts with no appended memory-leak report.

The earlier complete suite passed 90/90 files and 1758/1758 cases with no appended leak report, including TestTuiProvider and the two new DarkSkin palette tests. The final suite adds the null-render-target regression. It checks that detachment neither recreates realized rows nor changes scroll/selection state. Its final assertion and shutdown share the detachment frame because a separate idle frame with no rendering changes is invalid for this test harness. The existing palette tests check automatic initialization, existing and fresh controls, hover/press/disable bindings, selection baseline colors, document identity, caret, undo state and restoring default.

The final snapshot comparison inspected all 120 changed existing frame JSON files after resolving element references and ignoring generated IDs, while preserving geometry, child order, element descriptions, hit testing, cursor and window size. Of these, 118 are visually identical to HEAD. The other two differ only because the date picker highlights September 10 instead of September 9; those date-only baseline changes were restored. Twelve frames contain the same pre-existing missing renderer references at the same bounds in both versions. The remaining generated frame/report changes record fewer row recreations after the null-target guard, with no default-palette visual regression.

Live validation used the supported control/composition endpoints and the actual native remoting renderer's active element descriptions:

- CppTest Win32 and x64 exposed all six choices and the original default. Each settled selection had one matching radio, expected accent/window-border colors, and survived repeated refresh. Add 10/Clear operated on both text lists. Document text, selected range and search text survived; undo and redo worked afterward. Existing and newly opened child windows used the selected colors. A previously created menu reopened with the expected palette. Highlighted ribbon tabs, progress fill, selected tree/grid items and fixed orange/gray examples retained their expected values. After the null-target fix, a 60-item list retained its 192-pixel offset (item 9 at y=124) through Default -> Aurora -> Default and normal queued shutdown (exit 0).
- CppTest_Metaonly passed Default -> Lagoon -> Default; CppTest_Reflection passed Default -> Aurora -> Default, each with queued shutdown exit 0. The reflection host's initial concurrent launch collided with an already-running global shortcut; an isolated run passed. UI reads must observe settled layout before choosing another palette.
- GacUI_Host Win32 loaded the regenerated resource binaries and passed Default -> Aurora -> Default, with matching active border/accent colors and exit 0. Its first run was interrupted after a canceled HTTP read; the successful retry allowed interpreted refresh to finish without canceling the response.
- RemotingTest_Core /Pipe /FCT plus RemotingTest_Rendering_Win32 /Pipe passed Default -> Ember -> Default. Input entered through the renderer, and its active DOM elements showed #B66332/#B76433 then #007ACC/#017ACC. Both processes exited 0. Temporary debugger arguments were restored byte-for-byte.
- CppTest_Tui passed all six original presets via keyboard/PTY at 120x40 and 80x25 in Win32, and 120x40 in x64. Exact RGB values and one selected radio were observed; SkyBlue was restored and queued Close exited 0 with console state restored. SGR mouse injection had no effect, so these results establish keyboard behavior.

Release-GacUI completed both skin-generation/CodePack passes and the GacGen rebuild. The supported Release-Project pack was repeated after the native list fix. Both normal and IncludeOnly downstream skin/configuration consumers compiled in Debug x64 with zero errors (two MSB8029 warnings from the temporary build directory). Each IncludeOnly pair references its canonical Config files once; both obsolete TuiSkin config pairs remain absent. The relocated TuiSkin preset bodies are unchanged apart from removing a redundant old-path include. Official packing also incorporated previously unreleased existing core RefreshThemes/TUI changes; the only newly authored core change in this task is the null-render-target row-reload guard. The sibling Tools release-workflow change is committed and pushed as 3484925.

Validation limits and remaining issue:

- Native monitor capture failed twice (CreateForMonitor 0x80070057); native keyboard injection also failed with access denied. No screenshot or physical-input visual pass is claimed. A fake color dialog entered its normal modal loop, blocking the queued automation read; that run was deliberately terminated. Fake-dialog readability across every palette was not established.
- The six options fit at the showcase's 640-wide minimum and after its height expanded. Attempted border resizing through remoting input had no effect, so an independent narrow-width resize check is not claimed. Linux vmake inputs were updated, but Linux builds and generated makefiles were not validated on this Windows host.
- The additional remoting hidden-list scroll reproduction above still fails because asynchronous full-text measurements temporarily shrink new row heights. Palette switching itself works at the renderer. This existing measurement issue remains unresolved; the final suite's focused null-target test does not claim to cover it. The ineffective remote-font-cache experiment was removed.
