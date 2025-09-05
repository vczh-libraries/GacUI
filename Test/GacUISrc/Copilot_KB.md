# !!!KNOWLEDGE BASE!!!

# DOCUMENT REQUEST

## TOPIC

GacUI has a very powerful layout system, implemented by combinations of GuiGraphicsCompositions subclass. Please analysis the source code and answer:

- The overall structure of layout, whan a layout is triggered, what happens?
- There are three subclass of GuiGraphicsComposition, named after GuiGraphicsComposition_*, explain them.
- How layout is triggered
- A composition constraints its children, and children can also constraint the parent itself, how does this work
- How many predefined layout is offered in GacUI
- What should we do if we want to add a new composition, to make it fit into the current structure

### Insight

Overview
The layout system centers on the abstract base class `GuiGraphicsComposition` (in `GuiGraphicsComposition.h`) and its three classification subclasses: `GuiGraphicsComposition_Trivial`, `GuiGraphicsComposition_Controlled`, `GuiGraphicsComposition_Specialized`. Concrete layout containers (table / stack / flow / shared size / side aligned / partial view / window / simple bounds) derive from one of these. Item / cell / splitter / child helper compositions used only inside a particular container usually derive from the Controlled variant.

1. Core Data & Caches
Each `GuiGraphicsComposition` stores:
- Children list (`children`), parent pointer (`parent`)
- Owned element (`ownedElement`)
- Visibility & interaction flags (`visible`, `transparentToMouse`)
- Layout configuration (`internalMargin`, `preferredMinSize`, `minSizeLimitation`)
- Cached results: `cachedMinSize`, `cachedBounds` (updated only through `Layout_SetCachedMinSize` / `Layout_SetCachedBounds`)
- Host context (`relatedHostRecord`) so RenderTarget changes propagate (`UpdateRelatedHostRecord`).

2. Layout Algorithm Skeleton
The abstract interface requires overrides for:
- `Layout_CalculateMinSize()`
- `Layout_CalculateMinClientSizeForParent(Margin parentInternalMargin)`
- `Layout_CalculateBounds(Size parentSize)`
Generic orchestration lives in `GuiGraphicsComposition_Layout.cpp`:
- `Layout_CalculateMinSizeHelper()` implements the shared policy: start from `preferredMinSize`, merge element renderer min size (if `minSizeLimitation != NoLimit`), add `internalMargin`, call every child `child->Layout_UpdateMinSize()`, then if `minSizeLimitation == LimitToElementAndChildren` accumulate each child contribution via `child->Layout_CalculateMinClientSizeForParent(internalMargin)`.
- `Layout_UpdateMinSize()` calls virtual `Layout_CalculateMinSize()` then stores via `Layout_SetCachedMinSize()` (which raises `CachedMinSizeChanged`).
- `Layout_UpdateBounds(parentSize)` sets `cachedBounds` using `Layout_CalculateBounds(parentSize)` then recurses into children with the new own size.
- `ForceCalculateSizeImmediately()` triggers a synchronous pass (min size then bounds) starting at this node (used e.g. after splitter drag in `GuiTableSplitterCompositionBase::OnMouseMoveHelper`).

3. Rendering vs Layout
Rendering (`GuiGraphicsComposition::Render`) only uses `cachedBounds`; it never recalculates layout. Layout must be up to date before a render pass that needs new geometry; this is ensured by explicit calls (`ForceCalculateSizeImmediately`) from controls altering layout-affecting properties (e.g. table splitter drag, dynamic size adjustments) and by higher-level window size updates (host logic calls into compositions to recalc when client size changes). Thus layout is pull-driven (explicit recalculation) with invalidation flags prompting the next calculation.

4. When a Layout is Triggered
A composition invalidates itself (and schedules a future render) by calling `InvokeOnCompositionStateChanged()` (publicly surfaced indirectly through many property setters). Sources:
- Hierarchy mutations: `InsertChild`, `RemoveChild`, `MoveChild` call `InvokeOnCompositionStateChanged()`.
- Property setters: `SetInternalMargin`, `SetPreferredMinSize`, `SetVisible`, `SetMinSizeLimitation`, etc. each call `InvokeOnCompositionStateChanged()` (sometimes with `forceRequestRender` for visibility changes).
- Owned element changes: `SetOwnedElement` resets renderer target then invalidates.
- Container-specific state changes: e.g. `GuiTableComposition::SetRowOption`, `SetColumnOption`, `SetCellPadding`, `SetBorderVisible`, `SetRowsAndColumns`; `GuiStackComposition::SetDirection`, `SetPadding`, `EnsureVisible`; `GuiFlowComposition::SetRowPadding`, `SetAxis`, etc.; each ends with `InvokeOnCompositionStateChanged()`.
- Child min size changes propagate upward: item compositions attach to `CachedMinSizeChanged` (e.g. `GuiCellComposition` / `GuiStackItemComposition`) to set parent layout invalid flags (`layout_invalid = true`).
Actual recomputation happens when: (a) an explicit `ForceCalculateSizeImmediately()` is called (e.g. table splitter drag, programmatic size enforcement); (b) a higher level container (host / window) decides to recalc (e.g. during resize); (c) some containers lazily recalc inside their own `Layout_CalculateMinSize()` when a local `layout_invalid` flag is seen (table / stack / flow patterns).

5. Three Classification Subclasses
- `GuiGraphicsComposition_Trivial`: Free-standing container; responsible for computing children min sizes and bounds itself using the standard helper. (E.g. `GuiBoundsComposition`, `GuiTableComposition`, `GuiStackComposition`, `GuiFlowComposition`, `GuiSharedSizeRootComposition` derive from or through classes that ultimately inherit this.)
- `GuiGraphicsComposition_Controlled`: Its size & position are wholly decided by the parent. It overrides `Layout_CalculateMinSize()` to return existing `cachedMinSize` (no auto recompute) and `Layout_CalculateBounds()` to return existing `cachedBounds`. Parent directly writes its cache via internal methods (e.g. `GuiCellComposition::Layout_SetCellBounds`, `GuiStackItemComposition::Layout_SetStackItemBounds`, `GuiFlowItemComposition::Layout_SetFlowItemBounds`). Child still can affect parent min size indirectly when the parent queries it / recalculates. These controlled items typically set `MinSizeLimitation = LimitToElementAndChildren` so the parent can measure intrinsic content before positioning.
- `GuiGraphicsComposition_Specialized`: General container but with a custom bounds algorithm; still computes min size using `Layout_CalculateMinSizeHelper()` yet constrains how much it feeds back to parent by overriding `Layout_CalculateMinClientSizeForParent()` to `{0,0}` (meaning it does not enlarge parent beyond its own element/preferred size). Examples: `GuiSideAlignedComposition`, `GuiPartialViewComposition`, `GuiWindowComposition`.

6. Parent Constrains Children / Children Constrain Parent
Mechanism is two-directional:
- Parent -> Child: In `Layout_UpdateBounds` parent passes its own `cachedBounds.GetSize()` into each child’s `Layout_UpdateBounds`, the child computes position based on its specific layout algorithm (absolute bounds, table cell aggregation, stack position, flow wrapping, etc.). Controlled children skip their own calculation and rely on parent writing them via specialized setters.
- Child -> Parent: During parent `Layout_CalculateMinSizeHelper()`, after updating each child’s min size (`child->Layout_UpdateMinSize()`), the parent optionally reads each child’s desired client size contribution via `child->Layout_CalculateMinClientSizeForParent(internalMargin)`. If the parent’s `minSizeLimitation == LimitToElementAndChildren`, these contributions enlarge the parent’s min size. Controlled compositions usually return `{0,0}` here (they are measured explicitly by their parent) while some trivial ones compute a real required size. Additional parent-specific aggregation occurs inside containers: e.g. `GuiTableComposition::Layout_CalculateMinSize()` accumulates row/column sizes (absolute, min, percentage) with padding and border; `GuiStackComposition::Layout_UpdateStackItemMinSizes()` computes a running `layout_stackItemTotalSize`; `GuiFlowComposition::Layout_UpdateFlowItemLayout` calculates wrapping geometry first to derive min size.

7. Internal Invalidation Flags
Major containers keep custom flags to avoid repeated heavy recomputation:
- Table: `layout_invalid`, `layout_invalidCellBounds`, plus arrays for offsets/sizes; flag set on structural or sizing changes; recompute row/column sizes and per-cell rectangles lazily inside `Layout_CalculateMinSize` / `Layout_CalculateBounds`.
- Stack: `layout_invalid`, recalculated in `Layout_UpdateStackItemMinSizes()`.
- Flow: `layout_invalid`, plus last virtual width to detect width changes for reflow.
- Shared Size Root: dictionaries of group widths/heights are rebuilt each `Layout_CalculateMinSize`.
Splitters (`GuiRowSplitterComposition`, `GuiColumnSplitterComposition`) adjust adjacent Absolute cells and then call `tableParent->ForceCalculateSizeImmediately()` for immediate feedback while dragging.

8. Predefined Layout Families (High-level Containers)
Counting distinct layout algorithms (excluding leaf/item helper compositions):
1. Bounds (`GuiBoundsComposition`) – absolute / anchored via `expectedBounds` + `alignmentToParent`.
2. Table (`GuiTableComposition`) – grid with Absolute / Percentage / MinSize row & column sizing, plus spanning cells and interactive splitters.
3. Stack (`GuiStackComposition`) – linear (4 directions including reversed) with optional ensure-visible scrolling-like adjustment.
4. Flow (`GuiFlowComposition`) – multi-line wrapping with per-row alignment (`FlowAlignment`) and baseline options per item (`GuiFlowOption`).
5. Shared Size (`GuiSharedSizeRootComposition`) – groups items to equalize width and/or height across arbitrary subtrees.
6. Side Aligned (`GuiSideAlignedComposition`) – dock style to one parent edge with max length / ratio constraints.
7. Partial View (`GuiPartialViewComposition`) – viewport / page-ratio based sub-rectangle selection (used by scroll or zoom scenarios).
8. Window Root (`GuiWindowComposition`) – anchors client area to native window size.
(Helper/controlled types: `GuiCellComposition`, `GuiRowSplitterComposition`, `GuiColumnSplitterComposition`, `GuiStackItemComposition`, `GuiFlowItemComposition`, `GuiSharedSizeItemComposition` implement participation inside the above families.)

9. Layout Trigger Sequence (Detailed)
Example: Changing a column percentage in a table:
- `GuiTableComposition::SetColumnOption` stores new option then calls `InvokeOnCompositionStateChanged()`.
- That sets `layout_invalid = true` (via `GuiTableComposition::OnCompositionStateChanged()`), and if attached to a host, schedules a render (`GuiGraphicsHost::RequestRender()` inside `InvokeOnCompositionStateChanged`).
- Before next visual usage requiring accurate geometry: `Layout_CalculateMinSize()` sees `layout_invalid`, recomputes row/column base sizes (`Layout_UpdateCellBoundsInternal`), then on bounds calculation `Layout_CalculateBounds()` (if size changed or first pass) updates cell rectangles & each `GuiCellComposition` via `Layout_SetCellBounds()`.
- Rendering traverses compositions; each cell’s children now render with correct updated `cachedBounds`.
Interactive drag (splitter): `GuiTableSplitterCompositionBase::OnMouseMoveHelper` adjusts adjacent Absolute sizes, calls parent setters (each invalidates) and immediately `ForceCalculateSizeImmediately()` to keep UI responsive.

10. Adding a New Composition (Integration Steps)
A new layout container should:
- Choose classification: 
  * If parent will directly set child bounds (children are passive slots) derive child items from `GuiGraphicsComposition_Controlled` and container from `GuiGraphicsComposition_Trivial` or `GuiGraphicsComposition__Specialized` depending on whether it wants to participate in parent min-size accumulation.
  * If container computes its own min size from children, derive from `GuiGraphicsComposition__Trivial` or `GuiGraphicsComposition__Specialized` and implement the three layout virtuals.
- Store any container-specific invalidation flags; set them inside overridden `OnCompositionStateChanged()`.
- Override `Layout_CalculateMinSize()`:
  * Use `Layout_CalculateMinSizeHelper()` when standard accumulation (preferred + element + children + margin) suffices, then augment if needed.
  * OR replicate pattern in table/stack/flow to pre-measure controlled children first, manually calling `child->Layout_SetCachedMinSize(child->Layout_CalculateMinSizeHelper())` before aggregating.
- Override `Layout_CalculateBounds()` to compute placement; for complex multi-phase (e.g., table) separate into helper methods and call only when a dirty flag is set.
- Decide whether children may enlarge parent: implement `Layout_CalculateMinClientSizeForParent()` accordingly (return `{0,0}` to suppress; compute adjusted size for anchor/docking scenarios if needed).
- For controlled item types, implement helper methods like `Layout_Set<YourItem>Bounds` and expose any item-specific properties that cause parent invalidation.
- Use `InvokeOnCompositionStateChanged()` in every setter that changes geometry-affecting state.
- If interactive resizing is needed, mirror splitter pattern: handle mouse events via `GetEventReceiver()`, mutate model, call `ForceCalculateSizeImmediately()`.
- Attach to child `CachedMinSizeChanged` when the container must react to inner content changes (see `GuiCellComposition` / `GuiStackItemComposition`).

11. Summary of Constraint Flow
- Upward (child -> parent): by returning a non-zero from `Layout_CalculateMinClientSizeForParent` OR by parent-specific manual accumulation of controlled children. Ultimately affects `cachedMinSize` via `Layout_CalculateMinSizeHelper` or custom code, raising `CachedMinSizeChanged`.
- Downward (parent -> child): by invoking each child’s `Layout_UpdateBounds` after container-specific geometry computation; controlled children simply receive precomputed rectangles via internal setters.

12. Distinctions Between Layout Families
- Absolute/Anchored (Bounds): direct coordinates or parent-edge alignment via `alignmentToParent` margin semantics.
- Structured Grid (Table): multi-axis proportional & intrinsic sizing with spanning and interactive resizing.
- Linear (Stack): order + padding + optional ensure-visible virtual offset (horizontal/vertical + reversed).
- Wrapping (Flow): width-constrained repositioning, row alignment, and baseline alignment customization per item.
- Group Sizing (SharedSizeRoot): second-pass normalization of item min sizes by group name & chosen dimensions.
- Edge Docking (SideAligned): occupy a slice of parent edge with length clamped by max length / ratio.
- Viewport (PartialView): project a fractional sub-rectangle of parent client area for scrolling/paging.
- Root (Window): adapt to native window client size; other compositions attach beneath.

13. Proof References (selected key methods / fields)
- Invalidations: `GuiGraphicsComposition::InvokeOnCompositionStateChanged`, property setters (`SetInternalMargin`, `SetPreferredMinSize`, etc.), container setters (`GuiTableComposition::SetRowOption`, `GuiStackComposition::SetDirection`).
- Layout orchestration: `Layout_CalculateMinSizeHelper`, `Layout_UpdateMinSize`, `Layout_UpdateBounds`, `ForceCalculateSizeImmediately`.
- Table specifics: `GuiTableComposition::Layout_CalculateMinSize`, `Layout_UpdateCellBoundsInternal`, `Layout_UpdateCellBoundsPercentages`, `Layout_UpdateCellBoundsOffsets`, `GuiCellComposition::Layout_SetCellBounds`.
- Stack specifics: `GuiStackComposition::Layout_UpdateStackItemMinSizes`, `Layout_CalculateMinSize`, `Layout_UpdateStackItemBounds`, `GuiStackItemComposition::Layout_SetStackItemBounds`.
- Flow specifics: `GuiFlowComposition::Layout_UpdateFlowItemLayout`, `Layout_UpdateFlowItemLayoutByConstraint`, `GuiFlowItemComposition::Layout_SetFlowItemBounds` (indirectly set via flow parent helper).
- Shared size specifics: `GuiSharedSizeRootComposition::CalculateOriginalMinSizes`, `CollectSizes`, `AlignSizes`; item `GuiSharedSizeItemComposition::Layout_CalculateMinSize` conditional behavior when attached to a root.
- Splitter interaction: `GuiTableSplitterCompositionBase::OnMouseMoveHelper` (calls cell option setters then `ForceCalculateSizeImmediately`).
- Controlled items neutral layout overrides: in `GuiGraphicsComposition_Controlled` overridden `Layout_CalculateMinSize`, `Layout_CalculateBounds` simply return cached values.

This captures the end-to-end architecture, triggers, classification semantics, bidirectional constraints, available built-in layout families, and integration steps for extending the system.