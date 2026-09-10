`TODO_Task_TUI.md` was completed but multiple issues are found.
If skin or layout issue happens because `GacUILayout.md` said so or the guidance is not clear enough, fix this document.

## TuiControllerBase renderer clipping

- Currently the TUI render target supports clipper.
- Clipping must be implemented in VlppOS's TUI instead of in render target.
  - In this way we could keep DRY.
  - A clipper will be passed to any drawing functions. It could be a TuiClipper struct with x1,y1,x2,y2 in `vint`.
    - In drawing functions, we can first create a clipper equals to the buffer range, and see if null pointer comes in, if not do intersection.
    - In drawing functions such "normalized" clipper will be first calculated and then draw.
  - Render target could just simply calling them in this way.

## TUI Color Theme

- Add multiple predefined color options, allow changing them on the fly in `TuiControlTest`.
  - This is done by asking every window to recreate control templates recursively.
- General implementation:
  - Currently we have a `CreateDefaultColorPackage` for sky blue. We need to create:
    - `CreatePinkColorPackage`
    - `CreateOrangeColorPackage`
    - `CreateGrassPackage`
    - `CreateEmeraldPackage`
    - `CreateSkyblueColorPackage`
    - `CreatePurplePackage`
  - All these colors will not be pure and extreme colors, you need to pick a good one, you may want to get the idea from the internet.
  - And you will find them sharing a lot of code, so a internal function `CreateColorPackageInternal` could be created and every function just calls it.
  - `CreateDefaultColorPackage` will then call `CreateSkyblueColorPackage` internally.
  - Now we need a function to recreate control themes. Calling `SetColorPackage` won't update any existing color, the easiest way is to recreate all control themes.
    - `GuiApplication::RefreshThemes` function will be added for this, in this way it find all living `GuiWindow` instances and call its `RefreshThemes` function.
    - `GuiControl::RefreshThemes` will recreate theme inside recursively to all sub controls. A menu button doesn't need to care about its dropdown as it will be taken care of by `GuiApplication::RefreshThemes`, and a dropdown is not technically a child control of menu button anyway. So the implementation should be straight forward.
    - `GuiControl::RefreshThemes` should only work when its `ControlTemplate` is not assigned, as this property is designed to override the theme. Ideally it should just call `RebuildControlTemplate` for itself and recursively call other `RefreshThemes`.
- Verification
  - In `TuiControlTest`'s `Window Manager` page, split the page in half horizontally and in the right side we could put a group box with radio buttons listed in a vertical stack, set to `SkyBlue (default)`. When any button is clicked, it calls `SetColorPackage` and `GetApplication()->RefreshThemes` in `InvokeInMainThread`.
