# GacUI Layout Best Practice

- Layout for TUI based GacUI application is different mainly because `<ImageFrame/>` is unavailable and one pixel is one character.
- When creating a new skin (a set of control templates), it is required to list all templates using a `<ThemeTemplates/>` instance.
  - Missing specific control templates will lead to failure of creating controls, but this is also another way to disable some controls with a certain skin. If such disabling is intentional, it is not a bug.
  - Multiple skins could be stacked one on top of another, if any control template is offered in another skin below, the related control could still be created.
- Control/item templates are also recommended and almost the only way to change the look of controls. If a template is created for a specific feature instead of app-wise theme, `<ThemeTemplate/>` is not needed here, instead setting the template to a control explicitly would also work.
- `DarkSkin` is the official default skin for GUI based GacUI application, which is also a great example for learning how to create fully functional templates for any controls.

## Best Practice for Choosing Appropriate Compositions and Controls

## Best Practice for GUI Based Layout

## Best Practice for TUI Based Layout

## Best Practice for Creating/Updating Official TuiSkin

- Exclude ribbon and toolbar since they requires image.
  - Using them in GacUI XML Resource leading to crash is expected, due to not offering proper control templates.
- Colors are listed in fields of a struct in a workflow global variable. `-eval` binding could be used in XML to use them.
  - The type name will be `tuiskin::ColorPackage`.
  - The variable name will be `tuiColors`.
  - When a color is needed, there is only two ways: using `tuiColors` or any color property from a template. Hardcoded color values are not allowed.
  - Besides of generated C++ code, there are a pair of manually written C++ file for TuiSkin calls `TuiSkinConfig.(h|cpp)`.
    - Currently it has only two functions:
      - `tuiskin::CreateDefaultColorPackage` to create `tuiskin::ColorPackage` with all color assigned.
      - `tuiskin::SetColorPackage` to set a color package to the global variable.
    - `TuiSkinConfig.(h|cpp)` will `#include` generated files from TuiSkin's XML resource.
- Polygons on darkskin is to render icons without actually using an image, such thing should be replaced by one character in TuiSkin.

### Color List (CreateDefaultColorPackage's values)

- General, unless described below, these colors should be used:
  - `ControlBackground`: black, for any control background.
  - `ControlText`: white, for any control text.
  - `ControlBorder`: gray.
  - `LabelText`: gray.
- Menus:
  - `MenuBackground`: dark gray, for normal or disabled menu item and menu bar backgrounds.
  - `MenuText`: white, for normal menu text and menu separator.
  - `MenuTextDisabled`: gray.
  - `MenuBackgroundHighlighted`, dark blue.
  - `MenuTextHighlighted`, white.
- List items, tab headers:
  - `ItemBackground`: black, for normal or disabled item background.
  - `ItemText`: white, for normal item text.
  - `ItemTextDisabled`: gray.
  - `ItemBackgroundHighlighted`: dark blue.
  - `ItemTextHighlighted`: white.
  - `ItemBackgroundSelected`: light sky blue.
  - `ItemTextSelected`: drak gray.
  - In data grid, when a cell is selected, only the selected cell uses selected color, other cells in this row use highlighted color.
- Buttons:
  - `ButtonBackground`: dark gray, for normal or disabled button background.
  - `ButtonText`: white, for normal button text.
  - `ButtonIcon`: gray.
  - `ButtonTextDisabled`: gray.
  - `ButtonBackgroundHighlighted`: light sky blue.
  - `ButtonTextHighlighted`: dark gray.
  - `ButtonBackgroundPressed`: light gray.
  - `ButtonTextPressed`: white.
- Exceptions:
  - Although check boxes and radio buttons are buttons, but they should use label colors.
  - Combo box dropdown button and scroll bar arrow buttons are button with icon not with text.
    - When the button is in any state except normal, icon colors are text colors.

### Layouting
