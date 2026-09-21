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
