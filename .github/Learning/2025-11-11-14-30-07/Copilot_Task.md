# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.9: Implement nullable property storage and comparison (no overriding logic)

### description

Now that the type system supports nullable properties (Task No.8), implement nullable property storage and comparison. Treat null as a valid value that can be stored and compared, but do NOT implement any special "overriding" semantics yet.

**Key principle**: In this task, null is just another value. Two runs with `color=null` are considered to have the same color. A run with `color=null` is different from a run with `color=Red`.

**Learning from Task No.8**: The incremental approach (type changes first, then semantics) worked well. Task 9 continues this pattern by adding nullable storage without overriding logic. This maintains the testability and verifiability that made Task 8 successful.

**Step 1: Update AddTextRun implementation**

1. **Property comparison for merging**: When determining if two consecutive runs can merge, compare all properties including null values as regular values:
   - `color=null` equals `color=null` ✓
   - `color=Red` equals `color=Red` ✓
   - `color=null` does NOT equal `color=Red` ✗
   - `color=Red` does NOT equal `color=null` ✗

2. **Property storage during splits**: When splitting runs, just copy property values directly (null or not). No special "apply overrides" logic yet.

**Important**: AddTextRun already handles nullable comparison correctly via the updated `AreEqual` function from Task 8. The `Nullable<T>` operator== compares both null state and values. No changes needed to AddTextRun implementation in this task.

**Step 2: Update MergeRuns implementation**

Update the existing `ConvertToFullProperty` helper function to handle null values:

1. For text runs with null properties, provide default values:
   - `textColor`: if null, use Black `Color(0, 0, 0)`
   - `backgroundColor`: if null, use Black `Color(0, 0, 0)`
   - `textStyle`: if null, use `(TextStyle)0` (no styles)
   - `fontFamily` and `size`: must be non-null (validate with CHECK_ERROR)

2. Implementation approach:
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
	fullProp.textColor = propertyOverrides.textColor ? propertyOverrides.textColor.Value() : Color(0, 0, 0);
	fullProp.backgroundColor = propertyOverrides.backgroundColor ? propertyOverrides.backgroundColor.Value() : Color(0, 0, 0);
	
	auto style = propertyOverrides.textStyle ? propertyOverrides.textStyle.Value() : (IGuiGraphicsParagraph::TextStyle)0;
	fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != (IGuiGraphicsParagraph::TextStyle)0;
	// ... rest of style conversion unchanged ...
	
	return fullProp;
}
```

**Step 3: Update test helpers**

Update helper functions to handle nullable display:

1. `FormatRunProperty(const DocumentTextRunPropertyOverrides&)`: Display null values as `<null>`
```cpp
WString FormatRunProperty(const DocumentTextRunPropertyOverrides& prop)
{
	WString colorStr = prop.textColor ? 
		(itow(prop.textColor.Value().r) + L"," + itow(prop.textColor.Value().g) + L"," + itow(prop.textColor.Value().b)) :
		L"<null>";
	// ... similar for other fields ...
}
```

2. `CompareRunProperty` already handles nullable comparison correctly via operator==, no changes needed.

**Step 4: Verify existing tests**

All existing tests create runs with all properties defined (non-null via `CreateTextProp`), so they should continue to pass without modification.

### what to be done

1. Update `ConvertToFullProperty` helper function in `GuiRemoteGraphics_Document.cpp`:
   - Add validation for required fields (fontFamily, size)
   - Add default value application for optional fields (textColor, backgroundColor, textStyle)
2. Update `FormatRunProperty` in test file to display null values as `<null>`
3. No changes needed to AddTextRun (already handles nullable comparison)
4. No changes needed to `CompareRunProperty` (already handles nullable comparison)
5. Verify all existing test cases pass

### how to test it

Run the compiled unit test executable. All existing test cases should pass:
- AddTextRun tests verify merging considers null as a distinct value (via existing `AreEqual`)
- MergeRuns tests verify default value application and validation
- No new tests needed in this task (covered in Task No.11 and No.12)

**Key validation**: If existing tests pass, it proves:
1. Required field validation works
2. Default value application doesn't break existing behavior
3. The nullable system is ready for overriding logic (Task 10)

### file locations

Modified files:
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp` (update `ConvertToFullProperty` only)
- `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp` (update `FormatRunProperty` only)

### rationale

This task establishes the nullable value system without the complexity of overriding semantics. By treating null as "just another value" and focusing changes on a single helper function (`ConvertToFullProperty`), we:
1. Minimize risk - only one function changes behavior
2. Maintain testability - existing tests validate the changes
3. Validate the nullable storage mechanism works correctly
4. Set up proper validation for required fields (fontFamily, size)

**Learning from Task 8**: The helper function extraction (ConvertToFullProperty) makes this task much simpler. Instead of updating three duplicated code blocks, we update one function. This validates that the refactoring requested by the user improved maintainability.

This creates a stable foundation before adding the more complex overriding logic in Task No.10. If existing tests fail at this stage, it's easier to debug because we're only changing default value handling in one place.

# UPDATES

# INSIGHTS AND REASONING

## Task Summary

Task No.9 continues the incremental nullable property implementation that began with Task No.8. This task implements nullable property storage and comparison, treating null as a valid value (not yet as an "override" semantic). The key principle: null is just another value that can be stored and compared.

## Source Code Analysis

### Current State After Task No.8

From `GuiRemoteGraphics_Document.cpp`:

**1. DocumentTextRunPropertyOverrides struct** (already defined in Task No.8):
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

**2. AreEqual function** (already handles nullable comparison):
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

This function already uses `Nullable<T>` operator== which compares both null state and values correctly:
- `null == null` → true
- `null == value` → false
- `value1 == value2` → compares values

**3. ConvertToFullProperty function** (needs updates):
```cpp
remoteprotocol::DocumentTextRunProperty ConvertToFullProperty(const DocumentTextRunPropertyOverrides& propertyOverrides)
{
    remoteprotocol::DocumentTextRunProperty fullProp;
    fullProp.textColor = propertyOverrides.textColor.Value();           // CRASHES if null
    fullProp.backgroundColor = propertyOverrides.backgroundColor.Value(); // CRASHES if null
    fullProp.fontProperties.fontFamily = propertyOverrides.fontFamily.Value(); // CRASHES if null
    fullProp.fontProperties.size = propertyOverrides.size.Value();      // CRASHES if null
    
    auto style = propertyOverrides.textStyle.Value();                    // CRASHES if null
    // ... rest of style conversion ...
    
    return fullProp;
}
```

Currently calls `.Value()` unconditionally, which will crash if any property is null.

**4. Test helper FormatRunProperty** (needs updates):
```cpp
WString FormatRunProperty(const DocumentTextRunPropertyOverrides& prop)
{
    return L"Text(color:" + 
           itow(prop.textColor.Value().r) + L"," +    // CRASHES if null
           // ...
}
```

Also calls `.Value()` unconditionally.

### What AddTextRun Already Does Correctly

The AddTextRun function already handles nullable comparison correctly:

1. When merging consecutive runs, it calls `AreEqual(map[leftKey], map[currentKey])`
2. `AreEqual` uses `Nullable<T>` operator== for all properties
3. This correctly treats null as a distinct value:
   - Two runs with `color=null` will merge
   - Run with `color=null` and run with `color=Red` will NOT merge

**Conclusion**: No changes needed to AddTextRun for this task.

## Design Solution

### Solution: Update ConvertToFullProperty and FormatRunProperty

This is a minimal, focused change following the single-responsibility principle established in Task No.8.

#### Change 1: Update ConvertToFullProperty

**Location**: `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`

**Current behavior**: Crashes if any nullable property is null.

**New behavior**: 
1. Validate required properties (`fontFamily`, `size`) are non-null using `CHECK_ERROR`
2. Apply default values for optional properties if null:
   - `textColor`: Black `Color(0, 0, 0)`
   - `backgroundColor`: Black `Color(0, 0, 0)`
   - `textStyle`: `(IGuiGraphicsParagraph::TextStyle)0` (no styles)

**Implementation approach**:
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
    
    // Convert TextStyle enum flags to individual bool fields (unchanged)
    fullProp.fontProperties.bold = (style & IGuiGraphicsParagraph::TextStyle::Bold) != (IGuiGraphicsParagraph::TextStyle)0;
    fullProp.fontProperties.italic = (style & IGuiGraphicsParagraph::TextStyle::Italic) != (IGuiGraphicsParagraph::TextStyle)0;
    fullProp.fontProperties.underline = (style & IGuiGraphicsParagraph::TextStyle::Underline) != (IGuiGraphicsParagraph::TextStyle)0;
    fullProp.fontProperties.strikeline = (style & IGuiGraphicsParagraph::TextStyle::Strikeline) != (IGuiGraphicsParagraph::TextStyle)0;
    
    // Set default antialias values (unchanged)
    fullProp.fontProperties.antialias = true;
    fullProp.fontProperties.verticalAntialias = true;
    
    return fullProp;
}
```

**Rationale for required vs optional**:
- `fontFamily` and `size` are required by the protocol layer (`ElementDesc_DocumentParagraph`) for layout calculation
- `textColor`, `backgroundColor`, `textStyle` have sensible defaults (Black, Black, no styles)

#### Change 2: Update FormatRunProperty for Test Display

**Location**: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

**Current behavior**: Crashes when displaying null properties.

**New behavior**: Display null values as `<null>` for debugging clarity.

**Implementation approach**:
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

**Note**: The `CompareRunProperty` function doesn't need changes because it already uses `operator==` on `Nullable<T>` types, which correctly compares null states.

## Validation Strategy

### Why Existing Tests Should Pass

All existing test cases (Tasks No.2-7) create runs using `CreateTextProp()` helper:
```cpp
DocumentTextRunPropertyOverrides CreateTextProp(unsigned char colorValue)
{
    DocumentTextRunPropertyOverrides prop;
    prop.textColor = Color(colorValue, colorValue, colorValue);           // Non-null
    prop.backgroundColor = Color(255 - colorValue, 255 - colorValue, 255 - colorValue); // Non-null
    prop.fontFamily = L"TestFont";                                        // Non-null
    prop.size = 12;                                                       // Non-null
    prop.textStyle = (IGuiGraphicsParagraph::TextStyle)0;                // Non-null
    return prop;
}
```

All properties are explicitly set to non-null values. Therefore:

1. **AddTextRun tests**: Continue to work unchanged (merging logic already correct for nullable comparison)
2. **MergeRuns tests**: Continue to work because `ConvertToFullProperty` gets all-defined properties
3. **AddInlineObjectRun/ResetInlineObjectRun tests**: Unaffected (don't use text properties)
4. **DiffRuns tests**: Unaffected (operates on `DocumentRunPropertyMap`, not overrides)

### What This Task Validates

If all existing tests pass after these changes, it proves:

1. **Required field validation works**: The CHECK_ERROR statements don't trigger false positives
2. **Default value application doesn't break existing behavior**: When all properties are defined, the conditional `? Value() : default` branches correctly take the Value() path
3. **The nullable storage system is ready for overriding logic**: The foundation is solid for Task No.10

## Impact Summary

**Files Modified**:
1. `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp` - Update `ConvertToFullProperty` only
2. `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp` - Update `FormatRunProperty` for overrides only

**Files NOT Modified**:
- No changes to AddTextRun (already works correctly)
- No changes to MergeRuns algorithm (only helper function changes)
- No changes to test cases (they should pass as-is)
- No changes to other run management functions

**Risk Assessment**: **LOW**
- Minimal changes (2 functions)
- Changes are isolated to helper functions
- Existing tests provide comprehensive validation
- Clear success criteria (all tests pass)

## Next Steps

After Task No.9 completes:
- **Task No.10**: Implement nullable property overriding logic (null = "keep existing" in AddTextRun)
- **Task No.11**: Add comprehensive test cases for AddTextRun nullable scenarios
- **Task No.12**: Add comprehensive test cases for MergeRuns nullable scenarios

The incremental approach ensures each step is small, verifiable, and builds on a solid foundation.

# !!!FINISHED!!!
