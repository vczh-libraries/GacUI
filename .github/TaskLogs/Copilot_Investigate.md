# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Continue the ViewSizes implementation after review found that size overrides alone do not replace the permanent responsive view used by the original sample.

# UPDATES

## UPDATE

we continue to implement this refactoring according to your analysis:

- Now `ViewSizes` will contribute permanent levels no matter any direct/indirect child responsive compositions contribute those levels or not. Keep the original test coverage design to have that 4 or 8 cases, it help to verify the change
- I would expect to see in [EasyLayoutResponsiveWindow.xml](<GacUI repo>/Test/Resources/App/FullControlTest/EasyLayoutResponsiveWindow.xml) extra works could be removed leaving only `ViewSizes` replacing the original trick.

Only when the above refactoring succeeded, continue to update stale fact in KB pages and website created for the previous work and publish the website.

# TEST

- Keep the four ordinary ViewSizes cases (empty, shorter, equal and longer arrays), each exercising Horizontal and Vertical with direct and indirect children. Traverse effective thresholds repeatedly, including child-contributed levels beyond the array and array-contributed levels beyond the children.
- Verify permanent levels and current state while all responsive children are absent, resizing in that interval, initializing reattached children from the group, and synchronous row replacement from CurrentLevelChanged without extra notifications. Verify assignment, clearing, direction changes, owned array values and ordinary child-derived behavior with an empty array.
- Build Debug Win32/x64 and regenerate FullControlTest and metadata through the prescribed wrappers. Run unit tests and metadata comparison. Verify only ribbon document, responsive and Easy Layout responsive pages through native, hosted and Workflow consumers, including repeated resizing, scrolling, selection and reopening.
- Documentation and website publication start only after the implementation and focused verification succeed.

# PROPOSALS

- No.1 Child-derived levels with application-maintained restoration [DENIED]
- No.2 Permanent ViewSizes levels inside the responsive group [CONFIRMED]

## No.1 Child-derived levels with application-maintained restoration

### CODE CHANGE

The previous implementation used ViewSizes only for PreferredMinSize and added movieLevel, accountLevel and restoringMovieLevel to the sample. It also removed the responsive container's size-change gate.

### DENIED BY USER

The implementation did not preserve the original permanent view's level contribution and current state during virtual row removal. The sample became more complicated instead of replacing the original XML trick. The user's revised contract explicitly makes ViewSizes contribute permanent levels.

## No.2 Permanent ViewSizes levels inside the responsive group

Model the configured array as an implicit permanent responsive view: its count contributes to the group's maximum level count, and its own current level participates in the maximum current level. SetViewSizes initializes this contribution at its largest level, matching a newly configured ResponsiveView, and clearing removes the contribution. LevelUp/LevelDown update it together with real participating children, before the group publishes its final level change. Extrapolation still applies when children supply additional levels. An empty array preserves the original behavior.

Restore the original container gate and validate that permanent levels remove the sample's need for the broader layout change. Keep the independent virtual-repeat measurement-reset ordering fix. Simplify the sample back to the original initialization/reload flow, using the group's CurrentLevel and replacing the explicit view/fixed compositions with ViewSizes.

### CODE CHANGE

Implemented viewSizeLevel, count/current-level aggregation and transitions in <GacUI repo>/Source/GraphicsComposition/GuiGraphicsResponsiveComposition.h/.cpp. Restored Layout_CalculateBounds from before the previous ViewSizes work. Removed the sample's three state fields and restoration handlers in <GacUI repo>/Test/Resources/App/FullControlTest/EasyLayoutResponsiveWindow.xml; retained the original row initializer and movie reload using group.CurrentLevel. Extended the four tests in <GacUI repo>/Test/GacUISrc/UnitTest/TestCompositions_Responsive.cpp to both axes and direct/indirect children, permanent empty-group resize, synchronous event-driven replacement, assignment/clearing and direction filtering.

Added a direct 240-to-680-to-240 resize sequence to <GacUI repo>/Test/UIA_CppTest_Shared.cs, retaining its appearance, row-height, scrolling and selection assertions.

Validation so far: Debug x64 and Win32 solution builds passed with zero warnings/errors. The unfiltered x64 UnitTest run passed 1,820/1,820 cases across 93/93 files with no appended leak report. The four expanded ViewSizes cases passed with the original container gate. Reviewed and restored 42 unrelated snapshot outputs: 31 file-dialog paths/listings/timing, two current-month calendar frames, and nine Windows supplementary-character caret outputs. Generated-resource and focused application verification remain pending.

Metadata_Generate passed on Win32 and x64, and Metadata_Test passed on x64. The initial full GacUI_Compiler pass was deliberately canceled to avoid repeatedly rebuilding unchanged resources. For focused generation, temporarily select BUILD_FULLCONTROLTEST and load the existing dialog, snapshot-viewer and DarkSkin binaries before compiling; CompilerMain already generates both x86 and x64 outputs in one invocation. Restore the compiler's original source before final consumer builds and commits.

Focused FullControlTest generation completed successfully for x86 and x64. Restored the compiler source before final consumer builds; both rebuilt successfully with zero warnings/errors, followed by successful Win32/x64 Metadata_Generate and x64 Metadata_Test. No resource error files remain. Generated diffs include closure ordering/identifier changes and removal of the sample's fields and handlers.

The native CppTest_Metaonly focused scenario passed 1,877 assertions and normal shutdown/endpoint release. The added direct 240-to-680-to-240 resize restored movie heights from 26 to 106 to 26. UiaList loaded the Easy Layout window's preview and 32-node UIA tree, including the account menu and dialog actions, then exited normally. Windows screenshot capture returned black pixels for both the sample and inspector; visual pixel review is unavailable in this session. HTTP geometry/appearance assertions and UIA operation checks remain the runtime evidence.

### CONFIRMED

The Workflow-binary GacUI_Host scenario passed 1,879 assertions, and hosted CppTest passed 1,873 assertions. Both completed normal shutdown and endpoint release. All three runs covered only ribbon document, responsive and Easy Layout responsive pages, with direct and intermediate resizes, row-height contraction, end-scrolled resizing, unseen rows, selection and reopening. Workflow tab-header checks used Controls geometry because that consumer's UIA tab bounds describe page content. Final source review confirmed that the sample diff against the original placeholder-view version only replaces the two views with ViewSizes and changes the row initializer/factories to reference the named groups. No additional sample restoration state remains. Proceed to the authorized KB/manual corrections and website publication.
