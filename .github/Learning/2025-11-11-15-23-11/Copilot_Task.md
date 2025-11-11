# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.11: Add comprehensive test cases for nullable property scenarios in AddTextRun

### description

Add new test cases to `TestRemote_DocumentRunManagement.cpp` within the existing `TEST_CATEGORY("AddTextRun")` to thoroughly test the nullable property functionality added in Task No.8, No.9, and No.10.

**Test approach**:
- Add new TEST_CASE blocks within existing TEST_CATEGORY("AddTextRun")
- Use helper function variants that create overrides with some properties null
- Verify nullable property semantics: null = keep existing value in AddTextRun

**New test cases for AddTextRun**:

1. **Partial property updates**:
   - Existing run with all properties defined, add overlapping run with only fontFamily defined → verify only fontFamily changes
   - Existing run, add run with only textColor defined → verify only color changes
   - Existing run, add run with only textStyle defined → verify only style changes
   - Test all property combinations (fontFamily, size, textColor, backgroundColor, textStyle)

2. **Null property merging**:
   - Adjacent runs with identical properties including identical nullability → should merge
   - Example: Run1 [(0,5) fontFamily="Arial", color=null] + Run2 [(5,10) fontFamily="Arial", color=null] → should merge to [(0,10) fontFamily="Arial", color=null]
   - Adjacent runs where a property is null in one but defined in the other → should NOT merge
   - Example: Run1 [(0,5) fontFamily="Arial", color=null] + Run2 [(5,10) fontFamily="Arial", color=Red] → remain separate (null vs Red)
   - Example: Run1 [(0,5) fontFamily="Arial", color=Red] + Run2 [(5,10) fontFamily="Arial", color=null] → remain separate (Red vs null)
   - Adjacent runs with different defined properties → should NOT merge
   - Example: Run1 [(0,5) fontFamily="Arial"] + Run2 [(5,10) fontFamily="Times"] → remain separate

3. **Layered property application**:
   - Start with empty map
   - Add run [(0,10) fontFamily="Arial"]
   - Add run [(0,10) size=12]  → should merge to single run with both properties
   - Add run [(5,10) textColor=Red] → should split: [(0,5) Arial+12], [(5,10) Arial+12+Red]
   - Verify final state has correct property combinations

4. **Split with partial updates**:
   - Existing run [(0,20) with all properties defined]
   - Add run [(5,15) with only fontFamily defined]
   - Verify three fragments: [(0,5) original], [(5,15) updated fontFamily only], [(15,20) original]

5. **Edge cases**:
   - Add run with all properties null → effectively a no-op or only affects range structure
   - Consecutive additions with different nullable combinations
   - Null properties in complex merge scenarios (3+ consecutive runs merging)

**Helper function additions**:

Create new helper functions:
```cpp
// Create overrides with only specified properties defined
DocumentTextRunPropertyOverrides CreateTextPropPartial(
	Nullable<WString> fontFamily,
	Nullable<vint> size,
	Nullable<Color> textColor,
	Nullable<Color> backgroundColor,
	Nullable<IGuiGraphicsParagraph::TextStyle> textStyle);

// Create overrides with single property defined
DocumentTextRunPropertyOverrides CreateTextPropWithFont(const WString& fontFamily);
DocumentTextRunPropertyOverrides CreateTextPropWithSize(vint size);
DocumentTextRunPropertyOverrides CreateTextPropWithColor(Color textColor);
DocumentTextRunPropertyOverrides CreateTextPropWithStyle(IGuiGraphicsParagraph::TextStyle style);
```

### what to be done

1. Create helper functions for partial property override creation
2. Add a new TEST_CATEGORY("AddTextRun (partial application)") after the existing TEST_CATEGORY("AddTextRun")
3. Add new test cases to the new category:
   - Test partial property updates (5-10 test cases covering different property combinations)
   - Test null property merging rules (3-5 test cases)
   - Test layered property application (2-3 test cases building up complex states)
   - Test split with partial updates (2-3 test cases)
   - Test edge cases (2-3 test cases)
4. Each test case should:
   - Use the new partial helper functions
   - Set up initial state with specific null patterns
   - Perform AddTextRun operation
   - Use AssertMap to verify result matches expected state including nullability
5. Ensure test output clearly shows which properties are null vs defined

### how to test it

Run the compiled unit test executable. The new test cases validate:
1. AddTextRun correctly applies only non-null properties, preserving existing values for null properties
2. AddTextRun merging rules work correctly with nullable properties (merge when all properties including nullability match)
3. Complex combinations of nullable properties work correctly in realistic scenarios
4. The nullable property system enables the intended use case: partial/layered formatting operations

Success criteria:
- All new test cases pass
- Test coverage includes all property combinations
- Edge cases with multiple null properties are handled correctly

### file locations

Modified file: `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`

### rationale

Task No.8, No.9, and No.10 implement the nullable property mechanism for AddTextRun but maintain backward compatibility by only testing with fully-defined properties. This task validates that the nullable property system actually works as intended for AddTextRun's primary use cases:

1. **Partial formatting operations**: Real text editors need to change one property at a time (make bold, change color, etc.) without knowing or specifying all other properties. The nullable system enables this.

2. **Layered formatting**: Multiple formatting operations can compose naturally. First set font, then set color, then set bold - each operation only touches its property dimension.

3. **Merge semantics**: The merging rules need to work correctly with nullable properties. Two runs should merge if their properties match completely, including nullability. Null is treated as a valid value for comparison purposes.

Without comprehensive testing of nullable scenarios for AddTextRun, we risk:
- Subtle bugs in property application (null treated as value instead of "no change")
- Incorrect merging (runs that should merge staying separate due to nullability differences, or vice versa)
- Protocol corruption when layering multiple partial updates

This task focuses specifically on AddTextRun behavior, allowing for thorough testing of this critical function's nullable property handling.

# UPDATES

## UPDATE

I would like new test cases be in TEST_CATEGORY("AddTextRun (partial application)")

# INSIGHTS AND REASONING

## Understanding the Context

I've identified that this is Task No.11 from the scrum board, which focuses on adding comprehensive test cases for nullable property scenarios in the AddTextRun function. This task comes after:

- **Task No.8**: Defined `DocumentTextRunPropertyOverrides` type and updated function signatures (completed)
- **Task No.9**: Implemented nullable property storage and comparison (completed)
- **Task No.10**: Implemented nullable property overriding logic in AddTextRun (completed)

The foundation for nullable properties is complete, but there's no test coverage for the new functionality. All existing tests use fully-defined properties, so they don't exercise the nullable behavior at all.

## Key Design Considerations

### 1. Nullable Property Semantics in AddTextRun

From the previous tasks, I understand that in AddTextRun:
- **Null means "keep existing value"**: When adding a run with a null property, that property's value from the overlapping existing run should be preserved
- **Null is a value for comparison**: Two runs with the same null pattern can merge (e.g., both have `color=null`)
- **Different null patterns prevent merging**: A run with `color=null` cannot merge with a run with `color=Red`

This differs from MergeRuns where null means "use default value".

### 2. Test Coverage Strategy

Based on the task description and learnings from previous tasks (especially Task 6's lesson about starting simple), I should:

1. **Start with basic scenarios** before attempting comprehensive coverage
2. **Organize tests by functional category**: partial updates, merging, layering, splitting
3. **Use clear, descriptive test names** that explain what's being validated
4. **Focus on the critical contracts**: null preservation, merge semantics, layering behavior

### 3. Helper Function Design

The task specifies creating helper functions for partial property creation:

```cpp
CreateTextPropPartial(...)  // Full control over which properties are null
CreateTextPropWithFont(...)  // Only font defined, rest null
CreateTextPropWithSize(...)  // Only size defined, rest null
CreateTextPropWithColor(...) // Only color defined, rest null
CreateTextPropWithStyle(...) // Only style defined, rest null
```

These helpers enable readable test code: `CreateTextPropWithFont(L"Arial")` clearly shows intent.

### 4. Critical Test Scenarios

The most important scenarios to test:

**Scenario A: Partial property updates** (null = keep existing)
- Existing run: font="Arial", size=12, color=Black
- Add run with font="Times", size=null, color=null
- Expected: font changes to "Times", size stays 12, color stays Black

**Scenario B: Null property merging** (null is a value)
- Run1: font="Arial", color=null
- Run2 (adjacent): font="Arial", color=null
- Expected: Merge to single run (both have identical properties including nullability)

**Scenario C: Different nullability prevents merging**
- Run1: font="Arial", color=null
- Run2 (adjacent): font="Arial", color=Red
- Expected: Stay separate (null ≠ Red)

**Scenario D: Layered property application**
- Step 1: Add run with only font="Arial"
- Step 2: Add overlapping run with only size=12
- Expected: Result has both font and size defined

**Scenario E: Splitting with partial updates**
- Existing run [0,20]: font="Arial", size=12, color=Black
- Add run [5,15]: font="Times", size=null, color=null
- Expected three fragments:
  - [0,5]: font="Arial", size=12, color=Black (unchanged)
  - [5,15]: font="Times", size=12, color=Black (font updated, others preserved)
  - [15,20]: font="Arial", size=12, color=Black (unchanged)

### 5. Scope Adjustment

Given the complexity and following Task 6's lesson, I should plan for **10-15 focused test cases** initially:

- 3-4 tests for partial property updates (most common scenario)
- 3-4 tests for null property merging rules
- 2-3 tests for layered property application
- 2-3 tests for splitting with partial updates
- 1-2 edge case tests

This is more realistic than the "5-10 per category" mentioned in the original task description.

## Implementation Plan

### Phase 1: Helper Functions

Create the helper functions at the top of the test file:

1. `CreateTextPropPartial` - full control
2. `CreateTextPropWithFont` - only font defined
3. `CreateTextPropWithSize` - only size defined
4. `CreateTextPropWithColor` - only textColor defined
5. `CreateTextPropWithBgColor` - only backgroundColor defined
6. `CreateTextPropWithStyle` - only textStyle defined

### Phase 2: Test Cases Organization

Add a new TEST_CATEGORY after the existing TEST_CATEGORY("AddTextRun"):

```cpp
TEST_CATEGORY(L"AddTextRun")
{
    // ... existing test cases remain unchanged ...
});

TEST_CATEGORY(L"AddTextRun (partial application)")
{
    TEST_CASE(L"Partial property update - font only")
    {
        // Test changing only font, preserving other properties
    });
    
    TEST_CASE(L"Partial property update - color only")
    {
        // Test changing only color
    });
    
    TEST_CASE(L"Null property merging - identical nullability")
    {
        // Test that runs with same null pattern merge
    });
    
    TEST_CASE(L"Null property merging - different nullability")
    {
        // Test that runs with different null patterns don't merge
    });
    
    // ... more test cases ...
});
```

### Phase 3: Validation Strategy

Each test should:
1. Set up initial state using existing helpers (`CreateTextProp`) or new partial helpers
2. Perform AddTextRun operation
3. Use `AssertMap` to verify the result, including checking for correct nullability
4. Use descriptive assertions that explain what's being validated

## Potential Challenges

1. **Formatting null values**: Need to ensure `FormatRunProperty` clearly distinguishes null vs defined properties (already handled in Task 9)

2. **Complex merge scenarios**: When layering multiple partial updates, the merge logic can become tricky. Need to carefully verify that consecutive runs with identical properties (including nullability) merge correctly.

3. **Test readability**: With nullable properties, test setup can become verbose. The helper functions are critical for maintaining readability.

4. **Coverage vs complexity**: Need to balance comprehensive coverage with keeping individual test cases focused and understandable.

## Expected Outcomes

After completing this task:

1. **Confidence in nullable property implementation**: Comprehensive tests validate that the nullable system works as designed
2. **Documentation through tests**: Test cases serve as documentation of how nullable properties should behave
3. **Regression prevention**: Future changes to AddTextRun will be validated against these test cases
4. **Foundation for Task 12**: Similar testing approach can be used for MergeRuns nullable scenarios

## Design Decision: Test Case Priority

Based on realistic usage patterns, I'll prioritize:

1. **HIGH PRIORITY**: Partial property updates (most common - users change one property at a time)
2. **HIGH PRIORITY**: Null property merging (affects performance and result correctness)
3. **MEDIUM PRIORITY**: Layered property application (realistic multi-step formatting)
4. **MEDIUM PRIORITY**: Splitting with partial updates (validates preserve-existing-value semantics)
5. **LOW PRIORITY**: Edge cases with all-null runs (rare in practice)

This prioritization ensures the most important scenarios are tested first, following the incremental approach validated in previous tasks.

## Organizational Decision: Separate Test Category

Based on the update request, all new nullable property test cases should be organized into a separate `TEST_CATEGORY("AddTextRun (partial application)")` rather than adding them to the existing `TEST_CATEGORY("AddTextRun")`.

**Rationale for separate category**:

1. **Clear separation of concerns**: The existing AddTextRun tests validate the core functionality with fully-defined properties. The new category specifically tests nullable property scenarios, making it clear what each category covers.

2. **Better test output organization**: When tests run, the separate category makes it immediately obvious which nullable property scenarios pass or fail, without mixing them with the basic functionality tests.

3. **Easier maintenance**: Future developers can quickly locate nullable property tests without searching through the basic tests. This follows the pattern established in Task 7 where complex scenarios got their own category (`DiffRuns (Complex)`).

4. **Test independence**: Keeping them separate ensures existing tests remain untouched and validates that nullable property support is truly backward compatible (existing tests continue to pass without modification).

5. **Matches the incremental development pattern**: Just as Task 8-10 incrementally added nullable property support without breaking existing functionality, having a separate test category emphasizes that this is an additive feature, not a modification of existing behavior.

The new category name "AddTextRun (partial application)" clearly communicates that it tests partial property application scenarios, which is the primary use case for nullable properties.

# !!!FINISHED!!!
