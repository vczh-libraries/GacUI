# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

[TODO\_Task\_EazyLayoutSplitter.md](../../../TODO_Task_EazyLayoutSplitter.md)

additionally, I noticed that there is another pattern to clean up
```php-template
<Bounds AlignmentToParent="left:0 top:0 right:0 bottom:0">
  <ez:Layout/>
</Bounds>
```

Since ez:Layout is also a bounds, some properties should just be applied ez:Layout directly and remove bounds.

I think part of the reason is that `BuildLayout` would affect `AlignmentToParent`, but a bounds with 0 alignmenttoparent actually does nothing when the only child is `ez:Layout`, so alignmenttoparent does not need to add to ez:Layout in this case. But if there is other numbers, please think about is it able to be made by the combination of `Padding`, `Border` and `Margin` of ez:Layout.

There might be multiple GacUI\_Controls tutorials need to clean up, please pay attention to this, remember to make sure visible layouts of affected tutorials don't change.

# UPDATES

## UPDATE

A commit of only XML files should be made before committing other files

## UPDATE

you are right, Margin does not exist, it was removed a few years ago I though it was still there, but that's fine, Only keep the bounds when its AlignmentToParent could not be reproduced

# TEST [CONFIRMED]

Extend the existing direct composition, rejection, compiler and visible-frame tests according to TODO_Task_EazyLayoutSplitter.md. First reproduce the missing XML descriptor through the compiler. Verify both axes, reordered predecessors, shared boundaries, spans, forced docking tables, gutters, native dragging, rebuilding and ownership. Compare wrapped and unwrapped layout geometry independently, including asymmetric insets and preferred minimum sizes.

Capture tutorial baseline bounds and screenshots before XML edits. Regenerate resources through the owning tools, build both architectures and compare affected applications and shared-resource consumers. Passing builds alone do not establish visual equivalence. Inspect test summaries, compiler errors and Debug leak reports.

The new compiler acceptance test built successfully in Debug x64 (zero warnings/errors) and failed against the original runtime with `Precompile: Failed to find type "ez:Splitter".` followed by the expected zero-error assertion failure. Confirmed in CDB with `/F:TestResource.cpp`; an earlier full-suite reproduction run was stopped before reaching that file. No source/test commit was made because the XML-only commit must come first.

# PROPOSALS

- No.1 Resolve splitter markers in the existing layout plan and remove equivalent tutorial wrappers [CONFIRMED]

## No.1 Resolve splitter markers in the existing layout plan and remove equivalent tutorial wrappers

The current builder has no splitter descriptor kind. Add a leaf descriptor and reflection entry, preserving authored predecessor associations separately from generated tracks. Resolve interior boundaries during preparation, coalesce shared boundaries, and generate existing native splitters after table cells. Marker-bearing docking groups use the task's edge-aligned or trailing-content table representation; unmarked groups retain their current path.

BuildLayout explicitly replaces the layout root's AlignmentToParent with its border inset. Remove zero-aligned wrappers only after verifying minimum-size propagation. The current API has no Margin property. Existing InternalMargin is not an extra outside inset: explicitly aligned generated children ignore it. A wrapper inset equal to Padding can become Border=true when Border was false. Retain other insets and one-sided anchoring when the current spacing API cannot express them without changing child gutters. Do not add an unrelated public property merely to eliminate these necessary wrappers.

### CODE CHANGE

Added the runtime leaf kind, marker registration and validation, ordinary and shared boundary plans, forced docking tables, native splitter construction, reflection registration and XML grammar checks. The native splitter owns all mouse resizing behavior; descriptor options remain unchanged until an explicit rebuild restores their configured sizes.

Extended direct composition tests for both axes, all docking representations, explicit/inherited/fallback direction, empty tracks, reordered predecessors, shared and duplicate boundaries (including a rejected zero-span cell), native drag limits and inert option pairs, rebuilding, ownership, and equivalent Bounds removal. Extended compiler rejection/acceptance cases and visible frame tests for shared/nested splitters, dragging, resizing, rebuilding, and retained bindings. Added an inert splitter between the existing percentage fills in FullControlTest so generated C++ consumers compile and instantiate the descriptor under all reflection configurations without changing the initial geometry.

Removed 38 redundant Bounds wrappers from 19 authored controls XML files and replaced AddressBook's native table/splitter with the planned column descriptors. Removed the newly redundant GroupBox alignment assignments. Preserved seven Bounds wrappers: AddressBook's two 3-pixel insets, ColorPicker2's 1-pixel inset, DataGrid's 2-pixel inset, unanchored ScrollContainer content, the ribbon header's bottom anchoring, and TriplePhaseImageButton's top-left anchoring. Preserved CalculatorAndStateMachine's native table and ColorPicker2's repeated palette. Documented wrapper removal in EazyLayout.md.

The user's XML-only-first instruction was honored before committing source, tests, generated outputs or this investigation: Release commit `5545d330` contains only the 20 affected XML files, and the first GacUI commit contains only the upstream FullControlTest XML. Release commit `2485a24d` then publishes the runtime and regenerated resources. Both Release commits have been pushed. Generated files are produced only by their owning compiler, metadata, release and test workflows.

### CONFIRMED

The full Debug x64 unit run passed 93/93 test files and 1793/1793 cases, with no appended Debug leak report. Four further focused cases were added afterward, extending direction inference/empty tracks, reordered trailing markers/local table conversion, duplicate boundaries around a rejected zero-span cell, and the existing XML construction coverage. The final focused debugger runs passed all 4 selected files and 50/50 cases on both Win32 and x64, including explicit property-list construction and namespace overrides. No runtime implementation changed between these runs, and no Debug leak dump appeared. Unrelated nondeterministic application snapshots and binding-declaration ordering were excluded from the changes; new splitter snapshots and updated descriptor fixtures were retained.

The final Debug Win32 and x64 source solution builds passed with zero warnings and errors. An earlier x64 build reported one test-only integer-shift warning; the shift now uses `vint(1)`. Required Win32 metadata generation, x64 metadata generation, and x64 metadata tests passed before publication and again after resource generation and final builds. A later Win32 metadata launch displayed a native abort dialog; after inspecting and stopping that exact process, both the debugger rerun and the sequential wrapper rerun exited successfully without reproducing the abort. Its cause was not established. Final generated metadata for both architectures exactly matches the published files.

`GacUI_Compiler` completed both architectures successfully, with no resource-error artifacts. The Tools release workflow repacked GacUI, rebuilt GacGen and generated skins, and published metadata and imported sources. Tutorial caches were cleared with the owning tool before full `GacBuild` regeneration. The final tutorial generation had no resource errors, and `GacUI_Controls.sln` passed Debug Win32 and x64 builds with zero warnings and errors.

The Win32 generated C++ showcase was launched through the execution wrapper under full reflection, metadata-only reflection, and no reflection. Each opened the Easy Layout tab, rebuilt into the vertical arrangement, restored the horizontal arrangement, retained the bound text, and exited normally. The Workflow-binary-loaded `GacUI_Host` passed the same operations. These checks instantiate the new splitter through all requested construction paths.

Visual and interaction evidence is stored locally in `C:\Users\vczh\AppData\Local\Temp\GacUIRefactor-20260919`, with separate `splitter-before` and `splitter-after` captures, per-state metadata, HTTP trees, screenshots, and UIA logs. Baseline metadata records the source revision, client/window dimensions, renderer, fonts and 96-DPI scale.

- All 18 runnable tutorial applications completed initial, small, large, maximized, restored, and initial-restored captures. All 108 main-window geometry comparisons match after ignoring removed anonymous containers. AddressBook's six corresponding PNGs are pixel-identical.
- Across 187 paired size and interaction states, 186 complete visible-geometry comparisons match. The remaining complex-animation state has different moving-circle positions/counts because elapsed time differs; its fixed viewport and controls match and both screenshots were inspected. Localization clock strings likewise differ with time without moving controls. Minor active/inactive window colors do not indicate a layout change.
- The interaction checks cover all ListControls views, text-list mutations and tree expansion; scrolling; dialog tabs; palette popups; DataGrid views and filtering; both shared DocumentEditor consumers' menus, ribbon tabs, text selection and hyperlink dialogs; animation tabs; menu visibility; download completion; QueryService binding; GlobalHotKey menus; and localization selection.
- The Toolstrip hyperlink-dialog baseline helper could not open its second-level menu. The final helper used its toolbar dropdown successfully; the resulting shared dialog matches the Ribbon consumer's baseline dialog geometry. This is a shared-resource comparison, not a claimed paired Toolstrip baseline capture.
- AddressBook additionally passed repeated drags in both directions, both limits, top/middle/bottom gutter hit tests, resize after dragging without resetting the absolute width, and restoration to 180 pixels. Category selection, all six contact views, new/edit/cancel dialogs, creating/editing/deleting a contact, and retained bindings were exercised. There are 23 additional AddressBook states and three additional Toolstrip states.
- All 18 applications passed subsequent UIA ControlView traversal, window maximize/restore and resize/restore, available tab selection, and text/value access checks. A stale-element error in the Ribbon verification helper was resolved by reacquiring controls after responsive rebuilding; the application required no change.

The seven retained wrappers cannot be folded into the current layout spacing without changing visible behavior:

| Resource | Retained behavior |
| --- | --- |
| AddressBook, two wrappers | Independent 3-pixel inner insets with 5-pixel child gaps |
| ColorPicker2 | 1-pixel outer inset with zero child padding |
| DataGrid Components | 2-pixel outer inset with zero child padding |
| ContainersAndButtons ScrollContainer | Unanchored minimum-sized scroll content with 10-pixel padding |
| DocumentEditorRibbon AfterHeaders | Bottom anchoring with an unconstrained top edge |
| TriplePhaseImageButton | Top-left 10-pixel offset with unconstrained right/bottom edges |

CalculatorAndStateMachine retains its native table because of spanning cells and its `//Cell/Button` selector. ColorPicker2 retains its repeated/wrapping palette. Neither arrangement becomes equivalent merely because splitters are supported.
