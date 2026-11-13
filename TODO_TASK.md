A task to bring new folder, delete folder and delete file to file dialog box.
It includes two file dialog for `GuiFakeDialogService`, one for GUI, another one for TUI.

In GUI's file dialog, above the tree view and list view there is a long label for viewing folder path.
You are going to add a tool bar which only has two buttons, put it at the very right after the label.
The two images are already added to `Source/Utilities/FakeServices/Dialogs/ToolbarIcons`.
One for adding a new folder in the folder displaying in the list view.
One for deleting selected items in the list view.

In TUI's file dialog, there is the same structure, but instead of using toolbar, use two buttons directly.
Their text will be "New Folder" and "Delete" in English, make sure they are localizable like other text.

Both buttons in GUI/TUI file dialog has the same feature,
so you can implement the feature in their view model implementation,
and you should use the file system classes from `VlppOS` so that the implementation is naturally cross platform.

When "New Folder" is clicked,
it shows a modal window asking for the folder name, implemented for both GUI/TUI:
A label for "Enter the folder name".
A text box below it.
Two buttons below it aligned to the right say "OK" and "Cancel".

When clicking OK, it tries to create a folder, when failed, print a message box to tell the user:
- The folder already exists.
- A general failure message.
As the file system class does not return the reason. So you should check folder name existing first and then create the folder.
Only when succeeded the window is closed, otherwise it stays open.

When clicking Cancel, it just closes.

When "Delete" is clicked,
it first ask if the user would like to delete all selected items in a OKCancel message box,
and then delete them when OK is clicked.
File system supports deleting them one by one, be awared that a selected folder might have content, everything will be deleted.
When any failure happens do not stop, just remember what can't be deleted and displayed in a message box.
When there is no failure, nothing needs to prompt.

Here are message box contents:

```plaintext
In folder "xxxx", "yyyy" folder cannot be created because it already exists.
```

```plaintext
In folder "xxxx", "yyyy" folder cannot be created.
```

```plaintext
Are you sure to delete the following items in folder "xxxx"?
abc
def
ghi
```

```plaintext
The following items failed to delete in folder "xxxx":
abc
def
ghi
```

These actions might affect content in tree view and list view, they need to be updated.
Both controls are data binded so changing containers directly should work.

All text in new UI created in this task should be localizable.

To verify, make sure you don't delete wrong content. You can create a temporary folder in GacUI/Test/FileDialogPlayground, do anything you want in this folder for testing, and delete FileDialogPlayground before committing.

It is enough to only test against `CppTest` and `CppTest_Tui`.

No need to update `DebugRemoteProtocolSOP.md`.
