# !!!PLANNING!!!

# UPDATES

# IMPROVEMENT PLAN

## STEP 1: Update ConvertToFullProperty to handle nullable properties

**Location**: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

**Current implementation**:
```cpp
remoteprotocol::DocumentTextRunProperty ConvertToFullProperty(const DocumentTextRunPropertyOverrides& propertyOverrides)
{
	remoteprotocol::DocumentTextRunProperty fullProp;
	fullProp.textColor = propertyOverrides.textColor.Value();
	fullProp.backgroundColor = propertyOverrides.backgroundColor.Value();
	fullProp.fontProperties.fontFamily = propertyOverrides.fontFamily.Value();
	fullProp.fontProperties.size = propertyOverrides.size.Value();
	
	// Convert TextStyle enum flags to individual bool fields
	auto style = propertyOverrides.textStyle.Value();
	fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != (IGuiGraphicsParagraph::TextStyle)0;
	fullProp.fontProperties.italic = (style & IGuiGraphicsParagraph::TextStyle::Italic) != (IGuiGraphicsParagraph::TextStyle)0;
	fullProp.fontProperties.underline = (style & IGuiGraphicsParagraph::TextStyle::Underline) != (IGuiGraphicsParagraph::TextStyle)0;
	fullProp.fontProperties.strikeline = (style & IGuiGraphicsParagraph::TextStyle::Strikeline) != (IGuiGraphicsParagraph::TextStyle)0;
	
	// Set default values for antialias properties
	fullProp.fontProperties.antialias = true;
	fullProp.fontProperties.verticalAntialias = true;
	
	return fullProp;
}
```

**Problem**: Calls `.Value()` unconditionally on all nullable fields, which will crash if any property is null.

**New implementation**:
```cpp
remoteprotocol::DocumentTextRunProperty ConvertToFullProperty(const DocumentTextRunPropertyOverrides& propertyOverrides)
{
	remoteprotocol::DocumentTextRunProperty fullProp;
	
	// Required properties - validate non-null
	CHECK_ERROR(propertyOverrides.fontFamily, L"fontFamily must be defined");
	CHECK_ERROR(propertyOverrides.size, L"size must be defined");
	
	fullProp.fontProperties.fontFamily = propertyOverrides.fontFamily.Value();
	fullProp.fontProperties.size = propertyOverrides.size.Value();
	
	// Optional properties - use defaults if null
	fullProp.textColor = propertyOverrides.textColor ? 
		propertyOverrides.textColor.Value() : Color(0, 0, 0);
	fullProp.backgroundColor = propertyOverrides.backgroundColor ? 
		propertyOverrides.backgroundColor.Value() : Color(0, 0, 0);
	
	auto style = propertyOverrides.textStyle ? 
		propertyOverrides.textStyle.Value() : (IGuiGraphicsParagraph::TextStyle)0;
	
	// Convert TextStyle enum flags to individual bool fields
	fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != (IGuiGraphicsParagraph::TextStyle)0;
	fullProp.fontProperties.italic = (style & IGuiGraphicsParagraph::TextStyle::Italic) != (IGuiGraphicsParagraph::TextStyle)0;
	fullProp.fontProperties.underline = (style & IGuiGraphicsParagraph::TextStyle::Underline) != (IGuiGraphicsParagraph::TextStyle)0;
	fullProp.fontProperties.strikeline = (style & IGuiGraphicsParagraph::TextStyle::Strikeline) != (IGuiGraphicsParagraph::TextStyle)0;
	
	// Set default values for antialias properties
	fullProp.fontProperties.antialias = true;
	fullProp.fontProperties.verticalAntialias = true;
	
	return fullProp;
}
```

**Why this change is necessary**:
1. **Required field validation**: The protocol layer (`ElementDesc_DocumentParagraph`) requires `fontFamily` and `size` for layout calculation. MergeRuns must ensure these are always defined before converting to protocol format. `CHECK_ERROR` will catch violations early.

2. **Default value application**: The optional properties (`textColor`, `backgroundColor`, `textStyle`) have sensible defaults. Black color is a reasonable default for text, and no styles (value 0) is appropriate when style is unspecified. This allows text runs to exist with only font information defined.

3. **Conditional value extraction**: The ternary operator `property ? property.Value() : default` safely checks if the nullable has a value before extracting it. This prevents crashes from calling `.Value()` on null.

4. **Design consistency**: This matches the established pattern from Task 8 where null represents a valid state. In MergeRuns, null means "use default value" rather than AddTextRun's "keep existing value" semantic.

## STEP 2: Update FormatRunProperty to display nullable properties

**Location**: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

**Current implementation**:
```cpp
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

**Problem**: Calls `.Value()` unconditionally, which will crash when displaying test results if any property is null.

**New implementation**:
```cpp
WString FormatRunProperty(const DocumentTextRunPropertyOverrides& prop)
{
	WString colorStr = prop.textColor ? 
		(itow(prop.textColor.Value().r) + L"," + 
		 itow(prop.textColor.Value().g) + L"," + 
		 itow(prop.textColor.Value().b)) :
		L"<null>";
	
	WString bgColorStr = prop.backgroundColor ? 
		(itow(prop.backgroundColor.Value().r) + L"," + 
		 itow(prop.backgroundColor.Value().g) + L"," + 
		 itow(prop.backgroundColor.Value().b)) :
		L"<null>";
	
	WString fontStr = prop.fontFamily ? 
		prop.fontFamily.Value() : 
		L"<null>";
	
	WString sizeStr = prop.size ? 
		itow(prop.size.Value()) : 
		L"<null>";
	
	WString styleStr = prop.textStyle ? 
		itow((vint)prop.textStyle.Value()) : 
		L"<null>";
	
	return L"Text(color:" + colorStr + 
		   L", bg:" + bgColorStr + 
		   L", font:" + fontStr + 
		   L", size:" + sizeStr + 
		   L", style:" + styleStr + L")";
}
```

**Why this change is necessary**:
1. **Test debugging**: When test cases fail, the helper function prints actual vs expected maps. If any property is null, the current implementation crashes during test output, preventing developers from seeing what went wrong.

2. **Explicit null display**: Using `<null>` makes it immediately obvious which properties are undefined. This is crucial for debugging nullable property scenarios in future tasks (Task 11 and 12).

3. **Complete property display**: The new implementation includes all five properties (color, bgColor, font, size, style) for comprehensive debugging. The original only showed colors.

4. **Consistent pattern**: Each property uses the same conditional pattern: check if defined, extract value if so, otherwise use `<null>`. This makes the code easy to read and maintain.

**Note**: `CompareRunProperty(const DocumentTextRunPropertyOverrides&, const DocumentTextRunPropertyOverrides&)` already handles nullable comparison correctly via `Nullable<T>` operator==, so no changes are needed there.

## STEP 3: Verify AddTextRun already handles nullable comparison correctly

**Location**: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

**No changes needed to AddTextRun implementation**.

**Why**: The AddTextRun function uses the `AreEqual` helper function for property comparison during merging:

```cpp
if (leftKey.caretEnd == currentKey.caretBegin &&
	AreEqual(map[leftKey], map[currentKey]))
{
	// Merge runs
}
```

The `AreEqual` function compares properties using `Nullable<T>` operator==:

```cpp
bool AreEqual(const DocumentTextRunPropertyOverrides& a, const DocumentTextRunPropertyOverrides& b)
{
	return a.textColor == b.textColor &&
		   a.backgroundColor == b.backgroundColor &&
		   a.fontFamily == b.fontFamily &&
		   a.size == b.size &&
		   a.textStyle == b.textStyle;
}
```

The `Nullable<T>` operator== from Vlpp compares both null state and values:
- `null == null` → true
- `null == value` → false  
- `value == null` → false
- `value1 == value2` → compares the actual values

This means:
- Two runs with `color=null` will merge (both have same null state)
- Run with `color=null` and run with `color=Red` will NOT merge (different states)
- Run with `color=Red` and run with `color=null` will NOT merge (different states)
- Two runs with `color=Red` will merge (same value)

This is exactly the behavior specified in Task 9: "null is just another value" for comparison purposes.

# TEST PLAN

## Test Strategy

All existing test cases should pass without modification. This validates that the nullable property storage system works correctly for the non-null case (backward compatibility).

## Existing Test Cases Validation

### Task 2-7 Test Cases (AddTextRun, MergeRuns, DiffRuns, etc.)

All existing tests use the `CreateTextProp()` helper function:

```cpp
DocumentTextRunPropertyOverrides CreateTextProp(unsigned char colorValue)
{
	DocumentTextRunPropertyOverrides prop;
	prop.textColor = Color(colorValue, colorValue, colorValue);
	prop.backgroundColor = Color(255 - colorValue, 255 - colorValue, 255 - colorValue);
	prop.fontFamily = L"TestFont";
	prop.size = 12;
	prop.textStyle = (IGuiGraphicsParagraph::TextStyle)0;
	return prop;
}
```

This creates runs with **all properties defined (non-null)**.

**Expected behavior**:
1. **AddTextRun tests**: Continue to pass because merging logic uses `AreEqual`, which correctly compares non-null values via `Nullable<T>` operator==.

2. **MergeRuns tests**: Continue to pass because:
   - All properties are defined, so `CHECK_ERROR` validations pass
   - Conditional value extraction (`property ? property.Value() : default`) takes the Value() path since all properties are defined
   - The defaults are never used in these tests

3. **DiffRuns tests**: Continue to pass because they don't directly depend on nullable semantics—they compare runs using protocol types.

## What This Task Validates

If all existing tests pass after these changes, it proves:

1. **Required field validation works**: The `CHECK_ERROR` statements don't trigger false positives when all properties are defined.

2. **Default value application doesn't break existing behavior**: When all properties are defined, the conditional branches correctly extract values rather than using defaults.

3. **Nullable comparison works correctly**: The `AreEqual` function correctly compares non-null nullable values.

4. **Test infrastructure handles nullable display**: The `FormatRunProperty` function can display both null and non-null properties without crashing.

5. **The nullable storage system is ready for overriding logic**: The foundation is solid for Task 10 (overriding semantics) and Task 11-12 (comprehensive nullable tests).

## Key Test Scenarios

The existing test suite already covers:

1. **AddTextRun splitting**: When overlapping runs are added, splitting works correctly. No changes to this logic means it continues to work.

2. **AddTextRun merging**: Consecutive runs with identical properties merge. The `AreEqual` function ensures this works for nullable types.

3. **MergeRuns inline object priority**: When text runs and inline objects overlap, inline objects win. The `ConvertToFullProperty` helper is called for text runs, so default value application is exercised.

4. **DiffRuns change detection**: When runs change between old and new states, the diff correctly identifies them. This validates that nullable properties integrate correctly with the protocol layer.

## Regression Prevention

The test plan focuses on **non-regression**: all existing functionality must continue to work. This is the correct approach for Task 9 because:

1. **Incremental development**: Task 8 changed types, Task 9 adds nullable storage, Task 10 adds overriding semantics. Each step must validate the previous step still works.

2. **Low risk**: Only two functions change (`ConvertToFullProperty` and `FormatRunProperty`), and the changes are defensive (add null checks, don't change logic for non-null case).

3. **Clear success criteria**: If existing tests pass, the implementation is correct. If they fail, there's a bug in the nullable handling for the non-null case.

## Future Testing (Tasks 11-12)

Task 9 does NOT add new test cases. The comprehensive nullable property testing will be added in:

- **Task 11**: AddTextRun nullable scenarios (partial updates, null merging, layered formatting)
- **Task 12**: MergeRuns nullable scenarios (validation, default application, complex cases)

This separation follows the principle established in the task breakdown: get the infrastructure working first (Task 9), then test the new capabilities (Tasks 11-12).

# !!!FINISHED!!!
