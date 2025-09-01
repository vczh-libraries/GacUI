# !!!SCRUM!!!

# DESIGN REQUEST

- I would like to add an `autoFocusDropDown` boolean argument to `GuiComboBoxBase`'s constructor. All sub classes should provide `false` except for `GuiDateComboBox`.
- When this argument is set to `true`, when the sub menu displays, it should has the focus.
- We need to attach `OnKeyDown` also to the sub menu, so `[ESC]` will always work.

Everything could be in 1 single task.

# PROBLEM DESCRIPTION

## UPDATE

No unit test work is needed

## Phrase 1: Enhance GuiComboBoxBase for Automatic Dropdown Focus Support

This phrase introduces auto-focus capability to combo box controls, enabling dropdown contents to automatically receive focus when opened. This is particularly useful for controls like date pickers where immediate keyboard interaction with the dropdown is expected. The implementation will be backward-compatible and will specifically benefit GuiDateComboBox for better user experience.

### Task 1-1: Add AutoFocusDropDown Parameter to GuiComboBoxBase Constructor [PROCESSED]

Modify the GuiComboBoxBase constructor to accept an autoFocusDropDown parameter, implement automatic focus setting behavior when the submenu opens, and ensure ESC key handling works consistently across all submenu content.

**What to be done:**
- Add a `bool autoFocusDropDown` parameter to `GuiComboBoxBase` constructor with default value `false`
- Store the autoFocusDropDown flag as a protected member variable in GuiComboBoxBase
- Implement automatic focus setting in the submenu opening logic when autoFocusDropDown is true
- Attach a key event handler to the submenu's focusable composition to ensure ESC key works consistently
- Update all existing constructors of GuiComboBoxBase subclasses:
  - `GuiComboButton`: pass `false` for autoFocusDropDown parameter (maintain existing behavior)
  - `GuiComboBoxListControl`: pass `false` for autoFocusDropDown parameter (maintain existing behavior)
  - `GuiDateComboBox`: pass `true` for autoFocusDropDown parameter (enable auto-focus)
- Ensure the implementation is backward compatible and doesn't affect existing behavior for controls that don't need auto-focus
- Locate the appropriate event handler (likely in AfterSubMenuOpening or equivalent) to implement the focus setting logic
- Ensure proper focus restoration when the submenu closes

**What to test in Unit Test:**
- No unit test work is needed as specified by the user

**What to test manually:**
- Interactive keyboard navigation in GuiDateComboBox dropdown after opening
- Visual verification that focus indicator appears on the dropdown content when auto-focus is enabled
- Test that TAB navigation works properly within the focused dropdown
- Verify that the user experience feels natural and responsive for date selection scenarios
- Test that ESC key handling works properly in both auto-focus and non-auto-focus modes
- Test that GuiComboButton and GuiComboBoxListControl maintain their existing behavior (no auto-focus)
- Test that clicking outside the dropdown properly closes it and restores focus appropriately
- Test focus behavior with multiple levels of menus/submenus
- Test that auto-focus doesn't interfere with ALT+key navigation patterns
- Test integration with screen readers and accessibility tools

**Reasons why this task is necessary:**
- GuiDateComboBox currently requires users to click or manually focus the dropdown content before keyboard interaction works, creating poor user experience
- Adding ESC key handling to the submenu ensures consistent behavior regardless of focus state
- The autoFocusDropDown parameter provides a clean, opt-in mechanism that doesn't break existing controls
- This enhancement aligns with standard UI patterns where date pickers and similar controls automatically focus their dropdown content
- The implementation leverages existing GacUI focus management infrastructure, ensuring consistency with the framework

**Support evidences:**
- GuiComboBoxBase already inherits from GuiMenuButton which has submenu management capabilities
- The existing focus management and event handling infrastructure in GacUI provides the necessary foundation
- GacUI's focus management system through `GetEventReceiver()` and `SetFocus()` is well-established
- The pattern of attaching keyboard handlers to submenu compositions is already used for ESC handling in GuiComboBoxBase
- GuiDateComboBox's implementation shows it creates a GuiDatePicker in its submenu, which would benefit from immediate focus
- The existing GuiDateComboBox test infrastructure demonstrates that keyboard interaction with the dropdown is expected behavior

# !!!FINISHED!!!
