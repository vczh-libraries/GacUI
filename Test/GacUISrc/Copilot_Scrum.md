# !!!SCRUM!!!

# DESIGN REQUEST

Currently `GuiVirtualTextList` doesn't handle [SPACE] key. It should handle that to toggle all selected item's `Checked` property.

If all selected items are checked, uncheck every selected items. If at least one selected item is not checked, checke every selected items.

## UPDATE

I don't agree with the current design. By the way I think only one task is enough. Since `Checked` is a concept that only happen in `GuiVirtualTextList` and all sub classes, so we should never add anything to its base class (e.g. `GuiSelectableListControl` in your design).

You can just register a key handler just like how `GuiSelectableListControl` is doing, but just handle [SPACE]. And since the logic is simple, you may not add helper functions.

You should not touch any `GuiVirtualTextList` sub class, I believe `ITextItemView` is already enough for you.

## UPDATE

FYI `GuiVirtualTextList_TestCases` should be a good place to add the new test case. You not only need to follow existing test cases to create snapshots, you should also check after pressing [SPACE] if Checked of items changed or unchanged as expected.

You could name the test case `CheckItemsByKey`

## Phrase 1: Implement Space Key Handling in GuiVirtualTextList

This phrase focuses on implementing space key handling directly in `GuiVirtualTextList` to toggle the checked state of all selected items. The implementation will follow the existing pattern used in `GuiSelectableListControl` for keyboard handling but will be specific to the checked items functionality that only exists in `GuiVirtualTextList`.

The space key handling will register a key event handler in the constructor, similar to how `GuiSelectableListControl` handles its keyboard events. The logic will be straightforward: examine all selected items through the `ITextItemView` interface, determine the target state (check all if any are unchecked, uncheck all if all are checked), and update each selected item accordingly.

### Task 1-1: Add Space Key Handler to GuiVirtualTextList

Implement space key handling directly in `GuiVirtualTextList` by adding a key event handler that toggles the checked state of all selected items based on their current state.

**What to be done:**
- Add a key event handler to `GuiVirtualTextList` constructor that specifically handles `VKEY::KEY_SPACE`
- Implement the toggle logic inline within the event handler without additional helper functions
- Use the existing `ITextItemView` interface to query and modify checked states
- Follow the existing pattern used by `GuiSelectableListControl` for keyboard event handling
- Ensure the implementation uses the item provider's editing system (`PushEditing`/`PopEditing`) for consistency
- Fire the `ItemChecked` event for each item that changes state
- Only operate on currently selected items and do nothing if no items are selected

**What to test in Unit Test:**
- Test space key handling with single item selected (both checked and unchecked states)
- Test space key handling with multiple items selected in various combinations:
  - All items checked (should uncheck all)
  - All items unchecked (should check all)  
  - Mixed checked states (should check all unchecked items)
- Test space key with no items selected (should do nothing)
- Test space key works correctly with different `TextListView` types (Check, Radio)
- Verify that `ItemChecked` events are fired for each changed item
- Test that other keyboard navigation keys are not affected
- Test integration with item provider editing system
- Test that the functionality works with custom item providers that implement `ITextItemView`

**What to test manually:**
- Interactive testing to ensure space key feels natural with keyboard navigation flow
- Visual verification that checked state updates immediately when space is pressed
- Test with different control templates to ensure proper visual feedback
- Verify that the toggle behavior is intuitive and consistent with user expectations

**Reasons why this task is necessary:**
- Space key is a standard shortcut for toggling checked items in list controls across different UI frameworks
- The functionality should be contained within `GuiVirtualTextList` since checked state is specific to this control and its subclasses
- Using the existing keyboard event handling pattern ensures consistency with the rest of the codebase
- The `ITextItemView` interface already provides all necessary functionality for querying and modifying checked states
- Keeping the implementation simple without helper functions reduces complexity and maintenance overhead

**Support evidences:**
- `GuiSelectableListControl` already demonstrates the pattern for keyboard event handling in its constructor with `focusableComposition->GetEventReceiver()->keyDown.AttachMethod`
- The `ITextItemView` interface provides `GetChecked` and `SetChecked` methods specifically for this purpose
- The existing `ItemChecked` event in `GuiVirtualTextList` is designed to be fired when item checked states change
- The pattern of using `PushEditing`/`PopEditing` is established throughout the list control implementation for item modifications
- The `OnBulletSelectedChanged` method in `DefaultTextListItemTemplate` shows the correct pattern for modifying item checked states through the item provider

### Task 1-2: Add Unit Test Case for Space Key Toggle Functionality

Create a new test case named `CheckItemsByKey` within the existing `GuiVirtualTextList_TestCases` framework to thoroughly test the space key toggle functionality with snapshot verification.

**What to be done:**
- Add a new test case `CheckItemsByKey` to the `GuiVirtualTextList_TestCases` template function
- Create test scenarios that verify space key functionality in both Check and Radio view modes
- Implement test steps that select items, press space key, and verify checked state changes
- Use the existing GacUI unit test framework patterns for keyboard input simulation
- Create visual snapshots to verify the UI updates correctly after space key presses
- Test various selection and checked state combinations as outlined in the previous task
- Follow the existing test structure and naming conventions used in `GuiVirtualTextList_Template.cpp`

**What to test in Unit Test:**
- All the space key functionality scenarios described in Task 1-1
- Visual snapshot verification to ensure UI updates correctly
- Proper event firing sequence verification
- Integration with existing list control test infrastructure
- Verification that space key doesn't interfere with other test cases

**What to test manually:**
- Review test output and snapshots to ensure they capture the expected behavior
- Manual validation of test case execution to ensure comprehensive coverage

**Reasons why this task is necessary:**
- The existing test framework for `GuiVirtualTextList` already provides the infrastructure for testing list control behavior
- Snapshot testing ensures that visual changes are captured and can be reviewed for correctness
- Having automated tests for the space key functionality prevents regression when other parts of the codebase change
- Following the existing test patterns ensures consistency with the project's testing approach

**Support evidences:**
- The existing `GuiVirtualTextList_TestCases` function in `TestControls_List_GuiVirtualTextList_Template.cpp` shows the established pattern for testing list control functionality
- The test framework includes keyboard simulation capabilities through the `UnitTestRemoteProtocol` interface
- The existing test cases demonstrate how to verify item states and UI updates through snapshots
- The pattern of testing different view modes (Check, Radio) is already established in the existing tests

# !!!FINISHED!!!
