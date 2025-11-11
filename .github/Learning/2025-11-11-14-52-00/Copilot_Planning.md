# !!!PLANNING!!!

# UPDATES

(No updates yet - this is the initial planning phase)

# IMPROVEMENT PLAN

## STEP 1: Add Helper Function `ApplyOverrides`

**What to change**:

Add a new helper function `ApplyOverrides` in `GuiRemoteGraphics_Document.cpp`, right after the `AreEqual` comparison functions and before `AddTextRun`:

```cpp
DocumentTextRunPropertyOverrides ApplyOverrides(
	const DocumentTextRunPropertyOverrides& base,
	const DocumentTextRunPropertyOverrides& overrides)
{
	DocumentTextRunPropertyOverrides result;
	result.textColor = overrides.textColor ? overrides.textColor : base.textColor;
	result.backgroundColor = overrides.backgroundColor ? overrides.backgroundColor : base.backgroundColor;
	result.fontFamily = overrides.fontFamily ? overrides.fontFamily : base.fontFamily;
	result.size = overrides.size ? overrides.size : base.size;
	result.textStyle = overrides.textStyle ? overrides.textStyle : base.textStyle;
	return result;
}
```

**Why this change is necessary**:

This function implements the core override semantic: "if a property in the new run is null, keep the existing value; otherwise, use the new value". The ternary operator `overrides.prop ? overrides.prop : base.prop` uses Nullable<T>'s implicit bool conversion to check if a property is defined. If the override property is defined (non-null), it's used; otherwise, the base property is preserved.

This encapsulates the nullable property override logic in a single reusable function, making the main `AddTextRun` logic cleaner and easier to understand. It directly implements the requirement from the task description: "If a property in the new run is null: Keep the existing run's property value (don't change it)".

## STEP 2: Track Overlapping Old Runs in `AddTextRun`

**What to change**:

In the `AddTextRun` function, add a new list to track overlapping old runs and their overlap ranges. Modify the existing loop that processes overlapping runs:

```cpp
void AddTextRun(
	DocumentTextRunPropertyMap& map,
	CaretRange range,
	const DocumentTextRunPropertyOverrides& propertyOverrides)
{
	// ... existing overlap detection code unchanged ...
	
	List<Pair<CaretRange, DocumentTextRunPropertyOverrides>> fragmentsToReinsert;
	List<CaretRange> keysToRemove;
	List<Tuple<CaretRange, DocumentTextRunPropertyOverrides>> overlappingOldRuns;  // NEW
	
	if (firstOverlap != -1)
	{
		auto&& keys = map.Keys();
		for (vint i = firstOverlap; i < keys.Count(); i++)
		{
			auto&& key = keys[i];
			if (key.caretBegin >= range.caretEnd)
				break;
		
			auto&& oldProperty = map[key];
			
			// NEW: Record the overlapping portion
			vint overlapBegin = key.caretBegin < range.caretBegin ? range.caretBegin : key.caretBegin;
			vint overlapEnd = key.caretEnd > range.caretEnd ? range.caretEnd : key.caretEnd;
			overlappingOldRuns.Add({CaretRange{overlapBegin, overlapEnd}, oldProperty});
		
			// ... rest of existing code unchanged (beforeRange, afterRange, keysToRemove) ...
		}
		
		// ... existing removal code unchanged ...
	}
	
	// ... existing fragment reinsertion code unchanged ...
	
	// (The next step will replace the simple map.Add(range, propertyOverrides) here)
```

**Why this change is necessary**:

We need to remember which parts of the new range overlapped with existing runs, and what the old properties were in those overlapping regions. This information is essential for applying the override semantics correctly.

The overlap calculation `max(key.caretBegin, range.caretBegin)` to `min(key.caretEnd, range.caretEnd)` correctly identifies the intersection between each old run and the new run's range. By storing both the overlap range and the old property, we can later apply the merged properties only to the overlapping portions while leaving non-overlapping portions to use the new properties as-is.

## STEP 3: Decompose New Range with Override Semantics

**What to change**:

Replace the simple `map.Add(range, propertyOverrides)` line in `AddTextRun` with logic that decomposes the new range into subranges:

```cpp
	// ... (after fragment reinsertion code) ...
	
	// Replace: map.Add(range, propertyOverrides);
	// With:
	
	if (overlappingOldRuns.Count() > 0)
	{
		// The new range has overlaps with old runs - apply override semantics
		vint currentPos = range.caretBegin;
		
		for (vint i = 0; i < overlappingOldRuns.Count(); i++)
		{
			auto&& [oldRange, oldProp] = overlappingOldRuns[i];
			
			// Add any gap before this overlap with new properties as-is
			if (currentPos < oldRange.caretBegin)
			{
				map.Add(CaretRange{currentPos, oldRange.caretBegin}, propertyOverrides);
			}
			
			// Add the overlapping portion with merged properties
			auto mergedProp = ApplyOverrides(oldProp, propertyOverrides);
			map.Add(oldRange, mergedProp);
			
			currentPos = oldRange.caretEnd;
		}
		
		// Add any remaining part after all overlaps with new properties as-is
		if (currentPos < range.caretEnd)
		{
			map.Add(CaretRange{currentPos, range.caretEnd}, propertyOverrides);
		}
	}
	else
	{
		// No overlaps - add the entire range with new properties
		map.Add(range, propertyOverrides);
	}
	
	// ... existing merge logic unchanged ...
```

**Why this change is necessary**:

This is the core implementation of the override semantics. The new range can consist of multiple subranges:

1. **Gaps between/before/after overlaps**: These parts of the new range don't overlap any existing runs, so they should use the new properties directly (including any null values).

2. **Overlapping portions**: These parts overlap with existing runs, so we must apply the override semantic - use `ApplyOverrides` to merge the old and new properties, where null in the new run means "keep the old value".

The algorithm walks through the new range from left to right, identifying gaps and overlaps. For gaps, it adds the new properties unchanged. For overlaps, it computes merged properties using `ApplyOverrides` and adds those. This ensures every position in the new range is covered exactly once with the correct properties.

The example from the task description illustrates this perfectly:
- Range `(0,10)` exists with `{Arial, 12, Black, White}`
- Add range `(5,15)` with `{Times, null, null, null}`
- Result:
  - `(0,5)`: Unchanged (beforeRange fragment)
  - `(5,10)`: Overlap - merge gives `{Times, 12, Black, White}` (Times replaces Arial, nulls keep old values)
  - `(10,15)`: Gap beyond old run - use new properties `{Times, null, null, null}` as-is

## STEP 4: Verify Merging Logic Remains Unchanged

**What to verify**:

Ensure that the existing merging logic at the end of `AddTextRun` remains completely unchanged:

```cpp
	vint newIndex = map.Keys().IndexOf(range);
	
	while (newIndex > 0)
	{
		CaretRange leftKey = map.Keys()[newIndex - 1];
		CaretRange currentKey = map.Keys()[newIndex];
		
		if (leftKey.caretEnd == currentKey.caretBegin &&
			AreEqual(map[leftKey], map[currentKey]))
		{
			// ... merge left ...
		}
		else
		{
			break;
		}
	}

	while (newIndex < map.Keys().Count() - 1)
	{
		CaretRange currentKey = map.Keys()[newIndex];
		CaretRange rightKey = map.Keys()[newIndex + 1];
		
		if (currentKey.caretEnd == rightKey.caretBegin &&
			AreEqual(map[currentKey], map[rightKey]))
		{
			// ... merge right ...
		}
		else
		{
			break;
		}
	}
```

**Why this is important**:

The task explicitly states: "Keep merging logic unchanged (from Task No.9)". The merging logic consolidates consecutive runs with identical properties. This includes:
- Two runs with `color=null` can merge
- Two runs with `color=Red` can merge
- A run with `color=null` and a run with `color=Red` cannot merge

The `AreEqual` function already handles nullable comparison correctly (from Task No.9), so no changes are needed here. The merging logic will work correctly with the new override semantics because after applying overrides, we still end up with runs that have specific property values, and consecutive runs with matching values will be merged as before.

# TEST PLAN

## Test Strategy

The task explicitly states: "Do NOT modify test cases - they should continue to pass". This means we verify the implementation by running existing tests without modifications.

## Why Existing Tests Are Sufficient

**Reasoning**:

1. **All existing tests use fully-defined properties** (all properties are non-null)
2. **With fully-defined properties, override behavior is identical to replacement**: When the new run has all properties defined, `ApplyOverrides(old, new)` always returns `new` because the ternary `overrides.prop ? overrides.prop : base.prop` always picks `overrides.prop` (since it's never null)
3. **Therefore, existing tests validate backward compatibility**: If all existing tests pass, it proves that the override logic doesn't break the existing behavior for non-null properties

## Expected Test Results

**Success criteria**:
- Build succeeds without compilation errors
- All existing test cases pass
- No regressions in observable behavior

**Test categories that should pass**:
- All test cases in the unit test suite that exercise `AddTextRun`
- Tests that add runs with various property combinations
- Tests that verify merging of consecutive identical runs
- Tests that verify range splitting and fragmentation

## Corner Cases Already Covered

The existing tests likely cover these scenarios (which work identically with override semantics when properties are non-null):

1. **Non-overlapping runs**: No overlaps means `overlappingOldRuns.Count() == 0`, so the else branch executes and adds the entire range with new properties - identical to before
2. **Complete overlap**: Old run `(0,10)`, add `(0,10)` - the entire old run is replaced with merged properties (which equals new properties when new is fully defined)
3. **Partial overlap at beginning**: Old run `(5,15)`, add `(0,10)` - creates beforeRange, overlap, and afterRange fragments
4. **Partial overlap at end**: Old run `(0,10)`, add `(5,15)` - similar fragmentation
5. **Multiple overlapping runs**: Old runs `(0,10)` and `(10,20)`, add `(5,15)` - processes each overlap separately
6. **Merging after insertion**: Consecutive runs with identical properties get merged - still works because `AreEqual` compares all properties including nullability

## New Test Cases Not Needed in This Task

The task states: "The overriding logic only affects cases where properties are null, which existing tests don't use". 

Test cases specifically for nullable properties will be added in Task No.11 and No.12, which will cover:
- Adding runs with some null properties
- Verifying that null properties preserve existing values
- Verifying that defined properties override existing values
- Complex scenarios with mixed null and non-null properties

By deferring nullable-specific tests to later tasks, we maintain a clean incremental approach where each task builds on validated previous work.

# !!!FINISHED!!!
