# !!!SCRUM!!!

# DESIGN REQUEST

GuiListControl takes a pure data structure list::IItemProvider, converting it to an interactable and renderable control. Sub classes of GuiListControl requires the list::IItemProvider to offer more features via RequestView.

Similarily, a tree view control takes tree::INodeRootProvider. But in the end, a tree view is just a list control rendering all expanded tree items. So a tree view control needs to convert tree:INodeRootProvider to list::IItemProvider in order to construct its base class. tree::NodeItemProvider do this job.

Obviously, a tree::NodeItemProvider represents all expanded tree::INodeProvider nodes, which are managed by INodeRootProvider. The root node can be retrived via INodeRootProvider::GetRootNode. A tree::INodeProvider is either expanded or collapsed, controlled by GetExpanding and SetExpanding. If a node is collapsed, all sub tree are invisible to tree::NodeItemProvider. You must be also awared that the root node itself does not appear in tree::NodeItemProvider.

tree::INodeItemView offered a way to convert any visible tree::INodeProvide to its order in tree::NodeItemProvider (actually tree::NodeItemProvider implements this interface). Currently, in tree::INodeRootProvider, CanGetNodeByVisibleIndex and GetNodeByVisibleIndex is not in used so you can ignore it. tree::INodeProvider::CalculateTotalVisibleNodes calculates the number of all visible nodes including itself.

In this task, you need to pay attention to two category of tree::NodeItemProvider:
- To test the order between visible index and the node, they are dynamically changed after any node is expanded or collapsed
- To test list::IItemProvider retriving data offered by tree::INodeProvider, for example, GetTextValue and GetBindingValue and others.

Two implementation of tree::INodeRootProvider itself has been tested in TestItemProviders_TreeViewItem(Bindable)?RootProvider.cpp.

You will need to create 4 tasks:
- Create a TestItemProviders_NodeItemProvider.cpp in UnitTest project, put it in the Source Files solution explorer folder
- Design and implement test cases for the first category (visible index and node mapping)
- Design and implement test cases for the second category (data retrieval with TreeViewItemRootProvider)
- Design and implement a few simple integration test cases with TreeViewItemBindableRootProvider

You can refer to every TestItemProviders_*.cpp for knowledges and operations and API usage for related classes and interfaces, and how I want you to organize test cases.

You do not need to implement INodeRootProvider by yourself, you can just use TreeViewItemRootProvider and TreeViewItemBindableRootProvider. As they have been tested already, they can release your work to help focusing on tree::NodeItemProvider which is being tested

# UPDATES

## UPDATE

You will also need to run some test for expanding/collapsing/changing children container to an invisible node, which should not affect tree::NodeItemProvider in anyway

## UPDATE

Just to clarify, since the test subject is tree::NodeItemProvider, so the only callback you need to keep tracking to is IItemProviderCallback

## UPDATE

Keep task 3 using TreeViewItemRootProvider only, you can create a task 4 for integration with TreeViewItemBindableRootProvider. But at the moment, both NodeItemProvider and TreeViewItemBindableRootProvider are tested, you can just make a few (or maybe just one or two) simple cases in task 4

## UPDATE

I believe we can split Task No.3 into smaller tasks. I would like to keep any task to only add a few hundreds of code. Maybe one task for each test category

## UPDATE

I believe we can split Task No.2 into smaller tasks. I would like to keep any task to only add a few hundreds of code. Maybe one task for each test category

# TASKS

- [x] TASK No.1: Create TestItemProviders_NodeItemProvider.cpp and Add to Project
- [x] TASK No.2: Implement Test Cases for Basic Index Mapping
- [ ] TASK No.3: Implement Test Cases for Expand/Collapse Dynamics
- [ ] TASK No.4: Implement Test Cases for Edge Cases and Complex Scenarios
- [ ] TASK No.5: Implement Test Cases for Operations on Invisible Nodes
- [ ] TASK No.6: Implement Test Cases for Basic IItemProvider Data Retrieval
- [ ] TASK No.7: Implement Test Cases for Data Retrieval After Structural Changes
- [ ] TASK No.8: Implement Test Cases for TreeViewItemRootProvider Integration
- [ ] TASK No.9: Implement Test Cases for Callback Integration
- [ ] TASK No.10: Add Simple Integration Test Cases with TreeViewItemBindableRootProvider

## TASK No.1: Create TestItemProviders_NodeItemProvider.cpp and Add to Project

This task establishes the infrastructure for testing `tree::NodeItemProvider` by creating a new test file and integrating it into the UnitTest project build system.

### what to be done

- Create `TestItemProviders_NodeItemProvider.cpp` in `Test\GacUISrc\UnitTest\` directory
- Add basic test file structure with necessary includes:
  - Include `TestItemProviders.h`
  - Include `tree::NodeItemProvider` header
  - Use proper namespaces (`gacui_unittest_template`)
- Add a simple smoke test to verify the file compiles and the basic setup works:
  - Create a `TEST_FILE` block
  - Add a simple test case that constructs a `NodeItemProvider` with a `TreeViewItemRootProvider`
  - Verify basic operations like `Count()` return expected values
- Update `UnitTest.vcxproj` to include the new `.cpp` file in the `<ClCompile>` section after `TestItemProviders_TreeViewItemBindableRootProvider.cpp`
- Update `UnitTest.vcxproj.filters` to place the file in the "Source Files" filter group, following the pattern of other `TestItemProviders_*.cpp` files

### how to test it

- Build the UnitTest project to ensure the new file compiles without errors
- Run the smoke test case to verify:
  - The `NodeItemProvider` can be constructed with a `TreeViewItemRootProvider`
  - Basic API calls like `Count()` work correctly
  - The file is properly integrated into the test suite

### rationale

This task must come first as it establishes the foundation for all subsequent testing work. Without the test file created and properly integrated into the build system, no actual test cases can be written or executed.

The smoke test ensures that:
- The file is correctly added to both the project file and filters file
- All necessary headers and dependencies are properly included
- The basic `NodeItemProvider` construction and API work as expected

Evidence from codebase:
- All other `TestItemProviders_*.cpp` files follow this pattern (seen in `TestItemProviders_TextItemProvider.cpp`, `TestItemProviders_TreeViewItemRootProvider.cpp`, etc.)
- The project structure requires both `.vcxproj` and `.vcxproj.filters` to be updated for new source files
- The `tree::NodeItemProvider` class is defined in `Source\Controls\ListControlPackage\DataSourceImpl_IItemProvider_NodeItemProvider.h`

## TASK No.2: Implement Test Cases for Basic Index Mapping

This task implements test cases for the most fundamental aspect of `tree::NodeItemProvider`: the basic bidirectional mapping between visible indices and tree nodes in static (non-changing) tree structures.

### what to be done

Implement a test category "BasicIndexMapping" covering these scenarios:

- Test `RequestNode(vint index)` with a flat list of nodes (all at root level, all collapsed)
- Test `RequestNode(vint index)` with a simple expanded hierarchy (1-2 levels deep)
- Test `CalculateNodeVisibilityIndex(INodeProvider* node)` returns correct indices for visible nodes in a flat tree
- Test `CalculateNodeVisibilityIndex(INodeProvider* node)` returns correct indices for visible nodes in a simple expanded hierarchy
- Test `CalculateNodeVisibilityIndex(INodeProvider* node)` returns -1 for nodes in collapsed subtrees
- Verify that the root node itself is excluded from visible indices (requesting index 0 should return the first child of root)
- Test `Count()` returns correct value for flat trees
- Test `Count()` returns correct value for simple hierarchies with some expanded nodes

This task should add approximately 100-150 lines of test code.

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` and wrap it with `NodeItemProvider`
- Build simple tree structures (2-3 levels max) using `CreateTreeViewItem` helper
- Use `MockItemProviderCallback` to monitor `OnItemModified` events from the `NodeItemProvider` (since `tree::NodeItemProvider` implements `list::IItemProvider`, only `IItemProviderCallback` is relevant for testing)
- Verify `Count()`, `RequestNode()`, and `CalculateNodeVisibilityIndex()` return expected values
- Test with static tree structures (no expand/collapse operations during testing)
- **Code Style (from Task 1 learning):**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method being tested (e.g., "// Verify operation through INodeItemView interface")
  - Use blank lines between conceptually different test operations for clarity
  - Prioritize simple, direct code over defensive over-engineering

Organize under a `TEST_CATEGORY` block named "BasicIndexMapping".

### rationale

This task establishes the foundation for all index mapping tests by focusing only on static scenarios:

1. **Core Responsibility**: The primary purpose of `NodeItemProvider` is to make tree nodes accessible as a flat list for `GuiListControl`, which is evidenced by its implementation of both `list::ItemProviderBase` and `tree::INodeItemView`

2. **Manageable Scope**: By limiting to static tree structures (no expand/collapse during the test), this task adds only ~100-150 lines of code, making it easier to review and debug

3. **Foundation for Dynamic Tests**: Later tasks will build on these basic tests by adding expand/collapse dynamics

Evidence from implementation:
- `RequestNode()` uses `GetNodeByOffset()` which recursively traverses only expanded nodes
- `CalculateNodeVisibilityIndexInternal()` returns -2 for nodes in collapsed subtrees
- The root node is explicitly excluded (visible indices start from root's children)

This task should be completed first (after Task 1) because understanding the basic index mapping is essential for all subsequent tests.

## TASK No.3: Implement Test Cases for Expand/Collapse Dynamics

This task implements test cases for the dynamic behavior of `tree::NodeItemProvider`: how the visible index mapping changes when nodes are expanded or collapsed.

### what to be done

Implement a test category "ExpandCollapseDynamics" covering these scenarios:

- Test that expanding a collapsed node correctly increases `Count()` and adjusts visible indices of subsequent nodes
- Test that collapsing an expanded node correctly decreases `Count()` and adjusts visible indices
- Test visible index calculations with partially expanded multi-level trees (some branches expanded, others collapsed)
- Test that expanding/collapsing nested nodes correctly updates the entire visible index range
- Verify that `OnItemModified` callbacks from the `IItemProvider` interface are triggered with correct parameters during expand/collapse
- Test expanding a node at the beginning, middle, and end of the visible list
- Test collapsing a node at the beginning, middle, and end of the visible list
- Test multiple expand/collapse operations in sequence and verify indices remain consistent

This task should add approximately 150-200 lines of test code.

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` and wrap it with `NodeItemProvider`
- Build tree structures (2-3 levels) using `CreateTreeViewItem` helper
- Use `MockItemProviderCallback` to monitor `OnItemModified` events from the `NodeItemProvider`
- Record initial `Count()` and node positions
- Use `SetExpanding()` on nodes to test expand/collapse scenarios
- Verify `Count()`, `RequestNode()`, and `CalculateNodeVisibilityIndex()` return updated values after each operation
- Clear callback logs between test phases to isolate specific behaviors
- Use `AssertCallbacks()` to verify correct callback sequences with correct parameters (start, count, newCount)
- **Code Style (from Task 1 learning):**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method being tested
  - Use blank lines between conceptually different test operations for clarity
  - Prioritize simple, direct code over defensive over-engineering

Organize under a `TEST_CATEGORY` block named "ExpandCollapseDynamics".

### rationale

This task verifies the dynamic behavior that makes `NodeItemProvider` useful in real applications:

1. **Real-World Usage**: Tree controls constantly expand and collapse nodes in response to user actions, so this dynamic behavior is critical

2. **Complex State Management**: The implementation in `DataSourceImpl_IItemProvider_NodeItemProvider.cpp` shows sophisticated logic in:
   - `OnItemExpanded/OnItemCollapsed()` - updating visible ranges when state changes
   - `InvokeOnItemModified()` - notifying listeners about visible range changes

3. **Event-Driven Architecture**: Controls depend on `OnItemModified` callbacks to know when to refresh, so correct callback parameters are essential

4. **Builds on Task 2**: Task 2 verified basic mapping works; this task verifies it updates correctly with state changes

Evidence from implementation:
- `OnItemExpanded/OnItemCollapsed()` calls `InvokeOnItemModified()` to notify listeners about visible range changes
- Parameters to callbacks are carefully calculated based on the position and size of the expanded/collapsed subtree

This task should come after Task 2 because it builds on the foundation of static index mapping.

## TASK No.4: Implement Test Cases for Edge Cases and Complex Scenarios

This task implements test cases for edge cases and complex scenarios in `tree::NodeItemProvider`, ensuring robust behavior in unusual or extreme conditions.

### what to be done

Implement a test category "EdgeCasesAndComplexScenarios" covering these scenarios:

- Test `RequestNode` with invalid indices (negative, beyond count) - should handle gracefully
- Test `CalculateNodeVisibilityIndex` with null node pointer
- Test `CalculateNodeVisibilityIndex` with a node that doesn't belong to the tree
- Test visible indices in a deeply nested tree (4+ levels) with mixed expand/collapse states
- Test that adding/removing nodes to/from expanded parents correctly updates visible indices
- Test that modifications to collapsed subtrees don't affect visible indices of the parent tree
- Test expand/collapse operations on trees with many nodes (stress test with 50+ nodes)
- Test a tree where all nodes are collapsed (only root's children visible)
- Test a tree where all nodes are fully expanded (all levels visible)

This task should add approximately 100-150 lines of test code.

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` and wrap it with `NodeItemProvider`
- Build various tree structures including deeply nested trees (4+ levels) and trees with many nodes
- Use `MockItemProviderCallback` to monitor `OnItemModified` events
- Test boundary conditions and invalid inputs
- Verify that adding/removing children to expanded nodes triggers correct callbacks and index updates
- Verify that operations on collapsed subtrees don't affect the visible list
- Use `AssertCallbacks()` to verify correct callback sequences
- **Code Style (from Task 1 learning):**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method being tested
  - Use blank lines between conceptually different test operations for clarity
  - Prioritize simple, direct code over defensive over-engineering

Organize under a `TEST_CATEGORY` block named "EdgeCasesAndComplexScenarios".

### rationale

This task ensures robustness and correctness in edge cases:

1. **Error Handling**: Real applications may pass invalid indices or null pointers, so the implementation must handle these gracefully without crashing

2. **Scalability**: Testing with deeply nested trees and many nodes ensures the implementation scales to real-world tree sizes

3. **Completeness**: This task covers scenarios not covered in Tasks 2 and 3, ensuring comprehensive test coverage

Evidence from implementation:
- `RequestNode()` uses `GetNodeByOffset()` which must handle invalid indices
- Adding/removing children triggers callbacks through the `INodeRootProvider` interface
- Collapsed subtrees are explicitly excluded from visible index calculations

This task should come after Tasks 2 and 3 because it tests edge cases on top of the basic and dynamic functionality verified in those tasks.

## TASK No.5: Implement Test Cases for Operations on Invisible Nodes

This task implements test cases to verify that operations on invisible nodes (in collapsed subtrees) do not affect `tree::NodeItemProvider` in any way, ensuring correct isolation between visible and invisible portions of the tree.

### what to be done

Implement a test category "OperationsOnInvisibleNodes" covering these scenarios:

- Test that expanding a node that is currently invisible (its parent is collapsed) does not trigger any callbacks to `tree::NodeItemProvider`
- Test that collapsing a node that is currently invisible (its parent is collapsed) does not trigger any callbacks to `tree::NodeItemProvider`
- Test that adding/removing children to/from an invisible node (in a collapsed subtree) does not affect `Count()` of `tree::NodeItemProvider`
- Test that the expand state of invisible nodes is preserved (when parent is later expanded, the previously expanded invisible node's children become visible)
- Test that multiple structural changes to collapsed subtrees do not cause any `OnItemModified` events on `tree::NodeItemProvider`
- Verify that when a collapsed parent containing modified invisible nodes is expanded, the correct structure is reflected in visible indices
- Test that `CalculateNodeVisibilityIndex()` returns -1 for nodes in invisible subtrees regardless of their own expand state

This task should add approximately 100-150 lines of test code.

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` and wrap it with `NodeItemProvider`
- Build tree structures (3-4 levels) using `CreateTreeViewItem` helper
- Use `MockItemProviderCallback` to monitor `OnItemModified` events
- Collapse parent nodes to create invisible subtrees
- Perform operations on invisible nodes:
  - Expand/collapse invisible nodes via `SetExpanding()`
  - Add/remove children to invisible nodes
  - Modify properties of invisible nodes
- Verify that no callbacks are triggered and `Count()` doesn't change
- Clear callback logs to ensure isolation
- Later expand the parent and verify the invisible operations had the expected effect on the tree structure
- **Code Style (from Task 1 learning):**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method being tested
  - Use blank lines between conceptually different test operations for clarity
  - Prioritize simple, direct code over defensive over-engineering

Organize under a `TEST_CATEGORY` block named "OperationsOnInvisibleNodes".

### rationale

Testing operations on invisible nodes is critical because `tree::NodeItemProvider` should be completely unaware of and unaffected by changes in collapsed subtrees:

1. **Performance**: If `NodeItemProvider` reacts to every change in invisible nodes, it would waste CPU cycles recalculating indices for nodes that aren't displayed

2. **Correctness**: The `Count()` and visible indices should only reflect what's actually visible, not the state of hidden subtrees

3. **Event Discipline**: `OnItemModified` callbacks should only fire when the visible list actually changes, not when invisible nodes are modified

4. **State Preservation**: The tree structure maintains expand/collapse states even when nodes are invisible, so when a parent is later expanded, the child's previous state should be honored

Evidence from implementation:
- `OnItemExpanded/OnItemCollapsed()` checking `CalculateNodeVisibilityIndex()` before invoking callbacks
- The visibility check returns -1 for nodes in collapsed subtrees, preventing callback propagation
- The underlying `INodeRootProvider` managing the full tree structure independently of what's visible

This task should come after Tasks 2, 3, and 4 because it tests a specific isolation concern that builds on understanding of basic mapping, dynamics, and edge cases.

## TASK No.6: Implement Test Cases for Basic IItemProvider Data Retrieval

This task implements basic test cases to verify that `tree::NodeItemProvider` correctly retrieves and exposes data from underlying `tree::INodeProvider` nodes through the `list::IItemProvider` interface, using `TreeViewItemRootProvider` as the underlying provider.

### what to be done

Implement a test category "BasicDataRetrieval" covering these scenarios:

- Test `Count()` returns correct number of visible nodes (excluding root) for a flat tree structure (all nodes at root level, no children)
- Test `Count()` returns correct number for a simple 2-level expanded tree
- Test `Count()` returns correct number for a tree with some collapsed nodes
- Test `GetTextValue(vint itemIndex)` retrieves correct text from visible nodes at various positions (first, middle, last)
- Test `GetBindingValue(vint itemIndex)` retrieves correct binding values from visible nodes
- Test `GetTextValue()` and `GetBindingValue()` with invalid indices (negative, beyond count) return empty/default values
- Test `RequestView(identifier)` returns the `INodeItemView` interface when requested with the correct identifier
- Test `RequestView(identifier)` returns nullptr for unknown view types
- Verify `GetRoot()` returns the correct `INodeRootProvider` instance

This task should add approximately 100-200 lines of test code.

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` with simple, well-defined tree structures (2-3 levels max)
- Wrap it with `NodeItemProvider`
- Build trees with known text values and binding values for easy verification
- Call `GetTextValue()` and `GetBindingValue()` with various valid and invalid indices
- Verify returned values match expected data
- Test `Count()` against manually calculated visible node counts
- Use `RequestView()` with different identifiers and verify return values
- Call `GetRoot()` and verify it returns the original root provider
- **Code Style (from Task 1 learning):**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method (e.g., "// Retrieve data through IItemProvider interface")
  - Use blank lines to separate different test operations
  - Keep code simple and direct

Organize under a `TEST_CATEGORY` block named "BasicDataRetrieval".

### rationale

This task establishes the foundation for data retrieval testing by verifying the most basic operations:

1. **Core IItemProvider Methods**: Tests the fundamental contract of `IItemProvider` - `Count()`, `GetTextValue()`, `GetBindingValue()`, and `RequestView()`

2. **Static Scenarios**: Focuses on static tree structures (no expand/collapse during the test) to isolate basic data retrieval from dynamic behavior

3. **Manageable Scope**: By limiting to basic scenarios, this task adds only a few hundred lines of code, making it easier to review and debug

4. **Foundation for Complex Tests**: Later tasks will build on these basic tests by adding structural changes and integration aspects

Evidence from implementation:
- `GetTextValue(vint itemIndex)` calls `RequestNode(itemIndex)` and then `root->GetTextValue(node.Obj())`
- `GetBindingValue(vint itemIndex)` follows the same delegation pattern
- `Count()` calculates visible nodes using `root->GetRootNode()->CalculateTotalVisibleNodes() - 1`
- `RequestView()` checks the identifier and returns `this` for `INodeItemView::Identifier` or delegates to root provider

This task should come after Tasks 2-5 because it relies on the index-to-node mapping being correct. However, it only tests the simplest cases to keep the scope manageable.

## TASK No.7: Implement Test Cases for Data Retrieval After Structural Changes

This task implements test cases to verify that data retrieval through `tree::NodeItemProvider` works correctly after dynamic structural changes to the tree, such as expanding/collapsing nodes.

### what to be done

Implement a test category "DataRetrievalAfterStructuralChanges" covering these scenarios:

- Test that `GetTextValue()` returns correct values after a node is expanded (new children become visible)
- Test that `GetTextValue()` returns correct values after a node is collapsed (children become hidden)
- Test that `Count()` updates correctly after expand operations
- Test that `Count()` updates correctly after collapse operations
- Test that indices correctly map to the right nodes after multiple expand/collapse operations
- Test data retrieval at specific positions (e.g., nodes before the expanded node, nodes after the expanded node, newly visible children)
- Test that collapsing a parent doesn't affect data retrieval for nodes outside the collapsed subtree
- Test that expanding a deeply nested node correctly exposes all its children in the right order

This task should add approximately 150-250 lines of test code.

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` with a multi-level tree structure (3-4 levels)
- Wrap it with `NodeItemProvider`
- Build trees with distinct text values at each level for easy verification
- Record initial `Count()` and retrieve some text values as baseline
- Use `SetExpanding(true)` on collapsed nodes and verify:
  - `Count()` increases by the expected amount
  - `GetTextValue()` at new indices returns correct child node values
  - `GetTextValue()` at indices beyond the expansion point still returns correct values
- Use `SetExpanding(false)` on expanded nodes and verify:
  - `Count()` decreases by the expected amount
  - `GetTextValue()` at indices that were in the collapsed range are now different nodes
  - `GetTextValue()` for nodes outside the collapsed range remain correct
- Perform multiple expand/collapse operations in sequence and verify consistency
- **Code Style (from Task 1 learning):**
  - Call methods directly without explicit interface casting
  - Add comments to clarify which phase of testing is happening
  - Use blank lines between setup, operation, and verification phases

Organize under a `TEST_CATEGORY` block named "DataRetrievalAfterStructuralChanges".

### rationale

This task verifies that `NodeItemProvider`'s data retrieval remains correct when the visible node set changes dynamically:

1. **Real-World Usage**: In actual tree controls, users constantly expand and collapse nodes, so data retrieval must work correctly through these state changes

2. **Index Stability**: After expand/collapse, the mapping from indices to nodes changes, but each index must still return the correct data for its new node

3. **Builds on Previous Tasks**: Task 2 verified that index-to-node mapping updates correctly; Task 3 verified basic data retrieval works. This task combines both aspects

4. **Isolated Complexity**: By separating structural changes from integration testing, we keep each task focused and the code size manageable

Evidence from implementation:
- `OnItemExpanded/OnItemCollapsed()` in `NodeItemProvider` calls `InvokeOnItemModified()` to notify about range changes
- `GetNodeByOffset()` recalculates the node-to-index mapping based on current expand states
- The visible node count changes dynamically with `CalculateTotalVisibleNodes()`

This task should come after Task 6 because it extends basic data retrieval with dynamic scenarios, building on the foundation of static tests.

## TASK No.8: Implement Test Cases for TreeViewItemRootProvider Integration

This task implements test cases to verify that `tree::NodeItemProvider` correctly integrates with and delegates to `TreeViewItemRootProvider`, ensuring the adapter pattern works correctly.

### what to be done

Implement a test category "TreeViewItemRootProviderIntegration" covering these scenarios:

- Test that `NodeItemProvider` correctly delegates `GetTextValue(itemIndex)` to `root->GetTextValue(node)` by comparing direct root provider calls with delegated calls
- Test that `NodeItemProvider` correctly delegates `GetBindingValue(itemIndex)` to `root->GetBindingValue(node)` by comparing results
- Test that data returned via `NodeItemProvider` matches what would be obtained by:
  1. Using `RequestNode(index)` to get the node
  2. Directly calling `root->GetTextValue(node)` with that node
- Test that changes to the underlying tree items (via `TreeViewItem::SetText()`) are correctly reflected in subsequent `GetTextValue()` calls through `NodeItemProvider`
- Test that `GetRoot()` returns the same `INodeRootProvider` instance that was passed to the constructor
- Verify that `NodeItemProvider` doesn't cache data but always delegates to the root provider for fresh data

This task should add approximately 100-150 lines of test code.

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` with a simple tree (2-3 levels)
- Wrap it with `NodeItemProvider`
- For specific indices:
  - Retrieve data via `nodeItemProvider->GetTextValue(index)`
  - Use `nodeItemProvider->RequestNode(index)` to get the node
  - Call `rootProvider->GetTextValue(node)` directly
  - Verify both approaches return the same value
- Modify tree item data directly (e.g., `treeItem->SetText(L"New Text")`)
- Retrieve the value again through `NodeItemProvider` and verify it reflects the change
- Call `GetRoot()` and verify it's the same object as the original root provider
- **Code Style (from Task 1 learning):**
  - Call methods directly without explicit interface casting
  - Add comments explaining the delegation verification approach
  - Use clear variable names for root provider, node provider, and nodes

Organize under a `TEST_CATEGORY` block named "TreeViewItemRootProviderIntegration".

### rationale

This task verifies the correct implementation of the adapter pattern:

1. **Delegation Correctness**: `NodeItemProvider` must correctly delegate to the underlying `INodeRootProvider` for all data operations, transforming index-based access to node-based access

2. **No Data Caching**: Ensures `NodeItemProvider` doesn't cache data but always retrieves fresh values from the root provider

3. **Reference Integrity**: Verifies that `GetRoot()` returns the correct root provider, which is important for controls that need to access the root provider directly

4. **Adapter Pattern Validation**: This tests the core architectural pattern - `NodeItemProvider` is just a view over the root provider, not a data owner

Evidence from implementation:
- `GetTextValue(vint itemIndex)` implementation: `return root->GetTextValue(RequestNode(itemIndex).Obj())`
- `GetBindingValue(vint itemIndex)` follows identical pattern
- `GetRoot()` simply returns the `root` member variable
- No data caching logic exists in the implementation

This task should come after Tasks 6 and 7 because it verifies that the data retrieval tested in those tasks actually works through proper delegation, not through some coincidental behavior.

## TASK No.9: Implement Test Cases for Callback Integration

This task implements test cases to verify that `tree::NodeItemProvider` correctly manages `IItemProviderCallback` instances and fires appropriate events when the visible node set changes.

### what to be done

Implement a test category "CallbackIntegration" covering these scenarios:

- Test that `AttachCallback()` properly registers an `IItemProviderCallback`
- Test that `DetachCallback()` properly unregisters an `IItemProviderCallback`
- Test that callbacks receive `OnItemModified` events when a node is expanded
- Test that callbacks receive `OnItemModified` events when a node is collapsed
- Verify callback parameters are correct:
  - `start`: The index where visible nodes changed
  - `count`: The number of nodes that were visible before the change
  - `newCount`: The number of nodes that are visible after the change
  - `itemReferenceUpdated`: Should be false for expand/collapse (item references don't change)
- Test that multiple callbacks attached to the same provider all receive events
- Test that detached callbacks do not receive events after detachment
- Test that callbacks are fired in the correct order when multiple expand/collapse operations happen
- Test that data retrieval (`GetTextValue`, `GetBindingValue`) works correctly when called from within a callback handler

This task should add approximately 100-200 lines of test code.

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` and wrap with `NodeItemProvider`
- Use `MockItemProviderCallback` (or similar test mock) to track callback invocations
- Call `AttachCallback()` to register the mock
- Perform expand/collapse operations via `SetExpanding()`
- Verify that `OnItemModified` was called with correct parameters
- Use `AssertCallbacks()` or similar methods to verify callback sequences
- Clear callback logs between test phases to isolate specific behaviors
- Test detaching callbacks and verify events stop being received
- In some test cases, call `GetTextValue()` from within the callback to verify data retrieval works during event handling
- **Code Style (from Task 1 learning):**
  - Call methods directly without explicit interface casting
  - Add comments explaining what callbacks are expected for each operation
  - Use blank lines to separate different test phases

Organize under a `TEST_CATEGORY` block named "CallbackIntegration".

### rationale

This task verifies that `NodeItemProvider` correctly implements the observable pattern for list changes:

1. **Event-Driven Updates**: List controls depend on `OnItemModified` events to know when to refresh their display. Testing callbacks ensures this mechanism works

2. **Correct Parameters**: The parameters passed to `OnItemModified` tell the control exactly which range of items changed and how, enabling efficient updates

3. **Multiple Observers**: Multiple controls or components might observe the same provider, so callback management must be correct

4. **Callback Safety**: Data retrieval must work correctly even when called from within a callback handler, as this is a common pattern

Evidence from implementation:
- `OnItemExpanded/OnItemCollapsed()` calls `InvokeOnItemModified()` with calculated ranges
- `ItemProviderBase` base class manages callback registration through `AttachCallback/DetachCallback`
- Parameters to `InvokeOnItemModified()` are carefully calculated:
  - `start`: Uses `CalculateNodeVisibilityIndex()` to find where the change starts
  - `count`: Old visible count before expansion/collapse
  - `newCount`: New visible count after expansion/collapse
  - `itemReferenceUpdated`: Always false for expand/collapse

This task should come after Tasks 6, 7, and 8 because:
- It assumes basic data retrieval works (Task 6)
- It assumes structural changes work (Task 7)
- It may involve verifying delegation to root provider (Task 8)
- It's a higher-level integration concern that builds on lower-level functionality

## TASK No.10: Add Simple Integration Test Cases with TreeViewItemBindableRootProvider

This task adds a few simple test cases to verify that `tree::NodeItemProvider` works correctly with `TreeViewItemBindableRootProvider` as the underlying provider. Since both `NodeItemProvider` and `TreeViewItemBindableRootProvider` have been thoroughly tested individually, this task only needs to verify basic integration.

### what to be done

Implement one or two simple test cases covering:

**Basic Integration:**
- Create a `TreeViewItemBindableRootProvider` with a simple tree structure (2-3 levels)
- Wrap it with `NodeItemProvider`
- Test that `Count()` returns the correct number of visible nodes
- Test that `GetTextValue()` retrieves correct text values
- Test that `GetBindingValue()` retrieves correct binding values from bindable items
- Perform one expand/collapse operation and verify `Count()` updates correctly
- Verify that data retrieval still works after the structural change

The test should be minimal, just demonstrating that the two components work together correctly. No need for exhaustive testing since both components are already thoroughly tested.

### how to test it

Create a simple test case:
- Build a tree using `CreateBindableTreeViewItem` helper
- Create `TreeViewItemBindableRootProvider` and wrap with `NodeItemProvider`
- Verify basic operations: `Count()`, `GetTextValue()`, `GetBindingValue()`
- Expand one node and verify the visible count increases and data retrieval works
- Collapse the node and verify the visible count decreases
- **Code Style (from Task 1 learning):**
  - Use direct method calls without interface casting
  - Add interface-specific comments for clarity
  - Keep test simple since both components are already thoroughly tested

Organize under a `TEST_CATEGORY` block named "TreeViewItemBindableRootProviderIntegration" or similar.

### rationale

This task is intentionally lightweight because:

1. **Both Components Already Tested**: `TreeViewItemBindableRootProvider` has been tested in `TestItemProviders_TreeViewItemBindableRootProvider.cpp`, and `NodeItemProvider` is being tested in Tasks 2 and 3

2. **Simple Adapter Pattern**: `NodeItemProvider` is a straightforward adapter that delegates to whatever `INodeRootProvider` implementation it receives. The delegation mechanism is the same regardless of whether it's `TreeViewItemRootProvider` or `TreeViewItemBindableRootProvider`

3. **Minimal Risk**: Since both components are tested and the integration is through well-defined interfaces, there's minimal risk of integration issues

4. **Practical Verification**: A simple smoke test is sufficient to verify that:
   - The bindable provider's data is correctly accessed through `NodeItemProvider`
   - The expand/collapse mechanism works with bindable items
   - No unexpected interactions between the two components

Evidence from implementation:
- `NodeItemProvider` doesn't have any special logic for different root provider types
- All delegation is through the `INodeRootProvider` interface methods
- The behavior should be identical regardless of the concrete root provider implementation

This task should come last because:
- It depends on the test infrastructure established in Task 1
- It can reference patterns from Task 6 for data retrieval testing
- It's the least critical task and can be simplified if time is limited

# Impact to the Knowledge Base

## GacUI

No significant changes needed to the knowledge base. The testing of `tree::NodeItemProvider` follows established patterns and doesn't introduce new design concepts or APIs that need documentation. The class itself is an implementation detail of the tree view control system, and its behavior is well-defined by its interfaces (`list::ItemProviderBase` and `tree::INodeItemView`).

If anything needs to be added in the future, it would be under a potential "Choosing APIs" section for tree controls explaining:
- When to use `NodeItemProvider` directly (rarely, as it's typically managed by `GuiVirtualTreeListControl`)
- The relationship between `INodeRootProvider`, `NodeItemProvider`, and `GuiVirtualTreeListControl`

However, since this is an implementation detail rather than a user-facing API, documentation in the knowledge base is not critical at this time.

# !!!FINISHED!!!