# GacUI Layout Best Practice

- Layout for TUI based GacUI application is different mainly because `<ImageFrame/>` is unavailable and one pixel is one character.
- When creating a new skin (a set of control templates), it is required to list all templates using a `<ThemeTemplates/>` instance.
  - Missing specific control templates will lead to failure of creating controls, but this is also another way to disable some controls with a certain skin. If such disabling is intentional, it is not a bug.
  - Multiple skins could be stacked one on top of another, if any control template is offered in another skin below, the related control could still be created.
- Control/item templates are also recommended and almost the only way to change the look of controls. If a template is created for a specific feature instead of app-wise theme, `<ThemeTemplate/>` is not needed here, instead setting the template to a control explicitly would also work.
- `DarkSkin` is the official default skin for GUI based GacUI application, which is also a great example for learning how to create fully functional templates for any controls.

## Best Practice for Using Compositions and Controls Appropriately

- Layout of a control applies to its `BoundsComposition` property.
- Tt is recommended to set a composition's `MinSizeLimitation` to `LimitToElementAndChildren`, so it automatically expands to make all children visible.
  - This value needs to be explicitly set, because the default value is `NoLimit`, meaning not caring about its `OwnedElement` or children.
  - If a composition is known to have no children, `LimitToElement` would also be an more optimal option to ensure the `IGuiGraphicsElement` is visible completely.
- There is no such thing like setting an absolute position.
  - Use `AlignmentToParent` to define how it sticks to the border of its parent composition, a component of -1 means it doesn't care where is the parent's border in this direction.
  - Use `PreferredMinSize` if a minimum size is known.
  - All these properties are combined to calculate the actual position and size of a composition.
- There are 2 ways of centering an object:
  - When an object should expand to the size of the container, use `AlignmentToParent`.
  - When an object has its own size, use a 3x3 table, where the center column/row is `MinSize` and others are all `Percentage`.
- Use `Stack` when multiple objects are lined up vertically or horizontally, but there position does not affect or be affected by other objects in the other direction, in which case a table would do the work.
- Prefer `Label` over `SolidLabel` in normal UI because `Label` could load the expected font and control template.
- Prefer `SolidLabel` over `Label` in templates because `SolidLabel` is more lightway, more configuration, meanwhile expected font and color will come from bindable template properties.
  - Centering a `SolidLabel` could be easily done to set both alignment to `Center`.

## Best Practice for GUI Based Layout

- Ideal distance between objects are 5 pixels.
  - It should be the distance between two visible objects, not between positions of two GacUI classes.
  - For example, a table with `CellSpan` set to 5 will a fixed distance between cells. But when a cell embeds another table with `CellSpan` set to 5 with `Border` set to true, the distance between inner table cells to outer table cells is actually 10, which does not maintain the 5 pixels rule.
  - Another example, if a container has `InternalMargin` all values set to 5, if a control in it also has `Margin` all values set to 5, the distance between the border of the container and the child control is actually 10, which does not maintain the 5 pixels rule.

## Best Practice for TUI Based Layout

- Usually there should be no distance between objects, no matter vertically or horizontally.
  - Adjacent text rows and buttons can stack without blank rows. Add vertical spacing only to separate distinct groups.
    - There is always no need to leave one pixel between objects with borders in one or both, because borders in TUI already have a visible distance due to the representing character.
    - There is always no need to leave one pixel between objects stacking horizontally.
  - There is no need to keep a distance if they are stacking horizontally, as TuiSkin requires the control has one pixel to before and after the text.
  - There is no need to keep a distance between two objects with borders, since borders are rendered by characters, they are visibly having a distance.

- Dialogs and forms use zero table/stack padding between rows, and exactly one explicit blank row before the final action row. Size message, color and font content from its minimum height instead of retaining a large initial client height or an unused percentage row. Keep real textbox/list/group borders and useful list/preview areas. Explicit blank lines in the content are part of the text and must remain.
- Center the natural-width message action group across the entire dialog with percentage/minimum/percentage columns. Right-align Color, simple/full Font and File dialog action groups by placing a percentage column before their minimum-width button columns, including the full-font Pick a Color action. Message text uses a normal multiline label without a surrounding scroll-container border; preserve the separate icon label and default action lookup.
- Keep a child window's top border visible between its caption and caption buttons. Paint only the title text and its immediate spacing over the border, not an opaque full-width title strip. Keep caption buttons contiguous with no spacer columns between them, preserve ellipsis and close hit targets, and omit commands unsupported by the hosted window manager.
- Center form labels vertically while keeping their text left aligned, using local layout or an explicitly assigned template. A three-row table with percentage/minimum/percentage rows can center a label of its natural height in a bordered textbox's cell without changing every label's defaults.
- A preferred minimum height of one does not prevent a tracker from stretching. Center an HTracker in an absolute one-cell middle row between equal percentage rows, and stretch it horizontally only. Its enclosing RGB component can still accommodate a three-cell bordered textbox. Do not place the actual text/control into either percentage spacer row.

## Best Practice for Creating/Updating Official TuiSkin

- Exclude ribbon and toolbar since they requires image.
  - Using them in GacUI XML Resource leading to crash is expected, due to not offering proper control templates.
- Colors are listed in fields of a struct in a workflow global variable. `-eval` binding could be used in XML to use them.
  - The type name will be `tuiskin::ColorPackage`.
  - The variable name will be `tuiColors`.
  - When a color is needed, there is only two ways: using `tuiColors` or any color property from a template. Hardcoded color values are not allowed.
  - Besides of generated C++ code, there are a pair of manually written C++ file for TuiSkin calls `TuiSkinConfig.(h|cpp)`.
    - `tuiskin::CreatePinkColorPackage`, `CreateOrangeColorPackage`, `CreateGrassPackage`, `CreateEmeraldPackage`, `CreateSkyblueColorPackage`, and `CreatePurplePackage` create complete opaque palettes with muted accents and darker highlights. Every neutral role is shared.
    - `tuiskin::CreateDefaultColorPackage` returns the unchanged SkyBlue palette. `CreateColorPackageInternal` initializes all fields for the six factories.
    - `tuiskin::SetColorPackage` installs the global palette for new templates. To recolor existing default-themed controls, call `GuiApplication::RefreshThemes()` on the UI thread. From an input callback, queue installation and refresh together with `InvokeInMainThread`, capturing the preset by value.
    - `GuiControl::RefreshThemes()` preserves an assigned `ControlTemplate` while traversing its descendants. Application-owned `Color-eval` elements also retain their captured colors; these presets deliberately share their neutral roles. Arbitrary custom templates and captured element colors are not made reactive by installing a palette.
    - A palette selector uses one mutex group and processes only newly selected radio buttons. Keep selection in the persistent controls, with no selection-reset callback during refresh. Window Manager uses two equal percentage columns, existing controls on the left and a compact vertical palette group on the right.
    - `TuiSkinConfig.(h|cpp)` will `#include` generated files from TuiSkin's XML resource.
- Polygons on darkskin is to render icons without actually using an image, such thing should be replaced by one character in TuiSkin.

### Color List (CreateDefaultColorPackage's values)

- General, unless described below, these colors should be used:
  - `ControlBackground`: black, for any control background.
  - `ControlText`: white, for any control text.
  - `ControlBorder`: gray.
  - `ControlBorderDisabled`: dark gray.
  - `ControlBorderFocused`: light sky blue.
  - `LabelText`: gray.
- Menus (includes menu item in dropdown, and menu button in menu bar):
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
  - Group boxes also use label colors.
  - Combo box dropdown button and scroll bar arrow buttons are button with icon not with text.
    - When the button is in any state except normal, icon colors are text colors.
  - The mark of a check box of a radio button will use text color instead of icon color.
  - Grid lines in list view or data grid would use the control border colors, but when the control is focused, grid lines still use the normal color.

### Style and Layout

- Window frame.
  - Main window do not have a frame, its template will be a pure background with nothing.
    - It will use the frame from the OS CLI window.\
    - When query for all components (border, sizebox, maximizing button, minimizing button, etc) are all invisible.
  - Sub window could have a frame apon user settings:
    - When there is a sizable box, use double line.
    - When there is a border, use thick line.
    - When there is no box, there is no line.
      - Actual menus retain their own thin border. Ordinary popups, tooltips and combo/date/filter content dropdowns use opaque borderless templates; content controls keep their own borders.
- Unlike darkskin, scroll bars, buttons and menu items will have no border.
  - Two spaces are added around the control text, so that buttons, or menu buttons in a menu bar, could just be stacked together without spaces between them horizontally.
  - Do not add spaces around the text, instead use 1 pixel of distance in layout, which will be rendered with a space.
- Group box will be a round line, the group header is directly on the top border from the 3rd pixel.
- Focused control is represented by its border color when there is a border, and by bold plus underline when there is text. Include button, check/radio, combo and menu text; tab headers use `OwnerTab.Focused`.
  - Main window won't have difference because it is always activated.
- For buttons and menus, highlighted state renders underline on its text.
  - Pressed buttons also need underline on its text.
- Tab consists of tab buttons and a container. The container has a thin border in `ItemBackgroundSelected`, or `ItemBackground` when disabled.
  - The row of horizontally aligned tab headers, or the last row when there is multiple, is directly on the top border from the 3rd pixel.
- Any container control do not need a margin between the content to the border, meaning its `ContainerComposition` is located from (1,1), meanwhile the left-top position is defined as (0,0).

- Ordinary and date combos occupy one row, with button state colors, horizontal text insets and a separate dropdown arrow. Preserve selected-item compositions and `TextVisible`.
- ALT sequence labels use an opaque `ShortcutKeyBackground` (white by default) and `ShortcutKeyText` (black by default).
- Paint the whole scrollbar/tracker handle uniformly. Disabled handles blend into the track.
- Put a header sorting glyph before its title, using the title text color. Keep the right submenu button separate and reserve the following header's first cell for resizing the preceding column.
- `Table.CellPadding` adds both outer and inter-cell spacing. Use zero around already bordered lists/textboxes and between compact form rows; retain only insets inside actual borders.
- For a physical terminal main window, keep the outer bounds independent of child minimum sizes (`MinSizeLimitation="NoLimit"`). Aligned page contents must use the available viewport and scrolling; a hidden page must not enlarge the main window beyond the terminal.
- Embedded grid text editors should use an explicitly assigned borderless one-row template. Derive row heights from content and retain bottom/right separators; do not reserve rows for the ordinary bordered textbox template.
- In editable DataGrid view, selected-cell and highlighted-row backgrounds cover content only. Preserve the entire bottom separator row's normal `ItemBackground` and separator foreground, including intersections and the space beyond the last column. Inset both the row highlight and the cell selection fill; keep the existing compact row height and editor insets. Detail view, TextList and TreeView have no such separator row and keep selection across their full content height.
