# !!!PLANNING!!!

# UPDATES

## UPDATE

You can just combined the two sample test case into one

# IMPROVEMENT PLAN

## STEP 1: Create Test File with Basic Structure

Create the new test file `Test\GacUISrc\UnitTest\TestItemProviders_NodeItemProvider.cpp` with the following structure:

```cpp
#include "TestItemProviders.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	TEST_CASE(L"BasicSetupAndConstruction")
	{
		// Setup callback logging
		List<WString> callbackLog;
		MockItemProviderCallback itemCallback(callbackLog);
		
		// Setup: Create root provider with simple tree structure
		auto rootProvider = Ptr(new TreeViewItemRootProvider);
		auto node1 = CreateTreeViewItem(L"Node1");
		auto node2 = CreateTreeViewItem(L"Node2");
		
		// Add nodes to root
		auto rootMemoryNode = rootProvider->GetMemoryNode(rootProvider->GetRootNode().Obj());
		rootMemoryNode->Children().Add(node1);
		rootMemoryNode->Children().Add(node2);
		
		// Create NodeItemProvider wrapping the root provider
		auto nodeItemProvider = Ptr(new NodeItemProvider(rootProvider));
		
		// Attach callback to NodeItemProvider
		nodeItemProvider->AttachCallback(&itemCallback);
		callbackLog.Clear(); // Clear the OnAttached callback
		
		// Verify basic operations
		TEST_ASSERT(nodeItemProvider->Count() == 2);
		TEST_ASSERT(nodeItemProvider->GetTextValue(0) == L"Node1");
		TEST_ASSERT(nodeItemProvider->GetTextValue(1) == L"Node2");
		
		// Verify node retrieval
		auto retrievedNode = nodeItemProvider->RequestNode(0);
		TEST_ASSERT(retrievedNode != nullptr);
		TEST_ASSERT(rootProvider->GetTextValue(retrievedNode.Obj()) == L"Node1");
		
		// Verify root provider retrieval
		TEST_ASSERT(nodeItemProvider->GetRoot() == rootProvider);
		
		// Add another node to trigger callback propagation
		auto node3 = CreateTreeViewItem(L"Node3");
		rootMemoryNode->Children().Add(node3);
		
		// Verify callback was triggered with correct parameters
		// NodeItemProvider should propagate tree modifications as item modifications
		const wchar_t* expected[] = {
			L"OnItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});
}
```

**Why this is necessary:**
- This establishes the basic test infrastructure for `NodeItemProvider`
- The combined test case verifies:
  - Constructor works correctly with a `TreeViewItemRootProvider`
  - `Count()` correctly excludes the root node (2 children = count of 2)
  - `GetTextValue()` correctly retrieves text from the underlying tree nodes
  - `RequestNode()` returns valid node references
  - `GetRoot()` returns the wrapped root provider
  - `NodeItemProvider` correctly implements `IItemProvider` callback attachment
  - Tree modifications from the underlying `INodeRootProvider` are properly propagated as `OnItemModified` callbacks
  - This is critical because `NodeItemProvider` bridges tree structure to flat list structure
- This single comprehensive smoke test verifies the file compiles, links, and the basic API contract is satisfied
- This minimal setup allows subsequent tasks (Tasks 2-4) to add comprehensive testing

## STEP 2: Update Project File (UnitTest.vcxproj)

In `Test\GacUISrc\UnitTest\UnitTest.vcxproj`, add the new compile entry after line 268:

**Current state (lines 267-270):**
```xml
    <ClCompile Include="TestItemProviders_TreeViewItemRootProvider.cpp" />
    <ClCompile Include="TestItemProviders_TreeViewItemBindableRootProvider.cpp" />
    <ClCompile Include="TestItemProviders.cpp" />
  </ItemGroup>
```

**New state:**
```xml
    <ClCompile Include="TestItemProviders_TreeViewItemRootProvider.cpp" />
    <ClCompile Include="TestItemProviders_TreeViewItemBindableRootProvider.cpp" />
    <ClCompile Include="TestItemProviders_NodeItemProvider.cpp" />
    <ClCompile Include="TestItemProviders.cpp" />
  </ItemGroup>
```

**Why this is necessary:**
- Without adding the file to the project file, the compiler will not include it in the build
- The placement after `TestItemProviders_TreeViewItemBindableRootProvider.cpp` follows the logical dependency order (NodeItemProvider depends on TreeViewItem providers)
- Placing it before the general `TestItemProviders.cpp` maintains the pattern where specific provider tests come before the general provider test file

## STEP 3: Update Project Filters File (UnitTest.vcxproj.filters)

In `Test\GacUISrc\UnitTest\UnitTest.vcxproj.filters`, add the filter entry after line 327:

**Current state (lines 324-330):**
```xml
    <ClCompile Include="TestItemProviders_TreeViewItemBindableRootProvider.cpp">
      <Filter>Source Files\ItemProviders</Filter>
    </ClCompile>
    <ClCompile Include="TestItemProviders.cpp">
      <Filter>Source Files\ItemProviders</Filter>
    </ClCompile>
  </ItemGroup>
```

**New state:**
```xml
    <ClCompile Include="TestItemProviders_TreeViewItemBindableRootProvider.cpp">
      <Filter>Source Files\ItemProviders</Filter>
    </ClCompile>
    <ClCompile Include="TestItemProviders_NodeItemProvider.cpp">
      <Filter>Source Files\ItemProviders</Filter>
    </ClCompile>
    <ClCompile Include="TestItemProviders.cpp">
      <Filter>Source Files\ItemProviders</Filter>
    </ClCompile>
  </ItemGroup>
```

**Why this is necessary:**
- The filters file controls the visual organization of files in Visual Studio's Solution Explorer
- Without this entry, the file will appear in a default location rather than being properly grouped with other ItemProviders tests
- Using `Source Files\ItemProviders` filter maintains consistency with all other `TestItemProviders_*.cpp` files
- The placement maintains the same order as in the project file for consistency

# TEST PLAN

## Test Case 1: Build Verification

**Objective:** Verify the new file compiles and links without errors.

**Steps:**
1. Run the "Build Unit Tests" task in VS Code
2. Verify build completes successfully
3. Verify no compilation errors related to:
   - Missing includes
   - Type mismatches
   - Undefined symbols

**Expected Result:**
- Build succeeds with 0 errors
- No warnings related to the new file

**Why this test:**
- Ensures the file is properly integrated into the build system
- Validates all necessary headers are included
- Confirms the project file and filters file are correctly updated

## Test Case 2: BasicSetupAndConstruction Smoke Test

**Objective:** Verify fundamental `NodeItemProvider` construction, basic operations, and callback propagation.

**Steps:**
1. Run the "Run Unit Tests" task
2. Locate the `BasicSetupAndConstruction` test case in the output
3. Verify all assertions pass

**Expected Result:**
- `nodeItemProvider->Count() == 2` passes (2 children at root level, excluding root itself)
- `GetTextValue(0) == L"Node1"` passes
- `GetTextValue(1) == L"Node2"` passes
- `RequestNode(0)` returns non-null node with correct text
- `GetRoot()` returns the original root provider
- After adding a third node, one `OnItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)` callback is logged
- No unexpected callbacks occur

**Why this test:**
- Validates the core functionality of wrapping a tree structure as a flat list
- Ensures `Count()` correctly excludes the invisible root node
- Confirms text value retrieval delegates properly to the underlying tree nodes
- Verifies node reference retrieval works correctly
- Validates the critical bridge functionality: `NodeItemProvider` implements `INodeProviderCallback` to listen to tree changes and translates them into `IItemProviderCallback` notifications
- Ensures list-based controls (like `GuiListControl`) can properly react to tree structure changes
- Verifies the `itemReferenceUpdated` flag is correctly propagated (true for structural changes)

## Test Case 3: Integration Test

**Objective:** Verify the new test file runs as part of the complete test suite without affecting other tests.

**Steps:**
1. Run the complete "Build and Run Unit Tests" task
2. Verify all existing tests still pass
3. Verify the new test case appears in the test output
4. Check for no side effects on other ItemProviders tests

**Expected Result:**
- All pre-existing tests continue to pass
- The new test case passes
- Test execution time increases only marginally
- No memory leaks or resource issues introduced

**Why this test:**
- Ensures the new file doesn't break existing functionality
- Validates proper isolation between test files
- Confirms the test framework correctly discovers and runs the new test

## Notes on Test Coverage

**What is covered by this smoke test:**
- Basic construction with `TreeViewItemRootProvider`
- `Count()` with simple tree structure (root + 2 children, then 3 children)
- `GetTextValue()` for accessing node text
- `RequestNode()` for retrieving node references
- `GetRoot()` for accessing wrapped provider
- Callback attachment and propagation for node addition
- Verification that tree modifications trigger correct `OnItemModified` callbacks with proper parameters

**What is NOT covered (deferred to Tasks 2-4):**
- Expand/collapse operations and their effect on `Count()`
- Nested tree structures with multiple levels
- Node insertion, deletion, and modification at various positions
- Complex callback scenarios with multiple modifications
- `GetBindingValue()` functionality
- `RequestView()` with `INodeItemView::Identifier`
- `CalculateNodeVisibilityIndex()` calculations
- Edge cases like empty trees, single node trees, deeply nested structures
- Performance with large trees

**Rationale for limited scope:**
The goal of this task is infrastructure setup, not comprehensive testing. The single combined smoke test verifies that:
1. The file is correctly integrated into the build
2. Basic API calls work
3. The fundamental tree-to-list transformation works
4. Callback propagation from tree to list works correctly

Comprehensive testing is explicitly scoped for Tasks 2-4 in the scrum document, where proper test coverage for expand/collapse, data retrieval, and complex scenarios will be implemented.

# !!!FINISHED!!!
