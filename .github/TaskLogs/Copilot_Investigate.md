# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

your implementation is overall good but still need a few refactoring.

1\) `GuiGraphicsHost::activePoints` looks useless, it is only being updated and copied to points, you can just maintain `points`. As `points` is only kept glowing so there should be no trouble.

2\) I have updated the [GuiGraphicsEventReceiver.h](<GacUI repo>/Source/Application/GraphicsCompositions/GuiGraphicsEventReceiver.h) , open the last commit and check out what I did, my idea is that having 5 events on the composition make event consumption too heavy, so I compress 5 into 1, but it does not compile because affected code are not fixed. You are going to fix it, and if anything in [TODO_Touch.md](<GacUI repo>/TODO_Touch.md) does not reflect to this new design, you are going to also fix that.

# TEST [CONFIRMED]

- Reproduce the incomplete event migration with the Debug x64 solution build.
- Verify the unified event discriminator, Started constructors, reflection, and authored resource consumers.
- Cover sparse point slots, lifted contacts, receiver-relative coordinates, handled propagation, and retained stationary contacts after movement and DPI changes. The host must retain only one point list.
- Run the required unit tests, Win32/x64 metadata generation, Metadata_Test, resource generation, and regenerated consumer builds through the repository wrappers. Exercise the existing Touching showcase automation in both entry wrappers.

The initial Debug x64 build failed with 67 errors and no warnings. Diagnostics include removed point-event members in dispatch/initialization/reflection and mismatched GuiPointEventArgs constructors, confirming the incomplete migration.

# PROPOSALS

- No.1 Complete unified point events and maintain one sparse point list. [CONFIRMED]

## No.1 Complete unified point events and maintain one sparse point list

Finish the latest commit's pointAction design, with GuiPointEventsType distinguishing Down, Update, Up, Enter and Leave and Started reserved for the acceptance query. Register the enum and eventType field and migrate authored consumers before regenerating their outputs. Replace activePoints with in-place point-list rebasing, preserving holes and the captured sequence route. Update the design document and public comments.

### CODE CHANGE

Updated both GuiPointEventArgs constructors, event initialization, dispatch and reflection. Added public enum/action documentation. Replaced the native-sample dictionary with the sparse logical point list and its coordinate origin/scale; input refreshes only the changed slot, Up clears only its slot, and propagation shifts occupied entries in place. Migrated both authored XML consumers to pointAction, with the inspector reading eventType. Updated the interface declarations and routing/storage descriptions in <GacUI repo>/TODO_Touch.md. Added ordinary unit-test coverage of the reflected API, constructor kinds and borrowed sparse list in <GacUI repo>/Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp.

The initial Win32 implementation build compiled the libraries, metadata generator and UnitTest; its remaining 40 errors were confined to stale generated FullControlTest subscriptions. Win32 Metadata_Generate succeeded. The configured Win32 unit suite passed 4/4 files and 45/45 cases, including the new reflected point-event test, with no leak dump.

Playground verification caught a Workflow compile error in the new inspector trace: enums cannot cast directly to string. CDB located the failure at CompileResources, and the generated resource diagnostic identified both casts. Replaced them with an explicit event-name helper, matching the existing mouse-button sample pattern; both Started and action traces now consume eventType. The corrected resource compiles and runs on both architectures. Native automation verified all five action kinds, sticky Started acceptance, source identities, parent-relative coordinates, sparse growth across native-ID reuse, Up holes, and successful End. The x64 inspector also verified stationary point rebasing after a (20,30) window move, populated canceled-End slots, sequence reset, handled-query propagation, rejection with no actions, and clean exit. A test-driver mistake sent Up after retiring that contact with CaptureLost; CDB identified the existing Contact-is-not-down guard, and the corrected sequence passed. No production change was needed for that invalid stimulus.

The initial x64 implementation build likewise compiled the libraries and failed only on the 40 stale generated FullControlTest references. Metadata_Generate x64 and Metadata_Test succeeded. The configured x64 unit suite passed 7/7 files and 82/82 cases, including the new reflected event test, with no leak dump. Its Raw/Properties textual snapshot changed only declaration order; retain that generated test output in a separate snapshot commit. GacUI_Compiler completed successfully for both architectures; no resource diagnostics or incomplete generated files remain. Generated FullControlTest now subscribes once to pointAction, and later generated closure IDs are renumbered. Final consumer builds subsequently passed as recorded below.

A native DPI-notification test exposed a regression in the origin-only rebasing: after a 96-to-144 DPI change, a stationary second point incorrectly became (398,203) relative to the child. Preserve the single list and retain its host-level conversion scale alongside its origin. Rebase occupied logical screen positions by the old/new scale ratio before subtracting the new origin. Verify both an untouched stationary point and the next changed-point payload across DPI changes. No per-contact native cache is needed.

The final Debug Win32 solution build passed with zero warnings and errors. Metadata_Generate Win32 succeeded, and the configured unit suite again passed 4/4 files and 45/45 cases with no leak dump. The rebuilt Win32 inspector passed a native 96-to-144-to-96 DPI notification sequence: the untouched contact matched the expected native-to-logical position at 144 DPI and differed by one logical unit after returning to 96 DPI, consistent with retaining integer logical coordinates. Its changed-point payload and matching list slot agreed. Subsequent movement, canceled occupied slots, sequence reset, all five action kinds, successful End, and normal shutdown also passed. Final x64 and showcase results are recorded below.

### CONFIRMED

The single pointAction event now delivers all five action kinds, while Started and End remain separate lifecycle events. Constructors, event initialization, native-to-composition dispatch, reflection, authored XML, and generated C++/Workflow consumers agree on that API. The host retains one growing sparse point list instead of activePoints; origin/scale bookkeeping preserves receiver-relative positions across bubbling, movement, and DPI changes. Lifted slots stay empty, and the list remains available until End propagation finishes.

Final verification:

- Debug Win32 and x64 solution builds passed, both with zero warnings and zero errors.
- GacUI_Compiler completed both architectures without resource diagnostics. After regeneration, Metadata_Generate passed on Win32 and x64, followed by Metadata_Test on x64.
- The configured unit suites passed 4/4 files and 45/45 cases on Win32, and 7/7 files and 82/82 cases on x64. Finalized logs contained no leak dump.
- Native automation verified every action kind, sparse IDs/holes, sticky acceptance, handled query propagation, rejection, captured source identities, receiver-relative coordinates, movement, canceled and successful End, reset, and clean exit. The rebuilt Win32 app also passed the DPI-change checks described above; retaining integer logical coordinates can introduce rounding during DPI changes.
- The default Direct2D Touching scenarios in <GacUI repo>/Test/UIA_CppTest.ps1 and <GacUI repo>/Test/UIA_CppTest_Metaonly.ps1 each passed 152 assertions, including native contact transforms, membership changes, cancellation/draining, source reuse, first-contact fallback, UIA selection/bounds/resize, normal shutdown, and endpoint release. These cover hosted and ordinary windows respectively. The test script could not parse a UTF-8 BOM decoded from local debugger settings; removing the BOM temporarily allowed execution, and the original bytes were restored after each run.
- Generated source changes consist of the unified subscription and resulting closure renumbering. The separate Raw/Properties snapshot update only reorders declarations. No physical-touch hardware session was performed.
