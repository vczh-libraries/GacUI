# Windows UI Automation: fresh requirements review

Reviewed on 2026-09-18 against GacUI commit `57b97baa482fd0d0bf5acfe92ed4d59c95404e53`.

This review uses the current implementation and Microsoft's UI Automation documentation on Microsoft Learn (formerly MSDN). `TODO_Task_UiaReview.md` and previous review/task logs were not used. This is a review document only; no provider or test implementation was changed.

**Result: substantial required behavior is implemented and the four focused runtime runs below pass, but the implementation still has the source-level contract gaps listed here.** Passing those suites does not establish complete UIA conformance.

## Evidence and scope

- **Executed:** existing Windows UIA client tests, against freshly checked Debug/x64 build output, in ordinary HWND and hosted-window configurations.
- **Source-confirmed:** a specific implementation path contradicts a documented requirement. The verification steps under the findings are proposed follow-up tests, not claims of executed reproductions.
- **Needs verification:** remaining uncertainties are listed separately and are not counted as defects.
- The review covers semantic navigation, properties, events, all 19 advertised pattern interfaces, text ranges, and the control families mapped by the provider. It does not certify every application template, locale, renderer, assistive technology, or possible UIA extension.

The implementation entry points are [semantic nodes](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp), [COM providers and properties](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.cpp), [actions](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp), [patterns](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationPatterns.Windows.cpp), [text](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp), and [notifications/lifetime](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp). Line references below refer to the reviewed commit.

## Verification performed

The build command was run from `Test/GacUISrc`; the test commands were run from the repository root. All commands completed successfully.

```powershell
& C:/Code/VczhLibraries/GacUI/.github/Scripts/copilotBuild.ps1 -Configuration Debug -Platform x64
& C:/Code/VczhLibraries/GacUI/Test/UIA_CppTest_Shared.ps1 -Application CppTest -AsPort 8988 -Scenario Review -SkipBuild
& C:/Code/VczhLibraries/GacUI/Test/UIA_CppTest_Shared.ps1 -Application CppTest_Metaonly -AsPort 8990 -Scenario Review -SkipBuild
& C:/Code/VczhLibraries/GacUI/Test/UIA_CppTest_Shared.ps1 -Application Playground -AsPort 8991 -Scenario Transitions -SkipBuild
& C:/Code/VczhLibraries/GacUI/Test/UIA_CppTest_Shared.ps1 -Application Playground -AsPort 8991 -Scenario Transitions -HostedFixture -SkipBuild
```

| Run | Result | Scope of evidence |
|---|---|---|
| Debug/x64 solution build | Passed; 0 warnings, 0 errors | Incremental build verified current build output; not a clean rebuild. |
| CppTest / Review / hosted | 72 assertions; structure=37, property=30, text=2 | Tab body hit testing, empty/caret text ranges, embedded controls, combo subtree/selection, grid focus/selection, header bounds/orientation. |
| CppTest_Metaonly / Review / ordinary HWNDs | 72 assertions; structure=36, property=30, text=2 | Same focused contracts with native popup windows. |
| Playground / Transitions / ordinary HWNDs | 71 assertions; structure=0, property=12, text=1 | Password/read-only transitions, disabled text operations, horizontal text scrolling, progress increments, selection events, keyboard Invoke, non-mutex groups, concurrent MTA readers. |
| Playground / Transitions / hosted | 71 assertions; structure=0, property=12, text=1 | Same transition and concurrency contracts in hosted mode. |

Each application also passed the driver's normal `Window.Close`, shutdown, and automation-service endpoint-release checks. Event totals above are observations, not a complete event specification. The assertion bodies are in [UIA_CppTest_Shared.cs](Test/UIA_CppTest_Shared.cs#L370); these runs do not execute every scenario in that file.

## Requirements actually implemented

The following are positive findings from current code. Runtime evidence is identified where available; other entries were verified by reading the implementation.

| Area | Current implementation and evidence | Microsoft contract |
|---|---|---|
| Combo ownership and selection | Popup ownership resolves to the opener; combo selection is aggregated from its contained list, and selected items identify the combo as their SelectionContainer. ExpandCollapse is exposed; Scroll is absent from the combo root. Descendant navigation, hit testing, and failed removal preserving required selection passed in both Review runs. Native fragment composition still has R2-04. | [ComboBox](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcomboboxcontroltype) |
| Grid cell contents | The default grid exposes visualizer controls and the active editor beneath their logical cell. Parent mapping recognizes the cell before the enclosing row. Grid/Table dimensions, cells, unit spans, containing grid and column headers are implemented. Current editor placement is visible in [GuiDataGridControls.cpp](Source/Controls/ListControlPackage/GuiDataGridControls.cpp#L343). | [DataGrid](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdatagridcontroltype), [DataItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdataitemcontroltype) |
| Tabs | `Tab -> TabItem -> TabContent(Pane) -> controls` has reciprocal body ownership. Header bounds are separate; hit testing explicitly reaches the selected body. Tab Selection and TabItem SelectionItem are present; TabItem does not expose Invoke. Body hit testing passed. | [TabItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttabitemcontroltype) |
| Lists and trees | Model-backed items retain semantic identities; tree ancestry follows the model, ordinary traversal hides collapsed descendants, and leaves report LeafNode. ItemContainer searches model items; Realize expands ancestors and brings the row into view. | [TreeItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttreeitemcontroltype), [ItemContainer](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingitemcontainer) |
| Grid/list selection and focus | Conflicting single-selection AddToSelection is rejected for lists/grids. Grid GetSelection returns the active logical cell. SetFocus checks focusability and does not select arbitrary items. Active-cell focus and atomic rejection passed in Review. | [SelectionItem](https://learn.microsoft.com/en-us/dotnet/framework/ui-automation/implementing-the-ui-automation-selectionitem-control-pattern), [SetFocus](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-irawelementproviderfragment-setfocus) |
| Calendar core patterns | Default date picker exposes Selection/Grid/Table, six week rows and seven weekday columns, logical dates, weekday headers, and cell/header relationships. Focus and AddToSelection remain separate gaps. | [Calendar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcalendarcontroltype) |
| Text and Value | Document controls expose Value and, outside password mode, Text. Ranges, selection, search, attributes, rectangles, embedded GuiDocumentItem controls, and UTF-16/renderer position mapping exist. Empty visible ranges, collapsed selection, embedded enclosing-element behavior, and horizontal scrolling preserving the caret passed. This does not establish correct Word/Format/Unicode behavior. | [Text and TextRange](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtextandtextrange) |
| Protected/read-only text | Password IValueProvider::get_Value reads fail, Text becomes unavailable, retained ranges fail while protected, and password transitions discard cached plaintext before notification. Read-only writes and disabled selection mutations are rejected. Transition tests passed in both modes, including no plaintext in observed password-transition events. The direct property route is discussed separately below. | [Edit](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporteditcontroltype), [Value](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingvalue) |
| Actions and numeric patterns | Invoke and Close queue work and recheck liveness; button invocation checks lifetime between click stages. Toggle distinguishes checkbox behavior from mutex radios. RangeValue validates finite/range inputs, rejects progress writes, and gives progress NaN increments. SetScrollPercent validates both axes before changing either. | [Invoke](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinginvoke), [RangeValue](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingrangevalue), [Scroll](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingscroll) |
| Properties and events | BoundingRectangle is returned as a four-double SAFEARRAY. Header/tab orientation, password, focusability, grid counts, selection policy, and window state properties are exposed. Selection changes distinguish one selected item from additions/removals. Password/read-only, keyboard Invoke, and list selection event tests passed. Missing triggers/roles remain below. | [Automation element properties](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-automation-element-propids) |
| COM/lifetime | Atomic COM reference counts, UI-thread dispatch, retirement checks, provider disconnection and retained model lifetimes are implemented. Four concurrent MTA readers passed the transition fixture. This is limited concurrency evidence, not a proof for every destruction race. | [Provider threading options](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/ne-uiautomationcore-provideroptions), [client threading](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-threading) |

### Control and pattern coverage

[Role/Supports](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L220) currently maps 31 control-type constants. The advertised interfaces are Invoke, Toggle, Selection, SelectionItem, ExpandCollapse, Value, RangeValue, Scroll, ScrollItem, Grid, GridItem, Table, TableItem, ItemContainer, VirtualizedItem, MultipleView, Window, Transform, and Text. This table summarizes coverage; having an interface does not by itself satisfy its behavioral contract.

| Control family / roles | Implemented surface | Remaining concerns in this review |
|---|---|---|
| Button, CheckBox, RadioButton | Invoke, Toggle or SelectionItem according to behavior | Radio grouping: R2-08. |
| ComboBox, Menu, MenuItem, SplitButton, MenuBar | Conditional Invoke/Toggle/ExpandCollapse; combo Selection; opener ownership | R2-01, R2-04, R2-23. |
| List, ListItem, Tree, TreeItem | Selection, SelectionItem, Scroll, ItemContainer, ScrollItem, VirtualizedItem; tree ExpandCollapse; checkable items Toggle | Container child scrolling: R2-09. |
| DataGrid, DataItem, Header, HeaderItem | Grid/Table, GridItem/TableItem, cell selection/edit Invoke, header Invoke/Resize; list-view MultipleView | R2-10, R2-11, R2-24. |
| Edit, Document | Value, Text and text ranges, embedded controls | R2-01, R2-15 through R2-21. |
| Tab, TabItem | Selection/SelectionItem and explicit page-body Pane | R2-02, R2-07. |
| Calendar | Selection, Grid/Table, ItemContainer and logical date SelectionItem/GridItem/TableItem | R2-03, R2-07. |
| ScrollBar, Slider, ProgressBar | RangeValue, including read-only progress | No additional numeric-contract finding from this review. |
| Window | Window and Transform, ordinary and hosted routing | R2-06, R2-12 through R2-14, R2-22. |
| ToolTip | Owner relationship and opened/closed notifications | R2-04, R2-05. |
| Pane, Group, ToolBar, Text, Separator | Role/property exposure and composition-derived children; Scroll where backed by a scroll view | R2-09 and cross-cutting metadata/events. No universal action pattern is presumed for these roles. |

## Remaining source-confirmed gaps

P1 means an existing interactive control can be unreachable or lose its semantic keyboard focus. P2 means a specific supported operation/property/event violates its contract. P3 covers localized presentation. All entries remain open; the scenarios below should become targeted regression tests before a fix is considered complete.

### R2-01 [P2] Edit and combo names cannot expose an accessible label

Microsoft requires an Edit/ComboBox name derived from its label or supplied by the application, distinct from its current value; an associated static label must be exposed through LabeledBy. See [Edit properties](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporteditcontroltype) and [ComboBox properties](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcomboboxcontroltype).

[Name](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L215) now correctly avoids using input contents as the label, but always returns an empty string for document controls and combo boxes. The [property switch](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.cpp#L96) has no LabeledBy mapping or alternative accessible-name source. Thus even a visible labeled input cannot expose its label through these properties.

- [ ] Verify a labeled single-line editor and labeled combo return stable, nonempty names and the intended label element while their values change. Provide an application-supplied name path for inputs without a static label.

### R2-02 [P1] Ribbon header controls are omitted from semantic traversal

Fragment navigation must be reciprocal: children returned by parent navigation must be reachable from that parent. See [Navigate](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-irawelementproviderfragment-navigate).

[Children](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L429) enumerates only pages for every GuiTab, including GuiRibbonTab. [GuiRibbonTab](Source/Controls/ToolstripPackage/GuiRibbonControls.cpp#L39) also installs controls before/after its headers. The existing [ribbon document editor](Test/Resources/App/FullControlTest/DocumentEditorRibbon.xml#L225) supplies a HOME button and Search editor there. Their semantic Parent resolves to the ribbon, but its Children excludes them; recursive hit testing and GetFocus consequently cannot reach them.

- [ ] Open that ribbon example and verify HOME/Search are enumerable, hit-testable, and returned by focus lookup when focused.

### R2-03 [P1] Calendar-day focus is not mapped to the logical day

The fragment root must return its focused descendant. See [GetFocus](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-irawelementproviderfragmentroot-getfocus).

The [calendar children](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L438) replace real day buttons with CalendarDay nodes, but [IsFocusable/IsFocused](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L355) recognize only Control nodes and selected grid cells. They do not use the corresponding day button. The [actual day buttons](Source/Controls/Templates/GuiCommonTemplates.cpp#L293) take focus on [mouse-down](Source/Controls/GuiButtonControls.cpp#L124). The semantic day therefore reports no focus, SetFocus rejects it, and root traversal cannot find the underlying focused button. A focus notification can instead refer to the omitted underlying button provider.

- [ ] Click a date in the DarkSkin date picker; require GetFocus, HasKeyboardFocus, IsKeyboardFocusable, SetFocus and focus events to identify the same reachable logical day.

### R2-04 [P2] Native popup ownership crosses fragment-root navigation boundaries

Microsoft requires fragment roots to leave parent/sibling navigation to the default window provider; direct fragment navigation stays within its fragment. See [Navigate remarks](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-irawelementproviderfragment-navigate).

In ordinary HWND mode, [IsRoot](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L89) treats menu/tooltip windows as fragment roots. [Parent](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L472) nevertheless returns the popup opener before checking IsRoot. [Navigate](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.cpp#L231) follows that relationship, while HostRawElementProvider and FragmentRoot still identify the popup's own HWND/root. This mixes two fragment contracts. Hosted popups share a fragment and do not have this particular contradiction.

The combo subtree requirement is real, and the client-facing subtree now passes the existing tests. Correct ownership must be preserved while reconciling native HWND fragment composition; merely removing the owner relationship would regress that behavior.

- [ ] In ordinary mode, inspect the raw provider's host, FragmentRoot, and Parent/NextSibling/PreviousSibling for an open combo, submenu and tooltip; also verify the composed client tree still places each popup beneath its semantic owner.

### R2-05 [P2] Ordinary tooltip content has the wrong metadata/content-view behavior

ToolTip Name describes displayed text. Unfocusable tooltip information belongs in its owner's HelpText, and such a tooltip is excluded from the content view. See [ToolTip requirements](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttooltipcontroltype).

[IsContentElement](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.cpp#L127) always includes ToolTip, and HelpText is unhandled. Name reads the wrapper's GetText, but [ShowTooltip](Source/Application/Controls/GuiApplication.cpp#L305) supplies a separate content control; [SetTemporaryContentControl](Source/Application/Controls/GuiWindowControls.cpp#L1477) attaches it without copying its text. A normal label tooltip can therefore have an empty tooltip Name and no owner HelpText despite visible text.

- [ ] Show a simple label tooltip and check Name, owner HelpText, control/content-view membership, and owner ancestry. Test an interactive tooltip separately because its content-view requirement differs.

### R2-06 [P2] Plain GuiPopup has Window role without the required patterns

The desktop Window role requires Window and Transform. See [Window patterns](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportwindowcontroltype).

[Role](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L235) maps a plain GuiPopup through GuiWindow to Window. [Supports](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L327) excludes every GuiPopup from both patterns. GuiPopup is publicly constructible and can be displayed; this is separate from menu/tooltip subclasses with their own roles.

- [ ] Display a plain GuiPopup and reconcile its role with its supported operations. Preserve the distinct menu/tooltip contracts.

### R2-07 [P2] Tab/calendar AddToSelection replaces a conflicting selection

Adding another item to a single-selection container must fail without changing selection. See [SelectionItem implementation exceptions](https://learn.microsoft.com/en-us/dotnet/framework/ui-automation/implementing-the-ui-automation-selectionitem-control-pattern).

[UiaSelect](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp#L69) distinguishes Select from AddToSelection for grids/lists, but the calendar and tab branches ignore `clear` and always replace the selected day/page. Both containers report CanSelectMultiple=false. The current Review assertions cover the corrected grid case, not these branches.

- [ ] Select A, then call AddToSelection on B in a tab and calendar. Require invalid-operation failure, A remaining selected, and no spurious selection-change event. Recheck mutex radios after R2-08.

### R2-08 [P2] Radio SelectionContainer identifies layout ancestry instead of the mutex group

SelectionContainer identifies the provider implementing Selection for the item; radio peers must correspond to their selection group. See [SelectionContainer](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-iselectionitemprovider-get_selectioncontainer) and [RadioButton](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportradiobuttoncontroltype).

[get_SelectionContainer](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp#L128) returns an ordinary radio's semantic Parent. That parent is determined by composition ancestry and often does not support Selection. It does not model MutexGroupController: separate groups can share a parent, while one group can span multiple layout parents. The problem is returning an unrelated provider; adding Selection indiscriminately to every radio parent would not solve it.

- [ ] Test two independent mutex groups under one layout parent and one mutex group spanning two parents. Verify the returned container and its selection set identify the actual peers.

### R2-09 [P2] Ordinary children of scroll containers lack ScrollItem

Microsoft's [Scroll implementation guidance](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingscroll) requires its child controls to support ScrollItem.

[Supports](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L297) exposes ScrollItem only for synthetic list/tree nodes, while ordinary GuiScrollView/GuiScrollContainer expose Scroll. There is no Control-kind ScrollItem branch, and [Realize/ScrollIntoView](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp#L281) handles only lists. An offscreen ordinary button/editor in a scrolling container cannot request visibility through UIA.

- [ ] Put an ordinary focusable control outside a GuiScrollContainer viewport. Require ScrollItem to bring it into view without changing selection or focus; include nested scrolling containers.

### R2-10 [P2] Grid cell ScrollIntoView ignores horizontal position

[ScrollIntoView](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-iscrollitemprovider-scrollintoview) must make the target visible.

[Realize](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp#L289) requests only `EnsureItemVisible(ItemIndex())`. The [default fixed-height arranger](Source/GraphicsComposition/GuiGraphicsRepeatComposition_Virtual.cpp#L835) adjusts Y and preserves the current horizontal position. A far-right cell in an already visible row can return success while remaining outside the horizontal viewport.

- [ ] Obtain a cell in a horizontally hidden column, call ScrollIntoView, and verify that cell becomes visible. Retain the existing last-row scrolling test.

### R2-11 [P2, conditional] Text-editable DataItem cells lack Value

DataItem requires Value when its primary text is editable. See [DataItem patterns](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdataitemcontroltype).

The [logical Cell pattern switch](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L292) exposes Invoke for an editor factory but never Value. The existing [Name-cell scenario](Test/UIA_CppTest_Shared.cs#L216) edits primary text through a transient child editor. Value on that child does not provide Value on the DataItem before editing starts. This finding applies to text-editable cells, not every arbitrary custom editor.

- [ ] Query the existing text-editable Name cell before opening its editor; verify the DataItem's Value read/write/read-only contract and its value-change event.

### R2-12 [P2] WaitForInputIdle ignores its timeout and always reports success

[WaitForInputIdle](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-iwindowprovider-waitforinputidle) must distinguish idle completion from timeout.

[The implementation](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationPatterns.Windows.cpp#L233) checks that milliseconds is nonnegative, then returns TRUE through Read without using the timeout. [Dispatcher invocation](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomation.Windows.cpp#L67) uses synchronous SendMessage without a timeout. A busy UI thread can exceed the requested wait, and processing a provider callback is not itself proof that the application is idle.

- [ ] Keep the UI thread busy longer than a short timeout; require bounded completion and FALSE. Also verify a genuinely idle application returns TRUE.

### R2-13 [P2] Transform.Move can place windows wholly outside their container

[Move](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itransformprovider-move) requires an inaccessible requested location to be adjusted to an accessible position within the container's boundaries.

[Move](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationPatterns.Windows.cpp#L252) accepts finite coordinates and directly sets bounds. [WinNativeWindow](Source/PlatformProviders/Windows/WinNativeWindow.cpp#L1220) forwards them to MoveWindow; hosted child [FixBounds](Source/PlatformProviders/Hosted/GuiHostedWindowProxy_NonMain.cpp#L63) constrains size, not position. The existing small-offset transform check does not cover an entirely inaccessible target.

- [ ] Move a hosted child beyond each host edge and an ordinary window beyond the desktop; require an accessible clamped position consistent with the native contract.

### R2-14 [P2] Window modal state is inferred from unrelated enabled flags

BlockedByModalWindow must represent actual modal blocking, and IsModal must identify an actual modal window. See [WindowInteractionState](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/ne-uiautomationcore-windowinteractionstate) and [IWindowProvider properties](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iwindowprovider).

[Window getters](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationPatterns.Windows.cpp#L248) report every disabled window as BlockedByModalWindow and every owned window whose parent is disabled as IsModal. Public [SetEnabled](Source/Application/Controls/GuiWindowControls.cpp#L337) can disable a window independently of any modal operation. These cases therefore report false modal information.

- [ ] Manually disable a normal window and the owner of a normal owned window, then compare IsModal/WindowInteractionState with a real ShowModal case.

### R2-15 [P2] Attribute search and Format expansion can split a surrogate pair

Attribute search must return text carrying the requested attribute, and Format expansion must preserve the corresponding text unit. See [FindAttribute](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-findattribute) and [text units](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-uiautomationtextunits).

[Snapshot mapping](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L55) maps both UTF-16 halves to the same renderer caret. However, [Format boundaries](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L203) and [FindAttribute](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L280) sample individual code-unit intervals. For a uniformly normal U+1F600 (`D83D DE00`), `[0,1)` becomes an empty style query and reports Mixed, while `[1,2)` obtains the actual style. FindAttribute(FontWeight=400) can return only the low surrogate; Format can insert a boundary between the halves.

- [ ] Test attribute search and Format expansion on a supplementary character, including one split across document runs. Require complete characters and correct UTF-16 endpoints.

### R2-16 [P2] A caret range reports Mixed formatting for uniform text

A degenerate range at a unit boundary uses adjacent text properties; Mixed represents actual variation. See [endpoint rules](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-uiautomationtextunits) and [GetAttributeValue](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-getattributevalue).

[Attribute](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L241) calls SummarizeStyle with equal positions. [SummarizeStyle](Source/Resources/GuiDocument_Edit.cpp#L592) skips an empty range and returns an all-null style; the UIA adapter interprets those null font/color fields as Mixed. A caret at the beginning of uniformly formatted `abc` therefore reports Mixed font/weight/color.

- [ ] Query caret attributes at document start, a formatting boundary and document end; distinguish actual mixed ranges from empty ranges and unsupported attributes.

### R2-17 [P2] Move changes a range while reporting zero movement

Microsoft defines separate degenerate movement and nondegenerate normalization rules, and requires a failed nondegenerate move to preserve the range. See [Move](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-move).

[Move](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L403) floors the start to a unit boundary before counting movement. In `abc`, a caret at offset 2 followed by Move(Document,-1) moves to 0 but reports 0. A range containing only `b` followed by the same call becomes the whole document while reporting 0, although no previous document unit exists.

- [ ] Check both cases, endpoint preservation when no nondegenerate move is possible, partial movement counts, and zero-count calls.

### R2-18 [P2] Word and Format boundaries omit required distinctions

Word units include trailing separators, and embedded objects impose their own unit boundaries. See [Word/Format units](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-uiautomationtextunits) and [embedded-object units](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-textpattern-and-embedded-objects-overview).

[Boundaries](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L214) makes the space in `one two` a separate word. Adjacent incompatible embedded objects can be merged into one Word/Format unit because both are U+FFFC and have matching sampled styles. Format comparison checks font/color attributes without enforcing explicit object start/end boundaries.

- [ ] Require `one ` and `two` word units; separate adjacent embedded objects; stop Format at object boundaries. Include punctuation and non-English text in later coverage.

### R2-19 [P2] Document hyperlinks and images have no semantic object identity

Images and hyperlinks in a Text provider are individual embedded elements with discoverable ranges. See [embedded objects](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-textpattern-and-embedded-objects-overview).

[Append](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L19) flattens DocumentHyperlinkRun as a container. It substitutes a character for DocumentImageRun without recording object identity. [UiaTextChildren](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L71) enumerates only named GuiDocumentItem controls. Thus existing embedded controls work, but native document hyperlinks/images are absent from the control tree and GetChildren; hyperlinks cannot be invoked as semantic links.

- [ ] Display one document hyperlink and one image. Verify control-tree discovery, appropriate roles/actions, GetChildren, RangeFromChild and GetEnclosingElement, without losing the surrounding text stream.

### R2-20 [P2] RangeFromPoint never encloses an embedded object

[RangeFromPoint](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextprovider-rangefrompoint) specifies an enclosing child-object range when the point lies inside an embedded object.

[The implementation](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L510) always creates a degenerate range at the nearest caret. Even an already exposed embedded button therefore receives an empty point range instead of its object range.

- [ ] Use the center of an embedded button's rectangle and compare RangeFromPoint with RangeFromChild. Include a nested embedded container and an ordinary text point.

### R2-21 [P2] Direct document updates omit text/selection events

Text providers must notify clients about text and selection changes. See [Text/TextRange events](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtextandtextrange).

[SetDocument](Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp#L1054) and [NotifyParagraphUpdated](Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp#L1158) do not raise the control's TextChanged event. [GuiDocumentElement::SetDocument](Source/GraphicsElement/GuiGraphicsDocumentElement.cpp#L65) changes the model and silently resets the caret. The UIA bridge relies on [TextChanged](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L89) and SelectionChanged callbacks. Reading the new model works, but these direct update paths do not publish corresponding events.

- [ ] Subscribe, move the caret away from the beginning, then replace the document. Check both text and changed-selection notification. Separately mutate a paragraph and call NotifyParagraphUpdated with updatedText=true.

### R2-22 [P2] A hosted secondary window can move without a bounds-change event

Window controls require bounding-rectangle property-change events. See [Window required events](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportwindowcontroltype).

[UpdateProperties](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L295) watches the rectangle, but only runs after Notify. [Transform.Move](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationPatterns.Windows.cpp#L252) calls SetBounds without notifying UIA. For a hosted secondary window, [GuiHostedWindow::SetBounds](Source/PlatformProviders/Hosted/GuiHostedWindow.cpp#L100) reaches [Window::SetBounds](Source/PlatformProviders/Hosted/GuiHostedWindowManager.h#L266), which updates bounds/needRefresh; the [non-main proxy UpdateBounds](Source/PlatformProviders/Hosted/GuiHostedWindowProxy_NonMain.cpp#L77) is empty. No separate HWND provider exists to fill the notification gap.

The ChildCompositionUpdated hook does not cover pure translation: its [sources](Source/Application/GraphicsCompositions/GuiGraphicsComposition.cpp#L180) are insertion, removal and sibling reordering. UIA does not subscribe to the separate layout-bounds event either. Once initialization callbacks have drained, moving a static hosted secondary window therefore changes queried bounds without scheduling the required event. Resize/maximize can cause incidental notifications through other callbacks, so this finding does not assert that every such transition loses events.

- [ ] Subscribe to a hosted child window's BoundingRectangle event and move it without resizing/focusing/reparenting. Require the event without an unrelated later action. Also check ordinary windows, descendant screen rectangles and visual-state transitions, distinguishing OS-generated root events from GacUI's own semantic providers.

### R2-23 [P2] Menu-mode boundary events are absent

MenuModeStart precedes the first MenuOpened, and MenuModeEnd follows the last MenuClosed. See [MenuBar event ordering](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportmenubarcontroltype).

The bridge maps window opening/closing to menu events in [State](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L100), but Source contains no emission of UIA_MenuModeStartEventId or UIA_MenuModeEndEventId. The [menu Alt-host path](Source/Controls/ToolstripPackage/GuiMenuControls.cpp#L330) and [Alt manager](Source/Application/GraphicsHost/GuiGraphicsHost_Alt.cpp#L338) do capture navigation/text input, so this is applicable menu-mode behavior. Submenu-open notifications alone do not communicate that boundary.

- [ ] Enter menu mode by keyboard and pointer, open nested menus, then dismiss/activate a command. Verify one correctly ordered mode boundary pair around the menu-open/close sequence in ordinary and hosted modes.

### R2-24 [P3] MultipleView names have no localization path

[GetViewName](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-imultipleviewprovider-getviewname) requires a localized presentation name.

[GetViewName](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationPatterns.Windows.cpp#L185) returns fixed enum-style strings such as BigIcon and SmallIcon. There is no locale/application-supplied naming path. Stable numeric view IDs are useful and already present; they need not change when display names are localized.

- [ ] Verify readable view names under another application locale while retaining stable IDs. Audit other provider-generated user-facing strings such as the column-header group name and sort status at the same time.

## Additional verification, not established defects

- [ ] **Format attribute coverage:** Format boundary comparison does not explicitly include the supported paragraph-alignment attribute. Paragraph separators can already induce a boundary through other sampled styles, so establish an actual failing case before treating that omission as another defect.
- [ ] **Raw password property route:** IValueProvider::get_Value correctly fails, but direct UIA_ValueValuePropertyId returns VT_EMPTY in [Property](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.cpp#L143). Verify the client-facing error/default behavior separately. This review found no plaintext exposure through that route.
- [ ] **Geometry:** validate partly clipped glyphs, mixed-font lines, bidi/combining text, DPI changes, and overlapping/hosted windows. [GetVisibleRanges](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L549) uses caret visibility; a partially visible glyph can need a distinct check. Fragment bounds currently clip against ancestors; Microsoft's [rectangle API](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-irawelementproviderfragment-get_boundingrectangle) says clipping is not required, so clipping alone is not counted here as a proven violation.
- [ ] **Retained ranges across multiple edits:** [Normalize](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L138) infers one replacement from common prefixes/suffixes. Test multiple intervening edits and document replacement before a retained range is read again.
- [ ] **Lifetime/event ordering:** extend destruction, replacement, popup closure and ordinary header-click coverage. Current concurrent-reader tests do not establish that every terminal event is delivered on the intended semantic node before retirement.
- [ ] **Custom skins/templates:** the default grid/calendar composition paths were reviewed. Check custom arrangements against the same parent, header, cell and focus contracts.

## Interpretation limits

- A combo's dropdown belongs in the combo's semantic subtree, and active grid visualizers/editors belong beneath their cell. Both relationships now exist in the current implementation. R2-04 concerns how the native popup fragment participates in that tree, not whether ownership should be removed.
- Microsoft's "Typical Tree Structure" illustrations are not by themselves proof that every extra wrapper is forbidden. This review does not mark the combo's Menu/List wrappers or the calendar's lack of an intermediate DataGrid/Header wrapper as defects when their behavioral interfaces are present.
- Conditional patterns apply when the corresponding feature exists. Calendar Scroll is recommended for paging, not a universal prerequisite. Dock, annotation, drag/drop, TextEdit, TextChild, ITextProvider2 and ITextRangeProvider2 are not blanket requirements merely because the platform defines them.
- UIA can supply defaults for some standard properties. An unhandled property ID is not automatically a defect; the metadata findings above identify requirements the current provider cannot satisfy through its actual code paths.
- No implementation change is included in this review. Each unchecked item is a proposed correction/verification task for subsequent work.
