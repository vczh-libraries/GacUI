# !!!EXECUTION!!!

# UPDATES

# IMPROVEMENT PLAN

## STEP 1: Add Helper Functions for DocumentRunProperty [DONE]

Add three new helper functions to the `remote_document_paragrpah_tests` namespace in `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`:

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

## STEP 2: Add MergeRuns Test Category - Basic Merging [DONE]

Add a new `TEST_CATEGORY(L"MergeRuns")` block with test cases for basic merging scenarios:

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

## STEP 3: Add MergeRuns Test Category - Inline Object Priority [DONE]

Add test cases that verify inline objects always take precedence over text when they overlap:

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

## STEP 4: Add MergeRuns Test Category - Complex Scenarios [DONE]

Add test cases for real-world complex scenarios:

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

# TEST PLAN

## Test Execution

Run the compiled unit test executable with the new `MergeRuns` test category. All test cases should pass.

## Test Coverage

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

## Expected Outcomes

All 12 test cases should pass, validating:
1. The two-pointer merge algorithm correctly processes both input maps
2. Inline objects are never split or removed (atomic guarantee)
3. Text runs are correctly split when overlapping inline objects
4. The state machine (`hasCurrentText`, `currentTextRange`) correctly maintains partial text runs across iterations
5. The result map has correct variant types for each entry
6. No memory issues or crashes with the variant type handling

# FIXING ATTEMPTS

## Fixing attempt No.1

**Why the original change did not work:**
The original `MergeRuns` implementation had a logic error where it would unconditionally add all inline objects to the result without checking for overlaps with text runs. This caused text runs to be added to the result even when they should have been split or removed due to inline object overlaps.

**What needs to be done:**
Reorder the logic to:
1. Load a text run if we don't have one
2. If we only have text runs left (no more inline objects), add the current text run
3. If we only have inline objects left (no current text run), add the next inline object
4. If we have both, handle the overlap cases properly

**Why this should solve the issue:**
By checking `hasCurrentText && inlineIdx >= inlineKeys.Count()` before checking `!hasCurrentText && inlineIdx < inlineKeys.Count()`, we ensure that text runs are only added when we've finished processing all overlapping inline objects. This prevents text runs from being added prematurely without considering inline object overlaps.

## Fixing attempt No.2

**Why the previous fix did not work:**
The previous fix solved the basic overlap case but still has a bug with text runs that start in the middle of an inline object's range. When a text run is loaded (e.g., Text2 [25,40]) that starts within an inline object's range (Inline [10,30]), the code doesn't check if the newly loaded text run overlaps with inline objects that haven't been compared yet. The logic only handles overlaps when `hasCurrentText` is true AND we're looking at the next inline object, but when we first load a text run, we don't check for overlaps.

**What needs to be done:**
After loading a new text run, immediately check if it overlaps with the current inline object position before proceeding. If the text run starts within or before an inline object we haven't processed yet, we need to handle that overlap immediately.

**Why this should solve the issue:**
By checking for overlaps immediately after loading a text run, we ensure that text runs that start within inline object ranges are properly trimmed or split before being added to the result.

# !!!FINISHED!!!

# !!!VERIFIED!!!