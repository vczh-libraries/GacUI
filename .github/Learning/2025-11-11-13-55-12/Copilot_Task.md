# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.8: Define DocumentTextRunPropertyOverrides type and update signatures without nullable semantics

### description

Create the infrastructure for nullable property support by defining the `DocumentTextRunPropertyOverrides` type and updating function signatures, but WITHOUT implementing nullable property semantics yet. This task focuses on making the type system changes while keeping all behavior identical to the current implementation.

**Step 1: Define DocumentTextRunPropertyOverrides**

Create the struct in `GuiRemoteGraphics_Document.h` right after `CaretRange`:

```cpp
struct DocumentTextRunPropertyOverrides
{
	Nullable<Color> textColor;
	Nullable<Color> backgroundColor;
	Nullable<WString> fontFamily;
	Nullable<vint> size;
	Nullable<IGuiGraphicsParagraph::TextStyle> textStyle;
};
```

Note: Unlike `DocumentTextRunProperty` which has a `fontProperties` field of type `FontProperties`, the overrides struct expands font properties into individual nullable fields:
- `fontFamily` (from `FontProperties::fontFamily`)
- `size` (from `FontProperties::size`)
- `textStyle` (combines `bold`, `italic`, `underline`, `strikeline` as a single enum flag set)

**Step 2: Update DocumentTextRunPropertyMap type**

Change the map type alias to use overrides:
```cpp
using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, DocumentTextRunPropertyOverrides>;
```

**Step 3: Update AddTextRun signature**

Change signature:
```cpp
extern void AddTextRun(
	DocumentTextRunPropertyMap& map,
	CaretRange range,
	const DocumentTextRunPropertyOverrides& propertyOverrides);
```

**Step 4: Update MergeRuns signature**

No signature change needed (it already takes `const DocumentTextRunPropertyMap&`).

**Step 5: Implementation approach for this task**

**CRITICAL**: Do NOT implement nullable semantics yet. Instead:

1. **In AddTextRun**: 
   - Keep the exact same splitting and merging logic as before
   - When comparing properties for merging, compare all fields including nullability exactly
   - When applying properties during splits, just copy the property values as-is (null or not)
   - The behavior should be identical to before, just with a different type

2. **In MergeRuns**:
   - Keep the exact same merge logic as before
   - Just treat overrides as a struct with nullable fields
   - Convert to DocumentRunProperty by extracting values from Nullable<> wrappers
   - No special handling for null values yet (that comes in Task No.10)

**Step 6: Update test helper functions**

In `TestRemote_DocumentRunManagement.cpp`:
- Update `CreateTextProp()` to return `DocumentTextRunPropertyOverrides` with all fields defined (non-null)
- Update `CompareRunProperty()` to handle the overrides type
- Update `FormatRunProperty()` to display overrides type
- No changes needed to test logic - all existing tests should pass unchanged

### what to be done

1. Define `DocumentTextRunPropertyOverrides` struct in `GuiRemoteGraphics_Document.h`
2. Update `DocumentTextRunPropertyMap` type alias
3. Update `AddTextRun()` signature to use overrides
4. Update AddTextRun implementation to work with the new type (but no nullable semantics yet)
5. Update MergeRuns implementation to work with the new type (but no nullable semantics yet)
6. Update test helper functions to use the new type
7. Ensure all existing test cases pass without modification

### how to test it

Run the compiled unit test executable. All existing test cases should pass:
- Existing AddTextRun tests verify splitting and merging still work correctly
- Existing MergeRuns tests verify inline object priority still works
- Build succeeds (signature changes compile correctly)
- All test categories pass: CaretRange, AddTextRun, AddInlineObjectRun, ResetInlineObjectRun, MergeRuns, DiffRuns

### file locations

Modified files:
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.h`
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`
- `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

The previous attempt at Task 8 tried to do too much at once: change types, implement nullable storage, implement nullable semantics, and update tests. This led to confusion and failures.

By splitting into 3 incremental tasks:
1. **Task 8** (this task): Change the types and signatures, but keep behavior identical. This validates that the type system changes compile and don't break existing tests.
2. **Task 9**: Add nullable storage and comparison, treating null as a normal value. This validates the storage mechanism without complex overriding logic.
3. **Task 10**: Implement the actual overriding semantics (null = "keep existing" in AddTextRun, null = "use default" in MergeRuns).

This approach follows the principle of making one type of change at a time, making each step easier to debug and verify.

# UPDATES

# INSIGHTS AND REASONING

## Step 1: Analyzing Current Implementation

I've examined the existing code structure to understand what needs to be changed:

### Current Type Definitions (in `GuiRemoteGraphics_Document.h`)

```cpp
using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, remoteprotocol::DocumentTextRunProperty>;
using DocumentInlineObjectRunPropertyMap = collections::Dictionary<CaretRange, remoteprotocol::DocumentInlineObjectRunProperty>;
using DocumentRunPropertyMap = collections::Dictionary<CaretRange, remoteprotocol::DocumentRunProperty>;
```

The current implementation uses `remoteprotocol::DocumentTextRunProperty` directly, which is defined in the protocol schema as:

```cpp
struct DocumentTextRunProperty
{
	::vl::presentation::Color textColor;
	::vl::presentation::Color backgroundColor;
	::vl::presentation::FontProperties fontProperties;
};
```

Where `FontProperties` contains:
- `WString fontFamily`
- `vint size`
- `bool bold`, `italic`, `underline`, `strikeline`
- `bool antialias`, `verticalAntialias`

### Current Function Signatures

```cpp
extern void AddTextRun(
	DocumentTextRunPropertyMap& map,
	CaretRange range,
	const remoteprotocol::DocumentTextRunProperty& property);
```

### Test Helper Functions (in `TestRemote_DocumentRunManagement.cpp`)

The test file has well-structured helper functions:
- `CreateTextProp(unsigned char colorValue)` - creates a test property with all fields defined
- `FormatRunProperty()` - overloaded for text/inline/run properties
- `CompareRunProperty()` - overloaded for text/inline/run properties
- `PrintMap()` and `AssertMap()` - template functions for map comparison

## Step 2: Design Approach for Task 8

The goal is to introduce the `DocumentTextRunPropertyOverrides` type WITHOUT changing behavior. This is a pure refactoring task to prepare for nullable semantics in later tasks.

### Key Design Decisions

1. **New Struct Definition**: Create `DocumentTextRunPropertyOverrides` with nullable fields:
   - Expand `FontProperties` into individual nullable fields
   - Use `Nullable<Color>` instead of `Color`
   - Use `Nullable<IGuiGraphicsParagraph::TextStyle>` for style flags

2. **Type Alias Update**: Change `DocumentTextRunPropertyMap` to use the new overrides type

3. **Signature Update**: Change `AddTextRun` to accept `DocumentTextRunPropertyOverrides`

4. **Implementation Strategy**: 
   - In `AddTextRun`: Treat the overrides struct as-is, compare all fields for merging including nullability
   - In `MergeRuns`: Extract values from `Nullable<>` wrappers, convert to `DocumentRunProperty`
   - **Critical**: No special nullable semantics - null is just a value at this stage

5. **Test Updates**: Minimal changes
   - Update `CreateTextProp()` to return `DocumentTextRunPropertyOverrides` with all fields non-null
   - Update helper functions to work with the new type
   - All existing test logic remains unchanged

## Step 3: Anticipated Implementation Changes

### In GuiRemoteGraphics_Document.h

Add after `CaretRange` struct:
```cpp
struct DocumentTextRunPropertyOverrides
{
	Nullable<Color> textColor;
	Nullable<Color> backgroundColor;
	Nullable<WString> fontFamily;
	Nullable<vint> size;
	Nullable<IGuiGraphicsParagraph::TextStyle> textStyle;
};
```

Change type alias:
```cpp
using DocumentTextRunPropertyMap = collections::Dictionary<CaretRange, DocumentTextRunPropertyOverrides>;
```

Change function signature:
```cpp
extern void AddTextRun(
	DocumentTextRunPropertyMap& map,
	CaretRange range,
	const DocumentTextRunPropertyOverrides& propertyOverrides);
```

### In GuiRemoteGraphics_Document.cpp

Update `AreEqual` function to compare `DocumentTextRunPropertyOverrides`:
```cpp
bool AreEqual(const DocumentTextRunPropertyOverrides& a, const DocumentTextRunPropertyOverrides& b)
{
	// Compare all fields including nullability
	// Two overrides are equal only if all nullable fields match exactly
	return a.textColor == b.textColor &&
		   a.backgroundColor == b.backgroundColor &&
		   a.fontFamily == b.fontFamily &&
		   a.size == b.size &&
		   a.textStyle == b.textStyle;
}
```

Update `AddTextRun` to work with the new type (behavior remains identical).

Update `MergeRuns` to convert overrides to `DocumentRunProperty`:
```cpp
// For each text run, extract values from Nullable<> wrappers
DocumentTextRunProperty fullProp;
fullProp.textColor = currentTextProperty.textColor.Value();
fullProp.backgroundColor = currentTextProperty.backgroundColor.Value();
fullProp.fontProperties.fontFamily = currentTextProperty.fontFamily.Value();
fullProp.fontProperties.size = currentTextProperty.size.Value();
// Convert TextStyle enum to individual bool flags
auto style = currentTextProperty.textStyle.Value();
fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != 0;
fullProp.fontProperties.italic = (style & IGuiGraphicsParagraph::TextStyle::Italic) != 0;
fullProp.fontProperties.underline = (style & IGuiGraphicsParagraph::TextStyle::Underline) != 0;
fullProp.fontProperties.strikeline = (style & IGuiGraphicsParagraph::TextStyle::Strikeline) != 0;
// antialias and verticalAntialias - need to decide on default values
```

### In TestRemote_DocumentRunManagement.cpp

Update `CreateTextProp`:
```cpp
DocumentTextRunPropertyOverrides CreateTextProp(unsigned char colorValue)
{
	DocumentTextRunPropertyOverrides prop;
	prop.textColor = Color(colorValue, colorValue, colorValue);
	prop.backgroundColor = Color(255 - colorValue, 255 - colorValue, 255 - colorValue);
	prop.fontFamily = L"TestFont";
	prop.size = 12;
	// Create TextStyle with no flags set
	prop.textStyle = (IGuiGraphicsParagraph::TextStyle)0;
	return prop;
}
```

Update comparison and formatting functions to handle the new type.

## Step 4: Risk Assessment

### Low Risk Areas
- Type definition: straightforward struct creation
- Test helper updates: mechanical changes
- Signature changes: compile-time verified

### Medium Risk Areas
- `AreEqual` implementation: Must compare nullability correctly
- `MergeRuns` conversion: Must handle TextStyle bit flags correctly
- **Missing information**: What default values should be used for `antialias` and `verticalAntialias`?

### High Risk Areas
- `AddTextRun` logic: Complex splitting and merging logic must remain identical
- **Concern**: Need to ensure that when comparing for merging, null values are treated as distinct from defined values

## Step 5: Questions and Clarifications Needed

1. **Default values for antialias/verticalAntialias**: These fields are in `FontProperties` but not in the overrides struct. What defaults should MergeRuns use?
   - **Proposed solution**: Use `true` for both (typical default for anti-aliasing)

2. **TextStyle flag conversion**: Need to ensure conversion between the enum flags and individual bool fields is correct in both directions.

3. **Nullable comparison**: The `Nullable<T>` type should have proper `==` operator. Need to verify this works correctly.

## Step 6: Implementation Plan

1. **Add new struct definition** to header file
2. **Update type aliases** in header file
3. **Update function signature** for AddTextRun in header file
4. **Implement AreEqual for overrides** in cpp file
5. **Update AddTextRun implementation** to use new type (minimal changes)
6. **Update MergeRuns implementation** to convert overrides to full properties
7. **Update test helpers** to use new type
8. **Verify all tests pass** without modification to test logic

## Step 7: Success Criteria

✅ Code compiles without errors
✅ All existing test cases pass unchanged
✅ No behavioral changes in AddTextRun (same splitting/merging results)
✅ No behavioral changes in MergeRuns (same output)
✅ Clean separation of concerns: type change only, no semantic changes

This task sets up the infrastructure for nullable properties while maintaining 100% backward compatibility, making it safe and verifiable.

# !!!FINISHED!!!
