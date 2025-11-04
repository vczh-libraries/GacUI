# !!!EXECUTION!!!

# UPDATES

## UPDATE

The new created file was not added to the UnitTest project. Please do it and ensure it is in the Source Files\Remote solution explorer folder.

**Resolution:** Added `TestRemote_DocumentRunManagement.cpp` to:
- `UnitTest.vcxproj` in the `<ClCompile>` section
- `UnitTest.vcxproj.filters` in the "Source Files\Remote" filter

The project now compiles successfully.

# IMPROVEMENT PLAN

## STEP 1: Create Test File Structure [DONE]

Create a new test file `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp` with the standard GacUI test framework structure.

## STEP 2: Implement Helper Functions for Property Creation [DONE]

Add these helper functions before the `TEST_FILE` block:

```cpp
namespace remote_document_paragrpah_tests
{
	using namespace vl;
	using namespace vl::unittest;
	using namespace vl::collections;
	using namespace vl::presentation;
	using namespace vl::presentation::elements;
	using namespace vl::presentation::remoteprotocol;

	DocumentTextRunProperty CreateTextProp(vint colorValue)
	{
		DocumentTextRunProperty prop;
		prop.textColor = Color(colorValue, colorValue, colorValue);
		prop.backgroundColor = Color(255 - colorValue, 255 - colorValue, 255 - colorValue);
		prop.fontProperties.fontFamily = L"TestFont";
		prop.fontProperties.size = 12;
		prop.fontProperties.bold = false;
		prop.fontProperties.italic = false;
		prop.fontProperties.underline = false;
		prop.fontProperties.strikeline = false;
		prop.fontProperties.antialias = true;
		prop.fontProperties.verticalAntialias = true;
		return prop;
	}

	DocumentInlineObjectRunProperty CreateInlineProp(vint callbackId, vint width = 10)
	{
		DocumentInlineObjectRunProperty prop;
		prop.size = Size(width, 10);
		prop.baseline = 8;
		prop.breakCondition = IGuiGraphicsParagraph::BreakCondition::Follow;
		prop.backgroundElementId = -1;
		prop.callbackId = callbackId;
		return prop;
	}
}
using namespace remote_document_paragrpah_tests;
```

## STEP 3: Implement Property Formatting Functions [DONE]

Add these functions in the same anonymous namespace:

```cpp
	WString FormatRunProperty(const DocumentTextRunProperty& prop)
	{
		return L"Text(color:" + 
			   itow(prop.textColor.r) + L"," + 
			   itow(prop.textColor.g) + L"," + 
			   itow(prop.textColor.b) + 
			   L", bg:" + 
			   itow(prop.backgroundColor.r) + L"," + 
			   itow(prop.backgroundColor.g) + L"," + 
			   itow(prop.backgroundColor.b) + L")";
	}

	WString FormatRunProperty(const DocumentInlineObjectRunProperty& prop)
	{
		return L"Inline(callback:" + 
			   itow(prop.callbackId) + 
			   L", size:" + itow(prop.size.x) + L"x" + itow(prop.size.y) + L")";
	}
```

## STEP 4: Implement Property Comparison Functions [DONE]

Add these functions in the same anonymous namespace:

```cpp
	bool CompareRunProperty(
		const DocumentTextRunProperty& a,
		const DocumentTextRunProperty& b)
	{
		return a.textColor == b.textColor &&
			   a.backgroundColor == b.backgroundColor &&
			   a.fontProperties.fontFamily == b.fontProperties.fontFamily &&
			   a.fontProperties.size == b.fontProperties.size &&
			   a.fontProperties.bold == b.fontProperties.bold &&
			   a.fontProperties.italic == b.fontProperties.italic &&
			   a.fontProperties.underline == b.fontProperties.underline &&
			   a.fontProperties.strikeline == b.fontProperties.strikeline &&
			   a.fontProperties.antialias == b.fontProperties.antialias &&
			   a.fontProperties.verticalAntialias == b.fontProperties.verticalAntialias;
	}

	bool CompareRunProperty(
		const DocumentInlineObjectRunProperty& a,
		const DocumentInlineObjectRunProperty& b)
	{
		return a.size == b.size &&
			   a.baseline == b.baseline &&
			   a.breakCondition == b.breakCondition &&
			   a.backgroundElementId == b.backgroundElementId &&
			   a.callbackId == b.callbackId;
	}
```

## STEP 5: Implement Map Printing Function [DONE]

Add this template function in the same anonymous namespace:

```cpp
	template<typename T>
	void PrintMap(
		const Dictionary<CaretRange, T>& actualMap,
		const Dictionary<CaretRange, T>& expectedMap)
	{
		TEST_PRINT(L"Actual: (" + itow(actualMap.Count()) + L")");
		for (vint i = 0; i < actualMap.Count(); i++)
		{
			auto key = actualMap.Keys()[i];
			auto value = actualMap.Values()[i];
			TEST_PRINT(L"  [" + itow(key.caretBegin) + L"," + itow(key.caretEnd) + L"]: " + FormatRunProperty(value));
		}
		
		TEST_PRINT(L"Expected: (" + itow(expectedMap.Count()) + L")");
		for (vint i = 0; i < expectedMap.Count(); i++)
		{
			auto key = expectedMap.Keys()[i];
			auto value = expectedMap.Values()[i];
			TEST_PRINT(L"  [" + itow(key.caretBegin) + L"," + itow(key.caretEnd) + L"]: " + FormatRunProperty(value));
		}
	}
```

## STEP 6: Implement Map Assertion Function [DONE]

Add this template function in the same anonymous namespace:

```cpp
	template<typename T>
	void AssertMap(
		const Dictionary<CaretRange, T>& actualMap,
		const Dictionary<CaretRange, T>& expectedMap)
	{
		bool matches = (actualMap.Count() == expectedMap.Count());
		
		if (matches)
		{
			for (vint i = 0; i < expectedMap.Count(); i++)
			{
				auto expectedKey = expectedMap.Keys()[i];
				auto expectedValue = expectedMap.Values()[i];
				
				// Check if key exists in actual map
				vint actualIndex = actualMap.Keys().IndexOf(expectedKey);
				if (actualIndex == -1)
				{
					matches = false;
					break;
				}
				
				// Check if value matches
				auto actualValue = actualMap.Values()[actualIndex];
				if (!CompareRunProperty(actualValue, expectedValue))
				{
					matches = false;
					break;
				}
			}
		}
		
		if (!matches)
		{
			PrintMap(actualMap, expectedMap);
		}
		TEST_ASSERT(matches);
	}
```

## STEP 7: Create CaretRange Comparison Tests [DONE]

Add this `TEST_FILE` block with the first `TEST_CATEGORY`:

```cpp
TEST_FILE
{
	TEST_CATEGORY(L"CaretRange")
	{
		TEST_CASE(L"Equal ranges")
		{
			CaretRange r1{10, 20};
			CaretRange r2{10, 20};
			TEST_ASSERT(r1 == r2);
			TEST_ASSERT(!(r1 < r2));
			TEST_ASSERT(!(r2 < r1));
		});

		TEST_CASE(L"Different caretBegin")
		{
			CaretRange r1{10, 20};
			CaretRange r2{15, 20};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
			TEST_ASSERT(!(r2 < r1));
		});

		TEST_CASE(L"Same caretBegin, different caretEnd")
		{
			CaretRange r1{10, 20};
			CaretRange r2{10, 25};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
			TEST_ASSERT(!(r2 < r1));
		});

		TEST_CASE(L"Adjacent ranges - non-overlapping")
		{
			CaretRange r1{10, 20};
			CaretRange r2{20, 30};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
		});

		TEST_CASE(L"Overlapping ranges")
		{
			CaretRange r1{10, 25};
			CaretRange r2{20, 30};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
		});

		TEST_CASE(L"Contained range")
		{
			CaretRange r1{10, 30};
			CaretRange r2{15, 25};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);  // r1.caretBegin < r2.caretBegin
		});
	});
}
```

## STEP 8: Create AddTextRun Basic Operation Tests [DONE]

Add this `TEST_CATEGORY` after the CaretRange category:

```cpp
	TEST_CATEGORY(L"AddTextRun")
	{
		TEST_CASE(L"Add to empty map")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			
			AddTextRun(textMap, {0, 10}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 10}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Add non-overlapping runs")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {20, 30}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 10}, prop1);
			expectedMap.Add({20, 30}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Adjacent runs with different properties - no merge")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {10, 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 10}, prop1);
			expectedMap.Add({10, 20}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Adjacent runs with identical properties - merge")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {10, 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 20}, prop1);
			
			AssertMap(textMap, expectedMap);
		});
	});
```

## STEP 9: Create AddTextRun Splitting Tests [DONE]

Add these test cases to the `AddTextRun` category:

```cpp
		TEST_CASE(L"Complete overlap - replacement")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {10, 20}, prop1);
			AddTextRun(textMap, {10, 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({10, 20}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Partial overlap from left")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {10, 30}, prop1);
			AddTextRun(textMap, {5, 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({5, 20}, prop2);
			expectedMap.Add({20, 30}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Partial overlap from right")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {10, 30}, prop1);
			AddTextRun(textMap, {20, 35}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({10, 20}, prop1);
			expectedMap.Add({20, 35}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"New run completely contains old run")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {15, 25}, prop1);
			AddTextRun(textMap, {10, 30}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({10, 30}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"New run contained within old run - split into 3")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {10, 40}, prop1);
			AddTextRun(textMap, {20, 30}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({10, 20}, prop1);
			expectedMap.Add({20, 30}, prop2);
			expectedMap.Add({30, 40}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Overlap multiple consecutive runs")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(120);
			auto prop3 = CreateTextProp(140);
			auto propNew = CreateTextProp(200);
			
			AddTextRun(textMap, {10, 20}, prop1);
			AddTextRun(textMap, {20, 30}, prop2);
			AddTextRun(textMap, {30, 40}, prop3);
			AddTextRun(textMap, {15, 35}, propNew);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({10, 15}, prop1);
			expectedMap.Add({15, 35}, propNew);
			expectedMap.Add({35, 40}, prop3);
			
			AssertMap(textMap, expectedMap);
		});
```

## STEP 10: Create AddTextRun Merging Tests [DONE]

Add these test cases to the `AddTextRun` category:

```cpp
		TEST_CASE(L"Insert between two identical runs - all three merge")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {20, 30}, prop1);
			AddTextRun(textMap, {10, 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 30}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Merge left neighbor only")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {20, 30}, prop2);
			AddTextRun(textMap, {10, 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 20}, prop1);
			expectedMap.Add({20, 30}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Merge right neighbor only")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {0, 10}, prop2);
			AddTextRun(textMap, {20, 30}, prop1);
			AddTextRun(textMap, {10, 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 10}, prop2);
			expectedMap.Add({10, 30}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"No merge when neighbors differ")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			auto prop3 = CreateTextProp(200);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {20, 30}, prop3);
			AddTextRun(textMap, {10, 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 10}, prop1);
			expectedMap.Add({10, 20}, prop2);
			expectedMap.Add({20, 30}, prop3);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Merge after splitting creates merged regions")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {0, 50}, prop1);
			AddTextRun(textMap, {20, 30}, prop2);
			AddTextRun(textMap, {20, 30}, prop1);  // Replace prop2 with prop1
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 50}, prop1);  // All merged back
			
			AssertMap(textMap, expectedMap);
		});
```

# TEST PLAN

## Test Category 1: CaretRange Comparison

### Test Cases

1. **Equal ranges**: Verify `==` returns true and `<` returns false for identical ranges
2. **Different caretBegin**: Verify ordering by `caretBegin` when it differs
3. **Same caretBegin, different caretEnd**: Verify ordering by `caretEnd` when `caretBegin` is equal
4. **Adjacent ranges - non-overlapping**: Verify ranges that touch but don't overlap compare correctly
5. **Overlapping ranges**: Verify ranges that overlap compare correctly
6. **Contained range**: Verify a range fully contained within another compares correctly

## Test Category 2: AddTextRun Operations

### Test Cases - Basic Operations

1. **Add to empty map**: Verify first insertion works
2. **Add non-overlapping runs**: Verify multiple non-overlapping insertions coexist
3. **Adjacent runs with different properties**: Verify different properties prevent merging
4. **Adjacent runs with identical properties**: Verify identical properties trigger merging

### Test Cases - Splitting Scenarios

5. **Complete overlap - replacement**: Verify exact range match replaces old property
6. **Partial overlap from left**: Verify new run overlapping from left splits the old run
7. **Partial overlap from right**: Verify new run overlapping from right splits the old run
8. **New run completely contains old run**: Verify larger new run replaces smaller old run
9. **New run contained within old run**: Verify new run splits old run into three parts (before, new, after)
10. **Overlap multiple consecutive runs**: Verify new run can split and replace multiple runs

### Test Cases - Merging Scenarios

11. **Insert between two identical runs**: Verify all three merge into one
12. **Merge left neighbor only**: Verify merging with left but not right when right differs
13. **Merge right neighbor only**: Verify merging with right but not left when left differs
14. **No merge when neighbors differ**: Verify no merging occurs when all properties differ
15. **Merge after splitting creates merged regions**: Verify split-then-replace can re-merge

# FIXING ATTEMPTS

## Fixing attempt No.3

### Why the previous fix didn't fully work

The iterator invalidation bug was fixed, but there were two more bugs:

1. **Binary search doesn't guarantee finding the FIRST match**: When multiple entries match (return `equal` in the comparer), binary search returns ANY of them, not necessarily the first. This caused the splitting loop to miss earlier overlapping entries.

2. **Reference invalidation in merge loops**: The merge code used `auto&&` references to keys (`leftKey`, `currentKey`, `rightKey`). After calling `map.Remove()`, these references became invalid, causing undefined behavior and duplicate entries.

### What I did

1. **Fix binary search result**: After binary search finds an overlap, scan backwards to find the actual first overlapping entry:
```cpp
if (firstOverlap != -1)
{
    while (firstOverlap > 0)
    {
        auto&& prevKey = map.Keys()[firstOverlap - 1];
        if (prevKey.caretEnd <= range.caretBegin || prevKey.caretBegin >= range.caretEnd)
            break;
        firstOverlap--;
    }
}
```

2. **Fix reference invalidation**: Changed `auto&&` to value copies in merge loops:
```cpp
// Before: auto&& leftKey = map.Keys()[newIndex - 1];
// After:
CaretRange leftKey = map.Keys()[newIndex - 1];
CaretRange currentKey = map.Keys()[newIndex];
```

### Result

All 21 test cases now pass:
- **Build**: 0 Warning(s) and 0 Error(s)
- **Tests**: Passed test files: 1/1, Passed test cases: 21/21

The tests successfully exposed three critical bugs in the `AddTextRun` implementation, which have all been fixed.

## Fixing attempt No.2

### Why the tests failed

The unit tests exposed two critical bugs in the `AddTextRun` implementation in `GuiRemoteGraphics_Document.cpp`:

1. **Iterator invalidation bug**: The code iterates over `map.Keys()` while calling `map.Remove(key)` inside the loop. After the first `Remove()`, the keys array reference becomes invalid, causing undefined behavior.

2. **Index tracking bug**: In the merge loops, after calling `map.Remove()` and `map.Add()`, the `newIndex` variable becomes stale because the dictionary's internal ordering changes. The code doesn't recalculate the index of the merged range.

These bugs cause:
- "Overlap multiple consecutive runs" test: Wrong split points due to iterator invalidation
- "Insert between two identical runs" test: Duplicate entries due to stale index
- "Merge left neighbor only" test: Same duplicate entry issue
- "Merge after splitting creates merged regions" test: Same duplicate entry issue

### What I need to do

Fix both bugs in `GuiRemoteGraphics_Document.cpp`:

1. **Fix iterator invalidation**: Collect all keys to remove in a list first, then remove them in a separate loop
2. **Fix index tracking**: After each merge operation, recalculate `newIndex` by finding the merged range in the keys

### Why I think it would solve the test failures

- Fixing the iterator invalidation will ensure the splitting loop correctly processes all overlapping runs and creates the right fragments
- Fixing the index tracking will ensure the merge loops correctly combine adjacent identical runs without creating duplicates
- The tests are correctly written - they're exposing real bugs in the implementation

## Fixing attempt No.1

### Why the original change did not work

The test file `TestRemote_DocumentRunManagement.cpp` had three major issues:

1. **Missing include**: `CaretRange` is defined in `GuiRemoteGraphics_Document.h` but that header was not included
2. **Wrong enum value**: Used `IGuiGraphicsParagraph::BreakCondition::Follow` but the correct value is `IGuiGraphicsParagraph::BreakCondition::Alone` (the enum only has StickToPreviousRun, StickToNextRun, and Alone)
3. **Aggregate initialization syntax**: The code used `CaretRange{10, 20}` aggregate initialization which failed to compile, needing designated initializers `{.caretBegin = 10, .caretEnd = 20}`

### What I need to do

1. Add `#include "../../../Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.h"` to include the header where `CaretRange` and related functions are declared
2. Change `prop.breakCondition = IGuiGraphicsParagraph::BreakCondition::Follow;` to `prop.breakCondition = IGuiGraphicsParagraph::BreakCondition::Alone;`
3. Replace all occurrences of `{x, y}` with `{.caretBegin = x, .caretEnd = y}` for CaretRange initialization throughout the file

### Why I think it would solve the build break

- Adding the missing header will make `CaretRange`, `DocumentTextRunPropertyMap`, `DocumentInlineObjectRunPropertyMap`, and the `AddTextRun` function visible to the compiler
- Using the correct enum value `Alone` instead of the non-existent `Follow` will resolve the enum error
- Using designated initializers is the correct C++20 syntax for aggregate initialization with default member initializers, which CaretRange has

### Result

After applying all three fixes, the build succeeded with **6 Warning(s) and 0 Error(s)**. The warnings are about `vl::vint` to `unsigned char` conversions in the `CreateTextProp` function, which are expected and acceptable for this test code.

# !!!FINISHED!!!

# !!!VERIFIED!!!

# Comparing to User Edit

After carefully comparing `Copilot_Execution.md` with the final source code, I found the following user edits that were not caused by fixing attempts or updates:

## Critical Bug Fixes in AddTextRun

### 1. Binary Search Result Handling (GuiRemoteGraphics_Document.cpp)

**What was changed:**
After the binary search finds the first overlapping entry, the user added code to scan backwards to find the ACTUAL first overlapping entry:

```cpp
// Binary search may return any overlapping entry, scan backwards to find the first one
if (firstOverlap != -1)
{
    while (firstOverlap > 0)
    {
        auto&& prevKey = map.Keys()[firstOverlap - 1];
        if (prevKey.caretEnd <= range.caretBegin || prevKey.caretBegin >= range.caretEnd)
            break;
        firstOverlap--;
    }
}
```

**Why this matters:**
Binary search with a custom comparer that returns `equal` for any overlapping range does NOT guarantee finding the first match - it can return ANY matching entry. This is a fundamental property of binary search that I missed. When there are multiple consecutive overlapping entries, the binary search might return a middle one, causing the splitting loop to miss earlier overlaps and produce incorrect results.

**Lesson learned:** When using binary search with a range-based comparer (returning `equal` for overlaps), always scan backwards to find the first match. The standard binary search algorithm only guarantees finding *a* match, not the *first* match.

### 2. Reference Invalidation in Merge Loops (GuiRemoteGraphics_Document.cpp)

**What was changed:**
In both merge loops (left and right neighbors), the user changed `auto&&` references to value copies:

```cpp
// Before (my buggy code):
// auto&& leftKey = map.Keys()[newIndex - 1];
// auto&& currentKey = map.Keys()[newIndex];

// After (user's fix):
CaretRange leftKey = map.Keys()[newIndex - 1];
CaretRange currentKey = map.Keys()[newIndex];
```

**Why this matters:**
After calling `map.Remove(key)`, the internal key array is modified and reallocated. Any references (`auto&&`) to keys become invalid (dangling references). Using these invalidated references causes undefined behavior. By copying the key values before removal, we avoid referencing the potentially modified array.

**Lesson learned:** When iterating over a container and modifying it (especially with Remove/Add operations), never hold references to container elements. Always copy the values if you need them after modification. This is especially critical with Dictionary operations where internal reordering happens.

### 3. Aggregate Initialization Syntax (TestRemote_DocumentRunManagement.cpp)

**What was changed:**
Changed all CaretRange initializations from simple brace initialization to designated initializers:

```cpp
// Before (my code):
CaretRange r1{10, 20};

// After (user's fix):
CaretRange r1{.caretBegin = 10, .caretEnd = 20};
```

**Why this matters:**
The `CaretRange` struct likely has default member initializers or the compiler requires explicit member naming for aggregate initialization in C++20. Simple brace initialization `{10, 20}` failed to compile, requiring the designated initializer syntax.

**Lesson learned:** In modern C++ (C++20), especially with structs that have default member initializers, always use designated initializers for clarity and to avoid compilation errors. The syntax `{.member = value}` is more explicit and prevents accidental reordering issues.

### 4. Enum Value Correction (TestRemote_DocumentRunManagement.cpp)

**What was changed:**
```cpp
// Before:
prop.breakCondition = IGuiGraphicsParagraph::BreakCondition::Follow;

// After:
prop.breakCondition = IGuiGraphicsParagraph::BreakCondition::Alone;
```

**Why this matters:**
The `BreakCondition` enum doesn't have a `Follow` value. I assumed it existed without checking the actual enum definition. The correct value for a standalone inline object is `Alone`.

**Lesson learned:** Always verify enum values exist before using them. Don't assume based on naming patterns - check the actual definition.

## Summary of Key Learnings

1. **Binary Search Semantics**: Binary search with custom comparers that return `equal` for ranges doesn't guarantee finding the first match. Always post-process to find the first match when needed.

2. **Reference Invalidation**: When modifying containers during iteration, NEVER hold references to container elements. Always copy values before modification operations that might reallocate or reorder the container.

3. **Modern C++ Syntax**: In C++20, use designated initializers `{.member = value}` for aggregate types, especially when they have default member initializers or when compiler requires explicit naming.

4. **Verify Before Using**: Always verify enum values, API signatures, and data structures exist as expected. Don't assume based on patterns or memory - check the actual definitions.

These bugs were all caught by the comprehensive unit tests, demonstrating the value of thorough test coverage. The tests exposed edge cases (multiple consecutive overlaps, merge-after-split scenarios) that revealed these fundamental implementation bugs.
