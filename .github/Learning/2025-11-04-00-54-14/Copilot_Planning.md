# !!!PLANNING!!!

# UPDATES

## UPDATE

Remove all comment block for titles.
Remove all comment in header file code changes.

## UPDATE

Mark that this task does not do unit test, but you can keep the TEST PLAN for future references.

# IMPROVEMENT PLAN

## STEP 1: Define CaretRange Struct

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

**Why this change is necessary**:
- CaretRange provides type-safe representation of caret position ranges, preventing confusion with generic integer pairs
- The `operator<=>` with `=default` leverages C++20 to automatically generate all six comparison operators with correct lexicographic ordering
- This ordering (by `caretBegin` first, then `caretEnd`) is crucial for Dictionary's sorted property and enables efficient overlap detection
- Default member initialization ensures safe usage even with partial initialization

## STEP 2: Define Type Aliases for Run Maps

Add the type aliases immediately after the `CaretRange` struct definition in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`:

```cpp
	using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, DocumentTextRunProperty>;
	using DocumentInlineObjectRunPropertyMap = collections::Dictionary<CaretRange, DocumentInlineObjectRunProperty>;
	using DocumentRunPropertyMap = collections::Dictionary<CaretRange, DocumentRunProperty>;
```

**Why this change is necessary**:
- Separating text runs and inline object runs at the type level provides type safety and prevents mixing operations
- Each map type reflects different semantic rules: text runs can be split, inline objects cannot
- The unified `DocumentRunPropertyMap` is only used after merging, matching the protocol's transmission model
- Type aliases improve code readability and make the intent explicit

## STEP 3: Implement AddTextRun Function

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

**Why this change is necessary**:
- Text runs require overlapping keys to be split, maintaining the invariant that Dictionary keys never overlap
- `BinarySearchLambda` with custom comparer provides O(log n) overlap detection instead of O(n) linear scan
- The custom comparer handles the fact that overlapping ranges don't form total ordering by defining three regions
- After insertion, consecutive runs with identical properties are merged to minimize protocol transmission overhead
- The equality operator for `DocumentTextRunProperty` is necessary for merge detection
- Iterative merging handles transitive cases where multiple consecutive runs have identical properties

## STEP 4: Implement AddInlineObjectRun Function

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

**Why this change is necessary**:
- Inline objects cannot be split and must remain atomic
- Any overlap with existing inline objects is an error condition that must be rejected
- Using `BinarySearchLambda` provides O(log n) overlap detection for efficiency
- Return value allows caller to handle the error condition appropriately
- Since inline objects cannot overlap, we only need to detect if ANY overlap exists, not collect all overlaps

## STEP 5: Implement ResetInlineObjectRun Function

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

**Why this change is necessary**:
- Inline objects are identified by their exact range (key)
- Dictionary's `Remove()` method requires exact key match and returns success/failure status
- This function is essentially a type-safe wrapper that enforces the "no partial matches" semantic
- The implementation is simple because Dictionary already provides the needed functionality

## STEP 6: Implement MergeRuns Function

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

**Why this change is necessary**:
- Two separate maps must be combined into a unified view for protocol transmission
- Inline objects have priority over text runs when they overlap
- Two-pointer merge algorithm provides O(n + m) optimal complexity by leveraging sorted order
- The algorithm naturally handles text runs that span multiple inline objects by tracking partially consumed text runs
- Each position is processed exactly once, similar to merge sort's merge phase
- Using `DocumentRunProperty` variant type allows storing both run types in the result map

## STEP 7: Implement DiffRuns Function

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

**Why this change is necessary**:
- Protocol needs to know what changed to minimize data transmission
- Two-pointer merge provides O(n + m) optimal complexity by leveraging sorted key order
- The algorithm naturally handles key changes (splits, merges) by treating them as additions when keys don't match
- Critical requirement: `runsDiff` contains only complete keys from `newRuns`, never partial or mixed keys
- Inline object tracking by `callbackId` handles the case where an inline object is moved to a different position
- `SortedList` for tracking callbackIds ensures efficient lookup when computing differences
- Equality operator for `DocumentRunProperty` handles variant comparison correctly
- The result represents the new state - consumers can apply these changes to update their view

# TEST PLAN

**NOTE**: This task does not include unit test implementation. The test plan below is kept for future reference when tests are to be added.

## Test Coverage Strategy

The test plan will comprehensively validate all functions with focus on:
1. **Correctness**: All operations produce expected results
2. **Edge cases**: Empty inputs, boundary conditions, extreme values
3. **Performance**: Algorithms use optimal complexity
4. **Integration**: Functions work together correctly

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
		// Tests for CaretRange struct
	});

	TEST_CATEGORY(L"AddTextRun")
	{
		// Tests for AddTextRun function
	});

	TEST_CATEGORY(L"AddInlineObjectRun")
	{
		// Tests for AddInlineObjectRun function
	});

	TEST_CATEGORY(L"ResetInlineObjectRun")
	{
		// Tests for ResetInlineObjectRun function
	});

	TEST_CATEGORY(L"MergeRuns")
	{
		// Tests for MergeRuns function
	});

	TEST_CATEGORY(L"DiffRuns")
	{
		// Tests for DiffRuns function
	});
}
```

## Test Cases

### CaretRange Tests

**Test 1: Comparison Operators**
- Verify `operator<=>` generates all comparison operators correctly
- Test lexicographic ordering: `{0,5} < {0,10}` and `{0,10} < {5,10}`
- Test equality: `{5,10} == {5,10}` and `{5,10} != {5,11}`
- Test all six operators: `==`, `!=`, `<`, `<=`, `>`, `>=`

**Test 2: Default Initialization**
- Verify default values are `{0, 0}`
- Test that partially initialized structs work correctly

**Test 3: Dictionary Key Usage**
- Create Dictionary with CaretRange keys
- Verify sorted order in Dictionary
- Verify lookup by exact key

### AddTextRun Tests

**Test 1: Empty Map**
- Add a run to empty map
- Verify the run exists with correct properties

**Test 2: Non-Overlapping Runs**
- Add runs `[0,5]`, `[10,15]`, `[20,25]`
- Verify all runs exist independently
- Verify no merging occurs (different properties)

**Test 3: Consecutive Runs with Same Property**
- Add `[0,5]` with red color
- Add `[5,10]` with red color
- Verify they merge into `[0,10]`
- Add `[10,15]` with red color
- Verify further merge to `[0,15]`

**Test 4: Overlapping - New Run Splits Existing**
- Add `[0,20]` with red
- Add `[5,15]` with blue
- Verify result: `[0,5]` red, `[5,15]` blue, `[15,20]` red

**Test 5: Overlapping - New Run Contains Existing**
- Add `[5,10]` with red
- Add `[0,20]` with blue
- Verify result: `[0,20]` blue (existing run completely replaced)

**Test 6: Overlapping - Multiple Runs**
- Add `[0,10]`, `[10,20]`, `[20,30]` with red
- Add `[5,25]` with blue
- Verify result: `[0,5]` red, `[5,25]` blue, `[25,30]` red

**Test 7: Complex Merging**
- Add `[0,10]` with red
- Add `[20,30]` with red
- Add `[10,20]` with red
- Verify final result: `[0,30]` red (all three merged)

**Test 8: Property Equality**
- Verify runs with same text color, background color, and font merge
- Verify runs with different text color don't merge
- Verify runs with different background color don't merge
- Verify runs with different font properties don't merge

### AddInlineObjectRun Tests

**Test 1: Empty Map**
- Add an inline object to empty map
- Verify it returns `true` and exists

**Test 2: Non-Overlapping Inline Objects**
- Add `[0,5]`, `[10,15]`, `[20,25]`
- Verify all return `true` and exist independently

**Test 3: Exact Overlap Rejection**
- Add `[10,20]`
- Try to add `[10,20]` again
- Verify second add returns `false`

**Test 4: Partial Overlap Rejection - Left**
- Add `[10,20]`
- Try to add `[5,15]`
- Verify returns `false`

**Test 5: Partial Overlap Rejection - Right**
- Add `[10,20]`
- Try to add `[15,25]`
- Verify returns `false`

**Test 6: Partial Overlap Rejection - Contains**
- Add `[10,20]`
- Try to add `[12,18]`
- Verify returns `false`

**Test 7: Partial Overlap Rejection - Contained By**
- Add `[12,18]`
- Try to add `[10,20]`
- Verify returns `false`

**Test 8: Adjacent Runs OK**
- Add `[0,10]`
- Add `[10,20]`
- Verify both return `true` (not overlapping)

### ResetInlineObjectRun Tests

**Test 1: Remove Existing Run**
- Add `[10,20]`
- Reset `[10,20]`
- Verify returns `true` and run is removed

**Test 2: Remove Non-Existing Run**
- Reset `[10,20]` from empty map
- Verify returns `false`

**Test 3: Exact Match Required**
- Add `[10,20]`
- Try to reset `[10,25]`
- Verify returns `false` (range doesn't match exactly)
- Verify original run still exists

**Test 4: Multiple Runs**
- Add `[0,10]`, `[10,20]`, `[20,30]`
- Reset `[10,20]`
- Verify returns `true` and only middle run is removed
- Verify other runs remain

### MergeRuns Tests

**Test 1: Empty Maps**
- Merge two empty maps
- Verify result is empty

**Test 2: Only Text Runs**
- Text: `[0,10]`, `[10,20]`, `[20,30]`
- Inline: empty
- Verify result contains all text runs as DocumentRunProperty with text variant

**Test 3: Only Inline Objects**
- Text: empty
- Inline: `[0,10]`, `[10,20]`, `[20,30]`
- Verify result contains all inline objects as DocumentRunProperty with inline variant

**Test 4: Non-Overlapping Text and Inline**
- Text: `[0,10]`, `[20,30]`
- Inline: `[10,20]`
- Verify result: all three runs in correct order with correct types

**Test 5: Inline Object Cuts Text Run - Beginning**
- Text: `[0,30]` red
- Inline: `[0,10]`
- Verify result: `[0,10]` inline, `[10,30]` text red

**Test 6: Inline Object Cuts Text Run - Middle**
- Text: `[0,30]` red
- Inline: `[10,20]`
- Verify result: `[0,10]` text red, `[10,20]` inline, `[20,30]` text red

**Test 7: Inline Object Cuts Text Run - End**
- Text: `[0,30]` red
- Inline: `[20,30]`
- Verify result: `[0,20]` text red, `[20,30]` inline

**Test 8: Multiple Inline Objects in Single Text Run**
- Text: `[0,50]` red
- Inline: `[10,15]`, `[30,35]`
- Verify result: `[0,10]` text, `[10,15]` inline, `[15,30]` text, `[30,35]` inline, `[35,50]` text

**Test 9: Complex Interleaving**
- Text: `[0,10]` red, `[15,25]` blue, `[30,40]` red
- Inline: `[10,15]`, `[25,30]`
- Verify correct interleaved result with proper types

### DiffRuns Tests

**Test 1: Empty Maps**
- oldRuns: empty
- newRuns: empty
- Verify `runsDiff`, `createdInlineObjects`, `removedInlineObjects` are all empty

**Test 2: All New Runs**
- oldRuns: empty
- newRuns: `[0,10]` text red, `[10,20]` inline
- Verify `runsDiff` contains both runs
- Verify `createdInlineObjects` contains the inline object callbackId

**Test 3: All Removed Runs**
- oldRuns: `[0,10]` text red, `[10,20]` inline
- newRuns: empty
- Verify `runsDiff` is empty (no new runs to report)
- Verify `removedInlineObjects` contains the inline object callbackId

**Test 4: Identical Maps**
- oldRuns: `[0,10]` text red
- newRuns: `[0,10]` text red
- Verify all diff arrays are empty (no changes)

**Test 5: Property Changed**
- oldRuns: `[0,10]` text red
- newRuns: `[0,10]` text blue
- Verify `runsDiff` contains `[0,10]` text blue
- Verify inline object arrays are empty

**Test 6: Run Split**
- oldRuns: `[0,20]` text red
- newRuns: `[0,10]` text red, `[10,20]` text blue
- Verify `runsDiff` contains both `[0,10]` and `[10,20]` (complete keys from newRuns)
- This tests the critical requirement that result uses complete keys from newRuns

**Test 7: Runs Merged**
- oldRuns: `[0,10]` text red, `[10,20]` text red
- newRuns: `[0,20]` text red
- Verify `runsDiff` contains `[0,20]` (key doesn't exist in oldRuns)

**Test 8: Inline Object Added**
- oldRuns: `[0,30]` text red
- newRuns: `[0,10]` text red, `[10,20]` inline id=42, `[20,30]` text red
- Verify `runsDiff` contains all three runs
- Verify `createdInlineObjects` contains id 42

**Test 9: Inline Object Removed**
- oldRuns: `[0,10]` text red, `[10,20]` inline id=42, `[20,30]` text red
- newRuns: `[0,30]` text red
- Verify `runsDiff` contains `[0,30]`
- Verify `removedInlineObjects` contains id 42

**Test 10: Inline Object Moved**
- oldRuns: `[0,10]` inline id=42, `[10,20]` text red
- newRuns: `[0,10]` text red, `[10,20]` inline id=42
- Verify `runsDiff` contains both runs
- Verify inline object arrays are empty (same callbackId exists in both)

**Test 11: Multiple Changes**
- oldRuns: `[0,10]` text red, `[10,20]` inline id=1, `[20,30]` text blue
- newRuns: `[0,15]` text red, `[15,25]` inline id=2, `[25,35]` text blue
- Verify all runs in `runsDiff` have keys from newRuns
- Verify inline object changes tracked correctly

**Test 12: Complex Split and Merge**
- oldRuns: `[0,5]` text A, `[5,10]` text A, `[10,15]` text B
- newRuns: `[0,10]` text A, `[10,12]` text C, `[12,15]` text B
- Verify `runsDiff` correctly represents the changes with complete newRuns keys

## Integration Tests

**Integration Test 1: Full Workflow**
1. Create empty text and inline object maps
2. Add multiple text runs with AddTextRun
3. Add multiple inline objects with AddInlineObjectRun
4. Merge maps with MergeRuns
5. Modify runs and create second merged map
6. Compare with DiffRuns
7. Verify entire pipeline works correctly

**Integration Test 2: Paragraph Update Simulation**
1. Simulate initial paragraph state with text and inline objects
2. Simulate user edits (text changes, inline object insertions)
3. Compute diff
4. Verify diff can be applied to reconstruct new state

## Performance Tests

**Performance Test 1: Large Map Operations**
- Create map with 10,000 runs
- Measure AddTextRun performance with BinarySearchLambda
- Verify O(log n) behavior

**Performance Test 2: MergeRuns Scaling**
- Create large text and inline object maps (1,000 runs each)
- Measure MergeRuns performance
- Verify O(n + m) linear behavior

**Performance Test 3: DiffRuns Scaling**
- Create large old and new maps (1,000 runs each)
- Measure DiffRuns performance
- Verify O(n + m) linear behavior

## Existing Test Review

Before implementing new tests, review existing tests in:
- `Test\GacUISrc\UnitTest\TestRemote*.cpp`
- Any paragraph-related tests

Determine if existing tests need updates or if they already cover some scenarios.

# !!!FINISHED!!!
