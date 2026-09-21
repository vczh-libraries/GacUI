# Windows UI Automation

This page describes the Windows provider in `vl::presentation::windows`. It is an implementation inventory, not certification of every Microsoft UIA contract. Current gaps, historical review dispositions and verification requirements are consolidated in [the UI Automation gap report](../../../GacUI/ToDo/UiaGap.md); investigation evidence is in [Copilot_Investigate.md](https://github.com/vczh-libraries/GacUI/blob/a6e3f691424995b554efd6e2513b35e0e7be140c/.github/TaskLogs/Copilot_Investigate.md). See [Running-GacUI.md](../Guidelines/Running-GacUI.md#windows-specific) for execution guidance.

## Implementation and lifecycle

Provider sources live in [Source/PlatformProviders/Windows/UIAutomation](../../../GacUI/Source/PlatformProviders/Windows/UIAutomation):

| File stem | Responsibility |
| --- | --- |
| WindowsUIAutomation | Listener, native-window discovery, WM_GETOBJECT, UI-thread dispatch, accessibility metadata, bounded idle probes. |
| WindowsUIAutomationProvider | COM interfaces, properties, fragment navigation, identity, hit testing and focus. |
| WindowsUIAutomationSemantic | Roles, pattern availability, logical nodes, names, bounds and tree relationships. |
| WindowsUIAutomationActions | Invoke, Toggle, SelectionItem, ExpandCollapse, Value, RangeValue, Scroll and realization. |
| WindowsUIAutomationPatterns | Selection, Grid/Table, ItemContainer, MultipleView, Window and Transform. |
| WindowsUIAutomationText | Document snapshots, text ranges and embedded semantic objects. |
| WindowsUIAutomationState | Control/model hooks, property snapshots, events and retirement. |

The Windows Direct2D and GDI initialization paths install the listener. `WM_GETOBJECT` with `UiaRootObjectId` returns `UiaReturnRawElementProvider`. The listener needs a live `GuiApplication` and main window to bind semantic roots. Remote and TUI renderers do not have an equivalent semantic UIA transport, and the HTTP automation service is independent.

Window discovery records HWNDs without constructing semantic providers, scanning compositions or taking property snapshots. Accessibility metadata alone does not start observation. A UIA request or a subscribed event materializes only the COM interfaces needed to answer or deliver it. Radio groups and combo/popup owners are resolved from the live control tree independently of discovery order.

COM references are atomic. Each semantic node has a non-owning provider pointer protected by the same lock used by final `Release`; factory callers receive an acquired COM reference. Final release clears that pointer before destruction and posts observation cleanup to the UI thread. A newly acquired provider cannot resurrect the retiring object. Patterns share the provider's reference count; text ranges retain their required originating and embedded providers. Infrastructure and other clients may still own references after one client releases, so client disappearance is not an ownership signal.

Live control/model access is marshalled to the UI thread through the dispatcher. `Read` checks liveness, requested pattern and (for mutations) enabled state. Invoke and Close queue work and revalidate before execution. Clients observe completion separately. Represented controls retain detachable handler tokens and model callbacks through `WindowsUIAutomationLifetime`, including observation of their window's removal boundary. Native fragment roots implement `IRawElementProviderAdviseEvents`: matching additions/removals are counted per provider and across the context. Active subscriptions scan descendant event sources, allowing events from elements a client has never traversed. Only requested properties, focus and selection state are snapshotted; LabeledBy snapshots store semantic identity rather than COM references. When subscriptions end, bounds hooks and snapshots are cleared, while observation required by outstanding providers or ranges remains. Unrepresented nodes, hooks and internal lifetime properties are collected on the UI thread.

`ChildCompositionUpdated` insertion observes the inserted subtree synchronously when subscriptions are active, so immediate focus or other changes cannot precede hook installation. Delivery remains queued. Removal retires the detached subtree synchronously while its compositions are valid. Retirement detaches owned observation and invalidates retained providers and ranges before queuing `UiaDisconnectProvider`; disconnect never runs inside a SendMessage response. Reinsertion and cross-window moves create new attachments and identities. Old references remain unavailable. Model nodes and document runs are retained for identity, with validity checked against the current model. Dispatcher posting is synchronized with shutdown so final release on another thread cannot queue cleanup against a destroyed dispatch window.

Native-window destruction releases the HWND's raised-event map through `UiaReturnRawElementProvider(hwnd, 0, 0, nullptr)`. Application shutdown stops the context and dispatcher, then calls `UiaDisconnectAllProviders` to release remaining UIA references before the renderer exits. Verify native shutdown with the debugger's heap and leak diagnostics in addition to client assertions.

The COM interface set includes Simple, Fragment, FragmentRoot, root AdviseEvents and 19 pattern interfaces. QueryInterface alone does not establish current pattern availability: use GetPatternProvider or the pattern-availability properties. No Text2/TextEdit/TextChild, Selection2, Transform2, LegacyIAccessible or custom navigation interface is currently advertised.

## Names, labels and localization

Call the Windows C++ helpers in [WindowsUIAutomation.Windows.h](../../../GacUI/Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomation.Windows.h) on the UI thread:

- `SetWindowsUIAutomationName(control, name)` supplies an explicit accessible name, independently of editable contents.
- `SetWindowsUIAutomationId(control, id)` supplies a stable application identifier. Omitted identifiers are empty; RuntimeId remains the identity for an element during a session.
- `SetWindowsUIAutomationLabel(control, label)` supplies LabeledBy and a name derived from the label's text; passing null clears the relationship. Explicit Name takes precedence. Disposed labels are not dereferenced.
- `SetWindowsUIAutomationText(control, key, text)` supplies localized presentation strings. Lookup walks control ancestors. Keys are `HelpText`, `BigIcon`, `SmallIcon`, `List`, `Tile`, `Information`, `Detail`, `ColumnHeaders`, `Ascending`, and `Descending`.

Ordinary controls otherwise use GetText; editors/documents and combos have no inferred name from their mutable contents. Applications must provide their label/name. MultipleView numeric IDs remain the `ListViewView` values 0 through 5 regardless of displayed locale. English fallbacks are readable names, not enum spellings.

Tooltip names come from temporary content, recursively collecting control text. The owner's HelpText uses its tooltip contents unless explicitly supplied. Tooltips with focusable content participate in content view; ordinary label tooltips do not. Both remain in control view and belong beneath the tooltip owner.

FrameworkId is `GacUI`; ClassName is `GacUI.` plus role ID. AutomationId uses explicit application metadata and is otherwise empty. AccessKey uses Alt, and toolstrip AcceleratorKey uses the command shortcut. Bounds are physical screen coordinates clipped through composition ancestors; invisible/unrealized nodes have empty bounds. BoundingRectangle is also an element property. NativeWindowHandle is supplied only for native fragment roots. Password, enabled, offscreen, focus, orientation and pattern-state properties are provided where applicable. Unsupported properties return VT_EMPTY.

## Semantic tree and patterns

| Control/node | Role and behavior |
| --- | --- |
| GuiWindow | Window; Window and Transform. Uses actual modal-session state. Move and Resize constrain placement to the intersected monitor work area or hosted client container. A rectangle outside every monitor uses the primary monitor. |
| Plain GuiPopup | Pane. It has neither Window nor Transform. |
| GuiMenu / toolstrip menu | Menu (Group for ribbon group menus); owned by its opener. Popup wrappers do not expose Window/Transform. |
| GuiTooltip | ToolTip with owner/content metadata described above. |
| Ordinary button | Button / Invoke; selectable checkbox uses CheckBox / Toggle. Menu-button patterns depend on command, submenu and selection behavior. |
| Mutex selectable button | RadioButton / SelectionItem. A logical RadioGroup exposes Selection and contains the actual mutex-controller peers. Different groups stay distinct; peers under different layout controls share one group under their common container. |
| Tab | Tab / Selection. Contains TabItems; selected TabItem contains a TabContent Pane with its page controls. Ribbon before/after-header controls are also enumerated. |
| Calendar | Selection, Grid, Table and ItemContainer. Six week rows, seven columns, logical CalendarDays and weekday headers. Logical day focus, SetFocus and focus events map to the underlying day button. |
| List / Tree | Model-backed Items/TreeNodes expose SelectionItem, ScrollItem and VirtualizedItem as applicable; checkable list items also Toggle. Tree ancestry follows the model; leaves report LeafNode. |
| List view | MultipleView; Detail exposes Grid/Table. Built-in spatial arrangements expose Grid/GridItem on existing list items, using the repeat composition's current row/column capacity. Table remains limited to tabular views. View names are localized without changing numeric IDs. |
| Grid cell | DataItem with GridItem/TableItem, SelectionItem, ScrollItem and VirtualizedItem. Editor factory enables Invoke. String binding values expose Value; absence of an editor factory makes them read-only. |
| Grid header | Header with HeaderItems. Headers are outside content view. HeaderItem Invoke sorts; Transform.Resize changes width. Sorting ItemStatus and header names support localization. |
| Scroll view | Scroll when both scroll objects exist. Ordinary descendant controls expose ScrollItem. |
| Scroll / tracker / progress | RangeValue. Progress is read-only and reports NaN increments. |
| Document / textbox | Text and Value outside password restrictions, with logical document children described below. |

Single-selection AddToSelection rejects an already-selected different tab, calendar day, radio or grid/list item without changing selection. Radio SelectionContainer follows the actual mutex controller. Grid GetSelection returns the active logical cell; only that cell reports grid keyboard focus. Focusing and scrolling do not implicitly select another item.

Default grid cells contain the active editor or realized visualizer controls. Their logical cell identity survives editor changes. String Value writes update the binding model without opening an editor or changing selection. Other editor types continue to use their exposed editor controls/Invoke.

List nodes are cached by owner/kind/index; tree nodes additionally retain model identity. Model insertions shift unaffected indices; replacement/removal retires affected nodes. Detail-mode cells/headers become unavailable outside Detail. Realize expands ancestors and realizes rows; ScrollIntoView additionally adjusts horizontal and vertical ancestor view positions to expose the requested cell/control.

Native popups keep their HWND host provider and native FragmentRoot while exposing logical owner navigation. This follows Microsoft's explicit [Assigning a New Parent](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-serversideprovider#assigning-a-new-parent) exception. Hosted popups share the host fragment and retain the same semantic owner. Other hosted windows appear below the main root. Removing native logical ownership solely to follow the generic root navigation rule would regress the documented popup composition.

## Text ranges and objects

The text provider adapts GuiDocumentCommonInterface, including both textbox families. ViewOnly reports no selection; Selectable and Editable report one range, including a caret. Value writes require Editable. Password mode hides Text and plaintext Value; retained ranges reject access while protected. Read-only and disabled mutation checks apply independently.

Snapshots flatten text with CRLF paragraph separators and U+FFFC nontext placeholders. UIA offsets count UTF-16 code units; renderer positions keep surrogate pairs and CRLF pairs indivisible. Character/Format attribute scans use complete character intervals. Caret attributes use the following character, or the preceding character at document end. Word units include trailing separators. Format boundaries include style/alignment differences and object boundaries; Line uses rendered lines, Paragraph uses model rows, and Page/Document use document extent. Failed movement preserves the original range; returned counts reflect actual boundaries crossed.

Supported attributes include read-only, paragraph alignment, font name/size/weight, italic, underline, strikethrough, foreground and background. Mixed and unsupported sentinels are distinct. Text ranges retain their originating DocumentModel, so replacement invalidates old ranges. Same-model edits adjust offsets using common prefixes/suffixes; this is not an edit-history tracker.

Named embedded controls, hyperlink runs and image runs are semantic children in text order. Hyperlinks expose their text and Invoke through the document's ExecuteHyperlink action; images use their source as Name. Application image descriptions therefore need meaningful model sources. RangeFromChild, enclosing-element lookup and point hit testing preserve object identity. GetChildren returns contained children without repeating the enclosing object. Bounds come from the rendered character intervals. Nested run identity follows run parentage.

Text scrolling handles both axes through the document's visibility API and preserves the caret. Visible ranges include a degenerate range for an empty document. Bounding rectangles describe visible nondegenerate text. A point over an embedded child returns that child's range; other points resolve to a caret.

## Notifications and verification

Most notifications are posted/coalesced and compare previous property snapshots. Watched properties include Name/HelpText/LabeledBy, bounds, password/read-only, focus, window state, grid dimensions, selection policy, range/scroll values and pattern availability. Password transitions clear cached plaintext and retire document-object providers; protected document children are omitted. Document SetDocument and direct NotifyParagraphUpdated changes publish TextChanged and structure invalidation; replacement also reports caret changes. Composition CachedBoundsChanged, GuiWindow.BoundsChanged and hosted proxy movement update descendant geometry. Hooks use the composition's current host and coalesce on its window.

Paragraph alignment uses the last included character, preserving exclusive range ends. Separators inherit the preceding character's formatting within the paragraph; empty paragraphs use document defaults. Rectangle iteration uses complete UTF-16 characters. Tab orientation follows the template's TabOrder. Application radios under item/cell templates retain their mutex group semantics.

Document BeforeActiveHyperlinkExecuted exposes the active run before application callbacks, so pointer input and UIA Invoke raise one Invoked event for the same logical hyperlink. WindowClosed is raised synchronously before hosted modal deletion can retire its provider.

Selection notifications compare the container selection: one selected item uses ElementSelected, while multi-selection additions/removals use their respective events. Menu mode starts before the first MenuOpened and ends after the last MenuClosed. Those boundary events are ordered rather than deduplicated with ordinary queued changes. Tooltip open/close and window lifecycle use their corresponding IDs.

WaitForInputIdle posts a retained probe and completes it from a low-priority UI-thread timer after pending work. Client waiting is bounded by the requested timeout; a busy UI thread does not block a synchronous dispatch before the timeout starts. Shutdown signals pending probes unavailable.

[Test/UIA_CppTest_Shared.ps1](../../../GacUI/Test/UIA_CppTest_Shared.ps1) drives an out-of-process MTA client. Review uses the full showcase; Review2, Review3 and Transitions use Playground's [ResourceUiaReview.xml](../../../GacUI/Test/GacUISrc/Playground/Resources/ResourceUiaReview.xml). Select that resource in Playground's `GuiMain`; the fixture UI and handlers are authored in XML/Workflow, with native C++ hooks for Windows metadata and the busy-wait test. Run both ordinary and hosted modes (`-HostedFixture`). Review3 uses native IUIAutomation for WindowClosed because the managed client synthesizes that event from HWND messages and omits provider subscriptions. Each run repeats modal creation, subscription, deletion and disposal 25 times. Its secondary-monitor fallback check requires multiple monitors and reports the limitation on a single-monitor desktop. All also checks lists, trees, grids, text, calendars, tab traversal and window lifecycle. The scripts use repository execution wrappers, restore project user arguments and clean up owned processes. Reflection changes require metadata generation in Win32 then x64, followed by Metadata_Test; C++ changes also require configured UnitTest and leak-log inspection. See the investigation for exact completed results, not the existence of test code alone.

The `Lifetime` scenario uses [UIA_Lifetime.cs](../../../GacUI/Test/UIA_Lifetime.cs), a native IUIAutomation vtable client with explicit release of every returned pointer. It checks two independent client subscriptions, off-thread release, provider-valued labels, identity, event-only descendant discovery, detachment/reinsertion, cross-window moves, model/template replacement, child deletion and retained references across shutdown. `-GdiFixture` selects Playground's GDI renderer; combine it with `-HostedFixture` to exercise hosted GDI.

The focus checks always require one reachable logical focused element and check calendar focus events. Global desktop focus requires an active Windows session with the test application in the foreground: the client checks both prerequisites and explicitly reports that check as unverified when either is absent. Otherwise it requires exact desktop focus identity. Pointer and keyboard menu tests use the application's IO endpoint, so they verify application input routing rather than physical desktop input delivery.

Related design: [lists](./KB_GacUI_Design_ListControlArchitecture.md), [hosted windows](./KB_GacUI_Design_HostedModeWindowManagement.md), [focus](./KB_GacUI_Design_ControlFocusSwitchingAndTabAltHandling.md), and [platform initialization](./KB_GacUI_Design_PlatformInitialization.md).
