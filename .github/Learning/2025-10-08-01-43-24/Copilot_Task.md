# !!!TASK!!!

# PROBLEM DESCRIPTION

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

# INSIGHTS AND REASONING

## Step 1: Understanding the Implementation

From reading `DataSourceImpl_IItemProvider_NodeItemProvider.cpp`, I found the following key implementation details:

### RequestNode Behavior with Invalid Indices

```cpp
Ptr<INodeProvider> NodeItemProvider::RequestNode(vint index)
{
	if (index < 0 || index >= Count())
	{
		return nullptr;  // Returns nullptr for any invalid index
	}
	else if (root->CanGetNodeByVisibleIndex())
	{
		return root->GetNodeByVisibleIndex(index + 1);
	}
	else
	{
		return GetNodeByOffset(root->GetRootNode(), index + 1);
	}
}
```

**Key findings:**
- Invalid indices (negative or >= Count()) return `nullptr`
- This is consistent with the existing tests in `BasicIndexMapping` category
- There's no distinction between different types of invalid indices

### CalculateNodeVisibilityIndex Behavior

```cpp
vint NodeItemProvider::CalculateNodeVisibilityIndex(INodeProvider* node)
{
	vint result = CalculateNodeVisibilityIndexInternal(node);
	return result < 0 ? -1 : result;  // All negative values become -1
}

vint NodeItemProvider::CalculateNodeVisibilityIndexInternal(INodeProvider* node)
{
	auto parent = node->GetParent();
	if (!parent)
	{
		return -1;  // Root node has no parent
	}
	if (!parent->GetExpanding())
	{
		return -2;  // Node is in collapsed subtree
	}
	// ... recursively calculates index
}
```

**Key findings:**
- Public method `CalculateNodeVisibilityIndex()` always returns `-1` for any invisible node
- Internal method distinguishes between: `-1` (not found/root), `-2` (in collapsed subtree)
- But tests should only check for `-1` from the public API
- Passing `nullptr` would likely crash (no null check), so we should NOT test this

### GetTextValue and GetBindingValue with Invalid Indices

```cpp
WString NodeItemProvider::GetTextValue(vint itemIndex)
{
	if (auto node = RequestNode(itemIndex))
	{
		return root->GetTextValue(node.Obj());
	}
	return L"";  // Returns empty string for invalid index
}

description::Value NodeItemProvider::GetBindingValue(vint itemIndex)
{
	if (auto node = RequestNode(itemIndex))
	{
		return root->GetBindingValue(node.Obj());
	}
	return Value();  // Returns default Value for invalid index
}
```

**Key findings:**
- Both methods gracefully handle invalid indices by returning empty/default values
- They delegate to `RequestNode()` which returns `nullptr` for invalid indices

## Step 2: Reviewing Existing Test Patterns

From `TestItemProviders_NodeItemProvider.cpp`, I found:

### Already Tested Edge Cases in BasicIndexMapping

```cpp
TEST_CASE(L"BoundaryConditionsForRequestNode")
{
	// ...
	// Verify invalid indices through INodeItemView interface - should return nullptr
	TEST_ASSERT(nodeItemProvider->RequestNode(-1) == nullptr);
	TEST_ASSERT(nodeItemProvider->RequestNode(nodeItemProvider->Count()) == nullptr);
	TEST_ASSERT(nodeItemProvider->RequestNode(nodeItemProvider->Count() + 1) == nullptr);
	TEST_ASSERT(nodeItemProvider->RequestNode(999) == nullptr);
}
```

**What's already covered:**
- Invalid index testing with `RequestNode()`
- Testing at boundaries: -1, Count(), Count()+1, large number

### Pattern from ExpandCollapseDynamics

The existing tests demonstrate:
- Trees with 3 levels are considered "nested"
- Trees with 4 levels are considered "deeply nested"
- Adding/removing children is tested via tree modification callbacks
- Callback verification is done with `AssertCallbacks()`

## Step 3: Identifying What Needs to Be Tested

Based on the task requirements and what's already covered, here's what needs NEW test coverage:

### 1. Invalid Index Testing for GetTextValue/GetBindingValue
- **NOT covered yet**: Testing `GetTextValue()` and `GetBindingValue()` with invalid indices
- **Expected behavior**: Return empty string / default Value

### 2. CalculateNodeVisibilityIndex with Foreign Node
- **NOT covered yet**: Passing a node from a different tree
- **Expected behavior**: Should return `-1` (not found)
- **Important**: Do NOT test with nullptr (would crash, no null check in implementation)

### 3. Deeply Nested Tree (4+ levels)
- **Partially covered**: Task 3 has a 4-level test, but only for expand/collapse dynamics
- **Need**: Test index mapping correctness in a 4+ level tree with mixed expand states
- **Focus**: Verify all visible nodes have correct indices, and invisible nodes return -1

### 4. Adding/Removing Children to Expanded Nodes
- **NOT covered yet**: Dynamically adding/removing children and verifying index updates
- **Need**: Test that callbacks fire correctly and indices adjust properly
- **Important**: This tests the `OnBeforeItemModified` and `OnAfterItemModified` callbacks

### 5. Modifications to Collapsed Subtrees
- **Reserved for Task 5**: "Operations on Invisible Nodes" task explicitly covers this
- **Should NOT duplicate**: This is Task 5's responsibility

### 6. Large Tree Stress Test (50+ nodes)
- **NOT covered yet**: Testing with many nodes to ensure scalability
- **Focus**: Basic operations still work, no performance issues

### 7. All Collapsed vs All Expanded
- **Partially covered**: Flat tree (all collapsed) tested in BasicIndexMapping
- **Need**: Explicit test for "all expanded" scenario with multi-level tree

## Step 4: Avoiding Ambiguous Edge Cases

Based on learnings from Task 3 (documented in Copilot_Scrum.md):

**DO NOT TEST:**
- `CalculateNodeVisibilityIndex(nullptr)` - Would crash, no null check
- Operations on invisible nodes - Reserved for Task 5
- Expand state of invisible nodes - Uncertain semantics (learned from Task 3)

**DO TEST:**
- Well-defined behaviors with clear expected results
- Practical scenarios that matter in real applications
- Edge cases where implementation behavior is documented/verifiable

## Step 5: High-Level Design of Test Cases

I propose 6 test cases under the "EdgeCasesAndComplexScenarios" category:

### Test Case 1: InvalidIndicesForDataRetrieval
**Purpose**: Verify `GetTextValue()` and `GetBindingValue()` handle invalid indices gracefully

**Approach**:
- Create simple tree with 3 visible nodes
- Call `GetTextValue(-1)`, `GetTextValue(Count())`, `GetTextValue(999)`
- Verify all return empty string `L""`
- Call `GetBindingValue()` with same invalid indices
- Verify all return default `Value()`

**Why**: Complements existing `RequestNode()` invalid index tests

### Test Case 2: CalculateIndexForForeignNode
**Purpose**: Verify `CalculateNodeVisibilityIndex()` returns -1 for nodes not in the tree

**Approach**:
- Create two separate `TreeViewItemRootProvider` instances
- Create `NodeItemProvider` for tree A
- Pass a node from tree B to `nodeItemProviderA->CalculateNodeVisibilityIndex()`
- Verify returns `-1`

**Why**: Tests boundary condition - node pointer is valid but doesn't belong to this tree

### Test Case 3: DeeplyNestedTreeIndexMapping
**Purpose**: Verify index mapping works correctly in 4+ level deep trees with mixed expand states

**Approach**:
- Create a 5-level tree structure (Root -> A -> B -> C -> D -> E)
- Expand levels selectively: A expanded, B collapsed, C expanded (but invisible), D expanded (but invisible)
- Verify visible nodes have correct indices
- Verify all nodes in collapsed subtrees return -1
- Verify bidirectional consistency for visible nodes

**Why**: Tests scalability and correctness with deep nesting

### Test Case 4: DynamicChildAdditionRemoval
**Purpose**: Verify adding/removing children to expanded nodes updates indices correctly

**Approach**:
- Create tree with expanded parent
- Add a child dynamically using `Children().Add()`
- Verify `Count()` increased
- Verify callback fired with correct parameters
- Verify new child has correct index
- Remove the child using `Children().RemoveAt()`
- Verify `Count()` decreased
- Verify callback fired with correct parameters

**Why**: Tests dynamic tree modification, which triggers different code paths than expand/collapse

### Test Case 5: LargeTreeStressTest
**Purpose**: Verify operations work correctly with many nodes (50+)

**Approach**:
- Create tree with 10 top-level nodes, each with 10 children (100 nodes total visible when all expanded)
- Expand all nodes
- Verify `Count()` returns 110 (10 + 100)
- Test `RequestNode()` at various positions (first, middle, last)
- Test `CalculateNodeVisibilityIndex()` for various nodes
- Verify bidirectional consistency for sample nodes

**Why**: Tests scalability, ensures no performance issues or integer overflow

### Test Case 6: FullyExpandedVsFullyCollapsed
**Purpose**: Verify correct behavior in extreme expand states

**Approach**:
- Create 3-level tree (root with 2 children, each child has 2 children)
- Test Case 6a: All collapsed state
  - Verify only root's direct children visible (Count = 2)
  - Verify grandchildren return -1 from `CalculateNodeVisibilityIndex()`
- Test Case 6b: All expanded state
  - Expand all nodes
  - Verify all nodes visible (Count = 6)
  - Verify all nodes have valid indices
  - Verify bidirectional consistency

**Why**: Tests boundary conditions of tree expand state

## Step 6: Code Size Estimation

- Test Case 1: ~20-25 lines (simple invalid index testing)
- Test Case 2: ~15-20 lines (foreign node test)
- Test Case 3: ~40-50 lines (5-level tree setup + verification)
- Test Case 4: ~40-50 lines (dynamic add/remove with verification)
- Test Case 5: ~35-40 lines (large tree with loop-based verification)
- Test Case 6: ~30-35 lines (two sub-tests for collapsed/expanded)

**Total: ~180-220 lines**

This is slightly higher than the target of 100-150 lines, but each test case is essential:
- Test Cases 1-3 test API edge cases
- Test Case 4 tests dynamic modification
- Test Case 5 tests scalability
- Test Case 6 tests extreme states

If needed to reduce scope, we could:
- Merge Test Cases 1 and 2 (both test return values for edge cases)
- Simplify Test Case 5 (fewer nodes, e.g., 30 instead of 100)
- This would bring it down to ~150-170 lines

## Step 7: Comparison with Task 5

Task 5 focuses on "Operations on Invisible Nodes", which means:
- Operations performed on nodes in collapsed subtrees
- Verifying these operations don't affect `NodeItemProvider`
- Testing expand/collapse on invisible nodes (with caution per Task 3 learnings)

Task 4 (this task) focuses on:
- API edge cases (invalid inputs)
- Deep nesting and large trees
- Dynamic modifications to **visible** nodes

There's clear separation - no duplication of test coverage.

## Step 8: Evidence from Source Code

### Support for Dynamic Child Addition/Removal

From `NodeItemProvider::OnBeforeItemModified` and `OnAfterItemModified`:

```cpp
void NodeItemProvider::OnAfterItemModified(INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
{
	vint base = CalculateNodeVisibilityIndexInternal(parentNode);
	if (base != -2 && parentNode->GetExpanding())
	{
		vint offset = 0;
		// Calculate visible count of new children
		for (vint i = 0; i < newCount; i++)
		{
			auto child = parentNode->GetChild(start + i);
			offset += child->CalculateTotalVisibleNodes();
		}
		// ... calculate insertion point and fire callback
		InvokeOnItemModified(firstChildStart, offsetBeforeChildModified, offset, itemReferenceUpdated);
	}
}
```

**Evidence**: The implementation explicitly handles child addition/removal through `OnBeforeItemModified` and `OnAfterItemModified` callbacks. This is different from expand/collapse which uses `OnItemExpanded` and `OnItemCollapsed`.

### Support for Foreign Node Testing

From `NodeItemProvider::CalculateNodeVisibilityIndexInternal`:

```cpp
vint index = CalculateNodeVisibilityIndexInternal(parent.Obj());
if (index == -2)
{
	return -2;  // Parent is invisible
}

vint count = parent->GetChildCount();
for (vint i = 0; i < count; i++)
{
	auto child = parent->GetChild(i);
	bool findResult = child == node;
	// ...
}
return -1;  // Not found in parent's children
```

**Evidence**: If the node is not found in its parent's children list, the method returns -1. A foreign node would eventually hit this path and return -1 (converted to -1 by public method).

## Step 9: Design Decision

I recommend implementing all 6 test cases as designed, because:

1. **Each tests distinct functionality**: No duplication
2. **All are practical scenarios**: No ambiguous edge cases
3. **Clear expected behaviors**: All based on implementation analysis
4. **Complements existing tests**: Fills gaps in test coverage
5. **Reasonable code size**: ~200 lines is acceptable for edge case testing

The test cases follow established patterns from Tasks 2 and 3:
- Use `MockItemProviderCallback` for event tracking
- Use `AssertItems` for sequence verification
- Use `AssertCallbacks` for event verification
- Test through both `IItemProvider` and `INodeItemView` interfaces
- Comment which interface provides each method

## Step 10: Potential Simplifications

If the code size needs to be strictly under 150 lines, here are optional simplifications:

**Option A: Merge Test Cases 1 and 2**
- Combine invalid index testing and foreign node testing into one "InvalidInputs" test
- Saves ~15 lines by sharing setup code
- Still tests all edge cases

**Option B: Simplify Test Case 5**
- Reduce from 100 nodes to 30 nodes (3 top-level, each with 10 children)
- Still tests scalability, just smaller scale
- Saves ~10 lines

**Option C: Simplify Test Case 6**
- Only test fully expanded case (skip fully collapsed, which is partially covered in BasicIndexMapping)
- Saves ~15 lines

With Options A + B + C: ~200 lines → ~160 lines, closer to target.

However, I recommend keeping all 6 test cases as originally designed for comprehensive coverage, unless explicitly directed to reduce scope.

# !!!FINISHED!!!
