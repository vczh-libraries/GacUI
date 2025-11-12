# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.12: Add comprehensive test cases for nullable property scenarios in MergeRuns

### description

Add new test cases to `TestRemote_DocumentRunManagement.cpp` in a new `TEST_CATEGORY("MergeRuns (partial application)")` to thoroughly test the nullable property functionality added in Task No.8, No.9, and No.10.

**Test approach**:
- Add new TEST_CASE blocks in a new TEST_CATEGORY("MergeRuns (partial application)") after the existing TEST_CATEGORY("MergeRuns")
- Use helper functions that create text run maps with nullable properties
- Verify nullable property semantics: null = use default in MergeRuns

**New test cases for MergeRuns**:

1. **Mandatory property validation**:
   - Text run with null fontFamily → should fail assertion or produce error
   - Text run with null size → should fail assertion or produce error
   - Text run with both fontFamily and size defined → should succeed

2. **Default value application**:
   - Text run with null textColor → verify result uses Black (0,0,0)
   - Text run with null backgroundColor → verify result uses Black (0,0,0)
   - Text run with null textStyle → verify result uses (TextStyle)0
   - Text run with all optional properties null → verify result has defaults

3. **Mixing defined and null properties**:
   - Text run [(0,10) fontFamily="Arial", size=12, color=null] → verify result has Black color
   - Inline object run + text run with nulls → verify defaults applied, inline object priority preserved

4. **Complex nullable scenarios**:
   - Multiple text runs with different null patterns merging with inline objects
   - Ensure gap filling and splitting still work correctly with nullable properties
   - Verify inline object priority is preserved regardless of null properties in text runs

### what to be done

1. Add a new TEST_CATEGORY("MergeRuns (partial application)") after the existing TEST_CATEGORY("MergeRuns")
2. Add new test cases to the new category:
   - Test mandatory property validation (2-3 test cases)
   - Test default value application (3-4 test cases)
   - Test mixing defined and null properties (3-5 test cases)
   - Test complex nullable scenarios (2-3 test cases)
3. Each test case should:
   - Create DocumentTextRunPropertyMap with nullable properties
   - Call MergeRuns to produce DocumentRunPropertyMap result
   - Use AssertMap to verify result has correct default values for null properties
   - Verify mandatory properties (fontFamily, size) are validated
4. Test cases should verify that inline object priority and text run splitting work correctly even with nullable properties

### how to test it

Run the compiled unit test executable. The new test cases validate:
1. MergeRuns correctly enforces mandatory properties (fontFamily, size)
2. MergeRuns correctly applies default values for optional null properties
3. Complex combinations of nullable properties work correctly in MergeRuns
4. The nullable property system integrates correctly with inline object priority rules

Success criteria:
- All new test cases pass
- Test coverage includes validation of mandatory properties
- Default value application is correct for all optional properties
- Inline object priority is preserved with nullable text properties

### file locations

Modified file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

Task No.8, No.9, and No.10 implement the nullable property mechanism for MergeRuns but maintain backward compatibility by only testing with fully-defined properties. This task validates that MergeRuns correctly handles nullable properties according to its specific requirements:

1. **Protocol requirements**: MergeRuns must ensure fontFamily and size are always defined because the protocol layer needs complete FontProperties. This validation must be tested.

2. **Default value semantics**: Unlike AddTextRun where null means "keep existing", in MergeRuns null means "use default". This different semantic must be thoroughly verified.

3. **Integration with inline objects**: The inline object priority system must work correctly even when text runs have nullable properties. This prevents regressions in the complex merge logic.

Without comprehensive testing of nullable scenarios for MergeRuns, we risk:
- Missing validation allowing incomplete properties to reach the protocol layer
- Incorrect default values being applied
- Breaking inline object priority when text runs have nulls

This task focuses specifically on MergeRuns behavior, complementing Task No.11's focus on AddTextRun. Splitting these into two tasks allows for thorough, focused testing of each function's unique nullable property semantics. The separate test category "MergeRuns (partial application)" matches the organizational pattern established in Task No.11, making it clear that these tests validate nullable property behavior specifically.

# UPDATES

# INSIGHTS AND REASONING

## Analysis of Current State

From reading the test file `TestRemote_DocumentRunManagement.cpp`, I can see that:

1. **Test Infrastructure Already Exists**: The file has comprehensive helper functions for testing:
   - `CreateTextProp()`: Creates fully-defined properties
   - `CreateTextPropPartial()`: Creates properties with selective nullable fields
   - `CreateTextPropWithFont()`, `CreateTextPropWithSize()`, etc.: Convenience functions for single-property overrides
   - `CreateFullTextProp()`: Creates the full `DocumentTextRunProperty` (non-override version)
   - `AssertMap()`, `FormatRunProperty()`, `CompareRunProperty()`: Helper functions for test assertions
   
2. **Existing Test Structure**: There are several test categories:
   - `CaretRange`: Tests for range comparison operators
   - `AddTextRun`: Tests for text run addition with splitting and merging
   - `AddTextRun (partial application)`: Tests for nullable property handling in AddTextRun
   - `AddInlineObjectRun`: Tests for inline object addition
   - `ResetInlineObjectRun`: Tests for inline object removal
   - `MergeRuns`: Tests for basic merge functionality (text+inline priority)
   - `DiffRuns`: Tests for basic diff computation
   - `DiffRuns (Complex)`: Tests for complex diff scenarios

3. **Key Observation**: The `MergeRuns` test category exists but only tests basic scenarios with fully-defined properties. There is NO test category for nullable property handling in `MergeRuns`, which is what this task requires.

4. **MergeRuns Function Signature**: Based on the tests, `MergeRuns` takes:
   - `const DocumentTextRunPropertyMap&` (text runs with overrides)
   - `const DocumentInlineObjectRunPropertyMap&` (inline objects)
   - `DocumentRunPropertyMap&` (output with full `DocumentRunProperty`)

5. **Key Semantic Difference**: From Task No.9 and No.10 descriptions:
   - **AddTextRun**: null = "keep existing value" (overriding semantics)
   - **MergeRuns**: null = "use default value" (protocol conversion semantics)
   - MergeRuns requires `fontFamily` and `size` to be non-null (mandatory for protocol)

## Design Approach

### Test Category Structure

Following the established pattern from `AddTextRun (partial application)`, I will create a new test category:

```cpp
TEST_CATEGORY(L"MergeRuns (partial application)")
{
    // Test cases here
}
```

This category will be added after the existing `TEST_CATEGORY(L"MergeRuns")`.

### Test Cases to Implement

Based on the task requirements, I'll organize tests into 4 groups:

#### Group 1: Mandatory Property Validation (2-3 test cases)

These tests verify that `MergeRuns` enforces the requirement that `fontFamily` and `size` must be defined:

1. **Test: Null fontFamily triggers validation error**
   - Create text run with `fontFamily=null`, `size=12`
   - Expect: `CHECK_ERROR` or assertion failure
   - Use `TEST_ERROR()` macro to verify error is raised

2. **Test: Null size triggers validation error**
   - Create text run with `fontFamily="Arial"`, `size=null`
   - Expect: `CHECK_ERROR` or assertion failure
   - Use `TEST_ERROR()` macro

3. **Test: Both fontFamily and size defined succeeds**
   - Create text run with both `fontFamily="Arial"` and `size=12` defined
   - Other properties can be null
   - Verify: MergeRuns succeeds and produces expected output

**Note**: Looking at the implementation from Task No.9, `ConvertToFullProperty` uses `CHECK_ERROR` to validate mandatory fields. The `CHECK_ERROR` macro will throw/assert, so `TEST_ERROR()` is the appropriate way to test this.

#### Group 2: Default Value Application (3-4 test cases)

These tests verify that null optional properties receive correct default values:

1. **Test: Null textColor gets Black (0,0,0)**
   - Text run: `fontFamily="Arial", size=12, textColor=null`
   - Expected result: textColor should be `Color(0, 0, 0)`

2. **Test: Null backgroundColor gets Black (0,0,0)**
   - Text run: `fontFamily="Arial", size=12, backgroundColor=null`
   - Expected result: backgroundColor should be `Color(0, 0, 0)`

3. **Test: Null textStyle gets (TextStyle)0**
   - Text run: `fontFamily="Arial", size=12, textStyle=null`
   - Expected result: textStyle should be `(TextStyle)0` (no flags set)

4. **Test: All optional properties null get defaults**
   - Text run: only `fontFamily` and `size` defined, all others null
   - Expected result: textColor=Black, backgroundColor=Black, textStyle=0

**Implementation Note**: Need to create a helper function to verify the full `DocumentTextRunProperty` in the result, since `AssertMap` already exists but we need to check the actual default values were applied.

#### Group 3: Mixing Defined and Null Properties (3-5 test cases)

These tests verify that defined properties are preserved while null properties get defaults:

1. **Test: Some defined, some null properties**
   - Text run: `fontFamily="Arial", size=12, textColor=Red, backgroundColor=null, textStyle=null`
   - Expected: textColor=Red (preserved), backgroundColor=Black (default), textStyle=0 (default)

2. **Test: Inline object + text with nulls**
   - Inline object run at [10, 20]
   - Text run at [0, 30] with nullable properties (will be split around inline object)
   - Verify: inline object has priority, text runs get correct defaults

3. **Test: Multiple text runs with different null patterns**
   - Run1: `fontFamily="Arial", size=12, textColor=Red, backgroundColor=null`
   - Run2: `fontFamily="Times", size=14, textColor=null, backgroundColor=Blue`
   - Expected: Each gets appropriate defaults for null fields

4. **Test: Adjacent text runs with partial nulls should NOT merge**
   - Run1: `fontFamily="Arial", size=12, textColor=null`
   - Run2: `fontFamily="Arial", size=12, textColor=Red`
   - Note: MergeRuns doesn't do merging, but we verify both remain separate in output

5. **Test: Text run with all properties defined (no nulls)**
   - Verify: All properties preserved as-is in output
   - This is a sanity check that defined properties aren't affected

#### Group 4: Complex Nullable Scenarios (2-3 test cases)

These tests verify nullable properties work correctly with the inline object priority system:

1. **Test: Text with nulls overlaps inline object**
   - Text run [0, 30] with nullable properties
   - Inline object [10, 20]
   - Expected: Text split into [0,10] and [20,30], both get correct defaults

2. **Test: Multiple inline objects with text runs having different null patterns**
   - Text run [0, 50] with some nulls
   - Inline objects at [10, 15], [30, 35]
   - Expected: Text fragmented correctly, all fragments get correct defaults

3. **Test: Interleaved runs with gaps and nulls**
   - Mix of text runs (some with nulls) and inline objects with gaps between them
   - Verify: All runs preserve their relative positions, text runs get defaults

### Helper Function Additions (If Needed)

Based on analysis, the existing helper functions should be sufficient:

- `CreateTextPropPartial()` already exists for creating nullable overrides
- `CreateFullTextProp()` already exists for creating expected full properties
- `AssertMap()` already exists for comparing maps
- `CompareRunProperty()` already exists with overload for `DocumentTextRunProperty`

However, I may need to add a convenience function to create `DocumentTextRunPropertyOverrides` with only mandatory fields:

```cpp
DocumentTextRunPropertyOverrides CreateTextPropMandatoryOnly(
    const WString& fontFamily,
    vint size)
{
    DocumentTextRunPropertyOverrides prop;
    prop.fontFamily = fontFamily;
    prop.size = size;
    // Leave all others null (default initialized as Nullable<T>)
    return prop;
}
```

This makes test code more readable when we want to test default application.

### Implementation Strategy

1. **Add the new test category** after existing `TEST_CATEGORY(L"MergeRuns")`

2. **Implement Group 1 tests first** (mandatory validation)
   - Use `TEST_ERROR()` macro for null validation tests
   - Verify that providing both mandatory fields passes

3. **Implement Group 2 tests** (default value application)
   - Create text runs with various null optional properties
   - Call MergeRuns and verify result has correct defaults
   - Use `AssertMap()` to compare actual vs expected results

4. **Implement Group 3 tests** (mixing defined and null)
   - Create more complex scenarios with partial definitions
   - Verify defined properties preserved, null properties get defaults
   - Test interaction with inline objects

5. **Implement Group 4 tests** (complex scenarios)
   - Combine nullable properties with inline object priority system
   - Verify the complete merge logic works correctly

### Test Execution Flow

Each test case follows this pattern:

```cpp
TEST_CASE(L"Description of what we're testing")
{
    // Setup: Create text runs with nullable properties
    DocumentTextRunPropertyMap textMap;
    auto prop = CreateTextPropPartial(/* specific null pattern */);
    AddTextRun(textMap, {.caretBegin = ..., .caretEnd = ...}, prop);
    
    // Setup: Create inline objects if needed
    DocumentInlineObjectRunPropertyMap inlineMap;
    // ... add inline objects if test requires them
    
    // Execute: Call MergeRuns
    DocumentRunPropertyMap result;
    MergeRuns(textMap, inlineMap, result);
    
    // Verify: Check result has expected values
    DocumentRunPropertyMap expectedResult;
    expectedResult.Add({...}, DocumentRunProperty(CreateFullTextProp(...)));
    AssertMap(result, expectedResult);
});
```

For error validation tests:

```cpp
TEST_CASE(L"Null fontFamily should trigger error")
{
    DocumentTextRunPropertyMap textMap;
    auto prop = CreateTextPropPartial(
        Nullable<WString>(),  // null fontFamily
        12,                    // size defined
        /* ... other fields ... */);
    AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop);
    
    DocumentInlineObjectRunPropertyMap inlineMap;
    DocumentRunPropertyMap result;
    
    // Expect error when calling MergeRuns
    TEST_ERROR(MergeRuns(textMap, inlineMap, result));
});
```

## Key Design Decisions

### 1. Test Organization

Following the established pattern from Task No.11, I'm creating a separate test category `"MergeRuns (partial application)"` rather than adding to the existing `"MergeRuns"` category. This makes it clear that these tests are specifically for nullable property behavior.

**Rationale**: 
- Clear separation of concerns (basic merge logic vs nullable semantics)
- Mirrors the structure used for AddTextRun testing
- Makes it easy to find tests for specific functionality

### 2. Error Testing Approach

Using `TEST_ERROR()` macro to verify that `CHECK_ERROR` is raised for invalid input (null mandatory fields).

**Rationale**:
- This is the pattern used throughout the test file for error conditions
- `CHECK_ERROR` throws/asserts, so `TEST_ERROR` is the correct way to test it
- Provides clear failure messages when validation isn't working

### 3. Test Coverage Scope

Focusing on 10-15 test cases covering the most critical scenarios rather than exhaustive edge cases.

**Rationale**:
- Task No.6 learning: Start with focused tests covering critical scenarios
- MergeRuns is simpler than DiffRuns (no complex range transformations)
- Core functionality: mandatory validation + default application + inline priority
- Can expand coverage later if issues are found

### 4. No New Helper Functions

Reusing existing helper functions rather than creating new ones unless absolutely necessary.

**Rationale**:
- Existing helpers (`CreateTextPropPartial`, `AssertMap`, `CreateFullTextProp`) cover our needs
- Adding more helpers increases maintenance burden
- The one potential addition (`CreateTextPropMandatoryOnly`) is optional - can use `CreateTextPropPartial` directly

### 5. Default Values

Using the exact defaults specified in Task No.9:
- `textColor`: `Color(0, 0, 0)` (Black)
- `backgroundColor`: `Color(0, 0, 0)` (Black)
- `textStyle`: `(TextStyle)0` (no flags)

**Rationale**:
- These are the values coded in the `ConvertToFullProperty` function
- Tests must match implementation
- Provides clear expected values for test assertions

## Potential Issues and Mitigations

### Issue 1: `CHECK_ERROR` May Not Throw in Test Environment

**Concern**: If `CHECK_ERROR` is configured differently in test builds, `TEST_ERROR()` might not work as expected.

**Mitigation**: 
- If tests fail, check how `CHECK_ERROR` is configured
- May need to adjust test approach to check for specific error conditions
- Could add a test case that verifies expected behavior with mandatory fields defined (positive test)

### Issue 2: `CreateFullTextProp()` Helper May Not Support TextStyle

**Concern**: The existing `CreateFullTextProp(colorValue)` function takes a single color value and may not have a variant that accepts TextStyle.

**Mitigation**:
- Extend `CreateFullTextProp()` to accept additional parameters if needed
- Or create expected properties manually in test cases
- The existing function already exists and is used in DiffRuns tests, so it should be adaptable

### Issue 3: Complexity of Testing Inline Object + Nullable Properties

**Concern**: Testing the interaction between inline object priority and nullable properties might be complex.

**Mitigation**:
- Start with simple cases (single inline, single text run)
- Build up to complex cases incrementally
- Can reduce scope if complexity is too high (focus on core functionality first)

## Expected Test Count

Based on the design:
- Group 1 (Mandatory validation): 3 test cases
- Group 2 (Default application): 4 test cases
- Group 3 (Mixing defined and null): 5 test cases  
- Group 4 (Complex scenarios): 3 test cases

**Total: ~15 test cases**

This provides comprehensive coverage of the nullable property semantics in MergeRuns while staying within a manageable scope.

## Summary of Design

The design adds a new `TEST_CATEGORY(L"MergeRuns (partial application)")` with approximately 15 test cases organized into 4 groups:

1. **Mandatory Property Validation**: Verify `fontFamily` and `size` must be defined
2. **Default Value Application**: Verify null optional properties get correct defaults (Black, Black, TextStyle 0)
3. **Mixing Defined and Null**: Verify defined properties preserved, null get defaults
4. **Complex Scenarios**: Verify nullable properties work with inline object priority

The tests will:
- Reuse existing helper functions (`CreateTextPropPartial`, `AssertMap`, `CreateFullTextProp`)
- Follow established patterns from `AddTextRun (partial application)` tests
- Use `TEST_ERROR()` for validation failure tests
- Focus on critical scenarios rather than exhaustive edge cases
- Verify both success cases and error cases

This approach ensures thorough testing of MergeRuns' nullable property handling while maintaining consistency with the existing test structure.

# !!!FINISHED!!!
