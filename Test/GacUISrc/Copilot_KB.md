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
Rendering (`GuiGraphicsComposition::Render`) only uses `cachedBounds`; it never recalculates layout. `InvokeOnCompositionStateChanged()` does NOT perform layout; it only marks invalidation flags on the affected compositions (and requests a render from the host). The primary mechanism that performs (and repeats if necessary) layout recomputation is `GuiGraphicsHost::Render`: the host render loop keeps calling itself; on each iteration, if any composition (or window) is flagged invalid, the host triggers a full layout pass for the window before traversing to render. This repeats until no layout-invalid flags remain, ensuring geometry is settled. `ForceCalculateSizeImmediately()` is therefore an optimization / special-case tool for immediate synchronous feedback (e.g. interactive splitter drag) but not required for normal property changes; without calling it, the next host render cycle will still recompute layout automatically.

4. When a Layout is Triggered
A composition invalidates itself (and schedules a future render) by calling `InvokeOnCompositionStateChanged()` (publicly surfaced indirectly through many property setters). Sources:
- Hierarchy mutations: `InsertChild`, `RemoveChild`, `MoveChild` call `InvokeOnCompositionStateChanged()`.
- Property setters: `SetInternalMargin`, `SetPreferredMinSize`, `SetVisible`, `SetMinSizeLimitation`, etc. each call `InvokeOnCompositionStateChanged()` (sometimes with `forceRequestRender` for visibility changes).
- Owned element changes: `SetOwnedElement` resets renderer target then invalidates.
- Container-specific state changes: e.g. `GuiTableComposition::SetRowOption`, `GuiTableComposition::SetColumnOption`, `GuiTableComposition::SetCellPadding`, `GuiTableComposition::SetBorderVisible`, `GuiTableComposition::SetRowsAndColumns`; `GuiStackComposition::SetDirection`, `GuiStackComposition::SetPadding`, `GuiStackComposition::EnsureVisible`; `GuiFlowComposition::SetRowPadding`, `GuiFlowComposition::SetAxis`, etc.; each ends with `InvokeOnCompositionStateChanged()`.
- Child min size changes propagate upward: item compositions attach to `CachedMinSizeChanged` (e.g. `GuiCellComposition` / `GuiStackItemComposition`) to set parent layout invalid flags (`layout_invalid = true`).
Actual recomputation happens when: (a) the host render loop (`GuiGraphicsHost::Render`) sees any invalidation flag and performs a full window layout pass (repeating until stable), or (b) an explicit `ForceCalculateSizeImmediately()` is called for immediate synchronous recalculation (mainly interactive scenarios), or (c) higher level window size changes force the host to invalidate and then recompute in the next render iteration, or (d) inside certain containers that lazily recompute sub-layouts when their virtual methods are invoked during the host-driven pass. Thus the normal path is host-driven; `ForceCalculateSizeImmediately()` is optional.

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
- `GuiTableComposition::SetColumnOption` stores new option then calls `InvokeOnCompositionStateChanged()` (flagging invalid state; no immediate layout work is done here).
- `InvokeOnCompositionStateChanged()` causes the host to be requested for a render; on the next `GuiGraphicsHost::Render` iteration the host detects invalid layout flags and performs the full window layout pass before painting. If further invalidations arise during that pass, the host will loop additional render/layout cycles until stable.
- Internally the first layout pass for the invalid table sees `layout_invalid`, recomputes row/column base sizes (`Layout_UpdateCellBoundsInternal` and related helpers). On bounds calculation `Layout_CalculateBounds()` (within the same host-driven cycle) it updates cell rectangles & each `GuiCellComposition` via `Layout_SetCellBounds()`.
- Rendering traversal then uses the updated `cachedBounds`.
Interactive drag (splitter): `GuiTableSplitterCompositionBase::OnMouseMoveHelper` adjusts adjacent Absolute sizes, calls parent setters (each invalidates) and may call `ForceCalculateSizeImmediately()` to obtain immediate synchronous recalculation between host frames for smoother drag feedback (though the host loop would eventually catch up even without it).

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

## ASK

In `3. Rendering vs Layout`, `GuiGraphicsComposition::Render` does only use `cachedBounds`, but the saying around `ForceCalculateSizeImmediately` is incorrect. The fact is that `InvokeOnCompositionStateChanged` is just setting a flag, `GuiGraphicsHost::Render` (a function that keep being called repeatedly until everything is settled down) will trigger the layout of the whole window if anything is flagged.

### Insight

Clarification Added
- The previous explanation implied `ForceCalculateSizeImmediately()` was a primary mechanism to ensure layout is updated before rendering. This has been corrected in points 3, 4, and 9 above.
- Correct Flow: Property / structural changes call `InvokeOnCompositionStateChanged()`, which ONLY sets invalidation flags (and may request a render). The real layout recomputation is performed inside the host render loop `GuiGraphicsHost::Render`, which repeatedly executes: (detect flags) -> (perform full window layout pass) -> (render) until no compositions remain invalid.
- Role of `ForceCalculateSizeImmediately()`: It is an auxiliary, synchronous shortcut used mainly for interactive operations (e.g., splitter dragging) where immediate recomputation inside the same input event handler improves responsiveness. Omitting it would not break correctness; the next host render cycle would still perform the needed layout.
- The updated sections (3, 4, 9) now explicitly describe: (a) invalidation via `InvokeOnCompositionStateChanged()`, (b) host-driven iterative layout resolution, (c) optional nature of `ForceCalculateSizeImmediately()`.

## ASK

Talk about the reactive family composition

### Insight

(Clarification: The phrase "reactive family composition" in the source corresponds to the "responsive" layout family implemented under classes prefixed `GuiResponsive*` in `GuiGraphicsResponsiveComposition.h / .cpp`. Below uses the term "responsive" because all related symbols use it.)

1. Overview & Purpose
The responsive family dynamically switches among multiple visual "views" (levels) of a UI section as available space changes, allowing progressive disclosure or compaction without rebuilding control state. Core abstract base: `GuiResponsiveCompositionBase`. Concrete orchestrators and strategies:
- `GuiResponsiveViewComposition`: Holds an ordered collection of alternative views (each is a `GuiResponsiveCompositionBase`) and optionally a set of "shared" controls that migrate between views to preserve state.
- `GuiResponsiveStackComposition`: Aggregates children linearly; total level is the sum of children levels (minus 1) and level changes adjust one child at a time (smallest or largest depending on direction) to fine-tune space usage.
- `GuiResponsiveGroupComposition`: Aggregates children in lockstep; its level is the max child level; level change commands broadcast to children so they uniformly expand/contract.
- `GuiResponsiveFixedComposition`: Single-level terminal (always level 0) that prevents further responsive descent.
- `GuiResponsiveSharedComposition`: Placeholder inside a particular view that installs a shared control chosen from `GuiResponsiveViewComposition::GetSharedControls`, enabling the same control instance (stateful) to appear in different views.
- `GuiResponsiveContainerComposition`: Wrapper adjusting the level (up/down) of a target responsive composition automatically based on the container's current size, comparing it with recorded min-size ranges.

2. Class Responsibilities & Relationships
- All responsive compositions inherit from `GuiBoundsComposition` (through `GuiResponsiveCompositionBase`), thus they participate in the generic layout system (min size + bounds) but add a higher-layer notion of levels.
- `GuiResponsiveCompositionBase` tracks hierarchy linkage to a parent responsive composition (field `responsiveParent`, set in `OnParentLineChanged` by scanning ancestors). Upward notifications propagate via `OnResponsiveChildLevelUpdated` so root triggers an invalidation (`InvokeOnCompositionStateChanged`).
- `GuiResponsiveViewComposition` owns:
  * `views` (list of `GuiResponsiveCompositionBase*`), inserted through `GuiResponsiveViewCollection` which enforces the view being unattached (`BeforeInsert`) and auto-adds the first as `currentView`.
  * `sharedControls` (controls reused across views) managed by `GuiResponsiveSharedCollection` with checks ensuring a shared control is not already parented.
  * `usedSharedControls` tracking presently installed shared controls to ensure safe destruction.
  * Events: `BeforeSwitchingView` fired with `GuiItemEventArgs.itemIndex` referencing index in `views` just before adding the new view.

3. Level Semantics
- Level numbering is 0..(LevelCount-1); 0 represents smallest (most compact) representation. Transition operations: `LevelUp` (towards larger view / higher index) and `LevelDown` (towards smaller / lower resource) across all derived classes.
- `GuiResponsiveViewComposition::LevelCount` equals the sum of component view level counts (filtered by direction compatibility) or 1 if no views; computed in `CalculateLevelCount` iterating `views` and adding either child's `GetLevelCount()` (if direction matches) or 1.
- `CurrentLevel` is computed in `CalculateCurrentLevel` by reverse iteration of `views` accumulating counts until reaching `currentView`, adding that view's inner current level.
- `LevelDown` implementation tries to delegate to `currentView->LevelDown()` if direction-allowed; if that fails (child already minimal) the view pointer is advanced to the next view in `views` (smaller form) with event emission and layout invalidation.
- `LevelUp` mirrors the above moving to previous view when the child cannot enlarge further.

4. Direction Filtering
- Every responsive composition has a `ResponsiveDirection direction` mask. Parent operations only consider children whose direction bits intersect parent's direction (`((vint)direction & (vint)child->GetDirection()) != 0`). This gating is applied in all `Calculate*` methods (macros `DEFINE_AVAILABLE` inside Stack and Group) to exclude orthogonal-dimension views from influencing level aggregation or change logic.

5. Aggregation Strategies
- Stack (`GuiResponsiveStackComposition`): `levelCount` = Σ(children.levelCount - 1) + 1 across available children; `currentLevel` = Σ(children.currentLevel). Level change chooses a single child to adjust by scanning available children to find the "best candidate" (largest or smallest size depending on levelDown vs levelUp) using current cached bounds dimension(s). If the chosen child cannot change level (already at boundary), it is ignored and search continues; success triggers recalculation and invalidation.
- Group (`GuiResponsiveGroupComposition`): `levelCount` = max(children.levelCount); `currentLevel` = max(children.currentLevel). Level change iterates children: for LevelDown, any child at or above target level attempts `LevelDown()` once; for LevelUp, children repeatedly attempt `LevelUp()` while at or below target level, attempting to bring them up collectively; then `CalculateCurrentLevel` and invalidate.
- Fixed: trivial constant single-level endpoint; level changes always return false.

6. Shared Control Installation Flow
- A `GuiResponsiveSharedComposition` attaches under some view hierarchy. Its `OnParentLineChanged` searches upward until finding a `GuiResponsiveViewComposition` (current view). When switching views the parent chain changes causing removal of the previously installed shared control composition and potential re-installation into new view via `SetSharedControl` (which asserts membership in view's `sharedControls`). This preserves runtime state (selection, scroll offsets, etc.) by retaining the control instance.

7. Automatic Sizing & Container Adaptor
- `GuiResponsiveContainerComposition` acts like a smart wrapper around ANY `GuiResponsiveCompositionBase` (set via `SetResponsiveTarget`). During `Layout_CalculateBounds` it compares current container size to recorded `minSizeLowerBound` / `minSizeUpperBound` (initially the min size of the highest / lowest level).
  * If size shrinks below `minSizeLowerBound` it loops `Layout_AdjustLevelDown` which repeatedly calls `responsiveTarget->LevelDown()` and updates bounds until size fits or minimal level hit.
  * If size grows beyond `minSizeUpperBound`, `Layout_AdjustLevelUp` attempts `LevelUp` ascending while verifying the container still accommodates (rolling back one step if overshoot detected). The ordering logic (`Layout_CompareSize`) can test X, Y or both depending on direction mask of the target (flags `testX`, `testY`).
  * After each adjustment it calls `responsiveTarget->Layout_UpdateMinSize()` updating cached minimum, refining upper/lower bounds for further iterations.

8. Event & Invalidation Propagation
- Any meaningful structural or level change ends with either `InvokeOnCompositionStateChanged()` (root) or parent-propagated `OnResponsiveChildLevelUpdated()` to eventually reach root and flag a layout invalidation. Root invalidation is then resolved by the normal `GuiGraphicsHost::Render` iterative layout cycles (no custom render path is added).
- `LevelCountChanged` and `CurrentLevelChanged` events are emitted in respective recalculation methods only upon value change (comparison of old vs new). `BeforeSwitchingView` occurs exactly once per cross-view transition (not when only internal level of same view changes).

9. Branching & Edge Conditions
- View insertion logic (first view) sets `currentView` and adds it as child (skipping level recalculations temporarily with `skipUpdatingLevels`).
- Removal logic forbids removing a view still parented, preventing inconsistent state.
- Shared control collection forbids insertion/removal while a shared control is currently installed in a view, guaranteeing no mid-use deletion.
- For Stack strategy, if all children refuse to change level (already at boundaries) the change operation returns false leaving current state intact.
- Container adaptor uses two directional comparisons: If either axis requires shrink/grow (depending on monitored axes) it triggers adjustment; ambiguous (equal) comparison leaves levels unchanged ensuring stability at boundary sizes.

10. Recursive / Hierarchical Behavior
- Responsive compositions can nest: each child (if itself a `GuiResponsiveCompositionBase`) updates parent via `OnResponsiveChildLevelUpdated` recursing upward until root. Level recalculation thus travels O(height) on each change; internal loops inside Stack / Group may traverse O(children) but only after candidate level adjustments.
- During a multi-level operation (e.g., container auto-upgrades multiple levels) each step updates min sizes to ensure monotonic bounds for termination. No unbounded recursion: each LevelUp / LevelDown either changes `currentLevel` or is blocked and loop stops.

11. Integration in Controls
- Ribbon infrastructure (e.g., `GuiRibbonGroup`, `GuiRibbonTabPage`) composes responsive entities: `GuiRibbonGroup` uses `GuiResponsiveStackComposition` plus `GuiResponsiveViewComposition` for collapsible large-image / compact forms. Switching is driven by container sized by table/stack layout above; optional user actions (window resize) propagate down to responsive container which adjusts to fit available space.
- `GuiBindableRibbonGalleryList` uses a specialized responsive layout (`GalleryResponsiveLayout`, not part of the generic responsive family) but also demonstrates pattern: recompute size offsets on `CachedBoundsChanged` and drive dynamic arrangement.

12. Extensibility Guidelines for Responsive Extensions
To add a new responsive strategy:
- Derive from `GuiResponsiveCompositionBase`.
- Implement `GetLevelCount`, `GetCurrentLevel`, `LevelDown`, `LevelUp` with rules; ensure they only return true when a level transition occurs, otherwise false (to allow parent fallback logic like in `GuiResponsiveViewComposition`).
- Override `OnResponsiveChildInserted/Removed` if aggregating children; maintain internal lists similar to Stack/Group.
- After any child-level changes, call base `OnResponsiveChildLevelUpdated` by overriding and calling aggregation recalculations before base to refresh caches.
- Use direction masking to filter applicability and maintain consistent semantics with existing strategies.
- Provide deterministic candidate selection for partial adjustments (like Stack’s size-based heuristic) if not all children adjust simultaneously.

13. Key Code Touch Points (proof references)
- Hierarchy linking: `GuiResponsiveCompositionBase::OnParentLineChanged` (ascend ancestor chain, set `responsiveParent`).
- Root invalidation after recalculation: `GuiResponsiveCompositionBase::OnResponsiveChildLevelUpdated` (no parent -> `InvokeOnCompositionStateChanged`).
- View switching & events: `GuiResponsiveViewComposition::LevelDown`, `LevelUp` (remove/add child, fire `BeforeSwitchingView`).
- Level aggregation: `GuiResponsiveViewComposition::CalculateLevelCount`, `CalculateCurrentLevel`; Stack variants: `GuiResponsiveStackComposition::CalculateLevelCount`, `CalculateCurrentLevel`; Group variants analogous; Fixed returns constants.
- Shared control migration: `GuiResponsiveSharedComposition::OnParentLineChanged` + `SetSharedControl` (assert membership, install control, manage `usedSharedControls`).
- Automatic container adaptation: `GuiResponsiveContainerComposition::Layout_CalculateBounds`, `Layout_AdjustLevelUp`, `Layout_AdjustLevelDown`, helper comparison `Layout_CompareSize`.

14. Summary
The responsive family layers a discrete level model atop generic composition layout, enabling adaptive UI representations selected either:
- Explicitly via direct `LevelUp/LevelDown` calls (e.g., code reacting to hints), or
- Implicitly via `GuiResponsiveContainerComposition` reacting to size changes.
Different aggregation strategies (View / Stack / Group / Fixed) define how composite levels are computed and transitioned, while shared compositions preserve control state across view switches. The design confines adaptation logic to responsive nodes while reusing the existing invalidation & layout pass orchestration, minimizing coupling with the core layout engine.