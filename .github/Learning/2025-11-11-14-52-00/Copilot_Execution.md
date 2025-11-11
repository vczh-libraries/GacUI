# !!!EXECUTION!!!

# UPDATES

# IMPROVEMENT PLAN

## STEP 1: Add Helper Function `ApplyOverrides` [DONE]

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

## STEP 2: Track Overlapping Old Runs in `AddTextRun` [DONE]

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

## STEP 3: Decompose New Range with Override Semantics [DONE]

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

## STEP 4: Verify Merging Logic Remains Unchanged [DONE]

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

# TEST PLAN

## Test Strategy

The task explicitly states: "Do NOT modify test cases - they should continue to pass". Verify the implementation by running existing tests without modifications.

## Expected Test Results

**Success criteria**:
- Build succeeds without compilation errors
- All existing test cases pass
- No regressions in observable behavior

# FIXING ATTEMPTS

# !!!FINISHED!!!

# !!!VERIFIED!!!
