# !!!PLANNING!!!

# UPDATES

# IMPROVEMENT PLAN

## STEP 1: Add TEST_CATEGORY for AddInlineObjectRun

Add a new `TEST_CATEGORY(L"AddInlineObjectRun")` block to `TestRemote_DocumentRunManagement.cpp` after the existing `AddTextRun` category. This category will contain 11 comprehensive test cases covering all success scenarios, failure scenarios, and edge cases.

**Location**: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

**Code to add** (after the `AddTextRun` category closing `});`):

```cpp
TEST_CATEGORY(L"AddInlineObjectRun")
{
	// Success cases
	TEST_CASE(L"Add to empty map")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		
		bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
		
		TEST_ASSERT(result == true);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
		
		AssertMap(inlineMap, expectedMap);
	});

	TEST_CASE(L"Add non-overlapping objects with gap")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		auto prop2 = CreateInlineProp(200);
		
		AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
		bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40}, prop2);
		
		TEST_ASSERT(result == true);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
		expectedMap.Add({.caretBegin = 30, .caretEnd = 40}, prop2);
		
		AssertMap(inlineMap, expectedMap);
	});

	TEST_CASE(L"Adjacent objects - boundary test")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		auto prop2 = CreateInlineProp(200);
		
		AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
		bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
		
		TEST_ASSERT(result == true);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
		expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop2);
		
		AssertMap(inlineMap, expectedMap);
	});

	TEST_CASE(L"Multiple non-overlapping objects")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		auto prop2 = CreateInlineProp(200);
		auto prop3 = CreateInlineProp(300);
		
		AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
		AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40}, prop2);
		bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 50, .caretEnd = 60}, prop3);
		
		TEST_ASSERT(result == true);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
		expectedMap.Add({.caretBegin = 30, .caretEnd = 40}, prop2);
		expectedMap.Add({.caretBegin = 50, .caretEnd = 60}, prop3);
		
		AssertMap(inlineMap, expectedMap);
	});

	// Failure cases
	TEST_CASE(L"Complete overlap - exact match")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		auto prop2 = CreateInlineProp(200);
		
		AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
		bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
		
		TEST_ASSERT(result == false);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
		
		AssertMap(inlineMap, expectedMap);
	});

	TEST_CASE(L"Partial overlap from left")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		auto prop2 = CreateInlineProp(200);
		
		AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, prop1);
		bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25}, prop2);
		
		TEST_ASSERT(result == false);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop1);
		
		AssertMap(inlineMap, expectedMap);
	});

	TEST_CASE(L"Partial overlap from right")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		auto prop2 = CreateInlineProp(200);
		
		AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
		bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25}, prop2);
		
		TEST_ASSERT(result == false);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
		
		AssertMap(inlineMap, expectedMap);
	});

	TEST_CASE(L"New contains existing")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		auto prop2 = CreateInlineProp(200);
		
		AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25}, prop1);
		bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 30}, prop2);
		
		TEST_ASSERT(result == false);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 15, .caretEnd = 25}, prop1);
		
		AssertMap(inlineMap, expectedMap);
	});

	TEST_CASE(L"New contained within existing")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		auto prop2 = CreateInlineProp(200);
		
		AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 30}, prop1);
		bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25}, prop2);
		
		TEST_ASSERT(result == false);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 10, .caretEnd = 30}, prop1);
		
		AssertMap(inlineMap, expectedMap);
	});

	// Edge cases
	TEST_CASE(L"Gap fitting with multiple objects")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		auto prop2 = CreateInlineProp(200);
		auto prop3 = CreateInlineProp(300);
		auto prop4 = CreateInlineProp(400);
		
		AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
		AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, prop2);
		AddInlineObjectRun(inlineMap, {.caretBegin = 40, .caretEnd = 50}, prop3);
		bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop4);
		
		TEST_ASSERT(result == true);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
		expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop4);
		expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop2);
		expectedMap.Add({.caretBegin = 40, .caretEnd = 50}, prop3);
		
		AssertMap(inlineMap, expectedMap);
	});

	TEST_CASE(L"Adjacent chain")
	{
		DocumentInlineObjectRunPropertyMap inlineMap;
		auto prop1 = CreateInlineProp(100);
		auto prop2 = CreateInlineProp(200);
		auto prop3 = CreateInlineProp(300);
		
		bool result1 = AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
		bool result2 = AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
		bool result3 = AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, prop3);
		
		TEST_ASSERT(result1 == true);
		TEST_ASSERT(result2 == true);
		TEST_ASSERT(result3 == true);
		
		DocumentInlineObjectRunPropertyMap expectedMap;
		expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
		expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop2);
		expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop3);
		
		AssertMap(inlineMap, expectedMap);
	});
});
```

**Why this change is necessary**:

1. **Critical Validation Logic**: `AddInlineObjectRun` implements overlap detection for inline objects (images, controls) that cannot be split or overlap. Any bug here could cause protocol corruption or application crashes.

2. **Binary Search Correctness**: The implementation uses a custom binary search comparator that returns `equal` for ANY overlap. This must be tested exhaustively to ensure the comparator logic handles all overlap scenarios correctly.

3. **Atomic Object Guarantee**: Inline objects are atomic UI elements that must remain complete and non-overlapping. The test suite validates this critical invariant.

4. **Transaction Semantics**: Failed operations must leave the map unchanged. Tests verify both the return value (`false`) and map state consistency.

5. **Boundary Condition Testing**: Adjacent ranges (`caretEnd == caretBegin` of next) are non-overlapping and must be allowed. This is a critical boundary that the comparer handles with `<=` and `>=` checks.

6. **Protocol Integrity**: The remote rendering protocol depends on this guarantee. Comprehensive tests prevent bugs that could corrupt the protocol or cause remote rendering failures.

## STEP 2: Leverage Existing Test Infrastructure

The implementation will use existing helper functions and patterns from the test file without modification. This ensures consistency with the existing test suite and reduces code duplication.

**Existing infrastructure to use**:

1. **`CreateInlineProp(vint callbackId, vint width = 10)`**: Creates test inline object properties with unique callback IDs
2. **`AssertMap<T>(actualMap, expectedMap)`**: Template function that compares maps and prints differences on failure
3. **`FormatRunProperty(const DocumentInlineObjectRunProperty&)`**: Formats inline objects for debug output
4. **`CompareRunProperty(const DocumentInlineObjectRunProperty&, const DocumentInlineObjectRunProperty&)`**: Deep equality comparison

**Why this is necessary**:

- Reusing existing infrastructure ensures consistency across test categories
- The template-based `AssertMap` already works with `DocumentInlineObjectRunPropertyMap`
- No modifications to helper functions are needed
- Tests follow the established pattern from `AddTextRun` tests

# TEST PLAN

## Test Coverage Overview

The test plan comprehensively covers all aspects of `AddInlineObjectRun` functionality:

1. **Success cases**: 4 tests verifying correct addition when no overlap exists
2. **Failure cases**: 5 tests verifying rejection when overlap is detected
3. **Edge cases**: 2 tests verifying complex scenarios with multiple objects

Total: **11 test cases** ensuring complete coverage of the overlap detection logic.

## Success Cases (Return true, Map Updated)

### Test 1: Add to empty map
- **Setup**: Empty map
- **Action**: Add inline object `[0, 10]` with callback ID 100
- **Verify**: 
  - Return value is `true`
  - Map contains exactly one entry `[0, 10]` with correct property
- **Rationale**: Simplest success case, validates the empty map fast path

### Test 2: Add non-overlapping objects with gap
- **Setup**: Map contains `[10, 20]` with callback ID 100
- **Action**: Add inline object `[30, 40]` with callback ID 200
- **Verify**:
  - Return value is `true`
  - Map contains both entries with correct properties
- **Rationale**: Validates binary search correctly identifies non-overlapping ranges with clear gap

### Test 3: Adjacent objects - boundary test
- **Setup**: Map contains `[0, 10]` with callback ID 100
- **Action**: Add inline object `[10, 20]` with callback ID 200
- **Verify**:
  - Return value is `true`
  - Map contains both entries
- **Rationale**: **Critical boundary test** - adjacent ranges where `caretEnd == caretBegin` must NOT be considered overlapping. Tests the `<=` and `>=` conditions in the comparator.

### Test 4: Multiple non-overlapping objects
- **Setup**: Map contains `[0, 10]` and `[30, 40]` with callback IDs 100, 200
- **Action**: Add inline object `[50, 60]` with callback ID 300
- **Verify**:
  - Return value is `true`
  - Map contains all three entries
- **Rationale**: Validates that binary search correctly navigates maps with multiple entries

## Failure Cases (Return false, Map Unchanged)

### Test 5: Complete overlap - exact match
- **Setup**: Map contains `[10, 20]` with callback ID 100
- **Action**: Attempt to add `[10, 20]` with callback ID 200
- **Verify**:
  - Return value is `false`
  - Map still contains only original entry with callback ID 100
- **Rationale**: Validates exact range overlap detection and transaction semantics (no partial updates)

### Test 6: Partial overlap from left
- **Setup**: Map contains `[20, 30]` with callback ID 100
- **Action**: Attempt to add `[15, 25]` with callback ID 200
- **Verify**:
  - Return value is `false`
  - Map unchanged (still contains `[20, 30]` only)
- **Rationale**: Tests overlap detection when new range starts before and ends during existing range. Comparator should return `equal` because `15 < 30 && 25 > 20`.

### Test 7: Partial overlap from right
- **Setup**: Map contains `[10, 20]` with callback ID 100
- **Action**: Attempt to add `[15, 25]` with callback ID 200
- **Verify**:
  - Return value is `false`
  - Map unchanged
- **Rationale**: Tests overlap detection when new range starts during and ends after existing range. Comparator should return `equal` because `15 < 20 && 25 > 10`.

### Test 8: New contains existing
- **Setup**: Map contains `[15, 25]` with callback ID 100
- **Action**: Attempt to add `[10, 30]` with callback ID 200
- **Verify**:
  - Return value is `false`
  - Map unchanged
- **Rationale**: Tests overlap detection when new range completely encloses existing range. Comparator should return `equal` because `10 < 25 && 30 > 15`.

### Test 9: New contained within existing
- **Setup**: Map contains `[10, 30]` with callback ID 100
- **Action**: Attempt to add `[15, 25]` with callback ID 200
- **Verify**:
  - Return value is `false`
  - Map unchanged
- **Rationale**: Tests overlap detection when new range is completely inside existing range. Comparator should return `equal` because `15 < 30 && 25 > 10`.

## Edge Cases (Complex Scenarios)

### Test 10: Gap fitting with multiple objects
- **Setup**: Map contains `[0, 10]`, `[20, 30]`, `[40, 50]` with callback IDs 100, 200, 300
- **Action**: Add inline object `[10, 20]` with callback ID 400
- **Verify**:
  - Return value is `true`
  - Map contains all four entries in correct order
- **Rationale**: Validates binary search correctly navigates multiple entries and finds the gap. New object fits perfectly between first two without overlapping either.

### Test 11: Adjacent chain
- **Setup**: Empty map
- **Action**: Sequentially add three adjacent objects:
  - `[0, 10]` with callback ID 100
  - `[10, 20]` with callback ID 200
  - `[20, 30]` with callback ID 300
- **Verify**:
  - All three operations return `true`
  - Map contains all three entries covering `[0, 30]` continuously
- **Rationale**: Validates that multiple adjacent objects can be chained together. This is a common pattern in document rendering where inline objects are placed consecutively.

## Verification Strategy

Each test case follows a consistent verification pattern:

1. **Return Value Check**: Verify `AddInlineObjectRun` returns expected boolean
2. **Map State Check**: Build expected map and use `AssertMap` to compare
3. **Property Verification**: `AssertMap` uses `CompareRunProperty` to verify deep equality

## Coverage Analysis

### Branch Coverage
- ✅ Empty map fast path (Test 1)
- ✅ Binary search with no overlap (Tests 2, 3, 4, 10, 11)
- ✅ Binary search with overlap detected (Tests 5, 6, 7, 8, 9)

### Boundary Coverage
- ✅ Adjacent ranges (Tests 3, 10, 11)
- ✅ Exact match overlap (Test 5)
- ✅ Partial overlaps from both directions (Tests 6, 7)
- ✅ Containment relationships (Tests 8, 9)

### Overlap Detection Matrix
All possible overlap scenarios are tested:

| New Range vs Existing | Test Case |
|----------------------|-----------|
| No overlap - gap | Test 2, 4 |
| No overlap - adjacent | Test 3, 10, 11 |
| Complete overlap | Test 5 |
| Partial left | Test 6 |
| Partial right | Test 7 |
| Contains existing | Test 8 |
| Contained by existing | Test 9 |

### Binary Search Path Coverage
- ✅ Empty map (special case)
- ✅ Single entry in map
- ✅ Multiple entries with gaps
- ✅ Multiple adjacent entries
- ✅ Finding gap between entries
- ✅ Detecting overlap with any entry

## Quality Assurance

### Test Execution
- All tests execute in-memory with no I/O dependencies
- Tests are deterministic and repeatable
- Clear failure messages via `AssertMap` showing actual vs expected

### Expected Behavior
- All 11 test cases should pass when implementation is correct
- Any implementation bugs in overlap detection will be caught by failure cases
- Transaction semantics (no partial updates on failure) are verified by all failure cases

### Integration with Existing Tests
- Tests follow identical pattern to `AddTextRun` tests
- Uses same helper functions and assertion infrastructure
- Maintains consistency in test organization and naming

## Potential Implementation Issues Detected by Tests

1. **Off-by-one in comparator**: If `<=` or `>=` are incorrectly replaced with `<` or `>`, Test 3, 10, or 11 will fail
2. **Wrong comparator logic**: If overlap detection is incorrect, Tests 5-9 will fail
3. **Partial updates on failure**: If map is modified before returning false, failure tests will detect inconsistency
4. **Binary search issues**: If search doesn't correctly find overlaps, failure tests will pass incorrectly (caught by Test 10 specifically)

## Refactoring Considerations

This is new test code, not refactoring. The implementation (`AddInlineObjectRun` in `GuiRemoteGraphics_Document.cpp`) is already complete and correct. Existing tests in the file already provide good coverage for:

- `CaretRange` comparison operators
- `AddTextRun` with complex splitting and merging logic

The new tests **do not require any changes to existing tests**. They add a new test category in parallel to existing categories.

# !!!FINISHED!!!
