# !!!PLANNING!!!

# UPDATES

# IMPROVEMENT PLAN

## STEP 1: Add Helper Functions for DocumentRunProperty

### Changes Required

Add three new helper functions to the `remote_document_paragrpah_tests` namespace in `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp` to handle the variant type `DocumentRunProperty`:

**Function 1: FormatRunProperty for DocumentRunProperty**

```cpp
WString FormatRunProperty(const DocumentRunProperty& prop)
{
	if (auto textProp = prop.TryGet<DocumentTextRunProperty>())
	{
		return FormatRunProperty(*textProp);
	}
	else
	{
		auto inlineProp = prop.Get<DocumentInlineObjectRunProperty>();
		return FormatRunProperty(inlineProp);
	}
}
```

**Function 2: CompareRunProperty for DocumentRunProperty**

```cpp
bool CompareRunProperty(
	const DocumentRunProperty& a,
	const DocumentRunProperty& b)
{
	if (a.Index() != b.Index())
		return false;
	
	if (auto textA = a.TryGet<DocumentTextRunProperty>())
	{
		auto textB = b.Get<DocumentTextRunProperty>();
		return CompareRunProperty(*textA, textB);
	}
	else
	{
		auto inlineA = a.Get<DocumentInlineObjectRunProperty>();
		auto inlineB = b.Get<DocumentInlineObjectRunProperty>();
		return CompareRunProperty(inlineA, inlineB);
	}
}
```

### Why These Changes Are Necessary

The `MergeRuns` function returns a `DocumentRunPropertyMap` which maps `CaretRange` to `DocumentRunProperty`. `DocumentRunProperty` is a variant type that can hold either `DocumentTextRunProperty` or `DocumentInlineObjectRunProperty`.

The existing test infrastructure has:
- `FormatRunProperty(const DocumentTextRunProperty&)` for text properties
- `FormatRunProperty(const DocumentInlineObjectRunProperty&)` for inline object properties
- `CompareRunProperty` overloads for both types
- `AssertMap` template that works with any map type

To test `MergeRuns`, we need overloads that handle the variant type. These new functions dispatch to the appropriate existing overload based on which variant is active. This allows the existing `AssertMap` and `PrintMap` templates to work seamlessly with `DocumentRunPropertyMap`.

The variant API methods used:
- `Index()`: Returns which variant is currently active (0 for text, 1 for inline object)
- `TryGet<T>()`: Returns pointer if the variant holds type T, nullptr otherwise
- `Get<T>()`: Returns reference to the value (asserts if wrong type)

## STEP 2: Add MergeRuns Test Category - Basic Merging

### Changes Required

Add a new `TEST_CATEGORY(L"MergeRuns")` block with test cases for basic merging scenarios:

**Test Case 1: Empty maps**
```cpp
TEST_CASE(L"Empty maps")
{
	DocumentTextRunPropertyMap textMap;
	DocumentInlineObjectRunPropertyMap inlineMap;
	DocumentRunPropertyMap result;
	
	MergeRuns(textMap, inlineMap, result);
	
	TEST_ASSERT(result.Count() == 0);
}
```

**Test Case 2: Only text runs**
```cpp
TEST_CASE(L"Only text runs")
{
	DocumentTextRunPropertyMap textMap;
	auto prop1 = CreateTextProp(100);
	auto prop2 = CreateTextProp(150);
	
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
	AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop2);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	DocumentRunPropertyMap result;
	
	MergeRuns(textMap, inlineMap, result);
	
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(prop1));
	expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(prop2));
	
	AssertMap(result, expectedResult);
}
```

**Test Case 3: Only inline objects**
```cpp
TEST_CASE(L"Only inline objects")
{
	DocumentTextRunPropertyMap textMap;
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto prop1 = CreateInlineProp(100);
	auto prop2 = CreateInlineProp(200);
	
	AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
	AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40}, prop2);
	
	DocumentRunPropertyMap result;
	
	MergeRuns(textMap, inlineMap, result);
	
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(prop1));
	expectedResult.Add({.caretBegin = 30, .caretEnd = 40}, DocumentRunProperty(prop2));
	
	AssertMap(result, expectedResult);
}
```

**Test Case 4: Both types, no overlap**
```cpp
TEST_CASE(L"Both types, no overlap")
{
	DocumentTextRunPropertyMap textMap;
	auto textProp1 = CreateTextProp(100);
	auto textProp2 = CreateTextProp(150);
	
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, textProp1);
	AddTextRun(textMap, {.caretBegin = 40, .caretEnd = 50}, textProp2);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp = CreateInlineProp(200);
	
	AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, inlineProp);
	
	DocumentRunPropertyMap result;
	
	MergeRuns(textMap, inlineMap, result);
	
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp1));
	expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(inlineProp));
	expectedResult.Add({.caretBegin = 40, .caretEnd = 50}, DocumentRunProperty(textProp2));
	
	AssertMap(result, expectedResult);
}
```

### Why These Changes Are Necessary

These basic test cases validate:
1. **Empty maps**: The loop termination condition works correctly when both inputs are empty
2. **Only text runs**: Text runs are correctly converted to `DocumentRunProperty` variant and added to result
3. **Only inline objects**: Inline objects are correctly converted to variant and added to result
4. **No overlap**: The two-pointer merge algorithm correctly interleaves runs when there's no conflict

These are the simplest success cases that must work before testing complex overlap scenarios.

## STEP 3: Add MergeRuns Test Category - Inline Object Priority

### Changes Required

Add test cases that verify inline objects always take precedence over text when they overlap:

**Test Case 5: Text completely overlaps inline - text removed**
```cpp
TEST_CASE(L"Text completely overlaps inline - text removed")
{
	DocumentTextRunPropertyMap textMap;
	auto textProp = CreateTextProp(100);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 30}, textProp);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp = CreateInlineProp(200);
	AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, inlineProp);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Text [0,30] containing Inline [10,20] should result in:
	// Text [0,10], Inline [10,20], Text [20,30]
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp));
	expectedResult.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(inlineProp));
	expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(textProp));
	
	AssertMap(result, expectedResult);
}
```

**Test Case 6: Text partial overlap from left - text split**
```cpp
TEST_CASE(L"Text partial overlap from left - text split")
{
	DocumentTextRunPropertyMap textMap;
	auto textProp = CreateTextProp(100);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 25}, textProp);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp = CreateInlineProp(200);
	AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, inlineProp);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Text [0,25] overlapping Inline [20,30] from left should result in:
	// Text [0,20], Inline [20,30]
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 20}, DocumentRunProperty(textProp));
	expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(inlineProp));
	
	AssertMap(result, expectedResult);
}
```

**Test Case 7: Text partial overlap from right - text split**
```cpp
TEST_CASE(L"Text partial overlap from right - text split")
{
	DocumentTextRunPropertyMap textMap;
	auto textProp = CreateTextProp(100);
	AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 40}, textProp);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp = CreateInlineProp(200);
	AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 25}, inlineProp);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Text [20,40] overlapping Inline [10,25] from right should result in:
	// Inline [10,25], Text [25,40]
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 10, .caretEnd = 25}, DocumentRunProperty(inlineProp));
	expectedResult.Add({.caretBegin = 25, .caretEnd = 40}, DocumentRunProperty(textProp));
	
	AssertMap(result, expectedResult);
}
```

**Test Case 8: One text run overlaps multiple inline objects - fragmentation**
```cpp
TEST_CASE(L"One text run overlaps multiple inline objects - fragmentation")
{
	DocumentTextRunPropertyMap textMap;
	auto textProp = CreateTextProp(100);
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 50}, textProp);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp1 = CreateInlineProp(201, 5);
	auto inlineProp2 = CreateInlineProp(202, 5);
	auto inlineProp3 = CreateInlineProp(203, 5);
	
	AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 15}, inlineProp1);
	AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 25}, inlineProp2);
	AddInlineObjectRun(inlineMap, {.caretBegin = 40, .caretEnd = 45}, inlineProp3);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Text [0,50] with three inline objects should fragment into:
	// Text [0,10], Inline [10,15], Text [15,20], Inline [20,25], 
	// Text [25,40], Inline [40,45], Text [45,50]
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp));
	expectedResult.Add({.caretBegin = 10, .caretEnd = 15}, DocumentRunProperty(inlineProp1));
	expectedResult.Add({.caretBegin = 15, .caretEnd = 20}, DocumentRunProperty(textProp));
	expectedResult.Add({.caretBegin = 20, .caretEnd = 25}, DocumentRunProperty(inlineProp2));
	expectedResult.Add({.caretBegin = 25, .caretEnd = 40}, DocumentRunProperty(textProp));
	expectedResult.Add({.caretBegin = 40, .caretEnd = 45}, DocumentRunProperty(inlineProp3));
	expectedResult.Add({.caretBegin = 45, .caretEnd = 50}, DocumentRunProperty(textProp));
	
	AssertMap(result, expectedResult);
}
```

**Test Case 9: Multiple text runs overlap one inline object**
```cpp
TEST_CASE(L"Multiple text runs overlap one inline object")
{
	DocumentTextRunPropertyMap textMap;
	auto textProp1 = CreateTextProp(100);
	auto textProp2 = CreateTextProp(150);
	
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 15}, textProp1);
	AddTextRun(textMap, {.caretBegin = 25, .caretEnd = 40}, textProp2);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp = CreateInlineProp(200);
	AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 30}, inlineProp);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Text [0,15] and Text [25,40] with Inline [10,30] should result in:
	// Text [0,10], Inline [10,30], Text [30,40]
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp1));
	expectedResult.Add({.caretBegin = 10, .caretEnd = 30}, DocumentRunProperty(inlineProp));
	expectedResult.Add({.caretBegin = 30, .caretEnd = 40}, DocumentRunProperty(textProp2));
	
	AssertMap(result, expectedResult);
}
```

### Why These Changes Are Necessary

These test cases validate the **core contract** of `MergeRuns`: inline objects are atomic and cannot be split or removed. They always win when overlapping text runs.

The implementation uses a state machine with:
- `hasCurrentText` flag: Indicates a partial text run is being processed
- `currentTextRange`: The range of the current text run (may be mutated as it's split)
- `currentTextProperty`: The property of the current text run

When text overlaps an inline object:
1. **Before region** (line 268-272): If text starts before the inline object, output text for that region
2. **Inline object** (line 275-277): Always output the complete inline object
3. **After region** (line 279-286): If text extends beyond the inline object, update `currentTextRange.caretBegin` to continue processing in the next iteration

Test Case 8 (fragmentation) is critical because it validates the state machine can process one text run across multiple loop iterations, correctly maintaining state with `hasCurrentText` and mutating `currentTextRange`.

## STEP 4: Add MergeRuns Test Category - Complex Scenarios

### Changes Required

Add test cases for real-world complex scenarios:

**Test Case 10: Interleaved runs with gaps**
```cpp
TEST_CASE(L"Interleaved runs with gaps")
{
	DocumentTextRunPropertyMap textMap;
	auto textProp1 = CreateTextProp(100);
	auto textProp2 = CreateTextProp(150);
	
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, textProp1);
	AddTextRun(textMap, {.caretBegin = 25, .caretEnd = 35}, textProp2);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp1 = CreateInlineProp(200);
	auto inlineProp2 = CreateInlineProp(250);
	
	AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 20}, inlineProp1);
	AddInlineObjectRun(inlineMap, {.caretBegin = 40, .caretEnd = 45}, inlineProp2);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// All four runs should be preserved with gaps
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp1));
	expectedResult.Add({.caretBegin = 15, .caretEnd = 20}, DocumentRunProperty(inlineProp1));
	expectedResult.Add({.caretBegin = 25, .caretEnd = 35}, DocumentRunProperty(textProp2));
	expectedResult.Add({.caretBegin = 40, .caretEnd = 45}, DocumentRunProperty(inlineProp2));
	
	AssertMap(result, expectedResult);
}
```

**Test Case 11: Adjacent text runs separated by inline - no merge**
```cpp
TEST_CASE(L"Adjacent text runs separated by inline - no merge")
{
	DocumentTextRunPropertyMap textMap;
	auto textProp = CreateTextProp(100);  // Same property
	
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, textProp);
	AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, textProp);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp = CreateInlineProp(200);
	AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, inlineProp);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	// Despite identical properties, text runs should remain separate
	// because MergeRuns doesn't perform merging
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp));
	expectedResult.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(inlineProp));
	expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(textProp));
	
	AssertMap(result, expectedResult);
}
```

**Test Case 12: Multiple simultaneous overlaps**
```cpp
TEST_CASE(L"Multiple simultaneous overlaps")
{
	DocumentTextRunPropertyMap textMap;
	auto textProp1 = CreateTextProp(100);
	auto textProp2 = CreateTextProp(120);
	auto textProp3 = CreateTextProp(140);
	
	AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 25}, textProp1);
	AddTextRun(textMap, {.caretBegin = 35, .caretEnd = 55}, textProp2);
	AddTextRun(textMap, {.caretBegin = 65, .caretEnd = 100}, textProp3);
	
	DocumentInlineObjectRunPropertyMap inlineMap;
	auto inlineProp1 = CreateInlineProp(201);
	auto inlineProp2 = CreateInlineProp(202);
	auto inlineProp3 = CreateInlineProp(203);
	auto inlineProp4 = CreateInlineProp(204);
	
	AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, inlineProp1);
	AddInlineObjectRun(inlineMap, {.caretBegin = 40, .caretEnd = 50}, inlineProp2);
	AddInlineObjectRun(inlineMap, {.caretBegin = 70, .caretEnd = 75}, inlineProp3);
	AddInlineObjectRun(inlineMap, {.caretBegin = 80, .caretEnd = 85}, inlineProp4);
	
	DocumentRunPropertyMap result;
	MergeRuns(textMap, inlineMap, result);
	
	DocumentRunPropertyMap expectedResult;
	expectedResult.Add({.caretBegin = 0, .caretEnd = 20}, DocumentRunProperty(textProp1));
	expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(inlineProp1));
	expectedResult.Add({.caretBegin = 35, .caretEnd = 40}, DocumentRunProperty(textProp2));
	expectedResult.Add({.caretBegin = 40, .caretEnd = 50}, DocumentRunProperty(inlineProp2));
	expectedResult.Add({.caretBegin = 50, .caretEnd = 55}, DocumentRunProperty(textProp2));
	expectedResult.Add({.caretBegin = 65, .caretEnd = 70}, DocumentRunProperty(textProp3));
	expectedResult.Add({.caretBegin = 70, .caretEnd = 75}, DocumentRunProperty(inlineProp3));
	expectedResult.Add({.caretBegin = 75, .caretEnd = 80}, DocumentRunProperty(textProp3));
	expectedResult.Add({.caretBegin = 80, .caretEnd = 85}, DocumentRunProperty(inlineProp4));
	expectedResult.Add({.caretBegin = 85, .caretEnd = 100}, DocumentRunProperty(textProp3));
	
	AssertMap(result, expectedResult);
}
```

### Why These Changes Are Necessary

These complex scenarios test:
1. **Gaps**: Validates the algorithm correctly handles when there are gaps in the input (no runs covering certain ranges)
2. **No merge**: MergeRuns only combines the two maps; it doesn't perform property-based merging like AddTextRun does. This is intentional separation of concerns.
3. **Multiple overlaps**: Real-world stress test that combines all the overlap patterns in a single call

Test Case 12 is particularly important because it simulates a realistic paragraph with multiple formatting changes and inline objects (images, buttons, etc.), ensuring the algorithm handles complex documents correctly.

# TEST PLAN

## Test Execution

Run the compiled unit test executable with the new `MergeRuns` test category. All test cases should pass.

## Test Coverage

The test plan covers:

### 1. Basic Functionality
- **Empty maps**: Validates loop termination condition
- **Text-only**: Validates text run processing path
- **Inline-only**: Validates inline object processing path
- **No overlap**: Validates basic interleaving

### 2. Inline Object Priority (Core Contract)
- **Complete overlap**: Text run containing inline object → text split into before/after
- **Left overlap**: Text extends to the left of inline → text trimmed
- **Right overlap**: Text extends to the right of inline → text trimmed  
- **Fragmentation**: One text run overlapping multiple inline objects → text fragmented into gaps
- **Multiple texts**: Multiple text runs overlapping one inline object → all texts properly cut

### 3. Edge Cases
- **Gaps**: Non-contiguous runs in input maps
- **No merging**: Adjacent runs with identical properties remain separate
- **Complex scenarios**: Multiple overlaps in realistic combinations

## Corner Cases

1. **Empty inputs**: Both maps empty
2. **Single run**: One text run, no inline objects
3. **Single object**: One inline object, no text runs
4. **Adjacent runs**: Runs touching at boundaries
5. **Gaps**: Non-contiguous coverage
6. **State machine stress**: Text run split across multiple loop iterations (Test Case 8)
7. **Property preservation**: Different text properties don't merge

## Validation Strategy

Each test case uses `AssertMap` which validates:
- Correct count of result entries
- Correct keys (CaretRange values)
- Correct values (DocumentRunProperty with correct variant type and properties)
- No extra or missing entries
- Sorted order (guaranteed by Dictionary)

The helper functions `FormatRunProperty` and `CompareRunProperty` for `DocumentRunProperty` enable clear diagnostic output when tests fail, showing which specific variant and properties differ.

## Alignment with Task Requirements

✓ **Basic merging** - Test Cases 1-4
✓ **Inline object priority** - Test Cases 5-9 comprehensively test all overlap scenarios
✓ **Complex scenarios** - Test Cases 10-12 test real-world combinations
✓ **Helper functions** - Step 1 implements required overloads
✓ **Designated initializers** - All test cases use `{.caretBegin = x, .caretEnd = y}` syntax

## Expected Outcomes

All 12 test cases should pass, validating:
1. The two-pointer merge algorithm correctly processes both input maps
2. Inline objects are never split or removed (atomic guarantee)
3. Text runs are correctly split when overlapping inline objects
4. The state machine (`hasCurrentText`, `currentTextRange`) correctly maintains partial text runs across iterations
5. The result map has correct variant types for each entry
6. No memory issues or crashes with the variant type handling

# !!!FINISHED!!!