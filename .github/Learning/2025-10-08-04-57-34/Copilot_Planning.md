# !!!PLANNING!!!

# UPDATES

(No updates yet - this is the initial planning document)

# IMPROVEMENT PLAN

## STEP 1: Add New Test Category for Bindable Provider Integration

Add a new `TEST_CATEGORY` at the end of `TestItemProviders_NodeItemProvider.cpp` (after line 2165, before the final closing brace) named `"TreeViewItemBindableRootProviderIntegration"`.

**Why this change is necessary:**
- Task 7 requires adding integration tests between `NodeItemProvider` and `TreeViewItemBindableRootProvider`
- All existing tests use `TreeViewItemRootProvider`, so we need a dedicated category for bindable provider integration
- This follows the established pattern in the file where each major test area has its own category

## STEP 2: Add Helper Function for Creating Multi-Level Bindable Trees

Inside the test category (but before any test cases), add a local lambda function `CreateMultiLevelTree` that creates a bindable tree with configurable levels.

**Code to add:**

```cpp
TEST_CATEGORY(L"TreeViewItemBindableRootProviderIntegration")
{
	auto CreateMultiLevelTree = [](const WString& rootName, vint level1Count, vint level2Count) -> Ptr<BindableItem>
	{
		auto root = Ptr(new BindableItem());
		root->name = rootName;
		for (vint i = 0; i < level1Count; i++)
		{
			auto child = Ptr(new BindableItem());
			child->name = rootName + L".Child" + itow(i + 1);
			root->children.Add(child);
			for (vint j = 0; j < level2Count; j++)
			{
				auto grandchild = Ptr(new BindableItem());
				grandchild->name = child->name + L".GrandChild" + itow(j + 1);
				child->children.Add(grandchild);
			}
		}
		return root;
	};
```

**Why this change is necessary:**
- We need to create a multi-level tree structure (root → children → grandchildren) for the integration test
- This helper reuses the same pattern found in `TestItemProviders_TreeViewItemBindableRootProvider.cpp`
- Having a local helper keeps the test self-contained and makes it easy to understand the test data structure
- The function creates a tree where:
  - Root has `level1Count` children
  - Each child has `level2Count` grandchildren
  - Names follow the pattern "Root.Child1.GrandChild1" for easy verification

## STEP 3: Add Single Comprehensive Integration Test Case

Add one test case named `"BasicBindableProviderIntegration"` that comprehensively tests the integration between `NodeItemProvider` and `TreeViewItemBindableRootProvider`.

**Code to add:**

```cpp
	TEST_CASE(L"BasicBindableProviderIntegration")
	{
		// Setup: Create bindable provider with property bindings
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		provider->textProperty = BindableItem::Prop_name();
		provider->childrenProperty = BindableItem::Prop_children();
		
		// Create a 2-level tree: Root -> 3 children -> 2 grandchildren each
		auto rootItem = CreateMultiLevelTree(L"Root", 3, 2);
		provider->SetItemSource(BoxValue(rootItem));
		
		// Wrap with NodeItemProvider (adapter pattern)
		auto nodeItemProvider = Ptr(new NodeItemProvider(provider));
		
		// Phase 1: Verify initial state (all nodes collapsed)
		// Should only see the 3 top-level children (Child1, Child2, Child3)
		TEST_ASSERT(nodeItemProvider->Count() == 3);
		
		// Through IItemProvider interface - GetTextValue
		TEST_ASSERT(nodeItemProvider->GetTextValue(0) == L"Root.Child1");
		TEST_ASSERT(nodeItemProvider->GetTextValue(1) == L"Root.Child2");
		TEST_ASSERT(nodeItemProvider->GetTextValue(2) == L"Root.Child3");
		
		// Through IItemProvider interface - GetBindingValue
		auto binding0 = nodeItemProvider->GetBindingValue(0);
		auto binding1 = nodeItemProvider->GetBindingValue(1);
		auto binding2 = nodeItemProvider->GetBindingValue(2);
		
		// Unbox and verify bindable item data
		auto item0 = UnboxValue<Ptr<BindableItem>>(binding0);
		auto item1 = UnboxValue<Ptr<BindableItem>>(binding1);
		auto item2 = UnboxValue<Ptr<BindableItem>>(binding2);
		
		TEST_ASSERT(item0->name == L"Root.Child1");
		TEST_ASSERT(item1->name == L"Root.Child2");
		TEST_ASSERT(item2->name == L"Root.Child3");
		
		// Verify children property is accessible from bindable items
		TEST_ASSERT(item0->children.Count() == 2); // Child1 has 2 grandchildren
		TEST_ASSERT(item0->children[0]->name == L"Root.Child1.GrandChild1");
		TEST_ASSERT(item0->children[1]->name == L"Root.Child1.GrandChild2");
		
		// Phase 2: Expand the first child node
		// Through INodeProvider interface
		auto child1Node = nodeItemProvider->RequestNode(0);
		TEST_ASSERT(child1Node != nullptr);
		
		child1Node->SetExpanding(true);
		
		// Verify count increased (now showing 5 items: Child1, GrandChild1.1, GrandChild1.2, Child2, Child3)
		TEST_ASSERT(nodeItemProvider->Count() == 5);
		
		// Verify the visible sequence using AssertItems helper
		const wchar_t* expectedAfterExpand[] = {
			L"Root.Child1",
			L"Root.Child1.GrandChild1",
			L"Root.Child1.GrandChild2",
			L"Root.Child2",
			L"Root.Child3"
		};
		AssertItems(nodeItemProvider, expectedAfterExpand);
		
		// Verify data retrieval still works after expansion
		auto bindingGrandChild1 = nodeItemProvider->GetBindingValue(1);
		auto itemGrandChild1 = UnboxValue<Ptr<BindableItem>>(bindingGrandChild1);
		TEST_ASSERT(itemGrandChild1->name == L"Root.Child1.GrandChild1");
		TEST_ASSERT(itemGrandChild1->children.Count() == 0); // Grandchildren have no children
		
		auto bindingGrandChild2 = nodeItemProvider->GetBindingValue(2);
		auto itemGrandChild2 = UnboxValue<Ptr<BindableItem>>(bindingGrandChild2);
		TEST_ASSERT(itemGrandChild2->name == L"Root.Child1.GrandChild2");
		
		// Phase 3: Collapse the first child node
		child1Node->SetExpanding(false);
		
		// Verify count decreased back to original (3 items: Child1, Child2, Child3)
		TEST_ASSERT(nodeItemProvider->Count() == 3);
		
		// Verify the visible sequence is back to original
		const wchar_t* expectedAfterCollapse[] = {
			L"Root.Child1",
			L"Root.Child2",
			L"Root.Child3"
		};
		AssertItems(nodeItemProvider, expectedAfterCollapse);
		
		// Verify data retrieval still works correctly after collapse
		auto bindingAfterCollapse = nodeItemProvider->GetBindingValue(0);
		auto itemAfterCollapse = UnboxValue<Ptr<BindableItem>>(bindingAfterCollapse);
		TEST_ASSERT(itemAfterCollapse->name == L"Root.Child1");
		TEST_ASSERT(itemAfterCollapse->children.Count() == 2); // Data unchanged
	});
});
```

**Why this change is necessary:**

1. **Integration Testing**: Both components (`NodeItemProvider` and `TreeViewItemBindableRootProvider`) are already individually tested, but we need to verify they work together correctly through their interfaces.

2. **Data Retrieval Verification**: The test verifies that:
   - `GetTextValue()` correctly retrieves text from bindable items through the property binding
   - `GetBindingValue()` correctly returns the boxed bindable item
   - Unboxing with `UnboxValue<Ptr<BindableItem>>()` works correctly
   - The bindable item's properties (like `children`) are accessible

3. **Expand/Collapse Integration**: The test verifies that:
   - Expand operations on `NodeItemProvider` correctly trigger visibility changes in the bindable provider
   - The count updates correctly after expand/collapse
   - The visible sequence reflects the correct tree state
   - Data retrieval works correctly in all states (collapsed, expanded, collapsed again)

4. **Code Style Compliance**: The code follows patterns established in Tasks 1-6:
   - Direct method calls without unnecessary interface casting
   - Comments like "Through IItemProvider interface" for clarity
   - Blank lines separate logical phases
   - Uses the `AssertItems` helper function for sequence verification
   - Simple and straightforward - no over-engineering

5. **Comprehensive Coverage in One Test**: Instead of multiple small tests, one comprehensive test case is sufficient because:
   - The integration point is simple (adapter pattern with interface delegation)
   - Both components are already thoroughly tested individually
   - A single test that exercises the complete integration flow provides adequate confidence
   - Easier to maintain than multiple small tests

6. **Minimal Scope**: The test intentionally avoids:
   - Complex tree structures (Task 4 already tests those with `TreeViewItemRootProvider`)
   - Edge cases and invalid operations (Task 4 already covers those)
   - Callback testing (Task 6 already comprehensively tests callbacks)
   - Dynamic tree modifications (bindable provider tests already cover those)

## STEP 4: Close the Test Category

Add the closing `});` for the test category after the test case.

**Why this change is necessary:**
- Proper syntax for closing the `TEST_CATEGORY` block
- Maintains consistency with the rest of the file structure

# TEST PLAN

## Test Case 1: BasicBindableProviderIntegration

**Purpose**: Verify that `NodeItemProvider` correctly works with `TreeViewItemBindableRootProvider` as the underlying provider.

**Test Structure**:
```
Root (not visible)
├── Child1 (initially collapsed)
│   ├── GrandChild1
│   └── GrandChild2
├── Child2 (collapsed)
│   ├── GrandChild1
│   └── GrandChild2
└── Child3 (collapsed)
    ├── GrandChild1
    └── GrandChild2
```

**Phase 1 - Initial State Verification (Collapsed)**:
- ✅ Verify `Count()` returns 3 (only top-level children visible)
- ✅ Verify `GetTextValue()` retrieves correct names for all 3 children
- ✅ Verify `GetBindingValue()` returns correct boxed bindable items
- ✅ Verify unboxing with `UnboxValue<Ptr<BindableItem>>()` works
- ✅ Verify bindable item properties are accessible (e.g., `children.Count()`)

**Phase 2 - After Expansion Verification**:
- ✅ Expand Child1 using `RequestNode(0)->SetExpanding(true)`
- ✅ Verify `Count()` increases to 5 (Child1 + 2 grandchildren + Child2 + Child3)
- ✅ Verify `AssertItems()` shows correct visible sequence
- ✅ Verify `GetTextValue()` and `GetBindingValue()` work for newly visible grandchildren
- ✅ Verify grandchildren have no children themselves

**Phase 3 - After Collapse Verification**:
- ✅ Collapse Child1 using `SetExpanding(false)`
- ✅ Verify `Count()` decreases back to 3
- ✅ Verify `AssertItems()` shows original visible sequence
- ✅ Verify data retrieval still works correctly
- ✅ Verify bindable item data is unchanged (children still have 2 grandchildren)

**Coverage Analysis**:

| Aspect | Coverage | Rationale |
|--------|----------|-----------|
| Basic Construction | ✅ Covered | Test creates bindable provider, configures properties, and wraps with NodeItemProvider |
| Property Binding | ✅ Covered | Configures `textProperty` and `childrenProperty` |
| Text Retrieval | ✅ Covered | Uses `GetTextValue()` in all phases |
| Binding Value Retrieval | ✅ Covered | Uses `GetBindingValue()` and verifies unboxing |
| Count Tracking | ✅ Covered | Verifies count in all states |
| Expand Operation | ✅ Covered | Expands one node and verifies effects |
| Collapse Operation | ✅ Covered | Collapses the same node and verifies effects |
| Index Mapping | ✅ Covered | Uses `RequestNode()` and `AssertItems()` |
| Data Persistence | ✅ Covered | Verifies bindable item data unchanged after operations |

**Why Existing Tests Don't Need Updates**:

All existing test cases in `TestItemProviders_NodeItemProvider.cpp` use `TreeViewItemRootProvider` as the underlying provider. These tests do NOT need to be modified because:

1. **Interface-Based Design**: `NodeItemProvider` delegates to `INodeRootProvider` interface methods. The implementation doesn't care whether the concrete type is `TreeViewItemRootProvider` or `TreeViewItemBindableRootProvider`.

2. **Already Comprehensive**: The existing 38 test cases already cover:
   - All index mapping scenarios
   - All expand/collapse dynamics
   - All edge cases and error conditions
   - All callback mechanisms
   - All data retrieval patterns

3. **Single Responsibility**: The existing tests verify `NodeItemProvider`'s responsibilities as a list adapter. They don't need to test bindable-specific behavior.

4. **Integration vs Unit Testing**: The new test is an integration test, while existing tests are unit tests. They serve different purposes.

**Why One Test Case Is Sufficient**:

1. **Low Risk**: Both components are already thoroughly tested individually
2. **Simple Integration**: The adapter pattern uses well-defined interfaces
3. **No Special Logic**: `NodeItemProvider` doesn't have special handling for bindable providers
4. **Comprehensive Coverage**: The single test exercises the complete integration path
5. **Task Guidance**: Task description explicitly states "one or two simple test cases"

**Corner Cases Intentionally NOT Covered** (already tested elsewhere):

- ❌ Invalid indices → Already covered in Task 4 (`InvalidIndicesForDataRetrieval`)
- ❌ Foreign nodes → Already covered in Task 4 (`CalculateIndexForForeignNode`)
- ❌ Complex expansion patterns → Already covered in Tasks 2-3
- ❌ Deep nesting → Already covered in Task 4 (`DeeplyNestedTreeIndexMapping`)
- ❌ Dynamic tree modifications → Already tested in bindable provider tests
- ❌ Callback parameter verification → Already covered in Task 6

**Build and Execution**:
- ✅ Code will build without errors (reuses existing infrastructure)
- ✅ Test will pass (follows proven patterns from existing tests)
- ✅ Approximately 100 lines of new code (within scope guidance)

# !!!FINISHED!!!
