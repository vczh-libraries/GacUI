# UI Automation

# Goal

- Expose GacUI's semantic control/composition tree through Windows UI Automation (UIA), including control behavior, properties, events, text, and virtualized items.
- Preserve the architectural requirements in [1.4.1.3.md](./1.4.1.3.md), which take precedence over implementation suggestions in this analysis:
  - Store an internal object on each control to record whether its necessary events have been hooked. Report destruction and connection/disconnection from a top-level window. Hook once; do not unhook on detachment or install duplicate callbacks on reattachment. After the destruction report, or while disconnected, ignore ordinary callbacks. Lifetime/topology reports must still update membership and invalidate stale providers.
  - Provider wrappers may be allocated lazily, but event hookup and lifecycle tracking must not depend on clients first enumerating children. A detached control's retained hook object and its membership in the exposed tree are separate concerns.
  - Determine the actual control type with a most-derived-first `dynamic_cast` dispatch. Theme changes invalidate relevant presentation information; a theme name alone does not determine semantics.
  - An internal property on a composition extends the automation tree and replaces the associated control's default implementation when one exists. Apply this precedence to list/grid item templates: the active editor replaces the visualizer's exposure for the cell, while retaining the cell's required grid/header relationships. Do not expose duplicate visualizer/editor values.
  - Carry semantic automation information from remote-protocol core to renderer and route actions back to the owning core. Do not derive control semantics solely from renderer drawing elements or transport COM pointers.
  - Handle Windows `WM_GETOBJECT` through the exposed Windows message-handler interface; do not add UIA handling to an `INativeWindow` implementation. Use `INativeControllerListener` to discover top-level windows and handle Hosted Mode's one actual HWND and its logical windows.
- Supply Windows provider support, a debug tool, and skill support for operating applications. This document records contracts and constraints; it does not settle the remote protocol or internal-property API design.

https://learn.microsoft.com/en-us/windows/win32/winauto/entry-uiauto-win32#developer-audience
https://learn.microsoft.com/en-us/windows/win32/winauto/handling-the-wm-getobject-message

## Instructions

You goal is to fill `Plan` and `Details` in this document.
GacUI is going to support Windows UI Automation, rather than implementing a Microsoft Active Accessibility (MSAA) bridge.
The first job is to fill the `Plan` section. It includes:
- First item: You need to get the link from MSDN (aka learn.microsoft.com) about this topic, write it down under the bullet item.
- Second item: There maybe multiple MSDN pages associated with it, write them down under the bullet item.
- Third item: Inventory all UIA control types and pattern interfaces with MSDN links, required/conditional/forbidden behavior, properties and events. Control types are semantic identifiers, not derived COM interfaces. Distinguish actual interface inheritance from patterns that must be implemented together.
- Fourth item: Inventory every `vl::presentation::theme::ThemeName` from `Source/Application/Controls/GuiThemeManager.h`, grouped by template type. These enum values have no inheritance. Also map actual control classes and semantic items without a unique theme name.
- Other items: These controls are super complex, you must provide deeply details for them.

Now you got check lists of topics to research. Follow these instructions until you checks every item:
- Pick the first incomplete item in `Plan`, change from "- [ ]" to "- [+]" meaning you are working on it. You must immediately save the file to disk.
  - If the first incomplete item is "- [ ]", you are on a fresh start with this item. You can start a sub agent to work on it.
  - If the first incomplete item is "- [+]", it means you stopped at the middle, find out the corresponding title under `Details` and continue.
- Add it as a "###" title under `Details` section.
- Find out enough information and fill the title. The criteria of successful is that, one can follow the idea under this title, with helps from related mentioned MSDN links, and finish the coding work. You must immediatelly save the file to disk.
- Mark the item in `Plan` from "- [+]" to "- [*]" meaning you have finished this item.

At the moment, you don't need to mention in details about how to implement an UI Automation control interface using the GacUI control class. The more important work is to help one understand what a certain UI Automation control interface expectes the implementation to do. For examples, how many properties/events/actions can this UI Automation control interface offer, which in them are must have (aka not optional). You don't need to repeat in a child interface what is already in the parent interface.

## Plan

- [*] How WM_GETOBJECT works
  - Primary: https://learn.microsoft.com/en-us/windows/win32/winauto/wm-getobject
  - Related:
    - https://learn.microsoft.com/en-us/windows/win32/winauto/handling-the-wm-getobject-message
    - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiareturnrawelementprovider
    - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiahostproviderfromhwnd
- [*] How does UI Automation properties, events and actions work?
  - Primary: https://learn.microsoft.com/en-us/windows/win32/winauto/entry-uiauto-win32
  - Properties:
    - Overview: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-propertiesoverview
    - Property IDs: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-automation-element-propids
    - Not supported sentinel: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiagetreservednotsupportedvalue
  - Events:
    - Overview: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-eventsoverview
    - Event IDs: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-event-ids
    - Raise events:
      - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaraiseautomationevent
      - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaraiseautomationpropertychangedevent
      - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaraisestructurechangedevent
      - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaclientsarelistening
  - Actions (control patterns):
    - Overview: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controlpatternsoverview
    - Pattern IDs: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controlpattern-ids
- [*] UI Automation supported interfaces for controls
  - Primary: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-providerportal
  - Related:
    - Provider overview: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-providersoverview
    - Supporting control types: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportinguiautocontroltypes
    - Implementing patterns: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinguiautocontrolpatterns
  - [*] Provider interfaces (tree/navigation)
    - [*] `IRawElementProviderSimple`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementprovidersimple
      - [*] `IRawElementProviderSimple2` (optional context menu): https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementprovidersimple2
        - [*] `IRawElementProviderSimple3` (optional metadata): https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementprovidersimple3
    - [*] `IRawElementProviderFragment`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementproviderfragment
    - [*] `IRawElementProviderFragmentRoot`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementproviderfragmentroot
    - [*] `IRawElementProviderAdviseEvents` (optional): https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementprovideradviseevents
    - [*] `IRawElementProviderHwndOverride` (optional): https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementproviderhwndoverride
  - [*] Pattern provider interfaces (control behavior)
    - [*] Custom navigation: `ICustomNavigationProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-icustomnavigationprovider
    - [*] Invoke: `IInvokeProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iinvokeprovider
    - [*] Toggle: `IToggleProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itoggleprovider
    - [*] Selection: `ISelectionProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iselectionprovider
      - [*] Selection 2: `ISelectionProvider2` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iselectionprovider2
    - [*] Selection item: `ISelectionItemProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iselectionitemprovider
    - [*] Expand/collapse: `IExpandCollapseProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iexpandcollapseprovider
    - [*] Value: `IValueProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-ivalueprovider
    - [*] Range value: `IRangeValueProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irangevalueprovider
    - [*] Scroll: `IScrollProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iscrollprovider
    - [*] Scroll item: `IScrollItemProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iscrollitemprovider
    - [*] Grid: `IGridProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-igridprovider
    - [*] Grid item: `IGridItemProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-igriditemprovider
    - [*] Table: `ITableProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itableprovider
    - [*] Table item: `ITableItemProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itableitemprovider
    - [*] Text: `ITextProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextprovider
      - [*] Text 2: `ITextProvider2` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextprovider2
      - [*] Text edit: `ITextEditProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itexteditprovider
    - [*] Item container: `IItemContainerProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iitemcontainerprovider
    - [*] Virtualized item: `IVirtualizedItemProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-ivirtualizeditemprovider
    - [*] Window: `IWindowProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iwindowprovider
    - [*] Transform: `ITransformProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itransformprovider
    - [*] Transform 2: `ITransformProvider2` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itransformprovider2
    - [*] Dock: `IDockProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-idockprovider
    - [*] Multiple view: `IMultipleViewProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-imultipleviewprovider
    - [*] Object model: `IObjectModelProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iobjectmodelprovider
    - [*] Styles: `IStylesProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-istylesprovider
    - [*] Spreadsheet: `ISpreadsheetProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-ispreadsheetprovider
    - [*] Spreadsheet item: `ISpreadsheetItemProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-ispreadsheetitemprovider
    - [*] Drag: `IDragProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-idragprovider
    - [*] Drop target: `IDropTargetProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-idroptargetprovider
    - [*] Synchronized input: `ISynchronizedInputProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-isynchronizedinputprovider
    - [*] Annotation: `IAnnotationProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iannotationprovider
    - [*] Text child: `ITextChildProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextchildprovider
    - [*] Legacy bridge (reference only, outside the UIA-only implementation goal): `ILegacyIAccessibleProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-ilegacyiaccessibleprovider
  - [*] Text range support interfaces (returned by Text, not separately requested element patterns)
    - [*] `ITextRangeProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextrangeprovider
      - [*] `ITextRangeProvider2`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextrangeprovider2
- [*] Complete UIA control-type contract inventory (all 41 standard roles, with per-role references below)
  - Primary: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controltype-ids
- [*] Template groups and actual controls for `vl::presentation::theme::ThemeName`
  - Primary: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportinguiautocontroltypes
  - Related:
    - Control type IDs: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controltype-ids
    - Pattern overview: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controlpatternsoverview
  - [*] `WindowTemplate`
    - [*] `Window` (special selector resolved to SystemFrameWindow or CustomFrameWindow)
    - [*] `SystemFrameWindow`
    - [*] `CustomFrameWindow`
    - [*] `Tooltip`
  - [*] `ControlTemplate`
    - [*] `Unknown` (no default theme; actual control/explicit template determines behavior)
    - [*] `CustomControl`
    - [*] `GroupBox`
    - [*] `MenuBar`
    - [*] `MenuSplitter`
    - [*] `ToolstripToolBar`
    - [*] `ToolstripToolBarInMenu`
    - [*] `ToolstripSplitter`
    - [*] `ToolstripSplitterInMenu`
    - [*] `RibbonSplitter`
    - [*] `RibbonToolstripHeader`
  - [*] `LabelTemplate`
    - [*] `Label`
    - [*] `ShortcutKey`
  - [*] `ScrollViewTemplate`
    - [*] `ScrollView`
  - [*] `TabTemplate`
    - [*] `Tab`
  - [*] `ComboBoxTemplate`
    - [*] `ComboBox`
  - [*] `DocumentViewerTemplate`
    - [*] `DocumentViewer`
    - [*] `MultilineTextBox`
  - [*] `DocumentLabelTemplate`
    - [*] `DocumentLabel`
    - [*] `DocumentTextBox`
    - [*] `SinglelineTextBox`
  - [*] `ListViewTemplate`
    - [*] `ListView`
  - [*] `TreeViewTemplate`
    - [*] `TreeView`
  - [*] `TextListTemplate`
    - [*] `TextList`
    - [*] `RibbonGalleryItemList`
  - [*] `SelectableButtonTemplate`
    - [*] `ListItemBackground`
    - [*] `TreeItemExpander`
    - [*] `CheckTextListItem`
    - [*] `RadioTextListItem`
    - [*] `CheckBox`
    - [*] `RadioButton`
  - [*] `MenuTemplate`
    - [*] `Menu`
  - [*] `ToolstripButtonTemplate`
    - [*] `MenuBarButton`
    - [*] `MenuItemButton`
    - [*] `ToolstripButton`
    - [*] `ToolstripDropdownButton`
    - [*] `ToolstripSplitButton`
    - [*] `RibbonSmallButton`
    - [*] `RibbonSmallDropdownButton`
    - [*] `RibbonSmallSplitButton`
    - [*] `RibbonLargeButton`
    - [*] `RibbonLargeDropdownButton`
    - [*] `RibbonLargeSplitButton`
  - [*] Ribbon-specific templates
    - [*] `RibbonTabTemplate` / `RibbonTab`
    - [*] `RibbonGroupTemplate` / `RibbonGroup`
    - [*] `RibbonGroupMenuTemplate` / `RibbonGroupMenu`
    - [*] `RibbonIconLabelTemplate` / `RibbonIconLabel`
    - [*] `RibbonIconLabelTemplate` / `RibbonSmallIconLabel`
    - [*] `RibbonButtonsTemplate` / `RibbonButtons`
    - [*] `RibbonToolstripsTemplate` / `RibbonToolstrips`
    - [*] `RibbonGalleryTemplate` / `RibbonGallery`
    - [*] `RibbonToolstripMenuTemplate` / `RibbonToolstripMenu`
    - [*] `RibbonGalleryListTemplate` / `RibbonGalleryList`
  - [*] `ButtonTemplate`
    - [*] `Button`
  - [*] `DatePickerTemplate`
    - [*] `DatePicker`
  - [*] `DateComboBoxTemplate`
    - [*] `DateComboBox`
  - [*] `ScrollTemplate`
    - [*] `HScroll`
    - [*] `VScroll`
    - [*] `HTracker`
    - [*] `VTracker`
    - [*] `ProgressBar`
- [*] Details about container control with items (combo, list, tab, etc) and virtualization
  - Primary: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinguiautocontrolpatterns
  - Related:
    - `IVirtualizedItemProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-ivirtualizeditemprovider
    - `IScrollItemProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iscrollitemprovider
    - `IItemContainerProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iitemcontainerprovider
- [*] Details about list view detail view (grid/table semantics)
  - Primary: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-igridprovider
  - Related:
    - `IGridItemProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-igriditemprovider
    - `ITableProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itableprovider
    - `ITableItemProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itableitemprovider
- [*] Details about plain text and rich text controls (Text pattern)
  - Primary: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtextandtextrange
  - Related:
    - `ITextProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextprovider
    - `ITextRangeProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextrangeprovider
    - `ITextEditProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itexteditprovider

## Details

### How WM_GETOBJECT works

References:
- WM_GETOBJECT message: https://learn.microsoft.com/en-us/windows/win32/winauto/wm-getobject
- Handling the message: https://learn.microsoft.com/en-us/windows/win32/winauto/handling-the-wm-getobject-message
- Returning a provider (`UiaReturnRawElementProvider`): https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiareturnrawelementprovider

What the message means:
- `WM_GETOBJECT` is sent by accessibility clients to obtain an accessibility object from a window.
- The object identifier is in `lParam` (`dwObjId` in the message documentation). Compare it using the documented signed 32-bit identifier convention; on 64-bit Windows, do not compare an incorrectly zero-extended negative identifier. For `UiaRootObjectId`, return `UiaReturnRawElementProvider(hwnd, wParam, lParam, rootProvider)` as the message result. Preserve both original parameters; the result is not a cast COM pointer.
- Do not return an incompletely initialized or destroyed provider. Requests for other object identifiers continue through normal message handling.

Minimum returned object:
- For UI Automation, the returned object must implement `IRawElementProviderSimple`:
  - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementprovidersimple

Fragment tree model (needed for real control trees):
- If you want a navigable element tree (parent/child/siblings) and hit-testing/focus within the window, implement a fragment:
  - Root: `IRawElementProviderFragmentRoot` (hit-test + focus entry points)
    - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementproviderfragmentroot
  - Children: `IRawElementProviderFragment` (navigation, runtime id, bounding rect, focus)
    - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementproviderfragment

Composing with HWND hosting:
- For the root actually hosted by an HWND, return its default host provider from `HostRawElementProvider`; ordinary windowless descendants return null:
  - `UiaHostProviderFromHwnd`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiahostproviderfromhwnd
- `IRawElementProviderHwndOverride` repositions HWND-based elements within a fragment (for example, controls hosted in rebar bands). It is not required merely because GacUI controls are windowless:
  - `IRawElementProviderHwndOverride`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementproviderhwndoverride

What to get right (so clients can actually use it):
- The root provider must represent the correct top-level surface (usually the client-area root of the window).
- Bounding rectangles and hit-test points use physical screen coordinates. Translate logical GacUI coordinates, DPI scaling, Hosted Mode offsets, and the host client's screen origin exactly once. Offscreen state does not change this coordinate system.
- Focus must be coherent: `GetFocus` returns the focused element; `SetFocus` moves focus to what the user perceives as the focusable target.
- When the control tree changes, raise structure changed events so clients can update their cached model.

GacUI integration points:
- Use `vl::presentation::windows::IWindowsForm::InstallMessageHandler` with `vl::presentation::windows::INativeMessageHandler`, defined in `Source/PlatformProviders/Windows/WinNativeWindow.h`. The current dispatcher in `WinNativeWindow.cpp` lets `AfterHandle` set both `skip` and `result`; setting `skip` in `BeforeHandle` would bypass the phase that can set the result.
- Observe native-window creation/destruction with `INativeControllerListener` (`Source/NativeWindow/GuiNativeWindow.h`) and account for windows already present at service startup. For Hosted Mode, obtain the actual host through `IGuiHostedApplication` (`Source/PlatformProviders/Hosted/GuiHostedApplication.h`), rather than treating every virtual window as an HWND.
- Keep COM object lifetime separate from control lifetime. A client can retain a provider after its control is gone; invalidate its target and return `UIA_E_ELEMENTNOTAVAILABLE` rather than dereferencing deleted controls. Use the documented [provider disconnection APIs](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-serversideprovider#disconnecting-providers) when retiring Windows providers. This is separate from the milestone's persistent control-event hookups.
- Serialize access to GacUI state on its owning UI thread; provider COM calls must not race control mutation or deadlock a UI thread waiting for a remote response. Select `ProviderOptions` according to the implemented threading model, not merely to silence marshaling failures. See [ProviderOptions](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/ne-uiautomationcore-provideroptions).

### How does UI Automation properties, events and actions work?

References:
- Entry: https://learn.microsoft.com/en-us/windows/win32/winauto/entry-uiauto-win32
- Properties:
  - Overview: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-propertiesoverview
  - Property IDs: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-automation-element-propids
- Events:
  - Overview: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-eventsoverview
  - Event IDs: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-event-ids
- Control patterns:
  - Overview: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controlpatternsoverview
  - Pattern IDs: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controlpattern-ids

Properties:
- Clients query properties by `PROPERTYID`.
- Providers answer via `IRawElementProviderSimple::GetPropertyValue`.
- Initialize the output variant. For a property this provider does not supply, return `S_OK` with `VT_EMPTY`, allowing another provider/default to supply it. Use the reserved not-supported `VT_UNKNOWN` only when deliberately suppressing fallback. Text-attribute queries have their own sentinel contract. See [GetPropertyValue](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-irawelementprovidersimple-getpropertyvalue).
- Supply the semantic ControlType, Name/label relationships, control/content-view flags, enabled/offscreen state, keyboard-focus properties, and geometry appropriate to the role. Use LocalizedControlType for a meaningful localized custom role; preserve the standard role names for standard controls. AutomationId is recommended, not universally mandatory; when supplied it is language-independent and unique among siblings. It is distinct from the runtime ID of a live element. Expose HelpText, AccessKey, AcceleratorKey, ItemType/ItemStatus, orientation, and other properties when they describe actual UI information. The property-ID reference above specifies types and defaults.

Actions and “behavior”:
- UIA “actions” are modeled as control patterns (Invoke/Toggle/Selection/Scroll/Text/...).
- Clients request a pattern by `PATTERNID`; providers return an object implementing the matching provider interface from `GetPatternProvider`.
- An unsupported pattern returns `S_OK` and a null output pointer. A supported interface must implement every member of that contract, including state-appropriate failures; do not advertise a pattern with unimplemented required methods. See [GetPatternProvider](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-irawelementprovidersimple-getpatternprovider).
- A pattern is not a visual part; it is a contract for behavior. If the behavior exists, expose the pattern even if the control looks custom.

Events:
- Raise the applicable control-type and pattern events for user, application, and automation-triggered changes, after the corresponding state is queryable. Required events mean support for the documented trigger, not unconditional emission on every frame.
- Common raising functions:
  - `UiaRaiseAutomationEvent`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaraiseautomationevent
  - `UiaRaiseAutomationPropertyChangedEvent`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaraiseautomationpropertychangedevent
  - `UiaRaiseStructureChangedEvent`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaraisestructurechangedevent
- Use this to avoid expensive work when nobody listens:
  - `UiaClientsAreListening`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaclientsarelistening
- Listener checks only optimize event construction/emission; they must not disable the control lifecycle tracking required by the milestone. `IRawElementProviderAdviseEvents` subscriptions are counted per event/property rather than treated as one global Boolean.

“Must have” patterns by common control semantics (examples):
- Push button: `ControlType.Button` + `IInvokeProvider`.
- Check box: `ControlType.CheckBox` + `IToggleProvider`.
- Radio button: `ControlType.RadioButton` + `ISelectionItemProvider` (in a selection container).
- Slider/progress: RangeValue when the represented value is numeric; progress is read-only and indeterminate progress must not invent a value. ScrollBar has a separate conditional RangeValue contract; it never supplies Scroll itself.
- Tab requires Selection; List and Tree require it when they maintain selection. Scrolling and item patterns follow the exact control-type contracts below, rather than a blanket rule for all containers.

### UI Automation supported interfaces for controls

References:
- Provider portal: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-providerportal
- Providers overview: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-providersoverview
- Supporting control types: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportinguiautocontroltypes
- Implementing patterns: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinguiautocontrolpatterns

Provider interfaces (tree shape):
- `IRawElementProviderSimple` (required on every element, not a C++ base of Fragment):
  - Exposes properties and patterns and optionally a host provider for HWND.
  - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementprovidersimple
- `IRawElementProviderFragment` (element in a tree):
  - `Navigate`, `GetRuntimeId`, `BoundingRectangle`, `GetEmbeddedFragmentRoots`, `SetFocus`, and `FragmentRoot`. Runtime IDs identify logical elements, not recycled item-template addresses. An HWND root normally returns null for `GetRuntimeId` so UIA supplies its identity; windowless descendants use `UiaAppendRuntimeId` plus fragment-unique values.
  - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementproviderfragment
- `IRawElementProviderFragmentRoot` (tree root for hit-test and focus):
  - `ElementProviderFromPoint` and `GetFocus`.
  - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementproviderfragmentroot

Pattern interfaces (behavior):
- Use the minimal set that describes the control's user-facing behavior; avoid exposing patterns for internal visual parts.
- Typical mapping:
  - Command-like: `IInvokeProvider`
  - On/off: `IToggleProvider`
  - Selectable: `ISelectionProvider` / `ISelectionItemProvider`
  - Popup / hierarchy: `IExpandCollapseProvider`
  - Edit and Document: required `ITextProvider`; `IValueProvider` where the control-type contract permits and value replacement is meaningful
  - Ranged: `IRangeValueProvider`
  - Scroll containers: `IScrollProvider` (and `IScrollItemProvider` on items)
  - Virtualized items: `IVirtualizedItemProvider`
  - Tables/grids: `IGridProvider`/`IGridItemProvider` and optionally `ITableProvider`/`ITableItemProvider`
  - Text: `ITextProvider` + `ITextRangeProvider`; TextEdit adds composition/autocorrection observation, not general editing methods

Optional-but-useful interfaces (only when needed):
- `IRawElementProviderAdviseEvents` (UIA tells provider what events are subscribed): https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-irawelementprovideradviseevents
- `ICustomNavigationProvider` (non-standard navigation models): https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-icustomnavigationprovider

#### Interface relationships and required pattern members

The provider interfaces linked in Plan are the native COM contracts. Simple, Fragment, and FragmentRoot are separate C++ interfaces; a fragment root implements all three. Simple2 extends Simple, and Simple3 extends Simple2. Text2 and TextEdit each extend Text; TextRange2 extends TextRange; Selection2 extends Selection. Table/Grid and TableItem/GridItem are semantic dependencies implemented as separate interfaces. Windows SDK 10.0.26100.0 declares Transform2 as extending Transform; its Learn page's `IUnknown` inheritance sentence is inconsistent with that header, so use the SDK declaration for the actual ABI. Versioned patterns must be exposed under their documented pattern IDs and deployment versions.

Every advertised pattern implements all members listed below. Names without parentheses denote properties; `get_` COM spelling is omitted for readability. The individual interface links in Plan define signatures and return types. For trigger/error details, consult [Implementing control patterns](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinguiautocontrolpatterns) and its per-pattern pages; the following table records the implementation scope rather than replacing those contracts.

| Pattern | Required methods and properties | Behavior and notifications |
|---|---|---|
| Invoke | `Invoke()` | Perform the action without waiting for a modal interaction to finish; raise Invoked. Do not duplicate an action already represented by another pattern. |
| Toggle | `Toggle()`, `ToggleState` | Cycle supported states in the documented order and raise ToggleState changes; there is no SetToggleState method. |
| ExpandCollapse | `Expand()`, `Collapse()`, `ExpandCollapseState` | Include LeafNode/PartiallyExpanded when meaningful; report state and actual subtree changes. |
| Selection | `GetSelection()`, `CanSelectMultiple`, `IsSelectionRequired` | Return logical selected elements; raise selection events/invalidation under the selection conventions below. |
| Selection2 | `FirstSelectedItem`, `LastSelectedItem`, `CurrentSelectedItem`, `ItemCount`, plus Selection | ItemCount counts selected items, not every container item. Windows 10 version 1709 or later. |
| SelectionItem | `Select()`, `AddToSelection()`, `RemoveFromSelection()`, `IsSelected`, `SelectionContainer` | Enforce the container's rules; raise selected/added/removed notifications according to the result. |
| Value | `SetValue()`, `Value`, `IsReadOnly` | Whole-value assignment where supported; report Value/IsReadOnly changes and reject invalid/read-only operations. |
| RangeValue | `SetValue()`, `Value`, `IsReadOnly`, `Minimum`, `Maximum`, `SmallChange`, `LargeChange` | Validate numeric bounds; report property changes. Unsupported change increments use the documented NaN convention, not made-up steps. |
| Scroll | `Scroll()`, `SetScrollPercent()`, both axes' `ScrollPercent`, `ViewSize`, and `Scrollable` properties | Report changes to all six properties; use NoScroll and percent conventions below. |
| ScrollItem | `ScrollIntoView()` | Reveal the item without implicitly selecting it. No dedicated pattern event. |
| Grid | `GetItem()`, `RowCount`, `ColumnCount` | Zero-based row/column addressing; report changed counts/structure/layout as applicable. |
| GridItem | `Row`, `Column`, `RowSpan`, `ColumnSpan`, `ContainingGrid` | Maintain correct coordinates, positive spans, and containing-grid identity. |
| Table | `GetRowHeaders()`, `GetColumnHeaders()`, `RowOrColumnMajor`, plus separate Grid | Header providers describe the actual table and its traversal order. |
| TableItem | `GetRowHeaderItems()`, `GetColumnHeaderItems()`, plus separate GridItem | Return this cell's associated header providers. |
| MultipleView | `GetViewName()`, `SetCurrentView()`, `GetSupportedViews()`, `CurrentView` | Stable view IDs, localized names, CurrentView/SupportedViews change events. Do not equate arbitrary themes with alternate data views. |
| Window | `SetVisualState()`, `Close()`, `WaitForInputIdle()`, `CanMaximize`, `CanMinimize`, `IsModal`, `WindowVisualState`, `WindowInteractionState`, `IsTopmost` | Reflect actual window capabilities; support WindowOpened/WindowClosed and applicable property changes. WaitForInputIdle has a bounded timeout. |
| Transform | `Move()`, `Resize()`, `Rotate()`, `CanMove`, `CanResize`, `CanRotate` | Capability flags determine valid operations; report resulting bounds/state changes. |
| Transform2 | `Zoom()`, `ZoomByUnit()`, `CanZoom`, `ZoomLevel`, `ZoomMinimum`, `ZoomMaximum`, plus Transform | Viewport zoom capability and bounds, with property changes. Not required just because the OS uses DPI scaling. |
| Dock | `SetDockPosition()`, `DockPosition` | Real docking behavior and DockPosition changes. |
| ItemContainer | `FindItemByProperty()` | Search/enumerate logical items, including realizable placeholders; no dedicated event. |
| VirtualizedItem | `Realize()` | Make the item available; expose consequent actual tree changes. |
| Text / Text2 / TextRange / TextRange2 / TextEdit | All base and extension members described in the text sections | TextChanged and TextSelectionChanged are ordinary text events; TextEdit has its own composition/autocorrection events. |
| TextChild | `TextContainer`, `TextRange` | Connect an embedded element to its nearest Text ancestor and encompassing range. |
| CustomNavigation | `Navigate()` | Expose nonstandard semantic navigation under CustomNavigation's pattern ID; do not replace ordinary Fragment navigation. |
| ObjectModel | `GetUnderlyingObjectModel()` | Optional COM-accessible document model; ordinary GacUI C++ pointers are not a marshaled object model. |
| Annotation | `AnnotationTypeId`, `AnnotationTypeName`, `Author`, `DateTime`, `Target` | Optional semantic annotation metadata, not arbitrary tooltip text. |
| Styles | `StyleId`, `StyleName`, `FillColor`, `FillPatternStyle`, `Shape`, `FillPatternColor`, `ExtendedProperties` | Optional document/shape styling, not a direct exposure of GacUI ThemeName. |
| Spreadsheet | `GetItemByName()` | Optional spreadsheet cell addressing by name, not required for every data grid. Its cells must implement SpreadsheetItem; Grid is a recommended companion pattern. |
| SpreadsheetItem | `Formula`, `GetAnnotationObjects()`, `GetAnnotationTypes()` | Spreadsheet formula/annotation semantics; GridItem is a recommended companion pattern. |
| Drag | `IsGrabbed`, `DropEffect`, `DropEffects`, `GetGrabbedItems()` | Observe semantic drag operations; DragStart, DragCancel, DragComplete and property notifications. Does not provide a generic StartDrag command. |
| DropTarget | `DropTargetEffect`, `DropTargetEffects` | Observe target effects and DragEnter/DragLeave/Dropped events. Does not provide a generic Drop command. |
| SynchronizedInput | `StartListening()`, `Cancel()` | Observe delivery/discard of the requested input type and report InputReachedTarget, InputReachedOtherElement, or InputDiscarded. It does not inject input. |
| LegacyIAccessible | Reference only: `Select()`, `DoDefaultAction()`, `SetValue()`, `GetIAccessible()`, `GetSelection()` and its MSAA properties | Outside this UIA-only implementation; do not require an MSAA object from GacUI controls. |

For optional provider extensions, Simple2 adds `ShowContextMenu`; Simple3 adds `GetMetadataValue`. AdviseEvents supplies `AdviseEventAdded`/`AdviseEventRemoved`; HwndOverride supplies `GetOverrideProviderForHwnd`. Optional COM support does not mean optional members once that interface is advertised.

### Complete UIA control-type contract inventory

This inventories all 41 standard control types in the [control type identifier list](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controltype-ids). Control types are semantic identifiers, not COM interfaces or subclasses of one another. The GacUI mapping below determines which roles occur in the toolkit; this inventory does not require inventing controls that GacUI does not provide.

Pattern names below refer to their matching provider interfaces, for example `Invoke` means `IInvokeProvider`. **Required** applies whenever that control type is exposed. **Conditional** applies when the stated behavior exists, rather than being an implementation preference. An optional pattern is identified explicitly. A row with no required patterns still needs the element properties, navigation, and events appropriate to its role. Table and TableItem additionally require Grid and GridItem respectively.

| Control type and Microsoft contract | Required, conditional, and forbidden patterns |
| --- | --- |
| [AppBar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportappbarcontroltype) | No required patterns on the container. Commands belong to its children. |
| [Button](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportbuttoncontroltype) | Invoke for a command **or** Toggle for a stateful button, **not both**. An expansion button inside a SplitButton can use ExpandCollapse instead. |
| [Calendar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcalendarcontroltype) | **Grid + Table required**. Selection for selectable dates; Scroll recommended for paging. **No Value** for setting a calendar date: expose date selection through its items. |
| [CheckBox](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcheckboxcontroltype) | **Toggle required**. Follow the supported binary or three-state cycle. |
| [ComboBox](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcomboboxcontroltype) | **ExpandCollapse required**. Selection conditional, delegated to the list when feasible. Value required for arbitrary text input. **Never Scroll on the ComboBox**; scrolling belongs to its visible list. |
| [Custom](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controltype-ids) | No predefined pattern set. Use when no standard role describes the element; applicable standard patterns still describe its behavior. Custom does not itself require a custom pattern. |
| [DataGrid](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdatagridcontroltype) | **Grid required**. Table when headers exist; Selection when selectable; Scroll when scrollable. Items support GridItem, TableItem when headers exist, and SelectionItem/ScrollItem when applicable. |
| [DataItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdataitemcontroltype) | Conditional ExpandCollapse, GridItem, ScrollItem, SelectionItem, TableItem, Toggle, and Value according to behavior. Value is for editable primary text. Independent commands can expose Invoke. |
| [Document](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdocumentcontroltype) | **Text required**. Scroll for scrollable content. Value conditional for supported whole-value text entry. Appropriate for substantial or rich document content. |
| [Edit](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporteditcontroltype) | **Text required**. Value required for string input; RangeValue for numeric-range input. Intended for small amounts of plain text rather than a rich document. |
| [Group](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportgroupcontroltype) | ExpandCollapse when the group itself shows or hides information. Additional patterns depend on actual container behavior. |
| [Header](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportheadercontroltype) | Transform when resizable. This is a container of HeaderItem labels for rows or columns, not a generic section heading. |
| [HeaderItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportheaderitemcontroltype) | Invoke when clicking sorts data; Transform when resizable. |
| [Hyperlink](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporthyperlinkcontroltype) | **Invoke required**. Value conditional for a user-meaningful target such as a URL. Apply the role to the actual link, not its enclosing document or image. |
| [Image](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportimagecontroltype) | GridItem/TableItem conditional on the containing grid/table. **Never Invoke or SelectionItem** on Image; the actionable or selectable owner uses an appropriate role. |
| [List](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportlistcontroltype) | Selection when items maintain selection; Scroll when scrollable; Grid for spatial item navigation; MultipleView when multiple views are supported. **Never Table**: use DataGrid if Table is needed. |
| [ListItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportlistitemcontroltype) | Conditional SelectionItem, ScrollItem, GridItem, and ExpandCollapse. Invoke for a command distinct from selection; Toggle for checking distinct from selection; Value for editable item text. |
| [Menu](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportmenucontroltype) | No required patterns on the container. Items and embedded controls expose their own behavior. |
| [MenuBar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportmenubarcontroltype) | Conditional ExpandCollapse, Dock, and Transform if those behaviors exist. |
| [MenuItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportmenuitemcontroltype) | ExpandCollapse for submenus; Invoke for commands; Toggle for on/off options; SelectionItem for selecting among menu choices. A submenu-only item need not invent a command. |
| [Pane](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportpanecontroltype) | Conditional Scroll, Dock, and Transform. **Never Window**: use the Window role for that contract. Layout-only objects should not become Pane controls. |
| [ProgressBar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportprogressbarcontroltype) | RangeValue for numeric progress; Value for textual progress. Both are read-only; numeric SmallChange/LargeChange are NaN. Do not invent numeric completion for indeterminate progress. |
| [RadioButton](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportradiobuttoncontroltype) | **SelectionItem required**, including a meaningful SelectionContainer. **Never Toggle**. |
| [ScrollBar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportscrollbarcontroltype) | RangeValue required only when the owning container does not expose Scroll; a mouse-only scrollbar may expose no patterns. **Never Scroll on the scrollbar**. A standalone value selector uses Slider. |
| [SemanticZoom](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportsemanticzoomcontroltype) | Toggle when classification levels can be switched: Off means flat/zoomed in; On means grouped/zoomed out. The role describes the content container, not a visible zoom button. |
| [Separator](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportseparatorcontroltype) | No required patterns. A movable splitter uses **Thumb**, not Separator. |
| [Slider](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportslidercontroltype) | RangeValue for a numeric range; Selection and Value conditional for discrete choices. Selection exposes child ListItems. |
| [Spinner](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportspinnercontroltype) | Numeric spinners can expose RangeValue; discrete values/options can expose Value. Selection is required for a list of selectable choices and is single-selection. |
| [SplitButton](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportsplitbuttoncontroltype) | **Invoke + ExpandCollapse required**. Must provide both a default command and expansion behavior. A dropdown-only button is not a SplitButton. |
| [StatusBar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportstatusbarcontroltype) | Grid optional for addressing separate status fields. Children carry their own semantics. |
| [Tab](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttabcontroltype) | **Selection required**, with CanSelectMultiple=false and IsSelectionRequired=true. Scroll conditional when tab headers can scroll. |
| [TabItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttabitemcontroltype) | **SelectionItem required; never Invoke**. Its page may be a child Pane or referenced through ControllerFor. |
| [Table](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttablecontroltype) | **Grid + Table required**. Inner cell objects support GridItem + TableItem. The container itself needs item patterns only when nested in another table. |
| [Text](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttextcontroltype) | Text optional/recommended, particularly for rich attributes. GridItem + TableItem when a table cell. **Never Value**. |
| [Thumb](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportthumbcontroltype) | **Transform required**, primarily Move. Mouse dragging/resizing does not imply the Drag or DropTarget patterns. |
| [TitleBar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttitlebarcontroltype) | No required patterns. Window behavior is on the parent Window; title text is exposed through the parent's Name. |
| [ToolBar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttoolbarcontroltype) | Conditional Dock, ExpandCollapse, and Transform according to functionality. |
| [ToolTip](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttooltipcontroltype) | Text optional. **Window required when an exposed UI action can close the tooltip**. |
| [Tree](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttreecontroltype) | Selection when meaningful item selection is maintained; Scroll when scrollable. |
| [TreeItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttreeitemcontroltype) | **ExpandCollapse required even on leaves**, with LeafNode state. SelectionItem when selectable; ScrollItem when the container scrolls; Invoke for an independent command. SelectionContainer refers to the same owning tree across nesting levels. |
| [Window](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportwindowcontroltype) | **Window + Transform required for desktop windows**. Dock conditional. Transform capability flags report the operations actually permitted. |

The linked control-type contracts also specify element properties, tree shape, and events. Apply the following alongside the pattern table:

- **Names and labels:** Edit and ComboBox names identify the field's purpose; they must not contain the current text or selected value. Expose current content through Text/Value/Selection as applicable. Associate external labels through LabeledBy. Self-labeling controls use their caption or an alternate text equivalent. HeaderItem uses ItemStatus for sort order; TreeItem uses ItemType and ItemStatus when icons or changing status convey that information. Extensive Text content should be read through Text ranges rather than copied wholesale into Name.
- **Protected text:** an Edit containing a password sets IsPassword=true; querying its Value fails. Do not disclose its underlying password through Name, Text, events, or an alternate accessibility property. Follow the protected-text rules in the text section.
- **Control and content views:** AppBar, MenuBar, Header, HeaderItem, ScrollBar, Separator, Thumb, and TitleBar are excluded from content view. Text is content when its information is not already exposed by another element's Name. Informative Image elements need a textual equivalent; redundant image content is excluded from content view, and purely decorative images conveying no information are excluded from control/content views. ToolTip is content when keyboard-focusable; otherwise expose its help on the referring control's HelpText. The Menu control-type contract includes Menu in both views. Do not apply a blanket exclusion to every popup or every visual child.
- **Composite structure:** a Calendar exposes its spatial date grid, weekday header associations, and selectable dates. A Tab exposes semantic TabItems even if its templates use buttons; each TabItem owns its page or references it using ControllerFor. A Header contains HeaderItems. A List's selectable items use ListItem; DataGrid items and cells retain their grid/header relationships. Tooltip and submenu relationships should identify the control they describe or extend, while context menus may belong to a window or desktop root.
- **Focus and geometry:** Tab and ComboBox support keyboard focus, potentially delegated to an appropriate subtree target. Slider/ScrollBar child buttons and thumbs do not independently take keyboard focus. TitleBar is not keyboard-focusable. Report orientation for ScrollBar and Tab, and for other roles when applicable. Bounds cover the semantic element; provide a clickable point only when one exists, rather than inventing one for Tab or ScrollBar.
- **Common notifications:** support the focus, bounds, enabled/offscreen, name, and structure notifications specified by each role, raising them when the corresponding state actually changes. A nonfocusable element does not emit a fabricated focus event. Pattern notifications are additional requirements: Invoke_Invoked, ToggleState and ExpandCollapseState changes, Value/RangeValue changes, scrolling properties, selection events, and Text text/selection events as applicable.
- **Popup and window lifecycle:** MenuOpened/MenuClosed, ToolTipOpened/ToolTipClosed, and WindowOpened/WindowClosed describe their respective surfaces and cannot be replaced by StructureChanged alone. MenuModeStart precedes the first MenuOpened while entering menu navigation; MenuModeEnd follows the last MenuClosed when leaving it. ToolTip implementations exposing Window also implement that pattern's events.
- **Layout and loading:** Calendar, DataGrid, and Window support LayoutInvalidated, as does List when its child layout can change. Pane and Window support AsyncContentLoaded for asynchronous loading. MultipleView reports CurrentView changes; scrollable containers report changes to all affected scroll percentages, viewport sizes, and scrollability properties. Apply the linked role and pattern contracts instead of emitting every event for every update.

### Template groups, `vl::presentation::theme::ThemeName`, and UI Automation roles

The enum selects a default template; its values have no inheritance relationship. The actual template classes inherit as declared in [GuiThemeManager.h](../Source/Application/Controls/GuiThemeManager.h) and [GuiControlTemplates.h](../Source/Controls/Templates/GuiControlTemplates.h). Relevant template ancestry is:

- `GuiControlTemplate` derives from `GuiTemplate`.
  - `GuiLabelTemplate`, `GuiDocumentLabelTemplate`, `GuiScrollTemplate`, `GuiDatePickerTemplate`, and the basic ribbon container templates derive from `GuiControlTemplate`.
  - `GuiWindowTemplate` → `GuiMenuTemplate` → `GuiRibbonGroupMenuTemplate` / `GuiRibbonToolstripMenuTemplate`.
  - `GuiButtonTemplate` → `GuiSelectableButtonTemplate` → `GuiToolstripButtonTemplate` → `GuiListViewColumnHeaderTemplate` / `GuiComboBoxTemplate`; `GuiDateComboBoxTemplate` derives from `GuiComboBoxTemplate`.
  - `GuiScrollViewTemplate` → `GuiDocumentViewerTemplate` / `GuiListControlTemplate`; `GuiTextListTemplate`, `GuiListViewTemplate`, and `GuiTreeViewTemplate` derive from `GuiListControlTemplate`.
  - `GuiTabTemplate` → `GuiRibbonTabTemplate`.
  - `GuiRibbonGalleryTemplate` → `GuiRibbonGalleryListTemplate`.
- Item templates form a separate branch: `GuiTemplate` → `GuiListItemTemplate` → `GuiTextListItemTemplate` / `GuiTreeItemTemplate`. `GuiGridVisualizerTemplate` and `GuiGridEditorTemplate` derive from `GuiGridCellTemplate`, which derives from `GuiControlTemplate`.

Apply the most specific control-class mapping before its base classes, then inspect actual behavior and semantic context. A composition's explicit UIA override takes precedence as required by [1.4.1.3.md](./1.4.1.3.md). A template may change hit targets or behavior as well as appearance, so re-evaluate capabilities after installation; an appearance change alone does not justify a role change. Template inheritance does not imply UIA pattern inheritance.

`CT` below means the UIA control type. Pattern names refer to their provider interfaces, for example Selection = `ISelectionProvider`, SelectionItem = `ISelectionItemProvider`, Text = `ITextProvider`, and RangeValue = `IRangeValueProvider`. The [Microsoft control-type contracts](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportinguiautocontroltypes) determine required properties, tree structure, patterns and events; conditions below refer to actual features, not arbitrary implementation options. Disabled/read-only state must be reported accurately rather than being treated as disappearance of the element.

Default theme coverage (every enum value, including the two values outside `GUI_CONTROL_TEMPLATE_TYPES`):

| Template group | `ThemeName` values | Semantic mapping and conditions |
| --- | --- | --- |
| Explicit special values | `Unknown` | No default semantic role. Common for controls with an explicit template, including column headers and item decorators; use actual class and context. |
| Window alias | `Window` | Chooses window styling; map the actual `GuiWindow`, popup, menu or tooltip role instead of treating the alias as a distinct UIA type. |
| `GuiWindowTemplate` | `SystemFrameWindow`, `CustomFrameWindow` | Ordinary desktop windows: CT=Window + Window + Transform, with individual capability flags reporting the permitted operations even when move/resize is disabled. Hosted logical windows must not invent HWNDs. |
| `GuiWindowTemplate` | `Tooltip` | CT=ToolTip; Text depends on its contents. Include Window when an exposed UI action closes the tooltip; report tooltip lifecycle events. |
| `GuiControlTemplate` | `CustomControl` | Generic fallback CT=Custom, or Group/Pane when that is the actual container role. Specific subclasses and semantic overrides take priority. |
| `GuiControlTemplate` | `GroupBox` | CT=Group; ExpandCollapse only when the group actually shows/hides contents. |
| `GuiControlTemplate` | `MenuBar` | CT=MenuBar with MenuItem children. |
| `GuiControlTemplate` | `MenuSplitter`, `ToolstripSplitter`, `ToolstripSplitterInMenu`, `RibbonSplitter` | CT=Separator where exposed; no fabricated action or selection. Pure drawing details may remain outside the control/content views. |
| `GuiControlTemplate` | `ToolstripToolBar`, `ToolstripToolBarInMenu` | CT=ToolBar; expose its commands and other actual controls. |
| `GuiControlTemplate` | `RibbonToolstripHeader` | Text naming a menu/gallery group, or the associated Group's label. This is not a data-column Header. |
| `GuiLabelTemplate` | `Label`, `ShortcutKey` | CT=Text where the label is independently meaningful. Shortcut overlays annotate the owning action: AccessKey for a mnemonic/ALT sequence, AcceleratorKey for an actual shortcut. Display text alone does not prove a working shortcut. |
| `GuiScrollViewTemplate` | `ScrollView` | `GuiScrollContainer`: CT=Pane with Scroll when scrollable. More specific scroll-view subclasses retain their own roles. |
| `GuiTabTemplate` | `Tab` | CT=Tab + Selection; logical headers are TabItem + SelectionItem. |
| `GuiComboBoxTemplate` | `ComboBox` | `GuiComboBoxListControl`: CT=ComboBox + ExpandCollapse, with selection delegated to its list. `GuiComboButton` has arbitrary popup content and must not invent list selection or text entry. |
| `GuiDocumentViewerTemplate` | `DocumentViewer`, `MultilineTextBox` | CT=Document + Text for document/multiline surfaces; Scroll when scrollable. String-value replacement is conditional, not implied by the theme. |
| `GuiDocumentLabelTemplate` | `DocumentLabel`, `DocumentTextBox` | CT=Document + Text for rich document surfaces. A genuinely simple static label may be Text. `DocumentTextBox` is a theme/XML alias for `GuiDocumentLabel`, not a separate C++ class or proof of editable mode. |
| `GuiDocumentLabelTemplate` | `SinglelineTextBox` | Simple text entry: CT=Edit + Text + Value. Rich document content requires Document semantics; actual read-only/password behavior must be honored. |
| `GuiListViewTemplate` | `ListView` | Ordinary list views: List + Selection and Scroll as applicable; MultipleView for supported view choices. A detail representation exposing headers via Table uses DataGrid + Grid + Table. Actual data-grid subclasses always need their more specific mapping below. |
| `GuiTreeViewTemplate` | `TreeView` | CT=Tree + Selection, Scroll when scrollable. Hierarchical TreeItem nodes expose ExpandCollapse, SelectionItem and ScrollItem as applicable. |
| `GuiTextListTemplate` | `TextList`, `RibbonGalleryItemList` | CT=List + Selection, Scroll when scrollable. Spatial item navigation can also require Grid/GridItem. |
| `GuiSelectableButtonTemplate` | `ListItemBackground` | Visual wrapper for the owning item; merge/replace its semantics with ListItem, TreeItem or DataItem as appropriate. Do not emit an extra selectable ListItem for every background button. |
| `GuiSelectableButtonTemplate` | `TreeItemExpander` | Part of the owning TreeItem's ExpandCollapse behavior. A separately exposed expander is a redundant control-view detail, not an additional content item or list selection. |
| `GuiSelectableButtonTemplate` | `CheckTextListItem`, `RadioTextListItem` | Bullet styles for the item's separate Checked state. Default GacUI check and radio views need Toggle for Checked as well as SelectionItem for row selection; radio appearance does not establish mutual exclusion. |
| `GuiSelectableButtonTemplate` | `CheckBox`, `RadioButton` | Checkbox behavior: CheckBox + Toggle. A genuinely exclusive radio choice: RadioButton + SelectionItem tied to its selection group. Verify the group/selection behavior; do not infer it solely from style. |
| `GuiMenuTemplate` | `Menu` | CT=Menu; command, submenu, toggle and exclusive-choice behavior belongs to its MenuItem children. |
| `GuiToolstripButtonTemplate` | `MenuBarButton`, `MenuItemButton` | CT=MenuItem. Invoke for a command, ExpandCollapse for a submenu, Toggle for a checked option, SelectionItem for an exclusive choice, according to actual behavior. Do not duplicate another pattern's action with Invoke. |
| `GuiToolstripButtonTemplate` | `ToolstripButton`, `RibbonSmallButton`, `RibbonLargeButton` | CT=Button with Invoke for commands or Toggle for state changes, not both on the same ordinary button. |
| `GuiToolstripButtonTemplate` | `ToolstripDropdownButton`, `RibbonSmallDropdownButton`, `RibbonLargeDropdownButton` | Pure menu opener: MenuItem + ExpandCollapse with its popup menu. Arbitrary popup content needs a suitable aggregate role. Do not label it SplitButton without a separate default action. |
| `GuiToolstripButtonTemplate` | `ToolstripSplitButton`, `RibbonSmallSplitButton`, `RibbonLargeSplitButton` | Actual split action: CT=SplitButton + Invoke + ExpandCollapse; represent separate hit targets and their submenu consistently. If no independent default action exists, use the pure-dropdown mapping. |
| `GuiRibbonTabTemplate` | `RibbonTab` | CT=Tab + Selection, with TabItem headers and accessible content of the selected page. |
| `GuiRibbonGroupTemplate` | `RibbonGroup` | CT=Group. Its overflow/dropdown presentation needs accessible expansion of hidden contents; preserve commands as responsive layout moves them. The separate expand-command button invokes its command rather than being mistaken for group collapse. |
| `GuiRibbonGroupMenuTemplate` | `RibbonGroupMenu` | Popup for a ribbon group's controls. Use a Group/Pane representation for arbitrary grouped content; use Menu semantics only for actual menu structure. The C++ `GuiMenu` base alone does not force Menu. |
| `GuiRibbonIconLabelTemplate` | `RibbonIconLabel`, `RibbonSmallIconLabel` | Preserve the container's arbitrary children. Expose meaningful label/image content with appropriate semantics; decorative or duplicate images do not become independent content elements. |
| `GuiRibbonButtonsTemplate` | `RibbonButtons` | Command grouping: Group or ToolBar where that matches the user-facing role; layout-only wrappers need not add redundant content nodes. |
| `GuiRibbonToolstripsTemplate` | `RibbonToolstrips` | Group containing its toolbars/commands, or one logical ToolBar if that is the exposed aggregate. Do not duplicate responsive layout wrappers. |
| `GuiRibbonGalleryTemplate` | `RibbonGallery` | Generic gallery container with scroll/dropdown request events. It has no intrinsic selection or scroll-position model; derive additional patterns from actual content/behavior. |
| `GuiRibbonToolstripMenuTemplate` | `RibbonToolstripMenu` | Menu for menu commands, preserving the extra gallery/content region with its own semantic subtree. |
| `GuiRibbonGalleryListTemplate` | `RibbonGalleryList` | `GuiBindableRibbonGalleryList` is a composite gallery with an item list and popup. Use an aggregate such as Group with ExpandCollapse, retaining List + Selection + Scroll on the actual list; expose popup groups/items even when built from repeat compositions. |
| `GuiButtonTemplate` | `Button` | CT=Button + Invoke for its command. More specific button subclasses are dispatched first. |
| `GuiDatePickerTemplate` | `DatePicker` | CT=Calendar + Grid + Table + Selection for selectable dates. Model day items, weekday headers and date navigation; Value is not the calendar's date-selection API. |
| `GuiDateComboBoxTemplate` | `DateComboBox` | Composite date choice: CT=ComboBox + ExpandCollapse with a Calendar subtree and its Selection. Do not promise arbitrary-text Value for this non-editable calendar dropdown. |
| `GuiScrollTemplate` | `HScroll`, `VScroll` | ScrollBar when operating another container's scrolling; Orientation is required. RangeValue is conditional on the container's Scroll support. A standalone value selector uses Slider. |
| `GuiScrollTemplate` | `HTracker`, `VTracker` | CT=Slider + RangeValue, reporting actual orientation/range and supported changes. |
| `GuiScrollTemplate` | `ProgressBar` | CT=ProgressBar + read-only RangeValue for determinate progress; no writable action. Do not invent a numeric value if a custom implementation is indeterminate. |

Important contract boundaries:

- [Button](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportbuttoncontroltype), [SplitButton](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportsplitbuttoncontroltype) and [Invoke](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinginvoke) distinguish state changes, default commands and opening menus. `GuiMenuButton::GetSubMenuHost()` in [GuiMenuControls.cpp](../Source/Controls/ToolstripPackage/GuiMenuControls.cpp) reveals whether the template provides a separate dropdown hit target; it does not by itself prove an independent command exists.
- [List](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportlistcontroltype) never supports Table: choose DataGrid when exposing that contract. Grid/GridItem may also describe item-to-item spatial navigation in icon/tile layouts; their availability must not be restricted to detail mode merely because that mode looks like a table.
- [Edit](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporteditcontroltype) describes simple single-line text entry and requires Text; [Document](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdocumentcontroltype) covers richer/multiline surfaces and always requires Text. Value is a separate string-value contract, not a replacement for Text reading/navigation.
- [Calendar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcalendarcontroltype) requires Grid and Table, plus Selection when dates are selectable. [ComboBox](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportcomboboxcontroltype) requires ExpandCollapse; Scroll belongs to the visible list inside it, and editable Value applies only when arbitrary text can be entered. ComboBox Name identifies the field, not the currently selected date/item.
- [ScrollBar](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportscrollbarcontroltype) is a control-view element, not content; it never exposes Scroll itself. `GuiScroll`'s range maximum is `GetMaxPosition()` (`TotalSize - PageSize`), as defined in [GuiScrollControls.cpp](../Source/Controls/GuiScrollControls.cpp).

Exact-class and logical-node coverage that a theme-only dispatcher would miss:

| Actual class or semantic node | Required distinction and source |
| --- | --- |
| `GuiControl`, `GuiCustomControl`, `GuiLabel`, `GuiButton`, `GuiSelectableButton` | Distinguish semantic roles and selectable-button group behavior, including application-defined subclasses. See [GuiBasicControls.h](../Source/Application/Controls/GuiBasicControls.h), [GuiButtonControls.h](../Source/Controls/GuiButtonControls.h), and [XML control registrations](../Source/Compiler/InstanceLoaders/GuiInstanceLoader_Plugin.cpp). |
| `GuiControlHost`, `GuiWindow`, `GuiPopup`, `GuiTooltip` | A control host/popup is not automatically an ordinary Window. Preserve menus, tooltips and logical hosted windows; only real native hosts supply HWND relationships. See [GuiWindowControls.h](../Source/Application/Controls/GuiWindowControls.h). |
| `GuiTab`, `GuiRibbonTab`, `GuiTabPage`, `GuiRibbonTabPage` | The pages use `CustomControl` styling. Provide TabItem headers with SelectionItem and a coherent association with page content; page bounds must not be substituted for header bounds. See [GuiContainerControls.h](../Source/Controls/GuiContainerControls.h) and [GuiRibbonControls.h](../Source/Controls/ToolstripPackage/GuiRibbonControls.h). |
| `GuiScrollView`, `GuiScrollContainer`, `GuiScroll` | Container scrolling, scrollbar manipulation, standalone sliders and progress share implementation pieces but have different UIA contracts. See [GuiContainerControls.h](../Source/Controls/GuiContainerControls.h) and [GuiScrollControls.h](../Source/Controls/GuiScrollControls.h). |
| `GuiComboBoxBase`, `GuiComboButton`, `GuiComboBoxListControl`, `GuiDateComboBox` | The base and arbitrary-control popup do not supply list-selection semantics. Preserve the specific list/calendar child and its operations. See [GuiComboControls.h](../Source/Controls/ListControlPackage/GuiComboControls.h) and [GuiDateTimeControls.h](../Source/Controls/GuiDateTimeControls.h). |
| `GuiDocumentViewer`, `GuiMultilineTextBox`, `GuiDocumentLabel`, `GuiSinglelineTextBox` | Select document/edit/label semantics from the actual text surface and configuration. `GuiDocumentTextBox` is an XML virtual type for `GuiDocumentLabel`, registered in [GuiInstanceLoader_Document.cpp](../Source/Compiler/InstanceLoaders/GuiInstanceLoader_Document.cpp). See [GuiDocumentViewer.h](../Source/Controls/TextEditorPackage/GuiDocumentViewer.h). |
| `GuiListControl`, `GuiSelectableListControl`; `GuiVirtualTextList`, `GuiTextList`, `GuiBindableTextList` | Custom nonselectable contents must not inherit a fabricated Selection contract. Selectable subclasses expose their true selection policy; item provider/template content determines additional item capabilities. See [GuiListControls.h](../Source/Controls/ListControlPackage/GuiListControls.h), [GuiTextListControls.h](../Source/Controls/ListControlPackage/GuiTextListControls.h), and [GuiBindableListControls.h](../Source/Controls/ListControlPackage/GuiBindableListControls.h). |
| `GuiListViewBase`, `GuiVirtualListView`, `GuiListView`, `GuiBindableListView` | View support, item navigation and header-bearing detail mode determine List/Grid/DataGrid behavior; bindable and virtual variants are not separate semantic roles. See [GuiListViewControls.h](../Source/Controls/ListControlPackage/GuiListViewControls.h) and its `.cpp` view/arranger selection. |
| `GuiVirtualDataGrid`, `GuiBindableDataGrid` | Dispatch before `GuiVirtualListView`: DataGrid + Grid, Table for headers, selection matching the real row/cell model, and editable cell semantics. Both use ListView styling. See [GuiDataGridControls.h](../Source/Controls/ListControlPackage/GuiDataGridControls.h) and [GuiBindableDataGrid.h](../Source/Controls/ListControlPackage/GuiBindableDataGrid.h). |
| `GuiVirtualTreeListControl`, `GuiVirtualTreeView`, `GuiTreeView`, `GuiBindableTreeView` | Expose hierarchical nodes from the tree data model, not the flat list of currently visible rows. See [GuiTreeViewControls.h](../Source/Controls/ListControlPackage/GuiTreeViewControls.h). |
| `GuiListViewColumnHeader` and its header container | `Unknown` theme plus explicit template still means HeaderItem under Header. Invoke where sortable, Transform where resizable, and ItemStatus for sort order. See [GuiListViewControls.cpp](../Source/Controls/ListControlPackage/GuiListViewControls.cpp) and [Microsoft HeaderItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportheaderitemcontroltype). |
| `GuiMenu`, `GuiMenuBar`, `GuiMenuButton`; `GuiToolstripMenu`, `GuiToolstripMenuBar`, `GuiToolstripToolBar`, `GuiToolstripButton` | Class, containing menu/toolbar and actual command/submenu/state behavior jointly determine semantics. See [GuiMenuControls.h](../Source/Controls/ToolstripPackage/GuiMenuControls.h) and [GuiToolstripMenu.h](../Source/Controls/ToolstripPackage/GuiToolstripMenu.h). |
| `GuiToolstripNestedContainer`, `GuiToolstripGroupContainer`, `GuiToolstripGroup` | Preserve meaningful command groups while flattening layout-only wrappers in control/content views. See [GuiToolstripMenu.h](../Source/Controls/ToolstripPackage/GuiToolstripMenu.h). |
| `GuiRibbonGroup`, internal `GuiRibbonGroupMenu`, `GuiRibbonIconLabel`, `GuiRibbonButtons`, `GuiRibbonToolstrips`, `GuiRibbonGallery`, `GuiRibbonToolstripMenu` | Preserve actual controls as responsive layout moves them between ordinary content and popups. The generic gallery has only request events, not a selection data model. See [GuiRibbonControls.h](../Source/Controls/ToolstripPackage/GuiRibbonControls.h) and [GuiRibbonControls.cpp](../Source/Controls/ToolstripPackage/GuiRibbonControls.cpp). |
| `GuiBindableRibbonGalleryList` | Expose the embedded list, grouped popup items, selection and distinct item-application command. Popup items use repeat compositions rather than a list control. See [GuiRibbonGalleryList.h](../Source/Controls/ToolstripPackage/GuiRibbonGalleryList.h) and [gallery behavior](../.github/KnowledgeBase/manual/gacui/components/controls/ribbon/gallerylist.md). |
| Logical list items, tree nodes, grid rows/cells, tab headers and calendar days | Providers are not limited to `GuiControl` instances or realized templates. Stable logical identity must survive scrolling/template replacement; assign item patterns to the logical item, not each visual wrapper. TreeItem leaves still expose ExpandCollapse with LeafNode state, and selectable nodes identify the same tree SelectionContainer. See [Microsoft TreeItem](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttreeitemcontroltype). |
| Grid visualizers/editors and custom item-template content | Composition-level overrides extend the semantic tree and replace the associated control implementation. An active editor replaces the visualizer's semantics in the cell; preserve grid identity/position without exposing duplicate visualizer/editor content. See [1.4.1.3.md](./1.4.1.3.md) and [grid templates](../Source/Controls/Templates/GuiControlTemplates.h). |
| `compositions::GuiRowSplitterComposition`, `compositions::GuiColumnSplitterComposition` | These interactive compositions have no control/theme entry. Expose movable splitters as Thumb + Transform through the composition extension, with movement restricted to the permitted axis and layout bounds. Their layout table is not a semantic data Table merely because it arranges rows/columns. See [GuiGraphicsTableComposition.h](../Source/GraphicsComposition/GuiGraphicsTableComposition.h). |
| Embedded document controls, images and hyperlinks | Preserve interactive embedded elements and their document ranges; actual actions determine Invoke/other patterns. Purely decorative images or duplicate labels need not become separate content elements. See [GuiDocumentCommonInterface.h](../Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h). |

Text modes are independent of those class/style names. `GuiDocumentEditMode` in [GuiDocumentConfig.h](../Source/Controls/TextEditorPackage/GuiDocumentConfig.h) defines ViewOnly, Selectable and Editable. A Document provider still exposes text in every mode; report unsupported selection when ViewOnly, permit supported selection without editing in Selectable, and permit edits only in Editable. Read-only state and supported-selection values must follow the real model. A theme swap must not grant editing. Value/Text methods must also respect any password protection; neither a readable default nor a class name justifies leaking protected text.

The built-in `RadioTextListItem` does not enforce exclusive choice. [ItemTemplate_ITextItemView.cpp](../Source/Controls/ListControlPackage/ItemTemplate_ITextItemView.cpp) changes the radio bullet's style while synchronizing its Selected property with the data item's separate Checked value; [GuiTextListControls.cpp](../Source/Controls/ListControlPackage/GuiTextListControls.cpp) toggles checked values with Space. Expose this checked state independently from list row selection. Custom templates with true exclusive-choice behavior require a separate, correctly identified selection group.

Labels and wrappers also need semantic review: [RibbonToolstripHeader](../.github/KnowledgeBase/manual/gacui/components/controls/ribbon/header.md) labels groups rather than data columns, and [RibbonIconLabel](../.github/KnowledgeBase/manual/gacui/components/controls/ribbon/iconlabel.md) can contain arbitrary child controls. A tree assembled solely from enum values would either mislabel these nodes or hide their children.

### Details about container control with items (combo, list, tab, etc) and virtualization

References:
- Implementing patterns: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinguiautocontrolpatterns
- `ISelectionProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iselectionprovider
- `ISelectionItemProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iselectionitemprovider
- `IScrollProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iscrollprovider
- `IScrollItemProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iscrollitemprovider
- `IVirtualizedItemProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-ivirtualizeditemprovider
- `IItemContainerProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-iitemcontainerprovider

Item containers:
- The container exposes structure/selection/scrolling; items expose item-level behavior and properties.
- Items should be discoverable and actionable even if currently scrolled out or virtualized.

Selection:
- A selection-capable container implements `ISelectionProvider` and its selectable items implement `ISelectionItemProvider`. A generic container does not acquire selection just by having children.
- Raise selection-related events when selection changes.
  - Container must report `CanSelectMultiple` and `IsSelectionRequired` accurately.
  - `GetSelection()` returns selected items. UIA's virtualized-container guidance permits it to return only the nonvirtualized/viewport selection; document that limitation if used and support discovery through ItemContainer. Prefer complete logical selection where the data model can expose it without realizing every visual. A required-selection container can be empty while it has no selectable items; do not manufacture a selected provider.
  - Items must implement selection actions consistently:
    - `Select()` performs replacement selection, rather than multi-select addition. Follow the method's documented already-selected behavior. See [Select](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-iselectionitemprovider-select).
    - `AddToSelection()` / `RemoveFromSelection()` are not unconditionally forbidden on single-select controls. Reject an operation only when its resulting selection violates the container's rules (for example adding another item to a nonempty single-select control or removing the last required selection).
    - A result containing just one selected item raises `UIA_SelectionItem_ElementSelectedEventId`; otherwise raise the applicable added/removed events. Large aggregate changes can use the documented Selection invalidation convention. See [selection event rules](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-iselectionitemprovider-addtoselection).

Scrolling and "bring into view":
- Container implements `IScrollProvider` when scrollable.
- Items implement `IScrollItemProvider` so clients can bring them into view.
  - `IScrollProvider` must report:
    - `HorizontallyScrollable` / `VerticallyScrollable`
    - `HorizontalScrollPercent` / `VerticalScrollPercent`
    - `HorizontalViewSize` / `VerticalViewSize`
  - `Scroll()` and `SetScrollPercent()` should match user-perceived scrolling (page/line semantics should feel consistent).
  - Percent is in `[0, 100]`; report `UIA_ScrollPatternNoScroll` for a non-scrollable axis, whose view size is 100. In `SetScrollPercent`, NoScroll means leave that axis unchanged. Implement unsupported-axis and invalid-range failures according to the [Scroll contract](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingscroll).

Virtualization:
- Distinguish visual-template recycling from UIA virtualization. If complete logical item providers remain available without realizing visuals, they can remain ordinary elements. If an item is unavailable in the UIA tree until realization, its container must implement ItemContainer and its placeholder must implement VirtualizedItem. This is not an optional search optimization. See [Working with Virtualized Items](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-workingwithvirtualizeditems).
- `Realize()` makes the returned placeholder fully usable; it does not guarantee visibility or selection. Clients use ScrollItem separately to request visibility. Unsupported placeholder queries return `UIA_E_ELEMENTNOTAVAILABLE`; retain access to VirtualizedItem so realization remains possible.
- `FindItemByProperty` searches after `startAfter`, with null meaning the beginning; `propertyId == 0` enumerates the next item and ignores the value. GacUI should support Name, AutomationId, and IsSelected search where meaningful; Microsoft highly recommends, but does not require, these properties in the [ItemContainer contract](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingitemcontainer). Return null at the end/no match and the documented failure for an unsupported search property. Enumeration order is not a portable model-index API.
- Keep identity attached to data items/tree nodes, not recycled compositions or visible row indices. Preserve true tree-node parentage instead of flattening a visible-item adapter. Raise structure changes when realization, removal, expansion, or replacement changes the exposed tree; retire providers when the underlying data item is deleted.

Combo box popup behavior:
- Combo box element uses `IExpandCollapseProvider` to represent open/closed.
- Popup list presence must be consistent with expand state; raise property/structure changes appropriately.
- Selection may be aggregated from its list. Scroll belongs to the popup list, not to the ComboBox itself. An arbitrary-content popup button has different semantics from a list-selection combo; see the exact-class mapping.

Tab behavior:
- Tab control is the container (`ISelectionProvider`).
- Tab headers are items (`ISelectionItemProvider`), and selecting them switches the active content.

### Details about list view detail view (grid/table semantics)

References:
- `IGridProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-igridprovider
- `IGridItemProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-igriditemprovider
- `ITableProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itableprovider
- `ITableItemProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itableitemprovider

When to expose grid/table:
- Expose Grid/GridItem for actual row/column addressing, including spatial item navigation in an icon/tile list. Do not invent a logical grid merely because controls happen to align.
- A List can expose Grid for spatial navigation, but must never expose Table. A detail view with tabular headers uses DataGrid + Grid + Table, with appropriate DataItem/row/cell and header providers. See the [List contract](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportlistcontroltype) and [DataGrid contract](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportdatagridcontroltype).

Minimum model:
- The container implements `IGridProvider` (row/column counts + `GetItem(row, column)`).
- Each cell implements `IGridItemProvider` (row/column indices + containing grid).
- If headers exist, add `ITableProvider`/`ITableItemProvider` to associate cells with headers.
  - `IGridItemProvider` must report `Row`, `Column`, `RowSpan`, `ColumnSpan`, and `ContainingGrid`.
  - `ITableProvider` should report `RowOrColumnMajor` and return row/column header elements from `GetRowHeaders()` / `GetColumnHeaders()` when the UI has headers.
  - `ITableItemProvider::GetRowHeaderItems` and `GetColumnHeaderItems` return the headers associated with that cell. Table and TableItem require the separate Grid and GridItem interfaces; they do not inherit those C++ COM interfaces.

Selection, focus, scrolling:
- Model row selection separately from cell selection/focus. In particular, `GuiVirtualDataGrid` in `Source/Controls/ListControlPackage/GuiDataGridControls.h` has selected-cell and current-editor state that its list-view base mapping cannot replace.
- If scrollable, expose `IScrollProvider` on the container.
- `GetItem(row, column)` must work for valid grid coordinates: return the corresponding provider, possibly a realizable placeholder under the virtualization contract. A Grid implementation cannot leave valid offscreen coordinates generically unsupported; validate invalid indices. See [Grid](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementinggrid).
- When a cell enters editing, its editor replaces the visualizer exposure as required by the milestone. Retain its logical cell identity/coordinates and header associations where possible; if a semantic child is replaced, raise the structural change and make stale references safe. A commit or cancel restores the visualizer without exposing two competing cell values.

### Details about plain text and rich text controls (Text pattern)

References:
- Implementing text and ranges: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtextandtextrange
- `ITextProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextprovider
- `ITextRangeProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextrangeprovider
- `ITextEditProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itexteditprovider
- Mixed attribute sentinel: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiagetreservedmixedattributevalue

Plain text vs rich text:
- `IValueProvider` is not enough for many assistive technology scenarios; `ITextProvider` enables reading by word/line and reliable caret/selection support.
- Rich text requires correct ranges and attribute reporting; otherwise clients cannot interpret formatting.

Core expectations for `ITextProvider`:
- Provide `DocumentRange` and consistent range objects.
- Implement selection (`GetSelection`) and report correct `SupportedTextSelection`.
- Provide visible ranges (`GetVisibleRanges`) to reflect the viewport.
- Provide hit-testing (`RangeFromPoint`) and child mapping (`RangeFromChild`) when needed.
  - `DocumentRange` must span the whole document, not only visible text.
  - `GetVisibleRanges()` reflects the current viewport, including partial lines; it is not an enumeration of all document paragraphs. `GetSelection()` returns a degenerate caret range when an insertion point exists without selected text.

Core expectations for `ITextRangeProvider`:
- Ranges must normalize and behave consistently under all operations.
- `GetText(maxLength)` returns plain text (including source control characters), not document markup or an image's alternate-name string. Honor the requested maximum; `-1` means no limit. See [GetText](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nf-uiautomationcore-itextrangeprovider-gettext).
- Implement all 18 range methods: `Clone`, `Compare`, `CompareEndpoints`, `ExpandToEnclosingUnit`, `FindAttribute`, `FindText`, `GetAttributeValue`, `GetBoundingRectangles`, `GetChildren`, `GetEnclosingElement`, `GetText`, `Move`, `MoveEndpointByUnit`, `MoveEndpointByRange`, `Select`, `AddToSelection`, `RemoveFromSelection`, and `ScrollIntoView`. Range comparisons require the same text provider. Failed searches return null; endpoint movement must preserve ordered endpoints. See [ITextRangeProvider](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextrangeprovider).
- Movement/expansion must implement Character, Format, Word, Line, Paragraph, Page, and Document units or their documented larger-unit fallback.
  - `GetBoundingRectangles()` returns physical-screen rectangles for visible text portions (multiple rectangles for wrapped lines is normal); an empty array is appropriate for an offscreen or degenerate range, not a fabricated caret rectangle. Caret retrieval belongs to Text2's `GetCaretRange` and the text layout.
  - `ScrollIntoView(alignToTop)` should reveal the range in the viewport.
  - Selection methods (`Select`, `AddToSelection`, `RemoveFromSelection`) must match the control's selection model.
- `GetAttributeValue(TEXTATTRIBUTEID)` must return a uniform value, or the mixed-attribute reserved value when needed:
  - https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiagetreservedmixedattributevalue

Text extensions and events:
- `ITextProvider2` adds `GetCaretRange` and `RangeFromAnnotation`; `ITextRangeProvider2` adds `ShowContextMenu`. These are versioned extensions, not permission to omit base-interface members.
- Raise `UIA_Text_TextChangedEventId` after text edits, including offscreen edits, and `UIA_Text_TextSelectionChangedEventId` after selection/caret changes. If Value is also supported, raise its value-change event as well. Read-only text can still support selection and Text; do not remove Text solely because editing is disabled.
- `ITextEditProvider` adds `GetActiveComposition` and `GetConversionTarget` to Text for IME composition/autocorrection. It has no insert, replace, delete, copy, or paste method. Expose it when those composition/edit-observation semantics are implemented, rather than for every editable control. `UiaRaiseTextEditTextChangedEvent` reports the documented AutoCorrect, Composition, or CompositionFinalized payloads; ConversionTargetChanged reports target movement. It is not a generic incremental-diff event for every keystroke. See [TextEdit contract](https://learn.microsoft.com/en-us/windows/win32/winauto/textedit-control-pattern).

### Dynamic changes: `ControlType`, view modes, and pattern availability

References:
- Control type IDs: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-controltype-ids
- Automation element property IDs: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-automation-element-propids
- Pattern availability properties: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-control-pattern-availability-propids
- Property changed event raising: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaraiseautomationpropertychangedevent
- Structure changed event raising: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaraisestructurechangedevent
- Multiple view pattern: `IMultipleViewProvider` https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-imultipleviewprovider

What should (not) cause semantic changes:
- Theme changes (`ThemeName`) should normally not change UIA semantics. Visual restyling is not a semantic role change.
- Only real behavior changes should affect patterns: e.g. arbitrary text input becomes available or a list switches into a tabular view. Read-only and disabled states normally update pattern properties/action results; they do not remove the ability to read text or values.

Can a provider change `ControlType` and patterns dynamically?
- Yes, but it is a compatibility risk because UIA clients often cache properties and pattern pointers.
- UIA does not provide a dedicated "pattern invalidated" event; clients discover pattern availability by querying again.

Recommended strategy (in order of preference):
1. Keep logical identity and role stable where the control-type contract permits. Expose `IMultipleViewProvider` when the control offers multiple representations of the same data. A List may gain spatial Grid, but not Table: a tabular view needs DataGrid semantics, even if that requires a deliberate role/subtree transition.
2. When a pattern is actually unavailable, `GetPatternProvider` returns null. Do not simultaneously return a usable pattern and claim its availability is false. Raise applicable property changes for availability/state and view changes after updating the model. A transiently invalid operation (disabled control, read-only value, scrolling at an edge) is distinct from an absent pattern.
3. For a role change on the same logical element, update ControlType and its property notification, and report actual child changes. Alternatively, replace a semantic subtree and issue the corresponding structure notification; only genuinely replaced elements get new identities. Identity replacement is a GacUI design choice, not a UIA requirement for every mode change. Do not churn IDs for mere restyling or reordering.

What if the client already obtained a pattern that is no longer supported?
- You cannot assume the client will drop the old pointer immediately.
- Keep COM `QueryInterface` support static for the lifetime of each COM object; dynamic UIA pattern availability is controlled by `GetPatternProvider`, not by changing the object's COM interface set. See [QueryInterface rules](https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-queryinterface%28refiid_void%29).
- Make retained pattern objects safe: validate the target and operation, use the method's documented failure when its operation is no longer applicable, and use `UIA_E_ELEMENTNOTAVAILABLE` for a retired element. Never redirect a recycled item's provider to unrelated data.
- Always raise the relevant property changed events so clients have a chance to detect the transition and query the current pattern set again.

### Text editing details: unsupported text units, embedded objects, and copy/paste

References:
- Implementing text and ranges: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtextandtextrange
- `ITextEditProvider`: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itexteditprovider
- `ITextChildProvider` (embedded objects): https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcore/nn-uiautomationcore-itextchildprovider
- Text edit change event: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiaraisetextedittextchangedevent

Unsupported text units follow a defined fallback, not a provider-specific no-op:
- For `Move`, `MoveEndpointByUnit`, and `ExpandToEnclosingUnit`, substitute the next larger supported unit in this order: Character, Format, Word, Line, Paragraph, Page, Document. For example, unsupported Word falls back to Line if Line is supported; unsupported Page falls back to Document. Return the actual movement count, not zero merely because the requested unit was unsupported.
- Page means a document page. A scrolling viewport is not automatically a page. Line follows displayed wrapping; Format follows attribute runs and embedded-object boundaries. Character/Word are linguistic units, not arbitrary UTF-16 code units or a universal whitespace-only tokenization. Map GacUI positions to the documented units without splitting surrogate pairs.
- Use the actual control's word-boundary behavior where appropriate, with language-aware breaking when supported. Clients cannot query a universal "supported text units" list and must not infer real Paragraph support from a successful movement alone.

See [UI Automation text units](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-uiautomationtextunits).

Embedded images and embedded controls inside a document:
- UIA Text pattern supports embedded objects.
- Do not impose a universal object-replacement character in UIA `GetText`. Expose an object's textual content where it exists, not its alternate Name; nontext objects are discovered through ranges and children. `RangeFromChild` is degenerate for an object without text. A private document-model placeholder does not by itself define the UIA text stream. See [embedded objects](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-textpattern-and-embedded-objects-overview).
- Embedded objects should also exist as UIA elements (children) so clients can interact with them:
  - `ITextRangeProvider::GetChildren()` returns embedded element providers contained in the range.
  - `ITextProvider::RangeFromChild()` maps an embedded element back to its text position.
  - Add `ITextChildProvider` where needed to expose the nearest text container and encompassing `TextRange`; it complements `GetChildren`/`RangeFromChild` rather than replacing them. It is an additional pattern, not automatically mandatory on every embedded element.
- For an embedded button, the embedded element should expose its own semantic patterns (e.g. `IInvokeProvider`) while remaining anchored into the document text via the text child relationship.

Copy/paste expectations:
- UIA does not give a dedicated clipboard pattern; clients often rely on a combination of:
  - Selection (`ITextProvider::GetSelection` and range selection methods)
  - Reading selected text (`ITextRangeProvider::GetText`)
  - Whole-value replacement via `IValueProvider::SetValue` where supported
  - Keyboard interaction (Ctrl+C / Ctrl+V) when driving the UI like a user
- Make sure selection + caret behavior is correct and events are raised:
  - Use TextChanged and TextSelectionChanged for ordinary editing and selection. Use TextEdit events only for their documented composition/autocorrection triggers.
- For rich content, define a clear conversion policy for copy:
  - Clipboard conversion follows the application's copy command; UIA `GetText` is text retrieval, not a clipboard operation. Neither Text nor TextEdit writes the clipboard or deletes an arbitrary range. Expose accessible editing commands, or let clients drive existing keyboard commands.
  - If you support rich formats internally, ensure UIA text attributes and embedded children remain consistent with what the user sees and edits.

### Plain text vs rich text: how clients tell, and how to expose formatting

References:
- Text attributes overview: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-textattributes
- Text attribute IDs: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-textattribute-ids
- Not supported sentinel: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiagetreservednotsupportedvalue
- Mixed attribute sentinel: https://learn.microsoft.com/en-us/windows/win32/api/uiautomationcoreapi/nf-uiautomationcoreapi-uiagetreservedmixedattributevalue

How clients usually distinguish plain vs rich:
- There is no single "PlainText/RichText" flag in UIA that every client uses.
- Clients infer it from a combination of:
  - Role, attributes, and embedded objects together; Text is required for Edit as well as Document and does not by itself distinguish plain from rich content.
  - Whether text attributes are supported and vary inside a range (rich text).
  - Whether embedded objects exist (rich document behavior).

Recommended provider behavior:
- Edit and Document require `ITextProvider`; selectable read-only text also needs text navigation. A static unselectable label can expose its Name without a Text pattern.
- For plain text:
  - `ITextRangeProvider::GetAttributeValue(TEXTATTRIBUTEID)` can return:
    - A uniform default value (e.g. a single font for the entire document), or
    - The reserved not-supported value when the attribute has no meaning in this control.
  - If formatting is not user-editable and never varies, returning a single uniform value for common attributes (font name/size/foreground color) is often helpful.
- For rich text:
  - `GetAttributeValue` must return:
    - A specific value when the whole range has that attribute, or
    - The reserved mixed value when multiple attribute values exist in the range.
    - The reserved not-supported value for an attribute that the control does not implement; unsupported and mixed are different states.
  - At minimum, ensure the common attributes are correct and consistent with rendering:
    - Font name, font size, font weight/bold, italic, underline, strikethrough
    - Foreground/background color
    - Horizontal alignment, indentation (if applicable)

How to expose "this part is bold/italic/underlined" (what UIA expects):
- UIA expects rich formatting to be discoverable by querying attributes on a range.
- Clients will typically:
  - Read text via `GetText()`.
  - Move/expand ranges by units (character/word/line/paragraph).
  - Call `GetAttributeValue()` for specific `TEXTATTRIBUTEID`s to determine formatting at the caret or across a selection.
- If your document model has runs/spans, make sure range endpoints can fall inside runs and attribute queries reflect the run at that position.

Embedded objects and formatting together:
- Embedded images/controls should appear in `GetChildren()` and also influence attributes/range text as needed.
- Keep a stable mapping between:
  - (text position) <-> (embedded UIA element) using `RangeFromChild()` and `ITextChildProvider`.

### Testing and validating a UIA provider implementation

References:
- Inspect Objects (Inspect.exe): https://learn.microsoft.com/en-us/windows/win32/winauto/inspect-objects
- WM_GETOBJECT: https://learn.microsoft.com/en-us/windows/win32/winauto/wm-getobject
- Supporting control types: https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportinguiautocontroltypes

Basic workflow (manual testing):
- Use Inspect.exe (Windows SDK) to:
  - Navigate the UIA tree, verify element order and parent/child relationships.
  - Check key properties: `ControlType`, `Name`, `AutomationId` (if any), `BoundingRectangle`, `IsEnabled`, `IsOffscreen`, focus properties.
  - Check which patterns are available on each element (and that they match expected behavior).
- Exercise interactions while Inspect is attached:
  - Toggle/view-mode switches: verify `Is*PatternAvailable` properties and relevant behavior changes.
  - Add/remove children (list items, tree nodes): verify structure changed events are raised and the tree updates.
  - Text editing: verify caret/selection behavior and that text-change notifications fire during typing/paste.

What to specifically validate for dynamic behavior:
- Verify each exposed role against the inventory above, including required/conditional/forbidden patterns and its content/control-view flags. Cover every actual control family and composition-provided semantic item, not just one instance of each theme.
- Detach and reattach controls, swap templates, recycle list visuals, destroy controls while clients retain providers, and activate/commit/cancel grid editors. Check once-only event hooks, ignored disconnected callbacks, stable logical item identity, and replacement of visualizer semantics.
- Check ordinary Windows, Hosted Mode, and remote core/renderer operation: correct root HWND, physical-screen bounds, focus/hit-testing, actions reaching the owning control, and coherent state/events after updates. Do not treat the current HTTP automation endpoints or rendering DOM as proof that Windows UIA is implemented.
- If a view mode changes (e.g. list view detail vs icons):
  - Verify whether `IMultipleViewProvider` is present and reflects the current view.
  - Verify whether grid/table patterns appear only when meaningful, and that old pattern objects remain safe to call.

What to validate for Text pattern:
- Range correctness:
  - `RangeFromPoint` hits the expected caret/character.
  - `GetBoundingRectangles` matches rendering (including wrapped lines).
- Editing correctness:
  - Selection changes are reflected immediately.
  - Copy/paste behaves like the UI (selected text is what gets copied; paste updates the document consistently).
- Rich text correctness:
  - Attribute queries match rendered formatting.
  - Mixed vs uniform attributes are reported correctly on multi-style selections.

Debugging hints:
- If Inspect cannot see your elements at all, start from `WM_GETOBJECT` and confirm the window responds with a UIA provider (`UiaRootObjectId` path).
- If properties/patterns look right but clients behave oddly, it is often an eventing issue: ensure you raise property/structure/text-change events consistently when the UI changes.

### Client sample: drive UI Automation from an `HWND` (C++ and C#)

Scenario:
- Given the top-level window's `HWND`, select its second tab, select the fourth list item in that tab, and delete the editor's second paragraph.

Assumptions for these small examples:
- The window contains one intended tab control. Tab items are direct children in displayed order. The selected `TabItem` has a child `Pane` containing the intended list and editor, and page content is ready when selection returns. Other providers can associate a separate pane through `ControllerFor`; resolve that relationship or an application-specific `AutomationId` instead of searching the entire window for the first list/editor. [TabItem tree and relationships](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supporttabitemcontroltype)
- The list is nonvirtualized, with all its items exposed as direct `ListItem` children in displayed order. For virtualized lists, locate the intended item through a supported `ItemContainer` search property, call `VirtualizedItem.Realize`, and use `ScrollItem.ScrollIntoView` if visibility is needed. Neither the fourth realized child nor the fourth result of `FindItemByProperty` enumeration necessarily denotes the fourth displayed/model item; the latter has no specified iteration order. [Virtualized items](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-workingwithvirtualizeditems), [ItemContainer search ordering](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationclient/nf-uiautomationclient-iuiautomationitemcontainerpattern-finditembyproperty)
- The editor exposes Text, permits selection/editing, reports `IsReadOnly=false` for the target range, and implements actual Paragraph boundaries. The second paragraph is nonfinal and its reported range includes the separator needed by this editor to remove the paragraph. Unsupported units are replaced with the next larger supported unit; a returned movement count of one does not prove Paragraph support. The document remains unchanged while its range is prepared. [Text-range movement and fallback](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationclient/nf-uiautomationclient-iuiautomationtextrange-move)
- Run these functions on a dedicated MTA worker that owns no windows. The C++ apartment object is created before all COM pointers so they are released before `CoUninitialize`; only successful initialization is balanced. The managed caller also supplies an MTA worker. [UIA threading](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-threading), [COM shutdown](https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-couninitialize)
- UIA provides no universal range-deletion API. After focusing the editor, select the paragraph, including the paragraph separator supplied by the provider, and inject Delete. The target must retain foreground/focus, no modifier keys should be held, and the client must be allowed to inject input into the target. The focus checks cannot eliminate a subsequent focus race. [SendInput contract](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput)
- API failures and detected prerequisite failures propagate as exceptions; the remaining fixture assumptions must already be verified. These examples demonstrate issuing the edit. Successful input injection does not establish completed deletion. A test must wait for and verify the resulting text change; `SendKeys.SendWait` can also return before another process handles the input. [Managed SendWait limitations](https://learn.microsoft.com/en-us/dotnet/api/system.windows.forms.sendkeys.sendwait)

C++ (Windows SDK and ATL; link `Ole32.lib`, `OleAut32.lib`, `User32.lib`, and `Uuid.lib`):

```cpp
    #include <Windows.h>
    #include <UIAutomation.h>
    #include <atlbase.h>
    #include <atlcomcli.h>

    static void Check(HRESULT result)
    {
        if (FAILED(result)) throw result;
    }

    static void Require(bool condition)
    {
        if (!condition) throw E_FAIL;
    }

    struct ComApartment
    {
        ComApartment()
        {
            Check(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
        }

        ~ComApartment()
        {
            CoUninitialize();
        }

        ComApartment(const ComApartment&) = delete;
        ComApartment& operator=(const ComApartment&) = delete;
    };

    static ATL::CComPtr<IUIAutomationElement> FindFirstByControlType(
        IUIAutomation* uia, IUIAutomationElement* root, CONTROLTYPEID controlType)
    {
        Require(root != nullptr);
        ATL::CComPtr<IUIAutomationCondition> condition;
        ATL::CComVariant value(static_cast<LONG>(controlType));
        Check(uia->CreatePropertyCondition(UIA_ControlTypePropertyId, value, &condition));
        ATL::CComPtr<IUIAutomationElement> element;
        Check(root->FindFirst(TreeScope_Descendants, condition, &element));
        return element;
    }

    static ATL::CComPtr<IUIAutomationElement> FindChildAt(
        IUIAutomation* uia, IUIAutomationElement* root, CONTROLTYPEID controlType, int index)
    {
        Require(root != nullptr);
        ATL::CComPtr<IUIAutomationCondition> condition;
        ATL::CComVariant value(static_cast<LONG>(controlType));
        Check(uia->CreatePropertyCondition(UIA_ControlTypePropertyId, value, &condition));
        ATL::CComPtr<IUIAutomationElementArray> children;
        Check(root->FindAll(TreeScope_Children, condition, &children));
        Require(children != nullptr);
        int count = 0;
        Check(children->get_Length(&count));
        Require(index >= 0 && index < count);
        ATL::CComPtr<IUIAutomationElement> child;
        Check(children->GetElement(index, &child));
        Require(child != nullptr);
        return child;
    }

    void SampleDriveFromHwnd(HWND hwnd)
    {
        ComApartment apartment; // Declared first; destroyed after all COM pointers.
        ATL::CComPtr<IUIAutomation> uia;
        Check(CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&uia)));
        ATL::CComPtr<IUIAutomationElement> window;
        Check(uia->ElementFromHandle(hwnd, &window));
        Require(window != nullptr);

        // 1) Select the second tab and resolve that tab's content pane.
        auto tab = FindFirstByControlType(uia, window, UIA_TabControlTypeId);
        auto secondTabItem = FindChildAt(uia, tab, UIA_TabItemControlTypeId, 1);
        ATL::CComPtr<IUIAutomationSelectionItemPattern> tabSelection;
        Check(secondTabItem->GetCurrentPatternAs(UIA_SelectionItemPatternId, IID_PPV_ARGS(&tabSelection)));
        Require(tabSelection != nullptr);
        Check(tabSelection->Select());
        auto page = FindChildAt(uia, secondTabItem, UIA_PaneControlTypeId, 0);

        // 2) Select the fourth item of the nonvirtualized list in this page.
        auto list = FindFirstByControlType(uia, page, UIA_ListControlTypeId);
        auto fourthItem = FindChildAt(uia, list, UIA_ListItemControlTypeId, 3);
        ATL::CComPtr<IUIAutomationSelectionItemPattern> listSelection;
        Check(fourthItem->GetCurrentPatternAs(UIA_SelectionItemPatternId, IID_PPV_ARGS(&listSelection)));
        Require(listSelection != nullptr);
        Check(listSelection->Select());

        // 3) Focus the page's editor before preparing its final selection.
        auto editor = FindFirstByControlType(uia, page, UIA_DocumentControlTypeId);
        if (!editor) editor = FindFirstByControlType(uia, page, UIA_EditControlTypeId);
        Require(editor != nullptr);
        Check(editor->SetFocus());
        ATL::CComPtr<IUIAutomationTextPattern> textPattern;
        Check(editor->GetCurrentPatternAs(UIA_TextPatternId, IID_PPV_ARGS(&textPattern)));
        Require(textPattern != nullptr);
        ATL::CComPtr<IUIAutomationTextRange> document;
        Check(textPattern->get_DocumentRange(&document));
        Require(document != nullptr);
        ATL::CComPtr<IUIAutomationTextRange> paragraph;
        Check(document->Clone(&paragraph));
        Check(paragraph->MoveEndpointByRange(
            TextPatternRangeEndpoint_End, document, TextPatternRangeEndpoint_Start));
        int moved = 0;
        Check(paragraph->Move(TextUnit_Paragraph, 1, &moved));
        Require(moved == 1);
        int relativeToEnd = 0;
        Check(paragraph->CompareEndpoints(
            TextPatternRangeEndpoint_Start, document, TextPatternRangeEndpoint_End, &relativeToEnd));
        Require(relativeToEnd < 0);
        Check(paragraph->ExpandToEnclosingUnit(TextUnit_Paragraph));
        ATL::CComVariant readOnly;
        Check(paragraph->GetAttributeValue(UIA_IsReadOnlyAttributeId, &readOnly));
        Require(readOnly.vt == VT_BOOL && readOnly.boolVal == VARIANT_FALSE);
        Check(paragraph->ScrollIntoView(TRUE));
        Check(paragraph->Select());

        BOOL focused = FALSE;
        Check(editor->get_CurrentHasKeyboardFocus(&focused));
        Require(focused != FALSE && GetForegroundWindow() == hwnd);
        INPUT inputs[2] = {};
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_DELETE;
        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = VK_DELETE;
        inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
        Require(SendInput(2, inputs, sizeof(INPUT)) == 2);
        // The caller must observe and verify the resulting text change.
    }
```

C# (Windows desktop; reference `UIAutomationClient`, `UIAutomationTypes`, and `System.Windows.Forms`):

```csharp
    using System;
    using System.Runtime.InteropServices;
    using System.Threading;
    using System.Windows.Automation;
    using System.Windows.Automation.Text;
    using System.Windows.Forms;

    public static class UiaClientSample
    {
        [DllImport("user32.dll")]
        private static extern IntPtr GetForegroundWindow();

        private static AutomationElement FindFirstByControlType(AutomationElement root, ControlType type)
        {
            if (root == null) throw new InvalidOperationException("Missing search root.");
            return root.FindFirst(TreeScope.Descendants,
                new PropertyCondition(AutomationElement.ControlTypeProperty, type));
        }

        private static AutomationElement FindChildAt(AutomationElement root, ControlType type, int index)
        {
            if (root == null) throw new InvalidOperationException("Missing item container.");
            var children = root.FindAll(TreeScope.Children,
                new PropertyCondition(AutomationElement.ControlTypeProperty, type));
            if (index < 0 || index >= children.Count)
                throw new InvalidOperationException("The requested child is unavailable.");
            return children[index];
        }

        public static void SampleDriveFromHwnd(IntPtr hwnd)
        {
            // The caller runs this on an MTA worker that owns no windows.
            if (Thread.CurrentThread.GetApartmentState() != ApartmentState.MTA)
                throw new InvalidOperationException("An MTA worker is required.");
            var window = AutomationElement.FromHandle(hwnd);

            // 1) Select the second tab and resolve that tab's content pane.
            var tab = FindFirstByControlType(window, ControlType.Tab);
            var secondTabItem = FindChildAt(tab, ControlType.TabItem, 1);
            ((SelectionItemPattern)secondTabItem.GetCurrentPattern(SelectionItemPattern.Pattern)).Select();
            var page = FindChildAt(secondTabItem, ControlType.Pane, 0);

            // 2) Select the fourth item of the nonvirtualized list in this page.
            var list = FindFirstByControlType(page, ControlType.List);
            var fourthItem = FindChildAt(list, ControlType.ListItem, 3);
            ((SelectionItemPattern)fourthItem.GetCurrentPattern(SelectionItemPattern.Pattern)).Select();

            // 3) Focus the editor before preparing its final selection.
            var editor =
                FindFirstByControlType(page, ControlType.Document) ??
                FindFirstByControlType(page, ControlType.Edit);
            if (editor == null) throw new InvalidOperationException("Missing editor.");
            editor.SetFocus();
            var text = (TextPattern)editor.GetCurrentPattern(TextPattern.Pattern);
            var document = text.DocumentRange;
            var paragraph = document.Clone();
            paragraph.MoveEndpointByRange(
                TextPatternRangeEndpoint.End, document, TextPatternRangeEndpoint.Start);
            if (paragraph.Move(TextUnit.Paragraph, 1) != 1 ||
                paragraph.CompareEndpoints(
                    TextPatternRangeEndpoint.Start, document, TextPatternRangeEndpoint.End) >= 0)
            {
                throw new InvalidOperationException("The second paragraph is unavailable.");
            }
            paragraph.ExpandToEnclosingUnit(TextUnit.Paragraph);
            if (!(paragraph.GetAttributeValue(TextPattern.IsReadOnlyAttribute) is bool readOnly) || readOnly)
                throw new InvalidOperationException("The paragraph must be writable.");
            paragraph.ScrollIntoView(true);
            paragraph.Select();

            if (!editor.Current.HasKeyboardFocus || GetForegroundWindow() != hwnd)
                throw new InvalidOperationException("The editor must retain foreground and keyboard focus.");
            SendKeys.SendWait("{DEL}");
            // The caller must observe and verify the resulting text change.
        }
    }
```

Range APIs used here: [native MoveEndpointByRange](https://learn.microsoft.com/en-us/windows/win32/api/uiautomationclient/nf-uiautomationclient-iuiautomationtextrange-moveendpointbyrange), [managed TextPatternRange](https://learn.microsoft.com/en-us/dotnet/api/system.windows.automation.text.textpatternrange). Neither API exposes a `Collapse` method; moving the end endpoint to the document's start creates the initial degenerate range.
