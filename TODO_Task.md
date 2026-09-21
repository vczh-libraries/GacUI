In `FullControlHost` there is a `EazyLayout` page:
- Currently there are two rows at the end, insert a third row above them with 1 `<ez:Right/>` having a check button, and 3 `<ez:Left/>` each having a button, they do nothing.

DarkSkin:
  - `Test\Resources\App\DarkSkin\BaselineDocuments.xml`:
    - This file is not used anymore, remove it and clean up references to avoid build breaks.
  - `Test\Resources\App\DarkSkin\Style.xml`:
    - `textBoxBackgroundAndColor` seems not in use anymore, remove it, and clean up any styles that is not used.
  - `Test\Resources\App\TuiSkin\Index.xml`
    - Remove `InstallColorPackage` and just update the variable in `SetColorPackage`.

TuiSkin:
  - `Test\Resources\App\TuiSkin\Index.xml`
    - `CreateBaselineDocument` should be removed.
    - Remove `InstallColorPackage` and add `SetColorPackage` in this file as a static function to replace it.
  - Delete `CreateSkyblueColorPackage` from `TuiSkinConfig.cpp`.
  - Move `CreateColorPackageInternal` and `CreateDefaultColorPackage` from `TuiSkinConfig.cpp` to `TuiSkin.xml`, so that `tuiColors` could be initialized by `CreateDefaultColorPackage`.
  - This remove the requirement for every Tui application to call `tuiskin::SetColorPackage(tuiskin::CreateDefaultColorPackage());` in their `GuiMain` function, clean this up from test apps.

The above requests is to mirror APIs between DarkSkin and TuiSkin so it becomes a pattern.

UiaList tool
- ListView in property windows still feel low performance to scroll, since almost all pages have the refresh buttons, make sure those values are actually cached inside corresponding view model classes until refresh button is clicked.
- When running with `CppTest`, it seems the UI tab can highlight any sub window components, fix it.

`Project.md`:
- `Test/UIA_CppTest(_Metaonly)?.ps1` should be updated when `FullControlTest` is changed, to sync changes to this script, as well as making sure that UIA is working properly. `UiaList` tool could help reviewing the change, operate `UiaList` with automation service.
- All UIA related stuff should be in the `## Windows Specific` section.

`ToDo/1.4.1.3.md` under `New added functions`:
- Check out commits since `1.4.1.2` tags, some functions are added to controls due to implementing UIA and EazyLayout, list all of them, no need to explain what they do.
- This is a good time to check if they have proper comments. Add them when similar functions or objects are already commented.
- Do not include UIA and EazyLayout themselves to this list.

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
