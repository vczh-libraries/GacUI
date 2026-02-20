# !!!PLANNING!!!

# UPDATES

## UPDATE
The execution plan is solid and minimal; proceed with the fix, but incorporate the following mandatory clarifications and safeguards.

Set `needRefresh = true` in `GuiRemoteRendererSingle::RequestRendererEndRendering(...)` to guarantee a repaint at the end of every completed frame, including frames whose DOM diff is empty or only contains measurement/element updates. If `RequestRendererUpdateElement_DocumentParagraph(...)` can be invoked without a corresponding DOM diff in real flows, make the additional `needRefresh = true` there mandatory (otherwise clearly document why it is safe to keep optional).

When filtering diffs in `GuiRemoteRendererSingle::RequestRendererRenderDomDiff(...)`:
- Ignore `Deleted` diffs for element nodes and their virtual parents (`domId % 4 == 0/1`) to tolerate partial-frame DOM streams.
- If a `Created` diff for such a node arrives but the node already exists client-side, convert it to `Modified`.
- CRITICAL: guard empty `renderingDomIndex` before calling `collections::BinarySearchLambda(&renderingDomIndex[0], ...)` (this is undefined behavior when empty).

Keep hit-test nodes (`domId % 4 == 2/3`) deletable as-is to avoid stale hit-testing and uncontrolled growth.

Document (as comments near the filter) the key lifecycle invariant that makes this safe: DOM diffs manage structure, while `RequestRendererDestroyed` (and `availableElements` checks in `Render()`) governs whether an element is actually renderable, preventing “zombie” rendering.

Verification: keep the existing multi-paragraph insert/top/middle/line-break/resize steps, and add an explicit “delete a paragraph” scenario (select + Delete/Backspace) to confirm true deletions still remove rendered content. Also spot-check intermediate states during rapid typing/Enter to ensure no partial-frame regression.

# AFFECTED PROJECTS

- Build the solution in folder REPO-ROOT\Test\GacUISrc
  - Run Test Project UnitTest (optional; can be skipped for this task because only the client renderer is changed)

# EXECUTION PLAN

## STEP 1: Make refresh scheduling frame-driven (client-side)

### Change

In `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`, ensure a redraw is scheduled when a rendering cycle ends, even if the DOM diff is empty.

Proposed change (minimal, deterministic):

```cpp
void GuiRemoteRendererSingle::RequestRendererEndRendering(vint id)
{
	events->RespondRendererEndRendering(id, elementMeasurings);
	elementMeasurings = {};
	fontHeightMeasurings.Clear();

	// Ensure the client repaints on every completed core frame.
	// Some frames may only contain element updates / measurements (or an empty dom diff).
	needRefresh = true;
}
```

`RequestRendererEndRendering(...)` is the per-frame boundary and already forces a refresh for the completed frame, so `RequestRendererUpdateElement_DocumentParagraph(...)` does not need to set `needRefresh` for correctness.

If we later confirm paragraph updates can be invoked outside a completed frame, add refresh here as well:

```cpp
void GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph(vint id, const remoteprotocol::ElementDesc_DocumentParagraph& arguments)
{
	...
	proc();
	needRefresh = true;
}
```

### Why

`GuiRemoteRendererSingle` currently repaints only when `needRefresh` is set via `CheckDom()` (DOM update/diff) or OS `Paint()`. If the core completes a frame but the produced dom diff is empty (or only element updates occur), the client can miss a redraw and leave stale / partially-cleared content.

---

## STEP 2: Tolerate partial-frame DOM diffs without “forgetting” previously rendered element nodes

### Observation (from current code)

- The core-side dom-diff converter (`Source\PlatformProviders\Remote\GuiRemoteProtocol_DomDiff.cpp`) builds a `RenderingDom` from whatever `RequestRendererRenderElement/BeginBoundary` commands occurred in that rendering cycle.
- `GuiRemoteRendererSingle` treats the latest `renderingDom` as a *complete* scene graph and redraws the whole window every time (`GlobalTimer` does `rt->StartRendering(); Render(renderingDom); rt->StopRendering();`).
- If a rendering cycle is clipped / partial (e.g. invalid region update), the generated DOM can omit earlier paragraphs.
- `DiffDom(...)` then emits `Deleted` diffs for omitted nodes; the client applies them via `UpdateDomInplace(...)`, so previously-existing paragraphs disappear from the client DOM and therefore never get rendered again.

### Change

In `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`, pre-process incoming diffs before applying them:

1. For **element nodes** and their **virtual parents** (IDs `(elementId<<2)+0` and `(elementId<<2)+1`, i.e. `domId % 4 == 0/1`), treat `Deleted` as a no-op (do not remove these nodes from the client DOM).
2. Because the core may later re-send the same node as `Created` (if it was omitted in an intermediate partial frame), convert `Created` → `Modified` **when the node already exists** in the client-side `renderingDomIndex`.

Proposed code sketch (kept local to the handler; no protocol change):

```cpp
void GuiRemoteRendererSingle::RequestRendererRenderDomDiff(const remoteprotocol::RenderingDom_DiffsInOrder& arguments)
{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererRenderDomDiff(const RenderingDom_DiffsInOrder&)#"
	CHECK_ERROR(renderingDom, ERROR_MESSAGE_PREFIX L"This function must be called after RequestRendererRenderDom.");

	remoteprotocol::RenderingDom_DiffsInOrder filtered;
	filtered.diffsInOrder = Ptr(new collections::List<remoteprotocol::RenderingDom_Diff>);

	if (arguments.diffsInOrder)
	{
		for (auto&& diff : *arguments.diffsInOrder.Obj())
		{
			// dom id encoding (GuiRemoteProtocolSchema_FrameOperations.h):
			// element: (elementId<<2)+0, parent-of-element: (elementId<<2)+1
			bool isElementOrElementParent = (diff.id != -1) && (diff.id % 4 == 0 || diff.id % 4 == 1);

			if (isElementOrElementParent && diff.diffType == remoteprotocol::RenderingDom_DiffType::Deleted)
			{
				// Partial-frame tolerance: do not drop existing element nodes just because
				// they are not present in this rendering cycle's command stream.
				// Invariant: DOM diffs manage structure; RequestRendererDestroyed + availableElements govern renderability.
				continue;
			}

			if (isElementOrElementParent && diff.diffType == remoteprotocol::RenderingDom_DiffType::Created)
			{
				// NOTE: guard empty before taking &renderingDomIndex[0]
				if (renderingDomIndex.Count() > 0)
				{
					vint insert = 0;
					auto found = collections::BinarySearchLambda(
						&renderingDomIndex[0],
						renderingDomIndex.Count(),
						diff.id,
						insert,
						[](const remoteprotocol::DomIndexItem& item, vint id) { return item.id <=> id; }
						);

					if (found != -1)
					{
						auto modified = diff;
						modified.diffType = remoteprotocol::RenderingDom_DiffType::Modified;
						filtered.diffsInOrder->Add(modified);
						continue;
					}
				}
			}

			filtered.diffsInOrder->Add(diff);
		}
	}

	UpdateDomInplace(renderingDom, renderingDomIndex, filtered);
	CheckDom();
#undef ERROR_MESSAGE_PREFIX
}
```

### Why

This keeps the client-side DOM stable across partial frames, preventing earlier paragraphs from being removed just because the core only emitted rendering commands for a subset of the window in that cycle.

The `Created`→`Modified` conversion is necessary to preserve `UpdateDomInplace(...)` invariants: if the client kept a node that the core thinks was deleted, the core may legitimately send `Created` for it later; the client must not assert-fail when applying such diffs.

---

## STEP 3: Keep hit-test DOM deletion behavior unchanged

### Change

Do **not** apply the “ignore deletions” rule to hit-test nodes (`domId % 4 == 2/3`). Keep them deletable as-is.

### Why

Hit-test nodes can be numerous and can change frequently with invalidation/clip changes; retaining stale hit-test nodes risks incorrect hit-testing/cursor results and memory growth. The rendering bug is specifically about element nodes (paragraph elements), so we scope the tolerance to element-related node ids only.

---

## STEP 4: Verification plan (manual + build)

### Build

- Stop debugger:
  - `& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1`
- Build:
  - `cd REPO-ROOT\Test\GacUISrc`
  - `& REPO-ROOT\.github\Scripts\copilotBuild.ps1`

### Manual rendering verification (remote client)

Use an existing remote rendering sample (e.g. `Test\GacUISrc\RemotingTest_Rendering_Win32`, referenced by the manual) to reproduce and verify:

1. Create a document with **3+ paragraphs** using Enter.
   - Expected: all paragraphs remain visible.
2. Insert a paragraph at the **top** (Enter at beginning).
   - Expected: all previous paragraphs remain visible and shift down; no overlaps.
3. Insert a paragraph in the **middle**.
   - Expected: paragraphs above and below are all visible; layout updates correctly.
4. Delete a paragraph (select + Delete/Backspace).
   - Expected: the paragraph disappears and does not reappear after subsequent typing/resize.
5. During rapid typing / pressing Enter, spot-check intermediate states (after Enter, before the next frame fully settles).
   - Expected: previously existing paragraphs remain visible; no partial-frame disappearance.
6. Use Ctrl+Enter (or the editor’s “line break” command) inside a paragraph.
   - Expected: multi-line rendering within the same paragraph still works (no regression).
7. Resize the window and scroll (if supported) to force multiple invalidations.
   - Expected: content remains consistent; no “only last paragraph” behavior.

### Optional automated verification

If desired (not required for this task), run unit tests:
- `cd REPO-ROOT\Test\GacUISrc`
- `& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Executable UnitTest`

# !!!FINISHED!!!

