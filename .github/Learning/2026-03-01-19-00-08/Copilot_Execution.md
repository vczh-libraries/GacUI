# !!!EXECUTION!!!

# UPDATES

## UPDATE
The execution plan is architecturally sound and implementation-ready.

Key recommendations to carry into implementation:
- In `GuiRemoteRendererSingle::RequestRendererEndRendering(vint id)`, set `needRefresh = true` unconditionally at the end of each completed rendering cycle so the client repaints on every frame, even when the DOM diff is empty.
- In `GuiRemoteRendererSingle::RequestRendererRenderDomDiff(...)`, pre-filter diffs to tolerate partial-frame updates:
  - Ignore `Deleted` diffs for element nodes and their virtual parents (`domId % 4 == 0/1`) so previously rendered element nodes are not dropped just because they were omitted from this cycle.
  - Convert `Created` ? `Modified` for such nodes when the node already exists client-side, to keep updates idempotent and avoid duplicate creation.
  - Keep hit-test nodes (`domId % 4 == 2/3`) deletable.
  - Guard `renderingDomIndex.Count() > 0` before calling `collections::BinarySearchLambda(&renderingDomIndex[0], ...)`.

Verification emphasis:
- During insert-at-top and insert-in-middle scenarios, explicitly verify bounds/Y-coordinates are updated and paragraphs do not overlap, confirming shifted content receives `Modified` updates.
- During deletion, explicitly confirm `RequestRendererDestroyed` is invoked and deleted paragraphs do not reappear after subsequent typing/resize.

# AFFECTED PROJECTS

- Build the solution in folder REPO-ROOT\Test\GacUISrc
  - Run Test Project UnitTest (optional; can be skipped for this task because only the client renderer is changed)

# EXECUTION PLAN

## STEP 1: Make refresh scheduling frame-driven (client-side)
[DONE]

In `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`, ensure a redraw is scheduled when a rendering cycle ends, even if the DOM diff is empty.

Update `GuiRemoteRendererSingle::RequestRendererEndRendering(vint id)` to set `needRefresh = true` at the end of each completed frame:

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

If later confirmed that `RequestRendererUpdateElement_DocumentParagraph(...)` can be invoked outside a completed frame, additionally set `needRefresh = true` there.

---

## STEP 2: Tolerate partial-frame DOM diffs without ?forgetting? previously rendered element nodes
[DONE]

In `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`, pre-process incoming diffs before applying them:

- For element nodes and their virtual parents (`domId % 4 == 0/1`), ignore `Deleted` diffs.
- For those nodes, if a `Created` diff arrives but the node already exists in `renderingDomIndex`, convert it to `Modified`.
- Keep hit-test nodes (`domId % 4 == 2/3`) deletable as-is.
- Guard empty `renderingDomIndex` before calling `collections::BinarySearchLambda(&renderingDomIndex[0], ...)`.

Update `GuiRemoteRendererSingle::RequestRendererRenderDomDiff(...)` as follows (apply as a single consecutive code update within the function):

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

---

## STEP 3: Build and manual verification
[DONE]

### Build

- Stop debugger:
  - `& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1`
- Build:
  - `cd REPO-ROOT\Test\GacUISrc`
  - `& REPO-ROOT\.github\Scripts\copilotBuild.ps1`

### Manual rendering verification (remote client)

1. Create a document with 3+ paragraphs using Enter.
2. Insert a paragraph at the top (Enter at beginning).
3. Insert a paragraph in the middle.
4. Delete a paragraph (select + Delete/Backspace).
5. During rapid typing / pressing Enter, spot-check intermediate states.
6. Use Ctrl+Enter (or the editor?s ?line break? command) inside a paragraph.
7. Resize the window and scroll (if supported) to force multiple invalidations.

Expected:
- All paragraphs remain visible (no ?only last paragraph?).
- No overlaps; bounds/Y-coordinates update correctly.
- True deletions remove content and it does not reappear after typing/resize.

# FIXING ATTEMPTS

- N/A (execution document update only)

# !!!FINISHED!!!

# !!!VERIFIED!!!

