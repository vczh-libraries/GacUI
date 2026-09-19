# Windows UI Automation requirements review 3

Reviewed on 2026-09-19. Source baseline: `bc9c38c471d08cb76700e17d10220ce39d69cd62`. The parallel task advanced the branch to `8574352483579a2726951453d404f3a625feeccb` by changing a separate task document; the reviewed source remained identical.

Neither `TODO_Task_UiaReview.md` nor `TODO_Task_UiaReview2.md` was read for this pass. Findings were derived from current source, current test code, and the linked Microsoft Learn/MSDN contracts. The current knowledge-base inventory was used to locate implementation paths, not as proof of compliance.

**Result: 14 source-confirmed gaps: one P1, twelve P2, and one P3.** Many requirements have actual implementations, as recorded below. Several remaining gaps require particular supported configurations rather than occurring in every application.

## Method and limits

- This was a source and contract review. No builds, UI applications, or runtime tests were run during this pass, to avoid interfering with the task running in the same checkout. No implementation or test files were changed.
- A finding includes a documented rule, an implementation trace, and a concrete scenario. Its proposed regression check has **not** been executed here. The wrapped-text rectangle finding particularly needs renderer-level confirmation.
- Existing assertions in [UIA_CppTest_Shared.cs](Test/UIA_CppTest_Shared.cs) were inspected to understand coverage. Their presence is not reported as a passing test result. Earlier test logs/results were not reused.
- Coverage includes semantic navigation, properties, notifications, lifetime, Text/TextRange, and the 19 advertised pattern interfaces. This is not certification of every custom template, document configuration, locale, screen arrangement, or assistive-technology client.
- P1 denotes protected-content exposure; P2 denotes functional or documented-contract failures; P3 denotes the accessible but contractually incorrect monitor fallback. Qualified observations outside the 14 findings appear separately.

## Requirements with working implementation paths

The evidence in this table is source-level. It establishes real behavior beyond the presence of interface declarations, subject to the specific exceptions in the findings.

| Area | Current implementation | Microsoft contract |
|---|---|---|
| Combo/menu popup ownership | [Popup owner resolution](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L58) and semantic navigation place dropdowns beneath their opener. Combo Selection delegates to its contained list, whose items identify the combo as SelectionContainer. Native popups retain their HWND host provider; hosted popups share the host fragment. | [ComboBox](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcomboboxcontroltype), [Assigning a New Parent](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-serversideprovider#assigning-a-new-parent) |
| Grid cells and editing | [Cell children](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L513) expose the active editor or visualizer controls under the cell. Grid/Table return logical cells, dimensions, containing grid and headers. Grid Selection returns the active cell. String-valued cells expose Value and write the binding model without opening an editor. | [DataGrid](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdatagridcontroltype), [DataItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdataitemcontroltype) |
| Tabs, ribbon, tree and calendar | Selected tab content has an explicit Pane; ribbon before/after-header controls are enumerated. Tree parents follow model ancestry and collapsed descendants are omitted from ordinary navigation. Calendar exposes six-by-seven days and weekday headers; logical day focus delegates to the underlying button. | [TabItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttabitemcontroltype), [TreeItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttreeitemcontroltype), [Calendar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcalendarcontroltype) |
| Names, labels, help and localization | [Metadata helpers](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomation.Windows.cpp#L38) provide explicit names, labels and inherited localized text independently of input values. Label text changes notify associated controls. Tooltip control text supplies tooltip Name/owner HelpText; interactive tooltip content determines content-view membership. View IDs remain stable while view names are localized. | [Edit](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporteditcontroltype), [ToolTip](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttooltipcontroltype), [GetViewName](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-imultipleviewprovider-getviewname) |
| Ordinary selection groups | [Selection actions](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp#L72) reject conflicting AddToSelection for ordinary mutex groups, tabs, calendars and single-select list/grid controls. Ordinary radio groups have logical Selection containers; combo required-selection removal is rejected. | [SelectionItem](https://learn.microsoft.com/en-us/dotnet/framework/ui-automation/implementing-the-ui-automation-selectionitem-control-pattern) |
| Scrolling and virtualization | [Scroll/realization actions](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp) validate percentages/sentinels, expose ordinary descendant ScrollItem, realize rows/tree ancestors and adjust both viewport axes. ItemContainer supports model searches and enumeration. Richer providers for virtualized items are allowed. | [Scroll](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingscroll), [ItemContainer](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingitemcontainer) |
| Invocation and numeric state | [Read/Queue](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.cpp#L37) check target lifetime, pattern availability and enabled state. Invoke/Close queue actions and revalidate before execution. RangeValue rejects nonfinite/out-of-range inputs; progress is read-only with NaN increments. Ordinary button input emits Invoked. | [Invoke](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinginvoke), [RangeValue](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingrangevalue) |
| Windows and bounded idle waiting | [Window/Transform](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationPatterns.Windows.cpp#L244) use actual modal-session state and capability checks. Move constrains location to a screen/host container. [WaitForIdle](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomation.Windows.cpp#L168) posts a retained probe and bounds the client wait; shutdown releases pending probes. | [WaitForInputIdle](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-iwindowprovider-waitforinputidle), [Transform](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtransform) |
| Text and ranges | [Text implementation](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp) has document/selection/visible ranges, search, attributes, movement and object-aware ranges. Character scanning for attributes/Format respects UTF-16 surrogate pairs. Failed nondegenerate movement preserves endpoints. ViewOnly versus single-selection modes and disabled/read-only mutation checks are implemented. | [Text/TextRange](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtextandtextrange) |
| Document objects | Hyperlink/image runs and named embedded controls have semantic identities in text order. RangeFromChild, GetEnclosingElement and object-aware RangeFromPoint use those identities, including nested objects. Hyperlink Invoke routes to the document action. | [Embedded objects](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-textpattern-and-embedded-objects-overview), [RangeFromPoint](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextprovider-rangefrompoint) |
| Default password paths and range lifetime | Password mode suppresses Text, rejects IValueProvider::get_Value and retained TextRange access, and drops cached plaintext Value before notifications. Document replacement invalidates ranges tied to the old model. R3-01 identifies a different child-provider route under a nondefault supported configuration. | [Edit](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporteditcontroltype), [protected-content guidance](https://learn.microsoft.com/en-us/dotnet/framework/ui-automation/ui-automation-textpattern-overview#security) |
| Events and provider lifetime | [State hooks](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp) propagate window translation to descendants, publish direct document text/selection changes, track selection cardinality and order menu-mode boundaries around menu open/close. Atomic reference counts, UI-thread dispatch, retirement checks and provider disconnection are present. | [Event definitions](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-event-ids), [provider options](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/ne-uiautomationcore-provideroptions) |

The advertised patterns are Invoke, Toggle, Selection, SelectionItem, ExpandCollapse, Value, RangeValue, Scroll, ScrollItem, Grid, GridItem, Table, TableItem, ItemContainer, VirtualizedItem, MultipleView, Window, Transform and Text. Their dispatch is conditional on node/control behavior; QueryInterface alone is not evidence of current pattern availability.

## Remaining gaps

### R3-01 [P1] Rich-config password editors expose masked hyperlink text through children

**Requirement:** UIA providers must not expose protected/password content. See [Microsoft's protected-content guidance](https://learn.microsoft.com/en-us/dotnet/framework/ui-automation/ui-automation-textpattern-overview#security).

**Current path:** A GuiSinglelineTextBox accepts a custom configuration in [GuiDocumentViewer.cpp:307](Source/Controls/TextEditorPackage/GuiDocumentViewer.cpp#L307). An explicit `pasteAsPlainText=false` survives [configuration merging](Source/Controls/TextEditorPackage/GuiDocumentConfig.cpp#L73); FixConfig only adjusts autoExpand. [UserInput_FormatDocument](Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp#L992) strips rich runs only when pasteAsPlainText is true. The [renderer](Source/GraphicsElement/GuiGraphicsDocumentRenderer_GuiDocumentParagraphCache.cpp#L266) masks the paragraph when PasswordChar is set.

However, [semantic document children](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L535) still enumerate hyperlink objects, [DocumentObject liveness](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L87) does not check password state, and [object Name](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L109) reads the unmasked model. TextPattern/Value gating does not cover this route.

- [ ] Construct a single-line editor with pasteAsPlainText=false, supply a hyperlink containing `secret`, then set PasswordChar. Require newly discovered and retained hyperlink providers to reveal no protected text. Also retain the default plain-text password tests; their configuration strips the rich runs and does not exercise this case.

### R3-02 [P2] Application radios inside grid editors lose their selection group

**Requirement:** SelectionContainer identifies the provider implementing Selection; adding a different item to an occupied single-selection group must fail without changing selection. See [SelectionContainer](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-iselectionitemprovider-get_selectioncontainer) and [SelectionItem exceptions](https://learn.microsoft.com/en-us/dotnet/framework/ui-automation/implementing-the-ui-automation-selectionitem-control-pattern).

**Current path:** [UiaRadioController](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L28) suppresses radios whose ancestor scan reaches GuiListControl before a GuiTabPage (also suppressing some other control families). Yet [cell children](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L513) expose real application controls inside an active editor, and the [pattern switch](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L379) still gives their mutex buttons SelectionItem. [SelectionContainer](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp#L138) falls back to a layout parent/cell without Selection, while [AddToSelection](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp#L116) skips the peer conflict check when no logical radio group exists.

- [ ] Put two mutex radios A/B in a custom grid editor, select A and enumerate B through the cell. Require a real group SelectionContainer and rejection of B.AddToSelection without switching A to B. Distinguish genuine application radios from internal template selection buttons when deciding which controls to suppress.

### R3-03 [P2] Spatial icon-list navigation lacks Grid/GridItem

**Requirement:** A list arranged in rows/columns with item-to-item spatial navigation must expose GridItem on its items and the corresponding Grid container behavior. See [ListItem patterns](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportlistitemcontroltype) and [List patterns](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportlistcontroltype).

**Current path:** [GuiListView view selection](Source/Controls/ListControlPackage/GuiListViewControls.cpp#L465) uses a multicolumn arranger for built-in BigIcon, SmallIcon and Tile views. [Keyboard navigation](Source/GraphicsComposition/GuiGraphicsRepeatComposition_Virtual.cpp#L1027) moves up/down by the column count and left/right by one. The [UIA pattern switch](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L362) nevertheless limits GridItem to Cell/CalendarDay and Grid to Detail/data-grid/calendar cases.

- [ ] Display at least two rows and columns of BigIcon items. Verify GridItem coordinates/ContainingGrid, Grid dimensions/GetItem, and consistent updates when resizing changes the visual column count. Apply the rule to other views only where their actual spatial-navigation condition holds.

### R3-04 [P2] AutomationId is a runtime allocation counter

**Requirement:** AutomationId is optional, but when provided it must identify the same element consistently across application instances; unpredictable dynamic list/menu items should leave it blank. This does not require stability across different application releases. See [AutomationId property](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-automation-element-propids) and [dynamic ListItem IDs](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportlistitemcontroltype).

**Current path:** [Property](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.cpp#L108) always returns `gacui-<id>`. [Control allocation](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L105) and [logical-node allocation](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L299) assign nextId++. Opening the same fixed dialog after different numbers of other controls have been materialized changes those IDs without changing the dialog's identity.

- [ ] Compare a fixed dialog's controls across two application instances with different earlier window/provider discovery histories. Provide stable application identity where available and avoid presenting an unstable counter as AutomationId. RuntimeId can retain runtime allocation semantics.

### R3-05 [P2] Supported vertical tabs report Horizontal orientation

**Requirement:** Tab Orientation reports the actual horizontal/vertical arrangement. See [Tab properties](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttabcontroltype).

**Current path:** [Property](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationProvider.Windows.cpp#L125) hardcodes all GuiTab controls to Horizontal. The framework supports [TopToBottom/BottomToTop tab order](Source/Controls/Templates/GuiControlTemplates.h#L59), with corresponding [vertical arrow handling](Source/Controls/GuiContainerControls.cpp#L167).

- [ ] Use a vertical TabTemplate and verify Orientation and keyboard direction agree. Default horizontal DarkSkin tabs do not exercise this case.

### R3-06 [P2] Alignment attributes include text beyond the exclusive endpoint

**Requirement:** A range ending at the next unit's start does not inherit that next unit's attributes. See [text endpoint inclusivity](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-uiautomationtextunits#endpoint-inclusivity).

**Current path:** [Paragraph boundaries](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L322) put a first-paragraph range's end at the next paragraph's column zero. [Attribute](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L361) passes that endpoint directly to [SummarizeParagraphAlignment](Source/Resources/GuiDocument_Edit.cpp#L697), which includes end.row in its loop.

- [ ] Use left-aligned `first` followed by right-aligned `second`. Expand a range to the first Paragraph (`first\r\n`) and require Left, not Mixed. Cover FindAttribute and Format around the same paragraph boundary.

### R3-07 [P2] Uniform paragraph separators produce false Mixed attributes and Format breaks

**Requirement:** Mixed denotes varying attribute values; a Format unit encompasses text sharing its formatting. See [GetAttributeValue](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-getattributevalue) and [Format units](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-uiautomationtextunits#format).

**Current path:** For two identically styled/aligned paragraphs `a` and `b`, the snapshot is `a\r\nb`. [Separator positions](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L60) span the previous paragraph end to the next paragraph start. [Style summarization](Source/Resources/GuiDocument_Edit.cpp#L604) collects no actual characters for that separator interval and returns an empty style. [Attribute](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L350) interprets missing font fields as Mixed. [Format comparison](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L305) consequently inserts breaks despite uniform formatting.

- [ ] Require a uniform Format range across `a\r\nb` and meaningful, non-Mixed font attributes on its separator. Inspect the resulting FindAttribute spans as well; the current search stops at the artificial separator difference.

### R3-08 [P2] Wrapped supplementary characters generate an unrelated two-line rectangle

**Requirement:** GetBoundingRectangles describes the visible text in the requested range, using its text lines. See [GetBoundingRectangles](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-getboundingrectangles).

**Current path:** Snapshot mapping puts the high and low surrogate at the same renderer caret, but [GetBoundingRectangles](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationText.Windows.cpp#L472) iterates individual UTF-16 code units. For its first half it queries that same caret with opposite frontSide values. Direct2D [line lookup](Source/PlatformProviders/Windows/Direct2D/Renderers/GuiGraphicsLayoutProviderWindowsDirect2D.cpp#L958) and [caret bounds](Source/PlatformProviders/Windows/Direct2D/Renderers/GuiGraphicsLayoutProviderWindowsDirect2D.cpp#L1167) intentionally resolve those sides to different lines at a soft wrap. Their union covers the previous line's end and next line's start, followed by another rectangle for the actual character. The document-object bounds path already uses complete-character intervals.

- [ ] Force a soft wrap immediately before an emoji, obtain only that emoji's text range, and verify rectangles exclude the previous line. Also test a supplementary character away from wrapping. This conclusion is based on the renderer call chain; execute the targeted Direct2D case before changing geometry code.

### R3-09 [P2] Nonwindow layout changes can omit bounds/offscreen notifications

**Requirement:** Controls such as Button publish BoundingRectangle changes and supported IsOffscreen changes. See [Button required events](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportbuttoncontroltype).

**Current path:** [State callbacks](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L121) include GuiWindow.BoundsChanged, which handles moving a window and its descendants. They do not subscribe to ordinary composition [CachedBoundsChanged](Source/Application/GraphicsCompositions/GuiGraphicsComposition_Layout.cpp#L71). [Composition state changes](Source/Application/GraphicsCompositions/GuiGraphicsComposition.cpp#L125) request rendering; ChildCompositionUpdated is emitted for insertion/removal/reordering, not a pure bounds change. [Property-change event emission](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L362) only runs from queued Notify; initial property snapshots do not raise events.

- [ ] After callbacks have drained, move/resize an existing button's bounds composition within a fixed-size stationary window, without changing its text, focus, visibility or parent. Verify its BoundingRectangle event arrives without an unrelated action. Repeat across a clipping boundary for IsOffscreen. Keep the already-implemented window-translation path covered separately.

### R3-10 [P2] Changing logical document children does not invalidate the UIA structure

**Requirement:** Document/tree structure changes must be reported to clients. See [Document required events](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdocumentcontroltype) and [StructureChanged definition](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-event-ids).

**Current path:** [SetDocument](Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp#L1054) changes the model and publishes text/caret events. [GuiDocumentElement::SetDocument](Source/GraphicsElement/GuiGraphicsDocumentElement.cpp#L65) does not insert/remove compositions for ordinary hyperlink/image runs. [Semantic children](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L535) nevertheless change with those runs. The [TextChanged hook](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L122) calls Notify with structure=false, and [Notify](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L332) emits StructureChanged only when structure is true.

- [ ] In a fixed-size document without embedded GuiDocumentItem controls, replace plain text with the same text inside a hyperlink, or replace one image run with another. Require a structure notification for the changed child identities even if text, size and physical compositions are unchanged.

### R3-11 [P2] Clicking a document hyperlink omits its Invoked event

**Requirement:** Hyperlinks support Invoked when activated. See [Hyperlink required events](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporthyperlinkcontroltype).

**Current path:** UIA [Invoke](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationActions.Windows.cpp#L17) raises Invoked for a DocumentObject before routing the hyperlink action. Real document input takes [OnMouseUp](Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp#L675), which raises ActiveHyperlinkExecuted. The UIA [document hooks](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L259) do not subscribe to that event. The ordinary-button BeforeClicked bridge does not cover a DocumentHyperlinkRun.

- [ ] Subscribe on a semantic hyperlink in a ViewOnly document, click the link and verify Invoked identifies that hyperlink. Then invoke it through UIA and require exactly one corresponding event, avoiding duplication when the routes are unified.

### R3-12 [P2] Hosted modal-and-delete windows lose WindowClosed before notification dispatch

**Requirement:** A Window provider reports WindowClosed. See [Window required events](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportwindowcontroltype).

**Current path:** [ShowModalAndDelete](Source/Application/Controls/GuiWindowControls.cpp#L1135) schedules [native destruction](Source/Application/Controls/GuiWindowControls.cpp#L260) from the WindowReadyToClose callback. [Hosted Hide](Source/PlatformProviders/Hosted/GuiHostedWindow.cpp#L259) runs AfterClosing before closing the proxy. The ensuing WindowClosed callback posts [Notify](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L65). However, the [Windows message loop](Source/PlatformProviders/Windows/WinNativeWindow.cpp#L1995) executes native async tasks immediately after the current message, before the next posted UIA notification. [Hosted destruction](Source/PlatformProviders/Hosted/GuiHostedController.cpp#L1002) invokes NativeWindowDestroying, which [retires the root](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomation.Windows.cpp#L266). The queued notification then exits at [!IsLive](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationState.Windows.cpp#L321), dropping WindowClosed.

- [ ] Show a hosted secondary dialog through ShowModalAndDelete, subscribe to WindowClosed and close it. Require the event before disconnecting its provider. Keep the host application running; there is no separate child HWND provider to supply this notification. Test ordinary mode separately because native default-provider behavior can affect client observation.

### R3-13 [P2] Resize can make a partly visible window wholly offscreen

**Requirement:** Transform must correct the position when resizing would make the resulting window wholly outside its accessible container. See [Resize remarks](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itransformprovider-resize).

**Current path:** [Resize](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationPatterns.Windows.cpp#L306) applies positive new dimensions at the unchanged top-left, without validating the resulting rectangle. [Native SetBounds](Source/PlatformProviders/Windows/WinNativeWindow.cpp#L1220) forwards it to MoveWindow; [native bounds adjustment](Source/NativeWindow/GuiNativeWindow.cpp#L458) enforces minimum dimensions rather than screen containment.

- [ ] On a desktop with no monitor left of x=0, use an application/native placement of x=-900 and width=1200, with a minimum width below 400. UIA Resize(400,300) must retain an accessible position; the current path leaves its right edge at -500 and returns success. This setup deliberately starts partly visible and does not use the already-clamping UIA Move to create it.

### R3-14 [P3] Completely off-desktop Move targets the nearest rather than primary monitor

**Requirement:** If a transform puts an object completely beyond the combined desktop, Microsoft specifies placement on the primary monitor. See [Transform multimonitor convention](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtransform).

**Current path:** [Move](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationPatterns.Windows.cpp#L269) always chooses MONITOR_DEFAULTTONEAREST and clamps within that monitor's work area. With primary x=0..1919 and secondary x=1920..3839, moving a 400-pixel window to x=5000 places it on the secondary. It remains accessible, but misses the documented primary-monitor fallback.

- [ ] Test a target rectangle completely outside the combined desktop on a multimonitor system, and distinguish it from a valid request on a secondary monitor.

## Qualified observations and further verification

These are not included in the 14 confirmed implementation findings.

- **List-item focusability:** [IsFocusable](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L425) excludes ordinary logical Items, so SetFocus rejects them even for keyboard-navigable lists. [ListItem guidance](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportlistitemcontroltype) recommends TRUE when the container accepts keyboard input. Reconcile that recommendation with GacUI's container-held keyboard focus. This does not establish that every selected item must report HasKeyboardFocus, or that focusing an item should select it. The Tab contract expressly permits container-held focus.
- **Showcase tooltip metadata:** [DocumentEditorRibbon.xml](Test/Resources/App/FullControlTest/DocumentEditorRibbon.xml#L114) has a tooltip implemented as a textless CustomControl containing a SolidLabel element. [UiaTooltipText](Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomationSemantic.Windows.cpp#L464) reads control text, so that example's Name/owner HelpText remain empty. Supplying tooltip control Text fills both; explicit owner HelpText can fill the latter. An accessible Name on the custom tooltip content alone is not consulted by UiaTooltipText. This is an application accessibility gap under [ToolTip requirements](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttooltipcontroltype), not evidence that every arbitrary drawn composition must be inferred by the provider.
- **Geometry and range retention:** exercise bidi text, combining clusters, mixed font heights, partly clipped glyphs, DPI changes and several intervening edits. Prefix/suffix range adjustment is not an edit-history implementation; that alone does not establish a mandatory-contract failure.
- **Additional property/event paths:** verify label disposal, dynamic shortcuts, tooltip replacement, header activation, selection changes during model replacement, and shutdown ordering. Only the concrete event paths traced above are asserted as findings.
- **Pattern error edges:** fractional numeric values, partial two-axis Scroll failure, and VirtualizedItem on already-realized elements should be assessed against their exact contracts before being treated as defects.

## Contract interpretation

- Native popup owner navigation is explicitly permitted by Microsoft's [Assigning a New Parent](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-serversideprovider#assigning-a-new-parent) rules. A popup retaining its HWND host/fragment root while navigating to its logical owner is not, by itself, a violation of the general fragment-root rule. Preserve the combo/menu ownership behavior.
- The current default grid's visualizers/editors belong beneath their logical cell. Nested application radios in that editor reveal a separate group-contract gap, not a reason to undo cell ownership.
- Typical-tree diagrams are not blanket prohibitions on extra wrappers. Conditional patterns are required when the corresponding behavior exists; R3-03 traces that condition to actual built-in spatial navigation.
- Missing Text2, TextRange2, TextEdit, TextChild, Dock or drag/drop interfaces are not automatically missing basic Text/Window requirements. Likewise, standard LocalizedControlType defaults can be supplied by UIA.
- Microsoft permits a degenerate GetVisibleRanges result when no visible text matches. It also permits richer virtualized-item providers. These behaviors are not counted as gaps.
- The MenuItem article contains prose inconsistent with the current [Menu control contract](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportmenucontroltype), which requires Menu in content view. The implementation follows the direct Menu contract; no failure is asserted from the conflicting prose alone.

## Verification and change scope

- [x] Trace current implementation paths and compare them with official Microsoft contracts.
- [x] Review existing test source without treating unexecuted tests as evidence of passing behavior.
- [x] Keep runtime scenarios explicitly unexecuted in this review.
- [ ] Implement and execute targeted regressions for R3-01 through R3-14 in subsequent work.

This request changes only this review document. Source, tests, existing reports and the parallel task's files are outside its commit scope.
