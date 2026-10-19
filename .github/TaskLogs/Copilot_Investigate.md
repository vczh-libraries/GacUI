# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

The new `TouchingList` created in [TODO_Touch_Step3R1.md](TODO_Touch_Step3R1.md) has a little issue, please fix it:

- The item template doesn't look good. By saying lorem ipsum I meant to call the integrated lorem ipsum API so that each list item will have different content. And you don't need to tell the user how to drag in the list item, you can put a label aside the `Add 5 items` button as there are still many spaces. And it does not respond to selection, I would suggest you use the selected color theme from darkskin, they should be accessible during compiling `FullControlTest`, and then you can use all colors properly instead of white background black text.
- When scrolling the list very fast seems a big piece of red renders before the white part, I don't know why it looks confusing to me. Did you implement the dragging visual effect inside the item template? For example, you can add a "DragDistance" and "MaxDragDistance" property on the item template, and when dragging it you can actually retrieve the item template from the target item and set the "DragDistance" to show the `DELETE IT` red/white text in the right side. "MaxDragDistance" could be calculated. This property will be 0 gy default so I think this should avoid rendering a big piece of red while scrolling.

# TEST [CONFIRMED]

Extend the existing real-resource TouchingList case in `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_PointScrolling.cpp`. A closed row must contain no visible red background, including initial realization, cancellation, reset and rapid scrolling/recycling. Verify generated lorem-ipsum bodies differ, DarkSkin normal/selected/disabled colors, measured reveal width and partial/full/reset/deferred-removal frames. Retain single-contact completion, second-contact cancellation, final-Up motion and Clear/Add identity checks.

Update `<GacUI repo>/Test/UIA_CppTest_Shared.cs` and run both focused hosted/native wrappers plus the interpreted consumer. Check the help label beside Add, varying item text, selection, no red during scrolling, and existing swipe outcomes through automation and UIA. Regenerate resources, required Win32/x64 metadata, rebuild consumers, and inspect completed build/test summaries and leak output.

# PROPOSALS

- No.1 Template-owned reveal and DarkSkin presentation [CONFIRMED]

## No.1 Template-owned reveal and DarkSkin presentation

The baseline Debug x64 build passed. The new closed-row regression failed in the first Ready frame: the row contains an enabled red background at zero reveal. The authored template paints that background across the entire slot while its covering foreground gets its size from an asynchronous CachedBounds binding. This allows newly realized rows to expose red before the foreground catches up. The foreground also hard-codes light colors and never reads Selected. Every item currently repeats the same prose and instructions.

Move DragDistance (default zero) and calculated MaxDragDistance into the template. Restrict red to the requested right-edge width and hide it completely at zero. Align the closed foreground directly to its parent, and translate only an actively dragged foreground. Resolve the current visible template from the remembered item identity on each event, preserving deferred membership-checked removal without retaining a template across recycling. Read the installed DarkSkin palette when constructing each row, bind selection/enabled colors, generate each body with Sys::LoremIpsumSentence, and place one themed help label beside Add.

### CODE CHANGE

Updated the authored TouchingList resource with template-owned drag state, bounded red background, DarkSkin normal/selected/disabled colors, generated sample text and the shared toolbar hint. Extended its real-resource regression with closed-row rendering checks during realization and recycling, palette assertions and measured partial reveal. The shared native/UIA and remote/browser tests now verify the revised presentation and absence of red when closed. Generated outputs come from GacUI_Compiler and the required metadata workflow.

The rendered regression exposed deferred text measurement when the delete label itself was hidden: MaxDragDistance changed from 33 to 96 after the first reveal. Keep the measured label in the stable row slot underneath the parent-aligned opaque foreground; only the bounded red background is hidden at zero. This preserves pre-swipe measurement while preventing red during realization. The row data model now contains only Text; list events reacquire the template through the arranger using current item membership.

The real-resource test temporarily registers native DarkSkin reflection so the authored template can resolve the installed palette. It removes newly registered descriptors after resource teardown, preserving the core-only reflection inventory checked by subsequent tests.

### CONFIRMED

The zero-distance template has no visible red element and its opaque foreground fills the row directly, without waiting for CachedBounds. Red width follows only the current DragDistance. Initial realization, synchronous scroll/recycle checks, settled frames and live repeated scrolling all passed. The measured delete label remains available before the first swipe. Normal, selected and disabled colors match DarkSkin, and generated row bodies differ. Existing deferred identity removal, cancellation, second-contact and final-Up behavior remain correct.

- GacUI_Compiler completed both architectures successfully; only FullControlTest generated outputs changed, and no resource error files remain.
- Final Debug Win32 and x64 builds passed with zero warnings/errors. Metadata_Generate passed on both architectures, Metadata_Test x64 passed, and the final consumer rebuild passed.
- Win32 unit filter: 11/11 files and 225/225 cases. x64 unit filter: 13/13 files and 245/245 cases. Completed logs contain no leak output. The final tests include normal/selected/disabled colors and rapid closed-row recycling.
- Focused automation/UIA: CppTest on Win32 and x64, CppTest_Metaonly x64, and interpreted GacUI_Host x64 each passed 6,826 assertions and normal shutdown. These cover hint placement, differing text, selection, measured swipes and repeated scrolling.
- Http/Direct2D native remote checks passed, including renderer pixels, forwarding, TouchingList swipe/reset/removal/scrolling and shutdown. Chromium CDP touch checks passed with no page errors, including closed-row checks, gesture outcomes, scrolling, palette replacement and shutdown.
- Reviewed Chromium screenshots of reset and partial reveal: rows use the dark palette, closed rows have no red, and a partial swipe exposes only the narrow red region. Reviewed authored and generated diffs; git diff --check passed.
