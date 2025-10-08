# !!!TASK!!!

# PROBLEM DESCRIPTION

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

# INSIGHTS AND REASONING

## Understanding NodeItemProvider's Core Responsibility

From analyzing `Source\Controls\ListControlPackage\DataSourceImpl_IItemProvider_NodeItemProvider.h/cpp`, `tree::NodeItemProvider` serves as an adapter that converts a hierarchical tree structure (`INodeRootProvider`) into a flat list (`IItemProvider`) for use by `GuiListControl`.

Key architectural points:
1. **Dual Interface Implementation**: `NodeItemProvider` implements both `list::ItemProviderBase` (for list operations) and `tree::INodeItemView` (for tree-to-index mapping)
2. **Root Node Exclusion**: The root node is explicitly excluded from visible indices. This is implemented in:
   - `Count()`: returns `root->CalculateTotalVisibleNodes() - 1` (subtracts 1 for root)
   - `RequestNode(index)`: calls `GetNodeByOffset(root, index + 1)` (adds 1 to skip root)
3. **Visibility Semantics**: Only expanded nodes' children are visible. Collapsed subtrees are completely hidden from the list view.

## Core Methods to Test

### RequestNode(vint index) - Tree::INodeItemView Interface

**Implementation Details** (from DataSourceImpl_IItemProvider_NodeItemProvider.cpp lines 195-204):
```cpp
Ptr<INodeProvider> NodeItemProvider::RequestNode(vint index)
{
    if (root->CanGetNodeByVisibleIndex())
    {
        return root->GetNodeByVisibleIndex(index + 1);
    }
    else
    {
        return GetNodeByOffset(root->GetRootNode(), index + 1);
    }
}
```

Key behaviors:
- Adds 1 to the index to skip the root node
- Falls back to `GetNodeByOffset()` which recursively traverses only expanded nodes
- `GetNodeByOffset()` (lines 13-33) uses depth-first traversal, entering children only when `provider->GetExpanding()` is true

**What to Test**:
1. Flat list (all root children, no expansion) - verifies basic indexing
2. Simple hierarchy with expansion - verifies depth-first traversal
3. Invalid indices (negative, beyond count) - verifies boundary handling

### CalculateNodeVisibilityIndex(INodeProvider* node) - Tree::INodeItemView Interface

**Implementation Details** (from lines 206-210):
```cpp
vint NodeItemProvider::CalculateNodeVisibilityIndex(INodeProvider* node)
{
    vint result = CalculateNodeVisibilityIndexInternal(node);
    return result < 0 ? -1 : result;
}
```

**CalculateNodeVisibilityIndexInternal** (lines 130-170) returns:
- `-1`: node not found or is the root node
- `-2`: node is in a collapsed subtree (parent not expanded)
- `>= 0`: valid visible index

The public API converts `-2` to `-1` (both cases mean "not visible").

**What to Test**:
1. Visible nodes in flat tree - should return correct indices
2. Visible nodes in expanded hierarchy - should return indices in depth-first order
3. Nodes in collapsed subtrees - should return -1
4. Root node itself - should return -1
5. Null pointer - should handle gracefully

### Count() - List::IItemProvider Interface

**Implementation Details** (from lines 212-215):
```cpp
vint NodeItemProvider::Count()
{
    return root->GetRootNode()->CalculateTotalVisibleNodes()-1;
}
```

**What to Test**:
1. Flat tree (all collapsed) - should equal number of root's children
2. Partially expanded tree - should include expanded children
3. Fully expanded tree - should include all nodes except root

## Proposed Test Case Design

### Test Case 1: FlatTreeBasicMapping

**Purpose**: Verify the most basic scenario - a flat list with no hierarchy expansion.

**Tree Structure**:
```
Root (invisible)
├─ Node1 (collapsed)
├─ Node2 (collapsed)
└─ Node3 (collapsed)
```

**Test Steps**:
1. Create `TreeViewItemRootProvider` and add 3 nodes at root level
2. Create `NodeItemProvider` wrapping the root provider
3. Attach `MockItemProviderCallback` and clear logs

**Assertions**:
- `Count()` returns 3 (only root's children are visible)
- `RequestNode(0)` returns Node1, `RequestNode(1)` returns Node2, `RequestNode(2)` returns Node3
- `CalculateNodeVisibilityIndex(Node1)` returns 0
- `CalculateNodeVisibilityIndex(Node2)` returns 1
- `CalculateNodeVisibilityIndex(Node3)` returns 2
- Verify GetTextValue() on RequestNode(i) matches expected node names

**Why Important**: This establishes the baseline behavior without any hierarchy complexity.

### Test Case 2: SingleLevelExpansion

**Purpose**: Verify depth-first traversal when one parent node is expanded.

**Tree Structure**:
```
Root (invisible)
├─ Parent1 (expanded)
│  ├─ Child1-1
│  └─ Child1-2
└─ Parent2 (collapsed)
   ├─ Child2-1 (invisible)
   └─ Child2-2 (invisible)
```

**Test Steps**:
1. Create root with 2 parent nodes
2. Add 2 children to Parent1 and 2 children to Parent2
3. Expand Parent1 only (using `SetExpanding(true)`)
4. Create `NodeItemProvider` and clear callback logs

**Assertions**:
- `Count()` returns 4 (Parent1, Child1-1, Child1-2, Parent2)
- `RequestNode(0)` returns Parent1
- `RequestNode(1)` returns Child1-1
- `RequestNode(2)` returns Child1-2
- `RequestNode(3)` returns Parent2
- `CalculateNodeVisibilityIndex(Parent1)` returns 0
- `CalculateNodeVisibilityIndex(Child1-1)` returns 1
- `CalculateNodeVisibilityIndex(Child1-2)` returns 2
- `CalculateNodeVisibilityIndex(Parent2)` returns 3
- `CalculateNodeVisibilityIndex(Child2-1)` returns -1 (in collapsed subtree)
- `CalculateNodeVisibilityIndex(Child2-2)` returns -1 (in collapsed subtree)

**Why Important**: This verifies depth-first traversal and that collapsed subtrees are invisible.

### Test Case 3: MultiLevelExpansion

**Purpose**: Verify correct indexing with 3-level hierarchy and partial expansion.

**Tree Structure**:
```
Root (invisible)
├─ Level1-A (expanded)
│  ├─ Level2-A1 (expanded)
│  │  └─ Level3-A1a
│  └─ Level2-A2 (collapsed)
│     └─ Level3-A2a (invisible)
└─ Level1-B (collapsed)
   └─ Level2-B1 (invisible)
```

**Test Steps**:
1. Build 3-level tree structure
2. Expand Level1-A and Level2-A1, keep others collapsed
3. Create `NodeItemProvider` and clear callbacks

**Assertions**:
- `Count()` returns 5 (Level1-A, Level2-A1, Level3-A1a, Level2-A2, Level1-B)
- Verify `RequestNode(i)` for i=0..4 returns nodes in depth-first order
- Verify `CalculateNodeVisibilityIndex()` returns correct index for each visible node
- Verify `CalculateNodeVisibilityIndex()` returns -1 for Level3-A2a and Level2-B1

**Why Important**: This tests more complex hierarchy traversal and ensures the implementation correctly handles multiple levels of expansion.

### Test Case 4: RootNodeExclusionAndBidirectionalConsistency

**Purpose**: Explicitly verify root node is excluded and test bidirectional mapping consistency.

**Tree Structure**: Same as Test Case 2 (SingleLevelExpansion)

**Test Steps**:
1. Build tree with 2-level expansion (reuse structure from Test Case 2)
2. Get root node via `rootProvider->GetRootNode()`

**Assertions**:
- `CalculateNodeVisibilityIndex(root)` returns -1
- `RequestNode(0)` returns first child of root, NOT root itself
- For each visible index i from 0 to Count()-1:
  - `node = RequestNode(i)` 
  - `CalculateNodeVisibilityIndex(node) == i` (bidirectional consistency)
- For each visible node in the tree:
  - `index = CalculateNodeVisibilityIndex(node)`
  - `RequestNode(index) == node` (reverse bidirectional consistency)

**Why Important**: The root exclusion is a critical design decision that must be explicitly tested. Bidirectional consistency ensures the two mapping directions are synchronized.

### Test Case 5: BoundaryConditionsForRequestNode

**Purpose**: Verify boundary handling for invalid indices.

**Tree Structure**: Simple flat tree with 3 nodes

**Test Steps**:
1. Create flat tree with 3 nodes
2. Create `NodeItemProvider`

**Assertions**:
- `RequestNode(-1)` returns nullptr
- `RequestNode(Count())` returns nullptr (one past the end)
- `RequestNode(Count() + 1)` returns nullptr
- `RequestNode(999)` returns nullptr
- Valid indices 0..Count()-1 all return non-null nodes

**Why Important**: Robust error handling prevents crashes when invalid indices are used.

### Test Case 6: EmptyTreeAndSingleNode

**Purpose**: Test edge cases with minimal tree structures.

**Test Steps**:
1. **Empty tree**: Root with no children
   - `Count()` returns 0
   - `RequestNode(0)` returns nullptr
   
2. **Single node tree**: Root with one child
   - `Count()` returns 1
   - `RequestNode(0)` returns the single child
   - `CalculateNodeVisibilityIndex(child)` returns 0

**Why Important**: Edge cases often reveal off-by-one errors.

### Test Case 7: ComplexMixedExpansionWithCount

**Purpose**: Verify Count() accurately reflects visible nodes in complex scenarios.

**Tree Structure**:
```
Root (invisible)
├─ A (expanded)
│  ├─ A1 (collapsed)
│  │  └─ A1a (invisible)
│  └─ A2 (expanded)
│     ├─ A2a
│     └─ A2b
├─ B (collapsed)
│  └─ B1 (invisible)
└─ C (collapsed)
```

**Expected Visible Nodes**: A, A1, A2, A2a, A2b, B, C = 7 nodes

**Assertions**:
- `Count()` returns 7
- Verify each visible node is accessible via RequestNode() and has correct index
- Verify invisible nodes (A1a, B1) return -1 from CalculateNodeVisibilityIndex()

**Why Important**: This comprehensive test ensures Count() calculation is correct even with complex expansion patterns.

## Test Organization and Code Style

Based on Task No.1 learnings and existing test patterns:

### File Organization
- Add `TEST_CATEGORY(L"BasicIndexMapping")` block after the existing `BasicSetupAndConstruction` test
- Each test case is a separate `TEST_CASE` within the category
- Approximately 15-25 lines per test case, total ~150-175 lines

### Coding Patterns
```cpp
TEST_CATEGORY(L"BasicIndexMapping")
{
    TEST_CASE(L"FlatTreeBasicMapping")
    {
        // Setup: callback logging
        List<WString> callbackLog;
        MockItemProviderCallback itemCallback(callbackLog);
        
        // Setup: Create root provider with flat structure
        auto rootProvider = Ptr(new TreeViewItemRootProvider);
        auto node1 = CreateTreeViewItem(L"Node1");
        auto node2 = CreateTreeViewItem(L"Node2");
        auto node3 = CreateTreeViewItem(L"Node3");
        
        auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
        rootMemoryNode->Children().Add(node1);
        rootMemoryNode->Children().Add(node2);
        rootMemoryNode->Children().Add(node3);
        
        // Create NodeItemProvider
        auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
        nodeItemProvider->AttachCallback(&itemCallback);
        callbackLog.Clear(); // Clear after setup
        
        // Verify Count() through IItemProvider interface
        TEST_ASSERT(nodeItemProvider->Count() == 3);
        
        // Verify RequestNode() through INodeItemView interface
        auto retrieved0 = nodeItemProvider->RequestNode(0);
        auto retrieved1 = nodeItemProvider->RequestNode(1);
        auto retrieved2 = nodeItemProvider->RequestNode(2);
        
        TEST_ASSERT(retrieved0 == node1.Obj());
        TEST_ASSERT(retrieved1 == node2.Obj());
        TEST_ASSERT(retrieved2 == node3.Obj());
        
        // Verify CalculateNodeVisibilityIndex() through INodeItemView interface
        TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node1.Obj()) == 0);
        TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node2.Obj()) == 1);
        TEST_ASSERT(nodeItemProvider->CalculateNodeVisibilityIndex(node3.Obj()) == 2);
        
        // Verify GetTextValue() through IItemProvider interface
        TEST_ASSERT(nodeItemProvider->GetTextValue(0) == L"Node1");
        TEST_ASSERT(nodeItemProvider->GetTextValue(1) == L"Node2");
        TEST_ASSERT(nodeItemProvider->GetTextValue(2) == L"Node3");
    });
    
    // Additional test cases follow...
}
```

### Key Style Guidelines
1. **Interface Comments**: Add comments like `// Verify Count() through IItemProvider interface` to clarify which interface is being tested
2. **Blank Lines**: Use blank lines to separate setup, action, and verification phases
3. **Direct Calls**: Call methods directly on `nodeItemProvider` without explicit interface casting (e.g., `nodeItemProvider->RequestNode(0)` not `((INodeItemView*)nodeItemProvider)->RequestNode(0)`)
4. **Variable Names**: Use descriptive names like `retrieved0` for nodes retrieved by index, and `node1` for nodes added to tree
5. **Callback Handling**: Always clear callback logs after setup using `callbackLog.Clear()` to isolate test behavior from setup noise
6. **No Callback Verification**: Since this is testing static mapping, we don't verify callbacks (that's for dynamic tests in Task 3)

## Implementation Strategy

### Phase 1: Simple Test Cases (30-40 lines)
- Test Case 6: EmptyTreeAndSingleNode
- Test Case 1: FlatTreeBasicMapping

These establish the foundation and verify basic assumptions.

### Phase 2: Hierarchy Test Cases (50-70 lines)
- Test Case 2: SingleLevelExpansion
- Test Case 3: MultiLevelExpansion

These verify depth-first traversal works correctly.

### Phase 3: Edge Cases and Verification (40-50 lines)
- Test Case 4: RootNodeExclusionAndBidirectionalConsistency
- Test Case 5: BoundaryConditionsForRequestNode

These ensure robustness and correctness.

### Phase 4: Comprehensive Test (20-30 lines)
- Test Case 7: ComplexMixedExpansionWithCount

This is the final verification that all pieces work together.

**Total Estimated Lines**: ~150-180 lines of test code

## Evidence from Source Code

### Root Node Exclusion Evidence
From `DataSourceImpl_IItemProvider_NodeItemProvider.cpp`:
- Line 195-204: `RequestNode(index)` calls `GetNodeByOffset(root, index + 1)` - adds 1 to skip root
- Line 212-215: `Count()` returns `root->CalculateTotalVisibleNodes() - 1` - subtracts 1 for root

### Visibility Semantics Evidence  
From `GetNodeByOffset` (lines 13-33):
```cpp
if (provider->GetExpanding() && offset > 0)
{
    // Only recurse into children if node is expanded
    ...
}
```

Only expanded nodes have their children traversed.

From `CalculateNodeVisibilityIndexInternal` (lines 130-170):
```cpp
if (!parent->GetExpanding())
{
    return -2;  // Node is in collapsed subtree
}
```

Nodes whose parent is collapsed return -2 (converted to -1 in public API).

### Depth-First Traversal Evidence
From `GetNodeByOffset` (lines 13-33):
```cpp
for (vint i = 0; (!result && i < count); i++)
{
    auto child = provider->GetChild(i);
    vint visibleCount = child->CalculateTotalVisibleNodes();
    if (offset < visibleCount)
    {
        result = GetNodeByOffset(child, offset);  // Recurse into subtree
    }
    else
    {
        offset -= visibleCount;  // Skip this subtree
    }
}
```

This clearly implements depth-first traversal: for each child, either recurse into it or skip its entire subtree.

## Alternative Approaches Considered

### Alternative 1: Test All Scenarios in One Large Test Case
**Pros**: Less code duplication, all scenarios in one place
**Cons**: 
- Difficult to debug when one assertion fails
- Poor test isolation - one failure affects understanding of others
- Violates single responsibility principle for tests

**Decision**: REJECTED - Use separate test cases for clarity

### Alternative 2: Generate Random Tree Structures for Fuzz Testing
**Pros**: Could find edge cases not thought of
**Cons**: 
- Non-deterministic failures are hard to debug
- Doesn't fit the project's pattern of explicit test cases
- Out of scope for "basic" mapping tests

**Decision**: REJECTED - Save for future stress testing task (Task 4)

### Alternative 3: Test RequestNode and CalculateNodeVisibilityIndex in Separate Categories
**Pros**: More granular organization
**Cons**: 
- These methods are intrinsically linked (bidirectional mapping)
- Would require duplicate tree setup code
- Task description explicitly groups them together

**Decision**: REJECTED - Keep them together as they test the same concept from different directions

## Summary

This task will add 7 test cases (~150-180 lines) covering:
1. ✓ Basic flat tree mapping
2. ✓ Single-level hierarchy expansion
3. ✓ Multi-level hierarchy expansion  
4. ✓ Root node exclusion and bidirectional consistency
5. ✓ Boundary condition handling
6. ✓ Edge cases (empty, single node)
7. ✓ Complex mixed expansion patterns

All requirements from the task description are addressed:
- ✓ Test RequestNode() with flat and hierarchical trees
- ✓ Test CalculateNodeVisibilityIndex() for visible and invisible nodes
- ✓ Verify root node exclusion
- ✓ Test Count() with various expansion states
- ✓ Use MockItemProviderCallback (but focus on return values, not callbacks)
- ✓ Follow code style from Task 1
- ✓ Organize under TEST_CATEGORY(L"BasicIndexMapping")

The design is comprehensive, evidence-based, and follows established patterns in the codebase.

# !!!FINISHED!!!
