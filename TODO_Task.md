In `FullControlHost` there is a `EazyLayout` page:
- Currently there are two rows at the end, insert a third row above them with 1 `<ez:Right/>` having a check button, and 3 `<ez:Left/>` each having a button, they do nothing.

### DETAILS

- The existing page is `demo::EasyLayoutTabPage`, displayed as `Easy Layout` in FullControlTest, in `Test/Resources/App/FullControlTest/EasyLayoutTabPage.xml`. This is the target of the `FullControlHost` reference above.
- Insert the new row before the two rows in the bottom layout. An outer `ez:Row` accepts columns, not direct Left/Right descriptors: use one `ez:Column CellSpan="2"` containing a borderless nested `ez:Layout`. Declare the Right checkbox first, followed by the three Left buttons, to demonstrate that docking groups determine visual placement independently of declaration order.
- Give the four controls distinct labels and stable names for automation. No application event handlers are needed; the checkbox retains its ordinary toggle behavior. Preserve the existing two rows, splitter, rebuild behavior and editor binding.

### VERIFICATION

- Regenerate the showcase with `GacUI_Compiler`, then rebuild before launching. Follow the skin section below for the shared generation and metadata checks.
- Through automation, open `Easy Layout` and verify three left-aligned buttons and one right-aligned checkbox above the existing rows, with non-overlapping bounds at normal and smaller window sizes. Invoke each button and toggle the checkbox without application side effects.
- Rebuild horizontal/vertical content, drag the existing splitter and verify that editor text and its label binding survive. Exercise the generated showcase and the Workflow-loaded `GacUI_Host` path; include the new controls in both UIA script runs described below.

DarkSkin:
  - `Test\Resources\App\DarkSkin\BaselineDocuments.xml`:
    - This file is not used anymore, remove it and clean up references to avoid build breaks.
  - `Test\Resources\App\DarkSkin\Style.xml`:
    - `textBoxBackgroundAndColor` seems not in use anymore, remove it, and clean up any styles that is not used.
  - `Test\Resources\App\TuiSkin\Index.xml`
    - Remove `InstallColorPackage` and just update the variable in `SetColorPackage`.

### DETAILS

- The Index.xml path in this DarkSkin subsection refers to `Test/Resources/App/DarkSkin/Index.xml`. `darkskin::Theme::SetColorPackage` lives there, while `InstallColorPackage` is defined in `DarkSkin.xml`; remove that helper and assign the installed palette directly in the static setter.
- Update the native `darkskin::SetColorPackage` wrapper in `Source/Skins/DarkSkin/Config/DarkSkinConfig.cpp`, which currently calls the removed global helper. Keep native and Workflow entry points consistent.
- Remove the `BaselineDocuments.xml` resource link from `DarkSkin.xml` and any project inventory entries along with the file. Retain the live palette-based `darkskin::CreateBaselineDocument` function and document styles that call it; deleting the obsolete resource does not mean deleting baseline support.
- Audit style selectors and their consumers before removing unused styles. `textBoxBackgroundAndColor` and `tabDropdownBackground` are candidates; similarly named `documentBoxBackgroundAndColor` still calls the live baseline helper.
- Author changes under `Test/Resources/App/DarkSkin` and canonical `Source/Skins/DarkSkin/Config`. Follow the existing generation/release process for generated and mirrored skin files.

### VERIFICATION

- Search resource links, project inventories, authored styles and native wrappers for references to removed files/functions. After regeneration, check generated output for stale references as well.
- Exercise all DarkSkin presets and a return to default. New windows must retain the selected palette; existing document controls must retain text, selection, undo/redo and scroll state after theme refresh. Check textbox and document backgrounds, caret and selection colors, including a document-based grid editor.
- Complete the shared build, generation and unit-test checks in the TuiSkin section.

TuiSkin:
  - `Test\Resources\App\TuiSkin\Index.xml`
    - `CreateBaselineDocument` should be removed.
    - Remove `InstallColorPackage` and add `SetColorPackage` in this file as a static function to replace it.
  - Delete `CreateSkyblueColorPackage` from `TuiSkinConfig.cpp`.
  - Move `CreateColorPackageInternal` and `CreateDefaultColorPackage` from `TuiSkinConfig.cpp` to `TuiSkin.xml`, so that `tuiColors` could be initialized by `CreateDefaultColorPackage`.
  - This remove the requirement for every Tui application to call `tuiskin::SetColorPackage(tuiskin::CreateDefaultColorPackage());` in their `GuiMain` function, clean this up from test apps.

The above requests is to mirror APIs between DarkSkin and TuiSkin so it becomes a pattern.

### DETAILS

- `TuiTheme::CreateBaselineDocument` in `Test/Resources/App/TuiSkin/Index.xml` is a public wrapper. The internal `tuiskin::CreateBaselineDocument` in `TuiSkin.xml` is still used by document templates and must remain. The public wrapper has a live caller in `Test/Resources/App/TuiControlTest/DataGridComponents.xml`; replace that duplicate borderless document template with the existing `tuiskin::TuiDocumentLabelTemplate`, preserving the editor's bindings and layout.
- Define the shared palette initializer and complete default palette in `TuiSkin.xml`, initialize `tuiColors` once from that default, and add static `TuiTheme::SetColorPackage` in `Index.xml` to assign it directly. `InstallColorPackage` currently lives in `TuiSkin.xml`, not `Index.xml`.
- Keep `Source/Skins/TuiSkin/Config/TuiSkinConfig.h/.cpp` as the canonical native API. Following DarkSkin, retain a native `CreateDefaultColorPackage` forwarding wrapper and make the remaining named native factories call the Workflow initializer. Update the native setter to call the static theme setter. Do not duplicate palette defaults in C++ or reset them when creating a theme/window.
- Remove the declaration as well as the definition of `CreateSkyblueColorPackage`. Preserve every default field and alpha value, including SkyBlue accent `#87CEFA` and highlight `#000080`. Replace calls in `Test/GacUISrc/CppTest_Tui/Main.cpp` and `Test/GacUISrc/UnitTest/TestTuiProvider.cpp` with `CreateDefaultColorPackage`; retain the SkyBlue selector and its position in the showcase.
- Remove redundant startup default installation from application entry points. Explicit installations that restore a test's initial state or select a different palette remain meaningful.
- For both skins, installation affects future templates. Continue to call `GuiApplication::RefreshThemes()` on the UI thread to update existing controls, deferring palette installation and refresh together when invoked from input callbacks. Preserve the existing native configuration ownership, development-header selection and generated inventories. Update stale initialization/API descriptions in `Project.md` and `.github/KnowledgeBase/KB_GacUI_Design_TuiPlatformProvider.md` to match.

### VERIFICATION

- Follow `.github/Guidelines/Building.md`, `Running-CLI.md` and `Running-UnitTest.md`. Use the absolute-path `copilotBuild.ps1` and `copilotExecute.ps1` wrappers from `Test/GacUISrc`; do not hand-edit generated sources or metadata.
- Run `GacUI_Compiler` for the changed resources and inspect its output and any `*.UI.errors.txt`. Rebuild the generated consumers. Complete the `Project.md` metadata sequence after generation: Debug Win32 build and `Metadata_Generate`, Debug x64 build and `Metadata_Generate`, then x64 `Metadata_Test`. If reflection is changed before compilation, refresh the metadata needed by that compilation too.
- Run UnitTest after C++ changes, including relevant TUI palette/theme/document cases and Easy Layout/resource cases. In `TestTuiProvider.cpp`, compare the default palette directly with the old field values and opacity; replacing both sides of the old default-versus-SkyBlue assertion with the default factory would make it meaningless. Cover initialization before any setter and persistence after constructing another theme/window. Respect the test filter rules, verify intended files actually run, and require the final pass summary with no Debug leak dump. Review meaningful changed snapshots.
- Start `CppTest_Tui` without any startup palette installation and require the same default appearance. Follow `.github/Jobs/DebugTuiControlTestSop.md` for terminal launch and interaction; this application has no HTTP automation endpoint. Select each preset and return to SkyBlue at 120x40 and 80x25, then open a new window to check the palette is not reset.
- Verify existing controls and an active grid document editor retain text, selection, edits and scroll state through refresh. Confirm fake TUI dialogs and new document controls use the selected palette. Check authored and regenerated files for removed API references, including unit tests and native declarations.

UiaList tool
- ListView in property windows still feel low performance to scroll, since almost all pages have the refresh buttons, make sure those values are actually cached inside corresponding view model classes until refresh button is clicked.
- When running with `CppTest`, it seems the UI tab can highlight any sub window components, fix it.

### DETAILS

- Interpret the second bullet as "cannot highlight subwindow components": UiaList's existing contract requires the UI preview to highlight the deepest eligible UIA descendant under the pointer. Reproduce this with hosted `CppTest` windows before selecting the fix.
- `Tools/UiaList/UiaList/UI/PropertyWindow.xml` uses a `BindableDataGrid`. Its property row display values are already cached by `PropertyViewModel.cpp`; profile scrolling, row recycling, layout and rebinding instead of assuming every getter performs a UIA query. Repeated allocation in `TextRangeViewModel::GetSections` and repeated derived-label/argument work in `ActionViewModel.cpp` are investigation candidates, not confirmed causes.
- Keep target reads on the existing MTA worker and publish cached presentation values. Scrolling, painting and revisiting loaded tabs must not read the target again. Preserve initial loading, explicit Refresh/parameterized queries and existing readback after successful edits/actions. Local drafts and command availability must still respond to changes in local inputs.
- Trace highlighting through provider physical bounds/offscreen state, captured-image origin, image-local pointer coordinates, preview scrolling and the overlay. Fix the responsible layer, including GacUI's hosted provider if necessary. Preserve snapshot-generation checks so stale results cannot highlight another target.
- Read `Tools/UiaList/AGENTS.md` before implementation. If its authored XML changes, regenerate its `Source` through the documented GacBuild flow using `Tools/UiaList/GacUI.xml`; UiaList has a separate solution and generated source ownership.

### VERIFICATION

- Run Debug UiaList with `CppTest` and `CppTest_Metaonly` on distinct automation ports, for example 8891, 8888 and 8890 respectively. Operate UiaList through its `/Automation/UiaListApp/Controls` and `/IO` endpoints.
- Compare scrolling/profile evidence before and after the change on representative property grids, provider pages and text-range workspaces. Confirm presentation induces no target reads, cached values remain stable, and Refresh exposes external changes. Avoid arbitrary machine-dependent timing thresholds.
- Follow relevant P02/P04/P05/P08 cases and `Smoke.ps1 -GetterRegression` in `Tools/UiaList/Verification/README.md`: check draft retention during recycling, Escape without writes, one explicit commit/action with readback, unsupported-property filtering and stale asynchronous results.
- Hover controls in the hosted main window and an opened child window; require the correct descendant label and rectangle. Clicking the preview must select that same node and expand ancestors without sending input to the target. Repeat after preview scrolling, target move/resize followed by refresh, and target switching; compare native-window `CppTest_Metaonly` behavior.
- Include U02-U06 coverage for coordinates/DPI, overlap, clipping and snapshot consistency. Use automation geometry first and pixel captures only where needed to verify native capture/highlight rendering. Build UiaList's solution and run its documented regression checks in addition to applicable GacUISrc verification.

`Project.md`:
- `Test/UIA_CppTest(_Metaonly)?.ps1` should be updated when `FullControlTest` is changed, to sync changes to this script, as well as making sure that UIA is working properly. `UiaList` tool could help reviewing the change, operate `UiaList` with automation service.
- All UIA related stuff should be in the `## Windows Specific` section.

### DETAILS

- Put the maintenance rule, UIA script entry points, UiaList instructions and prerequisites under `Project.md`'s existing `## Windows Specific`. This organization request applies to the document, not to relocating UIA source files.
- `Test/UIA_CppTest.ps1` and `Test/UIA_CppTest_Metaonly.ps1` are entry wrappers. Shared setup is in `Test/UIA_CppTest_Shared.ps1`, while assertions are in `Test/UIA_CppTest_Shared.cs`; maintain the shared implementation when showcase changes affect expected behavior.
- Generic tab traversal already exists. Add targeted checks for the new Easy Layout controls' roles/names and Toggle/Invoke behavior; verify left/right arrangement using automation bounds rather than relying on successful tab traversal alone.

### VERIFICATION

- Run both UIA entry scripts against fresh processes after rebuilding FullControlTest. Require their assertions to pass and their owned processes/endpoints to shut down normally.
- Review the final Windows section for complete script paths and distinct automation-port examples; keep existing hosted/native distinctions and UiaList's Debug-only endpoint restriction accurate.

`ToDo/1.4.1.3.md` under `New added functions`:
- Check out commits since `1.4.1.2` tags, some functions are added to controls due to implementing UIA and EazyLayout, list all of them, no need to explain what they do.
- This is a good time to check if they have proper comments. Add them when similar functions or objects are already commented.
- Do not include UIA and EazyLayout themselves to this list.

### DETAILS

- Use the actual `1.4.1.2..HEAD` history and verify additions against current declarations. Include `Source/Application/Controls` as well as `Source/Controls`; folder names alone do not identify every control API.
- Current public control-function candidates are `GuiWindow::GetModal` and `GuiWindow::GetBlockedByModalWindow` in `Source/Application/Controls/GuiWindowControls.h`, plus `GuiDocumentCommonInterface::EnsureTextPositionVisible`, `GetActiveHyperlink` and `ExecuteHyperlink` in `Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h`. Verify this inventory while executing and list qualified names only under the existing release-note bullet.
- Exclude protected implementation helpers, UIA provider APIs and the EazyLayout subsystem. The new repeat-composition row/column-count helpers are composition APIs, not control functions. Newly added events and event-argument objects still belong in the adjacent documentation-comment audit even though they are not entries in the function list.
- Match neighboring documentation style, including parameters and return semantics where appropriate. In particular, check the window additions and `ExecuteHyperlink`, which currently lack comments. Do not change behavior or reflection registration merely to document these APIs.

### VERIFICATION

- Cross-check the names against the tag diff and current headers, with no pre-existing functions, duplicate inherited listings or implementation-only entries. Keep the release note free of behavior descriptions as requested.
- Review added comments against implementation behavior, and run the required C++ build/unit-test checks if headers are edited. Regenerate metadata only if reflection changes require it or as part of the skin generation already requested above.

In `Tools` repo:
- Update `GacUILayout.md`:
  - Fix stale fact.
  - Delete `## Best Practice for Creating/Updating Official TuiSkin` section and its sub sections as it is already in the code.
  - Under `Best Practice for GUI Based Layout` and `Best Practice for TUI Based Layout`, create `### Adding New DarkSkin Theme` and `### Adding New TuiSkin Theme` to describe how to make a new color package, explain each item and how `CreateColorPackageInternal` could help doing this.
  - Create `## Creating New Skin` at the end to explain how to make a new skin, do not have to be too deep into the topic, you should summary how to copy `DarkSkin` and `TuiSkin` (not in the source code perspective, but in a high level view about how many things need to pay attention), and the theme changing pattern in `DarkSkin` and `TuiSkin`.
  - Before `Best Practice for Using Compositions and Controls Appropriately` create a `## Using EazyLayout` section:
    - Saying that eazy layout coule be used to simplay the layout as it automatically covers many best practice.
    - **CUT** `GacUI/EazyLayout.md` to `Tools/Copilot/Guidelines/GacUIEazyLayout.md`.
    - Update `Tools/Copilot/copilot-instructions.md` to mention `GacUIEazyLayout.md`, repeat the statement that eazy layout should be the first choice as it automatically covers many best practice, and when the user need further control, check out `GacUILayout.md`.
- Update `Running-GacUI.md` before `## Best Practice for Using Compositions and Controls Appropriately` to say that:
  - Screenshot will be the last choice to debug layout issues, always prefer automation service.
  - Screenshot is only recommended when debugging native renderers.
  - Replace `gpt-5.3-codex-spark` with `gpt-5.6-luna`.
  - Add `TuiSkin` besides of `DarkSkin`.
- Update `GacUIXmlResource.md` to reflect the latest predefined namespace lists.
- All in your own words, simple, precise, informative.

### DETAILS

- The canonical files are in the sibling `Tools/Copilot/Guidelines` directory and `Tools/Copilot/copilot-instructions.md`. Read that repository's instructions before editing. Keep the GacUI `.github` copies of the affected guidance synchronized without overwriting unrelated local KB, prompts or scripts; the full `copilotInit.ps1` copies those too.
- The referenced `Best Practice for Using Compositions and Controls Appropriately` heading and existing DarkSkin mention are in `GacUILayout.md`, not `Running-GacUI.md`. Insert `Using EazyLayout` before that layout heading, add TuiSkin alongside DarkSkin in the layout introduction, and place screenshot guidance near the automation introduction in `Running-GacUI.md`.
- Explain the final skin APIs after this task: palette field roles, the initializer's inputs, automatic default initialization, named palette creation, installation and UI-thread refresh. Describe what an independent skin must supply at a high level: template coverage, registration/resource dependencies, GUI/TUI geometry and document baselines. Preserve the limitations for explicitly assigned templates and captured colors.
- Other concrete stale facts in `GacUILayout.md` include `<ThemeTemplate/>` instead of `<ThemeTemplates/>` and the native table spacing example's `CellSpan`/`Border` instead of `CellPadding`/`BorderVisible`.
- Move the complete EazyLayout guide to `Tools/Copilot/Guidelines/GacUIEazyLayout.md`, retaining its examples and splitter rules. Link to it as `./GacUIEazyLayout.md` from layout guidance and as `REPO-ROOT/.github/Guidelines/GacUIEazyLayout.md` from the shared instructions. Include the new guide in GacUI's `.github/Guidelines` mirror and remove the old root file. Update active links to the old location; historical task logs need not be rewritten.
- The namespace authority is `Source/Compiler/GuiInstanceRepresentation.cpp`. The existing unprefixed default list already matches; add the independent predefined `ez` list: `presentation::compositions::eazy_layout::GuiEasy*Composition;presentation::compositions::eazy_layout::GuiEasy*Layout`. Explain that explicit `xmlns` and `xmlns:ez` override their respective defaults independently. Correct the `presentation::elements::GuiButton` example to `presentation::controls::GuiButton`.
- Apply the requested model-name replacement in `Running-GacUI.md`. Keep its screenshot fallback consistent with applications lacking an automation endpoint, such as the TUI showcase; do not invent an endpoint for those applications.

### VERIFICATION

- Check canonical and mirrored guideline files, new links and anchors, the absence of the old root guide, and active references to removed palette APIs. Compare namespace documentation with the compiler constructor and the existing independent-default/override cases in `Test/GacUISrc/UnitTest/TestResource.cpp`.
- Review theme examples against the completed skin refactor; verify the requested headings, removed official-TuiSkin implementation section and model replacement. Ensure the new guide is available through the existing whole-Guidelines distribution.
- Review each repository's diff separately: the documentation move spans GacUI and Tools, and both sides must be included when executing and committing that task.

## REVIEW COMMENTS

No unresolved review comments. The details above record the source-backed corrections and implementation choices; verification steps describe work to perform when executing this task.
