# GacUI Layout Best Practice

- Layout for TUI based GacUI application is different mainly because `<ImageFrame/>` is unavailable and one pixel is one character.
- When creating a new skin (a set of control templates), it is required to list all templates using a `<ThemeTemplates/>` instance.
  - Missing specific control templates will lead to failure of creating controls, but this is also another way to disable some controls with a certain skin. If such disabling is intentional, it is not a bug.
  - Multiple skins could be stacked one on top of another, if any control template is offered in another skin below, the related control could still be created.
  - `DarkSkin` is the official default skin for GUI based GacUI application, which is also a great example for learning how to create fully functional templates for any controls.

## Best Practice for Choosing Appropriate Compositions and Controls

## Best Practice for GUI Based Layout

## Best Practice for TUI Based Layout

## Best Practice for Creating/Updating Official TuiSkin

- Exclude ribbon and toolbar since they requires image.
  - Using them in GacUI XML Resource leading to crash is expected, due to not offering proper control templates.
- Colors are listed in workflow global variables, not hardcoded in all XMLs. `-eval` binding could be used in XML to use them.
- Polygons on darkskin is to render icons without actually using an image, such thing should be replaced by one character in TuiSkin.

### Color List

### Layouting
