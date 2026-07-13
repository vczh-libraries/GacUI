# Implementing IGuiGraphicsElement

This document explains the end-to-end design and required implementation steps for adding a new graphics element to GacUI (a lightweight element similar to `GuiSolidLabelElement`, not a complex document-like element). It is organized around the involved interfaces, lifecycle, renderer abstraction, platform integration, and the event / rendering pipeline.

## 1. Core Interfaces and Their Responsibilities

1. IGuiGraphicsElement
   - Declares `GetRenderer()` and `GetOwnerComposition()` plus the protected `SetOwnerComposition(compositions::GuiGraphicsComposition*)` (invoked only by `GuiGraphicsComposition` as its friend).
   - Every concrete element inherits (indirectly) from this interface through `GuiElementBase<TElement>`.


2. IGuiGraphicsRenderer
   - Owns the platform-dependent drawing logic for exactly one bound element (lifecycle: `Initialize(element)`, `Finalize()`).
   - Responds to state changes: `OnElementStateChanged()`.
   - Receives a render target with `SetRenderTarget(IGuiGraphicsRenderTarget*)` and draws via `Render(Rect bounds)`.
   - Supplies layout info through `GetMinSize()`.


3. IGuiGraphicsRendererFactory
   - Creates renderers. Registered in the global `GuiGraphicsResourceManager` through `RegisterRendererFactory()` indirectly by `GuiElementRendererBase<...>::Register()`.


4. IGuiGraphicsRenderTarget (and concrete subclasses per backend)
   - Encapsulates drawing surface, clip stack (`PushClipper`, `PopClipper`, `GetClipper`, `IsClipperCoverWholeTarget`).
   - Manages rendering phases (`StartRendering`, `StopRendering`, hosted variants) and reports device failures (`RenderTargetFailure`).
   

5. GuiGraphicsRenderTarget (base implementation)
   - Implements clip stack logic and rendering phase orchestration, deferring platform specifics to derived classes (e.g., Direct2D / GDI targets override native start/stop and resource creation behavior).


## 2. Element Class Pattern (`GuiElementBase<TElement>`) and Example

A lightweight element (e.g., `GuiSolidLabelElement`) follows this template pattern provided by `GuiElementBase<T>`:
- Static `ElementTypeName` literal (e.g., `L"SolidLabel"`).
- A protected constructor initializing default property values.
- Public getters/setters; setters compare old vs new and call `InvokeOnElementStateChanged()` only when a change occurs (see `GuiSolidLabelElement::SetText`, `SetColor`, etc.).
- Shape / format or content properties stored as POD or small structs (e.g., `Color`, `FontProperties`, `Alignment`, `WString`).
- No rendering code appears in the element itself; it only stores state.

Typical additional element examples in `GuiGraphicsElement.h / .cpp` (all using the same pattern):
- `GuiSolidBorderElement`, `Gui3DBorderElement`, `Gui3DSplitterElement` (two-color, directional),
- `GuiSolidBackgroundElement`, `GuiGradientBackgroundElement`,
- `GuiInnerShadowElement`,
- `GuiImageFrameElement`,
- `GuiPolygonElement`.
All rely on property change notification through `InvokeOnElementStateChanged()` to invalidate caches in their renderers.

## 3. Renderer Abstraction (`GuiElementRendererBase<TElement, TRenderer, TRenderTarget>`)

Renderer bases implement a consistent lifecycle with overridable hooks:
- `InitializeInternal()` / `FinalizeInternal()` for creating / releasing persistent resources (e.g., brushes, text formats, cached geometries).
- `RenderTargetChangedInternal(old, new)` for resources tied to the render target (re-create when target switches or becomes available; also release on null).
- `Render(Rect bounds)` does immediate drawing each frame.
- `OnElementStateChanged()` invalidates or rebuilds caches when properties changed.
- `GetMinSize()` supplied by the base (often updated via specific helper like `UpdateMinSize()` in label renderers after layout recomputation).

Examples:
- Direct2D: `GuiSolidLabelElementRenderer` caches `ID2D1SolidColorBrush`, `Direct2DTextFormatPackage`, `IDWriteTextLayout`, re-building in `CreateTextLayout()` when font, text, wrapping, or max width changes; updates min size with `UpdateMinSize()`.
- GDI: `GuiSolidLabelElementRenderer` caches `WinFont` and updates min width/height similarly.
- Shared brush helpers: `GuiSolidBrushElementRendererBase` and `GuiGradientBrushElementRendererBase` unify brush caching logic in Direct2D path.

## 4. Cross-Platform Renderer Families

Per backend a parallel renderer hierarchy exists sharing naming but differing resource types:
- Direct2D (`elements_windows_d2d`): uses `IWindowsDirect2DRenderTarget`, `ID2D1RenderTarget`, DirectWrite text formatting, gradient/radial brushes, effects (e.g., focus rectangle effect in `GuiFocusRectangleElementRenderer`).
- GDI (`elements_windows_gdi`): uses `IWindowsGDIRenderTarget`, `WinDC`, `WinFont`, `WinPen`, `WinBrush`, Uniscribe text shaping for complex paragraphs or colorized text.
- Remote / Hosted modes: (not shown here) follow analogous patterns; Hosted leverages whichever native renderer factories are active.

Each concrete renderer is registered exactly once through its static `Register()` created by the CRTP base and invoked inside platform bootstrap functions (`RendererMainDirect2D`, `RendererMainGDI`, or remote manager initialization).

## 5. Registration Flow

1. Application selects backend (e.g., Direct2D or GDI), calling `RendererMainDirect2D()` or `RendererMainGDI()`.
2. Inside these functions each renderer’s `Register()` is invoked (e.g., `GuiSolidLabelElementRenderer::Register()`).
3. `Register()` calls `GetGuiGraphicsResourceManager()->RegisterRendererFactory(ElementTypeName, factory)` linking element type to a factory.
4. When an element instance is created via `GuiElementBase<T>::Create()`, the resource manager looks up the factory and constructs a matching renderer, calls `Initialize(element)`.
5. When a composition later receives a render target, it propagates to bound elements’ renderers through `SetRenderTarget` (under control of composition tree traversal / host initialization).

## 6. Composition Ownership and Rendering Invocation

- A `GuiGraphicsComposition` holds at most one `IGuiGraphicsElement`. On attach it calls the element’s protected `SetOwnerComposition`.
- Rendering pipeline (`GuiGraphicsHost::Render()`):
  1. Host invokes `windowComposition->Render(offset)` recursively.
  2. Each composition pushes clipping, then (if it has an element) obtains `element->GetRenderer()->Render(bounds)`.
  3. Render target clipping stack managed by `GuiGraphicsRenderTarget::PushClipper` / `PopClipper` ensures nested composition visibility.
  4. After traversal `StopRendering()` finalizes; any `RenderTargetFailure` is processed (e.g., lost device or resize triggers re-creation of render target and re-run of render).

## 7. State Change Propagation and Invalidation Chain

1. Setter in element detects a change and calls `InvokeOnElementStateChanged()` (provided by `GuiElementBase<T>`).
2. That method calls the bound renderer’s `OnElementStateChanged()` so it can drop caches (brushes, layouts) or lazily refresh on next `Render`.
3. `InvokeOnElementStateChanged()` also raises composition invalidation causing `GuiGraphicsHost` to mark `needRender = true`.
4. The main loop / timer triggers `GuiGraphicsHost::GlobalTimer()`, which if `needRender` calls `Render()`.
5. Min size recalculation done inside renderer (e.g., `GuiSolidLabelElementRenderer::UpdateMinSize()`) influences subsequent layout passes.

## 8. Min Size Calculation Strategy

- Renderers compute intrinsic size based on current element state and cached layout objects.
- Example: label renderer (Direct2D or GDI) measures text (wrapping, font, multiline) and stores result so `GetMinSize()` returns consistent values until invalidated.
- Property changes that affect geometry (text, font, wrap flags) call `InvokeOnElementStateChanged()` which triggers `UpdateMinSize()` inside the renderer on next render/state change handling.

## 9. Resource Lifetime and Render Target Changes

- Persistent resources independent of target (e.g., cached last element values) retained across target switches.
- Target-bound resources (Direct2D brushes, text layouts, GDI pens/brushes, bitmaps) created in `InitializeInternal()` if target already set, or in `RenderTargetChangedInternal()` when a new target arrives.
- On target loss (device lost / resize reported via `RenderTargetFailure`), host re-acquires target; each renderer releases old target objects in `RenderTargetChangedInternal(old, nullptr)` then recreates after new target available.

## 10. Adding a New Lightweight Element (Checklist)

Element Class:
- Add class `GuiXxxElement : public GuiElementBase<GuiXxxElement>` with `static constexpr const wchar_t* ElementTypeName = L"Xxx"`.
- Define private/protected property fields and defaults in constructor.
- Implement getters and setters in `.cpp`; setters compare old/new and call `InvokeOnElementStateChanged()` on change only.

Renderer (per backend):
- Derive `GuiXxxElementRenderer` from `GuiElementRendererBase<GuiXxxElement, GuiXxxElementRenderer, IWindowsDirect2DRenderTarget>` (and analog for GDI / Remote).
- Implement hooks: `InitializeInternal()`, `FinalizeInternal()`, `RenderTargetChangedInternal(old,new)`, `Render(bounds)`, `OnElementStateChanged()`.
- Cache necessary graphics resources; rebuild only when affected element properties change.
- Compute / update min size (helper method like `UpdateMinSize()`).

Registration:
- Insert `GuiXxxElementRenderer::Register()` in each backend initialization (Direct2D: `RendererMainDirect2D`, GDI: `RendererMainGDI`, Remote: remote resource manager initialization).

Testing:
- Instantiate via XML `<Xxx>` mapping to `presentation::elements::GuiXxxElement` or directly create in C++ via `GuiXxxElement::Create()`.
- Verify property mutations trigger re-render (breakpoint or visual change) and min size recomputation.

## 11. Differences From Complex Elements (e.g., `GuiDocumentElement`)

`GuiDocumentElement` adds a multi-layer model / cache system beyond the lightweight pattern:
- Manages `DocumentModel` with paragraphs, runs, hyperlink packages, embedded objects instead of flat properties.
- Has nested renderer (`GuiDocumentElementRenderer`) implementing `IGuiGraphicsParagraphCallback` and paragraph caching (`ParagraphCache` array) with lazy `IGuiGraphicsParagraph` creation.
- Maintains selection, caret, inline objects, per-paragraph measurement arrays, and dynamic caret rendering utilities (`OpenCaret`, `CloseCaret`, `SetSelection` etc.).
- Invalidation granularity per paragraph (`NotifyParagraphUpdated`).
- Rendering enumerates paragraphs, reflows only needed ones (width-sensitive caching via `lastMaxWidth`).
- Thus: far more complex state synchronization path vs. single-primitive elements where all state sits directly on the element and renderer only depends on a small fixed property set.

## 12. Common Pitfalls

- Forgetting to compare old vs new value in setter: causes redundant invalidations and potential performance issues.
- Allocating target-bound resources in constructor instead of `InitializeInternal()` / `RenderTargetChangedInternal()` leads to null target usage or leaks.
- Not releasing resources in `FinalizeInternal()` or when `RenderTargetChangedInternal(new == nullptr)` => leaks on device reset.
- Failing to update min size after relevant property change (text/font/wrap) => layout flickers or stale size.
- Omitting `Register()` call => element silently renders nothing (renderer never created).
- Using element state directly inside `Render()` without caching expensive conversions (e.g., text layout) => per-frame overhead.

## 13. End-to-End Flow Summary

Creation: `GuiXxxElement::Create()` -> element object + renderer via factory -> `renderer->Initialize(element)`.
Attachment: Composition calls `element->SetOwnerComposition(this)` -> composition tree now owns element.
Render Target Acquisition: Composition tree traversal sets render target on each renderer (`SetRenderTarget`). `RenderTargetChangedInternal()` fires; resources created.
Property Change: Setter -> `InvokeOnElementStateChanged()` -> renderer invalidation -> host scheduled.
Frame Render: Host `GuiGraphicsHost::Render()` -> composition traversal -> each renderer `Render(bounds)` with valid clipper.
Shutdown: Renderer `Finalize()` (invokes `FinalizeInternal()`), composition releases element, element destructor runs.

## 14. Quick Implementation Template (Narrative)

1. Declare element with properties + static name.
2. Implement setters calling `InvokeOnElementStateChanged()` only on real changes.
3. Add renderer (per backend) caching required native resources.
4. In renderer `OnElementStateChanged()` mark internal dirty flags, rebuild brushes / layouts next use (or immediately) and update min size.
5. Register renderer in backend startup.
6. Test lifecycle: create window, add composition + element, mutate properties, resize window, switch backend if applicable.

## 15. Lightweight Element Implementation Checklist (Condensed)

- [ ] Element class added with `ElementTypeName`.
- [ ] Constructor sets defaults.
- [ ] All setters compare and call `InvokeOnElementStateChanged()`.
- [ ] Renderer(s) created for every required backend.
- [ ] `InitializeInternal` / `FinalizeInternal` implemented.
- [ ] `RenderTargetChangedInternal` re/creates target-bound resources.
- [ ] `Render` draws respecting bounds & element properties.
- [ ] `OnElementStateChanged` invalidates & triggers min size recalculation.
- [ ] Registration calls inserted in each backend entry point.
- [ ] Remote protocol enums / serialization (if remote supported) implemented.
- [ ] Manual / unit tests cover property changes & rendering.

This pattern ensures every simple element integrates uniformly into the composition, rendering, and resource management infrastructure provided by GacUI.
