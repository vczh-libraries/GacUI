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

You will need to create 3 tasks:
- Create a TestItemProviders_NodeItemProvider.cpp in UnitTest project, put it in the Source Files solution explorer folder
- Design and implement test cases for the first category
- Design and implement test cases for the second category

You can refer to every TestItemProviders_*.cpp for knowledges and operations and API usage for related classes and interfaces, and how I want you to organize test cases.

You do not need to implement INodeRootProvider by yourself, you can just use TreeViewItemRootProvider. As it has been tested already, it can release your work to help focusing on tree::NodeItemProvider which is being tested

# TASKS

- [ ] TASK No.1: Create TestItemProviders_NodeItemProvider.cpp and Add to Project
- [ ] TASK No.2: Implement Test Cases for Visible Index and Node Mapping
- [ ] TASK No.3: Implement Test Cases for IItemProvider Data Retrieval

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

## TASK No.2: Implement Test Cases for Visible Index and Node Mapping

This task implements comprehensive test cases for the core functionality of `tree::NodeItemProvider`: the bidirectional mapping between visible indices and tree nodes, which changes dynamically with expand/collapse operations.

### what to be done

Implement test categories covering these scenarios:

**Basic Index Mapping:**
- Test `RequestNode(vint index)` with a flat list of nodes (all at root level, all collapsed)
- Test `RequestNode(vint index)` with a simple expanded hierarchy (1-2 levels deep)
- Test `CalculateNodeVisibilityIndex(INodeProvider* node)` returns correct indices for visible nodes
- Test `CalculateNodeVisibilityIndex(INodeProvider* node)` returns -1 for nodes in collapsed subtrees
- Verify that the root node itself is excluded from visible indices (requesting index 0 should return the first child of root)

**Expand/Collapse Dynamics:**
- Test that expanding a collapsed node correctly increases `Count()` and adjusts visible indices of subsequent nodes
- Test that collapsing an expanded node correctly decreases `Count()` and adjusts visible indices
- Test visible index calculations with partially expanded multi-level trees (some branches expanded, others collapsed)
- Test that expanding/collapsing nested nodes correctly updates the entire visible index range
- Verify that `OnItemModified` callbacks from the `IItemProvider` interface are triggered with correct parameters during expand/collapse

**Edge Cases and Complex Scenarios:**
- Test `RequestNode` with invalid indices (negative, beyond count)
- Test visible indices in a deeply nested tree (4+ levels) with mixed expand/collapse states
- Test that adding/removing nodes to/from expanded parents correctly updates visible indices
- Test that modifications to collapsed subtrees don't affect visible indices of the parent tree

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` and wrap it with `NodeItemProvider`
- Build various tree structures using `CreateTreeViewItem` helper
- Use `MockItemProviderCallback` to monitor `OnItemModified` events from the `NodeItemProvider` side
- Use `MockNodeProviderCallback` to monitor tree structure changes from the underlying provider
- Verify `Count()`, `RequestNode()`, and `CalculateNodeVisibilityIndex()` return expected values
- Use `SetExpanding()` on nodes to test expand/collapse scenarios
- Clear callback logs between test phases to isolate specific behaviors
- Use `AssertCallbacks()` to verify correct callback sequences

Each test category should be organized under `TEST_CATEGORY` blocks with descriptive names like "BasicIndexMapping", "ExpandCollapseDynamics", "EdgeCases", etc.

### rationale

This task focuses on the most fundamental responsibility of `NodeItemProvider`: converting a hierarchical tree structure into a flat list of visible items. This mapping is dynamic and complex because:

1. **Core Responsibility**: The primary purpose of `NodeItemProvider` is to make tree nodes accessible as a flat list for `GuiListControl`, which is evidenced by its implementation of both `list::ItemProviderBase` and `tree::INodeItemView`

2. **Complex State Management**: The implementation in `DataSourceImpl_IItemProvider_NodeItemProvider.cpp` shows sophisticated logic in:
   - `GetNodeByOffset()` - recursively traversing only expanded nodes
   - `OnItemExpanded/OnItemCollapsed()` - updating visible ranges when state changes
   - `CalculateNodeVisibilityIndexInternal()` - handling visibility checks and parent expansion states

3. **Dynamic Behavior**: The visible index mapping changes whenever:
   - A node is expanded or collapsed (via `SetExpanding()`)
   - Child nodes are added or removed from expanded parents
   - Any structural modification occurs in the visible portion of the tree

4. **Foundation for Data Retrieval**: The data retrieval functionality (Task 3) depends entirely on correct index-to-node mapping, making this task prerequisite knowledge for understanding the complete behavior

Evidence from implementation:
- `RequestNode()` uses `GetNodeByOffset()` which only traverses expanded nodes
- `CalculateNodeVisibilityIndexInternal()` returns -2 for nodes in collapsed subtrees
- `OnItemExpanded/OnItemCollapsed()` calls `InvokeOnItemModified()` to notify listeners about visible range changes
- The root node is explicitly excluded (visible indices start from root's children)

This task should be completed before Task 3 because understanding the index mapping is essential for verifying that data retrieval operates on the correct nodes.

## TASK No.3: Implement Test Cases for IItemProvider Data Retrieval

This task implements test cases to verify that `tree::NodeItemProvider` correctly retrieves and exposes data from underlying `tree::INodeProvider` nodes through the `list::IItemProvider` interface.

### what to be done

Implement test categories covering these scenarios:

**Basic Data Retrieval:**
- Test `Count()` returns correct number of visible nodes (excluding root) for various tree structures
- Test `GetTextValue(vint itemIndex)` retrieves correct text from visible nodes
- Test `GetBindingValue(vint itemIndex)` retrieves correct binding values from visible nodes
- Test `RequestView(identifier)` returns the `INodeItemView` interface when requested
- Test `RequestView(identifier)` delegates to underlying root provider for other view types
- Verify `GetRoot()` returns the correct `INodeRootProvider` instance

**Data Retrieval After Structural Changes:**
- Test that `GetTextValue()` returns correct values after nodes are expanded
- Test that `GetTextValue()` returns correct values after nodes are collapsed
- Test that `GetBindingValue()` returns correct values after tree modifications (add/remove nodes)
- Test that indices correctly map to the right nodes after expand/collapse operations
- Verify that data retrieval for out-of-bounds indices returns empty/default values

**Integration with Underlying Root Provider:**
- Test that `NodeItemProvider` correctly delegates `GetTextValue()` to the root provider's `GetTextValue(INodeProvider*)`
- Test that `NodeItemProvider` correctly delegates `GetBindingValue()` to the root provider's `GetBindingValue(INodeProvider*)`
- Test that both `TreeViewItemRootProvider` and `TreeViewItemBindableRootProvider` work correctly as underlying providers
- Verify that the data returned matches what would be obtained by directly calling methods on the root provider with the corresponding node

**Callback Integration:**
- Test that `AttachCallback()` properly registers `IItemProviderCallback`
- Test that callbacks receive `OnItemModified` events when expand/collapse changes visible items
- Verify callback parameters (start, count, newCount, itemReferenceUpdated) are correct
- Test that data retrieval works correctly in callback handlers

### how to test it

All test cases should:
- Create a `TreeViewItemRootProvider` with various tree structures
- Wrap it with `NodeItemProvider`
- Use `MockItemProviderCallback` to monitor `IItemProvider` events
- Build trees with known text values and binding values
- Call `GetTextValue()` and `GetBindingValue()` with various indices
- Verify returned values match expected data from the underlying nodes
- Test `Count()` against manually calculated visible node counts
- Use `RequestView()` to obtain `INodeItemView` and verify it's the same object
- Test expand/collapse operations and verify data retrieval still works correctly
- Use `SetExpanding()` to change visibility and verify `Count()` updates accordingly

Each test category should be organized under `TEST_CATEGORY` blocks with descriptive names like "BasicDataRetrieval", "DataRetrievalAfterChanges", "RootProviderIntegration", "CallbackIntegration", etc.

### rationale

This task verifies that `NodeItemProvider` correctly fulfills its role as an adapter between the tree structure (`INodeRootProvider`) and the list interface (`IItemProvider`). While Task 2 ensures the index-to-node mapping works correctly, this task ensures that:

1. **Interface Contract**: The `IItemProvider` interface methods (`Count()`, `GetTextValue()`, `GetBindingValue()`, `RequestView()`) are correctly implemented and delegate to the underlying `INodeRootProvider` for the right nodes

2. **Data Integrity**: The data retrieved through list indices matches the data stored in the corresponding tree nodes, demonstrating that the adapter pattern is working correctly

3. **Dynamic Consistency**: After tree structure changes (expand/collapse, add/remove nodes), the data retrieval still works correctly with updated indices

4. **Real Usage Pattern**: This tests how `GuiVirtualTreeListControl` and similar controls actually use `NodeItemProvider` - they rely on these data retrieval methods to render items

Evidence from implementation:
- `GetTextValue(vint itemIndex)` calls `RequestNode(itemIndex)` and then `root->GetTextValue(node.Obj())`
- `GetBindingValue(vint itemIndex)` follows the same delegation pattern
- `Count()` calculates visible nodes using `root->GetRootNode()->CalculateTotalVisibleNodes() - 1` (excluding root)
- `RequestView()` returns `INodeItemView` interface or delegates to root provider
- The class implements both `ItemProviderBase` and `INodeItemView`, making it a bridge between tree and list abstractions

This task should come after Task 2 because:
- It builds upon the index mapping verified in Task 2
- Understanding how indices map to nodes is prerequisite to verifying data retrieval
- The test cases naturally reference concepts tested in Task 2 (like what counts as a "visible node")

However, the tasks are largely independent in implementation and could be developed in parallel if needed, as they test different aspects of the same class.

# Impact to the Knowledge Base

## GacUI

No significant changes needed to the knowledge base. The testing of `tree::NodeItemProvider` follows established patterns and doesn't introduce new design concepts or APIs that need documentation. The class itself is an implementation detail of the tree view control system, and its behavior is well-defined by its interfaces (`list::ItemProviderBase` and `tree::INodeItemView`).

If anything needs to be added in the future, it would be under a potential "Choosing APIs" section for tree controls explaining:
- When to use `NodeItemProvider` directly (rarely, as it's typically managed by `GuiVirtualTreeListControl`)
- The relationship between `INodeRootProvider`, `NodeItemProvider`, and `GuiVirtualTreeListControl`

However, since this is an implementation detail rather than a user-facing API, documentation in the knowledge base is not critical at this time.

# !!!FINISHED!!!