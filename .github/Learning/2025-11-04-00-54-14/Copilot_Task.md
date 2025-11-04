# !!!TASK!!!

# PROBLEM DESCRIPTION

Define a `CaretRange` struct to represent a range of caret positions `(caretBegin, caretEnd)` in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`. Implement comparison operators (`<`, `==`, `!=`) to enable its use as a dictionary key. The comparison should order by `caretBegin` first, then by `caretEnd`.

Create three type aliases for managing runs:
- `using DocumentTextRunPropertyMap = Dictionary<CaretRange, DocumentTextRunProperty>;`
- `using DocumentInlineObjectRunPropertyMap = Dictionary<CaretRange, DocumentInlineObjectRunProperty>;`
- `using DocumentRunPropertyMap = Dictionary<CaretRange, DocumentRunProperty>;`

All types come from the protocol schema in `GuiRemoteProtocolSchema.h`.

Implement four global functions for managing runs:

1. **`AddTextRun(DocumentTextRunPropertyMap& map, CaretRange range, const DocumentTextRunProperty& property)`**: 
   - Adds a text run to the map with the specified range and property
   - Keys in the map cannot overlap - when overlap occurs, split existing runs
   - After adding, merge consecutive runs with identical property values into a single run
   - Algorithm: Insert the new run, then iterate through potentially affected ranges to split them, then scan for consecutive identical runs to merge

2. **`AddInlineObjectRun(DocumentInlineObjectRunPropertyMap& map, CaretRange range, const DocumentInlineObjectRunProperty& property) -> bool`**:
   - Adds an inline object run to the map
   - Inline objects cannot be split and cannot overlap with other inline objects
   - If the new range overlaps any existing inline object, the operation fails and returns `false`
   - Returns `true` on success

3. **`ResetInlineObjectRun(DocumentInlineObjectRunPropertyMap& map, CaretRange range) -> bool`**:
   - Removes the inline object run with the exact matching key (range)
   - Does NOT allow overlapping - must match the exact key
   - Returns `false` if the key cannot be found in the map
   - Returns `true` if the key is found and removed

4. **`MergeRuns(const DocumentTextRunPropertyMap& textRuns, const DocumentInlineObjectRunPropertyMap& inlineObjectRuns, DocumentRunPropertyMap& result)`**:
   - Combines two separate maps into a unified DocumentRunPropertyMap
   - When text runs and inline object runs overlap, inline objects have priority
   - Text runs that overlap inline objects must be cut/split to avoid the inline object ranges
   - Leverage the fact that Dictionary items are ordered by key for efficient iteration

5. **`DiffRuns(const DocumentRunPropertyMap& oldRuns, const DocumentRunPropertyMap& newRuns, ElementDesc_DocumentParagraph& result)`**:
   - Compares two run maps and fills the difference into the result
   - Populates `result.runsDiff` with runs that changed (added, removed, or modified)
   - Populates `result.createdInlineObjects` with inline objects that were added
   - Populates `result.removedInlineObjects` with inline objects that were removed
   - **IMPORTANT**: Keys between `oldRuns` and `newRuns` could be different (e.g., splitted or merged)
   - When a part of an old run is updated, the function must always keep complete keys of `newRuns` in the result
   - The result should represent the final state with `newRuns` keys, not a mixture of old and new keys
   - Efficiently process both maps in a single pass by leveraging their sorted key order

# UPDATES

## UPDATE

You can use a `auto operator<=>(const CaretRange&)=default;` to create a standard comparison feature.
You can use BinarySearchLambda when binary search is a better approach. Since caret may overlap so instead of using the default version, you may want to provide your own comparer.
The MergeRuns algorithms can be improved, Since you are merging two runs ordered by CaretRange, you can iterate both at the same time, just like the merge phrase in merge sort. Same for DiffRuns

# INSIGHTS AND REASONING

## Overview

This task requires designing data structures and algorithms for managing text formatting runs in the remote paragraph protocol. The core challenge is handling overlapping ranges with different priorities (inline objects cannot be split, text runs can be) and efficiently computing differences for protocol updates.

## CaretRange Struct Design

### Purpose
`CaretRange` represents a contiguous range of caret positions `[caretBegin, caretEnd)` (half-open interval) within a paragraph. It serves as the key type for Dictionary-based run maps.

### Structure
```cpp
struct CaretRange
{
    vint caretBegin;
    vint caretEnd;
    
    // Use C++20 three-way comparison operator for automatic generation of all comparison operators
    auto operator<=>(const CaretRange&) const = default;
};
```

### Comparison Semantics
- **Simplified with C++20**: Using `operator<=>` with `=default` automatically generates all six comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- **Lexicographic ordering**: Compares members in declaration order (first `caretBegin`, then `caretEnd`)
- **Primary ordering**: By `caretBegin` (ascending)
- **Secondary ordering**: By `caretEnd` (ascending) when `caretBegin` is equal
- This lexicographic ordering ensures:
  - Ranges are sorted by start position
  - Among ranges starting at the same position, shorter ones come first
  - Enables efficient overlap detection via Dictionary's sorted property

### Design Rationale for operator<=>
- **Less boilerplate**: No need to manually implement three separate comparison operators
- **Standard compliance**: Leverages C++20 standard feature for comparison
- **Correctness**: Default implementation guarantees consistent behavior across all comparison operators
- **Maintainability**: If fields are added/changed, comparison logic updates automatically

### Design Rationale
Using a struct instead of `Pair<vint, vint>` provides:
1. **Type safety**: Prevents confusion with other integer pairs
2. **Semantic clarity**: Field names `caretBegin`/`caretEnd` are self-documenting
3. **Future extensibility**: Easy to add helper methods (e.g., `Contains()`, `Overlaps()`, `Length()`)
4. **Consistency**: Matches naming conventions in protocol schema

## Type Alias Design

### Three Map Types
```cpp
using DocumentTextRunPropertyMap = Dictionary<CaretRange, DocumentTextRunProperty>;
using DocumentInlineObjectRunPropertyMap = Dictionary<CaretRange, DocumentInlineObjectRunProperty>;
using DocumentRunPropertyMap = Dictionary<CaretRange, DocumentRunProperty>;
```

### Separation Rationale
Separating text runs from inline object runs at the type level provides:

1. **Type safety**: Prevents accidentally mixing text and inline object operations
2. **Clear semantics**: Functions explicitly declare which run type they operate on
3. **Algorithmic clarity**: `AddTextRun` and `AddInlineObjectRun` have fundamentally different behaviors (splitting vs. no-splitting)
4. **Efficient storage**: No need for variant discrimination until merge time

The unified `DocumentRunPropertyMap` is only used after merging, reflecting the protocol's view where both run types are transmitted together.

## AddTextRun Algorithm Design

### Function Signature
```cpp
void AddTextRun(
    DocumentTextRunPropertyMap& map, 
    CaretRange range, 
    const DocumentTextRunProperty& property
);
```

### Core Requirements
1. **No overlapping keys**: Dictionary keys cannot overlap
2. **Splitting**: When new run overlaps existing runs, split existing runs
3. **Merging**: After insertion, merge consecutive runs with identical properties

### Algorithm Steps

**Step 1: Collect overlapping runs**
- Iterate through `map` to find all runs that overlap with `range`
- A run overlaps if: `run.caretEnd > range.caretBegin && run.caretBegin < range.caretEnd`
- Store overlapping runs in a temporary list
- Remove overlapping runs from `map`

**Step 2: Split overlapping runs**
For each overlapping run, split it into up to 3 parts:
- **Before part**: `[run.caretBegin, range.caretBegin)` if `run.caretBegin < range.caretBegin`
- **Overlap part**: Intersection of run and range (discarded, replaced by new run)
- **After part**: `[range.caretEnd, run.caretEnd)` if `run.caretEnd > range.caretEnd`

Reinsert non-overlapping parts back into `map`.

**Step 3: Insert new run**
- Add `(range, property)` to `map`

**Step 4: Merge consecutive identical runs**
- Find the newly inserted run and its neighbors (using Dictionary's sorted order)
- Check left neighbor: if `neighbor.caretEnd == range.caretBegin` and properties are identical, merge
- Check right neighbor: if `range.caretEnd == neighbor.caretBegin` and properties are identical, merge
- Continue merging transitively if multiple consecutive runs have identical properties

### Optimized Overlap Detection with BinarySearchLambda

Instead of linear scan, use `BinarySearchLambda` to find overlapping runs efficiently:

```cpp
// Find the first run that might overlap with range
// A run at position i might overlap if: run.caretEnd > range.caretBegin
auto comparer = [&](const Pair<CaretRange, DocumentTextRunProperty>& item) -> vint
{
    if (item.key.caretEnd <= range.caretBegin)
        return -1;  // This run is completely before range
    else if (item.key.caretBegin >= range.caretEnd)
        return 1;   // This run is completely after range
    else
        return 0;   // This run overlaps with range
};

vint firstOverlap = BinarySearchLambda(map.Keys(), comparer);
```

**Rationale for custom comparer**:
- Standard binary search finds exact match, but we need overlapping ranges
- Overlapping ranges don't form a total ordering (multiple ranges can overlap same position)
- Custom comparer defines three regions: before, overlapping, after
- `BinarySearchLambda` returns index of first item where comparer returns 0, or -1 if none

**After finding first overlap**:
- Iterate forward from `firstOverlap` to collect all overlapping runs
- Stop when `run.caretBegin >= range.caretEnd` (no more overlaps possible)

### Complexity Analysis
- **Overlap detection**: O(log n + k) where k is number of overlapping runs (using BinarySearchLambda)
- **Splitting/insertion**: O(k log n) where k is number of overlapping runs (Dictionary operations)
- **Merging**: O(1) to O(k) where k is length of consecutive identical runs
- **Overall**: O(log n + k log n) = O(k log n) typical case, much better than O(n)

### Edge Cases
1. **Empty map**: Simply insert the run
2. **No overlaps**: Direct insertion, check neighbors for merge
3. **New run completely contains existing run**: Existing run is entirely removed
4. **New run completely contained by existing run**: Split existing into 3 parts
5. **Multiple overlaps**: All affected runs are processed

### Property Comparison for Merging
Need to compare `DocumentTextRunProperty` instances:
```cpp
bool operator==(const DocumentTextRunProperty& a, const DocumentTextRunProperty& b)
{
    return a.textColor == b.textColor && 
           a.backgroundColor == b.backgroundColor &&
           a.fontProperties == b.fontProperties;  // FontProperties needs operator==
}
```

## AddInlineObjectRun Algorithm Design

### Function Signature
```cpp
bool AddInlineObjectRun(
    DocumentInlineObjectRunPropertyMap& map,
    CaretRange range,
    const DocumentInlineObjectRunProperty& property
);
```

### Core Requirements
1. **No overlapping**: Inline objects cannot overlap with each other
2. **Atomic**: Cannot be split
3. **Failure mode**: Return `false` if overlap detected, `true` on success

### Algorithm Steps

**Step 1: Check for overlaps**
- Iterate through `map` to check if any existing run overlaps with `range`
- An overlap exists if: `run.caretEnd > range.caretBegin && run.caretBegin < range.caretEnd`
- If any overlap found, return `false` immediately

**Step 2: Insert**
- If no overlaps, add `(range, property)` to `map`
- Return `true`

### Optimized Overlap Detection with BinarySearchLambda

Use the same binary search approach as in `AddTextRun`:

```cpp
// Find the first run that might overlap with range
auto comparer = [&](const Pair<CaretRange, DocumentInlineObjectRunProperty>& item) -> vint
{
    if (item.key.caretEnd <= range.caretBegin)
        return -1;  // This run is completely before range
    else if (item.key.caretBegin >= range.caretEnd)
        return 1;   // This run is completely after range
    else
        return 0;   // This run overlaps with range - reject insertion
};

vint firstOverlap = BinarySearchLambda(map.Keys(), comparer);
if (firstOverlap != -1)
    return false;  // Overlap found, insertion rejected
```

Since inline objects cannot overlap, we only need to find if ANY overlap exists, not collect all overlaps.

### Complexity Analysis
- **Overlap detection**: O(log n) using BinarySearchLambda
- **Insertion**: O(log n)
- **Overall**: O(log n)

## ResetInlineObjectRun Algorithm Design

### Function Signature
```cpp
bool ResetInlineObjectRun(
    DocumentInlineObjectRunPropertyMap& map,
    CaretRange range
);
```

### Core Requirements
1. **Exact match**: Only remove if exact key exists
2. **No partial matches**: Overlapping ranges don't count
3. **Return status**: `false` if key not found, `true` if removed

### Algorithm Steps

**Step 1: Attempt removal**
- Use `map.Remove(range)` which returns `bool` indicating success
- Dictionary's `Remove()` requires exact key match

**Step 2: Return result**
- Return the result from `map.Remove(range)`

### Complexity Analysis
- **Lookup and removal**: O(log n)
- **Overall**: O(log n)

### Implementation Note
This is the simplest function - it's essentially a wrapper around Dictionary's `Remove()` method. The key design point is that Dictionary comparison uses `operator==`, which requires exact match of both `caretBegin` and `caretEnd`.

## MergeRuns Algorithm Design

### Function Signature
```cpp
void MergeRuns(
    const DocumentTextRunPropertyMap& textRuns,
    const DocumentInlineObjectRunPropertyMap& inlineObjectRuns,
    DocumentRunPropertyMap& result
);
```

### Core Requirements
1. **Combine two maps**: Unite text runs and inline object runs
2. **Priority rule**: Inline objects take precedence over text runs
3. **Cut text runs**: When text run overlaps inline object, cut the text run
4. **Efficient**: Leverage ordered property of Dictionary

### Algorithm Steps: Two-Pointer Merge (Like Merge Sort)

Since both input maps are ordered by `CaretRange`, we can merge them in a single pass similar to the merge phase of merge sort:

**Step 1: Initialize**
- Clear `result` map
- Create indices for both input maps: `textIdx = 0`, `inlineIdx = 0`

**Step 2: Two-pointer merge with text run cutting**
```
while textIdx < textRuns.Count() or inlineIdx < inlineObjectRuns.Count():
    
    if textIdx >= textRuns.Count():
        // No more text runs, add remaining inline objects
        Add inlineObjectRuns[inlineIdx] to result
        inlineIdx++
        continue
    
    if inlineIdx >= inlineObjectRuns.Count():
        // No more inline objects, add remaining text runs
        Add textRuns[textIdx] to result
        textIdx++
        continue
    
    textRange = textRuns.Keys()[textIdx]
    inlineRange = inlineObjectRuns.Keys()[inlineIdx]
    
    // Case 1: Text run ends before inline object starts - no overlap
    if textRange.caretEnd <= inlineRange.caretBegin:
        Add textRuns[textIdx] to result
        textIdx++
    
    // Case 2: Inline object ends before text run starts - no overlap
    else if inlineRange.caretEnd <= textRange.caretBegin:
        Add inlineObjectRuns[inlineIdx] to result
        inlineIdx++
    
    // Case 3: Overlapping - need to cut text run
    else:
        // Add text fragment before inline object (if any)
        if textRange.caretBegin < inlineRange.caretBegin:
            result.Add([textRange.caretBegin, inlineRange.caretBegin), textRuns[textIdx])
        
        // Add inline object (takes priority)
        Add inlineObjectRuns[inlineIdx] to result
        inlineIdx++
        
        // Handle remaining part of text run
        if textRange.caretEnd > inlineRange.caretEnd:
            // Text run extends beyond inline object
            // Create a new text run for the remainder and continue processing it
            textRange.caretBegin = inlineRange.caretEnd
            // Don't advance textIdx yet, process remainder in next iteration
        else:
            // Text run completely consumed by inline object
            textIdx++
```

**Key insight**: When text run overlaps inline object:
1. Split text before inline if needed
2. Add inline object
3. Keep processing remainder of text (might overlap with next inline object)
4. This naturally handles multiple inline objects within a single text run

### Complexity Analysis
- **Single pass**: O(n + m) where n is text runs, m is inline objects
- **Each position processed once**: Text run may be processed multiple times only if it overlaps multiple inline objects, but each fragment is processed once
- **Space**: O(n + m) for result map
- **Optimal**: Cannot do better than O(n + m) since we must examine all input

This is significantly better than the previous O(n * m) approach and matches the efficiency of merge sort's merge phase.

## DiffRuns Algorithm Design

### Function Signature
```cpp
void DiffRuns(
    const DocumentRunPropertyMap& oldRuns,
    const DocumentRunPropertyMap& newRuns,
    ElementDesc_DocumentParagraph& result
);
```

### Core Requirements
1. **Detect changes**: Find added, removed, and modified runs
2. **Track inline objects**: Separately track created and removed inline objects
3. **Critical requirement**: Result must use complete keys from `newRuns`, not partial or mixed keys
4. **Efficient**: Single-pass algorithm leveraging sorted order

### Algorithm Steps

**Step 1: Initialize result arrays**
```cpp
result.runsDiff = new List<DocumentRun>();
result.createdInlineObjects = new List<vint>();
result.removedInlineObjects = new List<vint>();
```

**Step 2: Two-pointer comparison (Like Merge Sort Merge Phase)**
Iterate through both maps simultaneously using merge-like algorithm:
- `oldIdx` points to current position in `oldRuns`
- `newIdx` points to current position in `newRuns`

```
while oldIdx < oldRuns.Count() or newIdx < newRuns.Count():
    
    if oldIdx >= oldRuns.Count():
        // No more old runs, all remaining new runs are added
        Add newRuns[newIdx] to runsDiff
        Track inline object creation if applicable
        newIdx++
        continue
    
    if newIdx >= newRuns.Count():
        // No more new runs, all remaining old runs are removed
        Track inline object removal if applicable
        oldIdx++
        continue
    
    oldKey = oldRuns.Keys()[oldIdx]
    newKey = newRuns.Keys()[newIdx]
    
    // Case 1: Keys are identical
    if oldKey == newKey:
        Compare properties
        If properties differ:
            Add newRuns[newIdx] to runsDiff
        Track inline object changes if applicable
        oldIdx++
        newIdx++
    
    // Case 2: newKey < oldKey (new run comes before old run)
    else if newKey < oldKey:
        // This key is new (added)
        Add newRuns[newIdx] to runsDiff
        Track inline object creation if applicable
        newIdx++
    
    // Case 3: oldKey < newKey (old run comes before new run)
    else:
        // This key was removed
        Track inline object removal if applicable
        oldIdx++
```

**Step 3: Process differences**
This merge-like approach naturally handles all cases:
- **Identical keys**: Property comparison
- **New keys**: Added runs (including splits, merges, modifications)
- **Removed keys**: Deleted runs

**Step 4: Handle key splitting/merging**
CRITICAL: The algorithm above handles exact key matches, but keys can be split or merged. Need additional logic:

**Detecting splits**: When old run [0,10] becomes new runs [0,5] and [5,10]:
- When processing new run [0,5]: no exact match in old runs
  - Check if [0,5] is contained within any old run's range
  - If yes, this is a split - compare property of new run with old run's property at that position
  - Add to `runsDiff` if property changed

**Detecting merges**: When old runs [0,5] and [5,10] become new run [0,10]:
- When processing new run [0,10]: no exact match in old runs
  - Check if [0,10] spans multiple old runs
  - Compare property of new run with old runs' properties
  - Add to `runsDiff` if any property differs

**Simplified approach to handle splits/merges**:
Instead of complex split/merge detection, adopt a simpler rule:
- For each run in `newRuns`, check if there's a corresponding run in `oldRuns` with the same key AND same property
- If not found (either key differs or property differs), add to `runsDiff`
- This ensures result always uses complete keys from `newRuns`

**Revised Algorithm**:
```
// Phase 1: Process all new runs
For each (key, value) in newRuns:
    if oldRuns.Contains(key) and oldRuns[key] == value:
        // No change, skip
    else:
        // New or modified run
        runsDiff.Add(DocumentRun{key, value})
        if value is inline object:
            if oldRuns.Contains(key):
                oldValue = oldRuns[key]
                if oldValue is inline object and oldValue.callbackId == value.callbackId:
                    // Same inline object, don't track as created
                else:
                    createdInlineObjects.Add(value.callbackId)
            else:
                createdInlineObjects.Add(value.callbackId)

// Phase 2: Find removed runs
For each (key, value) in oldRuns:
    if not newRuns.Contains(key):
        // Run was removed
        runsDiff.Add(DocumentRun{key, null_marker})  // Need to define how to mark removal
        if value is inline object:
            // Check if this inline object appears anywhere in newRuns with different key
            found = false
            for each (newKey, newValue) in newRuns:
                if newValue is inline object and newValue.callbackId == value.callbackId:
                    found = true
                    break
            if not found:
                removedInlineObjects.Add(value.callbackId)
```

### Complexity Analysis with Two-Pointer Merge
- **Main loop**: O(n + m) where n is size of newRuns, m is size of oldRuns
- **Inline object tracking**: O(k) where k is total number of inline objects (linear scan during merge)
- **Overall**: O(n + m) - optimal complexity
- **No dictionary lookups needed**: Unlike the naive approach, we don't need to lookup keys in the other map
- **Similar to merge sort**: Same algorithmic pattern as merging two sorted arrays

### Critical Design Decision: Marking Removed Runs
Question: How to represent removed runs in `runsDiff`?

Option 1: Use a separate array for removed runs
Option 2: Use a sentinel value in DocumentRunProperty (e.g., special variant state)
Option 3: Only include added/modified runs in `runsDiff`, rely on absence to indicate removal

**Recommendation**: Option 3 is cleanest:
- The consumer can compare oldRuns and newRuns to detect removals
- `runsDiff` contains the new state (all runs that should exist after update)
- This aligns with the "complete keys from newRuns" requirement

**Revised interpretation**: `runsDiff` should contain:
- All runs from `newRuns` that differ from `oldRuns` (new or modified)
- This naturally ensures "complete keys from newRuns" requirement

### Inline Object Tracking
**createdInlineObjects**: callbackIds of inline objects in `newRuns` but not in `oldRuns`
**removedInlineObjects**: callbackIds of inline objects in `oldRuns` but not in `newRuns`

Need to track by `callbackId` (unique identifier) rather than key/range, since an inline object might be moved to a different position.

### Example Scenarios

**Scenario 1: Old run split**
- oldRuns: `[0,10] -> TextProperty{red}`
- newRuns: `[0,5] -> TextProperty{red}`, `[5,10] -> TextProperty{blue}`
- Result: `runsDiff` contains `[5,10] -> blue` (only the changed part)
  - Wait, this violates "complete keys from newRuns" requirement!
  
**Corrected**: `runsDiff` should contain BOTH `[0,5] -> red` AND `[5,10] -> blue` since neither key exactly matches `[0,10]` in oldRuns.

**Scenario 2: Runs merged**
- oldRuns: `[0,5] -> TextProperty{red}`, `[5,10] -> TextProperty{red}`
- newRuns: `[0,10] -> TextProperty{red}`
- Result: `runsDiff` contains `[0,10] -> red` since this key doesn't exist in oldRuns

**Scenario 3: Property changed**
- oldRuns: `[0,10] -> TextProperty{red}`
- newRuns: `[0,10] -> TextProperty{blue}`
- Result: `runsDiff` contains `[0,10] -> blue`

**Scenario 4: Inline object added**
- oldRuns: `[0,10] -> TextProperty{red}`
- newRuns: `[0,5] -> TextProperty{red}`, `[5,7] -> InlineObject{id=42}`, `[7,10] -> TextProperty{red}`
- Result: 
  - `runsDiff`: all three runs (none of these keys exist in oldRuns)
  - `createdInlineObjects`: [42]

### Final Algorithm Design with Two-Pointer Merge

```cpp
void DiffRuns(
    const DocumentRunPropertyMap& oldRuns,
    const DocumentRunPropertyMap& newRuns,
    ElementDesc_DocumentParagraph& result
)
{
    result.runsDiff = new List<DocumentRun>();
    result.createdInlineObjects = new List<vint>();
    result.removedInlineObjects = new List<vint>();
    
    // Track inline object callbackIds as we iterate
    Set<vint> oldInlineCallbackIds;
    Set<vint> newInlineCallbackIds;
    
    // Two-pointer merge algorithm
    vint oldIdx = 0;
    vint newIdx = 0;
    
    auto oldKeys = oldRuns.Keys();
    auto newKeys = newRuns.Keys();
    
    while (oldIdx < oldKeys.Count() || newIdx < newKeys.Count())
    {
        // Handle remaining items when one list is exhausted
        if (oldIdx >= oldKeys.Count())
        {
            // All remaining new runs are additions
            auto newKey = newKeys[newIdx];
            auto newValue = newRuns[newKey];
            
            AddToRunsDiff(result, newKey, newValue);
            TrackInlineObjectIfNeeded(newValue, newInlineCallbackIds);
            newIdx++;
            continue;
        }
        
        if (newIdx >= newKeys.Count())
        {
            // All remaining old runs are removals
            auto oldKey = oldKeys[oldIdx];
            auto oldValue = oldRuns[oldKey];
            
            TrackInlineObjectIfNeeded(oldValue, oldInlineCallbackIds);
            oldIdx++;
            continue;
        }
        
        auto oldKey = oldKeys[oldIdx];
        auto newKey = newKeys[newIdx];
        
        if (oldKey == newKey)
        {
            // Keys match - compare properties
            auto oldValue = oldRuns[oldKey];
            auto newValue = newRuns[newKey];
            
            TrackInlineObjectIfNeeded(oldValue, oldInlineCallbackIds);
            TrackInlineObjectIfNeeded(newValue, newInlineCallbackIds);
            
            if (!PropertiesEqual(oldValue, newValue))
            {
                AddToRunsDiff(result, newKey, newValue);
            }
            
            oldIdx++;
            newIdx++;
        }
        else if (newKey < oldKey)
        {
            // New key comes first - this is an addition
            auto newValue = newRuns[newKey];
            
            AddToRunsDiff(result, newKey, newValue);
            TrackInlineObjectIfNeeded(newValue, newInlineCallbackIds);
            newIdx++;
        }
        else
        {
            // Old key comes first - this is a removal
            auto oldValue = oldRuns[oldKey];
            
            TrackInlineObjectIfNeeded(oldValue, oldInlineCallbackIds);
            oldIdx++;
        }
    }
    
    // Compute inline object differences
    for (auto id : newInlineCallbackIds)
        if (!oldInlineCallbackIds.Contains(id))
            result.createdInlineObjects->Add(id);
    
    for (auto id : oldInlineCallbackIds)
        if (!newInlineCallbackIds.Contains(id))
            result.removedInlineObjects->Add(id);
}
```

This improved design ensures:
1. **Complete keys from newRuns**: Only newRuns keys appear in runsDiff
2. **Optimal efficiency**: O(n + m) single-pass merge algorithm, no dictionary lookups needed
3. **Correct inline object tracking**: By callbackId, tracked during merge
4. **Simple and clear**: Uses same pattern as merge sort's merge phase
5. **No redundant iterations**: All tracking done in single pass

## Implementation Location

### Header File: GuiRemoteGraphics_Document.h
Add after existing `GuiRemoteGraphicsParagraph` class declaration:

```cpp
namespace vl::presentation::remoteprotocol
{
    // CaretRange struct
    struct CaretRange
    {
        vint caretBegin;
        vint caretEnd;
        
        // Use C++20 three-way comparison for automatic generation of all comparison operators
        auto operator<=>(const CaretRange&) const = default;
    };
    
    // Type aliases
    using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, DocumentTextRunProperty>;
    using DocumentInlineObjectRunPropertyMap = collections::Dictionary<CaretRange, DocumentInlineObjectRunProperty>;
    using DocumentRunPropertyMap = collections::Dictionary<CaretRange, DocumentRunProperty>;
    
    // Function declarations
    extern void AddTextRun(
        DocumentTextRunPropertyMap& map,
        CaretRange range,
        const DocumentTextRunProperty& property);
    
    extern bool AddInlineObjectRun(
        DocumentInlineObjectRunPropertyMap& map,
        CaretRange range,
        const DocumentInlineObjectRunProperty& property);
    
    extern bool ResetInlineObjectRun(
        DocumentInlineObjectRunPropertyMap& map,
        CaretRange range);
    
    extern void MergeRuns(
        const DocumentTextRunPropertyMap& textRuns,
        const DocumentInlineObjectRunPropertyMap& inlineObjectRuns,
        DocumentRunPropertyMap& result);
    
    extern void DiffRuns(
        const DocumentRunPropertyMap& oldRuns,
        const DocumentRunPropertyMap& newRuns,
        ElementDesc_DocumentParagraph& result);
}
```

### Implementation File: GuiRemoteGraphics_Document.cpp
Create new file or add to existing file with full implementations of all functions.

## Testing Considerations

The comprehensive unit tests in Task 2 will validate:

1. **CaretRange comparison**: Proper ordering and equality
2. **AddTextRun**: All overlap scenarios, splitting, merging
3. **AddInlineObjectRun**: Overlap detection, failure modes
4. **ResetInlineObjectRun**: Exact match requirement
5. **MergeRuns**: Priority rules, cutting logic
6. **DiffRuns**: Change detection, inline object tracking, key preservation

The design prioritizes correctness over performance for the initial implementation, with clear opportunities for optimization once tests validate correctness.

## Alternative Design Considerations

### Alternative 1: Single Map with Union Type
Instead of separating text and inline object maps, use single DocumentRunPropertyMap throughout.

**Pros**:
- Simpler API with fewer types
- No merge step needed

**Cons**:
- Lost type safety - AddTextRun could accidentally receive inline objects
- Unclear semantics - functions would need runtime type checking
- Harder to enforce different rules (splitting vs. no-splitting)

**Decision**: Rejected - type safety and semantic clarity are more important

### Alternative 2: Interval Tree Data Structure
Use specialized interval tree instead of Dictionary for better overlap detection.

**Pros**:
- O(log n) overlap queries instead of O(n)
- More efficient for many overlapping ranges

**Cons**:
- Not available in Vlpp, would need custom implementation
- Added complexity
- Dictionary is "good enough" for typical paragraph sizes (< 1000 runs)

**Decision**: Rejected for initial implementation - can optimize later if performance issues arise

### Alternative 3: Diff by Range Iteration
Instead of comparing keys, iterate through all possible positions and compare properties.

**Pros**:
- Simpler logic, no key matching needed
- Automatically handles splits/merges

**Cons**:
- Less efficient for sparse changes
- Harder to track inline object creation/removal

**Decision**: Rejected - key-based comparison is more efficient and explicit

## Improvements from Update

The updated design incorporates several key improvements:

### 1. C++20 Three-Way Comparison Operator
**Change**: Use `auto operator<=>(const CaretRange&) const = default;` instead of manually implementing three separate comparison operators.

**Benefits**:
- **Less code**: Single line generates all six comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- **Correctness**: Default implementation guarantees consistency across all operators
- **Maintainability**: Automatic updates if fields change
- **Standard compliance**: Leverages modern C++20 feature

### 2. BinarySearchLambda for Overlap Detection
**Change**: Use `BinarySearchLambda` with custom comparer instead of linear scan to find overlapping ranges.

**Benefits**:
- **Performance**: O(log n) instead of O(n) for finding first overlap
- **Custom semantics**: Comparer defines "before/overlap/after" regions appropriate for range overlap
- **Vlpp integration**: Uses existing library function designed for this purpose

**Implementation**: Custom comparer returns:
- `-1` if range is completely before target
- `1` if range is completely after target
- `0` if range overlaps target

### 3. Two-Pointer Merge Algorithm for MergeRuns
**Change**: Process both input maps simultaneously like merge phase of merge sort, instead of processing text runs and then looking up inline objects.

**Benefits**:
- **Optimal performance**: O(n + m) instead of O(n * m)
- **Single pass**: Each element processed exactly once
- **Natural handling**: Overlaps handled incrementally as we encounter them
- **Simpler logic**: No need for nested loops or complex lookup

**Key insight**: Both maps are already sorted by `CaretRange`, so we can merge them efficiently like merging two sorted arrays.

### 4. Two-Pointer Merge Algorithm for DiffRuns
**Change**: Iterate through both `oldRuns` and `newRuns` simultaneously using merge algorithm, instead of separate phases.

**Benefits**:
- **Optimal performance**: O(n + m) with no dictionary lookups
- **Single pass**: All difference detection and inline object tracking in one iteration
- **Memory efficient**: No intermediate data structures needed
- **Clearer semantics**: Same pattern as MergeRuns for consistency

**Key insight**: Since both maps are sorted, we can compare them position-by-position like diff algorithms do with sorted lists.

## Summary

This design provides:

1. **Type-safe representation**: CaretRange struct with C++20 three-way comparison operator
2. **Separated concerns**: Different map types for text vs. inline objects
3. **Comprehensive algorithms**: Handles all overlap scenarios, splitting, merging, and diffing
4. **Optimal efficiency**: 
   - O(log n + k log n) for AddTextRun using BinarySearchLambda
   - O(log n) for AddInlineObjectRun using BinarySearchLambda
   - O(n + m) for MergeRuns using two-pointer merge
   - O(n + m) for DiffRuns using two-pointer merge
5. **Modern C++ features**: Leverages C++20 operator<=> and Vlpp's BinarySearchLambda
6. **Testable**: Clear requirements and edge cases for unit testing
7. **Extensible**: Easy to add helper methods or further optimize

The critical insights are:
- **DiffRuns key preservation**: Must preserve complete keys from newRuns, naturally handling splits, merges, and modifications
- **Sorted dictionary advantage**: Both MergeRuns and DiffRuns leverage sorted order for O(n + m) single-pass algorithms, similar to merge sort's merge phase
- **Custom overlap detection**: BinarySearchLambda with custom comparer efficiently finds overlapping ranges despite their non-total ordering

# !!!FINISHED!!!
