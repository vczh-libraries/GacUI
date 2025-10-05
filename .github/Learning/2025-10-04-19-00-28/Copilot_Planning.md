# !!!PLANNING!!!

# UPDATES

## UPDATE

TASK No.7: Test hierarchical binding scenarios for TreeViewItemBindableRootProvider

This task focuses on testing complex hierarchical scenarios with multi-level data structures, dynamic structure changes, and callback coordination across tree levels.

Key requirements:
- Continue expanding `TestItemProviders_TreeViewItemBindableRootProvider.cpp` with hierarchical binding scenario test cases
- Each test case must create its own callbackLog and callback objects for isolation and robustness
- Clear callback logs after SetItemSource calls (except when testing SetItemSource behavior itself)
- Test multi-level ObservableList changes and their callback propagation through tree hierarchy
- Test childrenProperty modifications and their impact on tree structure at various levels
- Test expansion state coordination when underlying data structure changes dynamically
- Test parent item property changes affecting child node visibility/structure
- Test callback coordination using only existing callback interfaces across tree levels
- Test manual notification scenarios for hierarchical property changes
- Test RemoveRange operations on hierarchical ObservableList structures
- Skip testing duplicate bindable items in the tree structure (Value objects cannot be reliably detected as duplicates)
- **CRITICAL**: Account for eager child preparation - all callback expectations must expect actual child counts, not zero

# IMPROVEMENT PLAN

## STEP 1: Add Multi-Level Tree Creation Helper

**What to change**: Add a new helper function `CreateMultiLevelTree` in the test file after the existing `CreateBindableTree` helper function.

**Code to add** (after line ~23 where `CreateBindableTree` is defined):

```cpp
auto CreateMultiLevelTree = [](const WString& rootName, vint level1Count, vint level2Count) -> Ptr<BindableItem>
{
	auto root = CreateBindableTree(rootName, level1Count);
	for (vint i = 0; i < level1Count; i++)
	{
		auto child = UnboxValue<Ptr<BindableItem>>(root->children[i]);
		for (vint j = 0; j < level2Count; j++)
		{
			auto grandchild = Ptr(new BindableItem());
			grandchild->name = rootName + L".Child" + itow(i + 1) + L".GrandChild" + itow(j + 1);
			child->children.Add(grandchild);
		}
	}
	return root;
};
```

**Why**: Hierarchical tests require multi-level tree structures. This helper enables consistent creation of 3-level trees (root → children → grandchildren) without code duplication. It builds upon `CreateBindableTree` to ensure consistency with existing tests.

## STEP 2: Add Hierarchical Binding Test Category

**What to change**: Add a new `TEST_CATEGORY(L"HierarchicalBindingScenarios")` at the end of the test file (before the closing brace of `TEST_FILE`).

**Why**: Task No.7 focuses specifically on hierarchical binding scenarios, which are distinct from the single-level operations tested in Task No.6. Grouping these tests in a dedicated category improves organization and makes it clear which tests verify multi-level behavior.

## STEP 3: Test Multi-Level ObservableList Add Operations

**What to change**: Add test case `ObservableListAddAtGrandchildLevel` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ObservableListAddAtGrandchildLevel")
{
	List<WString> callbackLog;
	MockNodeProviderCallback nodeCallback(callbackLog);
	
	auto provider = Ptr(new TreeViewItemBindableRootProvider());
	InitProvider(provider);
	
	auto rootItem = CreateMultiLevelTree(L"Root", 2, 1);
	provider->SetItemSource(BoxValue(rootItem));
	provider->AttachCallback(&nodeCallback);
	
	// Prepare all children to level 2 to subscribe to ObservableList events
	auto rootNode = provider->GetRootNode();
	rootNode->GetChildCount();
	auto child1 = rootNode->GetChild(0);
	child1->GetChildCount();
	callbackLog.Clear();
	
	// Add grandchild to Child1's children (level 2 operation)
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	auto newGrandchild = Ptr(new BindableItem());
	newGrandchild->name = L"Root.Child1.GrandChild2";
	child1Item->children.Add(newGrandchild);
	
	// Expect callback on Child1 node (not root), showing actual child count change
	const wchar_t* expected[] = {
		L"Root.Child1->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
		L"Root.Child1->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected);
	
	// Verify tree structure updated correctly
	TEST_ASSERT(child1->GetChildCount() == 2);
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(1).Obj()) == L"Root.Child1.GrandChild2");
}
```

**Why**: Verifies that ObservableList modifications at level 2 (grandchildren) trigger callbacks with the correct node context (Child1, not root). This tests callback propagation through the hierarchy. The test accounts for eager child preparation by expecting `start=1` (after the existing prepared grandchild).

## STEP 4: Test Multi-Level ObservableList Remove Operations

**What to change**: Add test case `ObservableListRemoveAtGrandchildLevel` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ObservableListRemoveAtGrandchildLevel")
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
	callbackLog.Clear();
	
	// Remove first grandchild from Child1
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	child1Item->children.RemoveAt(0);
	
	// Expect callback showing prepared child count reduction (2 → 1)
	const wchar_t* expected[] = {
		L"Root.Child1->OnBeforeItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)",
		L"Root.Child1->OnAfterItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected);
	
	TEST_ASSERT(child1->GetChildCount() == 1);
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(0).Obj()) == L"Root.Child1.GrandChild2");
}
```

**Why**: Tests removal at level 2, verifying callbacks fire with correct node path and show actual prepared child counts. This complements the Add test and ensures bidirectional operations work correctly at nested levels.

## STEP 5: Test Multi-Level ObservableList Clear Operations

**What to change**: Add test case `ObservableListClearAtGrandchildLevel` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ObservableListClearAtGrandchildLevel")
{
	List<WString> callbackLog;
	MockNodeProviderCallback nodeCallback(callbackLog);
	
	auto provider = Ptr(new TreeViewItemBindableRootProvider());
	InitProvider(provider);
	
	auto rootItem = CreateMultiLevelTree(L"Root", 2, 3);
	provider->SetItemSource(BoxValue(rootItem));
	provider->AttachCallback(&nodeCallback);
	
	auto rootNode = provider->GetRootNode();
	rootNode->GetChildCount();
	auto child1 = rootNode->GetChild(0);
	child1->GetChildCount();
	callbackLog.Clear();
	
	// Clear all grandchildren from Child1
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	child1Item->children.Clear();
	
	// Expect callback showing all 3 prepared children being removed
	const wchar_t* expected[] = {
		L"Root.Child1->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
		L"Root.Child1->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected);
	
	TEST_ASSERT(child1->GetChildCount() == 0);
}
```

**Why**: Tests bulk removal via Clear() at level 2, verifying that callbacks correctly report the actual number of prepared children being removed (count=3), not zero. This is critical for validating hierarchical cleanup operations.

## STEP 6: Test ObservableList Operations at Level 3

**What to change**: Add test case `ObservableListAddAtGreatGrandchildLevel` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ObservableListAddAtGreatGrandchildLevel")
{
	List<WString> callbackLog;
	MockNodeProviderCallback nodeCallback(callbackLog);
	
	auto provider = Ptr(new TreeViewItemBindableRootProvider());
	InitProvider(provider);
	
	// Create 4-level tree: Root → Child → GrandChild → GreatGrandChild
	auto rootItem = CreateMultiLevelTree(L"Root", 1, 1);
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	auto grandchild1Item = UnboxValue<Ptr<BindableItem>>(child1Item->children[0]);
	// Initially no great-grandchildren
	
	provider->SetItemSource(BoxValue(rootItem));
	provider->AttachCallback(&nodeCallback);
	
	// Prepare to level 3
	auto rootNode = provider->GetRootNode();
	rootNode->GetChildCount();
	auto child1 = rootNode->GetChild(0);
	child1->GetChildCount();
	auto grandchild1 = child1->GetChild(0);
	grandchild1->GetChildCount();
	callbackLog.Clear();
	
	// Add great-grandchild at level 3
	auto greatGrandchild = Ptr(new BindableItem());
	greatGrandchild->name = L"Root.Child1.GrandChild1.GreatGrandChild1";
	grandchild1Item->children.Add(greatGrandchild);
	
	// Expect callback on GrandChild1 node
	const wchar_t* expected[] = {
		L"Root.Child1.GrandChild1->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
		L"Root.Child1.GrandChild1->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected);
	
	TEST_ASSERT(grandchild1->GetChildCount() == 1);
	TEST_ASSERT(provider->GetTextValue(grandchild1->GetChild(0).Obj()) == L"Root.Child1.GrandChild1.GreatGrandChild1");
}
```

**Why**: Tests operations at level 3 (great-grandchildren) to verify the implementation handles deep hierarchies correctly. The callback node path should be the grandchild node, demonstrating proper context propagation at arbitrary depths.

## STEP 7: Test RemoveRange at Multiple Levels

**What to change**: Add test case `ObservableListRemoveRangeAtMultipleLevels` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ObservableListRemoveRangeAtMultipleLevels")
{
	List<WString> callbackLog;
	MockNodeProviderCallback nodeCallback(callbackLog);
	
	auto provider = Ptr(new TreeViewItemBindableRootProvider());
	InitProvider(provider);
	
	auto rootItem = CreateMultiLevelTree(L"Root", 3, 4);
	provider->SetItemSource(BoxValue(rootItem));
	provider->AttachCallback(&nodeCallback);
	
	auto rootNode = provider->GetRootNode();
	rootNode->GetChildCount();
	auto child2 = rootNode->GetChild(1);
	child2->GetChildCount();
	callbackLog.Clear();
	
	// RemoveRange on Child2's grandchildren (remove middle 2 out of 4)
	auto child2Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[1]);
	child2Item->children.RemoveRange(1, 2);
	
	// Expect callback showing 2 items removed from position 1
	const wchar_t* expected[] = {
		L"Root.Child2->OnBeforeItemModified(start=1, count=2, newCount=0, itemReferenceUpdated=true)",
		L"Root.Child2->OnAfterItemModified(start=1, count=2, newCount=0, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected);
	
	TEST_ASSERT(child2->GetChildCount() == 2);
	TEST_ASSERT(provider->GetTextValue(child2->GetChild(0).Obj()) == L"Root.Child2.GrandChild1");
	TEST_ASSERT(provider->GetTextValue(child2->GetChild(1).Obj()) == L"Root.Child2.GrandChild4");
}
```

**Why**: Tests RemoveRange (bulk removal) at level 2, which is distinct from single-item removal. This verifies that callback parameters correctly reflect the range being removed (`count=2`) and that remaining items are in correct order.

## STEP 8: Test Nested Callback Node Path Verification

**What to change**: Add test case `NestedCallbackNodePathVerification` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"NestedCallbackNodePathVerification")
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
	auto child2 = rootNode->GetChild(1);
	child2->GetChildCount();
	callbackLog.Clear();
	
	// Modify at different levels and verify node paths in callbacks
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	auto newGrandchild = Ptr(new BindableItem());
	newGrandchild->name = L"Root.Child1.GrandChild3";
	child1Item->children.Add(newGrandchild);
	
	// First callback set should show Child1 as the node
	TEST_ASSERT(callbackLog[0] == L"Root.Child1->OnBeforeItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)");
	TEST_ASSERT(callbackLog[1] == L"Root.Child1->OnAfterItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)");
	
	callbackLog.Clear();
	
	// Now modify Child2
	auto child2Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[1]);
	child2Item->children.RemoveAt(0);
	
	// Second callback set should show Child2 as the node
	const wchar_t* expected[] = {
		L"Root.Child2->OnBeforeItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)",
		L"Root.Child2->OnAfterItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected);
}
```

**Why**: Explicitly verifies that callbacks fire with the correct node path when modifications occur at different parts of the tree. This ensures the callback context accurately reflects which part of the hierarchy is being modified, not just the root.

## STEP 9: Test Adding Children to Expanded Nodes

**What to change**: Add test case `AddChildrenToExpandedNode` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"AddChildrenToExpandedNode")
{
	List<WString> callbackLog;
	MockNodeProviderCallback nodeCallback(callbackLog);
	
	auto provider = Ptr(new TreeViewItemBindableRootProvider());
	InitProvider(provider);
	
	auto rootItem = CreateMultiLevelTree(L"Root", 2, 1);
	provider->SetItemSource(BoxValue(rootItem));
	provider->AttachCallback(&nodeCallback);
	
	auto rootNode = provider->GetRootNode();
	rootNode->GetChildCount();
	auto child1 = rootNode->GetChild(0);
	child1->GetChildCount();
	
	// Expand child1
	provider->RequestExpanding(child1.Obj());
	TEST_ASSERT(child1->GetExpanding());
	callbackLog.Clear();
	
	// Add grandchild while expanded
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	auto newGrandchild = Ptr(new BindableItem());
	newGrandchild->name = L"Root.Child1.GrandChild2";
	child1Item->children.Add(newGrandchild);
	
	const wchar_t* expected[] = {
		L"Root.Child1->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
		L"Root.Child1->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected);
	
	// Verify expansion state maintained and new child visible
	TEST_ASSERT(child1->GetExpanding());
	TEST_ASSERT(child1->GetChildCount() == 2);
}
```

**Why**: Tests that adding children to an already-expanded node works correctly. The expansion state should be maintained and the new child should be immediately visible. This verifies coordination between expansion state and dynamic structure changes.

## STEP 10: Test Removing Children from Expanded Nodes

**What to change**: Add test case `RemoveChildrenFromExpandedNode` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"RemoveChildrenFromExpandedNode")
{
	List<WString> callbackLog;
	MockNodeProviderCallback nodeCallback(callbackLog);
	
	auto provider = Ptr(new TreeViewItemBindableRootProvider());
	InitProvider(provider);
	
	auto rootItem = CreateMultiLevelTree(L"Root", 2, 3);
	provider->SetItemSource(BoxValue(rootItem));
	provider->AttachCallback(&nodeCallback);
	
	auto rootNode = provider->GetRootNode();
	rootNode->GetChildCount();
	auto child1 = rootNode->GetChild(0);
	child1->GetChildCount();
	
	// Expand child1
	provider->RequestExpanding(child1.Obj());
	callbackLog.Clear();
	
	// Remove grandchild while expanded
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	child1Item->children.RemoveAt(1);
	
	const wchar_t* expected[] = {
		L"Root.Child1->OnBeforeItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)",
		L"Root.Child1->OnAfterItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected);
	
	// Verify expansion state maintained and correct children remain
	TEST_ASSERT(child1->GetExpanding());
	TEST_ASSERT(child1->GetChildCount() == 2);
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(0).Obj()) == L"Root.Child1.GrandChild1");
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(1).Obj()) == L"Root.Child1.GrandChild3");
}
```

**Why**: Tests removal from expanded nodes, ensuring expansion state persists correctly. This verifies that the provider properly handles modifications to visible children without unexpectedly collapsing the node.

## STEP 11: Test Clearing Children of Expanded Nodes

**What to change**: Add test case `ClearChildrenOfExpandedNode` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ClearChildrenOfExpandedNode")
{
	List<WString> callbackLog;
	MockNodeProviderCallback nodeCallback(callbackLog);
	
	auto provider = Ptr(new TreeViewItemBindableRootProvider());
	InitProvider(provider);
	
	auto rootItem = CreateMultiLevelTree(L"Root", 2, 3);
	provider->SetItemSource(BoxValue(rootItem));
	provider->AttachCallback(&nodeCallback);
	
	auto rootNode = provider->GetRootNode();
	rootNode->GetChildCount();
	auto child1 = rootNode->GetChild(0);
	child1->GetChildCount();
	
	// Expand child1
	provider->RequestExpanding(child1.Obj());
	callbackLog.Clear();
	
	// Clear all grandchildren while expanded
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	child1Item->children.Clear();
	
	const wchar_t* expected[] = {
		L"Root.Child1->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
		L"Root.Child1->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected);
	
	// Expansion state should be maintained even with no children
	TEST_ASSERT(child1->GetExpanding());
	TEST_ASSERT(child1->GetChildCount() == 0);
}
```

**Why**: Tests the edge case of clearing all children from an expanded node. The expansion state should persist (the node remains "expanded" even though it now has no children), and callbacks should correctly report the bulk removal.

## STEP 12: Test Expansion State Persistence Across Dynamic Changes

**What to change**: Add test case `ExpansionStatePersistsAcrossDynamicChanges` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ExpansionStatePersistsAcrossDynamicChanges")
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
	
	// Expand child1
	provider->RequestExpanding(child1.Obj());
	TEST_ASSERT(child1->GetExpanding());
	
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	
	// Perform multiple operations: add, remove, add again
	callbackLog.Clear();
	auto newGC1 = Ptr(new BindableItem());
	newGC1->name = L"Root.Child1.GrandChild3";
	child1Item->children.Add(newGC1);
	TEST_ASSERT(child1->GetExpanding());
	TEST_ASSERT(child1->GetChildCount() == 3);
	
	child1Item->children.RemoveAt(0);
	TEST_ASSERT(child1->GetExpanding());
	TEST_ASSERT(child1->GetChildCount() == 2);
	
	auto newGC2 = Ptr(new BindableItem());
	newGC2->name = L"Root.Child1.GrandChild4";
	child1Item->children.Add(newGC2);
	TEST_ASSERT(child1->GetExpanding());
	TEST_ASSERT(child1->GetChildCount() == 3);
	
	// Verify final state
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(0).Obj()) == L"Root.Child1.GrandChild2");
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(1).Obj()) == L"Root.Child1.GrandChild3");
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(2).Obj()) == L"Root.Child1.GrandChild4");
}
```

**Why**: Tests that expansion state survives multiple sequential modifications. This is critical for UI robustness - users expect expanded nodes to remain expanded even as the data structure changes dynamically.

## STEP 13: Test Nested ObservableList Replacement at Level 2

**What to change**: Add test case `ReplaceChildrenAtLevel2` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ReplaceChildrenAtLevel2")
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
	callbackLog.Clear();
	
	// Replace Child1's entire children ObservableList
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	ObservableList<Value> newChildren;
	auto gc1 = Ptr(new BindableItem());
	gc1->name = L"Root.Child1.NewGrandChild1";
	auto gc2 = Ptr(new BindableItem());
	gc2->name = L"Root.Child1.NewGrandChild2";
	auto gc3 = Ptr(new BindableItem());
	gc3->name = L"Root.Child1.NewGrandChild3";
	newChildren.Add(BoxValue(gc1));
	newChildren.Add(BoxValue(gc2));
	newChildren.Add(BoxValue(gc3));
	
	child1Item->children = newChildren;
	child1Item->UpdateProperty(BindableItem::Prop_children());
	
	// Expect callbacks showing old count (2) and new count (3)
	const wchar_t* expected[] = {
		L"Root.Child1->OnBeforeItemModified(start=0, count=2, newCount=3, itemReferenceUpdated=true)",
		L"Root.Child1->OnAfterItemModified(start=0, count=2, newCount=3, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected);
	
	// Verify new children are accessible
	TEST_ASSERT(child1->GetChildCount() == 3);
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(0).Obj()) == L"Root.Child1.NewGrandChild1");
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(2).Obj()) == L"Root.Child1.NewGrandChild3");
}
```

**Why**: Tests replacing an entire ObservableList at level 2, which is a different operation than individual add/remove. The callback should show both the old count (actual prepared children being replaced) and new count (newly prepared children), demonstrating that the provider properly handles collection replacement at nested levels.

## STEP 14: Test Sequential ObservableList Replacement at Multiple Levels

**What to change**: Add test case `ReplaceChildrenAtMultipleLevels` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ReplaceChildrenAtMultipleLevels")
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
	callbackLog.Clear();
	
	// Replace root's children (level 1)
	ObservableList<Value> newRootChildren;
	auto child1 = Ptr(new BindableItem());
	child1->name = L"Root.NewChild1";
	newRootChildren.Add(BoxValue(child1));
	rootItem->children = newRootChildren;
	rootItem->UpdateProperty(BindableItem::Prop_children());
	
	const wchar_t* expected1[] = {
		L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=1, itemReferenceUpdated=true)",
		L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=1, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected1);
	TEST_ASSERT(rootNode->GetChildCount() == 1);
	
	// Prepare new child1 and add grandchildren
	auto newChild1Node = rootNode->GetChild(0);
	newChild1Node->GetChildCount();
	callbackLog.Clear();
	
	// Replace new child1's children (level 2)
	ObservableList<Value> newGrandchildren;
	auto gc1 = Ptr(new BindableItem());
	gc1->name = L"Root.NewChild1.GrandChild1";
	auto gc2 = Ptr(new BindableItem());
	gc2->name = L"Root.NewChild1.GrandChild2";
	newGrandchildren.Add(BoxValue(gc1));
	newGrandchildren.Add(BoxValue(gc2));
	child1->children = newGrandchildren;
	child1->UpdateProperty(BindableItem::Prop_children());
	
	const wchar_t* expected2[] = {
		L"Root.NewChild1->OnBeforeItemModified(start=0, count=0, newCount=2, itemReferenceUpdated=true)",
		L"Root.NewChild1->OnAfterItemModified(start=0, count=0, newCount=2, itemReferenceUpdated=true)"
	};
	AssertCallbacks(callbackLog, expected2);
	TEST_ASSERT(newChild1Node->GetChildCount() == 2);
}
```

**Why**: Tests that ObservableList replacement works correctly at multiple levels in sequence. Each replacement should fire callbacks with the correct node path and counts, demonstrating hierarchical independence - changes at one level don't interfere with operations at other levels.

## STEP 15: Test Parent Text Property Changes Preserve Children

**What to change**: Add test case `ParentTextChangePreservesChildren` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ParentTextChangePreservesChildren")
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
	callbackLog.Clear();
	
	// Change child1's text property (not children)
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	child1Item->name = L"Root.Child1.Modified";
	child1Item->UpdateProperty(BindableItem::Prop_name());
	
	// No callbacks expected for children property (text-only change)
	const wchar_t* expected[] = {
	};
	AssertCallbacks(callbackLog, expected);
	
	// Verify text changed but children preserved
	TEST_ASSERT(provider->GetTextValue(child1.Obj()) == L"Root.Child1.Modified");
	TEST_ASSERT(child1->GetChildCount() == 2);
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(0).Obj()) == L"Root.Child1.GrandChild1");
	TEST_ASSERT(provider->GetTextValue(child1->GetChild(1).Obj()) == L"Root.Child1.GrandChild2");
}
```

**Why**: Verifies that updating a parent's text property (via `UpdateProperty` on the text property, not children property) doesn't trigger children modification callbacks. This ensures the provider distinguishes between different property types and doesn't unnecessarily re-process children when only non-structural properties change.

## STEP 16: Test Parent Property Change Callback Verification

**What to change**: Add test case `ParentPropertyChangeCallbackVerification` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"ParentPropertyChangeCallbackVerification")
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
	callbackLog.Clear();
	
	// Change root's text property
	rootItem->name = L"ModifiedRoot";
	rootItem->UpdateProperty(BindableItem::Prop_name());
	
	// No children modification callbacks expected
	TEST_ASSERT(callbackLog.Count() == 0);
	TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"ModifiedRoot");
	TEST_ASSERT(rootNode->GetChildCount() == 2);
	
	auto child1 = rootNode->GetChild(0);
	child1->GetChildCount();
	callbackLog.Clear();
	
	// Change child1's text and verify it doesn't affect its grandchildren
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	child1Item->name = L"ModifiedChild";
	child1Item->UpdateProperty(BindableItem::Prop_name());
	
	TEST_ASSERT(callbackLog.Count() == 0);
	TEST_ASSERT(provider->GetTextValue(child1.Obj()) == L"ModifiedChild");
	TEST_ASSERT(child1->GetChildCount() == 2);
}
```

**Why**: Explicitly verifies that non-children property changes at different levels don't trigger children modification callbacks. This tests the provider's ability to discriminate between structural (children) and non-structural (text) property updates across the hierarchy.

## STEP 17: Test UpdateBindingProperties Affects All Hierarchy Levels

**What to change**: Add test case `UpdateBindingPropertiesAffectsAllLevels` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"UpdateBindingPropertiesAffectsAllLevels")
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
	auto child2 = rootNode->GetChild(1);
	child2->GetChildCount();
	callbackLog.Clear();
	
	// Call UpdateBindingProperties
	provider->UpdateBindingProperties(true);
	
	// Expect callbacks for root and all prepared children showing maintained counts
	const wchar_t* expected[] = {
		L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
		L"Root.Child1->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
		L"Root.Child2->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
		L"Root.Child2->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
		L"Root.Child1->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
		L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)"
	};
	AssertCallbacks(callbackLog, expected);
}
```

**Why**: Tests that `UpdateBindingProperties` triggers callbacks at all prepared hierarchy levels. The callbacks should show maintained child counts (`count=2, newCount=2`) at each level, demonstrating that the update propagates through the tree and re-prepares all nodes correctly. The nested callback ordering (Before callbacks top-down, After callbacks bottom-up) verifies proper hierarchical update coordination.

## STEP 18: Test UpdateBindingProperties with Complex Deep Hierarchy

**What to change**: Add test case `UpdateBindingPropertiesWithComplexHierarchy` in the `HierarchicalBindingScenarios` category.

**Code to add**:

```cpp
TEST_CASE(L"UpdateBindingPropertiesWithComplexHierarchy")
{
	List<WString> callbackLog;
	MockNodeProviderCallback nodeCallback(callbackLog);
	
	auto provider = Ptr(new TreeViewItemBindableRootProvider());
	InitProvider(provider);
	
	// Create 4-level tree
	auto rootItem = CreateMultiLevelTree(L"Root", 2, 1);
	auto child1Item = UnboxValue<Ptr<BindableItem>>(rootItem->children[0]);
	auto grandchild1Item = UnboxValue<Ptr<BindableItem>>(child1Item->children[0]);
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
	
	// UpdateBindingProperties should affect all 4 levels
	provider->UpdateBindingProperties(true);
	
	// Verify callbacks fired at all levels with maintained counts
	const wchar_t* expected[] = {
		L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
		L"Root.Child1->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
		L"Root.Child1.GrandChild1->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
		L"Root.Child1.GrandChild1->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
		L"Root.Child1->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
		L"Root.Child2->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
		L"Root.Child2->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
		L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)"
	};
	AssertCallbacks(callbackLog, expected);
	
	// Verify all nodes still accessible after update
	TEST_ASSERT(rootNode->GetChildCount() == 2);
	TEST_ASSERT(child1->GetChildCount() == 1);
	TEST_ASSERT(grandchild1->GetChildCount() == 1);
}
```

**Why**: Tests `UpdateBindingProperties` with a 4-level deep hierarchy to verify the update mechanism handles arbitrary depth correctly. The callback ordering should follow the proper depth-first traversal pattern, and all nodes should remain accessible after the update, demonstrating that the re-preparation process preserves the entire tree structure.

# TEST PLAN

## Overview

The test plan verifies that hierarchical binding scenarios work correctly across multiple tree levels. All tests account for eager child preparation behavior where callbacks show actual prepared child counts, not zero.

## Test Categories

### Category 1: Multi-Level ObservableList Modifications (5 tests)

1. **ObservableListAddAtGrandchildLevel**
   - **Purpose**: Verify adding items to level 2 (grandchildren) triggers correct callbacks
   - **Setup**: 3-level tree (Root → 2 Children → 1 Grandchild each)
   - **Action**: Add grandchild to Child1's children
   - **Verify**: Callback fires with Child1 as context, shows `start=1, count=0, newCount=1`

2. **ObservableListRemoveAtGrandchildLevel**
   - **Purpose**: Verify removing items from level 2 triggers correct callbacks
   - **Setup**: 3-level tree (Root → 2 Children → 2 Grandchildren each)
   - **Action**: Remove first grandchild from Child1
   - **Verify**: Callback fires with Child1 as context, shows `start=0, count=1, newCount=0`, remaining child is GrandChild2

3. **ObservableListClearAtGrandchildLevel**
   - **Purpose**: Verify clearing all grandchildren triggers correct callbacks
   - **Setup**: 3-level tree (Root → 2 Children → 3 Grandchildren each)
   - **Action**: Clear Child1's children
   - **Verify**: Callback shows `count=3` (all prepared children removed), Child1 has 0 children after

4. **ObservableListAddAtGreatGrandchildLevel**
   - **Purpose**: Verify operations at level 3 (great-grandchildren) work correctly
   - **Setup**: 4-level tree (Root → Child → GrandChild → initially no GreatGrandChildren)
   - **Action**: Add great-grandchild
   - **Verify**: Callback fires with GrandChild1 as context, great-grandchild is accessible

5. **ObservableListRemoveRangeAtMultipleLevels**
   - **Purpose**: Verify RemoveRange works at level 2
   - **Setup**: 3-level tree (Root → 3 Children → 4 Grandchildren each)
   - **Action**: RemoveRange(1, 2) on Child2's grandchildren
   - **Verify**: Callback shows `start=1, count=2, newCount=0`, remaining children are GrandChild1 and GrandChild4

### Category 2: Nested Callback Propagation (1 test)

6. **NestedCallbackNodePathVerification**
   - **Purpose**: Explicitly verify callback node paths at different levels
   - **Setup**: 3-level tree with 2 children, 2 grandchildren each
   - **Action**: Add to Child1, then remove from Child2
   - **Verify**: First callbacks show "Root.Child1->", second show "Root.Child2->"

### Category 3: Expansion State Coordination (4 tests)

7. **AddChildrenToExpandedNode**
   - **Purpose**: Verify adding children to expanded node maintains expansion
   - **Setup**: 3-level tree, expand Child1
   - **Action**: Add grandchild to Child1
   - **Verify**: Child1 remains expanded, new grandchild is visible

8. **RemoveChildrenFromExpandedNode**
   - **Purpose**: Verify removing children from expanded node maintains expansion
   - **Setup**: 3-level tree with 3 grandchildren, expand Child1
   - **Action**: Remove middle grandchild from Child1
   - **Verify**: Child1 remains expanded, correct children remain (GrandChild1, GrandChild3)

9. **ClearChildrenOfExpandedNode**
   - **Purpose**: Verify clearing all children from expanded node maintains expansion
   - **Setup**: 3-level tree with 3 grandchildren, expand Child1
   - **Action**: Clear Child1's children
   - **Verify**: Child1 remains expanded despite having 0 children

10. **ExpansionStatePersistsAcrossDynamicChanges**
    - **Purpose**: Verify expansion survives multiple sequential operations
    - **Setup**: 3-level tree, expand Child1
    - **Action**: Add grandchild, remove grandchild, add another grandchild
    - **Verify**: Child1 remains expanded through all operations, final children are correct

### Category 4: Nested ObservableList Replacement (2 tests)

11. **ReplaceChildrenAtLevel2**
    - **Purpose**: Verify replacing entire children ObservableList at level 2
    - **Setup**: 3-level tree (2 grandchildren per child)
    - **Action**: Replace Child1's children with new ObservableList of 3 items
    - **Verify**: Callback shows `count=2, newCount=3`, new children are accessible

12. **ReplaceChildrenAtMultipleLevels**
    - **Purpose**: Verify sequential replacement at different levels
    - **Setup**: 3-level tree
    - **Action**: Replace root's children (2→1), then replace new child's children (0→2)
    - **Verify**: Each replacement fires correct callbacks with correct node context

### Category 5: Parent Property Changes (2 tests)

13. **ParentTextChangePreservesChildren**
    - **Purpose**: Verify non-children property changes don't affect children
    - **Setup**: 3-level tree
    - **Action**: Update Child1's text property (not children)
    - **Verify**: No children modification callbacks, text changed, children preserved

14. **ParentPropertyChangeCallbackVerification**
    - **Purpose**: Verify text property changes at multiple levels don't trigger children callbacks
    - **Setup**: 3-level tree
    - **Action**: Update text at root, then at Child1
    - **Verify**: No callbacks in both cases, structure preserved

### Category 6: Multi-Level UpdateBindingProperties (2 tests)

15. **UpdateBindingPropertiesAffectsAllLevels**
    - **Purpose**: Verify UpdateBindingProperties propagates through hierarchy
    - **Setup**: 3-level tree, prepare root and both children with their grandchildren
    - **Action**: Call UpdateBindingProperties(true)
    - **Verify**: Callbacks fire for root, Child1, Child2 with maintained counts, proper ordering

16. **UpdateBindingPropertiesWithComplexHierarchy**
    - **Purpose**: Verify UpdateBindingProperties works with deep hierarchies
    - **Setup**: 4-level tree (Root → 2 Children → 1 GrandChild each → 1 GreatGrandChild on first)
    - **Action**: Call UpdateBindingProperties(true)
    - **Verify**: Callbacks fire at all 4 levels, all nodes remain accessible

## Test Coverage Analysis

**What's covered**:
- Multi-level ObservableList operations (Add, Remove, RemoveRange, Clear) at levels 2 and 3
- Callback node path verification at different levels
- Expansion state coordination with dynamic structure changes
- ObservableList replacement at nested levels
- Property change isolation (text vs children)
- UpdateBindingProperties hierarchical propagation
- Edge cases (clearing expanded nodes, sequential operations)

**What's not covered** (as per task requirements):
- Duplicate bindable items (Value objects cannot be reliably detected - lesson from Task No.3)
- SetItemSource behavior (already covered in Task No.6)

## Quality Metrics

- **Total new test cases**: 16
- **Test isolation**: Each test creates its own callbackLog and callback objects
- **Callback log management**: Clear logs after setup (except when testing setup itself)
- **Eager preparation awareness**: All expectations account for actual prepared child counts
- **Hierarchy depth**: Tests cover 3 and 4 level hierarchies
- **Edge cases**: Expansion with no children, sequential operations, property change isolation

## Existing Test Coverage Review

Based on analysis of existing tests in `TestItemProviders_TreeViewItemBindableRootProvider.cpp` (664 lines):
- **AttachCallback**: Basic callback attachment (1 test)
- **SimpleItemAddition**: Root-level child addition (1 test)
- **PropertyBindingSetup**: Single-level binding (2 tests)
- **SetItemSourceOperations**: SetItemSource with various scenarios (3 tests)
- **ObservableListIntegration**: Root-level list operations only (4 tests)
- **ExpansionCollapseOperations**: Basic expansion behavior (4 tests)
- **NodeLifecycleAndRelationships**: Parent-child relationships (multiple tests)
- **UpdateBindingProperties**: Root-level update (multiple tests)
- **InterfaceMethods**: RequestView, GetTextValue (2 tests)

**Assessment**: Existing tests focus on single-level (root and immediate children) operations. No tests modify ObservableList at level 2+, no tests verify callback node paths at depth, no tests coordinate expansion with nested dynamic changes. The 16 new tests address these gaps comprehensively.

## Test Execution Strategy

1. Run tests incrementally as each test case is added
2. Verify each test in isolation before adding the next
3. If a test fails:
   - Check callback log output for unexpected sequences
   - Verify node paths in callback messages
   - Confirm child counts match eager preparation expectations
   - Check tree structure navigation after operations
4. Use existing test infrastructure:
   - `AssertCallbacks()` for callback verification
   - `TEST_ASSERT()` for structure verification
   - `MockNodeProviderCallback` for callback logging

# !!!FINISHED!!!
