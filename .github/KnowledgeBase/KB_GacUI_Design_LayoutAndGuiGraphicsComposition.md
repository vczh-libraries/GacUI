# Layout and GuiGraphicsComposition (Core Layout & Responsive Families)

## 1. Scope & Purpose
This document explains the GacUI layout architecture centered on `GuiGraphicsComposition` and its classification subclasses, the predefined layout container families, bidirectional constraint propagation, invalidation & recomputation workflow driven by `GuiGraphicsHost::Render`, extension guidelines, and the responsive ("reactive") composition family (`GuiResponsive*`). It consolidates prior Q&A insights into a coherent design reference for future maintenance and extension.

## 2. Core Abstractions
### 2.1 Base Class: `GuiGraphicsComposition`
Responsibilities:
- Maintain hierarchy: `children`, `parent` (mutation via `InsertChild`, `RemoveChild`, `MoveChild`).
- Own a single graphics element: `ownedElement` (setter `SetOwnedElement`).
- Store presentation & interaction flags: `visible`, `transparentToMouse`.
- Store layout configuration: `internalMargin`, `preferredMinSize`, `minSizeLimitation`.
- Cache measurement: `cachedMinSize` (set only by `Layout_SetCachedMinSize`), and arrangement: `cachedBounds` (set only by `Layout_SetCachedBounds`).
- Propagate host context: `relatedHostRecord` via `UpdateRelatedHostRecord`.
- Provide virtual layout interface:
  * `Layout_CalculateMinSize()` (measurement)
  * `Layout_CalculateMinClientSizeForParent(Margin parentInternalMargin)` (child -> parent contribution)
  * `Layout_CalculateBounds(Size parentSize)` (arrangement)
- Orchestrate recursive layout passes via helpers:
  * `Layout_UpdateMinSize()` (calls overridden min size function then caches and fires `CachedMinSizeChanged`).
  * `Layout_UpdateBounds(Size parentSize)` (computes own bounds then recurses to children).
  * `Layout_CalculateMinSizeHelper()` shared policy used by most containers.
- Invalidation trigger: `InvokeOnCompositionStateChanged(forceRequestRender)` sets flags; does not directly perform layout.
- Synchronous optional recompute: `ForceCalculateSizeImmediately()` (used sparingly, e.g., interactive splitters).
- Rendering entry: `Render` consumes only `cachedBounds` (no layout recalculation inside rendering).

### 2.2 Classification Subclasses
Purpose: Distinguish measurement & arrangement control patterns.
1. `GuiGraphicsComposition_Trivial`
   - Container autonomously computes measurement and bounds using standard algorithm (often via `Layout_CalculateMinSizeHelper`).
   - Examples: `GuiBoundsComposition`, `GuiTableComposition`, `GuiStackComposition`, `GuiFlowComposition`, `GuiSharedSizeRootComposition` (directly or indirectly).
2. `GuiGraphicsComposition_Controlled`
   - Measurement / bounds set externally by parent; overrides `Layout_CalculateMinSize` & `Layout_CalculateBounds` to simply return cached values (no internal recompute). Parent writes caches using specialized setters (e.g., `GuiCellComposition::Layout_SetCellBounds`, `GuiStackItemComposition::Layout_SetStackItemBounds`, `GuiFlowItemComposition::Layout_SetFlowItemBounds`).
   - Typical for helper items: `GuiCellComposition`, splitters, stack items, flow items, shared size items.
3. `GuiGraphicsComposition_Specialized`
   - Container with custom arrangement while limiting child -> parent enlargement; usually still uses `Layout_CalculateMinSizeHelper` but overrides `Layout_CalculateMinClientSizeForParent` to `{0,0}`.
   - Examples: `GuiSideAlignedComposition`, `GuiPartialViewComposition`, `GuiWindowComposition`.

## 3. Layout Pass Orchestration
### 3.1 Measurement Flow
`Layout_UpdateMinSize()` executes:
1. Calls overridden `Layout_CalculateMinSize()`.
2. That implementation typically delegates to `Layout_CalculateMinSizeHelper()`:
   - Start from `preferredMinSize`.
   - If `minSizeLimitation != NoLimit`, merge element renderer minimum.
   - Add `internalMargin`.
   - For each child: `child->Layout_UpdateMinSize()`.
   - If own `minSizeLimitation == LimitToElementAndChildren`, incorporate each child's contribution via `child->Layout_CalculateMinClientSizeForParent(internalMargin)`.
3. Result stored by `Layout_SetCachedMinSize()` raising `CachedMinSizeChanged` for observers (parents, containers, special logic like stack/table invalidation flags).

### 3.2 Arrangement Flow
`Layout_UpdateBounds(parentSize)` executes:
1. Compute own rectangle with overridden `Layout_CalculateBounds(parentSize)`, store via `Layout_SetCachedBounds`.
2. For each child: call `child->Layout_UpdateBounds(ownSize)`.
3. Controlled children rely on parent precomputed rectangles delivered via container-specific setters instead of independent calculation.

### 3.3 Host-Driven Iteration
- Changes call `InvokeOnCompositionStateChanged()` marking layout invalid (and optionally requesting a render).
- `GuiGraphicsHost::Render` loops: if any layout-invalid flag is set for the window root subtree, perform a full layout (measurement + arrangement cascade), then render. Repeats until no invalidation remains.
- Ensures deterministic stabilization without requiring synchronous recalculation inside property setters.
- `ForceCalculateSizeImmediately()` is an optimization for immediate feedback inside event handlers (e.g., splitter dragging) but never required for correctness.

## 4. Invalidation Sources & Flags
### 4.1 Generic Triggers
- Structural: `InsertChild`, `RemoveChild`, `MoveChild`.
- Property mutations: `SetInternalMargin`, `SetPreferredMinSize`, `SetVisible`, `SetMinSizeLimitation`, alignment & anchor changes.
- Element replacement: `SetOwnedElement`.

### 4.2 Container-Specific Triggers
- Table: `SetRowOption`, `SetColumnOption`, `SetCellPadding`, `SetBorderVisible`, `SetRowsAndColumns` (set `layout_invalid`, `layout_invalidCellBounds`).
- Stack: `SetDirection`, `SetPadding`, scroll / ensure visible logic (sets `layout_invalid`).
- Flow: `SetAxis`, `SetRowPadding`, other adjustments setting `layout_invalid`.
- SharedSizeRoot: any group membership changes or measuring resets dictionaries.
- Splitters: mouse move adjusting adjacent option values then invalidating parent + optional immediate recompute.
- Responsive (later section): level transitions call `InvokeOnCompositionStateChanged` at root responsive node.

### 4.3 Child-Originated Propagation
- Container listens to children `CachedMinSizeChanged` to raise its own invalidation flags (e.g., table cell, stack item, flow item compositions) to recompute aggregated geometry lazily on next layout pass.

## 5. Bidirectional Constraints
### 5.1 Parent -> Child Constraint
- Parent supplies available size during `Layout_UpdateBounds` recursion.
- Specialized containers compute per-child rectangles first (table grid, stack linear offsets, flow line wrapping) then assign them (especially controlled children) using internal `Layout_Set...Bounds` helpers.

### 5.2 Child -> Parent Constraint
- During parent measurement, after calling each child's `Layout_UpdateMinSize`, parent optionally adds `child->Layout_CalculateMinClientSizeForParent(internalMargin)` to its accumulated client size if own `minSizeLimitation == LimitToElementAndChildren`.
- Controlled children often contribute `{0,0}` because parent already manually accounts for them (e.g., table computing row/column sizes; stack computing cumulative lengths; flow computing wrapping layout).

## 6. Predefined Layout Families
Distinct algorithms (excluding helper-only controlled items):
1. Bounds: `GuiBoundsComposition` (absolute rectangle + `alignmentToParent`).
2. Table: `GuiTableComposition` (mixed absolute / percentage / min, spanning, splitters via `GuiRowSplitterComposition`, `GuiColumnSplitterComposition`).
3. Stack: `GuiStackComposition` (directional linear, reversed variants, ensure-visible virtualization support via internal offset logic).
4. Flow: `GuiFlowComposition` (adaptive multi-row wrapping with `FlowAlignment` & `GuiFlowOption` baseline/alignment choices).
5. Shared Size: `GuiSharedSizeRootComposition` (cross-subtree size normalization for named groups via per-dimension dictionaries; item: `GuiSharedSizeItemComposition`).
6. Side Aligned: `GuiSideAlignedComposition` (dock to a specified side with length ratio / max constraints).
7. Partial View: `GuiPartialViewComposition` (scroll / viewport style fractional sub-rectangle selection with offsets).
8. Window Root: `GuiWindowComposition` (ties client area to native window size, root for layout passes).
Helper controlled participants: `GuiCellComposition`, `GuiStackItemComposition`, `GuiFlowItemComposition`, `GuiSharedSizeItemComposition`, `GuiRowSplitterComposition`, `GuiColumnSplitterComposition`.

## 7. Internal Optimization Flags & Lazy Strategies
- Table maintains `layout_invalid`, `layout_invalidCellBounds`, recalculating row/column metrics and cell rectangles only when flagged inside `Layout_CalculateMinSize` / `Layout_CalculateBounds` (helpers: `Layout_UpdateCellBoundsInternal`, `Layout_UpdateCellBoundsPercentages`, `Layout_UpdateCellBoundsOffsets`).
- Stack uses `layout_invalid`; recalculations centralized in `Layout_UpdateStackItemMinSizes` and arrangement in `Layout_UpdateStackItemBounds`.
- Flow uses `layout_invalid` plus last constraint width; layout regeneration in `Layout_UpdateFlowItemLayout` / `Layout_UpdateFlowItemLayoutByConstraint`.
- Shared Size re-derives `CalculateOriginalMinSizes`, `CollectSizes`, `AlignSizes` each pass (group dictionary rebuild).
- Responsive container (later) recalculates bounds iteratively while adjusting levels.

## 8. Responsive (Reactive) Composition Family
### 8.1 Goals
Add adaptive multi-level representations (compact to expanded) without duplicating control state, layering on top of existing layout passes and invalidation system.

### 8.2 Key Classes
- Abstract: `GuiResponsiveCompositionBase` (inherits `GuiBoundsComposition`).
- Strategy / aggregator:
  * `GuiResponsiveViewComposition` (collection of alternative view subtrees + shared control migration).
  * `GuiResponsiveStackComposition` (summation-based level aggregation; adjust one child per level change).
  * `GuiResponsiveGroupComposition` (max-based aggregation; synchronized level propagation across children).
  * `GuiResponsiveFixedComposition` (terminal level 0 node).
  * `GuiResponsiveSharedComposition` (placeholder that installs a control from view's `sharedControls`).
  * `GuiResponsiveContainerComposition` (auto-resizes target by dynamic LevelUp/LevelDown based on live size comparisons).

### 8.3 Level Model & Operations
- Each responsive subtree exposes `GetLevelCount()` and `GetCurrentLevel()`.
- Transition methods: `LevelUp()`, `LevelDown()` return bool (true if level changed) enabling parent fallbacks.
- `GuiResponsiveViewComposition` maintains `currentView` pointer; switching occurs only when leaf view can't further down/up inside itself (delegation to child fails) then advances to next/previous view; fires `BeforeSwitchingView` event before installing new view child.
- Aggregation formulas:
  * View: `levelCount` = ?(child.levelCount or 1 if direction mismatch); `currentLevel` computed by reverse accumulation.
  * Stack: `levelCount` = ?(children.levelCount - 1) + 1; `currentLevel` = ?(children.currentLevel); change selects best candidate child by size heuristic.
  * Group: `levelCount` = max(children.levelCount); `currentLevel` = max(children.currentLevel); change broadcasts attempts.
  * Fixed: constants (1,0).

### 8.4 Direction Filtering
- Each node has `ResponsiveDirection direction` mask; only children whose direction overlaps (`child->GetDirection()`) participate in counts & changes (macro-assisted filtering in Stack/Group code).

### 8.5 Shared Control Migration
- `GuiResponsiveSharedComposition` locates ancestor `GuiResponsiveViewComposition` in `OnParentLineChanged` and installs its designated shared control (validated membership in `sharedControls`).
- On view switch: old subtree removed, new view added, placeholders reinstall matching shared controls preserving internal widget state.

### 8.6 Automatic Adaptive Wrapper
- `GuiResponsiveContainerComposition` monitors actual bounds vs `minSizeLowerBound` / `minSizeUpperBound`.
- If shrinking below lower bound: loop `Layout_AdjustLevelDown` (calls target `LevelDown` + remeasure until fits or minimal).
- If expanding beyond upper bound: loop `Layout_AdjustLevelUp` (speculative raise then rollback if overshoot) using `Layout_CompareSize` to test width/height axes indicated by direction bits.
- After each level change, invokes `responsiveTarget->Layout_UpdateMinSize()` to refresh bounds used for subsequent comparisons.

### 8.7 Invalidation & Events
- Any level recalculation culminating at root triggers `InvokeOnCompositionStateChanged` (root determined when `responsiveParent == nullptr`).
- Events: `LevelCountChanged`, `CurrentLevelChanged` emitted only on value change; `BeforeSwitchingView` emitted just prior to replacing the active view child.

### 8.8 Extension Guidelines (Responsive)
- Subclass `GuiResponsiveCompositionBase`.
- Implement level introspection and transitions (`GetLevelCount`, `GetCurrentLevel`, `LevelUp`, `LevelDown`).
- Maintain internal child collections with insertion/removal hooks (`OnResponsiveChildInserted/Removed`) if aggregating.
- Propagate recalculation upward via `OnResponsiveChildLevelUpdated` after recomputing own counts.
- Apply direction filtering semantics uniformly (mask intersection tests).

## 9. Detailed Constraint Examples
### 9.1 Table Example (Column Percentage Change)
1. Setter `GuiTableComposition::SetColumnOption` updates model then calls `InvokeOnCompositionStateChanged()`.
2. Next `GuiGraphicsHost::Render` detects invalid layout, triggers full window layout.
3. Table `Layout_CalculateMinSize` sees `layout_invalid` -> recomputes metrics; `Layout_CalculateBounds` updates each `GuiCellComposition` via `Layout_SetCellBounds`.
4. Rendering consumes new `cachedBounds`.

### 9.2 Interactive Splitter Drag
1. Mouse move updates adjacent Absolute options.
2. Each option setter invalidates table.
3. Drag handler optionally calls `ForceCalculateSizeImmediately()` for immediate geometry update (without waiting for host loop) to achieve fluid cursor feedback.

## 10. Adding a New Layout Composition (Non-Responsive)
Checklist:
1. Choose subclass archetype (Trivial vs Specialized vs Controlled) based on whether parent or child owns measurement/arrangement logic.
2. Define internal dirty flags mirroring existing patterns (e.g., `layout_invalid`).
3. Implement measurement using either `Layout_CalculateMinSizeHelper()` or custom pass collecting controlled child min sizes first (as table/stack/flow do).
4. Implement arrangement; for multi-phase (size distribution + placement) separate helpers and guard with dirty flags.
5. Decide child -> parent influence by overriding `Layout_CalculateMinClientSizeForParent` (return `{0,0}` to suppress growth).
6. Provide internal setters for controlled children to assign bounds then mark them (e.g., `Layout_SetXItemBounds`).
7. Fire `InvokeOnCompositionStateChanged()` in every state-mutating setter.
8. Subscribe to children `CachedMinSizeChanged` where dynamic re-aggregation is required.
9. Use `ForceCalculateSizeImmediately()` only for real-time adjustments inside input handlers when latency matters.

## 11. Adding a New Responsive Strategy
Steps:
1. Subclass `GuiResponsiveCompositionBase`.
2. Maintain children list & direction mask; filter by direction overlaps.
3. Implement `GetLevelCount` / `GetCurrentLevel` with chosen aggregation formula.
4. Implement `LevelDown` / `LevelUp` returning false when already at boundary (enabling parent fallback switching logic in view/stack/group containers).
5. Invoke recalculation & root invalidation through `OnResponsiveChildLevelUpdated`.
6. Integrate with container adaptation if needed (work with `GuiResponsiveContainerComposition`).
7. Provide deterministic selection heuristics for partial adjustments (e.g., choose largest or smallest child) if not all children change simultaneously.

## 12. Separation of Concerns
- Layout computation (measurement + arrangement) is strictly segregated from rendering (`Render` side-effect-free wrt layout state).
- Invalidation is single entry (`InvokeOnCompositionStateChanged`) leaving scheduling & iteration to the host.
- Responsive system layers on top without altering the base pass algorithm; it only manipulates subtree structure and min-size outputs between passes.

## 13. Reliability & Stability Patterns
- Iterative host loop ensures eventual fixpoint when cascading invalidations occur (e.g., responsive level changes causing new size measurements).
- Dirty flag + lazy recomputation prevents redundant heavy recomputations (especially table & flow geometry).
- Controlled compositions decouple per-item geometry from measuring logic to simplify parent algorithms.

## 14. Proof Reference Index (Quick Lookup)
- Invalidation Core: `GuiGraphicsComposition::InvokeOnCompositionStateChanged`.
- Layout Core Helpers: `Layout_CalculateMinSizeHelper`, `Layout_UpdateMinSize`, `Layout_UpdateBounds`.
- Forced Immediate Recompute: `ForceCalculateSizeImmediately`.
- Table: `GuiTableComposition::{SetRowOption,SetColumnOption,Layout_CalculateMinSize,Layout_UpdateCellBoundsInternal,Layout_UpdateCellBoundsPercentages,Layout_UpdateCellBoundsOffsets}`, cell bounds setter `GuiCellComposition::Layout_SetCellBounds`.
- Stack: `GuiStackComposition::{SetDirection,Layout_UpdateStackItemMinSizes,Layout_CalculateMinSize,Layout_UpdateStackItemBounds}`, item setter `GuiStackItemComposition::Layout_SetStackItemBounds`.
- Flow: `GuiFlowComposition::{SetAxis,SetRowPadding,Layout_UpdateFlowItemLayout,Layout_UpdateFlowItemLayoutByConstraint}`, item setter `GuiFlowItemComposition::Layout_SetFlowItemBounds`.
- Shared Size: `GuiSharedSizeRootComposition::{CalculateOriginalMinSizes,CollectSizes,AlignSizes}`, item logic `GuiSharedSizeItemComposition::Layout_CalculateMinSize`.
- Splitters: `GuiTableSplitterCompositionBase::OnMouseMoveHelper`.
- Responsive Base: `GuiResponsiveCompositionBase::{OnParentLineChanged,OnResponsiveChildLevelUpdated}`.
- Responsive View: `GuiResponsiveViewComposition::{LevelDown,LevelUp,CalculateLevelCount,CalculateCurrentLevel}`.
- Responsive Stack: `GuiResponsiveStackComposition::{CalculateLevelCount,CalculateCurrentLevel,LevelDown,LevelUp}`.
- Responsive Group: `GuiResponsiveGroupComposition::{CalculateLevelCount,CalculateCurrentLevel,LevelDown,LevelUp}`.
- Responsive Fixed: `GuiResponsiveFixedComposition` trivial implementations.
- Responsive Shared: `GuiResponsiveSharedComposition::{OnParentLineChanged,SetSharedControl}`.
- Responsive Container: `GuiResponsiveContainerComposition::{Layout_CalculateBounds,Layout_AdjustLevelUp,Layout_AdjustLevelDown,Layout_CompareSize}`.

## 15. Summary
The GacUI layout engine employs a clean separation between invalidation, measurement, arrangement, and rendering, with composition classification enabling optimized parent-child cooperation patterns. Predefined container families cover common layout paradigms (grid, linear, wrapping, docking, viewport, shared sizing) while the responsive layer introduces adaptive multi-level UI transformation without complicating the core pass. Extension points are well-localized: new algorithms implement three virtual layout functions (or override contribution behavior), while responsive strategies implement discrete level semantics. Lazy flags and controlled item abstractions keep recomputation efficient, and host-driven iterative stabilization guarantees correctness even under cascading dynamic changes.