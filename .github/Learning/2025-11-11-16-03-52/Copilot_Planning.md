# !!!PLANNING!!!

# UPDATES

## UPDATE

Some test cases looks like testing AddTextRun, e.g. `Multiple text runs with different null patterns`. Sure we need to see if unspecified properties are handled properly, but test cases like this seems duplidated. Because there is no inline objects.

Please identify them and remove, from Step 4 (Step 1-3 seems fine)

# IMPROVEMENT PLAN

## STEP 1: Add New Test Category "MergeRuns (partial application)"

### What to Change

Add a new `TEST_CATEGORY(L"MergeRuns (partial application)")` section in `TestRemote_DocumentRunManagement.cpp` immediately after the closing `});` of the existing `TEST_CATEGORY(L"MergeRuns")` (around line 1941) and before `TEST_CATEGORY(L"DiffRuns")` (line 1943).

The new test category will contain 9 test cases organized into 4 groups to thoroughly test nullable property handling in `MergeRuns`.

### Why This Change Is Necessary

The existing `MergeRuns` test category (lines 1641-1941) only tests scenarios where all text run properties are fully defined. It does not test the nullable property mechanism implemented in Tasks No.8, No.9, and No.10. 

This new test category is needed to:
1. Verify that `MergeRuns` correctly enforces mandatory properties (`fontFamily` and `size`)
2. Verify that `MergeRuns` correctly applies default values when optional properties are null
3. Verify that nullable properties work correctly with the inline object priority system
4. Prevent regressions in the nullable property implementation

## STEP 2: Implement Group 1 - Mandatory Property Validation Tests (3 test cases)

### What to Change

Add 3 test cases to verify that `MergeRuns` enforces the requirement that `fontFamily` and `size` must be non-null:

```cpp
TEST_CASE(L"Null fontFamily triggers error")
{
	DocumentTextRunPropertyMap textMap;
	// Create text run with null fontFamily but defined size
	auto prop = CreateTextPropPartial(
		Nullable<WString>(),      // null fontFamily - should fail
		12,                        // size defined
		Color(100, 100, 100),     // textColor defined
		Color(150, 150, 150),     // backgroundColor defined
		(IGuiGraphicsParagraph::TextStyle)0);  // textStyle defined
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	DocumentRunPropertyMap result;
	
	// MergeRuns should fail because fontFamily is mandatory
	TEST_ERROR(MergeRuns(textMap, inlineMap, result));
});

TEST_CASE(L"Null size triggers error")
{
	DocumentTextRunPropertyMap textMap;
	// Create text run with defined fontFamily but null size
	auto prop = CreateTextPropPartial(
		WString(L"Arial"),        // fontFamily defined
		Nullable<vint>(),         // null size - should fail
		Color(100, 100, 100),     // textColor defined
		Color(150, 150, 150),     // backgroundColor defined
		(IGuiGraphicsParagraph::TextStyle)0);  // textStyle defined
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	DocumentRunPropertyMap result;
	
	// MergeRuns should fail because size is mandatory
	TEST_ERROR(MergeRuns(textMap, inlineMap, result));
});

TEST_CASE(L"Both mandatory properties defined succeeds")
{
	DocumentTextRunPropertyMap textMap;
	// Create text run with both fontFamily and size defined, others null
	auto prop = CreateTextPropPartial(
		WString(L"Arial"),            // fontFamily defined
		12,                            // size defined
		Nullable<Color>(),            // textColor null - should use default
		Nullable<Color>(),            // backgroundColor null - should use default
		Nullable<IGuiGraphicsParagraph::TextStyle>());  // textStyle null - should use default
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	DocumentRunPropertyMap result;
	
	// MergeRuns should succeed
	MergeRuns(textMap, inlineMap, result);
	
	// Verify result has defaults for null optional properties
	DocumentRunPropertyMap expectedResult;
	DocumentTextRunProperty expectedProp;
	expectedProp.textColor = Color(0, 0, 0);  // Black default
	expectedProp.backgroundColor = Color(0, 0, 0);  // Black default
	expectedProp.fontProperties.fontFamily = L"Arial";
	expectedProp.fontProperties.size = 12;
	expectedProp.fontProperties.bold = false;
	expectedProp.fontProperties.italic = false;
	expectedProp.fontProperties.underline = false;
	expectedProp.fontProperties.strikeline = false;
	expectedProp.fontProperties.antialias = true;
	expectedProp.fontProperties.verticalAntialias = true;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(expectedProp));
	
	AssertMap(result, expectedResult);
});
```

### Why This Change Is Necessary

According to Task No.9, the `ConvertToFullProperty` function uses `CHECK_ERROR` to validate that `fontFamily` and `size` are not null before converting to `DocumentTextRunProperty`. This is necessary because the protocol layer requires complete `FontProperties`.

These tests ensure:
1. The validation logic is working correctly
2. `MergeRuns` rejects incomplete text runs that would cause protocol errors
3. The error handling path is exercised and verified
4. When both mandatory fields are provided, conversion succeeds even if optional fields are null

## STEP 3: Implement Group 2 - Default Value Application Tests (4 test cases)

### What to Change

Add 4 test cases to verify that `MergeRuns` correctly applies default values for null optional properties:

```cpp
TEST_CASE(L"Null textColor gets Black default")
{
	DocumentTextRunPropertyMap textMap;
	auto prop = CreateTextPropPartial(
		WString(L"Arial"),
		12,
		Nullable<Color>(),            // null textColor
		Color(200, 200, 200),         // backgroundColor defined
		(IGuiGraphicsParagraph::TextStyle)0);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Verify textColor is Black (0, 0, 0)
	TEST_ASSERT(result.Count() == 1);
	auto resultProp = result[{.caretBegin = 0, .caretEnd = 10}].Get<DocumentTextRunProperty>();
	TEST_ASSERT(resultProp.textColor.r == 0 && 
	            resultProp.textColor.g == 0 && 
	            resultProp.textColor.b == 0);
	TEST_ASSERT(resultProp.backgroundColor.r == 200 &&
	            resultProp.backgroundColor.g == 200 &&
	            resultProp.backgroundColor.b == 200);
});

TEST_CASE(L"Null backgroundColor gets Black default")
{
	DocumentTextRunPropertyMap textMap;
	auto prop = CreateTextPropPartial(
		WString(L"Arial"),
		12,
		Color(100, 100, 100),         // textColor defined
		Nullable<Color>(),            // null backgroundColor
		(IGuiGraphicsParagraph::TextStyle)0);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Verify backgroundColor is Black (0, 0, 0)
	TEST_ASSERT(result.Count() == 1);
	auto resultProp = result[{.caretBegin = 0, .caretEnd = 10}].Get<DocumentTextRunProperty>();
	TEST_ASSERT(resultProp.textColor.r == 100 && 
	            resultProp.textColor.g == 100 && 
	            resultProp.textColor.b == 100);
	TEST_ASSERT(resultProp.backgroundColor.r == 0 &&
	            resultProp.backgroundColor.g == 0 &&
	            resultProp.backgroundColor.b == 0);
});

TEST_CASE(L"Null textStyle gets TextStyle 0 default")
{
	DocumentTextRunPropertyMap textMap;
	auto prop = CreateTextPropPartial(
		WString(L"Arial"),
		12,
		Color(100, 100, 100),
		Color(150, 150, 150),
		Nullable<IGuiGraphicsParagraph::TextStyle>());  // null textStyle
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Verify textStyle is converted to FontProperties with all flags false
	TEST_ASSERT(result.Count() == 1);
	auto resultProp = result[{.caretBegin = 0, .caretEnd = 10}].Get<DocumentTextRunProperty>();
	TEST_ASSERT(resultProp.fontProperties.bold == false);
	TEST_ASSERT(resultProp.fontProperties.italic == false);
	TEST_ASSERT(resultProp.fontProperties.underline == false);
	TEST_ASSERT(resultProp.fontProperties.strikeline == false);
});

TEST_CASE(L"All optional properties null get defaults")
{
	DocumentTextRunPropertyMap textMap;
	// Only mandatory properties defined
	auto prop = CreateTextPropPartial(
		WString(L"TestFont"),
		14,
		Nullable<Color>(),            // null textColor
		Nullable<Color>(),            // null backgroundColor
		Nullable<IGuiGraphicsParagraph::TextStyle>());  // null textStyle
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 20}, prop);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Verify all optional properties have defaults
	DocumentRunPropertyMap expectedResult;
	DocumentTextRunProperty expectedProp;
	expectedProp.textColor = Color(0, 0, 0);
	expectedProp.backgroundColor = Color(0, 0, 0);
	expectedProp.fontProperties.fontFamily = L"TestFont";
	expectedProp.fontProperties.size = 14;
	expectedProp.fontProperties.bold = false;
	expectedProp.fontProperties.italic = false;
	expectedProp.fontProperties.underline = false;
	expectedProp.fontProperties.strikeline = false;
	expectedProp.fontProperties.antialias = true;
	expectedProp.fontProperties.verticalAntialias = true;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 20}, DocumentRunProperty(expectedProp));
	
	AssertMap(result, expectedResult);
});
```

### Why This Change Is Necessary

According to Task No.9, when `ConvertToFullProperty` encounters null optional properties, it applies these defaults:
- `textColor`: `Color(0, 0, 0)` (Black)
- `backgroundColor`: `Color(0, 0, 0)` (Black)
- `textStyle`: `(TextStyle)0` (no flags set, converted to FontProperties with all false)

These tests verify that:
1. Each optional property independently receives the correct default when null
2. The default values match the implementation in `ConvertToFullProperty`
3. Defined properties are preserved while null properties get defaults
4. All optional properties can be null simultaneously and still produce valid output

This is critical because `MergeRuns` has different semantics from `AddTextRun`: null means "use default" rather than "keep existing".

## STEP 4: Implement Group 3 - Nullable Text Properties with Inline Objects Tests (1 test case)

### What to Change

Add 1 test case to verify that nullable text properties work correctly with inline object priority:

```cpp
TEST_CASE(L"Inline object priority preserved with nullable text properties")
{
	DocumentTextRunPropertyMap textMap;
	// Text run with nullable properties that overlaps inline object
	auto textProp = CreateTextPropPartial(
		WString(L"Arial"),
		12,
		Nullable<Color>(),            // null textColor
		Color(100, 100, 100),         // backgroundColor defined
		Nullable<IGuiGraphicsParagraph::TextStyle>());  // null textStyle
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 30}, textProp);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp = CreateInlineProp(100);
	AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, inlineProp);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Verify text is split around inline object and gets correct defaults
	TEST_ASSERT(result.Count() == 3);
	
	// Text [0, 10]
	auto textProp1 = result[{.caretBegin = 0, .caretEnd = 10}].Get<DocumentTextRunProperty>();
	TEST_ASSERT(textProp1.textColor == Color(0, 0, 0));  // Default
	TEST_ASSERT(textProp1.backgroundColor == Color(100, 100, 100));  // Preserved
	
	// Inline [10, 20] - priority over text
	auto inline1 = result[{.caretBegin = 10, .caretEnd = 20}].Get<DocumentInlineObjectRunProperty>();
	TEST_ASSERT(inline1.callbackId == 100);
	
	// Text [20, 30]
	auto textProp2 = result[{.caretBegin = 20, .caretEnd = 30}].Get<DocumentTextRunProperty>();
	TEST_ASSERT(textProp2.textColor == Color(0, 0, 0));  // Default
	TEST_ASSERT(textProp2.backgroundColor == Color(100, 100, 100));  // Preserved
});
```

### Why This Change Is Necessary

This test verifies the critical integration point between nullable text properties and the inline object priority system:

1. **Inline object priority**: The inline object priority system must work correctly regardless of which text properties are null
2. **Text fragmentation with defaults**: When a text run with nullable properties is split by an inline object, both fragments must receive the same default values

This ensures that the nullable property mechanism integrates correctly with the most complex aspect of `MergeRuns` functionality: inline object priority and text splitting.

**Note**: Other combinations (e.g., multiple text runs without inline objects, all properties defined) are essentially testing `AddTextRun` behavior or basic conversion, which are already covered by other test groups. This focused test ensures the unique `MergeRuns` behavior - inline object priority - works correctly with nullable properties.

## STEP 5: Implement Group 4 - Complex Nullable Scenarios Tests (2 test cases)

### What to Change

Add 2 test cases to verify nullable properties work correctly in complex scenarios:

```cpp
TEST_CASE(L"Text with nulls overlaps multiple inline objects")
{
	DocumentTextRunPropertyMap textMap;
	// Single text run with nullable properties spanning multiple inline objects
	auto textProp = CreateTextPropPartial(
		WString(L"TestFont"),
		12,
		Color(50, 50, 50),            // textColor defined
		Nullable<Color>(),            // null backgroundColor
		Nullable<IGuiGraphicsParagraph::TextStyle>());  // null textStyle
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 50}, textProp);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp1 = CreateInlineProp(101, 5);
	auto inlineProp2 = CreateInlineProp(102, 5);
	AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 15}, inlineProp1);
	AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 35}, inlineProp2);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Text should be fragmented: [0,10], [10,15], [15,30], [30,35], [35,50]
	// Text fragments should all have the same properties with defaults applied
	TEST_ASSERT(result.Count() == 5);
	
	// Verify text fragments have correct defaults
	auto textFrag1 = result[{.caretBegin = 0, .caretEnd = 10}].Get<DocumentTextRunProperty>();
	TEST_ASSERT(textFrag1.textColor == Color(50, 50, 50));  // Preserved
	TEST_ASSERT(textFrag1.backgroundColor == Color(0, 0, 0));  // Default
	TEST_ASSERT(textFrag1.fontProperties.bold == false);  // Default from null textStyle
	
	// Inline objects preserved
	auto inline1 = result[{.caretBegin = 10, .caretEnd = 15}].Get<DocumentInlineObjectRunProperty>();
	TEST_ASSERT(inline1.callbackId == 101);
	
	auto textFrag2 = result[{.caretBegin = 15, .caretEnd = 30}].Get<DocumentTextRunProperty>();
	TEST_ASSERT(textFrag2.textColor == Color(50, 50, 50));
	TEST_ASSERT(textFrag2.backgroundColor == Color(0, 0, 0));
	
	auto inline2 = result[{.caretBegin = 30, .caretEnd = 35}].Get<DocumentInlineObjectRunProperty>();
	TEST_ASSERT(inline2.callbackId == 102);
	
	auto textFrag3 = result[{.caretBegin = 35, .caretEnd = 50}].Get<DocumentTextRunProperty>();
	TEST_ASSERT(textFrag3.textColor == Color(50, 50, 50));
	TEST_ASSERT(textFrag3.backgroundColor == Color(0, 0, 0));
});

TEST_CASE(L"Interleaved runs with gaps and nulls")
{
	DocumentTextRunPropertyMap textMap;
	// Multiple text runs with different null patterns and gaps between them
	auto prop1 = CreateTextPropPartial(
		WString(L"Font1"),
		10,
		Nullable<Color>(),            // null textColor
		Color(100, 100, 100),         // backgroundColor defined
		(IGuiGraphicsParagraph::TextStyle)0);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
	
	auto prop2 = CreateTextPropPartial(
		WString(L"Font2"),
		14,
		Color(200, 0, 0),             // textColor defined
		Nullable<Color>(),            // null backgroundColor
		Nullable<IGuiGraphicsParagraph::TextStyle>());  // null textStyle
	AddTextRun(textMap, {.caretBegin = 25, .caretEnd = 35}, prop2);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp1 = CreateInlineProp(201);
	auto inlineProp2 = CreateInlineProp(202);
	AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 20}, inlineProp1);
	AddInlineObjectRun(inlineMap, {.caretBegin = 40, .caretEnd = 45}, inlineProp2);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Expected: Text [0,10], gap [10,15], Inline [15,20], gap [20,25], 
	//           Text [25,35], gap [35,40], Inline [40,45]
	// Only non-gap ranges appear in result
	TEST_ASSERT(result.Count() == 4);
	
	// Text run 1 with defaults applied
	auto text1 = result[{.caretBegin = 0, .caretEnd = 10}].Get<DocumentTextRunProperty>();
	TEST_ASSERT(text1.textColor == Color(0, 0, 0));  // Default
	TEST_ASSERT(text1.backgroundColor == Color(100, 100, 100));  // Preserved
	
	// Inline object 1
	auto inline1 = result[{.caretBegin = 15, .caretEnd = 20}].Get<DocumentInlineObjectRunProperty>();
	TEST_ASSERT(inline1.callbackId == 201);
	
	// Text run 2 with defaults applied
	auto text2 = result[{.caretBegin = 25, .caretEnd = 35}].Get<DocumentTextRunProperty>();
	TEST_ASSERT(text2.textColor == Color(200, 0, 0));  // Preserved
	TEST_ASSERT(text2.backgroundColor == Color(0, 0, 0));  // Default
	TEST_ASSERT(text2.fontProperties.bold == false);  // Default from null textStyle
	
	// Inline object 2
	auto inline2 = result[{.caretBegin = 40, .caretEnd = 45}].Get<DocumentInlineObjectRunProperty>();
	TEST_ASSERT(inline2.callbackId == 202);
});
```

### Why This Change Is Necessary

These complex scenarios test the integration of nullable properties with the most challenging aspects of `MergeRuns`:

1. **Text fragmentation by multiple inline objects**: When a text run with nullable properties is split by multiple inline objects, each fragment must have defaults applied correctly
2. **Gap handling with inline objects**: Gaps between runs should remain as gaps; nullable properties don't affect gap logic, and inline object priority is maintained
3. **Multiple fragments from same source**: All fragments from the same original text run should have identical defaults applied

These tests ensure that nullable property handling is robust and doesn't break under complex real-world scenarios involving text fragmentation with inline objects. Unlike simpler cases, these tests specifically verify the interaction between nullable properties and MergeRuns' unique inline object priority system in complex layouts.

# TEST PLAN

## Test Organization

All new test cases will be added in a new test category `TEST_CATEGORY(L"MergeRuns (partial application)")` in `TestRemote_DocumentRunManagement.cpp`, positioned after the existing `TEST_CATEGORY(L"MergeRuns")` and before `TEST_CATEGORY(L"DiffRuns")`.

This organizational structure:
- Mirrors the pattern established by `AddTextRun (partial application)` in Task No.11
- Clearly separates tests for nullable property semantics from basic merge logic tests
- Makes it easy to locate and maintain nullable property tests

## Test Coverage

### Group 1: Mandatory Property Validation (3 test cases)

Tests that verify `MergeRuns` enforces mandatory property requirements:

1. **Null fontFamily triggers error**: Text run with `fontFamily=null` and `size=12` should trigger `CHECK_ERROR`
2. **Null size triggers error**: Text run with `fontFamily="Arial"` and `size=null` should trigger `CHECK_ERROR`
3. **Both mandatory properties defined succeeds**: Text run with both `fontFamily` and `size` defined (optional properties null) should succeed and apply defaults

**Coverage**: Validates that `ConvertToFullProperty` correctly enforces mandatory properties before protocol conversion. Uses `TEST_ERROR()` macro to verify error handling.

### Group 2: Default Value Application (4 test cases)

Tests that verify `MergeRuns` applies correct default values for null optional properties:

1. **Null textColor gets Black default**: Verify `textColor` defaults to `Color(0, 0, 0)`
2. **Null backgroundColor gets Black default**: Verify `backgroundColor` defaults to `Color(0, 0, 0)`
3. **Null textStyle gets TextStyle 0 default**: Verify `textStyle` defaults to `(TextStyle)0` (all FontProperties flags false)
4. **All optional properties null get defaults**: Verify all optional properties simultaneously get correct defaults when only mandatory properties defined

**Coverage**: Validates that each optional property independently receives the correct default value as specified in the `ConvertToFullProperty` implementation. Ensures the default values match Task No.9's specification.

### Group 3: Nullable Text Properties with Inline Objects (1 test case)

Tests that verify nullable text properties work correctly with inline object priority:

1. **Inline object priority preserved with nullable text properties**: Text run with nullable properties overlapping inline object to verify priority rules maintained and defaults applied to split fragments

**Coverage**: Validates the interaction between nullable properties and inline object priority. Ensures nullable property mechanism doesn't break the core MergeRuns functionality of splitting text runs around inline objects.

### Group 4: Complex Nullable with Multiple Inline Objects (2 test cases)

Tests that verify nullable properties work correctly with multiple inline objects:

1. **Text with nulls overlaps multiple inline objects**: Single text run with nullable properties fragmented by multiple inline objects
2. **Nullable text and inline objects with gaps**: Multiple text runs with nullable properties + multiple inline objects + gaps

**Coverage**: Validates that nullable property handling is robust under complex real-world scenarios involving text fragmentation, gap handling, and mixed run types. Ensures the complete merge logic works correctly with nullable properties.

## Corner Cases Covered

1. **Mandatory validation**: Both `fontFamily` and `size` null validation, and successful case with only mandatory defined
2. **Each optional property independently**: Separate tests for `textColor`, `backgroundColor`, and `textStyle` defaults
3. **All optional nulls simultaneously**: Verify multiple null properties handled correctly at once
4. **Inline object priority**: All Group 3 and Group 4 tests verify nullable text properties don't affect inline object priority
5. **Text fragmentation**: Verify that when text runs are split by inline objects, each fragment gets defaults applied
6. **Gap preservation**: Verify gaps between runs remain gaps regardless of nullable properties

## Success Criteria

All 9 new test cases must pass with the following verification:

1. **Validation tests**: `TEST_ERROR()` correctly catches `CHECK_ERROR` for null mandatory properties
2. **Default value tests**: All null optional properties receive exact defaults specified in Task No.9
3. **Defined property tests**: All defined properties are preserved without modification
4. **Priority tests**: Inline objects always have priority over text runs regardless of nullable properties
5. **Fragmentation tests**: Text runs split by inline objects produce correct fragments with defaults applied
6. **Integration tests**: Complex scenarios combining multiple features work correctly

## Testing Approach

### Execution

Run the compiled unit test executable using the existing test infrastructure:
- Build using: Build task in Visual Studio / VS Code
- Run using: Run Unit Tests task

### Verification Strategy

Each test case follows the pattern:

1. **Setup**: Create `DocumentTextRunPropertyMap` with nullable properties using `CreateTextPropPartial()`
2. **Setup**: Create `DocumentInlineObjectRunPropertyMap` if needed for priority tests
3. **Execute**: Call `MergeRuns(textMap, inlineMap, result)`
4. **Verify**: Check result against expected using `AssertMap()` or direct property comparison

For error validation tests:
- Use `TEST_ERROR()` macro to verify `CHECK_ERROR` is raised
- Provides clear failure messages when validation isn't working

For default value tests:
- Extract `DocumentTextRunProperty` from result
- Directly compare color values and font property flags
- Verify only null properties received defaults; defined properties unchanged

## Existing Test Integration

### No Changes to Existing Tests

The existing test categories remain unchanged:
- `CaretRange`: No changes needed
- `AddTextRun`: No changes needed
- `AddTextRun (partial application)`: No changes needed (Task No.11's tests)
- `AddInlineObjectRun`: No changes needed
- `ResetInlineObjectRun`: No changes needed
- `MergeRuns`: No changes needed (basic functionality tests)
- `DiffRuns`: No changes needed
- `DiffRuns (Complex)`: No changes needed

**Rationale**: Task No.8, No.9, and No.10 maintain backward compatibility by preserving existing behavior when all properties are fully defined. The nullable property mechanism is an addition, not a modification of existing functionality.

### Helper Functions

The test implementation will use existing helper functions:
- `CreateTextPropPartial()`: Create text runs with selective null properties
- `CreateTextProp()`: Create fully-defined text runs (for backward compatibility tests)
- `CreateFullTextProp()`: Create expected `DocumentTextRunProperty` results
- `CreateInlineProp()`: Create inline object properties
- `AddTextRun()`: Add text runs to map
- `AddInlineObjectRun()`: Add inline objects to map
- `AssertMap()`: Compare actual vs expected maps
- `CompareRunProperty()`: Compare individual properties
- `FormatRunProperty()`: Format properties for error messages

No new helper functions needed.

## Regression Prevention

These tests prevent regressions in:

1. **Mandatory validation**: Ensure incomplete properties never reach protocol layer
2. **Default value application**: Ensure default values remain consistent with specification
3. **Inline object priority**: Ensure nullable text properties don't interfere with priority rules
4. **Text fragmentation**: Ensure nullable properties work correctly when text runs are split
5. **Gap handling**: Ensure gaps remain gaps regardless of nullable properties

## Potential Issues and Mitigation

### Issue 1: `TEST_ERROR()` Macro Behavior

**Risk**: If `CHECK_ERROR` is configured differently in test builds, `TEST_ERROR()` might not work as expected.

**Mitigation**: The test file already uses `TEST_ERROR()` in other places (likely in other test categories), so it should work correctly. If tests fail, we can verify `CHECK_ERROR` configuration and adjust test approach.

### Issue 2: Default Value Changes

**Risk**: If default values in `ConvertToFullProperty` change in the future, tests will fail.

**Mitigation**: This is intentional - tests should fail if defaults change. Tests document the expected behavior and ensure changes are intentional and visible.

### Issue 3: Complex Scenario Test Complexity

**Risk**: Complex scenarios might be difficult to debug if they fail.

**Mitigation**: 
- Start with simple tests (Groups 1-2) that verify basic functionality
- Build to complex tests (Group 4) only after basics pass
- Use detailed assertions to pinpoint exact failure location
- Each test focuses on one aspect even in complex scenarios

# !!!FINISHED!!!
