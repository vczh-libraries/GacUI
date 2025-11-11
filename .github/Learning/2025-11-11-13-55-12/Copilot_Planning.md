# !!!PLANNING!!!

# UPDATES

# IMPROVEMENT PLAN

## STEP 1: Define DocumentTextRunPropertyOverrides struct in header file

**File**: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`

**Location**: Add the new struct definition right after the `CaretRange` struct (around line 27).

**Changes**:
```cpp
struct CaretRange
{
	vint caretBegin = 0;
	vint caretEnd = 0;

	auto operator<=>(const CaretRange&) const = default;
};

// Add this new struct
struct DocumentTextRunPropertyOverrides
{
	Nullable<Color> textColor;
	Nullable<Color> backgroundColor;
	Nullable<WString> fontFamily;
	Nullable<vint> size;
	Nullable<IGuiGraphicsParagraph::TextStyle> textStyle;
};
```

**Rationale**: 
- This struct represents property overrides where each field can be null (meaning "not specified" in future tasks).
- Unlike `DocumentTextRunProperty` which has a single `FontProperties` field, we expand font properties into individual nullable fields to support partial font property overrides.
- `textStyle` combines `bold`, `italic`, `underline`, `strikeline` into a single enum flag field as defined in `IGuiGraphicsParagraph::TextStyle`.
- The `antialias` and `verticalAntialias` fields from `FontProperties` are intentionally omitted as they are renderer-specific settings, not user-facing style properties.

## STEP 2: Update DocumentTextRunPropertyMap type alias

**File**: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`

**Location**: Around line 29 where type aliases are defined.

**Changes**:
```cpp
// Change this line:
using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, remoteprotocol::DocumentTextRunProperty>;

// To:
using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, DocumentTextRunPropertyOverrides>;
```

**Rationale**: The map now stores overrides instead of full property objects, preparing the infrastructure for nullable property semantics in future tasks.

## STEP 3: Update AddTextRun function signature in header

**File**: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`

**Location**: Around line 34 where `AddTextRun` is declared.

**Changes**:
```cpp
// Change this signature:
extern void AddTextRun(
	DocumentTextRunPropertyMap& map,
	CaretRange range,
	const remoteprotocol::DocumentTextRunProperty& property);

// To:
extern void AddTextRun(
	DocumentTextRunPropertyMap& map,
	CaretRange range,
	const DocumentTextRunPropertyOverrides& propertyOverrides);
```

**Rationale**: Function now accepts the new overrides type, matching the updated map type.

## STEP 4: Update AreEqual function for DocumentTextRunPropertyOverrides

**File**: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

**Location**: Around line 13 where `AreEqual` for `DocumentTextRunProperty` is defined.

**Changes**:
```cpp
// Change this function:
bool AreEqual(const remoteprotocol::DocumentTextRunProperty& a, const remoteprotocol::DocumentTextRunProperty& b)
{
	return a.textColor == b.textColor &&
		   a.backgroundColor == b.backgroundColor &&
		   a.fontProperties == b.fontProperties;
}

// To:
bool AreEqual(const DocumentTextRunPropertyOverrides& a, const DocumentTextRunPropertyOverrides& b)
{
	return a.textColor == b.textColor &&
		   a.backgroundColor == b.backgroundColor &&
		   a.fontFamily == b.fontFamily &&
		   a.size == b.size &&
		   a.textStyle == b.textStyle;
}
```

**Rationale**: 
- Compare all fields in the overrides struct including nullability.
- The `Nullable<T>` type's `operator==` will correctly compare both null state and values.
- At this stage, we treat null as just another value - two overrides are equal only if all fields match exactly (including null vs non-null).

## STEP 5: Update AddTextRun implementation

**File**: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

**Location**: The `AddTextRun` function (around line 43).

**Changes**:
```cpp
// Update function signature:
void AddTextRun(
	DocumentTextRunPropertyMap& map,
	CaretRange range,
	const DocumentTextRunPropertyOverrides& propertyOverrides)  // Changed parameter type
{
	// Keep all existing logic unchanged
	// The function body remains identical except:
	// - All references to 'property' become 'propertyOverrides'
	// - The variable type changes from DocumentTextRunProperty to DocumentTextRunPropertyOverrides
	
	// No changes to:
	// - Overlap detection logic
	// - Range splitting logic
	// - Merging logic using AreEqual
	// - All algorithm stays the same
	
	// Example changes in variable declarations:
	// List<Pair<CaretRange, remoteprotocol::DocumentTextRunProperty>> fragmentsToReinsert;
	// becomes:
	List<Pair<CaretRange, DocumentTextRunPropertyOverrides>> fragmentsToReinsert;
	
	// The call to map.Add(range, property) becomes:
	map.Add(range, propertyOverrides);
}
```

**Rationale**: 
- Only mechanical type changes - from `DocumentTextRunProperty` to `DocumentTextRunPropertyOverrides`.
- The splitting, merging, and comparison logic remains completely unchanged.
- At this stage, we treat nullable fields as regular values - no special "nullable semantics" yet.
- This ensures existing behavior is preserved while the type system changes.

## STEP 6: Update MergeRuns implementation

**File**: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

**Location**: The `MergeRuns` function (around line 171).

**Changes**:
```cpp
void MergeRuns(
	const DocumentTextRunPropertyMap& textRuns,  // Now contains DocumentTextRunPropertyOverrides
	const DocumentInlineObjectRunPropertyMap& inlineObjectRuns,
	DocumentRunPropertyMap& result)
{
	result.Clear();

	vint textIdx = 0;
	vint inlineIdx = 0;

	auto&& textKeys = textRuns.Keys();
	auto&& inlineKeys = inlineObjectRuns.Keys();

	CaretRange currentTextRange;
	DocumentTextRunPropertyOverrides currentTextProperty;  // Changed type
	bool hasCurrentText = false;
	vint lastInlineEnd = -1;

	while (textIdx < textKeys.Count() || inlineIdx < inlineKeys.Count() || hasCurrentText)
	{
		if (!hasCurrentText && textIdx < textKeys.Count())
		{
			currentTextRange = textKeys[textIdx];
			currentTextProperty = textRuns[currentTextRange];
			hasCurrentText = true;
			textIdx++;
			
			// ... existing overlap trimming logic unchanged ...
		}

		// When adding text runs to result, convert overrides to full DocumentTextRunProperty:
		if (hasCurrentText && inlineIdx >= inlineKeys.Count())
		{
			// Convert DocumentTextRunPropertyOverrides to DocumentTextRunProperty
			remoteprotocol::DocumentTextRunProperty fullProp;
			fullProp.textColor = currentTextProperty.textColor.Value();
			fullProp.backgroundColor = currentTextProperty.backgroundColor.Value();
			fullProp.fontProperties.fontFamily = currentTextProperty.fontFamily.Value();
			fullProp.fontProperties.size = currentTextProperty.size.Value();
			
			// Convert TextStyle enum flags to individual bool fields
			auto style = currentTextProperty.textStyle.Value();
			fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != 0;
			fullProp.fontProperties.italic = (style & IGuiGraphicsParagraph::TextStyle::Italic) != 0;
			fullProp.fontProperties.underline = (style & IGuiGraphicsParagraph::TextStyle::Underline) != 0;
			fullProp.fontProperties.strikeline = (style & IGuiGraphicsParagraph::TextStyle::Strikeline) != 0;
			
			// Set default values for antialias properties
			fullProp.fontProperties.antialias = true;
			fullProp.fontProperties.verticalAntialias = true;
			
			remoteprotocol::DocumentRunProperty runProp = fullProp;
			result.Add(currentTextRange, runProp);
			hasCurrentText = false;
			continue;
		}

		// ... rest of the logic with similar conversions where needed ...
	}
}
```

**Rationale**: 
- The map now contains `DocumentTextRunPropertyOverrides`, so we need to extract values from `Nullable<>` wrappers when converting to `DocumentRunProperty`.
- Use `.Value()` to extract from nullable fields (they will all be non-null at this stage based on test setup).
- Convert the `TextStyle` enum flags to individual bool fields for `FontProperties`.
- Set `antialias` and `verticalAntialias` to `true` (typical defaults for anti-aliasing).
- All merging and priority logic remains unchanged.

## STEP 7: Update test helper CreateTextProp

**File**: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

**Location**: Around line 12 where `CreateTextProp` is defined.

**Changes**:
```cpp
// Change this function:
DocumentTextRunProperty CreateTextProp(unsigned char colorValue)
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

// To:
DocumentTextRunPropertyOverrides CreateTextProp(unsigned char colorValue)
{
	DocumentTextRunPropertyOverrides prop;
	prop.textColor = Color(colorValue, colorValue, colorValue);
	prop.backgroundColor = Color(255 - colorValue, 255 - colorValue, 255 - colorValue);
	prop.fontFamily = L"TestFont";
	prop.size = 12;
	// Create TextStyle with no flags set (value 0)
	prop.textStyle = (IGuiGraphicsParagraph::TextStyle)0;
	return prop;
}
```

**Rationale**: 
- Return the new overrides type with all fields initialized (non-null).
- Use individual nullable fields instead of `fontProperties` struct.
- Initialize `textStyle` to 0 (no style flags set), matching the previous behavior where all bool flags were false.
- All fields are given values, so they are non-null, ensuring existing test behavior is preserved.

## STEP 8: Update test helper FormatRunProperty for overrides

**File**: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

**Location**: Around line 38 where `FormatRunProperty` for text properties is defined.

**Changes**:
```cpp
// Change this function:
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

// To:
WString FormatRunProperty(const DocumentTextRunPropertyOverrides& prop)
{
	return L"Text(color:" + 
		   itow(prop.textColor.Value().r) + L"," + 
		   itow(prop.textColor.Value().g) + L"," + 
		   itow(prop.textColor.Value().b) + 
		   L", bg:" + 
		   itow(prop.backgroundColor.Value().r) + L"," + 
		   itow(prop.backgroundColor.Value().g) + L"," + 
		   itow(prop.backgroundColor.Value().b) + L")";
}
```

**Rationale**: 
- Update to handle the overrides type.
- Use `.Value()` to extract from nullable fields (safe because test setup ensures all fields are non-null).
- Keep the same formatting to preserve test output compatibility.

## STEP 9: Update test helper CompareRunProperty for overrides

**File**: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

**Location**: Around line 59 where `CompareRunProperty` for text properties is defined.

**Changes**:
```cpp
// Change this function:
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

// To:
bool CompareRunProperty(
	const DocumentTextRunPropertyOverrides& a,
	const DocumentTextRunPropertyOverrides& b)
{
	return a.textColor == b.textColor &&
		   a.backgroundColor == b.backgroundColor &&
		   a.fontFamily == b.fontFamily &&
		   a.size == b.size &&
		   a.textStyle == b.textStyle;
}
```

**Rationale**: 
- Update to compare the overrides type fields.
- The `Nullable<T>` type's `operator==` handles both null state and value comparison.
- Simpler comparison since font properties are now individual fields.
- No need to compare `antialias` properties as they're not in the overrides struct.

## STEP 10: Add necessary includes to test file

**File**: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

**Location**: Check if includes are sufficient (around line 1-8).

**Changes**: Verify that these includes are present:
```cpp
#include "../../../Source/UnitTestUtilities/GuiUnitTestProtocol.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.h"
```

Also ensure the test file has access to `IGuiGraphicsParagraph::TextStyle` through the includes.

**Rationale**: The test file needs access to the `DocumentTextRunPropertyOverrides` type and `IGuiGraphicsParagraph::TextStyle` enum. The existing includes should be sufficient, but verify to ensure compilation.

# TEST PLAN

## Test Strategy

**Critical Principle**: This task is purely a refactoring - changing types without changing behavior. Therefore, the success criteria is that **ALL existing test cases pass without any modification to their logic**.

The existing test suite in `TestRemote_DocumentRunManagement.cpp` already comprehensively covers:
- CaretRange comparison operations
- AddTextRun functionality (various overlap scenarios, merging behavior)
- AddInlineObjectRun functionality
- ResetInlineObjectRun functionality
- MergeRuns functionality (priority handling, inline object interactions)
- DiffRuns functionality

## Verification Approach

### Phase 1: Compilation Verification
1. Build the project after all changes
2. Verify no compilation errors
3. Verify no type mismatch warnings

### Phase 2: Existing Test Execution
Run all existing test categories and verify they pass:

1. **CaretRange Tests** (7 test cases)
   - Tests for range comparison operations
   - Should pass unchanged as CaretRange is not modified

2. **AddTextRun Tests** (approximately 15+ test cases including):
   - Add to empty map
   - Add non-overlapping runs
   - Adjacent runs (with/without merge)
   - Complete overlap
   - Partial overlaps (left, right)
   - New run contains/contained by old run
   - Multiple run overlaps
   - Merge scenarios (left, right, both neighbors)
   
   **Expected behavior**: All tests should pass unchanged because:
   - `CreateTextProp()` creates fully-populated overrides (all fields non-null)
   - `AreEqual()` compares all fields including nullability
   - Splitting and merging logic is identical
   - Since all fields are always populated, behavior is identical to before

3. **AddInlineObjectRun Tests**
   - Should pass unchanged as this function is not modified

4. **ResetInlineObjectRun Tests**
   - Should pass unchanged as this function is not modified

5. **MergeRuns Tests** (multiple test cases):
   - Text-only runs
   - Inline-only runs
   - Mixed text and inline runs
   - Inline object priority (overlaps text)
   
   **Expected behavior**: All tests should pass because:
   - Conversion from overrides to full properties extracts all values
   - Default antialias values (true) are reasonable and shouldn't affect test logic
   - All existing test assertions remain valid

6. **DiffRuns Tests**
   - Should pass unchanged as input type is `DocumentRunPropertyMap` which hasn't changed

## Test Coverage Analysis

### What's Already Covered
The existing test suite already covers:
- ✅ Empty map operations
- ✅ Single run operations
- ✅ Adjacent run merging with identical properties
- ✅ Adjacent runs with different properties (no merge)
- ✅ Various overlap scenarios
- ✅ Multi-run overlaps
- ✅ Merge with left neighbor only
- ✅ Merge with right neighbor only
- ✅ Merge with both neighbors
- ✅ Inline object priority over text runs
- ✅ Range coverage validation

### What's NOT Covered (Yet) - By Design
The following scenarios are intentionally NOT covered in this task because we're not implementing nullable semantics yet:
- Comparing overrides where some fields are null vs non-null
- Merging behavior when nullable fields differ in null state
- Applying overrides with null fields (the "keep existing" semantic)

These will be covered in future tasks (Task 9 and Task 10) when nullable semantics are implemented.

## New Test Cases: NONE

**Rationale**: This is a pure refactoring task. We deliberately avoid adding new test cases because:
1. All existing tests should pass without modification, proving behavior is preserved
2. New test cases would test nullable semantics, which is explicitly out of scope for this task
3. Adding tests for nullable behavior now would fail, as that functionality isn't implemented yet

## Success Criteria

✅ **Compilation**: Project builds without errors or warnings
✅ **All Existing Tests Pass**: Every single test in `TestRemote_DocumentRunManagement.cpp` passes unchanged
✅ **No Test Logic Changes**: Test case logic (assertions, expected values) remains identical
✅ **Helper Functions Updated**: Only mechanical type changes to `CreateTextProp`, `FormatRunProperty`, `CompareRunProperty`
✅ **Behavior Preservation**: Output of AddTextRun and MergeRuns is identical to before the changes

## Test Execution Command

Run the test executable after building:
```
Build Unit Tests task
Run Unit Tests task
```

Expected output: All test categories pass with no failures.

## Debugging Strategy

If any test fails:

1. **Compilation Errors**: 
   - Check that all type changes are consistent (header and cpp files)
   - Verify includes are correct
   - Check for missing namespace qualifications

2. **AddTextRun Test Failures**:
   - Verify `AreEqual` compares all fields correctly
   - Check that nullable comparison works as expected
   - Confirm that `CreateTextProp` initializes all fields (non-null)
   - Ensure variable types in `AddTextRun` implementation are updated

3. **MergeRuns Test Failures**:
   - Verify conversion from overrides to full property extracts values correctly
   - Check TextStyle flag conversion (enum to bool fields)
   - Confirm antialias defaults don't break assertions
   - Ensure all conversion points in the function are updated

4. **Comparison Failures**:
   - Print actual vs expected maps using existing `PrintMap` helper
   - Compare field-by-field to identify which property differs
   - Check if nullability is being compared correctly

# !!!FINISHED!!!
