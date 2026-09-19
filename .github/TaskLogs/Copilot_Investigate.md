# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

TODO_Task_UiaReview2.md, fill the revealed gap between UIA for GacUI and requirements from MSDN

# UPDATES

## UPDATE

the overall implementation is fine, but you added UIA related stuff to Playground in a way that is not good. Check out [Project.md](Project.md) and you will find that Playground is for trying UI from XMLs so all UI should be fully implemented in XML instead of C++ (if [Project.md](Project.md) does not say that please add it). Perform refactoring on Playground. It is fine that Playground only loads one from multiple XML candidates by changing the main function, this is the purpose of "trying UI from XML".

# TEST [CONFIRMED]

Reproduce the R2-01 through R2-24 contracts from `TODO_Task_UiaReview2.md` in the existing UIA client and Playground fixtures. Preserve the review as historical evidence. Cover ordinary HWND and hosted configurations, observable state and identity, rejected operations without mutation, Unicode text units and event delivery. Run the configured UnitTest through repository wrappers and inspect its completed log for failures and leaks. Validate reflection metadata in both architectures if public registrations change.

For the Playground refactoring, require all controls, layout, document content and interaction handlers to be authored in XML/Workflow resources. `GuiMain` selects an XML candidate and `OpenMainWindow` creates its reflected window. Compile/load that resource at runtime and rerun Review2 and Transitions in ordinary and hosted modes without weakening their contracts. Run the configured UnitTest and inspect its completed log for leaks; build both architectures and complete the metadata sequence if reflection registrations change.

# PROPOSALS

- No.1 Repair the provider contracts at their semantic and control state owners [CONFIRMED]
- No.2 Author the Playground UIA fixture in XML [CONFIRMED]

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

## No.2 Author the Playground UIA fixture in XML [CONFIRMED]

The existing `RunUiaReview` bypasses Playground's resource compiler and builds its entire UI in C++. `Project.md` describes XML candidate selection but does not explicitly forbid C++ UI construction. Clarify that rule and move the fixture to `Resources/ResourceUiaReview.xml`, alongside the existing candidates and in the same project resource filter. Keep the existing provider implementation.

### CODE CHANGE

Declare the fixture's controls, bounds, tooltips, groups, document objects, secondary window and popup in the XML resource, with interaction behavior in Workflow handlers. Use narrow reflected native hooks only for Windows accessibility metadata and the named-event busy test, plus the existing non-mutex controller behavior. Remove the C++ fixture builder and command-line fixture selection; select the resource in `GuiMain`. Share the normal resource loading and application/service lifecycle in ordinary and hosted modes. Update the UIA driver and knowledge-base instructions to use the selected XML fixture.

The first fully compiled XML exposed an existing reflection identity typo in `Source/Resources/GuiDocument.h`: `DocumentEmbeddedObjectRun` inherits `Description<DocumentImageRun>`. CDB stopped in `FieldInfoImpl::SetValue` while the generated constructor assigned its newly created `DocumentEmbeddedObjectRun` to the corresponding typed field. Correct the description to its own type; add a reflection construction/property-access regression in `TestReflectionTypeList.cpp` and keep the declarative embedded-object fixture as runtime coverage. Rebuild and rerun validation with this correction. Write Playground's generated resource outputs beside its project, matching the existing ignore rules independently of the launch working directory.

The corrected Win32 XML fixture passed all 131 Review2 assertions. Its CDB run then reported a freed heap block modified during shutdown, detected when the application disposed its automation service. Continue tracing the actual freed-object lifetime before confirming the proposal; passing UI assertions alone is insufficient.

Independent native cleanup checks also found retained UIA providers after the transition scenario. CDB observed `UiaDisconnectProvider` returning `UIA_E_ELEMENTNOTAVAILABLE` after retirement. Complete the application's shutdown using Microsoft's documented [UiaDisconnectAllProviders](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiadisconnectallproviders) after stopping the context, and release each destroyed HWND's raised-event map with [UiaReturnRawElementProvider(hwnd, 0, 0, nullptr)](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiareturnrawelementprovider). Verify the actual exit leak dump; retain ordinary per-element retirement during operation.

The native heap warning's offset is consistent with the adjacent HTTP listener's overlapped receive. Its upstream `VlppOS/Source/InterProcess/Windows/HttpServerApi.Windows.cpp` drains wait callbacks, then closes the request queue without explicitly waiting for the receive's cancellation to complete. Add a focused upstream lifecycle regression that requires the receive to be completed when `Stop` returns, repeating idle start/stop without sleeps. Use that evidence to decide whether cancellation/draining needs correction at the upstream owner; do not work around the lifetime by changing Playground's allocation layout.

The initial upstream diagnostic passed 1,000 consecutive start/stop cycles: each receive was pending before Stop and completed on return. That result alone did not establish an HTTP defect, so the temporary diagnostic was removed while native debugging continued. The first x64 CDB transition run after the UIA shutdown correction passed 71 assertions and reached exit without a leak dump; repeated native/hosted runs were still needed to check the intermittent heap warning.

The next x64 CDB run reproduced the heap corruption: the freed block contained `0xc0000120` (`STATUS_CANCELLED`) at the HTTP receive's `OVERLAPPED::Internal` offset, and zero at `InternalHigh`, surrounded by the native freed-memory fill. Closing the request queue can complete cancellation after freeing the listener. Fix the upstream owner by explicitly cancelling the receive and waiting with `GetOverlappedResult` before closing the request queue; skip the uninitialized receive when never started. Restore lifecycle coverage (including never-started and repeated Stop), run the upstream suite, regenerate VlppOS release/import through CodePack, and repeat downstream debugger runs. The earlier uninstrumented stress success did not disprove this timing-dependent defect.

### CONFIRMED

Select No.2 for the Playground fixture because it follows Playground's XML authoring purpose and exercises the normal resource compiler, loader and reflected window construction. It supersedes the C++ fixture portion of No.1; retain No.1's provider contracts and assertions. Controls, layout, document models, tooltips, popup/window configuration and interaction handlers now live in `ResourceUiaReview.xml`. `GuiMain` selects that candidate without command-line fixture switches. C++ retains application setup and narrow native adapters. `Project.md` now explicitly states this rule.

The XML fixture exposed the embedded-document reflection typo and native shutdown defects described above. Retain the reflection correction, HWND/provider cleanup and upstream HTTP receive drain, with their regression coverage. The VlppOS fix and its CodePack output are committed as `fa0a485`; GacUI's imported Windows implementation exactly matches that generated release file.

- Final Debug/Win32 and Debug/x64 builds passed with zero warnings and errors (43.80 and 27.70 seconds). Full GacUI UnitTest passed 90/90 files and 1770/1770 cases in both architectures. The Win32 run covered the XML/reflection correction; the final x64 run also included both shutdown fixes. Each completed UnitTest log ended at its passing summary without an appended leak dump.
- Metadata_Generate completed for Win32, followed by the x64 build, Metadata_Generate x64 and Metadata_Test x64. All four reflection text round trips exactly match their generated `[2]` counterparts by SHA-256; this refactoring adds no global metadata changes.
- Final x64 Review2 passed 131 unchanged assertions in ordinary mode and 131 in hosted mode. Both completed normal Window.Close and endpoint release after compiling/loading the XML resource.
- Transitions passed 71 unchanged assertions per run across 25 consecutive CDB lifecycle runs spanning Win32/x64 and ordinary/hosted modes. Every process exited normally, without freed-heap modification, an unhandled exception or a CRT memory-leak dump. This includes the concurrent MTA readers and retained-provider/range cases.
- The upstream VlppOS build passed with zero warnings and errors. Its full UnitTest passed 16/16 files and 307/307 cases without appended leaks, including 1,000 pending-receive start/stop cycles, never-started Stop and repeated Stop.
- Generated snapshot changes were reviewed: the calendar highlight advances from September 18 to September 19. The file-dialog case records one additional intermediate frame and different element allocation order. After resolving element references and excluding allocation/frame IDs, all six captured rendered trees have identical content, bounds and geometry to the previous snapshots.
