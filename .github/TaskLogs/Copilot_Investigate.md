# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- Add `VTab` to `GUI_CONTROL_TEMPLATE_TYPES` with the `TabTemplate` contract. Register `GuiVTab` as a virtual `GuiTab` so GacUI XML resources can use `<VTab/>`.
  - Provide vertical tab templates in `DarkSkin` and `TuiSkin`. Use the existing FullControlTest Layout template as the starting point for DarkSkin.
- In `FullControlTest`:
  - The `Search` label in `Document Editor (Ribbon)` is always white, it should use `<Label/>` instead.
  - Use `<RibbonTab ControlThemeName="VTab">` for the nested tab on the `Layout` page. Keep the `Easy Layout (Responsive)` button in `AfterHeaders` and remove the showcase's local vertical tab template.
- In `TuiControlTest`:
  - Use `<VTab/>` on the `Layout` page for the existing `Repeat` and `Responsive` pages.
- To verify the above work, just launch `CppTest` and `CppTest_Tui` and see if expected changes happen.
- Release GacUI and update both showcases in `<Release repo>/Tutorial/GacUI_ControlTemplate`.
  - Add a `-Fast` switch to `<Tools repo>/Tools/Build.ps1` for both the default build and individual `-Project` targets.
    - For `-Project Vlpp`, skip `Build-Vlpp` and keep its import and release steps. Apply the same rule to each other library project except GacUI.
    - For `-Project GacUI`, pass the switch to `Build-GacUI` and skip only `Test-GacUI-SubProject "UnitTest"`. Generate and test the metadata on both architectures.
    - For `-Project Release`, keep packaging, Workflow and XML verification, and the Debug C++ tutorial builds and executable checks. Skip only the Release C++ tutorial builds.
  - Run `<Tools repo>/Tools/Build.ps1 -Fast` with no project selector. It must run the same projects in the same order as the ordinary default build, applying each project's fast-mode rules.
- Add `VTab` to the website, there are multiple places need to update, check out what pages reference `Tab` control.
  - Publish the website, when doing `job:copilotInitAll` skip learning.
- Commit and push all local changes in the affected repos. `<Tools repo>/Tools/CheckRepo.ps1 CheckAll` can report any remaining changes.

## DETAILS

- Append `F(TabTemplate, VTab)` to `GUI_CONTROL_TEMPLATE_TYPES` in `<GacUI repo>/Source/Application/Controls/GuiThemeManager.h` to preserve existing enum values. Register `ADD_VIRTUAL_CONTROL(VTab, GuiTab, VTab)` in `<GacUI repo>/Source/Compiler/InstanceLoaders/GuiInstanceLoader_Plugin.cpp`. The resource compiler then constructs `GuiTab(ThemeName::VTab)` for `<VTab/>`. Pages and selection use the existing tab API.
- Register `att.VTab` in both `<GacUI repo>/Test/Resources/App/DarkSkin/Index.xml` and `<GacUI repo>/Test/Resources/App/TuiSkin/Index.xml`. In DarkSkin, use a `RibbonTabTemplate` with `BeforeHeadersContainer` and `AfterHeadersContainer`. It satisfies the `TabTemplate` contract and gives the showcase button a place below the headers. In TuiSkin, use a `TabTemplate` with cell-sized spacing, a terminal border and the existing focus styling. In both skins, reuse the tab header, set `TabOrder="TopToBottom"`, and put the selected page on the right.
- Remove `<GacUI repo>/Test/Resources/App/FullControlTest/VerticalTabTemplate.xml`, its resource link and its entries in the GacUI_Compiler project and filters. Remove the Layout activation handler that assigns the explicit template back to itself. Theme refresh handles the registered default. In `<GacUI repo>/Test/Resources/App/FullControlTest/DocumentEditorRibbon.xml`, keep the `Search: ` caption, alignment and adjacent text box's `Alt="S"` while allowing the label template to supply the text color.
- The default build prepares the tools, runs the library tasks through Workflow, runs GacUI, then packages and verifies Release. Pass the switch into the same task functions used by the named project targets. Keep the tool builds inside the release steps. Skipping `Build-Workflow` leaves the separate RPC stdio check in place. That check needs the Workflow solution's Release x64 driver and service binaries.
- In `<Tools repo>/Tools/ProjectGacUI.ps1`, keep both architecture builds, metadata generation and testing, tool deployment, skin generation and packing. For C++ tutorial verification the default build calls `Task-Verify-Cpp` and the named Release target calls `Task-Verify-Cpp-OpenFolders`. Both call `Build-Release-Verify-GacUI-Cpp` in `<Tools repo>/Tools/BuildRelease.ps1`. Apply the fast-mode condition to the Release tutorial build loop in that shared function.
- Update the current manual under `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui`. Cover the component navigation, control hierarchy, container overview and tab reference, default template registration example, and TabTemplate reference. Add the `VTab` to `TabTemplate` mapping in `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/components/ctemplates/metadata/themeNames.json`. The custom vertical-template tutorial can point readers to the built-in control.

## VERIFICATION

1. Follow the generation sequence in `<GacUI repo>/Project.md`. Generate current Win32 and x64 core metadata before running GacUI_Compiler, which reads both. Rebuild the generated skins and showcases, regenerate and check the final metadata, then launch the updated Debug x64 apps.
2. In `CppTest`, check that Layout lists Repeat, Responsive, Easy Layout and Easy Layout (Table) down the left side in that order. Select pages by mouse and Up/Down. Open the responsive modal with the button and its displayed Alt sequence, close it, and check that the same page is selected. Resize and switch palettes. The button should stay five layout pixels below the last header, page edits should survive, and the ribbon's Search caption should match a normal label. The Controls snapshot reports the theme name. Update the Layout palette lookup in `<GacUI repo>/Test/UIA_CppTest_Shared.cs` from `RibbonTab` to `VTab`.
3. In an interactive `CppTest_Tui` session, check that Layout stacks Repeat above Responsive, each selection shows its content on the right, and Up/Down works with the tab focused. Resize from 120 by 40 to 80 by 25 and check header readability. The main tab strip should still be horizontal.
4. Check the default `-Fast` run for library imports and releases, Win32 and x64 GacUI metadata generation and testing, packaging, and Debug C++ tutorial verification in order. Compare the executed steps with the normal dispatch to verify the three skips above. In the packaged resources, verify that `<Release repo>/Tutorial/GacUI_ControlTemplate/BlackSkin/UI/FullControlTest` uses the ribbon tab with the `VTab` theme and `<Release repo>/Tutorial/GacUI_ControlTemplate/TuiSkin/UI/TuiControlTest` uses the `<VTab/>` tag. Check that the obsolete showcase template and its link are gone.
5. Follow `<WebsiteSource repo>/job.publish.prompt.md`. Check that the published tab reference and generated Markdown both describe `<VTab/>`. Start both copilotInitAll passes at “Merge Knowledge Base” as directed by `<Tools repo>/Jobs/job.Windows.copilotInitAll.prompt.md`. After pushing the affected repositories, inspect the output of `<Tools repo>/Tools/CheckRepo.ps1 CheckAll` for outstanding changes.

## REVIEW COMMENTS

Both comments are resolved.

# UPDATES

## UPDATE

The work is overall good and only one thing to fix in `TuiSkin`:

In the horizontal version of tab pages, you will see tab headers are actually on the top border with the left-top corner rendered. So the same thing must apply to vertical tab control like this:

- The last pixel of tab headers should cover the left border of the container border
- Tab headers begin at the second row

So we could still see the corner like this:
```shell
           +----
#####Repeat#
#Responsive#
           |
           |...
```

"#" means background of tab headers, including the left and right filling space and pixels that are not covered by text, so the right filling space is on the tab header. A solid background should do the trick.

And texts should be right aligned for vertical tab control's tab headers.&#x20;

The same to `DarkSkin`'s vertical tab header, make them right aligned.



To verify, you only need to start `CppTest` and `CppTest_Tui` and see if the vertical tab control is good. And then run:

`Build.ps1 -Project GacUI -Fast`

`Build.ps1 -Project Release -Fast`

To release the updated skins.

# TEST [CONFIRMED]

Exercise the Layout page in both Debug x64 showcases. Compare the nested tab theme, header order, keyboard and mouse selection, content geometry, palette refresh, retained page edits and modal access. Verify the normal label in the ribbon editor. Inspect the terminal at 120 by 40 and 80 by 25. Check both metadata architectures, the default and named fast-build dispatch, the two packaged resources and the built, published and synchronized tab reference.

Baseline resource inspection found the nested GUI ribbon tab explicitly installs the showcase template and the Search element sets its color to white. The TUI Layout uses the horizontal tab. The theme inventory, XML loader and build entry point have no VTab or fast switch.

For the follow-up, inspect the actual header glyphs and terminal cells. The corner should sit one row above the first header. Both captions should end one filled cell before the same border column, and the vertical border should resume below the last header. Check both DarkSkin and TuiSkin in the running showcases.

# PROPOSALS

- No.1 Register a vertical tab theme and ship its two skin implementations [CONFIRMED]
- No.2 Align the vertical tab captions and share the terminal page border with the headers

## No.1 Register a vertical tab theme and ship its two skin implementations

Append the theme name and bind a virtual XML control to the existing tab class so the page collection, selection and keyboard behavior stay shared. Move the showcase template into DarkSkin and build the terminal template with the existing focus-aware header. Register both through the skin indexes. Use the ribbon containers on the GUI Layout page and the plain virtual control on the terminal Layout page.

### CODE CHANGE

The Layout automation locates the nested control by its `VTab` theme. When switching palettes it waits for the selected palette to appear in the rendered controls. The old radio provider can survive a theme refresh. The test checks the resulting colors, selection, geometry and edited text across the change.

`VTab` is appended to the theme inventory and registered as a virtual `GuiTab`. DarkSkin now owns the former showcase ribbon template and its two header containers. TuiSkin uses its existing focus-aware header in a vertical stack beside the selected page. The GUI showcase uses the ribbon form, and the terminal showcase uses `<VTab/>`. A palette-styled `Label` supplies the ribbon Search caption. Both metadata architectures and the generated skin and showcase sources were refreshed.

The fast switch uses the existing default and named project dispatch. Each dependency still imports and releases its sources. GacUI still builds and tests both metadata architectures. Release still packages, verifies Workflow and XML, builds the Debug C++ tutorials and checks for their executables. The online and bundled manuals document the control, its shared tab API and both XML forms.

### CONFIRMED

Win32 and x64 Debug builds passed with zero warnings and errors. Each architecture generated its core metadata before the resource compiler ran. After rebuilding the generated sources, both metadata tests passed. Both core reflection listings assign `VTab` the value 65, immediately after `ProgressBar`.

The full Debug x64 GUI Layout scenario passed 3,767 assertions. The focused palette scenario passed 369. The four headers appeared in page order at y = 74, 96, 118 and 140. The last header ended at 162 and the modal button began at 167. Mouse and Up/Down selection, the displayed Alt sequence, modal dismissal, window resizing, the five-pixel gap and retained editor text all passed. The Search caption and a normal ribbon label both rendered in Segoe UI 12 with color `#F1F1F1FF`. The Layout checks waited for the palette accent in the rendered controls, allowing UI Automation to retain or replace its radio provider.

The terminal showcase ran in a live Windows pseudoconsole. At 120 by 40 and 80 by 25, Repeat and Responsive occupied column 2 on consecutive rows. Their page content began at column 18 or 17 respectively. The main strip stayed horizontal. Native Up/Down input selected both pages after shrinking and restoring the terminal. Exiting through the application menu returned zero.

The default `Build.ps1 -Fast` reported an elapsed time of 3 hours, 27 minutes and 39 seconds. A dispatch trace compared the ordinary and fast paths for the default and individual projects. The actual fast log confirmed project order, dependency imports, all releases, the Workflow RPC check, and Win32 and x64 metadata generation and testing. Release packaged both showcases, verified Workflow and XML, built all six Debug C++ tutorial solutions and checked all 55 expected executables. The only skips were the seven dependency library builds, the two GacUI unit-test launches and the Release C++ tutorial build loop.

The packaged BlackSkin XML uses `<RibbonTab ControlThemeName="VTab">` with the existing button and four pages. TuiSkin uses `<VTab>` with Repeat and Responsive. The old showcase template and resource link were removed. The distributed skin sources and metadata matched the files produced by GacUI. The verified package was pushed in `<Release repo>` commit `0c27b7e7`.

The website build and tests passed. GitHub Pages deployment `45507923791` succeeded, and a fresh browser loaded the published tab reference with its keyboard and selection documentation. Both knowledge-base synchronization passes started at Merge Knowledge Base. The final manual tree and index matched the canonical Tools copy in all eight receiving repositories. Both passes skipped learning.

## No.2 Align the vertical tab captions and share the terminal page border with the headers

Mirror the horizontal TUI tab layout on the left edge. Draw the page border first and place the vertical stack one row below the corner, with its existing opaque header backgrounds painting over one border cell. Use the installed tab template’s orientation to align the shared header label in each skin.

### CODE CHANGE

Anchor the border to the widest header, accounting for the single shared cell and empty tabs. Preserve the horizontal templates’ text alignment, header commands and focus handling.
