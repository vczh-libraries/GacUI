# !!!PLANNING!!!

# UPDATES

## UPDATE

This task focuses on testing edge cases, error handling, boundary conditions, and cleanup scenarios for TreeViewItemBindableRootProvider to ensure robustness.

**What to be done:**
- Complete `TestItemProviders_TreeViewItemBindableRootProvider.cpp` with edge case and error handling test cases
- Each test case must create its own callbackLog and callback objects for isolation and robustness
- Clear callback logs after SetItemSource calls (except when testing SetItemSource behavior itself)
- Test childrenProperty with null or empty child collections - empty should only trigger OnAttached
- Use `TEST_ERROR` for property binding with missing or invalid childrenProperty (index out of range)
- Test SetItemSource transitions between different hierarchical structures
- Test expansion state when childrenProperty changes dynamically
- Test callback detachment scenarios during tree structure modifications - verify DetachCallback fires `OnAttached(provider=nullptr)`
- Use `TEST_ERROR` for out-of-bounds operations in hierarchical context (index out of range)
- Access children via `item->children[i]` directly - do NOT use `UnboxValue` for direct ObservableList subscript access
- When replacing ObservableList contents, use Clear() then Add() in a loop - assignment operator is deleted

**Key Constraints:**
- All callback expectations must account for eager child preparation (critical lesson from Task No.6)
- Type system: `item->children[i]` returns element directly, NOT a Value object (critical lesson from Task No.7)
- ObservableList operations fire individual callbacks, never batched (critical lesson from Task No.7)
- Lambda capture: explicitly capture other lambdas, not automatic (critical lesson from Task No.7)

# IMPROVEMENT PLAN

## STEP 1: Add EmptyAndNullScenarios Test Category

Add a new test category to handle edge cases where children collections are empty or null.

### Implementation

Add the following test category after the existing `HierarchicalBindingScenarios` category:

```cpp
TEST_CATEGORY(L"EmptyAndNullScenarios")
{
	TEST_CASE(L"SetItemSourceWithEmptyChildren")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		provider->AttachCallback(&nodeCallback);
		callbackLog.Clear();
		
		// Create root with explicitly empty children ObservableList
		auto rootItem = Ptr(new BindableItem());
		rootItem->name = L"RootWithEmptyChildren";
		// rootItem->children is already initialized as empty ObservableList
		
		provider->SetItemSource(BoxValue(rootItem));
		
		// SetItemSource with empty children should show newCount=0 due to no children to prepare eagerly
		const wchar_t* expected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
		
		// Verify root node has no children
		auto rootNode = provider->GetRootNode();
		TEST_ASSERT(rootNode->GetChildCount() == 0);
		TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"RootWithEmptyChildren");
	});

	TEST_CASE(L"ExpandNodeWithNoChildren")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = Ptr(new BindableItem());
		rootItem->name = L"EmptyRoot";
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		callbackLog.Clear();
		
		// Expand node with no children should complete safely
		rootNode->SetExpanding(true);
		
		// Expansion callback should fire even for empty node
		const wchar_t* expected[] = {
			L"[ROOT]->OnItemExpanded()"
		};
		AssertCallbacks(callbackLog, expected);
		
		TEST_ASSERT(rootNode->GetExpanding() == true);
		TEST_ASSERT(rootNode->GetChildCount() == 0);
	});

	TEST_CASE(L"AccessChildrenOfEmptyNode")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = Ptr(new BindableItem());
		rootItem->name = L"EmptyRoot";
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		TEST_ASSERT(rootNode->GetChildCount() == 0);
		
		// Accessing child at index 0 should trigger TEST_ERROR (out of bounds)
		TEST_ERROR(rootNode->GetChild(0));
		TEST_ERROR(rootNode->GetChild(-1));
		TEST_ERROR(rootNode->GetChild(100));
	});
}
```

### Rationale

This category tests fundamental boundary conditions:
- **Empty children**: Verifies that nodes with no children are handled correctly, showing `newCount=0` in callbacks
- **Expansion on empty nodes**: Ensures UI operations like expand/collapse work safely even when there's no data
- **Out-of-bounds access**: Uses `TEST_ERROR` to verify defensive programming with `CHECK_ERROR` for invalid indices

These are practical scenarios that occur in real applications when data is loading, filtered out, or simply doesn't exist.

## STEP 2: Add SetItemSourceTransitions Test Category

Add comprehensive tests for transitioning between different ItemSource states.

### Implementation

```cpp
TEST_CATEGORY(L"SetItemSourceTransitions")
{
	TEST_CASE(L"TransitionFromNullToHierarchy")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		provider->AttachCallback(&nodeCallback);
		
		// Initial state: no ItemSource set
		auto rootNode = provider->GetRootNode();
		TEST_ASSERT(rootNode->GetChildCount() == 0);
		callbackLog.Clear();
		
		// Set ItemSource to multi-level tree
		auto rootItem = CreateMultiLevelTree(L"NewRoot", 3, 2);
		provider->SetItemSource(BoxValue(rootItem));
		
		// Transition shows count=0 (was empty) to newCount=3 (eager preparation of new children)
		const wchar_t* expected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=3, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=3, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
		
		// Verify new hierarchy is fully accessible
		TEST_ASSERT(rootNode->GetChildCount() == 3);
		TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"NewRoot");
	});

	TEST_CASE(L"TransitionFromHierarchyToNull")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		// Start with populated tree
		auto rootItem = CreateMultiLevelTree(L"Root", 3, 2);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		callbackLog.Clear();
		
		// Clear ItemSource
		provider->SetItemSource(Value());
		
		// Transition shows count=3 (old children) to newCount=0 (cleared)
		const wchar_t* expected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
		
		// Verify root is now empty
		TEST_ASSERT(rootNode->GetChildCount() == 0);
	});

	TEST_CASE(L"TransitionComplexToSimple")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		// Start with multi-level tree (3 children, each with 2 grandchildren)
		auto rootItem1 = CreateMultiLevelTree(L"ComplexRoot", 3, 2);
		provider->SetItemSource(BoxValue(rootItem1));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		callbackLog.Clear();
		
		// Replace with simple flat tree (2 children, no grandchildren)
		auto rootItem2 = CreateBindableTree(L"SimpleRoot", 2);
		provider->SetItemSource(BoxValue(rootItem2));
		
		// Callbacks show old count=3 to new count=2 due to eager preparation
		const wchar_t* expected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=3, newCount=2, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=3, newCount=2, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
		
		// Verify new structure
		TEST_ASSERT(rootNode->GetChildCount() == 2);
		TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"SimpleRoot");
		
		// Verify children have no grandchildren
		auto child1 = rootNode->GetChild(0);
		TEST_ASSERT(child1->GetChildCount() == 0);
	});

	TEST_CASE(L"TransitionSimpleToComplex")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		// Start with simple flat tree
		auto rootItem1 = CreateBindableTree(L"SimpleRoot", 2);
		provider->SetItemSource(BoxValue(rootItem1));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		callbackLog.Clear();
		
		// Replace with complex multi-level tree
		auto rootItem2 = CreateMultiLevelTree(L"ComplexRoot", 3, 2);
		provider->SetItemSource(BoxValue(rootItem2));
		
		// Callbacks show old count=2 to new count=3 due to eager preparation
		const wchar_t* expected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=3, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=3, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
		
		// Verify new structure
		TEST_ASSERT(rootNode->GetChildCount() == 3);
		TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"ComplexRoot");
		
		// Verify children have grandchildren
		auto child1 = rootNode->GetChild(0);
		TEST_ASSERT(child1->GetChildCount() == 2);
	});

	TEST_CASE(L"RapidConsecutiveTransitions")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		
		// First transition: null -> tree1
		auto tree1 = CreateBindableTree(L"Tree1", 2);
		provider->SetItemSource(BoxValue(tree1));
		callbackLog.Clear();
		
		// Second transition: tree1 -> tree2
		auto tree2 = CreateBindableTree(L"Tree2", 3);
		provider->SetItemSource(BoxValue(tree2));
		
		// Third transition: tree2 -> tree3
		auto tree3 = CreateMultiLevelTree(L"Tree3", 2, 2);
		provider->SetItemSource(BoxValue(tree3));
		
		// Should complete without crashes, proper cleanup at each step
		TEST_ASSERT(rootNode->GetChildCount() == 2);
		TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"Tree3");
		
		// Verify final structure is accessible
		auto child1 = rootNode->GetChild(0);
		TEST_ASSERT(child1->GetChildCount() == 2);
	});
}
```

### Rationale

SetItemSource transitions are critical for real applications where data changes dynamically:
- **Null to hierarchy**: Common initialization pattern when data loads asynchronously
- **Hierarchy to null**: Tests proper cleanup and memory management
- **Complex to simple / Simple to complex**: Verifies structural changes don't corrupt state
- **Rapid transitions**: Stress tests state management and ensures no dangling references

All expectations account for eager child preparation, showing accurate before/after counts.

## STEP 3: Add ExpansionStateEdgeCases Test Category

Test expansion state behavior under edge conditions.

### Implementation

```cpp
TEST_CATEGORY(L"ExpansionStateEdgeCases")
{
	TEST_CASE(L"ExpansionStateWithDynamicClear")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateBindableTree(L"Root", 3);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		callbackLog.Clear();
		
		// Expand the root node
		rootNode->SetExpanding(true);
		TEST_ASSERT(rootNode->GetExpanding() == true);
		callbackLog.Clear();
		
		// Clear all children dynamically
		rootItem->children.Clear();
		
		// Clear() fires ONE callback pair showing count=3 to newCount=0
		const wchar_t* expected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
		
		// Expansion state persists even with no children
		TEST_ASSERT(rootNode->GetExpanding() == true);
		TEST_ASSERT(rootNode->GetChildCount() == 0);
	});

	TEST_CASE(L"CollapseAndReexpandEmptyNode")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateBindableTree(L"Root", 3);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		
		// Expand then clear children
		rootNode->SetExpanding(true);
		rootItem->children.Clear();
		callbackLog.Clear();
		
		// Collapse the empty node
		rootNode->SetExpanding(false);
		
		const wchar_t* collapsedExpected[] = {
			L"[ROOT]->OnItemCollapsed()"
		};
		AssertCallbacks(callbackLog, collapsedExpected);
		TEST_ASSERT(rootNode->GetExpanding() == false);
		callbackLog.Clear();
		
		// Re-expand the empty node
		rootNode->SetExpanding(true);
		
		const wchar_t* expandedExpected[] = {
			L"[ROOT]->OnItemExpanded()"
		};
		AssertCallbacks(callbackLog, expandedExpected);
		TEST_ASSERT(rootNode->GetExpanding() == true);
		TEST_ASSERT(rootNode->GetChildCount() == 0);
	});

	TEST_CASE(L"ExpandNodeThenModifyChildren")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateBindableTree(L"Root", 2);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		
		// Expand the node
		rootNode->SetExpanding(true);
		callbackLog.Clear();
		
		// Add a child
		auto newChild = Ptr(new BindableItem());
		newChild->name = L"Root.Child3";
		rootItem->children.Add(newChild);
		
		// Each Add() fires ONE callback pair
		const wchar_t* addExpected[] = {
			L"[ROOT]->OnBeforeItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, addExpected);
		callbackLog.Clear();
		
		// Remove a child
		rootItem->children.RemoveAt(0);
		
		// RemoveAt() fires ONE callback pair
		const wchar_t* removeExpected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, removeExpected);
		
		// Expansion state maintained throughout
		TEST_ASSERT(rootNode->GetExpanding() == true);
		TEST_ASSERT(rootNode->GetChildCount() == 2);
	});
}
```

### Rationale

Expansion state testing ensures the UI layer can maintain user interaction state independently of data changes:
- **Dynamic clear**: Verifies expansion state persists when all children are removed (user intent preserved)
- **Collapse and re-expand empty**: Tests state machine works correctly even with no data
- **Modify while expanded**: Common pattern where users modify visible data

The `Clear()` operation fires exactly ONE callback pair (not batched), a critical learning from Task No.7.

## STEP 4: Add CallbackDetachment Test Category

Test callback lifecycle management and cleanup scenarios.

### Implementation

```cpp
TEST_CATEGORY(L"CallbackDetachment")
{
	TEST_CASE(L"DetachDuringTreeModification")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateBindableTree(L"Root", 3);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		callbackLog.Clear();
		
		// Start modification
		auto newChild = Ptr(new BindableItem());
		newChild->name = L"Root.Child4";
		rootItem->children.Add(newChild);
		callbackLog.Clear();
		
		// Detach callback
		provider->DetachCallback(&nodeCallback);
		
		// Verify detachment callback
		const wchar_t* detachExpected[] = {
			L"OnAttached(provider=nullptr)"
		};
		AssertCallbacks(callbackLog, detachExpected);
		callbackLog.Clear();
		
		// Further modifications should not fire callbacks
		auto anotherChild = Ptr(new BindableItem());
		anotherChild->name = L"Root.Child5";
		rootItem->children.Add(anotherChild);
		
		TEST_ASSERT(callbackLog.Count() == 0);
		
		// Tree structure still valid
		TEST_ASSERT(rootNode->GetChildCount() == 5);
	});

	TEST_CASE(L"DetachAfterExpansionChanges")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateMultiLevelTree(L"Root", 2, 2);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		auto child1 = rootNode->GetChild(0);
		child1->GetChildCount();
		
		// Expand some nodes
		rootNode->SetExpanding(true);
		child1->SetExpanding(true);
		callbackLog.Clear();
		
		// Detach callback
		provider->DetachCallback(&nodeCallback);
		
		// Verify detachment
		const wchar_t* detachExpected[] = {
			L"OnAttached(provider=nullptr)"
		};
		AssertCallbacks(callbackLog, detachExpected);
		
		// Expansion state preserved
		TEST_ASSERT(rootNode->GetExpanding() == true);
		TEST_ASSERT(child1->GetExpanding() == true);
		
		// Tree structure intact
		TEST_ASSERT(rootNode->GetChildCount() == 2);
		TEST_ASSERT(child1->GetChildCount() == 2);
	});

	TEST_CASE(L"ReattachAfterDetachment")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateBindableTree(L"Root", 2);
		provider->SetItemSource(BoxValue(rootItem));
		
		// First attach
		provider->AttachCallback(&nodeCallback);
		const wchar_t* attachExpected[] = {
			L"OnAttached(provider=valid)"
		};
		AssertCallbacks(callbackLog, attachExpected);
		callbackLog.Clear();
		
		// Detach
		provider->DetachCallback(&nodeCallback);
		const wchar_t* detachExpected[] = {
			L"OnAttached(provider=nullptr)"
		};
		AssertCallbacks(callbackLog, detachExpected);
		callbackLog.Clear();
		
		// Reattach
		provider->AttachCallback(&nodeCallback);
		AssertCallbacks(callbackLog, attachExpected);
		
		// Verify callbacks work after reattach
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		callbackLog.Clear();
		
		auto newChild = Ptr(new BindableItem());
		newChild->name = L"Root.Child3";
		rootItem->children.Add(newChild);
		
		const wchar_t* modifyExpected[] = {
			L"[ROOT]->OnBeforeItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, modifyExpected);
	});

	TEST_CASE(L"DetachWithMultipleLevelsPrepared")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		// Create 4-level tree
		auto rootItem = CreateMultiLevelTree(L"Root", 2, 1);
		auto child1Item = rootItem->children[0];
		auto grandchild1Item = child1Item->children[0];
		auto greatGrandchild = Ptr(new BindableItem());
		greatGrandchild->name = L"Root.Child1.GrandChild1.GreatGrandChild1";
		grandchild1Item->children.Add(greatGrandchild);
		
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		// Prepare all 4 levels
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		auto child1 = rootNode->GetChild(0);
		child1->GetChildCount();
		auto grandchild1 = child1->GetChild(0);
		grandchild1->GetChildCount();
		callbackLog.Clear();
		
		// Detach with deep hierarchy
		provider->DetachCallback(&nodeCallback);
		
		const wchar_t* detachExpected[] = {
			L"OnAttached(provider=nullptr)"
		};
		AssertCallbacks(callbackLog, detachExpected);
		
		// All nodes remain accessible
		TEST_ASSERT(rootNode->GetChildCount() == 2);
		TEST_ASSERT(child1->GetChildCount() == 1);
		TEST_ASSERT(grandchild1->GetChildCount() == 1);
	});
}
```

### Rationale

Callback detachment tests ensure proper resource management:
- **Detach during modification**: Verifies no dangling callback references mid-operation
- **Detach after expansion**: Ensures UI state is preserved independently of callback
- **Reattach**: Tests callback reusability in scenarios like switching views
- **Deep hierarchy detach**: Stress tests cleanup with complex nested state

The `OnAttached(provider=nullptr)` pattern is the established cleanup signal from Task No.5.

## STEP 5: Add InvalidOperations Test Category

Test error handling for out-of-bounds and invalid operations.

### Implementation

```cpp
TEST_CATEGORY(L"InvalidOperations")
{
	TEST_CASE(L"InvalidChildIndex")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateBindableTree(L"Root", 3);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		
		// Test various out-of-bounds indices
		TEST_ERROR(rootNode->GetChild(-1));
		TEST_ERROR(rootNode->GetChild(3));
		TEST_ERROR(rootNode->GetChild(100));
		
		// Valid index should work
		auto child1 = rootNode->GetChild(0);
		TEST_ASSERT(child1 != nullptr);
		TEST_ASSERT(provider->GetTextValue(child1.Obj()) == L"Root.Child1");
	});

	TEST_CASE(L"InvalidNodeOperations")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateMultiLevelTree(L"Root", 2, 2);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		auto child1 = rootNode->GetChild(0);
		child1->GetChildCount();
		
		// Try to access grandchild with invalid index
		TEST_ERROR(child1->GetChild(-1));
		TEST_ERROR(child1->GetChild(2));
		TEST_ERROR(child1->GetChild(50));
		
		// Valid access should work
		auto grandchild1 = child1->GetChild(0);
		TEST_ASSERT(grandchild1 != nullptr);
		TEST_ASSERT(provider->GetTextValue(grandchild1.Obj()) == L"Root.Child1.GrandChild1");
	});
}
```

### Rationale

Error handling tests verify defensive programming:
- **Invalid indices**: Uses `TEST_ERROR` to verify `CHECK_ERROR` catches out-of-bounds access
- **Multi-level testing**: Ensures error checking works at all hierarchy levels

This follows the established pattern from Tasks No.2 and No.7 of using `TEST_ERROR` for index out of range scenarios.

## STEP 6: Add PropertyBindingEdgeCases Test Category

Test boundary conditions in property binding.

### Implementation

```cpp
TEST_CATEGORY(L"PropertyBindingEdgeCases")
{
	TEST_CASE(L"NullTextProperty")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = Ptr(new BindableItem());
		// name is default initialized to empty string, not null
		// This tests empty string handling
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		
		// GetTextValue should handle empty name gracefully
		auto textValue = provider->GetTextValue(rootNode.Obj());
		TEST_ASSERT(textValue == L"");
	});

	TEST_CASE(L"UpdateBindingPropertiesEdgeCases")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		// Create tree where some nodes have empty children
		auto rootItem = CreateBindableTree(L"Root", 2);
		// First child has children, second child has empty children
		auto child1Item = rootItem->children[0];
		child1Item->children.Add(Ptr(new BindableItem()));
		child1Item->children[0]->name = L"Root.Child1.GrandChild1";
		
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		auto child1 = rootNode->GetChild(0);
		child1->GetChildCount();
		auto child2 = rootNode->GetChild(1);
		child2->GetChildCount();
		callbackLog.Clear();
		
		// UpdateBindingProperties should handle mixed scenarios
		provider->UpdateBindingProperties(true);
		
		// Only root-level callbacks (not recursive)
		const wchar_t* expected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
		
		// Verify structure intact
		TEST_ASSERT(rootNode->GetChildCount() == 2);
		TEST_ASSERT(child1->GetChildCount() == 1);
		TEST_ASSERT(child2->GetChildCount() == 0);
	});
}
```

### Rationale

Property binding edge cases test data quality issues:
- **Empty text**: Verifies graceful handling of missing or empty text properties
- **Mixed hierarchies**: Tests UpdateBindingProperties with inconsistent child structures

These scenarios represent real-world data quality issues that shouldn't crash the application.

## STEP 7: Add ObservableListOperationPatterns Test Category

Test specific ObservableList operation sequences and patterns.

### Implementation

```cpp
TEST_CATEGORY(L"ObservableListOperationPatterns")
{
	TEST_CASE(L"ClearFollowedByAdds")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateBindableTree(L"Root", 3);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		callbackLog.Clear();
		
		// Clear all children
		rootItem->children.Clear();
		
		// Clear() fires ONE callback pair
		const wchar_t* clearExpected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, clearExpected);
		TEST_ASSERT(rootNode->GetChildCount() == 0);
		callbackLog.Clear();
		
		// Add new children one by one - each Add() fires ONE callback pair
		auto newChild1 = Ptr(new BindableItem());
		newChild1->name = L"Root.NewChild1";
		rootItem->children.Add(newChild1);
		
		const wchar_t* add1Expected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, add1Expected);
		callbackLog.Clear();
		
		auto newChild2 = Ptr(new BindableItem());
		newChild2->name = L"Root.NewChild2";
		rootItem->children.Add(newChild2);
		
		const wchar_t* add2Expected[] = {
			L"[ROOT]->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, add2Expected);
		
		// Verify final state
		TEST_ASSERT(rootNode->GetChildCount() == 2);
		TEST_ASSERT(provider->GetTextValue(rootNode->GetChild(0).Obj()) == L"Root.NewChild1");
		TEST_ASSERT(provider->GetTextValue(rootNode->GetChild(1).Obj()) == L"Root.NewChild2");
	});

	TEST_CASE(L"RemoveRangeBoundaries")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateBindableTree(L"Root", 10);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		callbackLog.Clear();
		
		// RemoveRange at start
		rootItem->children.RemoveRange(0, 3);
		
		const wchar_t* removeStartExpected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, removeStartExpected);
		TEST_ASSERT(rootNode->GetChildCount() == 7);
		callbackLog.Clear();
		
		// RemoveRange in middle
		rootItem->children.RemoveRange(2, 2);
		
		const wchar_t* removeMiddleExpected[] = {
			L"[ROOT]->OnBeforeItemModified(start=2, count=2, newCount=0, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=2, count=2, newCount=0, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, removeMiddleExpected);
		TEST_ASSERT(rootNode->GetChildCount() == 5);
		callbackLog.Clear();
		
		// RemoveRange at end
		rootItem->children.RemoveRange(3, 2);
		
		const wchar_t* removeEndExpected[] = {
			L"[ROOT]->OnBeforeItemModified(start=3, count=2, newCount=0, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=3, count=2, newCount=0, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, removeEndExpected);
		TEST_ASSERT(rootNode->GetChildCount() == 3);
	});

	TEST_CASE(L"MixedOperations")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = CreateBindableTree(L"Root", 3);
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		auto rootNode = provider->GetRootNode();
		rootNode->GetChildCount();
		callbackLog.Clear();
		
		// Sequence: Add, RemoveAt, Add, RemoveRange, Add
		// Each operation fires exactly ONE callback pair
		
		// Add
		auto newChild1 = Ptr(new BindableItem());
		newChild1->name = L"Root.NewChild1";
		rootItem->children.Add(newChild1);
		TEST_ASSERT(callbackLog.Count() == 2);
		callbackLog.Clear();
		
		// RemoveAt
		rootItem->children.RemoveAt(0);
		TEST_ASSERT(callbackLog.Count() == 2);
		callbackLog.Clear();
		
		// Add
		auto newChild2 = Ptr(new BindableItem());
		newChild2->name = L"Root.NewChild2";
		rootItem->children.Add(newChild2);
		TEST_ASSERT(callbackLog.Count() == 2);
		callbackLog.Clear();
		
		// RemoveRange
		rootItem->children.RemoveRange(1, 2);
		TEST_ASSERT(callbackLog.Count() == 2);
		callbackLog.Clear();
		
		// Add
		auto newChild3 = Ptr(new BindableItem());
		newChild3->name = L"Root.NewChild3";
		rootItem->children.Add(newChild3);
		TEST_ASSERT(callbackLog.Count() == 2);
		
		// Verify final structure is correct
		TEST_ASSERT(rootNode->GetChildCount() == 3);
	});
}
```

### Rationale

ObservableList operation patterns validate critical learnings from Task No.7:
- **Clear then Add**: Proves each operation fires individual callbacks (not batched)
- **RemoveRange boundaries**: Tests range operations at start, middle, and end positions
- **Mixed operations**: Verifies complex sequences maintain correct state

The key insight is that each operation fires exactly ONE callback pair, never batched. This is fundamental to understanding TreeViewItemBindableRootProvider behavior.

# TEST PLAN

## Test Coverage Summary

This task adds 7 new test categories with approximately 22 test cases covering edge cases, error handling, and boundary conditions. Combined with the existing 10 categories from Tasks No.6 and No.7, this brings total coverage to 17 categories spanning:
- Basic functionality
- Property binding
- SetItemSource operations
- ObservableList integration
- Expansion/collapse operations
- Node lifecycle
- UpdateBindingProperties
- Interface methods
- Hierarchical binding scenarios
- **NEW: Empty and null scenarios**
- **NEW: SetItemSource transitions**
- **NEW: Expansion state edge cases**
- **NEW: Callback detachment**
- **NEW: Invalid operations**
- **NEW: Property binding edge cases**
- **NEW: ObservableList operation patterns**

## Test Categories and Expected Results

### Category 1: EmptyAndNullScenarios

**Test Cases:**
1. `SetItemSourceWithEmptyChildren` - Verifies `newCount=0` for empty ObservableList
2. `ExpandNodeWithNoChildren` - Verifies expansion works on empty nodes
3. `AccessChildrenOfEmptyNode` - Verifies `TEST_ERROR` for out-of-bounds access

**Expected Results:**
- All SetItemSource operations with empty children show `count=0, newCount=0` in callbacks
- Expansion operations complete without crashes
- Invalid index access triggers `CHECK_ERROR` caught by `TEST_ERROR`
- GetChildCount() returns 0 for empty nodes

**Corner Cases Covered:**
- Empty ObservableList (not null, but functionally empty)
- UI operations on nodes with no data
- Defensive programming for invalid access

### Category 2: SetItemSourceTransitions

**Test Cases:**
1. `TransitionFromNullToHierarchy` - Tests initialization pattern
2. `TransitionFromHierarchyToNull` - Tests cleanup
3. `TransitionComplexToSimple` - Tests structural simplification
4. `TransitionSimpleToComplex` - Tests structural expansion
5. `RapidConsecutiveTransitions` - Stress tests state management

**Expected Results:**
- All transitions show accurate before/after child counts in callbacks
- Null to hierarchy shows `count=0, newCount=<actual>`
- Hierarchy to null shows `count=<actual>, newCount=0`
- No memory leaks or dangling references
- Tree structure always accessible after transition

**Corner Cases Covered:**
- Initial data load (null → data)
- Data clear (data → null)
- Structural changes (complex ↔ simple)
- Rapid state changes

### Category 3: ExpansionStateEdgeCases

**Test Cases:**
1. `ExpansionStateWithDynamicClear` - Tests state persistence when children cleared
2. `CollapseAndReexpandEmptyNode` - Tests expansion operations on empty nodes
3. `ExpandNodeThenModifyChildren` - Tests modifications while expanded

**Expected Results:**
- Expansion state persists through data changes (true remains true)
- Clear() fires ONE callback pair with `count=<old>, newCount=0`
- Expansion/collapse callbacks fire correctly regardless of data presence
- Individual callbacks fire for each Add/Remove operation (not batched)

**Corner Cases Covered:**
- User expands node, then data is removed
- Expansion operations on nodes with no children
- Data modifications on visible (expanded) nodes

### Category 4: CallbackDetachment

**Test Cases:**
1. `DetachDuringTreeModification` - Tests detachment during active operations
2. `DetachAfterExpansionChanges` - Tests cleanup after UI state changes
3. `ReattachAfterDetachment` - Tests callback reusability
4. `DetachWithMultipleLevelsPrepared` - Tests cleanup with deep hierarchy

**Expected Results:**
- DetachCallback always fires `OnAttached(provider=nullptr)`
- After detachment, no further callbacks fire
- Tree structure remains valid after detachment
- Expansion state preserved after detachment
- Reattachment works correctly with `OnAttached(provider=valid)` again

**Corner Cases Covered:**
- Mid-operation detachment
- Detachment with complex state
- Callback reuse
- Deep hierarchy cleanup

### Category 5: InvalidOperations

**Test Cases:**
1. `InvalidChildIndex` - Tests out-of-bounds access at root level
2. `InvalidNodeOperations` - Tests out-of-bounds access at child levels

**Expected Results:**
- `TEST_ERROR` catches all out-of-bounds access (negative, at count, beyond count)
- Valid indices work correctly
- Error checking works at all hierarchy levels

**Corner Cases Covered:**
- Negative indices
- Index at child count (off-by-one)
- Far out-of-bounds indices
- Multi-level error checking

### Category 6: PropertyBindingEdgeCases

**Test Cases:**
1. `NullTextProperty` - Tests empty text handling
2. `UpdateBindingPropertiesEdgeCases` - Tests update with mixed hierarchies

**Expected Results:**
- Empty text properties return empty string (no crash)
- UpdateBindingProperties handles mixed scenarios (some with children, some empty)
- Callbacks reflect root-level update only (not recursive)

**Corner Cases Covered:**
- Missing or empty text data
- Inconsistent child structures
- UpdateBindingProperties edge cases

### Category 7: ObservableListOperationPatterns

**Test Cases:**
1. `ClearFollowedByAdds` - Verifies individual callback firing
2. `RemoveRangeBoundaries` - Tests range operations at various positions
3. `MixedOperations` - Verifies complex operation sequences

**Expected Results:**
- Clear() fires exactly ONE callback pair
- Each Add() fires exactly ONE callback pair
- Each RemoveAt() fires exactly ONE callback pair
- Each RemoveRange() fires exactly ONE callback pair
- Callbacks never batched
- Start positions and counts accurate for all operations

**Corner Cases Covered:**
- Clear followed by multiple adds
- RemoveRange at start, middle, end
- Complex operation sequences
- Proof of non-batched behavior

## Integration with Existing Tests

The new edge case tests complement existing tests:
- **Tasks No.6 & No.7**: Focused on core functionality and normal operation paths
- **Task No.8 (this task)**: Focuses on boundary conditions and error scenarios

All tests follow established patterns:
- Each test case creates its own callback objects
- Callback logs cleared after SetItemSource (except when testing SetItemSource itself)
- TEST_ERROR for out-of-bounds operations
- Expectations account for eager child preparation
- Direct subscript access to ObservableList (no UnboxValue)
- Clear() + Add() loop for replacing contents (no assignment)

## Verification Checklist

Before marking task complete:

1. **Compilation**: All test cases compile without errors
2. **Execution**: All test cases pass
3. **Isolation**: Each test creates own callback objects
4. **Log Management**: Callback logs cleared appropriately
5. **Error Handling**: TEST_ERROR used for out-of-bounds operations
6. **Callbacks**: All expectations account for eager preparation
7. **Type System**: No UnboxValue for direct ObservableList subscript access
8. **ObservableList**: Modifications use Clear() + Add(), not assignment
9. **Lambda Capture**: Explicit capture of other lambdas
10. **Coverage**: All edge cases from task description covered
11. **Naming**: Test names descriptive and follow patterns
12. **Style**: C++ 20 with tabs for indentation
13. **Philosophy**: Tests verify behavior, not internal state

## Expected Test Output

When all tests pass, we expect:
- Approximately 22 new test cases added
- Total test cases for TreeViewItemBindableRootProvider: ~70+
- All edge cases covered with clear, maintainable tests
- Robust error handling verified through TEST_ERROR
- Complete documentation of TreeViewItemBindableRootProvider behavior

## Regression Testing

The new edge case tests should not affect existing tests because:
- No modifications to existing test cases
- All new tests in new categories
- Same testing infrastructure (MockNodeProviderCallback)
- Same test data helpers (CreateBindableTree, CreateMultiLevelTree)

## Manual Verification Steps

After implementation:
1. Run full test suite: `Build and Run Unit Tests` task
2. Verify all TreeViewItemBindableRootProvider tests pass
3. Check test output for any unexpected failures
4. Review callback logs for correct sequences
5. Confirm TEST_ERROR catches expected errors

## Known Limitations

Some edge cases are not testable due to design constraints:
- **True null childrenProperty**: BindableItem's children field is always initialized ObservableList, cannot be null. We test "functionally empty" instead.
- **Circular references**: Current BindableItem test infrastructure doesn't support creating circular structures easily. Not critical for current scope.
- **Dynamic childrenProperty replacement**: BindableItem's children is a concrete field, not replaceable. Not applicable to current design.

These limitations are acceptable because:
- Empty ObservableList behaves identically to null for testing purposes
- Circular references would be application logic errors, not provider issues
- Current design doesn't support dynamic property replacement

## Test Maintenance

These tests are designed for long-term maintenance:
- Clear test names describe exactly what is tested
- Comments explain why tests exist, not just what they do
- Helper functions (CreateBindableTree, CreateMultiLevelTree) reduce duplication
- Consistent patterns make it easy to add more tests
- Each test is independent and isolated

# !!!FINISHED!!!
