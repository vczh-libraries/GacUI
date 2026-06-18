# GacUI Knowledge Base

Project introduction remains in [Index.md](./Index.md#gacui).

### Choosing APIs

#### Remote Protocol Unit Test Framework

Testing GacUI applications without real OS windows or rendering, using the remote protocol architecture with a mock renderer (`UnitTestRemoteProtocol`) that captures rendering snapshots and simulates user input.

- Use `GacUIUnitTest_Initialize` and `GacUIUnitTest_Finalize` for global test executable setup and teardown.
- Use `GacUIUnitTest_SetGuiMainProxy` to register frame-based test callbacks per test case.
- Use `GacUIUnitTest_LinkGuiMainProxy` for decorator-style proxy chaining to compose setup layers.
- Use `GacUIUnitTest_StartFast_WithResourceAsText<Theme>` for the most common entry point that compiles XML resources, registers themes, creates windows, and runs the application.
- Use `GacUIUnitTest_Start` and `GacUIUnitTest_StartAsync` for synchronous and async protocol stack tests.
- Use `OnNextIdleFrame(name, callback)` on `UnitTestRemoteProtocol` to register frame callbacks; the name describes the rendering result, not the upcoming action.
- Use `LocationOf(controlOrComposition)` to compute absolute screen coordinates for input simulation.
- Use `LClick`, `RClick`, `MClick`, `LDBClick`, `MouseMove` for mouse input simulation.
- Use `KeyPress`, `KeyDown`, `KeyUp`, `TypeString` for keyboard input simulation.
- Use `TryFindObjectByName<T>(window, name)` to look up named controls from GacUI XML resources.
- Use `GetApplication()->InvokeInMainThread` to defer IO actions that would trigger blocking functions like `ShowDialog`.

[API Explanation](./KB_GacUI_RemoteProtocolUnitTestFramework.md)

### Design Explanation

#### Platform Initialization and Multi-Platform Architecture

- GacUI implements a sophisticated multi-platform initialization system that provides consistent API across different operating systems and rendering backends while maintaining platform-specific optimizations.
- The initialization process follows a layered architecture from platform entry points through renderer setup to application framework. It supports:
  - Windows Direct2D/GDI
  - Linux GTK
  - macOS Cocoa
  - remote rendering for testing
  - hosted mode for embedded applications.
- The system uses a consistent naming pattern `Setup[Platform][Renderer][Mode]()` with standard mode providing full application framework, hosted mode running within a single native window, and raw mode bypassing GuiApplication entirely.
- Key features include hardware acceleration fallbacks, comprehensive error handling, frame-based unit testing through remote mode, and systematic service registration with proper dependency management.

[Design Explanation](./KB_GacUI_Design_PlatformInitialization.md)

#### Main Window and Modal Window Management

- GacUI provides a sophisticated multi-layered window management architecture that enables consistent main window and modal window behavior across all supported platforms while maintaining platform-specific optimizations.
- The application loop operates through hierarchical delegation from `GuiApplication` layer down to platform-specific `INativeWindowService` implementations, supporting Windows native, hosted mode, and remote mode environments.
- Modal windows achieve apparent "blocking" behavior without actually blocking the underlying event processing system through an event-driven callback architecture that maintains full user interaction capabilities.
- The system supports three modal window variants: `ShowModal` for basic modal behavior, `ShowModalAndDelete` for automatic cleanup, and `ShowModalAsync` for modern async/await integration patterns.
- Cross-platform consistency is maintained through unified modal APIs, continuous message loop processing, and platform-optimized implementations that abstract differences while providing rich windowing capabilities.

[Design Explanation](./KB_GacUI_Design_MainWindowModalWindow.md)

#### Implementing IGuiGraphicsElement

- Defines element lifecycle via `IGuiGraphicsElement`, `IGuiGraphicsRenderer`, renderer factories, and render targets.
- Uses `GuiElementBase<T>` pattern with property change notifications through `InvokeOnElementStateChanged` for invalidation and size recalculation.
- Renderer abstraction supplies hooks (`InitializeInternal`, `FinalizeInternal`, `RenderTargetChangedInternal`, `Render`, `OnElementStateChanged`, `GetMinSize`) and caches platform resources.
- Parallel renderer families per backend (Direct2D, GDI, Remote/Hosted) registered in backend initialization via static `Register()`.
- Composition + host rendering pipeline traverses compositions, applies clippers, calls element renderers; invalidation chain from property setter to `GuiGraphicsHost::Render`.
- Provides checklist, lifecycle summary, common pitfalls, and distinction from complex `GuiDocumentElement` model-based rendering architecture.

[Design Explanation](./KB_GacUI_Design_ImplementingIGuiGraphicsElement.md)

#### Layout and GuiGraphicsComposition

- Core layout system centered on `GuiGraphicsComposition` with measurement (`Layout_CalculateMinSize`) and arrangement (`Layout_CalculateBounds`) passes driven only by host render loop invalidation.
- Three subclass archetypes (`_Trivial`, `_Controlled`, `_Specialized`) define ownership of size calculation and parent-child constraint propagation.
- Eight predefined layout families (Bounds, Table, Stack, Flow, Shared Size, Side Aligned, Partial View, Window Root) plus responsive (`GuiResponsive*`) adaptive level-based system.
- Bidirectional constraints: parent supplies space; children optionally enlarge parent via `Layout_CalculateMinClientSizeForParent`; controlled items receive bounds from parent setters.
- Invalidation via `InvokeOnCompositionStateChanged`; `GuiGraphicsHost::Render` iteratively recalculates until stable; `ForceCalculateSizeImmediately` only for interactive latency.
- Responsive compositions add multi-level adaptive switching with aggregation strategies (View, Stack, Group, Fixed) and automatic container adjustment.

[Design Explanation](./KB_GacUI_Design_LayoutAndGuiGraphicsComposition.md)

#### Control Focus Switching and TAB/ALT Handling

- Three-layered architecture from composition focus (`GuiGraphicsHost`) through control focus (`GuiControl`) to automatic clearing on state changes.
- TAB navigation managed by `GuiTabActionManager` with `IGuiTabAction` service, prioritized control list building, wrapping navigation logic, and character suppression.
- ALT navigation managed by `GuiAltActionManager` with `IGuiAltAction` service, nested ALT host hierarchy (`IGuiAltActionHost`), visual label creation, and prefix-based key filtering.
- Critical `continue` barrier in `CollectAltActionsFromControl` blocks children from parent-level collection when control has ALT action, enabling nested context pattern.
- Custom `GetActivatingAltHost` implementations handle non-child relationships (menu popups), intentional blocking (combo boxes), dynamic content (grid editors), and scoped navigation (ribbon groups, date pickers).
- Event flow integration processes ALT before TAB in key event chain, with character suppression for both managers in character event chain.

[Design Explanation](./KB_GacUI_Design_ControlFocusSwitchingAndTabAltHandling.md)

#### List Control Architecture

- Three-layer architecture separates data management (`IItemProvider` with view system), layout arrangement (`IItemArranger` with virtual repeat composition), and visual rendering (item templates with background wrapping).
- Item lifecycle from creation (`InstallStyle`) through property updates to destruction (`UninstallStyle`) with event translation from compositions to item-level events via `ItemCallback`.
- Virtual repeat composition system delegates to `GuiVirtualRepeatCompositionBase` with four arranger types (free height, fixed height, fixed size multi-column, fixed height multi-column) supporting efficient virtualization.
- Provider hierarchy includes concrete providers (holding actual data), bindable providers (wrapping observable data sources via reflection), and converter providers (transforming tree to list via `NodeItemProvider`).
- Selection management in `GuiSelectableListControl` handles multi-selection with ctrl/shift modifiers, synchronizes with item templates, and provides keyboard navigation with special right-click behavior.
- Specialized controls (`GuiVirtualTextList`, `GuiVirtualListView`, `GuiVirtualTreeView`, `GuiVirtualDataGrid`, combo boxes, ribbon galleries) with view-specific templates and default implementations.
- Scroll view integration with size calculation (`QueryFullSize`), view updates (`UpdateView`), adopted size for responsive layouts, and lifecycle management (`OnRenderTargetChanged`).
- Template and arranger coordination through `SetStyleAndArranger` process with detach/clear/update/attach phases, `PredefinedListItemTemplate` pattern, and display item background wrapping.
- Data grid advanced features with visualizer system (cell rendering customization via decorator pattern), editor system (in-place editing with keyboard/mouse integration), sorter system (multi-level sorting with stable ordering), and filter system (row filtering with AND/OR/NOT composition).

[Design Explanation](./KB_GacUI_Design_ListControlArchitecture.md)

#### Adding a New Control

- Coordinated changes across control class definition, template system, theme management, reflection registration, and XML compiler integration.
- Control class inherits from `GuiControl` and `Description<T>`, specifies template type via macro, overrides lifecycle methods, attaches event handlers to `boundsComposition`, and defines events/properties.
- Template system with declaration in `GuiControlTemplates.h` using macro expansion, property definition macros, auto-generated implementations including getters/setters/change events.
- Inheritance pattern for derived controls using parent template as base, selective lifecycle override, and attachment to parent events instead of re-implementing handlers.
- Reflection registration in three steps: type list addition, control class registration with base/constructor/members, automatic template registration.
- XML loader registration via `ADD_TEMPLATE_CONTROL` or `ADD_VIRTUAL_CONTROL` for themed variants.
- Theme integration through `GUI_CONTROL_TEMPLATE_TYPES` macro generating `ThemeName` enum values.
- Minimal working example demonstrates complete lifecycle from class definition through template, theme, reflection, to XML loader registration.
- File modification checklist covers 10+ files across Controls, Templates, Application, Reflection, and Compiler directories.
- Header organization requires updates to `IncludeForward.h` and `IncludeAll.h` for proper compilation order.

[Design Explanation](./KB_GacUI_Design_AddingNewControl.md)

#### Hosted Mode Window Management

- Hosted mode runs the entire GUI application inside a single OS native window, virtualizing all sub-windows, dialogs, popups, and menus as graphics within that window.
- `GuiHostedController` is the central class wrapping a real native controller, implementing its own window manager, input dispatching, and service delegation while replacing the global `INativeController`.
- The window manager template `hosted_window_manager::WindowManager<T>` maintains z-ordered lists (ordinary and top-most), a parent-child tree, and handles activation, focus, hit testing, dragging, and resizing.
- `GuiHostedWindow` implements `INativeWindow` with a proxy pattern: `PlaceholderProxy` for unassigned windows, `MainProxy` delegating to the real native window, and `NonMainProxy` operating purely through the window manager.
- Input dispatching uses `HandleMouseCallback` templates with pluggable PreAction/GetSelectedWindow/PostAction strategies, and `HandleKeyboardCallback` routes to the active window.
- The rendering pipeline renders all hosted windows in a single begin/end session via `StartHostedRendering`/`StopHostedRendering`, with per-window offset via `GetRenderingOffset()`.
- Remote mode inherently requires hosted mode, with `GuiHostedController` wrapping `GuiRemoteController` in the same architecture.

[Design Explanation](./KB_GacUI_Design_HostedModeWindowManagement.md)

#### Remote Protocol Core Architecture

- Remote protocol mode separates GacUI into a core side (application logic) and a renderer side (rendering and OS services), communicating through `IGuiRemoteProtocol`.
- Messages flow core → renderer via `IGuiRemoteProtocolMessages`; events and responses flow renderer → core via `IGuiRemoteProtocolEvents`.
- `GuiRemoteMessages` provides synchronous batched request-response with auto-incrementing IDs and blocking `Submit()`.
- `GuiRemoteController` implements `INativeController` and all sub-services as virtual stubs: single window only, intentionally null clipboard/dialog services, synchronous key state queries.
- Connection lifecycle: `SetupRemoteNativeController` creates a layered stack (`GuiRemoteController` → `GuiHostedController` → resource managers), with connect/disconnect/reconnection handling that re-sends all window state.
- Rendering pipeline: element lifecycle via ID allocation, diff-based element updates, frame rendering flow (`StartRenderingOnNativeWindow` → traversal → `StopRenderingOnNativeWindow`), and measurement feedback loop (font heights, min sizes, image metadata, inline object bounds).
- `GuiRemoteGraphicsParagraph` handles rich text with run property system, incremental diff synchronization, delegated layout queries, and caret bounds caching.
- DOM diff layer (`GuiRemoteProtocolDomDiffConverter`) converts per-frame command streams into diffed tree structures.
- Protocol combinator and filter layers enable composable transformations and traffic optimization via `[@DropRepeat]`/`[@DropConsecutive]` annotations.
- Channel layer (`IGuiRemoteProtocolChannel`, `GuiRemoteProtocolAsyncChannelSerializer`) supports real remote deployment with async IO on a separate thread.

[Design Explanation](./KB_GacUI_Design_RemoteProtocolCoreArchitecture.md)

#### Remote Protocol Renderer and Serialization

- `GuiRemoteRendererSingle` is the renderer-side implementation that bridges `IGuiRemoteProtocol` to a real native window with actual graphics rendering, relying on an existing platform provider (e.g., Windows Direct2D).
- It implements `IGuiRemoteProtocol` to receive protocol messages and translates them into native element operations, and implements `INativeWindowListener`/`INativeControllerListener` to forward OS events back as protocol events.
- Rendering pipeline: receives `RequestRendererBeginRendering` with `OrdinaryElementDescVariant` updates, applies them to real graphics elements, renders the DOM tree in `GlobalTimer()`, and returns measurement feedback via `RespondRendererEndRendering`.
- Event forwarding coalesces high-frequency events (mouse move, wheel, key auto-repeat) and sends discrete events immediately; hit testing is performed locally by traversing the rendering DOM tree.
- Layered channel architecture for protocol serialization: `IGuiRemoteProtocol` ↔ `JsonObject` (via `GuiRemoteProtocolFromJsonChannel`/`GuiRemoteJsonChannelFromProtocol`) ↔ `WString` (via `JsonToStringSerializer`) ↔ user-implemented transport.
- JSON envelope format with `semantic`, `id`, `name`, `arguments` fields; protocol types code-generated from `Protocol/*.txt` with `JsonHelper<T>` specializations.
- `GuiRemoteProtocolAsyncChannelSerializer` provides thread separation (channel thread for IO, UI thread for application logic) with queued event delivery and connection-safe request matching.
- Demo project pair (`RemotingTest_Core` and `RemotingTest_Rendering_Win32`) demonstrates full protocol stack assembly for both core and renderer sides with named-pipe/HTTP transport.

[Design Explanation](./KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md)
