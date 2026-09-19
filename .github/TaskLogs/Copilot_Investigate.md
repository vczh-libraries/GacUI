# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

TODO_Task_UiaReview2.md, fill the revealed gap between UIA for GacUI and requirements from MSDN

# TEST [CONFIRMED]

Reproduce the R2-01 through R2-24 contracts from `TODO_Task_UiaReview2.md` in the existing UIA client and Playground fixtures. Preserve the review as historical evidence. Cover ordinary HWND and hosted configurations, observable state and identity, rejected operations without mutation, Unicode text units and event delivery. Run the configured UnitTest through repository wrappers and inspect its completed log for failures and leaks. Validate reflection metadata in both architectures if public registrations change.

# PROPOSALS

- No.1 Repair the provider contracts at their semantic and control state owners [CONFIRMED]

## No.1 Repair the provider contracts at their semantic and control state owners [CONFIRMED]

The initial ordinary Playground `Review2` run reproduces six failures covering R2-15 through R2-18: supplementary-character attribute search and Format expansion, caret attributes, degenerate movement counts, nondegenerate failed-move preservation and trailing Word separators. Extend this regression set as each remaining area is implemented.

Use complete renderer-character intervals for text attributes and unit boundaries, preserve failed movement endpoints, expose logical document objects, and map point ranges to embedded children. Repair navigation/focus, selection-group identity, scrolling and metadata in the semantic adapter; publish model and window changes at their owning transitions. Use explicit accessibility metadata where labels/localization cannot be inferred safely.

R2-04 requires qualification: Microsoft's [Assigning a New Parent](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-serversideprovider#assigning-a-new-parent) explicitly instructs known popup providers to retain `UiaHostProviderFromHwnd` and override logical parent/child navigation. Preserve this documented exception and test the composed tree instead of removing ownership to satisfy the generic fragment-root rule.

### CODE CHANGE

Extend the existing shared UIA client and controlled Playground fixture. Update the Windows semantic, pattern, text and event adapters and the owning controls where an event or public metadata path is missing. Confirm both ordinary and hosted behavior, existing suites, configured UnitTest and any required metadata generation before marking the proposal confirmed.

- R2-01/05/24: add explicit Windows UIA name, label and localized-text metadata; derive tooltip text from its content, publish owner HelpText, and distinguish interactive content. Keep numeric view IDs stable.
- R2-02/03/07/08: enumerate ribbon header controls, map calendar-day focus to logical nodes, reject conflicting AddToSelection atomically, and expose radio selection groups according to their mutex controller.
- R2-06/09/10/11: represent an unadorned popup as a Pane, expose ScrollItem on scroll-view descendants with two-axis scrolling, and provide Value for string-valued editable grid cells.
- R2-12/13/14/22: queue bounded idle probes completed after pending input, clamp inaccessible window movement, expose actual modal-session state, and publish bounds changes including hosted translations.
- R2-15/16/17/18: use renderer-character intervals for surrogate pairs, choose caret attributes from the adjacent character, count actual movement, preserve failed-move endpoints, and include trailing separators in Word units while retaining object boundaries.
- R2-19/20/21/23: retain semantic hyperlink/image identities with range/enclosure/point mapping and hyperlink actions; publish document replacement and direct paragraph-update events; pair and order menu-mode events around the first opened and last closed menu.

The initial six text regressions passed after the first text repair. The expanded fixtures also check control metadata, radio-group identity, rejected selection, two-axis scrolling, object traversal and invocation, direct-model events, bounded idle waits, window events, calendar focus, ribbon traversal, editable cells and menu ordering. The completed validation below covers both ordinary HWND and hosted configurations.

The expanded ordinary fixture passed its first 27 contract assertions, then a native failure occurred after hyperlink invocation. CDB stopped in `SetPropertiesVisitor::Visit(DocumentImageRun*)`: the Direct2D paragraph rejected reapplying an existing inline image whenever another inline object existed. Its uniqueness check ran against unrelated objects before recognizing the reused range. Move that check after range lookup and apply it only to new objects. The adjacent-image fixture exercises this renderer path during hyperlink formatting and subsequent selection changes.

The first completed ordinary Review2 run passed 86 assertions and normal shutdown. Showcase testing confirmed editable cell Value, horizontal scrolling and logical calendar focus. CDB traced the calendar's underlying FocusedChanged callback into the correct logical-day notification. The focus-event test registered after a rejected .NET selection operation had already focused its target; explicitly move focus away before registering. Wait for row layout after resizing a column before testing offscreen geometry.

Global desktop focus is a separate environment limitation: `query session` reports this process's session as disconnected, and CDB confirmed that the global lookup did not call the fragment provider's GetFocus. The client queries WTSConnectState and explicitly reports global focus as unverified in WTSDisconnected; reachable unique logical focus and focus events remain mandatory. Keep provider SetFocus limited to internal focus, as required by Microsoft's [SetFocus contract](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-irawelementproviderfragment-setfocus).

Final source review found that a named embedded control nested inside a hyperlink also needs reciprocal semantic parent navigation and enclosure precedence. Resolve its parent through the document-object children and prefer the embedded child when its exact range is enclosed by a hyperlink. Add a nested hyperlink/image/button fixture, and exercise menu event ordering through keyboard and pointer entry as well as ExpandCollapse.

### CONFIRMED

The provider changes satisfy the exercised review contracts in both configurations: semantic identity and navigation agree with the control model, rejected selection operations preserve state, scrolling and editable values operate on their owning controls, text ranges preserve Unicode/object boundaries, and observable transitions deliver the expected events. Retain this implementation and its regressions. R2-04 is qualified by Microsoft's documented popup-parent exception; global desktop focus remains an explicitly unverified environment-dependent check, with logical focus and focus-event identity verified.

- The complete Debug/x64 and final Debug/Win32 UnitTest runs each passed 90/90 files and 1769/1769 cases. Both completed Execute.log files ended at those summaries with no appended memory-leak dump. The Win32 run used the final C++ sources, including the nested-object correction. Entries named `ERROR: ... partial coverage` are successful negative test cases, not failures.
- Debug/Win32 and Debug/x64 builds completed with zero warnings and errors. The last incremental builds took 44.92 and 42.27 seconds respectively. The compiler produced an invalid COMDAT for a recursive local lambda in the new parent lookup; using a direct recursive helper removed the problematic closure and both builds passed.
- Metadata_Generate ran successfully for Win32, followed by the x64 build, Metadata_Generate x64 and Metadata_Test x64. Generated differences contain ExecuteHyperlink, BoundsChanged, Modal and BlockedByModalWindow.
- Snapshot differences were inspected: the theme fixture's unseeded calendar selects September 18 instead of September 11, the file dialog records this checkout's working directory, and hosted movement notifications add intermediate render frames. Captured final UI geometry remains unchanged outside the expected path/date differences.
- The showcase grid test must restore its horizontal scroll percentage as well as its resized column width; otherwise its later combo-editor test starts outside the viewport. Row layout also settles after the header layout, so scroll preconditions wait for the target row's actual visibility.
- Playground Review2 passed 131 assertions in each mode, including nested object parent/enclosure/point identity, modal and bounds changes, and bounded busy/idle waits. Playground Transitions passed 71 assertions in each mode, including four concurrent MTA readers. Each completed with normal Window.Close and endpoint release.
- The final hosted CppTest All run passed 3341 assertions (structure=516, property=367, text=35), including all three menu entry paths, grids/editors, text, calendars, all showcase tabs, window transforms, modal dialogs and palette replacement. It completed normal Window.Close and endpoint release.
- The final ordinary CppTest_Metaonly All run passed 3347 assertions (structure=515, property=367, text=35), covering the same showcase contracts and completing normal Window.Close and endpoint release. The native message-box test dismissed its owned button with BM_CLICK because physical SendInput was unavailable.
- All four reflection text round trips (`Reflection32`, `Reflection64`, `ReflectionCore32`, `ReflectionCore64`) exactly match their generated `[2]` counterparts by SHA-256.
- The Windows session reconnected during the ordinary sweep. A focused calendar rerun identified the desktop's focused element as the foreground Codex application, rather than any GacUI provider. The client now also checks the foreground process before the global assertion. It reports the desktop check as unverified when the test application lacks foreground ownership, while keeping unique reachable logical focus and event identity mandatory.
