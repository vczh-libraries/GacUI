# !!!SCRUM!!!

# DESIGN REQUEST

GuiListControl takes a pure data structure list::IItemProvider, converting it to an interactable and renderable control. Sub classes of GuiListControl requires the list::IItemProvider to offer more features via RequestView.

Similarily, a tree view control takes tree::INodeRootProvider. But in the end, a tree view is just a list control rendering all expanded tree items. So a - Test that `GetTextValue()` returns correct values after a node is expanded (new children become visible)
- Test that `GetTextValue()` returns correct values after a node is collapsed (children become hidden)
- Test that `Count()` updates correctly after expand operations
- Test that `Count()` updates correctly after collapse operations
- Test that indices correctly map to the right nodes after multiple expand/collapse operations
- Test data retrieval at specific positions (e.g., nodes before the expanded node, nodes after the expanded node, newly visible children)
- Test that collapsing a parent doesn't affect data retrieval for nodes outside the collapsed subtree
- Test that expanding a deeply nested node correctly exposes all its children in the right order
- **NOTE**: Invalid indices should trigger `CHECK_ERROR` as established in Task 4 - do not test for empty/default return valuesw control needs to convert tree:INodeRootProvider to list::IItemProvider in order to construct its base class. tree::NodeItemProvider do this job.

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

## UPDATE

Check out TestItemProviders_NodeItemProvider.cpp, check out unfinished tasks, see if any test in unfinished tasks has already been implemented, remove such tests from unfinished tasks

## UPDATE

Since Task 6/7/8 already partially completed, please merge it into one
It looks like Task 9 is unnecessary, please remove it. But I think the DetachCallback needs a test case, please add it to the above merged task.

# TASKS

- [x] TASK No.1: Create TestItemProviders_NodeItemProvider.cpp and Add to Project
- [x] TASK No.2: Implement Test Cases for Basic Index Mapping
- [x] TASK No.3: Implement Test Cases for Expand/Collapse Dynamics
- [x] TASK No.4: Implement Test Cases for Edge Cases and Complex Scenarios
- [x] TASK No.5: Implement Test Cases for Operations on Invisible Nodes
- [x] TASK No.6: Implement Remaining Data Retrieval and Integration Test Cases
- [x] TASK No.7: Add Simple Integration Test Cases with TreeViewItemBindableRootProvider

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
- **Code Style (from Task 1 & 2 learnings):**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method being tested
  - Use blank lines between conceptually different test operations for clarity
  - Prioritize simple, direct code over defensive over-engineering
  - Use `AssertItems` helper with `const wchar_t*` arrays for compact and readable verification of visible node sequences
  - For empty lists, directly assert `Count() == 0` rather than using helper functions

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
- Test `CalculateNodeVisibilityIndex` with null node pointer (only if the API allows it without assertion)
- Test `CalculateNodeVisibilityIndex` with a node that doesn't belong to the tree
- Test visible indices in a deeply nested tree (4+ levels) with mixed expand/collapse states
- Test that adding/removing nodes to/from expanded parents correctly updates visible indices
- Test that modifications to collapsed subtrees don't affect visible indices of the parent tree
- Test expand/collapse operations on trees with many nodes (stress test with 50+ nodes)
- Test a tree where all nodes are collapsed (only root's children visible)
- Test a tree where all nodes are fully expanded (all levels visible)

This task should add approximately 100-150 lines of test code.

### how to test it

**BEFORE WRITING ANY TEST CODE:**
1. **Read the implementation** of the methods being tested (`RequestNode`, `CalculateNodeVisibilityIndex`, etc.) in `DataSourceImpl_IItemProvider_NodeItemProvider.cpp`
2. **Check existing test patterns** in other `TestItemProviders_*.cpp` files to understand expected return values for edge cases
3. **Verify actual behavior** by checking what the implementation returns for invalid inputs (e.g., `-1` vs `-2` vs `nullptr`)

All test cases should:
- Create a `TreeViewItemRootProvider` and wrap it with `NodeItemProvider`
- Build various tree structures including deeply nested trees (4+ levels) and trees with many nodes
- Use `MockItemProviderCallback` to monitor `OnItemModified` events
- Test boundary conditions and invalid inputs **based on actual implementation behavior, not assumptions**
- Verify that adding/removing children to expanded nodes triggers correct callbacks and index updates
- Verify that operations on collapsed subtrees don't affect the visible list
- Use `AssertCallbacks()` to verify correct callback sequences
- **Code Style (from completed tasks):**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method being tested
  - Use blank lines between conceptually different test operations for clarity
  - Prioritize simple, direct code over defensive over-engineering
  - Use `AssertItems` helper with `const wchar_t*` arrays for verifying visible node sequences
  - **IMPORTANT: Check return values from implementation before writing assertions** (e.g., `CalculateNodeVisibilityIndex()` returns `-1` for invisible nodes, not `-2`)
  - **Avoid testing ambiguous edge cases** where the expected behavior is unclear or undocumented

Organize under a `TEST_CATEGORY` block named "EdgeCasesAndComplexScenarios".

### rationale

This task ensures robustness and correctness in edge cases:

1. **Error Handling**: Real applications may pass invalid indices or null pointers, so the implementation must handle these gracefully without crashing

2. **Scalability**: Testing with deeply nested trees and many nodes ensures the implementation scales to real-world tree sizes

3. **Completeness**: This task covers scenarios not covered in Tasks 2 and 3, ensuring comprehensive test coverage

4. **Learning from Task 3**: Avoid testing edge cases with uncertain semantics. Focus on well-defined behaviors that matter in practice.

Evidence from implementation:
- `RequestNode()` uses `GetNodeByOffset()` which must handle invalid indices
- Adding/removing children triggers callbacks through the `INodeRootProvider` interface
- Collapsed subtrees are explicitly excluded from visible index calculations
- **Key insight from Task 3**: `CalculateNodeVisibilityIndex()` public method converts all negative internal results to `-1`, so tests should expect `-1` not `-2` for invisible nodes

This task should come after Tasks 2 and 3 because it tests edge cases on top of the basic and dynamic functionality verified in those tasks.

## TASK No.5: Implement Test Cases for Operations on Invisible Nodes

This task implements test cases to verify that operations on invisible nodes (in collapsed subtrees) do not affect `tree::NodeItemProvider` in any way, ensuring correct isolation between visible and invisible portions of the tree.

### what to be done

Implement a test category "OperationsOnInvisibleNodes" covering these scenarios:

- Test that adding/removing children to/from an invisible node (in a collapsed subtree) does not affect `Count()` of `tree::NodeItemProvider`
- Test that the expand state of invisible nodes is preserved (when parent is later expanded, the previously expanded invisible node's children become visible)
- Test that multiple structural changes to collapsed subtrees do not cause any `OnItemModified` events on `tree::NodeItemProvider`
- Verify that when a collapsed parent containing modified invisible nodes is expanded, the correct structure is reflected in visible indices
- Test that `CalculateNodeVisibilityIndex()` returns -1 for nodes in invisible subtrees regardless of their own expand state
- **NOTE**: Avoid testing expand/collapse operations on invisible nodes themselves, as this behavior has uncertain semantics (see learnings from Task 3)

This task should add approximately 100-150 lines of test code.

### how to test it

**BEFORE WRITING TESTS:**
1. **Do NOT test** `SetExpanding()` on invisible nodes unless you first verify the expected behavior in the implementation
2. **Focus on clear, practical scenarios** where the expected behavior is well-defined
3. **Avoid ambiguous edge cases** like "what happens when you expand an already-collapsed invisible node"

All test cases should:
- Create a `TreeViewItemRootProvider` and wrap it with `NodeItemProvider`
- Build tree structures (3-4 levels) using `CreateTreeViewItem` helper
- Use `MockItemProviderCallback` to monitor `OnItemModified` events
- Collapse parent nodes to create invisible subtrees
- Perform **well-defined operations** on invisible nodes:
  - Add/remove children to invisible nodes (clear expected behavior: no callbacks)
  - Modify properties of invisible nodes (if applicable)
  - **Consider carefully** whether to test expand/collapse of invisible nodes (learned from Task 3: this has uncertain semantics)
- Verify that no callbacks are triggered and `Count()` doesn't change
- Clear callback logs to ensure isolation
- Later expand the parent and verify the invisible operations had the expected effect on the tree structure
- **Code Style (from completed tasks):**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method being tested
  - Use blank lines between conceptually different test operations for clarity
  - Prioritize simple, direct code over defensive over-engineering
  - Use `AssertItems` helper with `const wchar_t*` arrays for verifying visible node sequences
  - **Focus on practical scenarios** that matter in real applications
  - **Remove any test** that has unclear expected behavior during implementation

Organize under a `TEST_CATEGORY` block named "OperationsOnInvisibleNodes".

### rationale

Testing operations on invisible nodes is critical because `tree::NodeItemProvider` should be completely unaware of and unaffected by changes in collapsed subtrees:

1. **Performance**: If `NodeItemProvider` reacts to every change in invisible nodes, it would waste CPU cycles recalculating indices for nodes that aren't displayed

2. **Correctness**: The `Count()` and visible indices should only reflect what's actually visible, not the state of hidden subtrees

3. **Event Discipline**: `OnItemModified` callbacks should only fire when the visible list actually changes, not when invisible nodes are modified

4. **State Preservation**: The tree structure maintains expand/collapse states even when nodes are invisible, so when a parent is later expanded, the child's previous state should be honored

5. **Learning from Task 3**: Avoid testing operations on invisible nodes where the expected behavior is not well-documented. The practical test case is: "collapse a parent with expanded children, then re-expand the parent - children should still be expanded" (already covered in `CollapseNodeWithExpandedDescendants` test from Task 3)

Evidence from implementation:
- `OnItemExpanded/OnItemCollapsed()` checking `CalculateNodeVisibilityIndex()` before invoking callbacks
- The visibility check returns -1 for nodes in collapsed subtrees, preventing callback propagation
- The underlying `INodeRootProvider` managing the full tree structure independently of what's visible
- **Key learning**: The semantics of calling `SetExpanding()` on invisible nodes is implementation-dependent and not clearly documented

This task should come after Tasks 2, 3, and 4 because it tests a specific isolation concern that builds on understanding of basic mapping, dynamics, and edge cases.

## TASK No.6: Implement Remaining Data Retrieval and Integration Test Cases

This task implements all remaining test cases for data retrieval through `list::IItemProvider` interface, delegation to `TreeViewItemRootProvider`, callback management through `IItemProviderCallback`, and integration verification. Many tests have already been implemented, so this task focuses on the remaining gaps.

**ANALYSIS OF ALREADY IMPLEMENTED TESTS:**
- ✅ `Count()` is extensively tested throughout BasicIndexMapping and ExpandCollapseDynamics
- ✅ `GetTextValue()` is tested via `AssertItems()` helper throughout all categories
- ✅ `GetTextValue()` after expand/collapse is tested throughout ExpandCollapseDynamics via `AssertItems()`
- ✅ `GetBindingValue()` with invalid indices is tested in EdgeCasesAndComplexScenarios::InvalidIndicesForDataRetrieval
- ✅ `GetTextValue()` with invalid indices is tested in EdgeCasesAndComplexScenarios::InvalidIndicesForDataRetrieval
- ✅ `GetRoot()` is tested in BasicSetupAndConstruction
- ✅ `AttachCallback()` is used throughout all test categories
- ✅ `OnItemModified` events on expand/collapse are verified throughout ExpandCollapseDynamics
- ✅ Callback parameters (start, count, newCount) are verified via `AssertCallbacks()` throughout all categories
- ✅ All previous tasks have been using `TreeViewItemRootProvider` as the underlying provider

**REMAINING GAPS:**
- ❌ `GetBindingValue()` with valid indices is NOT explicitly tested
- ❌ `GetBindingValue()` after structural changes (expand/collapse) is NOT explicitly tested
- ❌ `RequestView(identifier)` is NOT tested
- ❌ **Explicit validation** that data returned via `NodeItemProvider` matches direct calls to root provider is NOT tested
- ❌ Testing that `NodeItemProvider` doesn't cache data (always delegates) is NOT explicitly tested
- ❌ Testing that changes to underlying tree items are reflected in `NodeItemProvider` is NOT tested
- ❌ `DetachCallback()` functionality is NOT tested

### what to be done

Implement test cases covering the remaining scenarios organized into logical groups:

**1. Basic Data Retrieval (BasicDataRetrieval category):**
- Test `GetBindingValue(vint itemIndex)` retrieves correct binding values from visible nodes at various positions (first, middle, last)
- Test `GetBindingValue(vint itemIndex)` returns correct values after expand/collapse operations
- Test `RequestView(identifier)` returns the `INodeItemView` interface when requested with the correct identifier
- Test `RequestView(identifier)` returns nullptr for unknown view types

**2. TreeViewItemRootProvider Integration (TreeViewItemRootProviderIntegration category):**
- Test that data returned via `NodeItemProvider` matches what would be obtained by:
  1. Using `RequestNode(index)` to get the node
  2. Directly calling `root->GetTextValue(node)` with that node
  3. Directly calling `root->GetBindingValue(node)` with that node
- Test that changes to the underlying tree items (via `TreeViewItem::SetText()`) are correctly reflected in subsequent `GetTextValue()` calls through `NodeItemProvider`
- Verify that `NodeItemProvider` doesn't cache data but always delegates to the root provider for fresh data

**3. Callback Management (CallbackManagement category):**
- Test that `DetachCallback()` properly unregisters an `IItemProviderCallback`
- Test that detached callbacks do not receive events after detachment
- Test that multiple callbacks attached to the same provider all receive events

This task should add approximately 200-300 lines of test code total across all categories.

### how to test it

**For Basic Data Retrieval tests:**
- Create a `TreeViewItemRootProvider` with simple, well-defined tree structures (2-3 levels max)
- Wrap it with `NodeItemProvider`
- Build trees with known binding values for easy verification
- Call `GetBindingValue()` with various valid indices and verify returned values
- Perform expand/collapse operations and verify `GetBindingValue()` returns correct values for updated visible indices
- Use `RequestView()` with different identifiers (`INodeItemView::Identifier` and unknown ones) and verify return values
- **Code Style:**
  - Call methods directly without explicit interface casting unless the compiler requires it
  - Add comments specifying which interface provides each method
  - Use blank lines to separate different test operations
  - Keep code simple and direct

**For TreeViewItemRootProvider Integration tests:**
- Create a `TreeViewItemRootProvider` with a simple tree (2-3 levels)
- Wrap it with `NodeItemProvider`
- For specific indices:
  - Retrieve data via `nodeItemProvider->GetTextValue(index)` and `nodeItemProvider->GetBindingValue(index)`
  - Use `nodeItemProvider->RequestNode(index)` to get the node
  - Call `rootProvider->GetTextValue(node)` and `rootProvider->GetBindingValue(node)` directly
  - Verify both approaches return the same value
- Modify tree item data directly (e.g., `treeItem->SetText(L"New Text")`)
- Retrieve the value again through `NodeItemProvider` and verify it reflects the change
- Call `GetRoot()` and verify it's the same object as the original root provider
- **Code Style:**
  - Add comments explaining the delegation verification approach
  - Use clear variable names for root provider, node provider, and nodes

**For Callback Management tests:**
- Create a `TreeViewItemRootProvider` and wrap with `NodeItemProvider`
- Use `MockItemProviderCallback` to track callback invocations
- Attach multiple callbacks to the same provider
- Perform expand/collapse operations and verify all callbacks receive events
- Test detaching one callback and verify only remaining callbacks receive subsequent events
- Use `AssertCallbacks()` to verify callback sequences
- Clear callback logs between test phases to isolate specific behaviors
- **Code Style:**
  - Add comments explaining what callbacks are expected for each operation
  - Use blank lines to separate different test phases

Organize under `TEST_CATEGORY` blocks named "BasicDataRetrieval", "TreeViewItemRootProviderIntegration", and "CallbackManagement".

### rationale

This task consolidates all remaining data retrieval, integration, and callback testing into a single comprehensive task:

1. **Comprehensive Coverage**: Addresses all remaining gaps identified in the analysis, ensuring `tree::NodeItemProvider` is fully tested for its `list::IItemProvider` responsibilities

2. **Manageable Scope**: While this task covers multiple areas, each area has a small number of test cases (most work has already been done in previous tasks), keeping the total scope around 200-300 lines

3. **Logical Organization**: Groups related tests into three clear categories that each address a different aspect:
   - **BasicDataRetrieval**: Verifies the `IItemProvider` contract is correctly implemented
   - **TreeViewItemRootProviderIntegration**: Verifies the adapter pattern works correctly
   - **CallbackManagement**: Verifies the observable pattern works correctly

4. **Efficiency**: Merging these tasks avoids context switching and allows similar test infrastructure to be reused

Evidence from implementation:
- `GetTextValue(vint itemIndex)` and `GetBindingValue(vint itemIndex)` both call `RequestNode(itemIndex)` and delegate to `root->GetTextValue/GetBindingValue(node)`
- `RequestView()` checks the identifier and returns `this` for `INodeItemView::Identifier` or delegates to root provider
- `GetRoot()` simply returns the `root` member variable
- `ItemProviderBase` base class manages callback registration through `AttachCallback/DetachCallback`
- No data caching logic exists in the implementation - all data is retrieved fresh on each call

This task should come after Tasks 2-5 because it assumes the index-to-node mapping (tested in Tasks 2-3) and edge case handling (tested in Tasks 4-5) are already working correctly. It focuses on verifying the data access layer and integration aspects.

## TASK No.7: Add Simple Integration Test Cases with TreeViewItemBindableRootProvider

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
- Build a tree using bindable items (refer to existing bindable provider test patterns)
- Create `TreeViewItemBindableRootProvider` and wrap with `NodeItemProvider`
- Verify basic operations: `Count()`, `GetTextValue()`, `GetBindingValue()`
- Expand one node and verify the visible count increases and data retrieval works
- Collapse the node and verify the visible count decreases
- **Code Style (from completed tasks):**
  - Use direct method calls without interface casting
  - Add interface-specific comments for clarity
  - Keep test simple since both components are already thoroughly tested
  - Use `AssertItems` helper for verifying visible node sequences
  - **Do NOT call `OnAttached()` on root provider after initial tree construction** - tree updates happen automatically (learned from Task 6)
  - **If testing callbacks**: Remember that `DetachCallback()` calls `OnAttached(nullptr)`, so clear logs AFTER detaching, not before (learned from Task 6)
  - **Use specialized provider APIs** when available (e.g., if `TreeViewItemBindableRootProvider` provides specialized accessors like `GetTreeViewData()`, use those instead of generic `GetData().Cast<>()` - learned from Task 6)

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

# KEY LEARNINGS APPLIED TO FUTURE TASKS

Based on the completion of Task No.3 (Expand/Collapse Dynamics) and Task No.4 (Edge Cases and Complex Scenarios), the following learnings have been applied to future task descriptions:

## 1. Always Verify Implementation Before Writing Tests

**What happened**: In Task 3, I initially expected `CalculateNodeVisibilityIndex()` to return `-2` for invisible nodes, but it actually returns `-1`.

**Applied to**: 
- Task No.4: Added explicit instruction to read implementation first
- Task No.4: Added reminder that public method converts all negative values to `-1`

## 2. Avoid Edge Cases With Uncertain Semantics

**What happened**: In Task 3, I created a test `ExpandNodeWithExpandedChildren` that tested calling `SetExpanding()` on invisible nodes. The expected behavior was unclear and I had to remove the test.

**Applied to**:
- Task No.4: Added instruction to "avoid testing ambiguous edge cases where expected behavior is unclear"
- Task No.5: Added warning not to test expand/collapse on invisible nodes
- Task No.5: Removed the scenarios for expanding/collapsing invisible nodes
- Task No.5: Added note referencing the learning from Task 3

## 3. Provide Clear Rationale When Removing Tests

**What happened**: When I removed `ExpandNodeWithExpandedChildren`, I explained:
- Why the test was ambiguous
- That equivalent functionality was tested elsewhere
- That it was better to have 6 solid tests than 7 with an ambiguous one

**Applied to**:
- All future tasks: Maintain pattern of clear explanations when tests are unclear
- Future task instructions remind to focus on practical scenarios

## 4. Read Implementation to Understand Return Values

**What happened**: I should have checked `CalculateNodeVisibilityIndex()` implementation before assuming it returns `-2`.

**Applied to**:
- Task No.4: Added "BEFORE WRITING ANY TEST CODE" section
- Task No.4: Explicit instruction to read implementation files first
- Task No.5: Similar guidance added

## 5. Quality Over Coverage

**What happened**: The user accepted removal of the ambiguous test, showing that quality matters more than test count.

**Applied to**:
- All tasks: Emphasis on practical, well-defined scenarios
- Tasks 4-5: Instructions to remove unclear tests during implementation
- Focus shifted from "comprehensive coverage" to "meaningful coverage"

## 6. Existing Test Patterns Are Your Friend

**What happened**: I found the correct expected value (`-1`) by checking existing tests in `BasicIndexMapping` category.

**Applied to**:
- Task No.4: Added instruction to check existing test patterns before writing new tests
- Reinforced the importance of learning from existing codebase

These learnings ensure future tasks avoid the same mistakes and maintain high-quality, practical test coverage.

## 7. Programming Errors Should Use CHECK_ERROR, Not Sentinel Values

**What happened**: In Task 4, I initially expected `CalculateNodeVisibilityIndex()` to return `-1` for foreign nodes (nodes from a different tree). The user changed this to use `CHECK_ERROR` instead, triggering an immediate error.

**Why this matters**: 
- Passing a foreign node to `CalculateNodeVisibilityIndex()` is a programming error, not a valid use case
- Using `CHECK_ERROR` catches bugs immediately during development
- Returning sentinel values like `-1` could mask bugs by allowing invalid operations to continue silently
- This aligns with defensive programming principles: fail fast at API boundaries

**The distinction**:
- **Valid "not found" cases** (e.g., a node in a collapsed subtree): Return sentinel values like `-1`
- **Programming errors** (e.g., foreign nodes, out-of-range indices, null required parameters): Use `CHECK_ERROR` with descriptive error messages

**Applied to**:
- Task No.6: Changed "return empty/default values" to "trigger CHECK_ERROR" for invalid indices in `GetTextValue()` and `GetBindingValue()`
- Task No.7: Added note that invalid indices should trigger `CHECK_ERROR`
- Future implementation: Always distinguish between legitimate "not found" vs invalid input

**Pattern to follow**:
```cpp
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::tree::ClassName::MethodName(ParamTypes)#"
CHECK_ERROR(condition, ERROR_MESSAGE_PREFIX L"Descriptive error message.");
#undef ERROR_MESSAGE_PREFIX
```

**In tests**:
- Use `TEST_ERROR(expression)` to verify that `CHECK_ERROR` is triggered
- Invalid indices, foreign nodes, and other programming errors should all be tested with `TEST_ERROR`

## 8. Error Messages Should Include Full Context

**What happened**: The user's error message included the full method signature with namespace: `vl::presentation::controls::tree::NodeItemProvider::CalculateNodeVisibilityIndexInternal(INodeProvider*)#`

**Applied to**: All future error checking code should follow the `ERROR_MESSAGE_PREFIX` pattern with:
- Full namespace path
- Class name
- Method name with parameter types
- Descriptive error message explaining what went wrong

These learnings ensure future tasks maintain consistency with established error handling patterns and user preferences.

## 9. Tree Providers Auto-Update, Don't Call OnAttached() After Construction

**What happened**: In Task 6 (Fixing Attempt No.1), I called `rootProvider->OnAttached(rootProvider->GetRootNode().Obj())` after adding nodes and modifying the tree. This caused compilation errors because:
- `GetRootNode()` returns `INodeProvider*` but `OnAttached` expects `INodeRootProvider*` (type mismatch)
- Looking at existing tests, they never call `OnAttached` after initial tree creation

**Why this matters**:
- Tree providers automatically handle notifications when nodes are added or modified
- The tree updates happen automatically through the node provider's internal mechanisms
- Calling `OnAttached()` manually is unnecessary and can cause errors

**Applied to**:
- Task No.7: Added explicit note "Do NOT call `OnAttached()` on root provider after initial tree construction"
- Future implementation: Rely on automatic tree updates after `SetExpanding()`, adding children, or other tree modifications

## 10. Use Specialized Provider APIs Over Generic GetData().Cast<>()

**What happened**: In Task 6 (Fixing Attempt No.2), I used `node->GetData().Cast<TreeViewItem>()` to access the TreeViewItem data. The user pointed out that `TreeViewItemRootProvider` provides `GetTreeViewData(node)` which is cleaner and more type-safe.

**Why this matters**:
- Provider classes often provide specialized accessor methods for their specific data types
- These specialized methods are more readable, type-safe, and follow better API design
- They internally do the same cast but provide a cleaner interface

**The pattern**:
```cpp
// ❌ Generic but verbose
auto treeItem = node->GetData().Cast<TreeViewItem>();
treeItem->text = L"New Text";

// ✅ Specialized and clean
rootProvider->GetTreeViewData(node.Obj())->text = L"New Text";
```

**Applied to**:
- Task No.7: Added note to use specialized provider APIs when available
- Future implementation: Always check if the provider class offers specialized accessors before falling back to `GetData().Cast<>()`

## 11. Callback Lifecycle: DetachCallback() Triggers OnAttached(nullptr)

**What happened**: In Task 6 (Fixing Attempt No.3), test `DetachCallbackStopsEvents` failed because I cleared callback logs BEFORE calling `DetachCallback()`. The issue was that `DetachCallback()` calls `callback->OnAttached(nullptr)` to notify the callback it's being detached, and this was being logged.

**Why this matters**:
- `AttachCallback()` calls `callback->OnAttached(provider)` to notify attachment
- `DetachCallback()` calls `callback->OnAttached(nullptr)` to notify detachment
- These are side effects that will appear in callback logs
- Test sequencing must account for these lifecycle events

**The correct pattern**:
```cpp
// ❌ Wrong order - OnAttached(null) will be in the log
callbackLog.Clear();
provider->DetachCallback(&callback);
// Now callbackLog has "OnAttached(provider=null)"

// ✅ Correct order - lifecycle event is cleared
provider->DetachCallback(&callback);
callbackLog.Clear();
// Now callbackLog is empty and ready for new events
```

**Applied to**:
- Task No.7: Added note about callback lifecycle if testing callbacks
- Future implementation: Always sequence callback operations as: attach → clear → test → detach → clear

These learnings from Task 6 ensure Task 7 and future implementations avoid the same mistakes and follow established patterns.

# !!!FINISHED!!!