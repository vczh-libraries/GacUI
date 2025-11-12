# !!!EXECUTION!!!

# UPDATES

# IMPROVEMENT PLAN

## STEP 1: Add New Test Category "MergeRuns (partial application)" [DONE]

Add a new `TEST_CATEGORY(L"MergeRuns (partial application)")` section in `TestRemote_DocumentRunManagement.cpp` immediately after the closing `});` of the existing `TEST_CATEGORY(L"MergeRuns")` (around line 1941) and before `TEST_CATEGORY(L"DiffRuns")` (line 1943).

The new test category will contain 9 test cases organized into 4 groups.

## STEP 2: Implement Group 1 - Mandatory Property Validation Tests (3 test cases) [DONE]

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

## STEP 3: Implement Group 2 - Default Value Application Tests (4 test cases) [DONE]

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

## STEP 4: Implement Group 3 - Nullable Text Properties with Inline Objects Tests (1 test case) [DONE]

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

## STEP 5: Implement Group 4 - Complex Nullable Scenarios Tests (2 test cases) [DONE]

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

# TEST PLAN

All new test cases will be added in a new test category `TEST_CATEGORY(L"MergeRuns (partial application)")` in `TestRemote_DocumentRunManagement.cpp`, positioned after the existing `TEST_CATEGORY(L"MergeRuns")` and before `TEST_CATEGORY(L"DiffRuns")`.

## Group 1: Mandatory Property Validation (3 test cases)

1. **Null fontFamily triggers error**: Text run with `fontFamily=null` and `size=12` should trigger `CHECK_ERROR`
2. **Null size triggers error**: Text run with `fontFamily="Arial"` and `size=null` should trigger `CHECK_ERROR`
3. **Both mandatory properties defined succeeds**: Text run with both `fontFamily` and `size` defined (optional properties null) should succeed and apply defaults

## Group 2: Default Value Application (4 test cases)

1. **Null textColor gets Black default**: Verify `textColor` defaults to `Color(0, 0, 0)`
2. **Null backgroundColor gets Black default**: Verify `backgroundColor` defaults to `Color(0, 0, 0)`
3. **Null textStyle gets TextStyle 0 default**: Verify `textStyle` defaults to `(TextStyle)0` (all FontProperties flags false)
4. **All optional properties null get defaults**: Verify all optional properties simultaneously get correct defaults when only mandatory properties defined

## Group 3: Nullable Text Properties with Inline Objects (1 test case)

1. **Inline object priority preserved with nullable text properties**: Text run with nullable properties overlapping inline object to verify priority rules maintained and defaults applied to split fragments

## Group 4: Complex Nullable with Multiple Inline Objects (2 test cases)

1. **Text with nulls overlaps multiple inline objects**: Single text run with nullable properties fragmented by multiple inline objects
2. **Nullable text and inline objects with gaps**: Multiple text runs with nullable properties + multiple inline objects + gaps

## Success Criteria

All 9 new test cases must pass with the following verification:

1. **Validation tests**: `TEST_ERROR()` correctly catches `CHECK_ERROR` for null mandatory properties
2. **Default value tests**: All null optional properties receive exact defaults specified in Task No.9
3. **Defined property tests**: All defined properties are preserved without modification
4. **Priority tests**: Inline objects always have priority over text runs regardless of nullable properties
5. **Fragmentation tests**: Text runs split by inline objects produce correct fragments with defaults applied
6. **Integration tests**: Complex scenarios combining multiple features work correctly

# FIXING ATTEMPTS

# !!!VERIFIED!!!

# Comparing to User Edit

No user edit found.

The current implementation in `GuiRemoteGraphics_Document.cpp` exactly matches what was documented in the execution plan for Task 12. All test cases were added as specified:
- Group 1: 3 mandatory property validation tests
- Group 2: 4 default value application tests  
- Group 3: 1 nullable text properties with inline objects test
- Group 4: 2 complex nullable scenarios tests

The task was marked as `!!!VERIFIED!!!` indicating all tests passed successfully.

I verified that the `ApplyOverrides` function, which was visible in the user's active selection, was added in Task 10 (documented in `.github\Learning\2025-11-11-14-52-00\Copilot_Execution.md`) and is correctly implemented in the current source code as documented.

# !!!FINISHED!!!
