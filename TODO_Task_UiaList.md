This task mainly focuses on improving `UiaList`.
- `Processes` tab:
  - Besides `Refresh` there should be a combo box taking the whole width, listing all process trees. But windows is not listed in this combo box dropdown.
    - A process is visible in this dropdown when there it or any sub process owns a visible window.
  - The tree view is replaced by a list view, when a process is selected, listing all visible windows directly owns by this process.
    - Columns show title and whatever is currently printed in the list view.
    - Hidden/Visible is no longer needed as hidden ones are not listed.
  - When left click, instead of left double click, jumps to the `UI` tab.
- `Nodes`.
  - Double clicking is no more opening a window, we need a right click context menu with "Inspect". The reason is that, currently double clicking would expand/collapse a node if it is not a leaf node.
- The property window:
  - `Properties` tab: Not supported properties is not listed.
  - `Actions` node:
    - Currently there are buttons for each provider to expand or collapse the view for that provider.
    - Expanding or collapsing is not needed, instead replace it with group boxes.
    - Keep a small margin around the group boxes, like 5, is enough to visibly distince different providers.
  - Provider UI:
    - Currently there are small gaps between properties, which is fine. But around action buttons there are big gaps, use the same gap.
    - You should check what is actual rendering via automation service to know is there big gaps between anything, I don't like big gaps, all gaps should look the same size.

Test scripts.
- Rename `CppTest_Metaonly.ps1` to `UIA_CppTest_Metaonly.ps1`.
- Rename `UIA_Showcase.*` to `UIA_CppTest_Shared.*`.
- Move `AutomationArguments.h` to `Test\GacUISrc\SharedArguments.h`.
- `AutomationPorts.ps1` is not needed, remove it.

Update stale fact in documents affected by this task.
