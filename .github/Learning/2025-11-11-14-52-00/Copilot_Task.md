# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.10: Implement nullable property overriding logic in AddTextRun and default application in MergeRuns

### description

Now that nullable properties can be stored and compared (Task No.9), implement the actual overriding semantics: in AddTextRun, null means "keep existing value", and in MergeRuns, defaults are already applied.

**Critical note**: MergeRuns already applies defaults in Task No.9, so no changes needed there. This task only changes AddTextRun.

**Step 1: Update AddTextRun overriding logic**

When adding a new run that overlaps existing runs, the new run's properties should override:
- If a property in the new run is **defined** (non-null): Replace the existing run's property value
- If a property in the new run is **null**: Keep the existing run's property value (don't change it)

**Example scenario**:
- Existing map: `[(0,10) fontFamily="Arial", size=12, color=Black, bgColor=White]`
- Add run: `[(5,15) fontFamily="Times", color=null, bgColor=null, size=null]`
- Result:
  - `[(0,5) fontFamily="Arial", size=12, color=Black, bgColor=White]` - unchanged part
  - `[(5,10) fontFamily="Times", size=12, color=Black, bgColor=White]` - fontFamily updated, others kept
  - `[(10,15) fontFamily="Times", size=<null>, color=<null>, bgColor=<null>]` - new range beyond original

Note: The overriding only applies to existing ranges. For new ranges (like 10-15 above), just use the new run's properties as-is.

**Step 2: Keep merging logic unchanged**

The merging logic from Task No.9 remains:
- Two runs merge if all properties match exactly (including nullability)
- `color=null` can merge with `color=null`
- `color=null` cannot merge with `color=Red`

**Step 3: Implementation approach**

When splitting an existing run because of overlap with a new run:
1. Identify the overlapping fragment
2. For each property in the fragment:
   - If the new run's property is defined: Use the new value
   - If the new run's property is null: Use the existing fragment's value
3. Continue with merging logic as before

### what to be done

1. Update AddTextRun implementation to apply overriding semantics
2. When a new run overlaps existing runs, apply the "null = keep existing" rule
3. Keep merging logic unchanged (from Task No.9)
4. Do NOT modify test cases - they should continue to pass
5. Verify no regression in existing behavior

### how to test it

Run the compiled unit test executable. All existing test cases should pass:
- Tests with all properties defined (non-null) should behave identically to Task No.9
- The overriding logic only affects cases where properties are null, which existing tests don't use
- No new tests needed in this task (covered in Task No.11)

Success criteria:
- All existing test categories pass
- Build succeeds
- No regressions in observable behavior for non-null property cases

### file locations

Modified files:
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

### rationale

This task completes the nullable property implementation by adding the overriding semantics. By deferring this to the third task in the sequence, we ensure:
1. The type system changes are validated (Task No.8)
2. The nullable storage and comparison is validated (Task No.9)
3. Only then do we add the complex overriding logic (this task)

Since existing tests use all-defined properties, the overriding logic won't affect them - they'll continue to pass. The new behavior will be thoroughly tested in Task No.11 and No.12, which add test cases specifically for nullable property scenarios.

This incremental approach makes each step small and verifiable, avoiding the "disaster" of trying to do everything at once.

# UPDATES

# INSIGHTS AND REASONING

## Understanding the Current Implementation

I've reviewed the current `AddTextRun` implementation in `GuiRemoteGraphics_Document.cpp`. The current implementation (from Task No.9) has these characteristics:

1. **Overlap detection**: Uses binary search with a custom comparer to find overlapping runs
2. **Range splitting**: When the new run overlaps existing runs, it removes the overlapping runs and creates fragments:
   - `beforeRange`: Part of old run before overlap (if any)
   - `afterRange`: Part of old run after overlap (if any)
3. **Property handling**: Currently, when splitting, it just copies the old property values directly for the fragments
4. **Merging**: After insertion, merges consecutive runs with identical properties (using `AreEqual` which compares all fields including nullability)

## The Problem: Missing Override Logic

The current implementation treats the new run's property as a complete replacement - when it splits an existing run, it doesn't apply the "null = keep existing" semantic. Specifically:

**Current behavior** (Task No.9):
- When fragmenting an overlapping run, both `beforeRange` and `afterRange` fragments just copy `oldProperty` as-is
- The new run is inserted with its properties (including nulls) directly

**Required behavior** (Task No.10):
- `beforeRange` fragment: Should keep the complete old property (this is correct)
- The **overlapping portion**: Should apply the new run's properties where they are non-null, but keep the old run's properties where the new run has nulls
- `afterRange` fragment: Should keep the complete old property (this is correct)

## Design Solution

The key insight is that we need to create an **overlapping fragment** with merged properties when the new run overlaps an existing run within the existing run's range.

### Current Code Flow

```cpp
// Current code creates fragments like this:
if (key.caretBegin < range.caretBegin)
{
    // Before fragment - keeps old property (CORRECT)
    CaretRange beforeRange{ key.caretBegin, range.caretBegin };
    fragmentsToReinsert.Add({ beforeRange, oldProperty });
}

if (key.caretEnd > range.caretEnd)
{
    // After fragment - keeps old property (CORRECT)
    CaretRange afterRange{ range.caretEnd, key.caretEnd };
    fragmentsToReinsert.Add({ afterRange, oldProperty });
}
// Missing: overlapping fragment with merged properties
```

Then later:
```cpp
map.Add(range, propertyOverrides);  // Adds new run covering entire range
```

### Proposed Solution

The issue is that when we add the new run at `map.Add(range, propertyOverrides)`, it covers the entire range including parts that may overlap with existing runs. For those overlapping parts, we should have **merged** the properties.

**Strategy**: Before removing the old overlapping runs, we need to create an intermediate merged property for the overlapping portions. Here's the approach:

1. **For overlapping ranges that will be overwritten by the new run**:
   - Calculate the overlap region: `[max(key.caretBegin, range.caretBegin), min(key.caretEnd, range.caretEnd)]`
   - Create a merged property: For each field in the new run, use the new value if non-null, otherwise use the old run's value
   - This overlapping fragment will be part of what gets re-added later

2. **Modify the fragment generation logic**:
   ```cpp
   // For each overlapping existing run:
   if (key.caretBegin < range.caretBegin)
   {
       // Before: old property unchanged
       fragmentsToReinsert.Add({ {key.caretBegin, range.caretBegin}, oldProperty });
   }
   
   // Calculate overlap region
   vint overlapBegin = max(key.caretBegin, range.caretBegin);
   vint overlapEnd = min(key.caretEnd, range.caretEnd);
   
   // Create merged property for the overlap
   DocumentTextRunPropertyOverrides mergedProperty = ApplyOverrides(oldProperty, propertyOverrides);
   // (Store this for later - we'll add it after processing all overlaps)
   
   if (key.caretEnd > range.caretEnd)
   {
       // After: old property unchanged
       fragmentsToReinsert.Add({ {range.caretEnd, key.caretEnd}, oldProperty });
   }
   ```

3. **Helper function needed**:
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

4. **Final insertion**: Instead of just adding `propertyOverrides` for the entire range, we need to consider:
   - Parts of the range that didn't overlap anything: Use `propertyOverrides` as-is
   - Parts of the range that overlapped existing runs: Use merged properties

### Refined Approach: Simpler Implementation

Actually, looking more carefully at the code flow, I realize there's a simpler approach:

**The key realization**: The current code removes all overlapping runs and then adds the new run for the entire range. But what we should do is:

1. For portions of existing runs that overlap with the new run's range:
   - Create a merged property (override semantics)
   - These portions should be re-added with merged properties

2. For portions outside the new run's range:
   - Keep old properties unchanged

**Simplest implementation**: 

When building fragments to reinsert:
- `beforeRange` fragments: Use old property (already correct)
- `afterRange` fragments: Use old property (already correct)
- For the **overlapping middle portion** of each old run: Don't create a fragment - instead, we'll handle it after the loop

After removing old runs and reinserting fragments, instead of simply:
```cpp
map.Add(range, propertyOverrides);
```

We need to:
1. Identify which parts of `range` were covered by old runs
2. For those parts, compute merged properties and add them
3. For parts not covered by old runs, add with `propertyOverrides` directly

**However**, this gets complex with tracking covered regions. Let me reconsider...

### Final Simplified Solution

The simplest approach is to:

1. Keep the current fragment generation for `beforeRange` and `afterRange` (unchanged)
2. Add intermediate fragments for overlapping portions with merged properties
3. Do NOT add `map.Add(range, propertyOverrides)` at the end
4. Instead, let the merging logic handle consolidating the fragments

Wait, but we still need to handle the case where the new range extends beyond existing runs...

### Actual Correct Approach

After more thought, here's the correct and clean solution:

**The new run's range can be decomposed into subranges**:
- Subranges that overlap with existing runs → use merged properties
- Subranges that don't overlap with existing runs → use new properties as-is

**Implementation strategy**:

1. Build a list of all subranges within the new run's range
2. For each subrange, determine if it overlapped an old run:
   - If yes: Compute merged property
   - If no: Use new property
3. Add all subranges with their appropriate properties
4. Let the existing merge logic consolidate consecutive identical properties

**Concrete steps**:

```cpp
// After identifying overlapping runs, before removing them:
// Collect info about which parts of the new range overlap with old runs
List<Tuple<vint, vint, DocumentTextRunPropertyOverrides>> oldRunCoverage;
for each overlapping old run:
    vint overlapBegin = max(key.caretBegin, range.caretBegin);
    vint overlapEnd = min(key.caretEnd, range.caretEnd);
    oldRunCoverage.Add({overlapBegin, overlapEnd, oldProperty});

// Remove overlapping old runs and reinsert non-overlapping fragments
// (current logic for beforeRange and afterRange)

// Now add the new run's range in pieces:
vint currentPos = range.caretBegin;
for (auto&& coverage : oldRunCoverage)
{
    if (currentPos < coverage.f0)
    {
        // Gap before this old run - use new properties
        map.Add({currentPos, coverage.f0}, propertyOverrides);
    }
    
    // Overlapping part - use merged properties
    auto merged = ApplyOverrides(coverage.f2, propertyOverrides);
    map.Add({coverage.f0, coverage.f1}, merged);
    
    currentPos = coverage.f1;
}

if (currentPos < range.caretEnd)
{
    // Remaining part beyond all old runs - use new properties
    map.Add({currentPos, range.caretEnd}, propertyOverrides);
}

// Then run the existing merge logic
```

This is getting complex. Let me think of an even simpler approach...

### Simplest Working Solution

Looking at the code again, I see the fundamental issue: when we do `map.Add(range, propertyOverrides)`, we're adding a single entry for the entire range. Instead, we should:

**Modified approach**:
1. Keep all the overlap detection and fragmentation logic as-is
2. When creating fragments for the portions that will be covered by the new run, apply the merge logic
3. Don't add a single `map.Add(range, propertyOverrides)` at the end

**Actually**, I realize the cleanest approach:

The current code creates `fragmentsToReinsert` for before/after portions. We can extend this to also create fragments for the **overlapping middle portions** with merged properties. Then we don't add `map.Add(range, propertyOverrides)` at all - everything is handled through fragments.

**But**: We also need to handle parts of the new range that don't overlap anything.

### Final Clean Design

After all this analysis, here's the clearest solution:

**Current issue**: Line `map.Add(range, propertyOverrides)` adds properties for entire range without considering overlaps.

**Fix**: Instead of adding one entry for the entire range, decompose it into subranges:

1. **Collect overlap information** while processing existing runs:
   - For each overlapping old run, record: `{overlapBegin, overlapEnd, oldProperty}`
   
2. **Generate fragments for the new range**:
   - Walk through the new range from `range.caretBegin` to `range.caretEnd`
   - For each position, check if it was covered by an old run
   - If covered: Use merged property `ApplyOverrides(oldProp, newProp)`
   - If not covered: Use new property directly
   
3. **Add all fragments**, then run merge logic as before

The implementation can reuse the overlapping run list we already build in the first loop.

## Proposed Implementation

Here's the concrete change needed:

```cpp
void AddTextRun(
    DocumentTextRunPropertyMap& map,
    CaretRange range,
    const DocumentTextRunPropertyOverrides& propertyOverrides)
{
    // [Existing overlap detection code - unchanged]
    
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
            
            // Record the overlapping portion - NEW
            vint overlapBegin = key.caretBegin < range.caretBegin ? range.caretBegin : key.caretBegin;
            vint overlapEnd = key.caretEnd > range.caretEnd ? range.caretEnd : key.caretEnd;
            overlappingOldRuns.Add({CaretRange{overlapBegin, overlapEnd}, oldProperty});
        
            // [Rest of existing fragment generation - unchanged]
            if (key.caretBegin < range.caretBegin) { ... }
            if (key.caretEnd > range.caretEnd) { ... }
            keysToRemove.Add(key);
        }
        
        // [Existing removal code - unchanged]
    }

    // [Existing fragment reinsertion - unchanged]
    
    // NEW: Instead of map.Add(range, propertyOverrides), decompose into subranges
    if (overlappingOldRuns.Count() > 0)
    {
        // Sort overlapping runs by start position (should already be sorted)
        vint currentPos = range.caretBegin;
        
        for (auto&& [oldRange, oldProp] : overlappingOldRuns)
        {
            // Add gap before this overlap (if any) with new properties
            if (currentPos < oldRange.caretBegin)
            {
                map.Add(CaretRange{currentPos, oldRange.caretBegin}, propertyOverrides);
            }
            
            // Add overlap with merged properties
            auto mergedProp = ApplyOverrides(oldProp, propertyOverrides);
            map.Add(oldRange, mergedProp);
            
            currentPos = oldRange.caretEnd;
        }
        
        // Add remaining part after all overlaps (if any) with new properties
        if (currentPos < range.caretEnd)
        {
            map.Add(CaretRange{currentPos, range.caretEnd}, propertyOverrides);
        }
    }
    else
    {
        // No overlaps - add entire range with new properties
        map.Add(range, propertyOverrides);
    }
    
    // [Existing merge logic - unchanged]
}

// NEW helper function
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

## Verification Against Requirements

Let's verify this meets the requirements from the task description:

**Example scenario**:
- Existing map: `[(0,10) fontFamily="Arial", size=12, color=Black, bgColor=White]`
- Add run: `[(5,15) fontFamily="Times", color=null, bgColor=null, size=null]`

**Processing**:
1. Overlap detected: key `(0,10)` overlaps with range `(5,15)`
2. Overlapping portion: `(5,10)`
3. Fragments created:
   - Before: `(0,5)` with `{Arial, 12, Black, White}` ✓
   - After: None (since key.caretEnd = 10 is not > range.caretEnd = 15)
4. overlappingOldRuns = `[(5,10), {Arial, 12, Black, White}]`
5. Decompose new range `(5,15)`:
   - currentPos = 5
   - Process overlap `(5,10)`:
     - No gap before (currentPos = 5 = oldRange.caretBegin)
     - Merged prop = ApplyOverrides({Arial,12,Black,White}, {Times,null,null,null})
       = {Times, 12, Black, White} ✓
     - Add `(5,10)` with merged prop ✓
     - currentPos = 10
   - After loop: currentPos = 10 < 15
     - Add `(10,15)` with new props `{Times, null, null, null}` ✓

**Result**:
- `[(0,5) Arial, 12, Black, White]` ✓
- `[(5,10) Times, 12, Black, White]` ✓
- `[(10,15) Times, null, null, null]` ✓

Perfect! This matches the expected behavior.

## Impact on Existing Tests

The task states: "Do NOT modify test cases - they should continue to pass"

Will existing tests pass?
- All existing tests use fully-defined properties (all non-null)
- With fully-defined properties: `ApplyOverrides(old, new)` will always pick `new` values (since new is never null)
- This means behavior is identical to before for fully-defined properties ✓

Therefore, no test changes needed and all existing tests should pass.

## Summary

**Solution**: 
1. Add `ApplyOverrides` helper function to merge properties (null = keep base value)
2. Modify `AddTextRun` to:
   - Collect overlapping old runs and their overlap ranges
   - Decompose the new range into subranges (gaps and overlaps)
   - Add gaps with new properties, overlaps with merged properties
   - Keep existing merge logic unchanged

**Why this works**:
- Respects the "null = keep existing" semantic for overlapping portions
- Handles non-overlapping portions correctly (use new properties as-is)
- Maintains backward compatibility (fully-defined properties behave identically)
- Clean separation of concerns (override logic in `ApplyOverrides`, decomposition in main function)

**Files to modify**:
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

# !!!FINISHED!!!
