# \<OpenFileDialog\>

**\<OpenFileDialog/\>**displays a OS native open file dialog.

## Input / Output Properties

Set these properties before calling**ShowDialog**to create a default user input. These properties will be changed after calling**ShowDialog**if the user change the default input.

### FileName

This property defines the full path of the selected file. If**ShowDialog**returns**false**, the value of this property is undefined.

## Input Properties

Set these properties before calling**ShowDialog**to define the content of the dialog.

### Filter

This property defines a file filter by wildcard.

Pattern names and wildcards in a filter are splitted by "|". For example, a filter to display text files or all files will typically be:
```
Text Files (*.txt)|*.txt|All Files (*.*)|*.*
```
This filter defines two filters:
- **Text Files (*.txt)**: its wildcard is *.txt
- **All Files (*.*)**: its wildcard is *.*

### FilterIndex

This property defines the default selected filter, starting from**0**.

### EnabledPreview

Set this property to**true**to enable the preview area. When a file is selected, its content will be displayed in the preview area.

### Title

This property defines the title of the dialog.

### Directory

This property defines the default folder of the dialog.

### DefaultExtension

This property defines the default extension of the dialog.

If a file is selected by manually entering a file name, the**DefaultExtension**will be appended after the file name if the file extension is not entered.

**IMPORTANT**: The value of the**DefaultExtension**property does not include the "." character. For example, an extension for text files is**txt**, instead of**.txt**.

### Options

The value of this property must be one of the following values, or multiple values combined together:
- **FileDialogAllowMultipleSelection**: Allow multiple selection.
- **FileDialogFileMustExist**: Prevent the user to select unexisting files.
- **FileDialogShowReadOnlyCheckBox**: Show the "Read Only" check box.
- **FileDialogDereferenceLinks**: Dereference link files.**FileName**and**FileNames**will contain the full path of actual files.
- **FileDialogShowNetworkButton**: Show the "Network" button to allow selection of objects across the network.
- **FileDialogPromptCreateFile**: Prompt if a new file is going to be created.
- **FileDialogPromptOverwriteFile**: Promt if a existing file is going to be overwritten.
- **FileDialogDirectoryMustExist**: Prevent the user to select an unexisting directory.
- **FileDialogAddToRecent**: Add user selected files to "Recent" directory.

## ShowDialog Method

Call this function to display a dialog, and return**true**if a file is selected.

## Output Properties

Read these properties after calling**ShowDialog**for user input.

### FileNames

This property returns full paths of all selected files. If**ShowDialog**returns**false**, the value of this property is undefined.

