# !!!PLANNING!!!

# UPDATES

## UPDATE

`Edge case - overwrite with same partial properties` could add another similar case that actually changes the font

# IMPROVEMENT PLAN

## STEP 1: Add helper functions for creating partial property overrides

**Location**: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

Add these helper functions after the existing `CreateTextProp` function (around line 22):

```cpp
DocumentTextRunPropertyOverrides CreateTextPropPartial(
	Nullable<WString> fontFamily,
	Nullable<vint> size,
	Nullable<Color> textColor,
	Nullable<Color> backgroundColor,
	Nullable<IGuiGraphicsParagraph::TextStyle> textStyle)
{
	DocumentTextRunPropertyOverrides prop;
	prop.fontFamily = fontFamily;
	prop.size = size;
	prop.textColor = textColor;
	prop.backgroundColor = backgroundColor;
	prop.textStyle = textStyle;
	return prop;
}

DocumentTextRunPropertyOverrides CreateTextPropWithFont(const WString& fontFamily)
{
	DocumentTextRunPropertyOverrides prop;
	prop.fontFamily = fontFamily;
	return prop;
}

DocumentTextRunPropertyOverrides CreateTextPropWithSize(vint size)
{
	DocumentTextRunPropertyOverrides prop;
	prop.size = size;
	return prop;
}

DocumentTextRunPropertyOverrides CreateTextPropWithColor(Color textColor)
{
	DocumentTextRunPropertyOverrides prop;
	prop.textColor = textColor;
	return prop;
}

DocumentTextRunPropertyOverrides CreateTextPropWithBgColor(Color backgroundColor)
{
	DocumentTextRunPropertyOverrides prop;
	prop.backgroundColor = backgroundColor;
	return prop;
}

DocumentTextRunPropertyOverrides CreateTextPropWithStyle(IGuiGraphicsParagraph::TextStyle textStyle)
{
	DocumentTextRunPropertyOverrides prop;
	prop.textStyle = textStyle;
	return prop;
}
```

**Why**: These helper functions enable creating property overrides with specific properties defined and others null, which is essential for testing partial property application. The naming pattern follows the existing `CreateTextProp` helper and makes test code highly readable.

## STEP 2: Add new TEST_CATEGORY for partial application tests

**Location**: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

Add a new test category after the existing `TEST_CATEGORY(L"AddTextRun")` block (which ends around line 825):

```cpp
TEST_CATEGORY(L"AddTextRun (partial application)")
{
	// Test cases will be added in subsequent steps
});
```

**Why**: Following the organizational pattern from Task 7 (which created `DiffRuns (Complex)`), a separate category clearly distinguishes nullable property tests from basic AddTextRun tests, improving test organization and output readability.

## STEP 3: Add test cases for partial property updates

**Location**: Inside the new `TEST_CATEGORY(L"AddTextRun (partial application)")` block

Add these test cases:

```cpp
TEST_CASE(L"Partial update - font only preserves other properties")
{
	DocumentTextRunPropertyMap textMap;
	auto initial = CreateTextProp(100);  // All properties defined
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, initial);
	
	// Update only font, leaving others null
	auto fontUpdate = CreateTextPropWithFont(L"NewFont");
	AddTextRun(textMap, {.caretBegin = 5, .caretEnd = 15}, fontUpdate);
	
	// Expected: three fragments
	// [0,5]: original properties
	// [5,15]: font changed to "NewFont", other properties preserved from original
	// [15,20]: original properties
	DocumentTextRunPropertyMap expectedMap;
	
	auto fragment1 = initial;  // [0,5] unchanged
	expectedMap.Add({.caretBegin = 0, .caretEnd = 5}, fragment1);
	
	auto fragment2 = initial;  // [5,15] font updated, others preserved
	fragment2.fontFamily = L"NewFont";
	expectedMap.Add({.caretBegin = 5, .caretEnd = 15}, fragment2);
	
	auto fragment3 = initial;  // [15,20] unchanged
	expectedMap.Add({.caretBegin = 15, .caretEnd = 20}, fragment3);
	
	AssertMap(textMap, expectedMap);
});

TEST_CASE(L"Partial update - color only preserves other properties")
{
	DocumentTextRunPropertyMap textMap;
	auto initial = CreateTextProp(100);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, initial);
	
	auto colorUpdate = CreateTextPropWithColor(Color(255, 0, 0));  // Red
	AddTextRun(textMap, {.caretBegin = 5, .caretEnd = 15}, colorUpdate);
	
	DocumentTextRunPropertyMap expectedMap;
	
	auto fragment1 = initial;
	expectedMap.Add({.caretBegin = 0, .caretEnd = 5}, fragment1);
	
	auto fragment2 = initial;
	fragment2.textColor = Color(255, 0, 0);
	expectedMap.Add({.caretBegin = 5, .caretEnd = 15}, fragment2);
	
	auto fragment3 = initial;
	expectedMap.Add({.caretBegin = 15, .caretEnd = 20}, fragment3);
	
	AssertMap(textMap, expectedMap);
});

TEST_CASE(L"Partial update - size only preserves other properties")
{
	DocumentTextRunPropertyMap textMap;
	auto initial = CreateTextProp(100);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, initial);
	
	auto sizeUpdate = CreateTextPropWithSize(16);
	AddTextRun(textMap, {.caretBegin = 5, .caretEnd = 15}, sizeUpdate);
	
	DocumentTextRunPropertyMap expectedMap;
	
	auto fragment1 = initial;
	expectedMap.Add({.caretBegin = 0, .caretEnd = 5}, fragment1);
	
	auto fragment2 = initial;
	fragment2.size = 16;
	expectedMap.Add({.caretBegin = 5, .caretEnd = 15}, fragment2);
	
	auto fragment3 = initial;
	expectedMap.Add({.caretBegin = 15, .caretEnd = 20}, fragment3);
	
	AssertMap(textMap, expectedMap);
});

TEST_CASE(L"Partial update - multiple properties, some null")
{
	DocumentTextRunPropertyMap textMap;
	auto initial = CreateTextProp(100);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, initial);
	
	// Update font and color, leave others null
	auto update = CreateTextPropPartial(
		L"PartialFont", 
		Nullable<vint>(),  // size null
		Color(0, 255, 0),   // green
		Nullable<Color>(),  // backgroundColor null
		Nullable<IGuiGraphicsParagraph::TextStyle>());  // style null
	AddTextRun(textMap, {.caretBegin = 5, .caretEnd = 15}, update);
	
	DocumentTextRunPropertyMap expectedMap;
	
	auto fragment1 = initial;
	expectedMap.Add({.caretBegin = 0, .caretEnd = 5}, fragment1);
	
	auto fragment2 = initial;
	fragment2.fontFamily = L"PartialFont";
	fragment2.textColor = Color(0, 255, 0);
	expectedMap.Add({.caretBegin = 5, .caretEnd = 15}, fragment2);
	
	auto fragment3 = initial;
	expectedMap.Add({.caretBegin = 15, .caretEnd = 20}, fragment3);
	
	AssertMap(textMap, expectedMap);
});
```

**Why**: These tests validate the core nullable property feature - partial updates that preserve existing values. Each test focuses on updating specific properties while verifying others remain unchanged. The multiple-property test covers realistic scenarios where some properties are updated and others preserved.

## STEP 4: Add test cases for null property merging

**Location**: Continue in the same `TEST_CATEGORY(L"AddTextRun (partial application)")` block

```cpp
TEST_CASE(L"Merge - identical nullability patterns merge")
{
	DocumentTextRunPropertyMap textMap;
	
	// Both runs have font defined, color null
	auto prop1 = CreateTextPropWithFont(L"Arial");
	auto prop2 = CreateTextPropWithFont(L"Arial");
	
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
	AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
	
	// Should merge because all properties (including nullability) match
	DocumentTextRunPropertyMap expectedMap;
	expectedMap.Add({.caretBegin = 0, .caretEnd = 20}, prop1);
	
	AssertMap(textMap, expectedMap);
});

TEST_CASE(L"Merge - null vs defined prevents merge")
{
	DocumentTextRunPropertyMap textMap;
	
	// First run: font defined, color null
	auto prop1 = CreateTextPropWithFont(L"Arial");
	
	// Second run: font defined, color defined
	auto prop2 = CreateTextPropPartial(
		L"Arial",
		Nullable<vint>(),
		Color(255, 0, 0),  // Red - defined
		Nullable<Color>(),
		Nullable<IGuiGraphicsParagraph::TextStyle>());
	
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
	AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
	
	// Should NOT merge (color differs: null vs Red)
	DocumentTextRunPropertyMap expectedMap;
	expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
	expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop2);
	
	AssertMap(textMap, expectedMap);
});

TEST_CASE(L"Merge - defined vs null prevents merge")
{
	DocumentTextRunPropertyMap textMap;
	
	// First run: font and color defined
	auto prop1 = CreateTextPropPartial(
		L"Arial",
		Nullable<vint>(),
		Color(255, 0, 0),
		Nullable<Color>(),
		Nullable<IGuiGraphicsParagraph::TextStyle>());
	
	// Second run: font defined, color null
	auto prop2 = CreateTextPropWithFont(L"Arial");
	
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
	AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
	
	// Should NOT merge (color differs: Red vs null)
	DocumentTextRunPropertyMap expectedMap;
	expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
	expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop2);
	
	AssertMap(textMap, expectedMap);
});

TEST_CASE(L"Merge - multiple null properties can merge")
{
	DocumentTextRunPropertyMap textMap;
	
	// Both runs have only font defined, all others null
	auto prop1 = CreateTextPropWithFont(L"Courier");
	auto prop2 = CreateTextPropWithFont(L"Courier");
	
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
	AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
	
	// All properties match (including multiple nulls), should merge
	DocumentTextRunPropertyMap expectedMap;
	expectedMap.Add({.caretBegin = 0, .caretEnd = 20}, prop1);
	
	AssertMap(textMap, expectedMap);
});
```

**Why**: These tests validate the critical merge semantics for nullable properties. Null is treated as a distinct value for comparison - two runs with the same null pattern can merge, but different nullability patterns prevent merging. This ensures the implementation correctly handles null as a value during merging.

## STEP 5: Add test cases for layered property application

**Location**: Continue in the same category

```cpp
TEST_CASE(L"Layered - sequential partial updates accumulate properties")
{
	DocumentTextRunPropertyMap textMap;
	
	// Step 1: Add font only
	auto fontOnly = CreateTextPropWithFont(L"Arial");
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, fontOnly);
	
	// Step 2: Add size only (same range)
	auto sizeOnly = CreateTextPropWithSize(14);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, sizeOnly);
	
	// Should merge into single run with both properties
	DocumentTextRunPropertyMap expectedMap;
	auto combined = CreateTextPropPartial(
		L"Arial",
		14,
		Nullable<Color>(),
		Nullable<Color>(),
		Nullable<IGuiGraphicsParagraph::TextStyle>());
	expectedMap.Add({.caretBegin = 0, .caretEnd = 20}, combined);
	
	AssertMap(textMap, expectedMap);
});

TEST_CASE(L"Layered - partial overlap creates split with accumulated properties")
{
	DocumentTextRunPropertyMap textMap;
	
	// Step 1: Add font for full range
	auto fontOnly = CreateTextPropWithFont(L"Arial");
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, fontOnly);
	
	// Step 2: Add size for half range
	auto sizeOnly = CreateTextPropWithSize(14);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, sizeOnly);
	
	// Step 3: Add color for different half range
	auto colorOnly = CreateTextPropWithColor(Color(0, 0, 255));
	AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, colorOnly);
	
	// Should result in two runs with different property combinations
	DocumentTextRunPropertyMap expectedMap;
	
	auto left = CreateTextPropPartial(
		L"Arial",
		14,
		Nullable<Color>(),
		Nullable<Color>(),
		Nullable<IGuiGraphicsParagraph::TextStyle>());
	expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, left);
	
	auto right = CreateTextPropPartial(
		L"Arial",
		Nullable<vint>(),
		Color(0, 0, 255),
		Nullable<Color>(),
		Nullable<IGuiGraphicsParagraph::TextStyle>());
	expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, right);
	
	AssertMap(textMap, expectedMap);
});

TEST_CASE(L"Layered - three-step application with overlapping ranges")
{
	DocumentTextRunPropertyMap textMap;
	
	// Step 1: Font for [0, 30]
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 30}, CreateTextPropWithFont(L"Arial"));
	
	// Step 2: Size for [0, 20]
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, CreateTextPropWithSize(12));
	
	// Step 3: Color for [10, 30]
	AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 30}, CreateTextPropWithColor(Color(255, 0, 0)));
	
	// Expected three regions with different property combinations:
	// [0,10]: font+size
	// [10,20]: font+size+color
	// [20,30]: font+color
	DocumentTextRunPropertyMap expectedMap;
	
	auto region1 = CreateTextPropPartial(L"Arial", 12, Nullable<Color>(), Nullable<Color>(), Nullable<IGuiGraphicsParagraph::TextStyle>());
	expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, region1);
	
	auto region2 = CreateTextPropPartial(L"Arial", 12, Color(255, 0, 0), Nullable<Color>(), Nullable<IGuiGraphicsParagraph::TextStyle>());
	expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, region2);
	
	auto region3 = CreateTextPropPartial(L"Arial", Nullable<vint>(), Color(255, 0, 0), Nullable<Color>(), Nullable<IGuiGraphicsParagraph::TextStyle>());
	expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, region3);
	
	AssertMap(textMap, expectedMap);
});
```

**Why**: These tests validate realistic text editing workflows where formatting is applied incrementally. Text editors typically let users select text and apply one formatting change at a time (bold, then color, then font). The nullable system must correctly accumulate these partial updates, which these tests verify.

## STEP 6: Add test case for edge cases

**Location**: Continue in the same category

```cpp
TEST_CASE(L"Edge case - all null properties")
{
	DocumentTextRunPropertyMap textMap;
	
	// Start with fully defined properties
	auto initial = CreateTextProp(100);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, initial);
	
	// Add run with all properties null
	auto allNull = CreateTextPropPartial(
		Nullable<WString>(),
		Nullable<vint>(),
		Nullable<Color>(),
		Nullable<Color>(),
		Nullable<IGuiGraphicsParagraph::TextStyle>());
	AddTextRun(textMap, {.caretBegin = 5, .caretEnd = 15}, allNull);
	
	// All-null update preserves all existing properties
	// Should result in three fragments, middle has all properties from original
	DocumentTextRunPropertyMap expectedMap;
	expectedMap.Add({.caretBegin = 0, .caretEnd = 5}, initial);
	expectedMap.Add({.caretBegin = 5, .caretEnd = 15}, initial);  // All preserved
	expectedMap.Add({.caretBegin = 15, .caretEnd = 20}, initial);
	
	// But middle fragment cannot merge with sides because it was separately added
	// Actually, after splitting and merging logic, they should merge back
	// Let me reconsider...
	
	// After AddTextRun with all-null, the splits happen:
	// [0,5]: original
	// [5,15]: all properties copied from overlapped original
	// [15,20]: original
	// Then merge phase checks: if [0,5] and [5,15] have identical properties, they merge
	// Since all properties match, all three should merge back
	DocumentTextRunPropertyMap expectedMapMerged;
	expectedMapMerged.Add({.caretBegin = 0, .caretEnd = 20}, initial);
	
	AssertMap(textMap, expectedMapMerged);
});

TEST_CASE(L"Edge case - overwrite with same partial properties")
{
	DocumentTextRunPropertyMap textMap;
	
	// Initial: only font defined
	auto initial = CreateTextPropWithFont(L"Arial");
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, initial);
	
	// Update with same font
	auto update = CreateTextPropWithFont(L"Arial");
	AddTextRun(textMap, {.caretBegin = 5, .caretEnd = 15}, update);
	
	// Properties are identical (font="Arial", all others null)
	// After split and merge, should merge back to single run
	DocumentTextRunPropertyMap expectedMap;
	expectedMap.Add({.caretBegin = 0, .caretEnd = 20}, initial);
	
	AssertMap(textMap, expectedMap);
});

TEST_CASE(L"Edge case - overwrite with different partial properties")
{
	DocumentTextRunPropertyMap textMap;
	
	// Initial: only font defined as Arial
	auto initial = CreateTextPropWithFont(L"Arial");
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, initial);
	
	// Update with different font (Courier)
	auto update = CreateTextPropWithFont(L"Courier");
	AddTextRun(textMap, {.caretBegin = 5, .caretEnd = 15}, update);
	
	// Should create three fragments with different fonts
	DocumentTextRunPropertyMap expectedMap;
	expectedMap.Add({.caretBegin = 0, .caretEnd = 5}, initial);  // Arial
	expectedMap.Add({.caretBegin = 5, .caretEnd = 15}, update);   // Courier
	expectedMap.Add({.caretBegin = 15, .caretEnd = 20}, initial); // Arial
	
	AssertMap(textMap, expectedMap);
});
```

**Why**: Edge cases test boundary conditions. The all-null case verifies that updating with no defined properties effectively preserves everything (and triggers proper merging). The same-property case ensures redundant updates don't cause unnecessary fragmentation. The different-property case validates that actual changes create proper splits and prevent merging when properties differ.

# TEST PLAN

## Existing Test Coverage

The existing test suite in `TEST_CATEGORY(L"AddTextRun")` provides comprehensive coverage of:
- Basic AddTextRun operations (empty map, non-overlapping, adjacent runs)
- Splitting scenarios (complete overlap, partial overlap, contained runs)
- Merging logic (adjacent identical runs merge, different runs don't merge)

These tests use the existing `CreateTextProp()` helper which creates fully-defined property overrides. All these tests should continue to pass unchanged, validating backward compatibility.

## New Test Coverage

The new `TEST_CATEGORY(L"AddTextRun (partial application)")` provides focused coverage of nullable property scenarios:

### Category 1: Partial Property Updates (4 tests)
Tests that partial updates (with some properties null) preserve existing values for null properties:
- Single property updates (font only, color only, size only)
- Multiple property updates with mixed null/defined values

**Success criteria**: After applying a partial update to an existing run, only the defined properties in the update change; null properties preserve their original values.

### Category 2: Null Property Merging (4 tests)
Tests that merging logic correctly treats null as a distinct value:
- Identical nullability patterns merge
- Null vs defined prevents merge
- Defined vs null prevents merge  
- Multiple null properties can merge

**Success criteria**: Two adjacent runs merge if and only if ALL properties match, including nullability (null matches null, Red doesn't match null).

### Category 3: Layered Property Application (3 tests)
Tests realistic multi-step formatting workflows:
- Sequential updates on same range accumulate properties
- Partial overlaps create correct fragments with accumulated properties
- Three-step application with complex overlapping

**Success criteria**: Multiple partial updates correctly layer properties, creating appropriate fragments at range boundaries with correct property combinations.

### Category 4: Edge Cases (3 tests)
Tests boundary conditions:
- All-null update preserves everything and merges correctly
- Same-property update doesn't cause fragmentation
- Different-property update creates proper splits

**Success criteria**: Edge cases behave correctly and efficiently (no unnecessary fragmentation for no-op updates; correct fragmentation for actual changes).

## Test Execution

Run using the existing unit test framework:
```
Build and Run Unit Tests task
```

All tests should pass. The test output will show two categories:
- `AddTextRun`: Original tests (should all pass - validates backward compatibility)
- `AddTextRun (partial application)`: New tests (validates nullable property functionality)

## Coverage Analysis

Total new test cases: **14**
- 4 partial update tests
- 4 merging tests
- 3 layered application tests
- 3 edge case tests

This provides focused coverage of the nullable property feature without being exhaustive. Following the lesson from Task 6, we start with essential scenarios that validate the core contracts:
1. Null means "keep existing" during application
2. Null is treated as a value during comparison/merging
3. Multiple partial updates compose correctly

If issues are found, additional tests can be added incrementally.

## Integration with Existing Tests

The new tests complement the existing AddTextRun tests:
- **Existing tests**: Validate core splitting/merging logic with fully-defined properties
- **New tests**: Validate nullable property semantics layered on top of that core logic

This separation ensures that:
1. Backward compatibility is clearly validated (existing tests unchanged)
2. New functionality is isolated and easy to locate
3. Test failures can be quickly categorized (core logic vs nullable feature)

# !!!FINISHED!!!
