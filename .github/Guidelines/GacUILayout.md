# GacUI Layout Best Practice

- Layout for TUI based GacUI application is different mainly because `<ImageFrame/>` is unavailable and one pixel is one character.
- When creating a new skin (a set of control templates), it is required to list all templates using a `<ThemeTemplates/>` instance.
  - Missing specific control templates will lead to failure of creating controls, but this is also another way to disable some controls with a certain skin. If such disabling is intentional, it is not a bug.
  - Multiple skins could be stacked one on top of another, if any control template is offered in another skin below, the related control could still be created.
- Control/item templates are also recommended and almost the only way to change the look of controls. If a template is created for a specific feature instead of app-wise theme, `<ThemeTemplates/>` is not needed here, instead setting the template to a control explicitly would also work.
- `DarkSkin` is the official GUI skin and `TuiSkin` is the terminal skin. Both demonstrate complete control templates and a shared palette installation/refresh pattern.

## Using EazyLayout

Use EazyLayout first for application layout. Its docking, shared table tracks, minimum-size propagation and spacing automatically cover many of these practices. Read [the EazyLayout guide](./GacUIEazyLayout.md) for descriptors, examples, rebuilding and splitter rules. Use the composition guidance below when you need more control. For terminal layouts, choose cell-sized spacing explicitly, usually `Padding="0"` and `Border="false"`.

## Best Practice for Using Compositions and Controls Appropriately

- Layout of a control applies to its `BoundsComposition` property.
- It is recommended to set a composition's `MinSizeLimitation` to `LimitToElementAndChildren`, so it automatically expands to make all children visible.
  - This value needs to be explicitly set, because the default value is `NoLimit`, meaning not caring about its `OwnedElement` or children.
  - If a composition is known to have no children, `LimitToElement` would also be an more optimal option to ensure the `IGuiGraphicsElement` is visible completely.
- Bounds compositions use `ExpectedBounds` for requested parent-relative position and size. Parent alignment and minimum-size constraints can override those requests; prefer layout relationships for resizable interfaces.
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
  - For example, a table with `CellPadding` set to 5 leaves a fixed distance between cells. But when a cell embeds another table with `CellPadding` set to 5 with `BorderVisible` set to true, the distance between inner table cells to outer table cells is actually 10, which does not maintain the 5 pixels rule.
  - Account for the visible insets inside control templates as well as composition spacing. Measure the resulting bounds; adding five pixels at each nesting level can double the intended gap.

### Adding New DarkSkin Theme

Create a color package for a new palette; reuse the existing templates. The authored `Test/Resources/App/DarkSkin/DarkSkin.xml` defines `darkskin::ColorPackage`, `CreateColorPackageInternal` and the complete default. Native preset factories belong in `Source/Skins/DarkSkin/Config/DarkSkinConfig.cpp`.

`CreateColorPackageInternal(accents)` accepts a ColorPackage whose twelve accent fields are populated. It copies those fields and supplies every neutral field. Follow an existing named factory, pass its accent package to `vl_workflow_global::DarkSkin::Instance().CreateColorPackageInternal`, and return the complete result. In Workflow, call `darkskin::CreateColorPackageInternal` directly. The fields have these roles:

| Fields | Role |
| --- | --- |
| `GeneralBackground`, `GeneralBorder`, `GeneralAccent` | General surface, outline and primary accent. |
| `ContentBackground`, `ContentBorder`, `Transparent` | Editor/content surfaces, outlines and explicitly transparent paint. |
| `WindowBorderActive` | Active window outline. |
| `TextSecondary`, `TextDisabled`, `GroupText`, `TextNormal`, `TextBright` | Secondary, disabled, group-caption, normal and bright text. |
| `ButtonBackgroundHovered`, `ButtonBorderHovered`, `ControlAccentHovered` | Hovered button surface/outline and emphasized control outline. |
| `ListColumnBorder` | List/grid column separators. |
| `TabHighlightedSelected`, `TabHighlightedHovered`, `TabHighlightedBackground` | Selected, hovered and base colors of highlighted tabs. |
| `ScrollBackground`, `ArrowDisabled`, `ArrowAccentHovered` | Scroll track and disabled/hovered arrow colors. |
| `ScrollHandle`, `ScrollHandleHovered`, `ScrollHandlePressed` | Normal, hovered and pressed scroll handles. |
| `ProgressBackground`, `ProgressBorder`, `ProgressFilling` | Progress track, border and completed portion. |
| `ItemBackgroundSelected` | Selected list/tree/grid item surface. |
| `MenuBackground`, `MenuBorder`, `MenuItemHovered` | Menu surface, outline and hovered item. |
| `SplitterDark`, `SplitterLight` | The two splitter lines. |
| `ComboArrowBackgroundHovered` | Hovered combo arrow region. |
| `ColumnHeaderBackground`, `ColumnHeaderBackgroundHovered` | Normal and hovered column headers. |
| `ExpandingArrowHovered` | Hovered tree expansion arrow. |
| `RibbonExpandingArrow`, `RibbonExpandingArrowPressed` | Normal and pressed ribbon expansion arrows. |

The twelve inputs are `GeneralAccent`, `WindowBorderActive`, `ControlAccentHovered`, `ArrowAccentHovered`, `ItemBackgroundSelected`, `ExpandingArrowHovered`, `RibbonExpandingArrow`, `RibbonExpandingArrowPressed`, `TabHighlightedSelected`, `TabHighlightedHovered`, `TabHighlightedBackground` and `ProgressFilling`. Other fields in the input are ignored. Preserve neutral roles when making an accent-only preset; use the returned full package to make intentional neutral overrides.

The installed palette initializes once from the Workflow default. `darkskin::CreateDefaultColorPackage()` forwards to that default; creating a theme or window does not reinstall it. Install a new package with native `darkskin::SetColorPackage` or Workflow `darkskin::Theme::SetColorPackage`, then call `GuiApplication::RefreshThemes()` on the UI thread to update existing controls.

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

### Adding New TuiSkin Theme

The authored `Test/Resources/App/TuiSkin/TuiSkin.xml` owns `tuiskin::ColorPackage`, `CreateColorPackageInternal(accent, highlight)` and the default. The initializer accepts two opaque Colors: the accent supplies focused borders, selected items and hovered buttons; the darker highlight supplies hovered menu/list items. It fills every neutral role. Native factories in `Source/Skins/TuiSkin/Config/TuiSkinConfig.cpp` call `vl_workflow_global::TuiSkin::Instance().CreateColorPackageInternal`; Workflow calls `tuiskin::CreateColorPackageInternal`.

| Fields | Role |
| --- | --- |
| `ControlBackground`, `ControlText` | General control surface and text. |
| `ControlBorder`, `ControlBorderDisabled`, `ControlBorderFocused` | Normal, disabled and focused outlines; grid lines retain the normal border. |
| `LabelText` | Labels, group captions and checkbox/radio captions. |
| `ShortcutKeyBackground`, `ShortcutKeyText` | Opaque ALT-key hint surface and text. |
| `MenuBackground`, `MenuText`, `MenuTextDisabled` | Menu bar/item surface, normal text/separators and disabled text. |
| `MenuBackgroundHighlighted`, `MenuTextHighlighted` | Highlighted menu surface and text. |
| `ItemBackground`, `ItemText`, `ItemTextDisabled` | List/tree/grid items and tab headers in normal/disabled states. |
| `ItemBackgroundHighlighted`, `ItemTextHighlighted` | Hovered/highlighted item surface and text. |
| `ItemBackgroundSelected`, `ItemTextSelected` | Selected item surface and text; an active grid cell uses these within the highlighted row. |
| `ButtonBackground`, `ButtonText`, `ButtonIcon`, `ButtonTextDisabled` | Normal/disabled button surface, normal text, normal icon and disabled text. |
| `ButtonBackgroundHighlighted`, `ButtonTextHighlighted` | Hovered button surface and text. |
| `ButtonBackgroundPressed`, `ButtonTextPressed` | Pressed button surface and text. |

Combo/scroll arrows use icon colors normally and state text colors when highlighted or pressed. Checkbox/radio marks use text colors. Keep alpha at 255 for opaque terminal surfaces and preserve readable contrast for focused, disabled, hovered and selected states.

`CreateDefaultColorPackage` returns the SkyBlue default (`#87CEFA` accent and `#000080` highlight). Workflow initializes the installed palette once from it, so application startup needs no default setter. Create and install a named package with native `tuiskin::SetColorPackage` or Workflow `tuiskin::TuiTheme::SetColorPackage`, then refresh existing themes on the UI thread. New themes/windows retain the current package.

## Creating New Skin

Start from DarkSkin for GUI geometry or TuiSkin for character-cell geometry. Copy the authored resource structure into an independent resource/namespace, give its ThemeTemplates registration a distinct name, and update resource dependencies and generation inventories. Supply templates for every control family your application creates, including their nested scrollbars, popups, menus, item templates, grid editors and dialogs. Missing templates may fall through to a lower registered skin; intentionally unsupported families must remain explicit.

Choose renderer-compatible elements and dimensions. GUI skins can use images and polygons; TUI skins need supported cell-based elements, text glyphs and compact borders. Define document baselines from the palette, including normal text/background and selection colors. A borderless grid document editor can reuse a suitable document-label template instead of copying it.

Follow the shared palette pattern: define the complete default and initializer in Workflow, initialize the installed package once, and let native named factories supply accents. A setter changes the installed package for future templates. To update existing controls, install the palette and call `GuiApplication::RefreshThemes()` together on the UI thread. From an input callback, defer both operations with `InvokeInMainThread`, capturing the selected preset by value. Theme/window constructors must not reset the selected package.

An explicitly assigned `ControlTemplate` factory survives refresh, although its descendants are visited. Application-owned `Color-eval` values retain the color captured during construction. Palette installation alone does not make custom templates or captured colors reactive. Check new and existing windows, hidden popups, focus, selection, document identity, undo/redo and scroll position after a refresh; include both native and Workflow resource consumers.