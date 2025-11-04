# !!!EXECUTION!!!

# UPDATES

# IMPROVEMENT PLAN

## STEP 1: Define CaretRange Struct [DONE]

Add the `CaretRange` struct after the namespace declaration and before the `GuiRemoteGraphicsParagraph` class in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`:

```cpp
namespace vl::presentation::remoteprotocol
{
	struct CaretRange
	{
		vint caretBegin = 0;
		vint caretEnd = 0;

		auto operator<=>(const CaretRange&) const = default;
	};
}
```

## STEP 2: Define Type Aliases for Run Maps [DONE]

Add the type aliases immediately after the `CaretRange` struct definition in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`:

```cpp
	using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, DocumentTextRunProperty>;
	using DocumentInlineObjectRunPropertyMap = collections::Dictionary<CaretRange, DocumentInlineObjectRunProperty>;
	using DocumentRunPropertyMap = collections::Dictionary<CaretRange, DocumentRunProperty>;
```

## STEP 3: Implement AddTextRun Function [DONE]

Add the declaration in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`:

```cpp
	extern void AddTextRun(
		DocumentTextRunPropertyMap& map,
		CaretRange range,
		const DocumentTextRunProperty& property);
```

Add the implementation in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`:

```cpp
namespace vl::presentation::remoteprotocol
{
	bool operator==(const DocumentTextRunProperty& a, const DocumentTextRunProperty& b)
	{
		return a.textColor == b.textColor &&
			   a.backgroundColor == b.backgroundColor &&
			   a.fontProperties == b.fontProperties;
	}

	void AddTextRun(
		DocumentTextRunPropertyMap& map,
		CaretRange range,
		const DocumentTextRunProperty& property)
	{
		auto comparer = [&](const Pair<CaretRange, DocumentTextRunProperty>& item) -> vint
		{
			if (item.key.caretEnd <= range.caretBegin)
				return -1;
			else if (item.key.caretBegin >= range.caretEnd)
				return 1;
			else
				return 0;
		};

		vint firstOverlap = collections::BinarySearchLambda(map.Keys(), comparer);

		List<Pair<CaretRange, DocumentTextRunProperty>> fragmentsToReinsert;
		
		if (firstOverlap != -1)
		{
			auto&& keys = map.Keys();
			for (vint i = firstOverlap; i < keys.Count(); i++)
			{
				auto&& key = keys[i];
				if (key.caretBegin >= range.caretEnd)
					break;

				auto&& oldProperty = map[key];

				if (key.caretBegin < range.caretBegin)
				{
					CaretRange beforeRange{ key.caretBegin, range.caretBegin };
					fragmentsToReinsert.Add({ beforeRange, oldProperty });
				}

				if (key.caretEnd > range.caretEnd)
				{
					CaretRange afterRange{ range.caretEnd, key.caretEnd };
					fragmentsToReinsert.Add({ afterRange, oldProperty });
				}

				map.Remove(key);
			}
		}

		for (auto&& fragment : fragmentsToReinsert)
		{
			map.Add(fragment.key, fragment.value);
		}

		map.Add(range, property);

		vint newIndex = map.Keys().IndexOf(range);
		
		while (newIndex > 0)
		{
			auto&& leftKey = map.Keys()[newIndex - 1];
			auto&& currentKey = map.Keys()[newIndex];
			
			if (leftKey.caretEnd == currentKey.caretBegin &&
				map[leftKey] == map[currentKey])
			{
				CaretRange mergedRange{ leftKey.caretBegin, currentKey.caretEnd };
				auto mergedProperty = map[leftKey];
				
				map.Remove(leftKey);
				map.Remove(currentKey);
				map.Add(mergedRange, mergedProperty);
				
				newIndex--;
			}
			else
			{
				break;
			}
		}

		while (newIndex < map.Keys().Count() - 1)
		{
			auto&& currentKey = map.Keys()[newIndex];
			auto&& rightKey = map.Keys()[newIndex + 1];
			
			if (currentKey.caretEnd == rightKey.caretBegin &&
				map[currentKey] == map[rightKey])
			{
				CaretRange mergedRange{ currentKey.caretBegin, rightKey.caretEnd };
				auto mergedProperty = map[currentKey];
				
				map.Remove(currentKey);
				map.Remove(rightKey);
				map.Add(mergedRange, mergedProperty);
			}
			else
			{
				break;
			}
		}
	}
}
```

## STEP 4: Implement AddInlineObjectRun Function [DONE]

Add the declaration in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`:

```cpp
	extern bool AddInlineObjectRun(
		DocumentInlineObjectRunPropertyMap& map,
		CaretRange range,
		const DocumentInlineObjectRunProperty& property);
```

Add the implementation in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`:

```cpp
	bool AddInlineObjectRun(
		DocumentInlineObjectRunPropertyMap& map,
		CaretRange range,
		const DocumentInlineObjectRunProperty& property)
	{
		auto comparer = [&](const Pair<CaretRange, DocumentInlineObjectRunProperty>& item) -> vint
		{
			if (item.key.caretEnd <= range.caretBegin)
				return -1;
			else if (item.key.caretBegin >= range.caretEnd)
				return 1;
			else
				return 0;
		};

		vint firstOverlap = collections::BinarySearchLambda(map.Keys(), comparer);
		
		if (firstOverlap != -1)
			return false;

		map.Add(range, property);
		return true;
	}
```

## STEP 5: Implement ResetInlineObjectRun Function [DONE]

Add the declaration in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`:

```cpp
	extern bool ResetInlineObjectRun(
		DocumentInlineObjectRunPropertyMap& map,
		CaretRange range);
```

Add the implementation in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`:

```cpp
	bool ResetInlineObjectRun(
		DocumentInlineObjectRunPropertyMap& map,
		CaretRange range)
	{
		return map.Remove(range);
	}
```

## STEP 6: Implement MergeRuns Function [DONE]

Add the declaration in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`:

```cpp
	extern void MergeRuns(
		const DocumentTextRunPropertyMap& textRuns,
		const DocumentInlineObjectRunPropertyMap& inlineObjectRuns,
		DocumentRunPropertyMap& result);
```

Add the implementation in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`:

```cpp
	void MergeRuns(
		const DocumentTextRunPropertyMap& textRuns,
		const DocumentInlineObjectRunPropertyMap& inlineObjectRuns,
		DocumentRunPropertyMap& result)
	{
		result.Clear();

		vint textIdx = 0;
		vint inlineIdx = 0;

		auto&& textKeys = textRuns.Keys();
		auto&& inlineKeys = inlineObjectRuns.Keys();

		CaretRange currentTextRange;
		DocumentTextRunProperty currentTextProperty;
		bool hasCurrentText = false;

		while (textIdx < textKeys.Count() || inlineIdx < inlineKeys.Count() || hasCurrentText)
		{
			if (!hasCurrentText && textIdx < textKeys.Count())
			{
				currentTextRange = textKeys[textIdx];
				currentTextProperty = textRuns[currentTextRange];
				hasCurrentText = true;
				textIdx++;
			}

			if (!hasCurrentText && inlineIdx < inlineKeys.Count())
			{
				auto&& inlineKey = inlineKeys[inlineIdx];
				DocumentRunProperty runProp = inlineObjectRuns[inlineKey];
				result.Add(inlineKey, runProp);
				inlineIdx++;
				continue;
			}

			if (hasCurrentText && inlineIdx >= inlineKeys.Count())
			{
				DocumentRunProperty runProp = currentTextProperty;
				result.Add(currentTextRange, runProp);
				hasCurrentText = false;
				continue;
			}

			if (hasCurrentText && inlineIdx < inlineKeys.Count())
			{
				auto&& inlineKey = inlineKeys[inlineIdx];

				if (currentTextRange.caretEnd <= inlineKey.caretBegin)
				{
					DocumentRunProperty runProp = currentTextProperty;
					result.Add(currentTextRange, runProp);
					hasCurrentText = false;
				}
				else if (inlineKey.caretEnd <= currentTextRange.caretBegin)
				{
					DocumentRunProperty runProp = inlineObjectRuns[inlineKey];
					result.Add(inlineKey, runProp);
					inlineIdx++;
				}
				else
				{
					if (currentTextRange.caretBegin < inlineKey.caretBegin)
					{
						CaretRange beforeRange{ currentTextRange.caretBegin, inlineKey.caretBegin };
						DocumentRunProperty runProp = currentTextProperty;
						result.Add(beforeRange, runProp);
					}

					DocumentRunProperty runProp = inlineObjectRuns[inlineKey];
					result.Add(inlineKey, runProp);
					inlineIdx++;

					if (currentTextRange.caretEnd > inlineKey.caretEnd)
					{
						currentTextRange.caretBegin = inlineKey.caretEnd;
					}
					else
					{
						hasCurrentText = false;
					}
				}
			}
		}
	}
```

## STEP 7: Implement DiffRuns Function [DONE]

Add the declaration in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`:

```cpp
	extern void DiffRuns(
		const DocumentRunPropertyMap& oldRuns,
		const DocumentRunPropertyMap& newRuns,
		ElementDesc_DocumentParagraph& result);
```

Add the implementation in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`:

```cpp
	bool operator==(const DocumentRunProperty& a, const DocumentRunProperty& b)
	{
		if (a.GetType() != b.GetType())
			return false;

		if (auto textA = a.TryGet<DocumentTextRunProperty>())
		{
			auto textB = b.Get<DocumentTextRunProperty>();
			return *textA == textB;
		}
		else
		{
			auto inlineA = a.Get<DocumentInlineObjectRunProperty>();
			auto inlineB = b.Get<DocumentInlineObjectRunProperty>();
			return inlineA.size == inlineB.size &&
				   inlineA.baseline == inlineB.baseline &&
				   inlineA.breakCondition == inlineB.breakCondition &&
				   inlineA.backgroundElementId == inlineB.backgroundElementId &&
				   inlineA.callbackId == inlineB.callbackId;
		}
	}

	void DiffRuns(
		const DocumentRunPropertyMap& oldRuns,
		const DocumentRunPropertyMap& newRuns,
		ElementDesc_DocumentParagraph& result)
	{
		result.runsDiff = Ptr(new List<DocumentRun>());
		result.createdInlineObjects = Ptr(new List<vint>());
		result.removedInlineObjects = Ptr(new List<vint>());

		collections::SortedList<vint> oldInlineCallbackIds;
		collections::SortedList<vint> newInlineCallbackIds;

		vint oldIdx = 0;
		vint newIdx = 0;

		auto&& oldKeys = oldRuns.Keys();
		auto&& newKeys = newRuns.Keys();

		while (oldIdx < oldKeys.Count() || newIdx < newKeys.Count())
		{
			if (oldIdx >= oldKeys.Count())
			{
				auto&& newKey = newKeys[newIdx];
				auto&& newValue = newRuns[newKey];

				DocumentRun run;
				run.caretBegin = newKey.caretBegin;
				run.caretEnd = newKey.caretEnd;
				run.props = newValue;
				result.runsDiff->Add(run);

				if (auto inlineObj = newValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					newInlineCallbackIds.Add(inlineObj->callbackId);
				}

				newIdx++;
				continue;
			}

			if (newIdx >= newKeys.Count())
			{
				auto&& oldKey = oldKeys[oldIdx];
				auto&& oldValue = oldRuns[oldKey];

				if (auto inlineObj = oldValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					oldInlineCallbackIds.Add(inlineObj->callbackId);
				}

				oldIdx++;
				continue;
			}

			auto&& oldKey = oldKeys[oldIdx];
			auto&& newKey = newKeys[newIdx];

			if (oldKey == newKey)
			{
				auto&& oldValue = oldRuns[oldKey];
				auto&& newValue = newRuns[newKey];

				if (auto inlineObj = oldValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					oldInlineCallbackIds.Add(inlineObj->callbackId);
				}
				if (auto inlineObj = newValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					newInlineCallbackIds.Add(inlineObj->callbackId);
				}

				if (!(oldValue == newValue))
				{
					DocumentRun run;
					run.caretBegin = newKey.caretBegin;
					run.caretEnd = newKey.caretEnd;
					run.props = newValue;
					result.runsDiff->Add(run);
				}

				oldIdx++;
				newIdx++;
			}
			else if (newKey < oldKey)
			{
				auto&& newValue = newRuns[newKey];

				DocumentRun run;
				run.caretBegin = newKey.caretBegin;
				run.caretEnd = newKey.caretEnd;
				run.props = newValue;
				result.runsDiff->Add(run);

				if (auto inlineObj = newValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					newInlineCallbackIds.Add(inlineObj->callbackId);
				}

				newIdx++;
			}
			else
			{
				auto&& oldValue = oldRuns[oldKey];

				if (auto inlineObj = oldValue.TryGet<DocumentInlineObjectRunProperty>())
				{
					oldInlineCallbackIds.Add(inlineObj->callbackId);
				}

				oldIdx++;
			}
		}

		for (vint i = 0; i < newInlineCallbackIds.Count(); i++)
		{
			auto id = newInlineCallbackIds[i];
			if (!oldInlineCallbackIds.Contains(id))
			{
				result.createdInlineObjects->Add(id);
			}
		}

		for (vint i = 0; i < oldInlineCallbackIds.Count(); i++)
		{
			auto id = oldInlineCallbackIds[i];
			if (!newInlineCallbackIds.Contains(id))
			{
				result.removedInlineObjects->Add(id);
			}
		}
	}
```

# TEST PLAN

**NOTE**: This task does not include unit test implementation. The test plan below is kept for future reference when tests are to be added.

## Test Coverage Strategy

The test plan will comprehensively validate all functions with focus on:
1. Correctness: All operations produce expected results
2. Edge cases: Empty inputs, boundary conditions, extreme values
3. Performance: Algorithms use optimal complexity
4. Integration: Functions work together correctly

For refactoring work, existing test cases should already cover most scenarios. We need to:
1. Review existing paragraph-related tests
2. Add tests specifically for the new run management functions
3. Ensure corner cases are covered

## Unit Test Structure

Tests should be organized in: `Test\GacUISrc\UnitTest\TestRemote_Protocol_Document.cpp`

```cpp
TEST_FILE
{
	TEST_CATEGORY(L"CaretRange")
	{
	});

	TEST_CATEGORY(L"AddTextRun")
	{
	});

	TEST_CATEGORY(L"AddInlineObjectRun")
	{
	});

	TEST_CATEGORY(L"ResetInlineObjectRun")
	{
	});

	TEST_CATEGORY(L"MergeRuns")
	{
	});

	TEST_CATEGORY(L"DiffRuns")
	{
	});
}
```

## Test Cases

### CaretRange Tests

**Test 1: Comparison Operators**

Verify `operator<=>` generates all comparison operators correctly.
Test lexicographic ordering: `{0,5} < {0,10}` and `{0,10} < {5,10}`.
Test equality: `{5,10} == {5,10}` and `{5,10} != {5,11}`.
Test all six operators: `==`, `!=`, `<`, `<=`, `>`, `>=`.

**Test 2: Default Initialization**

Verify default values are `{0, 0}`.
Test that partially initialized structs work correctly.

**Test 3: Dictionary Key Usage**

Create Dictionary with CaretRange keys.
Verify sorted order in Dictionary.
Verify lookup by exact key.

### AddTextRun Tests

**Test 1: Empty Map**

Add a run to empty map.
Verify the run exists with correct properties.

**Test 2: Non-Overlapping Runs**

Add runs `[0,5]`, `[10,15]`, `[20,25]`.
Verify all runs exist independently.
Verify no merging occurs (different properties).

**Test 3: Consecutive Runs with Same Property**

Add `[0,5]` with red color.
Add `[5,10]` with red color.
Verify they merge into `[0,10]`.
Add `[10,15]` with red color.
Verify further merge to `[0,15]`.

**Test 4: Overlapping - New Run Splits Existing**

Add `[0,20]` with red.
Add `[5,15]` with blue.
Verify result: `[0,5]` red, `[5,15]` blue, `[15,20]` red.

**Test 5: Overlapping - New Run Contains Existing**

Add `[5,10]` with red.
Add `[0,20]` with blue.
Verify result: `[0,20]` blue (existing run completely replaced).

**Test 6: Overlapping - Multiple Runs**

Add `[0,10]`, `[10,20]`, `[20,30]` with red.
Add `[5,25]` with blue.
Verify result: `[0,5]` red, `[5,25]` blue, `[25,30]` red.

**Test 7: Complex Merging**

Add `[0,10]` with red.
Add `[20,30]` with red.
Add `[10,20]` with red.
Verify final result: `[0,30]` red (all three merged).

**Test 8: Property Equality**

Verify runs with same text color, background color, and font merge.
Verify runs with different text color don't merge.
Verify runs with different background color don't merge.
Verify runs with different font properties don't merge.

### AddInlineObjectRun Tests

**Test 1: Empty Map**

Add an inline object to empty map.
Verify it returns `true` and exists.

**Test 2: Non-Overlapping Inline Objects**

Add `[0,5]`, `[10,15]`, `[20,25]`.
Verify all return `true` and exist independently.

**Test 3: Exact Overlap Rejection**

Add `[10,20]`.
Try to add `[10,20]` again.
Verify second add returns `false`.

**Test 4: Partial Overlap Rejection - Left**

Add `[10,20]`.
Try to add `[5,15]`.
Verify returns `false`.

**Test 5: Partial Overlap Rejection - Right**

Add `[10,20]`.
Try to add `[15,25]`.
Verify returns `false`.

**Test 6: Partial Overlap Rejection - Contains**

Add `[10,20]`.
Try to add `[12,18]`.
Verify returns `false`.

**Test 7: Partial Overlap Rejection - Contained By**

Add `[12,18]`.
Try to add `[10,20]`.
Verify returns `false`.

**Test 8: Adjacent Runs OK**

Add `[0,10]`.
Add `[10,20]`.
Verify both return `true` (not overlapping).

### ResetInlineObjectRun Tests

**Test 1: Remove Existing Run**

Add `[10,20]`.
Reset `[10,20]`.
Verify returns `true` and run is removed.

**Test 2: Remove Non-Existing Run**

Reset `[10,20]` from empty map.
Verify returns `false`.

**Test 3: Exact Match Required**

Add `[10,20]`.
Try to reset `[10,25]`.
Verify returns `false` (range doesn't match exactly).
Verify original run still exists.

**Test 4: Multiple Runs**

Add `[0,10]`, `[10,20]`, `[20,30]`.
Reset `[10,20]`.
Verify returns `true` and only middle run is removed.
Verify other runs remain.

### MergeRuns Tests

**Test 1: Empty Maps**

Merge two empty maps.
Verify result is empty.

**Test 2: Only Text Runs**

Text: `[0,10]`, `[10,20]`, `[20,30]`.
Inline: empty.
Verify result contains all text runs as DocumentRunProperty with text variant.

**Test 3: Only Inline Objects**

Text: empty.
Inline: `[0,10]`, `[10,20]`, `[20,30]`.
Verify result contains all inline objects as DocumentRunProperty with inline variant.

**Test 4: Non-Overlapping Text and Inline**

Text: `[0,10]`, `[20,30]`.
Inline: `[10,20]`.
Verify result: all three runs in correct order with correct types.

**Test 5: Inline Object Cuts Text Run - Beginning**

Text: `[0,30]` red.
Inline: `[0,10]`.
Verify result: `[0,10]` inline, `[10,30]` text red.

**Test 6: Inline Object Cuts Text Run - Middle**

Text: `[0,30]` red.
Inline: `[10,20]`.
Verify result: `[0,10]` text red, `[10,20]` inline, `[20,30]` text red.

**Test 7: Inline Object Cuts Text Run - End**

Text: `[0,30]` red.
Inline: `[20,30]`.
Verify result: `[0,20]` text red, `[20,30]` inline.

**Test 8: Multiple Inline Objects in Single Text Run**

Text: `[0,50]` red.
Inline: `[10,15]`, `[30,35]`.
Verify result: `[0,10]` text, `[10,15]` inline, `[15,30]` text, `[30,35]` inline, `[35,50]` text.

**Test 9: Complex Interleaving**

Text: `[0,10]` red, `[15,25]` blue, `[30,40]` red.
Inline: `[10,15]`, `[25,30]`.
Verify correct interleaved result with proper types.

### DiffRuns Tests

**Test 1: Empty Maps**

oldRuns: empty.
newRuns: empty.
Verify `runsDiff`, `createdInlineObjects`, `removedInlineObjects` are all empty.

**Test 2: All New Runs**

oldRuns: empty.
newRuns: `[0,10]` text red, `[10,20]` inline.
Verify `runsDiff` contains both runs.
Verify `createdInlineObjects` contains the inline object callbackId.

**Test 3: All Removed Runs**

oldRuns: `[0,10]` text red, `[10,20]` inline.
newRuns: empty.
Verify `runsDiff` is empty (no new runs to report).
Verify `removedInlineObjects` contains the inline object callbackId.

**Test 4: Identical Maps**

oldRuns: `[0,10]` text red.
newRuns: `[0,10]` text red.
Verify all diff arrays are empty (no changes).

**Test 5: Property Changed**

oldRuns: `[0,10]` text red.
newRuns: `[0,10]` text blue.
Verify `runsDiff` contains `[0,10]` text blue.
Verify inline object arrays are empty.

**Test 6: Run Split**

oldRuns: `[0,20]` text red.
newRuns: `[0,10]` text red, `[10,20]` text blue.
Verify `runsDiff` contains both `[0,10]` and `[10,20]` (complete keys from newRuns).

**Test 7: Runs Merged**

oldRuns: `[0,10]` text red, `[10,20]` text red.
newRuns: `[0,20]` text red.
Verify `runsDiff` contains `[0,20]` (key doesn't exist in oldRuns).

**Test 8: Inline Object Added**

oldRuns: `[0,30]` text red.
newRuns: `[0,10]` text red, `[10,20]` inline id=42, `[20,30]` text red.
Verify `runsDiff` contains all three runs.
Verify `createdInlineObjects` contains id 42.

**Test 9: Inline Object Removed**

oldRuns: `[0,10]` text red, `[10,20]` inline id=42, `[20,30]` text red.
newRuns: `[0,30]` text red.
Verify `runsDiff` contains `[0,30]`.
Verify `removedInlineObjects` contains id 42.

**Test 10: Inline Object Moved**

oldRuns: `[0,10]` inline id=42, `[10,20]` text red.
newRuns: `[0,10]` text red, `[10,20]` inline id=42.
Verify `runsDiff` contains both runs.
Verify inline object arrays are empty (same callbackId exists in both).

**Test 11: Multiple Changes**

oldRuns: `[0,10]` text red, `[10,20]` inline id=1, `[20,30]` text blue.
newRuns: `[0,15]` text red, `[15,25]` inline id=2, `[25,35]` text blue.
Verify all runs in `runsDiff` have keys from newRuns.
Verify inline object changes tracked correctly.

**Test 12: Complex Split and Merge**

oldRuns: `[0,5]` text A, `[5,10]` text A, `[10,15]` text B.
newRuns: `[0,10]` text A, `[10,12]` text C, `[12,15]` text B.
Verify `runsDiff` correctly represents the changes with complete newRuns keys.

## Integration Tests

**Integration Test 1: Full Workflow**

Create empty text and inline object maps.
Add multiple text runs with AddTextRun.
Add multiple inline objects with AddInlineObjectRun.
Merge maps with MergeRuns.
Modify runs and create second merged map.
Compare with DiffRuns.
Verify entire pipeline works correctly.

**Integration Test 2: Paragraph Update Simulation**

Simulate initial paragraph state with text and inline objects.
Simulate user edits (text changes, inline object insertions).
Compute diff.
Verify diff can be applied to reconstruct new state.

## Performance Tests

**Performance Test 1: Large Map Operations**

Create map with 10,000 runs.
Measure AddTextRun performance with BinarySearchLambda.
Verify O(log n) behavior.

**Performance Test 2: MergeRuns Scaling**

Create large text and inline object maps (1,000 runs each).
Measure MergeRuns performance.
Verify O(n + m) linear behavior.

**Performance Test 3: DiffRuns Scaling**

Create large old and new maps (1,000 runs each).
Measure DiffRuns performance.
Verify O(n + m) linear behavior.

## Existing Test Review

Before implementing new tests, review existing tests in `Test\GacUISrc\UnitTest\TestRemote*.cpp` and any paragraph-related tests.

Determine if existing tests need updates or if they already cover some scenarios.

# FIXING ATTEMPTS

## Fixing attempt No.1

**Why the original change did not work:**
1. `BinarySearchLambda` was called incorrectly - it requires 5 parameters (buffer, count, item, index reference, orderer) but I only passed 2 (collection and comparer).
2. `GetType()` doesn't exist on `Variant` - the correct method is `Index()` which returns the index of the currently active variant type.

**What needs to be done:**
1. Fix `BinarySearchLambda` calls to use the correct signature with proper buffer pointer, count, dummy search item, index out parameter, and the comparer function.
2. Replace `GetType()` with `Index()` in the DocumentRunProperty comparison operator.

**Why this should solve the build break:**
These are the exact APIs documented in `Import/Vlpp.h`. The `BinarySearchLambda` function at line 2452 shows the correct signature, and the `Index()` method at line 6798 is the correct way to get the variant's active type index.

## UPDATE

User feedback: The previous fix attempt was reverted. The correct approach is:
- Run binary search directly on `Keys()` collection (not on the map items)
- Use `&Keys()[0]` to get the pointer to the key buffer
- Handle empty map case - `&Keys()[0]` will crash if there are no keys
- Still must call `BinarySearchLambda` per the instruction

## Fixing attempt No.2 [DONE]

**Why the original change did not work:**
The previous attempt was calling `BinarySearchLambda` with incorrect parameters - passing the entire map's key-value pairs instead of just the keys buffer.

**What needs to be done:**
1. For `AddTextRun`: Search only on keys (CaretRange objects) using `&map.Keys()[0]` as the buffer pointer
2. For `AddInlineObjectRun`: Same approach - search on keys only
3. Handle empty map case - check `map.Count() == 0` before accessing `&map.Keys()[0]`
4. The comparer lambda should take `(const CaretRange& bufferItem, const CaretRange& searchItem)` and return a `std::strong_ordering`
5. Fix `GetType()` to `Index()` in DocumentRunProperty comparison

**Why this should solve the build break:**
- `BinarySearchLambda` signature is: `vint BinarySearchLambda(const T* buffer, vint count, const K& item, vint& index, F&& orderer)`
- We search on keys: buffer = `&map.Keys()[0]`, count = `map.Keys().Count()`, item = `range` (the CaretRange we're searching for)
- The orderer compares a key from the buffer with the search range to determine overlap
- This matches the exact API signature and performs binary search correctly on the sorted keys

**Result:**
The code now compiles successfully. Changes made:
1. `AddTextRun`: Now checks for empty map, then uses `BinarySearchLambda(&map.Keys()[0], map.Keys().Count(), range, index, comparer)` with proper ordering return type
2. `AddInlineObjectRun`: Same pattern applied
3. `DocumentRunProperty` comparison: Changed `GetType()` to `Index()`

# !!!FINISHED!!!

# Comparing to User Edit

## User Edit Found: Namespace Location Change

**What was changed:**
The execution document planned to place `CaretRange`, type aliases, and run management functions in the `vl::presentation::remoteprotocol` namespace (as shown in Step 1 of the execution plan).

However, the actual implementation places these in the `vl::presentation::elements` namespace instead.

**Location of change:**
- File: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`
- The definitions appear under `namespace vl::presentation::elements` (after the `DiffRuns` comment)
- All function implementations in the `.cpp` file are also in the `elements` namespace

**Rationale for user's choice:**
This is a sensible architectural decision because:
1. `GuiRemoteGraphicsParagraph` class is already declared in the `elements` namespace
2. The run management functions are tightly coupled to paragraph rendering, which is an element concern
3. Protocol types (like `DocumentTextRunProperty`, `DocumentRunProperty`) remain in `remoteprotocol` namespace and are referenced with the `remoteprotocol::` prefix where needed
4. This separation maintains clear boundaries: protocol schema types stay in `remoteprotocol`, while paragraph-specific helper logic stays in `elements`

**Impact:**
- The function signatures use `remoteprotocol::` prefixes for protocol types (e.g., `const remoteprotocol::DocumentTextRunProperty& property`)
- This is correct and maintains type safety across namespace boundaries
- The change doesn't affect functionality, only code organization

**Learning:**
When implementing features that bridge multiple subsystems:
- Consider which namespace best represents the primary responsibility
- Helper functions should be placed close to their primary usage site
- Use explicit namespace prefixes when crossing namespace boundaries for clarity
- The `elements` namespace is appropriate for rendering-related utilities, even when they interact with protocol types

````
