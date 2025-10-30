# \<ColorDialog\>

**\<ColorDialog/\>** displays a OS native color dialog.

## Input / Output Properties

Set these properties before calling **ShowDialog** to create a default user input. These properties will be changed after calling **ShowDialog** if the user change the default input.

### SelectedColor

This property defines the selected color. If **ShowDialog** returns **false**, the value of this property is undefined.

## Input Properties

Set these properties before calling **ShowDialog** to define the content of the dialog.

### EnabledCustomColor

Set this property to **true** to enable an extended area, allowing the user to choose a color that is not in the predefined color list.

### OpenedCustomColor

Set this property to **true** to display the custom color area by default. Otherwise the area is collapsed, but the user could still open it in the dialog.

## ShowDialog Method

Call this function to display a dialog, and return **true** if a color is selected.

## Output Properties

Read these properties after calling **ShowDialog** for user input.

### CustomColors

This property returns all colors in the custom color area.

