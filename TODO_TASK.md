- Add `VTab` to `GUI_CONTROL_TEMPLATE_TYPES` which is a `TabTemplate`:
  - Add the vertical tab version to `DarkSkin` and `TuiSkin`. A new virtual class `GuiVTab` from `GuiTab` needs to be registered, making `<VTab/>` available in GacUI XML Resource.
  - The `DarkSkin` version could take `FullControlTest` as a reference by looking at `Layout` test page.
- In `FullControlTest`:
  - The `Search` label in `Document Editor (Ribbon)` is always white, it should use `<Label/>` instead.
  - Update `Layout` test page to use `VTab`, and delete the vertical tab template definition from `FullControlTest`.
- In `TuiControlTest`:
  - Update `Layout` test page to use `VTab` like `FullControlTest`.
- To verify the above work, just launch `CppTest` and `CppTest_Tui` and see if expected changes happen.
- Release `GacUI` to `Release` and make sure `FullControlTest` and `TuiControlTest` are updated to `GacUI_ControlTemplates` tutorial.
  - In `Build.ps1` adds a `-Fast` option, it does:
    - For `-Project Vlpp`, it skips `Build-Vlpp`, same to other projects if not mentioned below. This could be done by add a `fast` argument to `Task-Vlpp` and other functions.
    - For `-Project GacUI` it keep all others and calls `Build-GacUI` with fast argument, skipping `Test-GacUI-SubProject "UnitTest"` and keep others, so that metadata binary files are generated.
    - For `-Project Release` it keep all others and calls `Task-Verify-Cpp` with fast argument, which only builds debug but not release.
  - Use `Build.ps1 -Release -Fast`.
- Add `VTab` to the website, there are multiple places need to update, check out what pages reference `Tab` control.
  - Publish the website, when doing `job:copilotInitAll` skip learning.
- commit and push all local changes in affected repos, `CheckRepo.ps1 CheckAll` could be used to verify if there is any remaining local change.

## DETAILS

- Append `F(TabTemplate, VTab)` to `GUI_CONTROL_TEMPLATE_TYPES` in `<GacUI repo>/Source/Application/Controls/GuiThemeManager.h` to preserve existing enum values. Register `ADD_VIRTUAL_CONTROL(VTab, GuiTab, VTab)` in `<GacUI repo>/Source/Compiler/InstanceLoaders/GuiInstanceLoader_Plugin.cpp`. The resource compiler then constructs `GuiTab(ThemeName::VTab)` for `<VTab/>`. Pages and selection use the existing tab API.
- Register `att.VTab` in both `<GacUI repo>/Test/Resources/App/DarkSkin/Index.xml` and `<GacUI repo>/Test/Resources/App/TuiSkin/Index.xml`. Reuse each skin's tab header, give the vertical template `TabOrder="TopToBottom"`, and place the selected page to the right. Use TuiSkin's cell dimensions, border and focus styling.
- Remove `<GacUI repo>/Test/Resources/App/FullControlTest/VerticalTabTemplate.xml`, its resource link and its entries in the GacUI_Compiler project and filters. Remove the Layout activation handler that assigns the explicit template back to itself. Theme refresh handles the registered default. In `<GacUI repo>/Test/Resources/App/FullControlTest/DocumentEditorRibbon.xml`, keep the `Search: ` caption, alignment and adjacent text box's `Alt="S"` while allowing the label template to supply the text color.
- Thread `-Fast` through the named project and default build paths in `<Tools repo>/Tools/Build.ps1`. Ordinary projects still import and release their outputs. `Task-Workflow` calls the RPC stdio check separately after `Build-Workflow`. Skipping just that build leaves the check dependent on previously built Release x64 driver and service binaries.
- In `<Tools repo>/Tools/ProjectGacUI.ps1`, keep both architecture builds, metadata generation and testing, tool deployment, skin generation and packing. Skip the UnitTest invocation in fast mode. The Release branch calls `Task-Verify-Cpp-OpenFolders`. Both C++ verification entry points call `Build-Release-Verify-GacUI-Cpp` in `<Tools repo>/Tools/BuildRelease.ps1`. Guard the Release tutorial build loop there and retain the Debug executable checks, Workflow verification and XML generation.
- Update the current manual under `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui`. Cover the component navigation, control hierarchy, container overview and tab reference, default template registration example, and TabTemplate reference. Add the `VTab` to `TabTemplate` mapping in `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/components/ctemplates/metadata/themeNames.json`. The custom vertical-template tutorial can point readers to the built-in control.

## VERIFICATION

1. Follow the generation sequence in `<GacUI repo>/Project.md`. Generate current Win32 and x64 core metadata before running GacUI_Compiler, which reads both. Rebuild the generated skins and showcases, regenerate and check the final metadata, then launch the updated Debug x64 apps.
2. In `CppTest`, check that Layout lists Repeat, Responsive, Easy Layout and Easy Layout (Table) down the left side in that order. Select pages by mouse and Up/Down. Open the responsive modal with the button and its displayed Alt sequence, close it, and check that the same page is selected. Resize and switch palettes. The button should stay five layout pixels below the last header, page edits should survive, and the ribbon's Search caption should match a normal label. Update the existing palette lookup in `<GacUI repo>/Test/UIA_CppTest_Shared.cs` to identify the Layout control by its new theme name.
3. In an interactive `CppTest_Tui` session, check that Layout stacks Repeat above Responsive, each selection shows its content on the right, and Up/Down works with the tab focused. Resize from 120 by 40 to 80 by 25 and check header readability. The main tab strip should still be horizontal.
4. Check the fast build log for Win32 and x64 metadata generation and testing, then Debug C++ tutorial verification. Confirm that the GacUI UnitTest invocation and the Release tutorial build loop still run in ordinary mode and are skipped in fast mode. Verify that the packaged showcases under `<Release repo>/Tutorial/GacUI_ControlTemplate/BlackSkin/UI/FullControlTest` and `<Release repo>/Tutorial/GacUI_ControlTemplate/TuiSkin/UI/TuiControlTest` use the new theme and that the obsolete showcase template is gone.
5. Follow `<WebsiteSource repo>/job.publish.prompt.md`. Check that the published tab reference and generated Markdown both describe `<VTab/>`. Start both copilotInitAll passes at “Merge Knowledge Base” as directed by `<Tools repo>/Jobs/job.Windows.copilotInitAll.prompt.md`. After pushing the affected repositories, inspect the output of `<Tools repo>/Tools/CheckRepo.ps1 CheckAll` for outstanding changes.

## REVIEW COMMENTS

### Keeping the responsive button below the Layout headers

**review comment**: FullControlTest places the button in `AfterHeaders`, a property of `GuiRibbonTab` in `<GacUI repo>/Source/Controls/ToolstripPackage/GuiRibbonControls.h`. A literal `<VTab/>` backed by `GuiTab` cannot accept it. Existing checks cover the five-pixel gap, the button role and its W access key. Windows UI Automation also collects header-slot children specifically from ribbon tabs.

**suggested solution**: The smallest change is to use `<RibbonTab ControlThemeName="VTab">` for this one showcase and have DarkSkin supply a `RibbonTabTemplate` for the new theme, as it already does for `Tab`. TuiControlTest can use `<VTab/>` directly. If both showcases must use the literal tag, move the header slots and their template attachment, reflection and UI Automation support to the base tab types so the button keeps its position and behavior.

### Choosing the fast release command

**review comment**: `<Tools repo>/Tools/Build.ps1` currently declares only the `-Project` parameter. Its `Release` target copies previously packed GacUI outputs. Adding only `-Fast` would leave the requested `-Release -Fast` spelling undefined, and running only the existing Release target could package stale skins and metadata.

**suggested solution**: Use `-Project GacUI -Fast` followed by `-Project Release -Fast`. If the literal one-command spelling is required, define `-Release -Fast` to run those two stages in that order.
