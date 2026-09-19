# GacUI Windows UI Automation review details

Companion review of [TODO_Task_UiaReview.md](TODO_Task_UiaReview.md), dated 2026-09-18. The original task remains unchanged. All corrections, interpretations and additional verification requirements from this review are recorded here.

Reviewed checkout: `0a0e7e565a7c10c4532589259c3a43bc514c8cd0`. The original task uses `e242162299e8efde6c3a5e3be400607704bbcc18`; comparison between those revisions found no changes under `Source` or in the four `Test/UIA_CppTest*` suite files. Source findings below therefore apply to the original baseline as well. Microsoft references were checked on 2026-09-18. This was a source and requirements review; no application, build or UIA test was executed, and no implementation claim is made.

The original task's T1-T8, X1-X5, P1-P2 and E1-L2 identifiers remain the cross-reference scheme. `Semantic`, `Provider`, `Actions`, `Patterns`, `Text`, `State` and `Windows` refer to its source map; their provider classes and helpers are in `vl::presentation::windows`. The [UIA implementation inventory](.github/KnowledgeBase/KB_GacUI_Design_UIAutomation.md), [GacUI index](.github/KnowledgeBase/Index_GacUI.md), [project instructions](Project.md) and [review workflow](.github/prompts/review.prompt.md) provide the repository context.

## DETAILS

### D1. Scope, evidence and completion status

Keep this task a review backlog. A source gap may be confirmed by inspection while its client-visible consequence still needs reproduction. Completing the later review means giving every original checkbox an explicit disposition and evidence, including recorded failures; it does not mean implementing every discrepancy or declaring full UIA conformance. Track implementation work separately, as the original task requests.

Use these dispositions: implementation verified; source discrepancy with client reproduction; source discrepancy awaiting reproduction; documented interpretation accepted; condition inapplicable with evidence; or verification blocked with the exact missing fixture/environment. A generic suite pass, missing fixture, successful COM return, or source mechanism alone cannot close a behavior check as verified. Record outcomes in this companion while preserving the original document.

L2 remains a separate project-design assessment. For composition customization, first record required override precedence, ownership, identity, parent/child reciprocity and preservation of grid/selection/header contracts. For remoting, record the missing semantic data and action path, lifetime on reconnect and correspondence with local controls. Neither item authorizes implementation of an API or transport during this review, nor does it block reviewing the existing local provider.

### D2. Calendar Table support is required (T6)

**Factual correction:** the original Calendar inventory calls Table conditional. Microsoft's Calendar contract requires both Grid and Table; Selection depends on selection behavior. Check both required patterns for every supported date-picker template. In `Semantic`, `UiaCalendar` searches for `GuiCommonDatePickerLook`, and `WindowsUIAutomationNode::Supports` gates the patterns on that lookup. A custom template that still exposes Calendar but cannot supply these patterns is a concrete capability gap. [Calendar required patterns](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcalendarcontroltype)

Keep the hierarchy and weekday-header geometry checks independent from pattern availability. Identify whether retained CalendarDay nodes represent visible grid positions or dates when the displayed month changes; require consistent Name, selection and coordinates for that stated identity, rather than silently assuming both identities can be preserved.

### D3. Popup ancestry must account for native fragment boundaries (T1-T2, L1)

The acceptance criterion is the client's merged UIA tree: dropdown content is discoverable from its logical owner without duplicate popup subtrees. Ordinary popup HWNDs currently provide separate fragment roots. Any later ownership change must reconcile `Semantic::IsRoot`/`Parent`, `Provider::get_HostRawElementProvider`, `get_FragmentRoot`, `GetRuntimeId`, the provider's captured `rootProvider` flag and `Windows`' `WM_GETOBJECT` handling. Adding only `Children` and `Parent` edges is insufficient.

Microsoft describes a fragment root as having no provider parent; its host joins it to the larger tree. Therefore apply direct-provider parent/child reciprocity inside a fragment and separately check client navigation across native host boundaries. A null provider parent on an HWND fragment root alone is not proof of broken client ancestry. [Provider fragments and hosts](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-providersoverview)

**Resolved review policy:** for genuine menus, prefer the Menu page's explicit inclusion in both control and content views over the flattened typical diagrams on the MenuItem/SplitButton pages. Record this interpretation when comparing trees. Continue requiring owner ancestry in control view and retain the context-menu exception. A combo implemented using `GuiMenu` does not automatically need Menu semantics for its dropdown; evaluate its logical List, Calendar or arbitrary content separately. [Menu contract](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportmenucontroltype)

T1's rejected-selection check is actionable: `Actions::UiaSelect` changes list selection before rejecting combo removal. The failure oracle must compare selection and application callbacks before and after the rejected call; an error HRESULT is insufficient if the call already mutated state.

### D4. Structural fixes must also preserve hit testing (T3, T5)

`Provider::UiaHitTest` descends only when the immediate child's bounds contain the point. A new page Pane below the existing header-bounded TabItem would still leave the page body unreachable. The same issue applies to a popup extending outside its opener after a semantic ownership change. Require root hit testing to find the visible page/popup target while preserving an appropriate clickable point for the tab header or opener. Merely inserting an intermediary node cannot satisfy this test.

For grid editors, `Semantic::UiaCollectChildren` begins below its supplied composition. An editor root with its own associated control can be skipped while descendants' `Parent` finds that control. T3's custom-root fixture must check both enumeration and reverse navigation by identity, including the editor root itself. Verify the default hierarchy separately from custom templates, and repeat when the editor is removed or replaced.

### D5. Grid selection and offscreen editing have distinct cases (T3-T4)

Define the selection unit separately for `GuiVirtualDataGrid` in [GuiDataGridControls.h](Source/Controls/ListControlPackage/GuiDataGridControls.h) and Detail `GuiVirtualListView` in [GuiListViewControls.h](Source/Controls/ListControlPackage/GuiListViewControls.h). `Semantic::IsSelected` uses the data grid's active cell for its cells, but detail-list cells inherit row selection. `Patterns::UiaSelectedChildren` returns selected rows and only recurses through tree nodes. A blanket change to return cells would conflate row multiselection and active-cell state. For each configuration, document which nodes represent selection and require container selection, item state, operations and focus to agree with that policy.

**Source clarification:** `GuiVirtualDataGrid::SelectCell(value, true)` in [GuiDataGridControls.cpp](Source/Controls/ListControlPackage/GuiDataGridControls.cpp) calls `EnsureItemVisible` when changing to a different valid cell. Its already-selected-cell branch attempts `StartEdit` without that call; `StartEdit` requires a realized default item template. Test newly selected offscreen cells separately from a selected cell scrolled out of view. Also, the Boolean result means an editor is open: `SelectCell(value, false)` can successfully select while returning false. Do not treat that return value as a general selection-success flag.

The default showcase saves cell edits immediately. Its [grid suite](Test/UIA_CppTest_Shared.cs) explicitly checks that Escape closes the editor without undoing saved text. T3's commit/cancel terminology must preserve each editor's actual policy; accessibility must not introduce rollback semantics the application does not have.

### D6. Group membership alone does not establish radio semantics (T8)

`GuiSelectableButton::GroupController` in [GuiButtonControls.h](Source/Controls/GuiButtonControls.h) is an extensible policy. Its `MutexGroupController` subclass supplies mutual exclusion in [GuiButtonControls.cpp](Source/Controls/GuiButtonControls.cpp). `Semantic::Role` currently maps every nonnull group controller to RadioButton. Add a non-mutex custom controller to role verification rather than assuming every group is mutually exclusive. For mutex groups, compare two groups sharing a parent and one group spanning parents; composition ancestry cannot by itself identify the actual selection group.

### D7. Password transitions can expose a cached event old value (X1, X5)

**Additional source gap:** `State::Control` takes an initial property snapshot; `State::UpdateProperties` watches Value and supplies the previous cached VARIANT to `UiaRaiseAutomationPropertyChangedEvent`. `Provider::Property` changes protected Value to `VT_EMPTY`, while `GuiSinglelineTextBox::SetPasswordChar` in [GuiDocumentViewer.cpp](Source/Controls/TextEditorPackage/GuiDocumentViewer.cpp) has no matching UIA notification hook. After a plaintext value has been cached, a later unrelated notification can submit that plaintext as the old Value after password mode is enabled. This is a source-level disclosure path; client receipt has not been reproduced.

Acceptance covers all new provider reads and event payloads after protection, including cached/queued provider data. It cannot require revoking a plaintext copy a client legitimately obtained before the transition. Keep this distinction explicit in X1's cache wording. Test only with synthetic sentinel text. [TextPattern security guidance](https://learn.microsoft.com/en-us/dotnet/framework/ui-automation/ui-automation-textpattern-overview#security)

### D8. Text selection needs degenerate-range and enablement oracles (X1-X2)

`Text::WindowsUIAutomationTextRange::AddToSelection` permits only an empty current selection or an identical range; `RemoveFromSelection` requires an identical range. With a nonempty selection, both reject a different degenerate range. The method contracts specify moving the insertion point for a degenerate range. Test this independently of whether a nonempty disjoint range can be represented by this single-selection control. [AddToSelection](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-addtoselection), [RemoveFromSelection](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-removefromselection)

Text-range Select checks control enablement but not modal-window enablement, while RemoveFromSelection lacks the control check too. Compare with `Provider::Read(..., enabled=true)`, which checks both. Require rejected operations to leave caret and selection unchanged. Keep edit mode separate from enablement: an editable but disabled field can still report that its value is not read-only while rejecting actions because it is disabled.

### D9. Qualify scrolling and strengthen visible-range checks (X3)

**Requirement qualification:** the cited Win32 text-range ScrollIntoView contract specifies vertical scrolling and top/bottom alignment. Its citation alone does not prove that vertical-only scrolling violates Microsoft requirements. Preserve horizontal visibility as an explicit GacUI acceptance requirement. `GuiDocumentLabel::EnsureRectVisible` in [GuiDocumentViewer.cpp](Source/Controls/TextEditorPackage/GuiDocumentViewer.cpp) already supports horizontal viewport movement, so single-line overflow is a meaningful case. Scrolling should not change selection just to reveal a range. [ScrollIntoView](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-scrollintoview)

**Additional source gap:** `Text::GetVisibleRanges` creates an empty SAFEARRAY when there are no visible caret rectangles, including an empty document. Although the API's general output description mentions an empty array, its specific remarks prescribe a degenerate text range for empty/no-visible-text/all-scrolled-out cases. Use those remarks as the explicit test oracle. [GetVisibleRanges](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextprovider-getvisibleranges)

Keep geometry risks as reproduction targets: visible-range detection uses leading caret rectangles, which can miss partly visible glyphs; rectangle merging in `Text::GetBoundingRectangles` does not decrease the previous left coordinate when a later rectangle extends left. Use clipped glyphs and mixed RTL/LTR runs to determine the actual rendered consequence.

### D10. Embedded objects need a text-store distinction (X2, X4)

**Requirement qualification:** a one-marker `RangeFromChild` is not inherently wrong. Microsoft permits U+FFFC for an embedded object with an independent, non-compatible text store. Require its text through its own provider rather than flattening every embedded textbox into the outer document. Same-stream hyperlinks need separate evaluation. [Embedded objects and text stores](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-textpattern-and-embedded-objects-overview)

The relationship gap remains: `Text::GetEnclosingElement` always returns the outer document, and `GetChildren` can return the same top-level embedded child for a range obtained from that child. Check the RangeFromChild enclosure relationship and that subsequent range-child traversal proceeds into the appropriate subtree without repeatedly rediscovering the same object. [RangeFromChild](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextprovider-rangefromchild)

`Text::Boundaries(Format)` compares style attributes without explicit object boundaries. Add equally styled embedded objects/hyperlinks so incidental font or color changes cannot conceal missing format breaks. Conversely, unsupported Page movement falling back to Document is allowed and is not by itself a defect. [Text-unit movement](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-move)

### D11. Selection events depend on the resulting selection (E2)

**Requirement qualification:** absence of an explicit removal event for the previous selected item is not independently a defect when the operation leaves one selected item. Microsoft's rule uses resulting cardinality: one remaining item calls for ElementSelected; otherwise use Added/Removed as appropriate. `Actions::UiaSelect` currently chooses its event from the requested `selected` Boolean, so Add from one to two and Remove from two to one need distinct checks. In the latter case, check the event source against the surviving selected item. [SelectionItem events](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingselectionitem)

Also distinguish Invoke timing: pre-action Invoked is permitted for actions that block or require user interaction, while ordinary nonblocking actions should report after invocation. Do not impose one universal before/after rule on both a simple command and a modal command. Queued success still requires observing completion. [Invoke conventions](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinginvoke)

### D12. Focus is independent of selection (P2, E1)

The focus issue is broader than multiselection. `Provider::Property` can mark both the owning focused control and selected synthetic descendants as focused. For a Detail list, `Semantic::IsSelected` can mark a selected row and all its cells selected. `Provider::UiaFindFocus` returns the first depth-first match, while `State::Control` raises focus events on the owning control and selection callbacks mainly invalidate selection.

Require a coherent focused semantic target across HasKeyboardFocus, fragment GetFocus and focus-event source. Establish the target from actual keyboard/caret behavior, not the first selected row. Include keyboard movement within an already-focused list/grid, multiselection whose active endpoint is not the first selected item, and entry into/exit from a cell editor.

### D13. Metadata and provider fallback need separate evidence (P1-P2)

The original AutomationId concern is valid, but tests must distinguish sibling uniqueness, stability across instances of the same application version and RuntimeId lifetime identity. Do not require AutomationId to be globally unique or unchanged across application versions. Treat RuntimeId as opaque; do not use the current counter format as a test contract. For bounds, distinguish enclosing geometry, partial clipping and IsOffscreen; other-window occlusion does not alone make an element offscreen. [Element property definitions](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-automation-element-propids)

`VT_EMPTY` means that this provider does not supply a property, allowing fallback. Explicit suppression instead uses the reserved not-supported value. A missing switch case therefore needs separate raw-provider and client observations, especially at HWND roots. Compare current and cached client properties with direct pattern getters; record VARIANT types/default suppression so empty strings, defaults and unsupported values are distinguishable. [Provider GetPropertyValue contract](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-irawelementprovidersimple-getpropertyvalue)

For labels, use an explicit application-supplied label and relationship as the fixture oracle; do not infer a label from an arbitrary nearby control or the input value. ProgressBar step tests should check quiet NaN with an NaN predicate, not equality, where that documented value is adopted. [ProgressBar pattern properties](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportprogressbarcontroltype)

### D14. Virtualization needs cost and retained-state measurements (L1)

`Semantic::Children` enumerates every list row; `State::Item` linearly scans the context's node collection for each lookup. `Patterns::FindItemByProperty` materializes the item collection before finding the requested match. First-child navigation can therefore enumerate the whole model, and repeated lookup can have quadratic cost in the number of semantic nodes. This is separate from visual-template virtualization.

`State::Retire` disconnects providers but leaves retired nodes in the context collection until context shutdown. Repeated model replacement can grow retained state and lookup work even after external clients release providers. Measure cold/warm access and replacement cycles separately, with semantic-node counts, realized-template counts, memory and latency. This is a source-derived performance risk, not a measured leak or a claim about an acceptable maximum model size.

### D15. Event bursts, dispatcher waits and retirement need explicit limits (E1, P2, L1)

`State::Notify` keeps pending event IDs in a sorted set. Repeated events and original order are lost, and property comparisons see final state. Add open-close-open in one dispatcher turn, selection A-B-A and transient focus to distinguish permissible final-state coalescing from lost lifecycle meaning. If retirement occurs before dispatch, the live-node check drops queued events entirely. Capture terminal event identity independently of access to the disposed control.

`Patterns::WaitForInputIdle` ignores the timeout after validating it. `Windows::WindowsUIAutomationDispatcher::Run` uses synchronous `SendMessage`, so even a zero-timeout request can wait for a busy UI thread before reporting true. Compare zero, short and sufficient timeouts against controlled UI work with an independently enforced client deadline; reaching the dispatcher is not an observation of an idle condition. [WaitForInputIdle](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-iwindowprovider-waitforinputidle)

Retirement checks apply to semantic reads/actions, not every COM method. Reference counting, QueryInterface and immutable GetRuntimeId intentionally remain usable for disconnect/release. Verify release after context shutdown, Detail-other-Detail transitions, cross-HWND moves and queued work during shutdown without requiring those identity/lifetime operations to fail.

## VERIFICATION

### V1. Establish the environment and baseline

These steps are for the later investigation. They were not executed during this document review.

1. Record commit, Windows version, renderer, ordinary/hosted mode, application/template, client bitness, DPI/monitor placement and desktop/session state. Verify actual application setup; the two existing suites cover different mode/reflection combinations and do not prove every renderer/reflection combination.
2. Use [Running-GacUI.md](.github/Guidelines/Running-GacUI.md) to establish the same operation through the automation service and then UIA. Use a windowless MTA client and identify the owned HWND/PID. Give concurrently running apps distinct automation ports. A locked session may support queries but fail native input; record the specific limitation.
3. Run the existing suites sequentially with absolute script paths: `& C:\Code\VczhLibraries\GacUI\Test\UIA_CppTest.ps1 -AsPort 8888` and `& C:\Code\VczhLibraries\GacUI\Test\UIA_CppTest_Metaonly.ps1 -AsPort 8890`. Their shared wrapper builds and launches through the repository scripts. Preserve each run's output before the next overwrites shared logs.
4. Use focused scenarios through [UIA_CppTest_Shared.ps1](Test/UIA_CppTest_Shared.ps1) for reproduction; extend that existing suite or use a small targeted fixture for contracts it cannot observe. Inspector synthetic tests establish inspector behavior only.
5. Follow [Building.md](.github/Guidelines/Building.md), [Running-UnitTest.md](.github/Guidelines/Running-UnitTest.md) and [Project.md](Project.md) if a later implementation changes C++ or generated/reflected resources. No build or unit test is required for this companion-only change.

### V2. Record identities and events, not just counts

The shared C# suite currently counts broad structure/property/text events. Its popup helpers search other windows, its label assertion only excludes the input value, and its tab walk treats AutomationId as a broad uniqueness key. Its Concurrent scenario checks independent application targets; it does not itself establish simultaneous-client or reentrant-callback behavior against one provider. These are useful baseline checks but cannot establish the missing ancestry, labeling, focus or precise event contracts.

For each focused operation, retain:

- Original checkbox ID and expected result, including the Microsoft reference or explicit project requirement.
- Before/after raw, control and content paths; RuntimeIds compared as opaque identities; HWND and fragment boundaries where relevant.
- Pattern availability, direct getter values, current/cached property values, exact HRESULT/VARIANT or managed exception, and application/rendered state.
- An operation-local event log containing type/property ID, source RuntimeId, old/new values and observation order. Subscribe before the action and capture identity while the target is live so close events do not require querying a destroyed object.
- Whether the event came from GacUI, the native host or UIA core where that can be established. Provider emission and client receipt are different observations.

Use bounded waits tied to expected state or event completion. Keep callbacks small and perform normal state queries on the client worker; exercise reentrant queries separately as a deliberate stress case. Test one mutation at a time before testing bursts, since the provider coalesces notifications. Assert only ordering the relevant contract requires. Do not demand descendant IsEnabled events solely because an ancestor became disabled. [Enabled-property event qualification](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-automation-element-propids)

### V3. Focused cases and pass criteria

| Original scope | Concrete fixture/action | Required observation |
| --- | --- | --- |
| T1-T2, L1 | Open/close/reopen a list combo, date combo, nested submenu, genuine context menu and split-button menu in ordinary and hosted modes; include a combo inside a grid editor and a popup beyond its opener bounds. | Expected owner ancestry in the merged tree; no duplicate popup; reciprocal navigation within fragments; correct fragment/host identity; hit testing reaches popup content. Invoke and expansion are independent where both are required. |
| T1, E2 | Retain a selected combo item; call RemoveFromSelection; try invalid single-selection AddToSelection; repeat with popup closed and open. | Rejected operations preserve the full selection, application callbacks and expansion state; successful operations produce consistent Selection/GetSelection/SelectionContainer results. |
| T2, P1 | Show a nonfocusable tooltip, then a focusable/dismissible variant; inspect the owner as well as the popup. | HelpText/content inclusion, owner relationship, available Window/Transform capabilities and open/close events match the actual behavior. |
| T3-T4 | Default/custom visualizer and editor roots; text, checkbox, combo and date editors; immediate-save Escape; sort/filter/remove/rebuild while references are retained. | One active editing subtree, correct parent for every exposed child, cell identity retained only for the same logical cell, correct grid/header metadata, and retired editor/model targets unavailable. |
| T4 | Invoke a different offscreen cell; then select a cell, scroll that row away without changing selection, invoke the same cell, realize it and retry. | Record editor creation and selection for each path, rather than interpreting queued S_OK or the SelectCell Boolean as proof of editing success. |
| T4, P2 | Ordinary Detail list with row multiselection; data grid with active cell and with MultiSelect enabled; move the keyboard selection endpoint. | Document selection unit per control, then compare the exact selected identity set with item states, SelectionContainer and focus; preserve application behavior. |
| T5, P2 | Hit-test a button in a selected page body and a nested tab; select clipped headers; remove the selected and final page. | Correct descendant hit target, header activation point, selected page and focus. Empty-tab behavior is recorded explicitly rather than expecting a nonexistent selected child. |
| P2 | Hit-test overlapping hosted windows, root background, clipped/decorative content and points outside the fragment. | Compare with actual input targeting. Root background can return the root; a point outside the fragment must not succeed merely through unconditional root fallback. |
| T6 | Default and alternate Calendar templates; month boundary and spillover; locale change; retained day/header references. | Both Grid and Table exist; weekday/header geometry is visible and accurate; coordinates, names, selection and declared date/slot identity remain consistent. |
| T7-T8 | Rich generic ListItem controls, custom tree content, collapsed and offscreen nodes; two mutex groups under one parent, one group across parents and a custom non-mutex controller. | Correct content filtering and roles, discoverable applicable actions, group identity independent of layout, no provider retargeting after model replacement. |
| X1, X5 | Synthetic plaintext sentinel cached before password mode; retain Value/Text/ranges, enable protection, then trigger another notification. Repeat with queued text changes and change text again before draining the queue. | No new provider read or event argument discloses the sentinel after protection. Inspect oldValue as well as newValue, raw VARIANTs and client events. Pre-transition client copies are outside the revocation claim. |
| X1-X2, X5 | Cross ViewOnly/Selectable/Editable with enabled/disabled/modal-blocked states; transition both directions. Select [1,4), then independently Add/Remove a [6,6) range in an enabled editable control. | Pattern/read-only/selection state matches edit mode; action enablement is separate. Degenerate operations move the caret to 6 with no selected text. Rejected operations do not mutate state. Transition events do not depend on an unrelated later action. |
| X2-X3 | Empty document; no visible text; partially clipped glyph; wrapped/mixed RTL-LTR/supplementary text; retained ranges over disjoint edits and model replacement. | Degenerate visible range in the specified empty cases; valid offsets and endpoint normalization; rectangle extents agree with rendering; stale-model ranges fail instead of binding to replacement text. |
| X3 | Top/bottom scrolling for ranges smaller/larger than the viewport, boundary-clamped ranges and horizontally clipped single-line content. | Document vertical contract and horizontal project requirement separately; achieved visibility/alignment is measured and selection remains unchanged. |
| X2, X4 | Independent embedded textbox, image/button and same-stream hyperlink, including nested and equally styled objects. | Parent marker policy is explicit; independent textbox text is available through its own Text provider; RangeFromChild enclosure/children are coherent; Format movement respects object boundaries. |
| P1-P2 | Explicit labels/image-only commands, cold starts with varied enumeration order, partial clipping, window occlusion and multiple DPI scales; compare raw/current/cached/pattern properties. | Meaningful label independent of value, justified optional properties, appropriate identifier stability, distinguishable defaults/unsupported values and geometry matching the relevant property contract. |
| E1-E2 | Repeat selection, toggle and invocation through UIA, real mouse/keyboard/ALT/shortcut and application mutation. Include Add 0-to-1/1-to-2, Remove 2-to-1/1-to-0, replacement Select and no-op operations; separately test D15's transition bursts. | Correct event identity/type and final state; Added for expansion of a multiple selection, Selected on the sole survivor, Removed when no item remains. Ordinary, modal and self-destroying Invoke cases have separate timing/identity observations. |
| L1, P2 | Retain providers/ranges across detach/reattach, cross-window moves, hide/show, destruction, model replacement and application shutdown; queue Invoke/Close while another MTA client reads. | No deadlock, crash or retargeting. Distinguish a still-live hidden/collapsed control from a destroyed/retired target; final close identity remains usable without dereferencing disposed controls. Check GetFocus and zero-timeout WaitForInputIdle behavior separately. |
| L1 | Compare fresh/warm FirstChild, sibling traversal and first/last/missing ItemContainer matches at increasing model sizes (for example 100, 1,000 and 10,000 rows), then repeated replacement with external references released. | Record latency, memory, context node count and realized-template count. Explain scaling and retained-state growth; do not call semantic virtualization efficient solely because visual templates remain bounded. |

### V4. Completion and follow-up

Cover the original inventory's conditional properties and events even when the table above does not repeat them. For each unmet requirement, attach the smallest reproducer and distinguish a provider defect, application/template limitation, host behavior, documentation interpretation or blocked observation. Keep native input verification distinct from HTTP-injected input, which can bypass the boundary under test.

Investigate a runtime crash or timeout immediately using [Running-ComputerUse.md](.github/Guidelines/Running-ComputerUse.md) and [Debugging.md](.github/Guidelines/Debugging.md); do not let a blocking native runtime dialog masquerade as an ordinary slow UIA call. Repeat unstable lifetime/concurrency reproducers according to the repository's 25-successful-runs rule once a fix is being verified. This is a later verification requirement, not evidence collected by this review.

## REVIEW COMMENTS

No unresolved review comments. The corrections and missing details above have reasonable review policies and concrete follow-up checks. Client behavior that still needs measurement remains verification work, and L2 API/transport design remains a separate follow-up; neither requires a blocking question before this review can be recorded.

## Implementation investigation (2026-09-18)

The subsequent `investigate repro` request authorizes implementation. The original task file remains unchanged; the review above is historical source analysis, while this section records later execution. Reproduction is committed in `191e67589`, and the implementation proposal in `c0cba1e5b`. The investigation journal is `.github/TaskLogs/Copilot_Investigate.md`.

The new `Review` scenario initially failed twelve independent checks against the unchanged hosted provider: empty visible ranges, degenerate Add/Remove selection, selected tab body hit testing, embedded range enclosure and children, combo descendants, active grid-cell selection, unique grid focus, Header/Tab orientation and weekday-header geometry. Rejected combo removal preserved its public selection in this fixture, so that particular assertion was not a reproduced client failure. Source inspection nevertheless found and removed a mutation preceding its error return.

### Implemented local behavior

- Known menu/combo/gallery popups have reciprocal owner navigation in both modes. Ordinary popups retain their HWND host provider, following [Microsoft's popup reparenting procedure](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-serversideprovider). Hosted hit testing selects the actual hosted window before walking its semantic subtree.
- A selected TabItem exposes a body Pane independently of its header geometry. Data grids enumerate their active cell as selection; ordinary detail lists retain row selection. Grid focus identifies the active cell instead of duplicating focus across the container, row and cells. Synthetic model nodes without an independent keyboard target are not advertised as focusable; the active grid cell shares its container's keyboard target. SetFocus checks that capability and never selects an item. Default Header/Tab/MenuBar orientation and calendar weekday geometry are available.
- Empty or wholly invisible text returns a degenerate visible range. Degenerate Add/Remove moves the insertion point; selection mutations check both control and native-window enablement. RangeFromChild retains its requested embedded identity, including containers with several controls sharing one marker. Independent embedded text stores remain separate. Rectangle merging preserves both left and right extents, and text scrolling uses the existing document visibility machinery without moving selection.
- Document controls expose Value with edit-mode-dependent read-only status. `EditModeChanged` and `PasswordCharChanged` are explicit reflected events. Password transitions discard cached plaintext Value before queued notifications; retained Text ranges reject access while masking is active.
- Selection events compare the previous selection with the resulting selection, including application mutations. Cardinality changes use Added, Selected or Removed on the appropriate item. Button Invoked follows the common BeforeClicked path, including keyboard activation. Invoked remains before potentially modal/destructive command handlers, as permitted by the documented contract.
- Property reads and change snapshots now include bounds, password/focusability/access keys, grid dimensions, selection capabilities and window state. Rectangle arrays and NaN step values use value-aware comparison instead of emitting changes for equivalent snapshots.
- ProgressBar step values are NaN. Only the concrete mutex button controller implies RadioButton/SelectionItem; other groups retain CheckBox/Toggle semantics. Non-dismissable popups no longer inherit Window/Transform merely by subclassing GuiWindow. Invoking an already-selected offscreen grid cell realizes its row before opening the editor.

The grid rejection regression exposed an additional focus interaction: CDB showed UIA calling SetFocus before AddToSelection, and the former implementation selected the cell during that preliminary call. The Add rejection guard consequently saw the requested cell already selected. Separating focus from selection closes that mutation path. Provider-owned focus alone did not solve it and was not retained. This is distinct from the native-window focus behavior described by [Microsoft's SetFocus contract](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-irawelementproviderfragment-setfocus).

### Reproducers and execution record

`Test/UIA_CppTest_Shared.ps1 -Application CppTest -AsPort 8888 -Scenario Review` and the corresponding `CppTest_Metaonly` command with port 8890 exercise the original failures. `All` includes these contracts and the existing showcase scenarios. Popup discovery now starts beneath the combo rather than accepting unrelated top-level-window descendants.

The existing Playground has an opt-in `/UiaReview` fixture and `/UiaHosted` renderer option. Run it through `Test/UIA_CppTest_Shared.ps1 -Application Playground -AsPort 8892 -Scenario Transitions`, adding `-HostedFixture` for hosted mode. It checks password event payloads and retained ranges, read-only and disabled rejection, horizontal scrolling without caret changes, ProgressBar values, non-mutex groups, exact selection event sources/cardinalities, keyboard Invoked, and concurrent MTA readers of the same providers/ranges. Ordinary Playground behavior is preserved when these arguments are absent.

Final-source verification used the repository build/execute wrappers:

| Verification | Result |
| --- | --- |
| Debug x64 and Win32 solution builds | Both passed, zero warnings/errors. |
| Metadata_Generate Win32, Metadata_Generate x64, Metadata_Test x64 | Generated and validated the reflected events/API for both architectures. |
| UnitTest Debug x64 | 90/90 files, 1,769/1,769 cases; no appended leak report. |
| Ordinary `Review` | 72 assertions passed, including atomic rejection of a different-cell AddToSelection. |
| Hosted `All`, corrected nested-tab walker | 3,250 assertions passed; normal shutdown and endpoint release. |
| Ordinary `All`, before the client-only walker correction | 1,265 assertions passed for the named scenarios; supplemented by the corrected `Walk` run below. |
| Ordinary corrected `Walk` | 2,339 assertions passed; normal shutdown and endpoint release. |
| Ordinary and hosted Playground `Transitions` | 71 assertions passed in each mode; normal shutdown and endpoint release. |

The new TabItem body Pane required updating `WalkTabs` to enter that Pane before finding nested tabs. The earlier 65-assertion Review and 1,260-assertion hosted runs are intermediate evidence, not the final coverage result. The ordinary message-box proxy required the existing owned-button BM_CLICK fallback because native SendInput was unavailable. Fixture keyboard commands use HTTP-injected input; these runs do not claim verification of physical keyboard delivery.

No unit-test snapshot changes are included: the file-path/current-day differences are environment dependent, and all 11 other changed file-dialog frames were identical after resolving element references and removing allocation IDs/frame numbers. The original task file's SHA-256 remains `58E00028A48EC942441CD3820C36479CD3BC0E925E6A89D841D00CB1788DEABE`.

### L2 extension and transport design boundary

These are explicit design decisions for the separate cross-platform extension, not claims that the following API/transport is implemented:

1. Attach an optional semantic source to a `GuiGraphicsComposition`. Give it an explicit augment/replace choice, stable application-supplied identity, role/capabilities/properties, child enumeration and action dispatch. Walk the composition root as well as its descendants. Replacement suppresses the corresponding default subtree; augmentation preserves it. Reject duplicate/cyclic identities instead of silently merging unrelated controls.
2. Keep owner-provided grid coordinates, selection container, header relationships and model identity in the semantic node contract. An item-template replacement can replace presentation and add actions, but must explicitly delegate or implement those obligations. The composition's disposal/detachment and its owning model generation bound node lifetime. Do not use a recycled item-template pointer as persistent model identity.
3. Factor the current Windows-only semantic queries/actions into a platform-neutral core model; retain COM, HWND hosting, DPI conversion and UIA event conversion in the Windows adapter. Core nodes own logical identifiers and revisions. A renderer must not reconstruct names, selection or commands from drawing elements or guess them from HTTP inspection output.
4. Extend the authored `Source/PlatformProviders/Remote/Protocol/*.txt` schema with a negotiated semantic version/capability, a full semantic snapshot, revisioned deltas and action request/completion records. Reuse the existing typed JSON channel and connection epochs. Core-to-renderer messages carry snapshots/deltas/completions; renderer-to-core events carry uniquely correlated action requests. The current request/response direction is core-to-renderer, so renderer actions need their own explicit correlation rather than reversing `GuiRemoteMessages::Submit`.
5. Commit each semantic delta atomically before emitting local UIA notifications. On reconnect, replace the renderer cache from a complete snapshot and retire providers from the old connection epoch. Validate node generation, capability and enablement again in the core before executing actions. Never send password contents, cached old plaintext, or live object pointers. Synchronous COM getters read a coherent renderer cache; action completion carries success/failure and resulting revision, without blocking the renderer UI thread on a core round trip.
6. Validate native hosted and remote adapters against the same semantic contract fixtures, then test reconnect/replacement, out-of-order/stale requests, disabled/modal controls, multiclient requests and renderer shutdown. Generate protocol code with `Metadata_UpdateProtocol`, update schema serialization and filtering rules, and add remote snapshot/action tests. Keep commands and lifetime notifications out of lossy repeat-drop filtering.

This follows the repository's [remote core architecture](.github/KnowledgeBase/KB_GacUI_Design_RemoteProtocolCoreArchitecture.md) and [renderer/channel architecture](.github/KnowledgeBase/KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md). The current patch is a local Windows provider repair; it does not provide that cross-platform model or semantic transport.

### Remaining review coverage

The original checkboxes are intentionally unchanged. The local fixes do not establish full UIA conformance. Still requiring focused implementation or evidence are semantic label/HelpText/AutomationId customization; complete split-button substructure and focusable tooltip policies; custom calendar/grid/tree/list templates and content filtering; same-stream hyperlinks and richer text-unit boundaries; exact per-transition event delivery under bursts, immediate destruction and all input sources; idle timeout semantics; multiselection keyboard focus endpoints outside data grids; DPI/RTL/occlusion coverage; and scale/memory benchmarks for node caching/retirement. The L2 implementation is separately scoped above. The four-reader test covers concurrent reads, not the 25-run shutdown-race acceptance criterion.
