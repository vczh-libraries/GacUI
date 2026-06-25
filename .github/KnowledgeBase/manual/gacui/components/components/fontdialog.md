# \<FontDialog\>

**\<FontDialog/\>** displays a OS native font dialog.

## Input / Output Properties

Set these properties before calling **ShowDialog** to create a default user input. These properties will be changed after calling **ShowDialog** if the user change the default input.

### SelectedFont

This property defines the selected font. If **ShowDialog** returns **false**, the value of this property is undefined.

### SelectedColor

This property defines the selected color. If **ShowDialog** returns **false**, the value of this property is undefined.

## Input Properties

Set these properties before calling **ShowDialog** to define the content of the dialog.

### ShowSelection

Set this property to **true** to highlight the **SelectedFont** and **SelectedColor**.

### ShowEffect

Set this property to **true** to enable the font review area.

### ForceFontExist

Set this property to **true** to ensure user cannot select an unexisting font.

## ShowDialog Method

Call this function to display a dialog, and return **true** if a font is selected.

