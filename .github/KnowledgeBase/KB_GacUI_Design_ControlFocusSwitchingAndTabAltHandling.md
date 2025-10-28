# Control Focus Switching and TAB/ALT Handling

## Architecture Overview

Control focus management in GacUI operates across three distinct architectural layers, each with specific responsibilities:

### Layer 1: Composition-Level Focus (`GuiGraphicsComposition`)

The lowest layer handles focus at the composition level through `GuiGraphicsHost`:

- `GuiGraphicsHost` maintains the currently focused composition in the `focusedComposition` member variable
- `GuiGraphicsHost::SetFocusInternal` is the core focus-switching method that:
  - Fires `lostFocus` event on the currently focused composition (if any)
  - Updates `focusedComposition` to the new composition
  - Resets caret point to (0,0)
  - Fires `gotFocus` event on the newly focused composition
- `GuiGraphicsHost::SetFocus` validates that the composition belongs to this host before calling `SetFocusInternal`
- `GuiGraphicsHost::ClearFocus` removes focus from any composition
- `GuiGraphicsHost::GetFocusedComposition` returns the currently focused composition
- Each composition has an event receiver that fires `gotFocus` and `lostFocus` events

### Layer 2: Control-Level Focus (`GuiControl`)

The middle layer connects controls to composition-level focus:

- Each `GuiControl` has a `focusableComposition` property (can be null)
- `GuiControl::SetFocusableComposition` attaches `gotFocus` and `lostFocus` event handlers to the composition
- When composition gains focus: `GuiControl::OnGotFocus` is called, which:
  - Sets `isFocused = true`
  - Updates control template state
  - Fires `FocusedChanged` event
- When composition loses focus: `GuiControl::OnLostFocus` is called, which:
  - Sets `isFocused = false`
  - Updates control template state
  - Fires `FocusedChanged` event
- `GuiControl::GetFocused` checks if the control is currently focused
- `GuiControl::SetFocused` is the public API to focus a control:
  - Returns early if no focusable composition exists
  - Returns early if control is not visually enabled
  - Returns early if focusable composition is not eventually visible
  - Gets the graphics host from the focusable composition
  - Calls `host->SetFocus(focusableComposition)`

### Layer 3: Focus Loss on State Changes

Focus is automatically cleared when controls undergo certain state changes:

- **When disabled**: `GuiControl::SetEnabled(false)` calls `UpdateVisuallyEnabled`, which triggers `OnLostFocus` if the control was focused
- **When hidden**: `GuiControl::SetVisible(false)` does NOT automatically clear focus (based on test case comments indicating unclear behavior)
- **When deleted**: `GuiGraphicsHost::DisconnectCompositionInternal` is called recursively, clearing `focusedComposition` if it matches the disconnected composition

## TAB Key Navigation

### Architecture Components

TAB navigation is managed by `GuiTabActionManager`, which is created and owned by `GuiGraphicsHost`.

#### IGuiTabAction Interface

Controls implement `IGuiTabAction` service to participate in TAB navigation:

- `GetAcceptTabInput()`: Returns whether control accepts TAB character as input (if true, TAB key is passed to control instead of used for navigation)
- `GetTabPriority()`: Returns priority for TAB order (-1 = lowest priority, 0, 1, 2... = higher priorities)
- `IsTabEnabled()`: Returns whether control can be tabbed to (checks if control is visible and enabled)
- `IsTabAvailable()`: Returns whether control is available for TAB navigation (checks if focusable composition exists)

#### Control List Building (`BuildControlList`)

The TAB manager builds a prioritized list of focusable controls:

- Uses `tab_focus::CollectControls` to recursively collect all controls from the control host
- Groups controls by their `TabPriority` value
- Inserts controls in priority order: 0, 1, 2, ..., then -1 (default/unprioritized)
- For each control, recursively collects its children and inserts them after the parent
- Result is a flattened list `controlsInOrder` sorted by TAB priority
- Cache is invalidated when a control's `TabPriority` changes via `GuiControl::SetTabPriority`

#### Focus Navigation (`GetNextFocusControl`)

Finding the next focusable control:

- Rebuilds control list if cache is invalidated (`available == false`)
- Takes current focused control and offset (+1 for forward TAB, -1 for backward SHIFT+TAB)
- Finds start index in the control list
- Wraps around using modulo arithmetic: `(index + offset + count) % count`
- Loops through controls until finding one that is both `IsTabAvailable()` and `IsTabEnabled()`
- Returns the next focusable control or null if none found

#### Key Event Handling

TAB key processing in `GuiTabActionManager::KeyDown`:

- Only processes TAB key when CTRL and ALT are NOT pressed
- If focused composition's control has `GetAcceptTabInput() == true`, TAB is NOT intercepted (returns false to let control handle it)
- Otherwise, calls `GetNextFocusControl` with offset based on SHIFT key state
- If next control found: calls `next->SetFocused()` and sets `supressTabOnce = true` to prevent TAB character insertion
- Returns true to indicate key was handled

Character suppression in `GuiTabActionManager::Char`:

- Suppresses the TAB character (`\t`) if `supressTabOnce` is true
- Resets `supressTabOnce` to false after checking

## ALT Key Navigation

### Architecture Components

ALT navigation is managed by `GuiAltActionManager`, which is created and owned by `GuiGraphicsHost`.

#### IGuiAltAction Interface

Controls implement `IGuiAltAction` service to have ALT shortcuts:

- `GetAlt()`: Returns the ALT key string (e.g., "F" for ALT+F)
- `IsAltEnabled()`: Returns whether ALT action is enabled (checks visible and enabled state)
- `IsAltAvailable()`: Returns whether ALT action is available (checks if focusable composition and alt string exist)
- `GetAltComposition()`: Returns the composition to show the ALT key label on
- `GetActivatingAltHost()`: Returns the alt host to enter when this action is activated (null = just activate)
- `OnActiveAlt()`: Called when ALT key is activated (default implementation calls `SetFocused()`)
- `IGuiAltAction::IsLegalAlt`: Validates that ALT string contains only uppercase letters (A-Z) and digits (0-9)

#### IGuiAltActionContainer Interface

Controls can implement this to provide multiple ALT actions:

- `GetAltActionCount()`: Returns the number of ALT actions
- `GetAltAction(index)`: Returns the ALT action at the specified index

#### IGuiAltActionHost Interface

Hosts create a hierarchy of ALT contexts that can be entered/exited:

- `GetAltComposition()`: Returns the composition for this host
- `GetPreviousAltHost()`: Returns the parent host in the hierarchy
- `OnActivatedAltHost(previousHost)` and `OnDeactivatedAltHost()`: Called when entering/leaving
- `CollectAltActions(actions)`: Collects all ALT actions for this host

ALT action collection from controls (`IGuiAltActionHost::CollectAltActionsFromControl`):

- Recursively traverses control tree starting from the specified control
- If control has `IGuiAltActionContainer`: collects all actions from it
- Else if control has `IGuiAltAction` and `IsAltAvailable()` and `IsAltEnabled()`: adds single action
- **Critical behavior**: When a control has an ALT action, executes `continue` which prevents its children from being added to the traversal queue
- This creates a "barrier" effect where children are hidden unless you enter a nested ALT context
- Recursively processes all children only if the control doesn't have its own ALT action

### ALT Mode Lifecycle

#### Activation

Pressing ALT (KEY_MENU) without CTRL or SHIFT:

- `GuiAltActionManager::KeyDown` checks if control host implements `IGuiAltActionHost`
- Calls `EnterAltHost` to enter ALT mode

`EnterAltHost` process:

- Calls `ClearAltHost` to clean up previous state
- Calls `host->CollectAltActions(actions)` to get all available actions
- If no actions available: calls `CloseAltHost` and exits
- Calls `host->OnActivatedAltHost(currentAltHost)` with previous host
- Updates `currentAltHost` to the new host
- Calls `CreateAltTitles` to create visual labels

#### Visual Label Creation (`CreateAltTitles`)

Creating ALT key labels:

- For each ALT action key (grouped by alt string):
  - If single action and non-empty key: uses key as-is
  - If multiple actions with same key: appends numeric suffix (0-999) with zero-padding
  - Creates `currentActiveAltActions` dictionary mapping full key (e.g., "F", "F00", "F01") to action
- For each action in `currentActiveAltActions`:
  - Creates a `GuiLabel` with `ThemeName::ShortcutKey` theme
  - Sets label text to the key string
  - Adds label to the action's composition
  - Stores label in `currentActiveAltTitles` dictionary
- Calls `FilterTitles` to update visibility

#### Key Input Handling (`EnterAltKey`)

Processing typed characters in ALT mode:

- Appends typed character to `currentAltPrefix`
- Checks if `currentAltPrefix` matches any action key in `currentActiveAltActions`
- If no match: calls `FilterTitles` and removes last character if no visible titles remain
- If exact match found:
  - If action has `GetActivatingAltHost()`: calls `EnterAltHost` to enter nested ALT host
  - Otherwise: calls `CloseAltHost` to exit ALT mode
  - Calls `action->OnActiveAlt()` to activate the action (usually focuses the control)
  - Sets `supressAltKey` to prevent key from being processed further
  - Returns true

#### Label Filtering (`FilterTitles`)

Updating label visibility based on current prefix:

- Iterates through all labels in `currentActiveAltTitles`
- If label's key starts with `currentAltPrefix`: shows label and formats text with brackets around next character (e.g., "F[0]0")
- Otherwise: hides label
- Returns count of visible labels

#### Exiting ALT Mode

Ways to exit ALT mode:

- Press ESCAPE: calls `LeaveAltHost` to exit current host and restore previous
- Press BACKSPACE: calls `LeaveAltKey` to remove last character from prefix
- Clicking or any other key: depends on whether key matches an action
- `CloseAltHost`: clears all state, deletes all labels, and exits all hosts in the hierarchy

#### Character and Key Suppression

Input suppression while in ALT mode:

- `GuiAltActionManager::Char` returns true to suppress all character input while in ALT mode or if `supressAltKey` is set
- `GuiAltActionManager::KeyUp` suppresses the key-up event for the key stored in `supressAltKey`

### Nested ALT Hosts

#### Why Nested ALT Hosts Are Needed

The `continue` statement in `CollectAltActionsFromControl` creates a "barrier" when a control has its own ALT action. This prevents the control's children from being collected at the parent level. Nested ALT hosts provide a mechanism to enter the control's context and collect children's ALT actions at a nested level.

**Design Rationales for Custom `GetActivatingAltHost` Implementations:**

1. **Non-Child Control Relationships** (`GuiMenuButton`):
   - Problem: A menu button's submenu is a separate popup window, NOT a child control
   - Default collection only traverses child controls, cannot reach submenu items
   - Solution: Returns `subMenu->QueryTypedService<IGuiAltActionHost>()` to switch context to submenu window
   - Result: Pressing ALT+[key] on menu button enters submenu's ALT context

2. **Intentional Blocking** (`GuiComboBoxBase`):
   - Problem: UX design requires ALT key to just open dropdown, NOT enter nested navigation
   - Default would collect ALT actions from dropdown's controls
   - Solution: Explicitly returns `GuiSelectableButton::GetActivatingAltHost()` (usually null)
   - Result: ALT key only opens dropdown without entering nested mode

3. **Dynamic/Temporary Content** (`GuiVirtualDataGrid`):
   - Problem: Cell editor is created on-demand, not a permanent child
   - Default collection only sees permanent children, misses temporary editor
   - Solution: When `currentEditor` exists and has ALT action, calls `SetAltComposition(currentEditor->GetTemplate())` and `SetAltControl(focusControl, true)`, returns `this`
   - Result: Can navigate within cell editor using ALT keys

4. **Scoped Navigation for Dense Control Groups** (`GuiRibbonGroup`):
   - Problem: Ribbon groups contain many buttons, exposing all at window level creates too many conflicts
   - Default would collect all buttons at same level as the group (if group didn't have its own ALT action)
   - But: `GuiRibbonGroup` has its own ALT action (from `GuiControl` base), which blocks children via `continue`
   - Solution: Returns `this` when `IsAltAvailable()` is true, creating two-level navigation
   - Result: First press ALT+[group-key] to enter group, then press ALT+[button-key] to select button
   - Benefit: Reduces conflicts and creates logical grouping

5. **Optional Scoped Navigation** (`GuiDatePicker`):
   - Problem: Date picker displays calendar with many clickable date cells, needs optional nested navigation
   - Solution: Returns `this` when `nestedAlt` constructor parameter is true
   - Result: When enabled, pressing ALT+[key] enters calendar's ALT context for date selection

#### The Critical Role of the `continue` Barrier

The `continue` statement in `CollectAltActionsFromControl` serves as a crucial design element:

- **Without nested hosts**: When a control has an ALT action, the `continue` prevents children from being collected, making them unreachable
- **With nested hosts**: `GetActivatingAltHost` provides a way to "un-hide" children by entering a nested context that re-collects them
- **For containers like `GuiRibbonGroup`**: This creates hierarchical navigation instead of flat navigation, reducing ALT key conflicts and improving scalability for interfaces with many controls

## Event Flow Integration

### Key Event Processing Chain

`GuiGraphicsHost::KeyDown` processes native window key events in this order:

1. First tries `GuiAltActionManager::KeyDown`: if ALT manager is active or activates, returns true to consume event
2. Then tries `GuiTabActionManager::KeyDown`: if TAB key is handled, returns true to consume event
3. Then tries shortcut key manager (if exists)
4. Finally delivers to focused composition's event receiver if not consumed

### Character Event Processing Chain

`GuiGraphicsHost::Char` processes character input in this order:

1. First tries `GuiTabActionManager::Char`: suppresses TAB character if just navigated
2. Then tries `GuiAltActionManager::Char`: suppresses all input while in ALT mode
3. Finally delivers to focused composition's event receiver if not suppressed

### Control Visibility and Enable State

`GuiControl::IsControlVisibleAndEnabled` determines if a control can receive focus:

- Traverses up parent chain checking both `GetVisible()` and `GetEnabled()`
- Used by both `IsAltEnabled()` and `IsTabEnabled()` to determine if control can receive focus via ALT or TAB
- Ensures that if a parent is disabled or invisible, all children are excluded from ALT/TAB navigation

## Test Coverage

### TAB Navigation Tests

`TestApplication_Tab.cpp` contains comprehensive TAB navigation tests:

- `TabNavigate`: Tests forward TAB through 3 buttons and backward SHIFT+TAB
- `TabNavigateWithContainer`: Tests TAB through 5 buttons including nested containers (GroupBox)
- `FocusedAndDisable`: Tests that disabling focused control clears focus
- `FocusedAndHide`: Tests hiding focused control (focus behavior noted as unclear in comment)
- `FocusedAndDelete`: Tests deleting focused control clears focus

### ALT Focus Test

`TestControls_CoreApplication_GuiControl.cpp` contains `AltFocus` test:

- Tests ALT key activation in nested control scenarios
