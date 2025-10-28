# !!!KNOWLEDGE BASE!!!

# DOCUMENT REQUEST

## TOPIC

About what happens during control focus switching, and how to switch using TAB and ALT

### Insight

#### Focus Management Architecture

The focus management in GacUI is implemented across three main layers:

**Layer 1: GuiGraphicsComposition Level**
- `GuiGraphicsComposition` has an event receiver that fires `gotFocus` and `lostFocus` events
- `GuiGraphicsHost` maintains the currently focused composition in `focusedComposition` member
- `GuiGraphicsHost::SetFocusInternal` is the core method that switches focus between compositions
  - Fires `lostFocus` event on the currently focused composition (if any)
  - Updates `focusedComposition` to the new composition
  - Resets caret point to (0,0)
  - Fires `gotFocus` event on the newly focused composition
- `GuiGraphicsHost::SetFocus` is the public API that validates the composition belongs to this host and calls `SetFocusInternal`
- `GuiGraphicsHost::ClearFocus` removes focus from any composition
- `GuiGraphicsHost::GetFocusedComposition` returns the currently focused composition

**Layer 2: GuiControl Level**
- Each `GuiControl` has a `focusableComposition` property (can be null)
- `GuiControl::SetFocusableComposition` attaches `gotFocus` and `lostFocus` event handlers to the composition
- When composition gets focus: `GuiControl::OnGotFocus` is called, which sets `isFocused = true`, updates control template state, and fires `FocusedChanged` event
- When composition loses focus: `GuiControl::OnLostFocus` is called, which sets `isFocused = false`, updates control template state, and fires `FocusedChanged` event
- `GuiControl::GetFocused` checks if the control is currently focused
- `GuiControl::SetFocused` is the public API to focus a control:
  - Returns early if no focusable composition
  - Returns early if control is not visually enabled
  - Returns early if focusable composition is not eventually visible
  - Gets the graphics host from the focusable composition
  - Calls `host->SetFocus(focusableComposition)`

**Layer 3: Focus Loss on State Changes**
- When a control is disabled: `GuiControl::SetEnabled(false)` calls `UpdateVisuallyEnabled`, which triggers `OnLostFocus` if the control was focused
- When a control is hidden: `GuiControl::SetVisible(false)` does NOT automatically clear focus (based on test case comment)
- When a control is deleted: `GuiGraphicsHost::DisconnectCompositionInternal` is called recursively, which clears `focusedComposition` if it matches the disconnected composition

#### TAB Key Navigation

**GuiTabActionManager Architecture**
- Created and owned by `GuiGraphicsHost`
- Manages TAB key navigation between focusable controls

**IGuiTabAction Interface**
- Controls implement `IGuiTabAction` service to participate in TAB navigation
- Key methods:
  - `GetAcceptTabInput()`: Returns whether control accepts TAB character (if true, TAB key is NOT used for navigation but passed to control)
  - `GetTabPriority()`: Returns priority for TAB order (-1 means lowest priority, 0, 1, 2... are higher priorities)
  - `IsTabEnabled()`: Returns whether control can be tabbed to (checks if control is visible and enabled)
  - `IsTabAvailable()`: Returns whether control is available for TAB navigation (checks if focusable composition exists)

**Control List Building (BuildControlList)**
- Recursively collects all controls from the control host
- Uses `tab_focus::CollectControls` to traverse control hierarchy
- Groups controls by their `TabPriority` value
- Inserts controls in priority order: 0, 1, 2, ..., then -1 (default/unprioritized)
- For each control, recursively collects its children and inserts them after the parent
- Result is a flattened list `controlsInOrder` sorted by TAB priority

**Focus Navigation (GetNextFocusControl)**
- Rebuilds control list if cache is invalidated (`available == false`)
- Takes current focused control and offset (+1 for forward TAB, -1 for backward SHIFT+TAB)
- Finds start index in the control list
- Wraps around using modulo arithmetic: `(index + offset + count) % count`
- Loops through controls until finding one that is both `IsTabAvailable()` and `IsTabEnabled()`
- Returns the next focusable control or null if none found

**Key Event Handling (GuiTabActionManager::KeyDown)**
- Only processes TAB key when CTRL and ALT are NOT pressed
- If focused composition's control has `GetAcceptTabInput() == true`, TAB is NOT intercepted (returns false)
- Otherwise, calls `GetNextFocusControl` with offset based on SHIFT key state
- If next control found: calls `next->SetFocused()`, sets `supressTabOnce = true` to prevent TAB character from being inserted
- Returns true to indicate key was handled

**Character Suppression (GuiTabActionManager::Char)**
- Suppresses the TAB character (`\t`) if `supressTabOnce` is true
- Resets `supressTabOnce` to false after checking

**Cache Invalidation**
- `GuiTabActionManager::InvalidateTabOrderCache()` sets `available = false` and clears `controlsInOrder`
- Called when control's `TabPriority` changes via `GuiControl::SetTabPriority`
- Next TAB key press triggers rebuild

#### ALT Key Navigation

**GuiAltActionManager Architecture**
- Created and owned by `GuiGraphicsHost`
- Manages ALT-combined shortcut keys for controls

**IGuiAltAction Interface**
- Controls implement `IGuiAltAction` service to have ALT shortcuts
- Key methods:
  - `GetAlt()`: Returns the ALT key string (e.g., "F" for ALT+F)
  - `IsAltEnabled()`: Returns whether ALT action is enabled (checks visible and enabled state)
  - `IsAltAvailable()`: Returns whether ALT action is available (checks if focusable composition and alt string exist)
  - `GetAltComposition()`: Returns the composition to show the ALT key label on
  - `GetActivatingAltHost()`: Returns the alt host to enter when this action is activated (null means just activate)
  - `OnActiveAlt()`: Called when ALT key is activated (default implementation calls `SetFocused()`)
- `IGuiAltAction::IsLegalAlt` validates that ALT string contains only uppercase letters (A-Z) and digits (0-9)

**IGuiAltActionContainer Interface**
- Controls can implement this to provide multiple ALT actions
- `GetAltActionCount()` and `GetAltAction(index)` enumerate actions

**IGuiAltActionHost Interface**
- Hosts can be entered/exited, creating a hierarchy of ALT contexts
- `GetAltComposition()`: Returns the composition for this host
- `GetPreviousAltHost()`: Returns the parent host in the hierarchy
- `OnActivatedAltHost(previousHost)` and `OnDeactivatedAltHost()`: Called when entering/leaving
- `CollectAltActions(actions)`: Collects all ALT actions for this host
- `IGuiAltActionHost::CollectAltActionsFromControl` recursively traverses control tree:
  - If control has `IGuiAltActionContainer`: collects all actions from it
  - Else if control has `IGuiAltAction` and `IsAltAvailable()` and `IsAltEnabled()`: adds single action
  - Recursively processes all children

**ALT Mode Activation**
- Press ALT (KEY_MENU) without CTRL or SHIFT
- `GuiAltActionManager::KeyDown` checks if control host implements `IGuiAltActionHost`
- Calls `EnterAltHost` to enter ALT mode

**EnterAltHost Process**
- Calls `ClearAltHost` to clean up previous state
- Calls `host->CollectAltActions(actions)` to get all available actions
- If no actions available, calls `CloseAltHost` and exits
- Calls `host->OnActivatedAltHost(currentAltHost)` with previous host
- Updates `currentAltHost` to the new host
- Calls `CreateAltTitles` to create visual labels

**CreateAltTitles Process**
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

**Key Input Handling (EnterAltKey)**
- Appends typed character to `currentAltPrefix`
- Checks if `currentAltPrefix` matches any action key in `currentActiveAltActions`
- If no match: calls `FilterTitles` and removes last character if no visible titles remain
- If exact match found:
  - If action has `GetActivatingAltHost()`: calls `EnterAltHost` to enter nested ALT host
  - Otherwise: calls `CloseAltHost` to exit ALT mode
  - Calls `action->OnActiveAlt()` to activate the action (usually focuses the control)
  - Sets `supressAltKey` to prevent key from being processed further
  - Returns true

**FilterTitles Process**
- Iterates through all labels in `currentActiveAltTitles`
- If label's key starts with `currentAltPrefix`: shows label and formats text with brackets around next character (e.g., "F[0]0")
- Otherwise: hides label
- Returns count of visible labels

**Exiting ALT Mode**
- Press ESCAPE: calls `LeaveAltHost` to exit current host and restore previous
- Press BACKSPACE: calls `LeaveAltKey` to remove last character from prefix
- Clicking or any other key: depends on whether key matches an action
- `CloseAltHost` clears all state, deletes all labels, and exits all hosts in the hierarchy

**Character and Key Suppression**
- `GuiAltActionManager::Char` returns true to suppress all character input while in ALT mode or if `supressAltKey` is set
- `GuiAltActionManager::KeyUp` suppresses the key-up event for the key stored in `supressAltKey`

#### Event Flow Integration

**GuiGraphicsHost Key Event Processing**
- `GuiGraphicsHost::KeyDown` processes native window key events
- First tries `GuiAltActionManager::KeyDown`: if ALT manager is active or activates, returns true to consume event
- Then tries `GuiTabActionManager::KeyDown`: if TAB key is handled, returns true to consume event
- Then tries shortcut key manager (if exists)
- Finally delivers to focused composition's event receiver if not consumed

**GuiGraphicsHost Char Event Processing**
- `GuiGraphicsHost::Char` processes character input
- First tries `GuiTabActionManager::Char`: suppresses TAB character if just navigated
- Then tries `GuiAltActionManager::Char`: suppresses all input while in ALT mode
- Finally delivers to focused composition's event receiver if not suppressed

**Control Visibility and Enable State**
- `GuiControl::IsControlVisibleAndEnabled` traverses up parent chain checking both `GetVisible()` and `GetEnabled()`
- Used by both `IsAltEnabled()` and `IsTabEnabled()` to determine if control can receive focus via ALT or TAB
- This ensures that if a parent is disabled or invisible, all children are excluded from ALT/TAB navigation

#### Test Coverage

**TAB Navigation Tests**
- `TestApplication_Tab.cpp` contains comprehensive tests:
  - `TabNavigate`: Tests forward TAB through 3 buttons and backward SHIFT+TAB
  - `TabNavigateWithContainer`: Tests TAB through 5 buttons including nested containers (GroupBox)
  - `FocusedAndDisable`: Tests that disabling focused control clears focus
  - `FocusedAndHide`: Tests hiding focused control (focus behavior noted as unclear in comment)
  - `FocusedAndDelete`: Tests deleting focused control clears focus

**ALT Focus Test**
- `TestControls_CoreApplication_GuiControl.cpp` contains `AltFocus` test
- Tests ALT key activation in nested control scenarios

## ASK

Usually nested tab hosts are created based on nested controls. But Tab and Menu controls has implemented their own nested tab hosts. Is there any other control doing the same thing? What do they actually do?

### Insight

#### Controls Implementing Nested ALT Hosts

The following controls override `GetActivatingAltHost()` to provide nested ALT host functionality (not TAB host):

**1. GuiMenuButton** (`Source/Controls/ToolstripPackage/GuiMenuControls.h/.cpp`)
- Returns the submenu as a nested ALT host if it exists
- Implementation in `GuiMenuButton::GetActivatingAltHost`:
  - If `subMenu` exists: returns `subMenu->QueryTypedService<IGuiAltActionHost>()`
  - Otherwise: returns `GuiSelectableButton::GetActivatingAltHost()`
- When you press ALT key on a menu button with submenu, it enters the submenu's ALT context

**2. GuiComboBoxBase** (`Source/Controls/ListControlPackage/GuiComboControls.h/.cpp`)
- Does NOT provide nested ALT host even though it has dropdown
- Implementation in `GuiComboBoxBase::GetActivatingAltHost`:
  - Always returns `GuiSelectableButton::GetActivatingAltHost()`
  - Comment says: "When the combo box is opened by an Alt action, it will not continue into the dropdown"
- This prevents ALT navigation from entering the dropdown automatically

**3. GuiVirtualDataGrid** (`Source/Controls/ListControlPackage/GuiDataGridControls.h/.cpp`)
- Provides nested ALT host when cell editor is active
- Implementation in `GuiVirtualDataGrid::GetActivatingAltHost`:
  - If `currentEditor` exists and its focus control has `IGuiAltAction` service available and enabled:
    - Calls `SetAltComposition(currentEditor->GetTemplate())`
    - Calls `SetAltControl(focusControl, true)`
    - Returns `this` (the data grid itself becomes the ALT host)
  - Otherwise:
    - Calls `SetAltComposition(nullptr)` and `SetAltControl(nullptr, false)`
    - Returns `GuiVirtualListView::GetActivatingAltHost()`
- This allows ALT navigation into the active cell editor

**4. GuiRibbonGroup** (`Source/Controls/ToolstripPackage/GuiRibbonControls.h/.cpp`)
- Implements `GuiAltActionHostBase` and can be a nested ALT host itself
- Implementation in `GuiRibbonGroup::GetActivatingAltHost`:
  - If `IsAltAvailable()` returns true (when `alt` string is not empty):
    - Returns `this` (the ribbon group becomes the ALT host)
  - Otherwise:
    - Returns `GuiControl::GetActivatingAltHost()`
- This allows the ribbon group to have its own ALT context for its contained buttons

**5. GuiDatePicker** (`Source/Controls/GuiDateTimeControls.h/.cpp`)
- Implements `GuiAltActionHostBase` and can be a nested ALT host
- Implementation in `GuiDatePicker::GetActivatingAltHost`:
  - If `nestedAlt` is true (passed in constructor):
    - Returns `this` (the date picker becomes the ALT host)
  - Otherwise:
    - Returns `GuiControl::GetActivatingAltHost()`
- When `nestedAlt` is true, ALT keys navigate within the date picker's calendar cells
- Sets up ALT context with `SetAltComposition(boundsComposition)` and `SetAltControl(this, false)` in constructor

**6. GuiWindow** (`Source/Application/Controls/GuiWindowControls.h`)
- Implements `GuiAltActionHostBase` as the root ALT host
- All windows serve as the top-level ALT host for their contained controls

**Base Class: GuiControl** (`Source/Application/Controls/GuiBasicControls.h/.cpp`)
- Default implementation in `GuiControl::GetActivatingAltHost`:
  - Returns `activatingAltHost` member variable
  - This can be set by derived classes
- Most controls just inherit this default behavior

#### What Nested ALT Hosts Actually Do

When `GetActivatingAltHost()` returns a non-null `IGuiAltActionHost`:

**During ALT Mode Activation:**
- `GuiAltActionManager::EnterAltHost` is called with the returned host
- The host's `CollectAltActions` is called to gather all ALT actions from its children
- Visual ALT key labels are created for each action
- The user can type ALT keys to navigate within this nested context

**Key Differences from Normal ALT Navigation:**
- **Nested Context**: Instead of collecting all ALT actions from the entire window, only actions within the nested host are collected
- **Hierarchy**: You can press ESCAPE to exit the nested host and return to the parent ALT context
- **Focused Scope**: This restricts ALT navigation to a specific part of the UI (e.g., inside a menu, inside a ribbon group, inside a date picker calendar)

**Important Distinction:**
- There is NO "nested TAB host" concept in the codebase
- Only ALT navigation supports nesting through `IGuiAltActionHost`
- TAB navigation is flat and managed by a single `GuiTabActionManager` per window
- The question mentioned "nested tab hosts" but the implementation only supports nested ALT hosts

## ASK

Why the listed controls need to implement their own alt host? What is the reason behind it you believe that the default nested alt host handling can't achieve?

### Insight

#### Understanding Default ALT Host Behavior

**Default Behavior in GuiControl**
- `GuiControl::GetActivatingAltHost()` returns the `activatingAltHost` member variable (usually null)
- When null, no nested ALT host is created; ALT actions are just collected from the control hierarchy

**Default Collection Process (GuiAltActionHostBase::CollectAltActions)**
- Calls `CollectAltActionsFromControl(control, includeControl, actions)`
- This function recursively traverses ALL descendant controls in the control hierarchy
- For each control, if it has `IGuiAltActionContainer` or `IGuiAltAction` service, adds its actions
- This means by default, activating ALT on a control collects actions from all its permanent child controls

#### Why Custom GetActivatingAltHost Implementations Are Needed

**Problem 1: Non-Child Control Relationships (GuiMenuButton)**
- A menu button's submenu is a separate `GuiMenu` popup window, NOT a child control
- The submenu is in a different window hierarchy (it's a popup)
- Default `CollectAltActionsFromControl` only traverses child controls, so it cannot reach the submenu's items
- **Solution**: `GuiMenuButton::GetActivatingAltHost()` returns `subMenu->QueryTypedService<IGuiAltActionHost>()`
- This explicitly tells the ALT system to switch context to the submenu window's ALT host
- Result: When you press ALT+[key] on a menu button with submenu, you enter the submenu's ALT context and can navigate its items

**Problem 2: Intentional Blocking of Nested Navigation (GuiComboBoxBase)**
- A combo box also has a dropdown containing selectable items
- Unlike menus, the UX design dictates that pressing ALT+[key] should just open the dropdown, NOT enter ALT navigation inside it
- Default behavior would collect ALT actions from the dropdown's controls
- **Solution**: `GuiComboBoxBase::GetActivatingAltHost()` explicitly returns `GuiSelectableButton::GetActivatingAltHost()` (which is usually null)
- Comment in code: "When the combo box is opened by an Alt action, it will not continue into the dropdown"
- Result: ALT key activation only opens the dropdown, without entering nested ALT mode for dropdown items

**Problem 3: Dynamic/Temporary Content (GuiVirtualDataGrid)**
- When editing a cell, an editor control is dynamically created and shown
- The editor is NOT a permanent child of the grid; it's created on-demand and destroyed when editing ends
- The editor might be a complex control (dropdown, date picker, text box) with its own ALT shortcuts
- Default collection would only see permanent children, missing the temporary editor
- **Solution**: `GuiVirtualDataGrid::GetActivatingAltHost()` checks if `currentEditor` exists
  - If editor's focus control has available and enabled `IGuiAltAction`: calls `SetAltComposition(currentEditor->GetTemplate())` and `SetAltControl(focusControl, true)`, then returns `this`
  - This makes the grid itself become a temporary ALT host specifically for the editor
  - If no editor: returns `GuiVirtualListView::GetActivatingAltHost()` (default behavior)
- Result: When editing a cell and pressing ALT, you can navigate within the editor control

**Problem 4: Scoped Navigation for Dense Control Groups (GuiRibbonGroup)**
- A ribbon group contains many buttons (potentially dozens)
- If all buttons were at the same ALT level, there would be too many ALT key conflicts
- Ribbon groups can collapse into a dropdown when space is limited
- **Solution**: `GuiRibbonGroup::GetActivatingAltHost()` returns `this` when `IsAltAvailable()` is true (when alt string exists)
- The ribbon group implements `GuiAltActionHostBase` and sets up its composition/control
- Result: Two-level ALT navigation: first press ALT+[group-key] to enter the group, then press ALT+[button-key] to select a button
- This reduces conflicts and creates logical grouping

**Problem 5: Optional Scoped Navigation (GuiDatePicker)**
- A date picker displays a calendar with many clickable date cells
- Each date cell can have an ALT shortcut for keyboard navigation
- This feature is optional (controlled by `nestedAlt` constructor parameter)
- **Solution**: `GuiDatePicker::GetActivatingAltHost()` returns `this` when `nestedAlt` is true
- The date picker implements `GuiAltActionHostBase` with `SetAltComposition(boundsComposition)` and `SetAltControl(this, false)` in constructor
- Result: When `nestedAlt=true`, pressing ALT+[key] enters the calendar's ALT context for date selection

#### Why Default Handling Is Insufficient

**Limitation 1: Only Traverses Permanent Child Hierarchy**
- `CollectAltActionsFromControl` uses `GetChild(i)` to traverse children
- Cannot reach: separate popup windows (menus), dynamically created controls (grid editors), or non-child UI elements

**Limitation 2: Cannot Create Scoped Contexts**
- Default behavior collects ALL actions from ALL descendants into a flat list
- Cannot create hierarchical navigation where you enter a group first, then select within it
- Results in ALT key conflicts when too many controls exist

**Limitation 3: Cannot Block Navigation**
- Default behavior always collects from all available children
- Cannot prevent entering a nested context when UX design requires just activation without nesting (like combo boxes)

**Limitation 4: Cannot Handle Dynamic State**
- Default collection is based on static control hierarchy
- Cannot adapt to dynamic content (like temporary editors) that come and go

#### Summary of Custom Implementations

| Control | Purpose | What Default Can't Do |
|---------|---------|----------------------|
| `GuiMenuButton` | Enter submenu popup's ALT context | Submenu is a separate window, not a child |
| `GuiComboBoxBase` | Prevent entering dropdown's ALT context | Need to explicitly block nested navigation |
| `GuiVirtualDataGrid` | Enter temporary cell editor's ALT context | Editor is dynamic, not a permanent child |
| `GuiRibbonGroup` | Create scoped navigation for button groups | Need hierarchical ALT levels to reduce conflicts |
| `GuiDatePicker` | Optionally enter calendar's ALT context | Need configurable nested navigation |

## ASK

GuiRibbonGroup is also a container, so what is actually differences comparing to the default behavior in terms of alt handling?

### Insight

#### The Critical Difference: How CollectAltActionsFromControl Traverses Controls

The key to understanding this is in `IGuiAltActionHost::CollectAltActionsFromControl` in `Source/Application/GraphicsHost/GuiGraphicsHost_Alt.cpp`:

```cpp
while (index < controls.Count())
{
    auto current = controls[index++];

    if (current != control || includeThisControl)
    {
        if (auto container = current->QueryTypedService<IGuiAltActionContainer>())
        {
            // ... collect actions from container
            continue;  // ← STOPS HERE, children NOT traversed
        }
        else if (auto action = current->QueryTypedService<IGuiAltAction>())
        {
            if (action->IsAltAvailable() && action->IsAltEnabled())
            {
                actions.Add(action->GetAlt(), action);
                continue;  // ← STOPS HERE, children NOT traversed
            }
        }
    }

    // Only reached if control doesn't have IGuiAltAction or IGuiAltActionContainer
    vint count = current->GetChildrenCount();
    for (vint i = 0; i < count; i++)
    {
        controls.Add(current->GetChild(i));  // ← Children added only if no ALT action
    }
}
```

**The `continue` statement is critical**: When a control has `IGuiAltAction` (or `IGuiAltActionContainer`) service and it's available/enabled, the function adds the control's ALT action to the collection BUT SKIPS adding its children to the traversal queue. This means children's ALT actions are NOT collected.

#### GuiRibbonGroup's Two Behaviors

**Base Class Facts:**
- `GuiControl` (base class of all controls) implements `IGuiAltAction` (`Source/Application/Controls/GuiBasicControls.h`, line 78)
- `GuiRibbonGroup` inherits from both `GuiControl` and `GuiAltActionHostBase` (line 116 in `GuiRibbonControls.h`)
- All controls have `GetAlt()`, `SetAlt()`, `IsAltAvailable()`, etc. from `GuiControl`

**Behavior 1: When GuiRibbonGroup Has an ALT Key (alt string is NOT empty)**
- `GuiRibbonGroup::IsAltAvailable()` returns true (`Source/Controls/ToolstripPackage/GuiRibbonControls.cpp`, line 289)
- `GuiRibbonGroup::GetActivatingAltHost()` returns `this` (line 294-300)
- When parent ALT host calls `CollectAltActionsFromControl`:
  - Encounters `GuiRibbonGroup`, which has `IGuiAltAction` service
  - Calls `IsAltAvailable()` → returns true
  - Adds the group's own ALT action (e.g., "H" for Home group)
  - Executes `continue` → **SKIPS** adding the group's button children to traversal
  - Result: Only the group's ALT key is collected, NOT the buttons inside
- When user presses ALT+[group-key]:
  - `EnterAltKey` is called with the group's key
  - Finds the action, calls `action->GetActivatingAltHost()` → returns the group itself
  - Calls `EnterAltHost(group)` to enter nested context
  - Calls `group->CollectAltActions(actions)` which calls `CollectAltActionsFromControl(this, false, actions)`
  - Now traverses the group's children and collects button ALT actions
  - Visual ALT labels are created for buttons

**Behavior 2: When GuiRibbonGroup Has NO ALT Key (alt string is empty)**
- `GuiRibbonGroup::IsAltAvailable()` returns false (line 289: `return alt != L""`)
- `GuiRibbonGroup::GetActivatingAltHost()` returns `GuiControl::GetActivatingAltHost()` (usually null)
- When parent ALT host calls `CollectAltActionsFromControl`:
  - Encounters `GuiRibbonGroup`, which has `IGuiAltAction` service
  - Calls `IsAltAvailable()` → returns false
  - Does NOT add the group's action (no `continue` executed)
  - Falls through to add children to traversal queue
  - Result: The group's button children's ALT actions are collected at parent level

#### Default Behavior Comparison

**If GuiRibbonGroup Used Default Behavior (always returned null from GetActivatingAltHost):**
- Assume the group has alt string "H" and contains 3 buttons with alt strings "B", "C", "D"
- Parent calls `CollectAltActionsFromControl`:
  - Encounters group → `IsAltAvailable()` returns true → adds "H" action → executes `continue`
  - Children (buttons "B", "C", "D") are NOT traversed
- When user presses ALT at window level: sees only "H" for the group
- When user presses "H": calls `action->OnActiveAlt()` which calls `SetFocused()` on the group
- **Problem**: User cannot navigate to buttons inside using ALT keys because:
  - Buttons' ALT actions were never collected at parent level (blocked by `continue`)
  - No nested ALT host is entered, so no new collection happens

**With GuiRibbonGroup's Custom Implementation:**
- Same setup: group has "H", buttons have "B", "C", "D"
- Parent collection: sees only "H" (children blocked by `continue`)
- When user presses "H": calls `GetActivatingAltHost()` → returns `this` → calls `EnterAltHost(group)`
- `EnterAltHost` calls `group->CollectAltActions()` → collects "B", "C", "D" from buttons
- User can now type "B", "C", or "D" to navigate to buttons

#### Summary of Differences

| Aspect | Default Behavior (if GetActivatingAltHost returned null) | GuiRibbonGroup's Custom Behavior |
|--------|--------------------------------------------------------|----------------------------------|
| **Parent-level collection** | Only group's "H" collected; buttons blocked by `continue` | Only group's "H" collected; buttons blocked by `continue` |
| **After pressing ALT+H** | Just focuses the group, no further ALT navigation | Enters nested ALT context, collects button ALT actions |
| **Button ALT keys visible** | Never (blocked at parent level, no nested collection) | Yes (collected when entering group's nested context) |
| **Navigation levels** | Single-level (can only activate group) | Two-level (activate group → navigate within group) |
| **Why custom needed** | Default can't provide nested navigation when control itself has an ALT action | Custom creates nested context to allow button navigation |

#### The Core Insight

The reason `GuiRibbonGroup` needs custom `GetActivatingAltHost` is NOT because it's a container (all containers have children). It's because:

1. **The group itself implements `IGuiAltAction`** (inherited from `GuiControl`) and has its own ALT key
2. **Having an ALT action blocks child traversal** due to the `continue` statement in `CollectAltActionsFromControl`
3. **Default behavior would make buttons unreachable** via ALT navigation
4. **Custom behavior creates a nested context** that re-collects children's actions when the group is activated

This is fundamentally different from a plain container like `GuiGroupBox` or `GuiStackPanel` which don't have their own ALT actions, so their children are automatically collected at the parent level without needing nested contexts.

## ASK

Follow up the question in the last ASK, I think the default alt behavior collects every visible control, but ribbon group want to hide all children into its nested level, that is the reason, what do you think?

### Insight

#### Yes, Your Hypothesis Is Fundamentally Correct!

**Summary of Your Insight:**
You are **correct**. The default ALT behavior would collect every control from the control hierarchy at the top level. `GuiRibbonGroup` uses a nested ALT host to create a **two-level navigation structure** instead of exposing all button ALT keys at the parent level.

#### The Core Mechanism: Why Nested ALT Host Is Needed

**The Design Goal:**
- Ribbon groups can contain many buttons (potentially dozens in a typical ribbon interface)
- If all buttons' ALT keys were exposed at the window level, there would be too many ALT key conflicts
- Solution: Create a hierarchical navigation - first activate the group, then navigate within the group

**How It Works:**

**Level 1: Window-Level ALT Collection (When Group Has ALT Key)**
- `CollectAltActionsFromControl` encounters the ribbon group
- Ribbon group has `IGuiAltAction` service with `IsAltAvailable()` returning true
- Adds the ribbon group's own ALT action (e.g., "H" for Home group)
- Executes `continue` which **blocks traversal of the group's children**
- Result: Only the group's ALT key is visible at the window level, NOT the individual buttons

**Level 2: Group-Level ALT Collection (After Entering Group's Nested Context)**
- User presses ALT+H (the group's key)
- `GetActivatingAltHost()` returns `this` (the ribbon group becomes a nested ALT host)
- `EnterAltHost(group)` is called
- `group->CollectAltActions(actions)` collects ALT actions from the group's children
- Visual ALT labels are created for each button inside the group
- User can now type button ALT keys to navigate within the group

#### The Critical Role of the `continue` Statement

In `CollectAltActionsFromControl`:
```cpp
else if (auto action = current->QueryTypedService<IGuiAltAction>())
{
    if (action->IsAltAvailable() && action->IsAltEnabled())
    {
        actions.Add(action->GetAlt(), action);
        continue;  // ← Prevents children from being added to traversal queue
    }
}
```

**Why This Matters:**
- **Without the `continue`**: All controls with ALT actions AND their children would be collected at the same level
- **With the `continue`**: A control with an ALT action acts as a "barrier" - its children are not collected unless you enter its nested ALT context
- **For GuiRibbonGroup**: This `continue` prevents buttons from being collected at the window level, even though they may be control children

#### What Happens Without Custom GetActivatingAltHost

**If GuiRibbonGroup Used Default Behavior (returned null from GetActivatingAltHost):**
1. Window-level ALT collection sees only the ribbon group's "H" key (due to `continue`)
2. User presses ALT+H
3. `GetActivatingAltHost()` returns null (no nested context)
4. `action->OnActiveAlt()` is called, which just focuses the ribbon group
5. ALT mode exits because there's no nested host to enter
6. **Problem**: User can never access the buttons inside using ALT keys

**With GuiRibbonGroup's Custom Implementation:**
1. Window-level collection sees only "H"
2. User presses ALT+H
3. `GetActivatingAltHost()` returns `this`
4. `EnterAltHost(group)` creates a new ALT context
5. Buttons' ALT keys are collected and displayed
6. User can navigate to buttons

#### Summary: Why Your Understanding Is Correct

**Your Insight:** "The default alt behavior collects every visible control, but ribbon group wants to hide all children into its nested level"

**Why It's Correct:**
1. Default behavior WOULD collect all controls if there were no barriers
2. The `continue` statement creates a barrier when a control has its own ALT action
3. `GuiRibbonGroup` has its own ALT action, so the barrier is activated
4. This "hides" children from parent-level collection
5. Custom `GetActivatingAltHost` provides a way to "un-hide" them by entering a nested context
6. Result: Two-level navigation instead of flat navigation

**The Benefit:**
- Reduces ALT key conflicts at the window level
- Creates logical grouping (group → buttons within group)
- Scales better for interfaces with many controls (like ribbon interfaces)
