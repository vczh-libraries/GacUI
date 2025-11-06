# !!!PLANNING!!!

# UPDATES

## UPDATE

I found that I didn't clarify enough feature definition.

`DiffRuns` actually need to report error, when a run belong to a caret range is updated to nothing. For example `Text→Inline: Different range, shifted right`, (10-15)text is updated to (15-25)inline, (10-15)is updated to nothing, it needs to CHECK_FAIL (which ends up with TEST_ERROR in test case). The function may not implemented the check, you may need to also update it.

Even when (10-15)text is updated to (11-15)anything, (10-11)is updated to nothing which is not correct.

Fix existing planned test cases to use correct caret range, and add some cases to cover error detection

## UPDATE

When doing assertion, you need to use CHECK_ERROR instead of CHECK_FAIL.

And please search for how CHECK_ERROR in other place of the project, follow the `ERROR_MESSAGE_PREFIX` pattern and remember to put full namespace in it

## UPDATE

Not a correct usage of `TEST_ERROR`. Please search for examples. It accepts an expression instead of a code block.

## UPDATE

Some cases are redundant. You can actually update a few of them to cover cases that, old cases and new cases cut in different way. For example:

0-10, 10-20, 20-30 -> 0-15, 15-30
and cover items increased/decreased/same amount, all with different cuts

# IMPROVEMENT PLAN

## STEP 0: Add Error Checking to DiffRuns Function

Update the `DiffRuns` function in `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp` to add validation that old run ranges are fully covered by new run ranges.

**Code Change:**

Add range coverage validation before the callback ID tracking. Insert after line 424 (after the main while loop, before callback ID processing):

```cpp
		}

#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::DiffRuns(const DocumentRunPropertyMap&, const DocumentRunPropertyMap&, remoteprotocol::ElementDesc_DocumentParagraph&)#"
		// Validate that all old run ranges are fully covered by new run ranges
		for (vint oldI = 0; oldI < oldKeys.Count(); oldI++)
		{
			auto&& oldKey = oldKeys[oldI];
			vint coveredEnd = oldKey.caretBegin;
			
			// Find all new ranges that cover this old range
			for (vint newI = 0; newI < newKeys.Count(); newI++)
			{
				auto&& newKey = newKeys[newI];
				
				// Check if this new range overlaps with uncovered part of old range
				if (newKey.caretBegin <= coveredEnd && newKey.caretEnd > coveredEnd)
				{
					coveredEnd = newKey.caretEnd > oldKey.caretEnd ? oldKey.caretEnd : newKey.caretEnd;
				}
			}
			
			CHECK_ERROR(coveredEnd >= oldKey.caretEnd,
				ERROR_MESSAGE_PREFIX L"Old run range not fully covered by new runs");
		}
#undef ERROR_MESSAGE_PREFIX

		for (vint i = 0; i < newInlineCallbackIds.Count(); i++)
```

**Rationale:**

This validation ensures that every old run's range is fully covered by new runs. The algorithm:
1. For each old run range
2. Start with `coveredEnd` at the beginning of the old range
3. Find new ranges that overlap with the uncovered portion and extend `coveredEnd`
4. After checking all new ranges, verify that `coveredEnd` reached the end of the old range
5. If not fully covered, `CHECK_ERROR` triggers (showing as `TEST_ERROR` in test cases)

This prevents invalid transformations like (10-20)text → (15-25)inline where (10-15) is left uncovered.

The `ERROR_MESSAGE_PREFIX` macro follows the project convention: it contains the full function signature with complete namespace `vl::presentation::elements::DiffRuns(...)` followed by `#`. This is used with `CHECK_ERROR` to provide clear error messages showing which function detected the error.

## STEP 1: Add TEST_CATEGORY(L"DiffRuns (Complex)") Block

Add a new test category immediately after the existing `TEST_CATEGORY(L"DiffRuns")` block in `Test\GacUISrc\UnitTest\TestRemote_DocumentRunManagement.cpp`.

**Code Change:**

After the closing `});` of `TEST_CATEGORY(L"DiffRuns")` (around line 1446), add:

```cpp
	TEST_CATEGORY(L"DiffRuns (Complex)")
	{
		// Helper functions reused from DiffRuns category
		auto MakeExpectedList = []() {
			return Ptr(new List<remoteprotocol::DocumentRun>());
		};
		
		auto AddExpectedRun = [](Ptr<List<remoteprotocol::DocumentRun>> list, vint begin, vint end, const DocumentRunProperty& prop) {
			remoteprotocol::DocumentRun run;
			run.caretBegin = begin;
			run.caretEnd = end;
			run.props = prop;
			list->Add(run);
		};
		
		auto MakeExpectedIds = [](std::initializer_list<vint> ids) {
			auto list = Ptr(new List<vint>());
			for (auto id : ids)
			{
				list->Add(id);
			}
			return list;
		};

		// Test cases will be added here
	});
```

**Rationale:**

The test category needs its own scope with helper functions. These helpers are identical to those in the basic DiffRuns category, but must be redefined because they are local lambdas. This follows the established pattern from Task 6 and ensures the tests are self-contained.

## STEP 2: Implement Valid Text→Inline Object Transformation Tests

Add test cases covering valid scenarios where text runs are replaced by inline object runs. All ranges must satisfy the coverage requirement (old ranges fully covered by new ranges). Uses different cut points to cover increase/decrease/same count scenarios.

**Code Change:**

Inside the `TEST_CATEGORY(L"DiffRuns (Complex)")` block, add:

```cpp
		// Category 1: Valid Text → Inline Object Transformations
		
		TEST_CASE(L"Text→Inline: Same range, type changed")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: text run at [10, 20]
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: inline object at same range (fully covers old - VALID)
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(1, 100)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Verify diff contains the new inline object
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateInlineProp(1, 100)));
			AssertDiffList(desc.runsDiff, expected, L"Text→Inline at same range");
			
			// Verify callback tracking
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({1}), L"Inline object created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Text→Inline: Different cuts - 3 old runs → 2 new inlines (decreased count)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: three text runs at [0, 10], [10, 20], [20, 30] (cut at 10, 20)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x60)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// New: two inline objects at [0, 15], [15, 30] (cut at 15) - different cuts, fewer runs
			newRuns.Add({.caretBegin = 0, .caretEnd = 15}, DocumentRunProperty(CreateInlineProp(3, 150)));
			newRuns.Add({.caretBegin = 15, .caretEnd = 30}, DocumentRunProperty(CreateInlineProp(4, 150)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Both new inline objects in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 15, DocumentRunProperty(CreateInlineProp(3, 150)));
			AddExpectedRun(expected, 15, 30, DocumentRunProperty(CreateInlineProp(4, 150)));
			AssertDiffList(desc.runsDiff, expected, L"3 text runs → 2 inlines with different cuts");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({3, 4}), L"Two inlines created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Text→Inline: Different cuts - 2 old runs → 3 new inlines (increased count)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: two text runs at [0, 15], [15, 30] (cut at 15)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 15}, DocumentRunProperty(CreateTextProp(0x80)));
			oldRuns.Add({.caretBegin = 15, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// New: three inline objects at [0, 10], [10, 20], [20, 30] (cut at 10, 20) - different cuts, more runs
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateInlineProp(5, 50)));
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(6, 60)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateInlineProp(7, 70)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// All three inline objects in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateInlineProp(5, 50)));
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateInlineProp(6, 60)));
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateInlineProp(7, 70)));
			AssertDiffList(desc.runsDiff, expected, L"2 text runs → 3 inlines with different cuts");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({5, 6, 7}), L"Three inlines created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
```

**Rationale:**

Consolidated from 3 test cases to 3 more focused test cases using different cut points strategy:
1. Same range transformation (baseline)
2. 3→2 runs with different cuts (0-10-20-30 → 0-15-30): decreased count, different boundaries
3. 2→3 runs with different cuts (0-15-30 → 0-10-20-30): increased count, different boundaries

This eliminates redundancy while still covering:
- Type transformation detection
- Multiple scenarios with different run counts (increase/decrease)
- Different boundary alignments between old and new runs
- Callback ID tracking for created inline objects

## STEP 3: Implement Valid Inline Object→Text Transformation Tests

Add test cases for valid reverse transformations: inline objects replaced by text runs with full coverage. Uses different cut points to cover various count scenarios.

**Code Change:**

Continue inside the test category:

```cpp
		// Category 2: Valid Inline Object → Text Transformations
		
		TEST_CASE(L"Inline→Text: Same range, type changed")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: inline object at [10, 20]
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(10, 100)));
			
			// New: text run at same range (fully covers old - VALID)
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Verify diff contains the new text run
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateTextProp(0xA0)));
			AssertDiffList(desc.runsDiff, expected, L"Inline→Text at same range");
			
			// Verify callback tracking: old inline removed
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({10}), L"Inline object removed");
		});
		
		TEST_CASE(L"Inline→Text: Different cuts - 3 old inlines → 2 new text runs (decreased count)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: three inline objects at [0, 8], [8, 16], [16, 24] (cut at 8, 16)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 8}, DocumentRunProperty(CreateInlineProp(12, 20)));
			oldRuns.Add({.caretBegin = 8, .caretEnd = 16}, DocumentRunProperty(CreateInlineProp(13, 30)));
			oldRuns.Add({.caretBegin = 16, .caretEnd = 24}, DocumentRunProperty(CreateInlineProp(14, 40)));
			
			// New: two text runs at [0, 12], [12, 24] (cut at 12) - different cuts, fewer runs
			newRuns.Add({.caretBegin = 0, .caretEnd = 12}, DocumentRunProperty(CreateTextProp(0xFF)));
			newRuns.Add({.caretBegin = 12, .caretEnd = 24}, DocumentRunProperty(CreateTextProp(0xCC)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Both new text runs in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 12, DocumentRunProperty(CreateTextProp(0xFF)));
			AddExpectedRun(expected, 12, 24, DocumentRunProperty(CreateTextProp(0xCC)));
			AssertDiffList(desc.runsDiff, expected, L"3 inlines → 2 text runs with different cuts");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({12, 13, 14}), L"Three inlines removed");
		});
		
		TEST_CASE(L"Inline→Text: Different cuts - 2 old inlines → 3 new text runs (increased count)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: two inline objects at [10, 25], [25, 40] (cut at 25)
			oldRuns.Add({.caretBegin = 10, .caretEnd = 25}, DocumentRunProperty(CreateInlineProp(15, 150)));
			oldRuns.Add({.caretBegin = 25, .caretEnd = 40}, DocumentRunProperty(CreateInlineProp(16, 150)));
			
			// New: three text runs at [10, 20], [20, 30], [30, 40] (cut at 20, 30) - different cuts, more runs
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x40)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x80)));
			newRuns.Add({.caretBegin = 30, .caretEnd = 40}, DocumentRunProperty(CreateTextProp(0xC0)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// All three text runs in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateTextProp(0x40)));
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateTextProp(0x80)));
			AddExpectedRun(expected, 30, 40, DocumentRunProperty(CreateTextProp(0xC0)));
			AssertDiffList(desc.runsDiff, expected, L"2 inlines → 3 text runs with different cuts");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({15, 16}), L"Two inlines removed");
		});
```

**Rationale:**

Consolidated to 3 focused test cases using different cut points strategy:
1. Same range transformation (baseline)
2. 3→2 runs with different cuts (0-8-16-24 → 0-12-24): decreased count, different boundaries
3. 2→3 runs with different cuts (10-25-40 → 10-20-30-40): increased count, different boundaries

This eliminates redundancy while covering:
- Type transformation detection (inline→text)
- Various run count changes (increase/decrease)
- Different boundary alignments
- Callback ID tracking for removed inline objects

## STEP 4: Implement Text→Text Property Change Tests

Add test cases for text runs with property or range changes, all maintaining full coverage. Uses different cut points to cover various scenarios efficiently.

**Code Change:**

Continue inside the test category:

```cpp
		// Category 3: Text → Text Property Changes
		
		TEST_CASE(L"Text→Text: Same range, property changed")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: text with color 0x80
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: same range but different color
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0xFF)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Changed property should appear in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateTextProp(0xFF)));
			AssertDiffList(desc.runsDiff, expected, L"Text property changed");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Text→Text: CRITICAL - Multiple old runs merge to single new run")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			auto prop1 = CreateTextProp(0x40);
			auto prop2 = CreateTextProp(0x80);
			
			// Old: three runs with alternating properties
			oldRuns.Add({.caretBegin = 1, .caretEnd = 2}, DocumentRunProperty(prop1));
			oldRuns.Add({.caretBegin = 2, .caretEnd = 5}, DocumentRunProperty(prop2));
			oldRuns.Add({.caretBegin = 5, .caretEnd = 10}, DocumentRunProperty(prop1));
			
			// New: single merged run covering entire range (fully covers old - VALID)
			newRuns.Add({.caretBegin = 1, .caretEnd = 10}, DocumentRunProperty(prop1));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// CRITICAL: Must report complete new range (1,10), NOT partial like (2,5)
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 1, 10, DocumentRunProperty(prop1));
			AssertDiffList(desc.runsDiff, expected, L"CRITICAL: Merged range uses new key (1,10)");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Text→Text: Different cuts - 2 old runs → 3 new runs (increased, properties changed)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: two runs at [0, 15], [15, 30] (cut at 15)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 15}, DocumentRunProperty(CreateTextProp(0x80)));
			oldRuns.Add({.caretBegin = 15, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: three runs at [0, 10], [10, 20], [20, 30] (cut at 10, 20) with different properties
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x20)));
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// All three new runs appear in diff (different cuts + properties changed)
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateTextProp(0x20)));
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateTextProp(0x60)));
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateTextProp(0xA0)));
			AssertDiffList(desc.runsDiff, expected, L"2→3 runs, different cuts + property changes");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Text→Text: Different cuts - 4 old runs → 2 new runs (decreased, merged with new property)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			auto propFinal = CreateTextProp(0xFF);
			
			// Old: four runs at [0, 5], [5, 10], [10, 18], [18, 24] (cut at 5, 10, 18)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 5}, DocumentRunProperty(CreateTextProp(0x20)));
			oldRuns.Add({.caretBegin = 5, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 18}, DocumentRunProperty(CreateTextProp(0x60)));
			oldRuns.Add({.caretBegin = 18, .caretEnd = 24}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: two runs at [0, 12], [12, 24] (cut at 12) with uniform property
			newRuns.Add({.caretBegin = 0, .caretEnd = 12}, DocumentRunProperty(propFinal));
			newRuns.Add({.caretBegin = 12, .caretEnd = 24}, DocumentRunProperty(propFinal));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Both new runs in diff (different cuts + property changed)
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 12, DocumentRunProperty(propFinal));
			AddExpectedRun(expected, 12, 24, DocumentRunProperty(propFinal));
			AssertDiffList(desc.runsDiff, expected, L"4→2 runs, different cuts + property changes");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
```

**Rationale:**

Consolidated to 4 focused test cases using different cut points strategy:
1. Same range, property changed (baseline)
2. **CRITICAL:** 3→1 run merge validation (1-2-5-10 → 1-10), ensures new key used
3. 2→3 runs with different cuts (0-15-30 → 0-10-20-30): increased count + property changes
4. 4→2 runs with different cuts (0-5-10-18-24 → 0-12-24): decreased count + property changes

This eliminates redundancy from previous 4 test cases while comprehensively covering:
- Property changes at same range
- **CRITICAL requirement:** merged runs use complete new ranges
- Different cut points with increased run count
- Different cut points with decreased run count
- All with property modifications

## STEP 5: Implement Inline Object→Inline Object Change Tests

Add test cases for inline object modifications with full coverage. Uses different cut points to efficiently cover various scenarios.

**Code Change:**

Continue inside the test category:

```cpp
		// Category 4: Inline Object → Inline Object Changes
		
		TEST_CASE(L"Inline→Inline: Same range, callback ID changed")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: inline with callback ID 20
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(20, 100)));
			
			// New: different inline at same range (fully covers old - VALID)
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(21, 100)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// New inline appears in diff (property changed)
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateInlineProp(21, 100)));
			AssertDiffList(desc.runsDiff, expected, L"Inline replaced at same range");
			
			// Old inline removed, new inline created
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({21}), L"New inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({20}), L"Old inline removed");
		});
		
		TEST_CASE(L"Inline→Inline: Different cuts - 3 old inlines → 2 new inlines (decreased count)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: three inline objects at [0, 6], [6, 12], [12, 18] (cut at 6, 12)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 6}, DocumentRunProperty(CreateInlineProp(23, 30)));
			oldRuns.Add({.caretBegin = 6, .caretEnd = 12}, DocumentRunProperty(CreateInlineProp(24, 40)));
			oldRuns.Add({.caretBegin = 12, .caretEnd = 18}, DocumentRunProperty(CreateInlineProp(25, 50)));
			
			// New: two inlines at [0, 9], [9, 18] (cut at 9) - different cuts, fewer runs
			newRuns.Add({.caretBegin = 0, .caretEnd = 9}, DocumentRunProperty(CreateInlineProp(26, 90)));
			newRuns.Add({.caretBegin = 9, .caretEnd = 18}, DocumentRunProperty(CreateInlineProp(27, 90)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// New inlines in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 9, DocumentRunProperty(CreateInlineProp(26, 90)));
			AddExpectedRun(expected, 9, 18, DocumentRunProperty(CreateInlineProp(27, 90)));
			AssertDiffList(desc.runsDiff, expected, L"3→2 inlines with different cuts");
			
			// Old inlines removed, new inlines created
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({26, 27}), L"Two new inlines created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({23, 24, 25}), L"Three old inlines removed");
		});
```

**Rationale:**

Consolidated to 2 focused test cases using different cut points strategy:
1. Same range transformation (baseline)
2. 3→2 runs with different cuts (0-6-12-18 → 0-9-18): decreased count, different boundaries

This eliminates redundancy while covering:
- Callback ID replacement at same range
- Different cut points with count changes
- Callback lifecycle tracking (both creation and removal)

## STEP 6: Implement Error Detection Tests

Add test cases that intentionally violate the coverage requirement to verify error detection.

**Code Change:**

Continue inside the test category:

```cpp
		// Category 5: Error Detection - Invalid Range Coverage
		
		TEST_CASE(L"ERROR: Text→Inline with partial coverage (shifted right)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: text at [10, 20]
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: inline at [15, 25] - only covers [15, 20], leaving [10, 15] uncovered - INVALID
			newRuns.Add({.caretBegin = 15, .caretEnd = 25}, DocumentRunProperty(CreateInlineProp(100, 50)));
			
			// This should trigger CHECK_ERROR and result in TEST_ERROR
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
		
		TEST_CASE(L"ERROR: Text→Text with partial coverage (shifted left)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: text at [10, 20]
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: text at [5, 15] - only covers [10, 15], leaving [15, 20] uncovered - INVALID
			newRuns.Add({.caretBegin = 5, .caretEnd = 15}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// This should trigger CHECK_ERROR and result in TEST_ERROR
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
		
		TEST_CASE(L"ERROR: Text→Text with truncated coverage")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: text at [10, 20]
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: text at [10, 15] - only covers [10, 15], leaving [15, 20] uncovered - INVALID
			newRuns.Add({.caretBegin = 10, .caretEnd = 15}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// This should trigger CHECK_ERROR and result in TEST_ERROR
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
		
		TEST_CASE(L"ERROR: Multiple old runs with gap in new coverage")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: two consecutive text runs
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			
			// New: two runs with a gap [5, 7] uncovered - INVALID
			newRuns.Add({.caretBegin = 0, .caretEnd = 5}, DocumentRunProperty(CreateTextProp(0x80)));
			newRuns.Add({.caretBegin = 7, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// This should trigger CHECK_ERROR for first old run [0, 10]
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
		
		TEST_CASE(L"ERROR: Inline→Text with partial coverage")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: inline at [20, 30]
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateInlineProp(50, 80)));
			
			// New: text at [22, 35] - only covers [22, 30], leaving [20, 22] uncovered - INVALID
			newRuns.Add({.caretBegin = 22, .caretEnd = 35}, DocumentRunProperty(CreateTextProp(0x60)));
			
			// This should trigger CHECK_ERROR and result in TEST_ERROR
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
```

**Rationale:**

These test cases validate that the error checking in STEP 0 correctly detects invalid transformations. Each test uses `TEST_ERROR` to expect `CHECK_FAIL` from DiffRuns. These cover:
1. Shifted right (leaving left portion uncovered)
2. Shifted left (leaving right portion uncovered)
3. Truncated (new range shorter than old)
4. Gap in coverage (multiple new runs with gap)
5. Type transformation with partial coverage

## STEP 7: Implement Complex Mixed Scenario and Edge Case Tests

Add test cases for complex scenarios and edge cases, all maintaining full coverage.

**Code Change:**

Continue inside the test category and close it:

```cpp
		// Category 6: Complex Mixed Scenarios
		
		TEST_CASE(L"Mixed transformations: text→inline, inline→text, text→text in one diff")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: mixed runs
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));  // will become inline
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(30, 50)));  // will become text
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x60)));  // property change
			
			// New: transformed runs (all fully cover old - VALID)
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateInlineProp(31, 100)));  // text→inline
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));  // inline→text
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0xA0)));  // text→text
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// All three transformations in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateInlineProp(31, 100)));
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateTextProp(0x80)));
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateTextProp(0xA0)));
			AssertDiffList(desc.runsDiff, expected, L"Mixed transformations");
			
			// Callback tracking: one created, one removed
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({31}), L"Inline 31 created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({30}), L"Inline 30 removed");
		});
		
		TEST_CASE(L"Range splitting + type changes")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: single text run [0, 9]
			oldRuns.Add({.caretBegin = 0, .caretEnd = 9}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: split into inline + text + inline (fully covers old - VALID)
			newRuns.Add({.caretBegin = 0, .caretEnd = 3}, DocumentRunProperty(CreateInlineProp(40, 30)));
			newRuns.Add({.caretBegin = 3, .caretEnd = 6}, DocumentRunProperty(CreateTextProp(0xA0)));
			newRuns.Add({.caretBegin = 6, .caretEnd = 9}, DocumentRunProperty(CreateInlineProp(41, 40)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// All three new runs in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 3, DocumentRunProperty(CreateInlineProp(40, 30)));
			AddExpectedRun(expected, 3, 6, DocumentRunProperty(CreateTextProp(0xA0)));
			AddExpectedRun(expected, 6, 9, DocumentRunProperty(CreateInlineProp(41, 40)));
			AssertDiffList(desc.runsDiff, expected, L"Split with type changes");
			
			// Two inline objects created
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({40, 41}), L"Two inlines created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		// Category 7: Edge Cases
		
		TEST_CASE(L"Modify first run in paragraph")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: three runs, first will change
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: first run property changed, others unchanged (all fully covered - VALID)
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0xFF)));
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x80)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Only first run in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateTextProp(0xFF)));
			AssertDiffList(desc.runsDiff, expected, L"Only first run modified");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Modify last run in paragraph")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: three runs, last will change
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: last run becomes inline, others unchanged (all fully covered - VALID)
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateInlineProp(50, 100)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Only last run in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateInlineProp(50, 100)));
			AssertDiffList(desc.runsDiff, expected, L"Only last run modified");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({50}), L"Inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Modify middle run surrounded by unchanged runs")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: five runs, middle one will change
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x20)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x40)));
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x60)));
			oldRuns.Add({.caretBegin = 30, .caretEnd = 40}, DocumentRunProperty(CreateTextProp(0x80)));
			oldRuns.Add({.caretBegin = 40, .caretEnd = 50}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// New: middle run property changed (all fully covered - VALID)
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x20)));
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x40)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0xFF)));
			newRuns.Add({.caretBegin = 30, .caretEnd = 40}, DocumentRunProperty(CreateTextProp(0x80)));
			newRuns.Add({.caretBegin = 40, .caretEnd = 50}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Only middle run in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateTextProp(0xFF)));
			AssertDiffList(desc.runsDiff, expected, L"Only middle run modified");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
	});
```

**Rationale:**

These test cases verify complex scenarios and edge cases with full coverage maintained. Tests verify:
1. Mixed transformations in a single diff
2. Range splitting combined with type changes
3. First run modification (boundary at start)
4. Last run modification (boundary at end)
5. Middle run modification (isolated change)

All ranges maintain full coverage to be valid under the new error checking.

# TEST PLAN

## Overview

The test plan addresses the clarified requirement: **DiffRuns must validate that all old run ranges are fully covered by new run ranges, and report errors (CHECK_ERROR) when coverage is incomplete.**

Task 7 builds upon Task 6's basic DiffRuns tests and adds:
1. **Implementation change:** Error checking in DiffRuns to validate coverage
2. **Valid scenario tests:** ~14 test cases with correct coverage using efficient different-cut-point strategy
3. **Error detection tests:** ~5 test cases that intentionally violate coverage

## Test Categories and Coverage

### Category 1: Valid Text→Inline Object Transformations (3 test cases)

**Purpose:** Verify text→inline transformations with full coverage using different cut points efficiently.

**Test Cases:**
1. Same range, type changed - Baseline transformation
2. 3→2 runs with different cuts (0-10-20-30 → 0-15-30): **decreased count**, different boundaries
3. 2→3 runs with different cuts (0-15-30 → 0-10-20-30): **increased count**, different boundaries

**Coverage:**
- Type transformation detection (text→inline)
- Different cut points with run count **decrease**
- Different cut points with run count **increase**
- Callback ID tracking (`createdInlineObjects`)
- Full coverage validation passes

**Expected Behavior:**
- New inline objects appear in `runsDiff`
- Callback IDs appear in `createdInlineObjects`
- No errors (coverage is valid)

### Category 2: Valid Inline Object→Text Transformations (3 test cases)

**Purpose:** Verify inline→text transformations with full coverage using different cut points efficiently.

**Test Cases:**
1. Same range, type changed - Baseline
2. 3→2 runs with different cuts (0-8-16-24 → 0-12-24): **decreased count**, different boundaries
3. 2→3 runs with different cuts (10-25-40 → 10-20-30-40): **increased count**, different boundaries

**Coverage:**
- Type transformation detection (inline→text)
- Different cut points with run count **decrease**
- Different cut points with run count **increase**
- Callback ID tracking (`removedInlineObjects`)
- Full coverage validation passes

**Expected Behavior:**
- New text runs appear in `runsDiff`
- Old callback IDs appear in `removedInlineObjects`
- No errors (coverage is valid)

### Category 3: Text→Text Property Changes (4 test cases)

**Purpose:** Verify text property changes with full coverage, including the CRITICAL merge test. Uses different cut points efficiently.

**Test Cases:**
1. Same range, property changed - Baseline
2. **CRITICAL:** 3→1 run merge (1-2-5-10 → 1-10) - validates new key usage
3. 2→3 runs with different cuts (0-15-30 → 0-10-20-30): **increased count** + property changes
4. 4→2 runs with different cuts (0-5-10-18-24 → 0-12-24): **decreased count** + property changes

**Coverage:**
- Property change detection
- **CRITICAL:** Merged ranges use NEW complete keys
- Different cut points with run count **increase**
- Different cut points with run count **decrease**
- Full coverage validation passes

**Expected Behavior:**
- Changed runs appear in `runsDiff` with NEW keys
- **CRITICAL:** Merged run reports (1-10), NOT (2-5)
- No inline lifecycle changes
- No errors (coverage is valid)

### Category 4: Inline Object→Inline Object Changes (2 test cases)

**Purpose:** Verify inline object modifications with full coverage using different cut points efficiently.

**Test Cases:**
1. Same range, callback ID changed - Baseline
2. 3→2 runs with different cuts (0-6-12-18 → 0-9-18): **decreased count**, different boundaries

**Coverage:**
- Callback ID replacement detection
- Different cut points with run count **decrease**
- Full coverage validation passes

**Expected Behavior:**
- New inline appears in `runsDiff`
- Callback lifecycle tracked correctly
- No errors (coverage is valid)

### Category 5: Error Detection - Invalid Range Coverage (5 test cases)

**Purpose:** Verify that DiffRuns correctly detects and reports coverage violations.

**Test Cases:**
1. Text→Inline with shifted right (left portion uncovered)
2. Text→Text with shifted left (right portion uncovered)
3. Text→Text with truncated coverage (right portion uncovered)
4. Multiple old runs with gap in new coverage
5. Inline→Text with partial coverage

**Coverage:**
- Error detection for partial coverage
- Error detection for shifted ranges
- Error detection for gaps in coverage
- Error detection across different transformation types

**Expected Behavior:**
- Each test uses `TEST_ERROR` with `DiffRuns(...)` call as the expression
- DiffRuns throws error via `CHECK_ERROR` about uncovered range
- Test passes when error is correctly thrown

### Category 6: Complex Mixed Scenarios (2 test cases)

**Purpose:** Verify complex scenarios with multiple transformation types.

**Test Cases:**
1. Mixed transformations in one diff
2. Range splitting + type changes

**Coverage:**
- Simultaneous multiple transformation types
- Combined range and type operations
- Full coverage validation passes

**Expected Behavior:**
- All transformations correctly reported
- Callback tracking correct across mixed types
- No errors (coverage is valid)

### Category 7: Edge Cases (3 test cases)

**Purpose:** Verify boundary conditions.

**Test Cases:**
1. Modify first run
2. Modify last run
3. Modify middle run with unchanged neighbors

**Coverage:**
- First position handling
- Last position handling
- Middle position handling

**Expected Behavior:**
- Only modified runs in `runsDiff`
- Unchanged neighbors not in diff
- No errors (coverage is valid)

## Test Execution Strategy

### Phase 1: Implement Error Checking (STEP 0)

Update `DiffRuns` to add coverage validation. This is critical infrastructure.

**Success Criteria:**
- Error checking code compiles
- Existing Task 6 tests still pass (they have valid coverage)

### Phase 2: Valid Scenario Tests (Categories 1-4, 6-7)

Execute ~14 test cases covering valid transformations using efficient different-cut-point strategy.

**Success Criteria:**
- All tests pass without errors
- `runsDiff` contains expected entries
- Callback tracking is correct
- **CRITICAL test passes:** merged runs use complete new ranges

### Phase 3: Error Detection Tests (Category 5)

Execute 5 test cases that intentionally violate coverage.

**Success Criteria:**
- All tests correctly detect errors using `TEST_ERROR`
- DiffRuns throws `CHECK_FAIL` with appropriate messages
- Different types of coverage violations are all detected

## Corner Cases Covered

**Efficient Different-Cut-Point Strategy:**
1. **Same range transformations:** Type or property changes without position changes (baseline)
2. **Different cuts with count decrease:** Old cuts at (10, 20) → New cuts at (15) - fewer runs, different boundaries
3. **Different cuts with count increase:** Old cuts at (15) → New cuts at (10, 20) - more runs, different boundaries
4. **Mixed transformations:** Multiple transformation types in one diff
5. **Boundary positions:** First run, last run, middle run
6. **Error: Shifted right:** Old range left portion uncovered
7. **Error: Shifted left:** Old range right portion uncovered
8. **Error: Truncated:** Old range right portion uncovered
9. **Error: Gap in coverage:** Multiple new runs with gap
10. **Error: Type transformation with partial coverage:** Transformation doesn't maintain coverage

**Key Insight:** By testing different cut points systematically (e.g., 0-10-20-30 → 0-15-30, and 0-15-30 → 0-10-20-30), we cover both increased and decreased run counts with different boundaries in fewer test cases, eliminating redundancy from the original plan.

## Validation Approach

**Valid scenario tests:**
1. Setup: Create `oldRuns` and `newRuns` with full coverage but different cut points
2. Execute: Call `DiffRuns(oldRuns, newRuns, desc)`
3. Verify: Use `AssertDiffList()` and `AssertCallbackIdList()`
4. Result: No errors, expected diff produced

**Error detection tests:**
1. Setup: Create `oldRuns` and `newRuns` with INVALID coverage
2. Execute: Use `TEST_ERROR(DiffRuns(oldRuns, newRuns, desc))` - expects `CHECK_ERROR` to be thrown
3. Result: Error correctly thrown and caught by `TEST_ERROR`

## Changes from Original Plan

**Key Optimization - Different Cut Points Strategy:**
- **Before:** Separate tests for "multiple→one", "one→multiple", each with same cut points
- **After:** Consolidated tests using different cut points (e.g., 0-10-20-30 → 0-15-30 and reverse)
- **Benefit:** Covers increased/decreased/same run counts with different boundaries more efficiently

**Removed test cases (redundant):**
- Separate "multiple text→one inline" and "one text→multiple inline" with same cuts
- Separate "multiple inline→one text" and "one inline→multiple text" with same cuts
- "Range changed, property unchanged" (covered by different-cut-point tests)
- "Both range and property changed" (covered by different-cut-point tests with properties)
- Various inline object "range changed" tests (covered by different-cut-point strategy)

**Added components:**
- **STEP 0:** Error checking implementation in `DiffRuns`
- **Category 5:** Error detection test cases (5 tests)
- **Different-cut-point strategy:** Systematically tests count increase/decrease with different boundaries

**Test count change:**
- **Original plan:** ~22 tests (17 valid + 5 error)
- **Updated plan:** ~19 tests (14 valid + 5 error)
- **Efficiency gain:** 15% fewer tests with same comprehensive coverage

## Success Metrics

- **Error checking implemented** in `DiffRuns`
- **All ~14 valid scenario tests pass** using efficient different-cut-point strategy
- **All 5 error detection tests pass** (correctly detect invalid coverage)
- **CRITICAL requirement validated:** Merged runs use complete new ranges
- **Callback lifecycle correctly tracked** in all scenarios
- **Edge cases handled correctly**
- **Different cut points with increased/decreased counts** validated

## Risk Analysis

**Low Risk:**
- Error checking implementation (straightforward validation logic)
- Basic transformations with same ranges
- Edge cases (well-defined boundaries)

**Medium Risk:**
- Error detection tests (must use `TEST_ERROR` correctly with expression, not code block)
- Complex mixed scenarios

**High Risk:**
- **CRITICAL test:** 3→1 run merge with different properties (most important validation)

## Fallback Plan

If error detection implementation fails:
1. Analyze failure in valid tests (may indicate overly strict validation)
2. Review error checking algorithm
3. Fix validation logic
4. Re-run all tests

If error detection tests fail to catch errors:
1. Verify `TEST_ERROR` usage is correct
2. Check that invalid test cases truly violate coverage
3. Debug error checking logic
4. Update tests if needed

# !!!FINISHED!!!
