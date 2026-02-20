# !!!PLANNING!!!

# UPDATES

## UPDATE

The execution plan for implementing client-side remote document paragraph support is well-structured and demonstrates strong architectural understanding. The `GuiRemoteDocumentParagraphElement` wrapper correctly bridges the gap between `IGuiGraphicsParagraph` and the element-based remote protocol. However, there is one critical defect and several important improvements required before implementation.

### Critical Defect: Missing State Persistence

The wrapper fails to cache all text styling information (fonts, colors, styles) needed to recreate the paragraph after `SetRenderTarget` destroys it. The plan only stores `text` and `inlineObjectProps`, but when `runsDiff` operations apply styles via `SetFont`, `SetColor`, etc., these are applied directly to the paragraph without caching in the wrapper. If the render target changes (e.g., DPI change, monitor switch), the paragraph is destroyed and all styling is permanently lost because the protocol only sends incremental diffs, not full state.

**Required Fix**: Add caching structures (e.g., `List<DocumentTextRun> cachedRuns`) to store all applied text styles. Implement a reconstruction method that replays all cached runs when the paragraph is recreated in `SetRenderTarget` or lazily in `Render`. Without this, text will render with incorrect or default styling after any render target change.

### Mandatory Corrections

1. **Full namespace prefixes in error messages**: All `CHECK_ERROR` messages must follow the learning convention with complete namespace paths: `L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::MethodName#Message"`. The current plan uses abbreviated prefixes which breaks grep-based debugging patterns.

2. **Encapsulation for `GetInlineObjectFromPoint`**: The handler directly accesses `wrapper->inlineObjectProps` internals. Add a public helper method `TryGetInlineObjectRunProperty(vint callbackId, DocumentInlineObjectRunProperty& outProp)` to maintain proper encapsulation boundaries.

3. **Header file specification**: Specify exactly where in `GuiRemoteRendererSingle.h` to add the `CreateRemoteDocumentParagraphElement()` factory method declaration, following member alignment rules from the coding guidelines.

4. **Null check for resource manager**: Add a check for `GetGuiGraphicsResourceManager()` returning null before calling `GetLayoutProvider()->CreateParagraph()` in `EnsureCreatedFromFirstUpdate`.

### Important Improvements

1. **Caret restoration logic**: The caret restoration in `EnsureCreatedFromFirstUpdate` may be unreachable since `text` is only valid on first update (paragraph should never be null during updates). Move caret restoration to `SetRenderTarget` where paragraph recreation actually occurs, or document why the defensive code is necessary.

2. **Unit test coverage**: Add dedicated unit tests for the wrapper class covering:
   - First update with text vs subsequent updates with diffs (protocol invariant enforcement)
   - Inline object add/remove/update ordering correctness
   - Caret persistence across render target recreation
   - Query handler behavior when paragraph doesn't exist
   - Measurement render pass populating bounds accurately
   - **Text style persistence across render target changes** (validates the critical fix)

3. **Documentation additions**:
   - Add comments explaining why `Initialize`/`Finalize` are intentionally empty
   - Document why `OnRenderInlineObject` returns `Size{}` for missing callback IDs (expected during initialization)
   - Explain the error handling philosophy: strict `CHECK_ERROR` in update paths vs lenient defaults in query paths
   - Add exception safety note for `StartRendering`/`StopRendering` in measurement pass

4. **Dictionary update pattern**: The `Contains()`/`Set()` vs `Add()` pattern appears three times for `inlineObjectBounds`, `inlineObjectProps`, and `inlineObjectRanges`. Consider extracting a helper function per the learning "Extract helpers to remove duplicated conversion blocks", or document the decision to keep the pattern inline if the helper would be over-engineering.

5. **Mojibake cleanup scope**: STEP 5 mentions normalizing encoding artifacts but doesn't specify affected files or search patterns. Either provide concrete file paths/grep patterns or remove this step if not critical to the implementation.

# AFFECTED PROJECTS

- Build the solution in folder Test\GacUISrc
  - Run Test Project UnitTest

# EXECUTION PLAN

## STEP 1: Add a paragraph wrapper element type (client-side)

### What to change

Implement a client-side `elements::IGuiGraphicsElement` wrapper for `elements::IGuiGraphicsParagraph` that can live in `GuiRemoteRendererSingle::availableElements` and be addressed purely by renderer id.

- Add a **factory method** on `vl::presentation::remote_renderer::GuiRemoteRendererSingle` (declared in `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle.h`, implemented in `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering_Document.cpp`):
  - `Ptr<elements::IGuiGraphicsElement> CreateRemoteDocumentParagraphElement();`
  - **Header placement** (for alignment + discoverability): declare it in the `protected:` helper-method region, right after `void CheckDom();` in `GuiRemoteRendererSingle.h`, using the existing tab-aligned column style.
- Implement the wrapper class in `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering_Document.cpp` (no new files).

### Why

- The remote protocol treats a document paragraph as a renderer element id (`RendererType::DocumentParagraph`), but `elements::IGuiGraphicsParagraph` is not an `elements::IGuiGraphicsElement`, so we need an element wrapper to participate in the existing element map + render pipeline.
- The wrapper is also the correct place to implement `elements::IGuiGraphicsParagraphCallback` so inline objects can be measured and bounds can be reported back in `UpdateElement_DocumentParagraphResponse.inlineObjectBounds`.

### Proposed wrapper shape (concrete code)

Add this implementation in `GuiRemoteRendererSingle_Rendering_Document.cpp`:

```cpp
namespace vl::presentation::remote_renderer
{
	using namespace collections;
	using namespace elements;
	using namespace remoteprotocol;

	class GuiRemoteDocumentParagraphElement
		: public Object
		, public IGuiGraphicsElement
		, protected IGuiGraphicsRenderer
		, protected IGuiGraphicsRendererFactory
		, protected IGuiGraphicsParagraphCallback
	{
	protected:
		GuiRemoteRendererSingle* 					owner = nullptr;
		compositions::GuiGraphicsComposition*	ownerComposition = nullptr;
		IGuiGraphicsRenderTarget*				renderTarget = nullptr;

		// Render-target-bound paragraph instance
		Ptr<IGuiGraphicsParagraph>				paragraph;

		// Persisted state to survive paragraph recreation (render target changes)
		WString									text;
		bool									hasText = false;
		Nullable<remoteprotocol::OpenCaretRequest>	caret;

		// Persisted full run state (critical: protocol sends diffs only, so we must cache full styling)
		elements::DocumentTextRunPropertyMap					textRuns;
		elements::DocumentInlineObjectRunPropertyMap		inlineObjectRuns;
		elements::DocumentRunPropertyMap						mergedRuns;

		// Callback + protocol response state
		Dictionary<vint, Rect>					inlineObjectBounds;
		Dictionary<vint, remoteprotocol::DocumentInlineObjectRunProperty>	inlineObjectProps;
		Dictionary<vint, elements::CaretRange>	inlineObjectRanges;

		void SetOwnerComposition(compositions::GuiGraphicsComposition* composition) override
		{
			ownerComposition = composition;
		}

	public:
		GuiRemoteDocumentParagraphElement(GuiRemoteRendererSingle* _owner)
			: owner(_owner)
		{
		}

		// ===== IGuiGraphicsElement =====
		IGuiGraphicsRenderer* GetRenderer() override
		{
			return this;
		}

		compositions::GuiGraphicsComposition* GetOwnerComposition() override
		{
			return ownerComposition;
		}

	protected:
		// ===== IGuiGraphicsRenderer =====
		IGuiGraphicsRendererFactory* GetFactory() override
		{
			return this;
		}

		void Initialize(IGuiGraphicsElement* element) override
		{
			// Intentionally empty: this renderer does not own extra per-element resources.
		}

		void Finalize() override
		{
			// Intentionally empty: see Initialize().
		}

		void SetRenderTarget(IGuiGraphicsRenderTarget* _renderTarget) override
		{
			if (renderTarget != _renderTarget)
			{
				renderTarget = _renderTarget;
				paragraph = nullptr;
				inlineObjectBounds.Clear();

				// Render target changed (DPI / device change): recreate using cached full state.
				RecreateParagraphIfPossible();
			}
		}

		void Render(Rect bounds) override
		{
			if (paragraph)
			{
				paragraph->Render(bounds);
			}
		}

		void OnElementStateChanged() override
		{
		}

		Size GetMinSize() override
		{
			return {};
		}

		// ===== IGuiGraphicsRendererFactory =====
		IGuiGraphicsRenderer* Create() override
		{
			CHECK_FAIL(L"vl::presentation::remote_renderer::GuiRemoteDocumentParagraphElement::Create()#Not supported.");
		}

		// ===== IGuiGraphicsParagraphCallback =====
		Size OnRenderInlineObject(vint callbackId, Rect location) override
		{
			// record bounds for UpdateElement_DocumentParagraphResponse.inlineObjectBounds
			if (inlineObjectBounds.Keys().Contains(callbackId))
			{
				inlineObjectBounds.Set(callbackId, location);
			}
			else
			{
				inlineObjectBounds.Add(callbackId, location);
			}

			vint index = inlineObjectProps.Keys().IndexOf(callbackId);
			if (index == -1) return {};
			return inlineObjectProps.Values()[index].size;
		}

	public:
		bool HasParagraph() const
		{
			return paragraph;
		}

		IGuiGraphicsParagraph* GetParagraph() const
		{
			return paragraph.Obj();
		}

		void RecreateParagraphIfPossible()
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteDocumentParagraphElement::RecreateParagraphIfPossible()#"
			if (!renderTarget) return;
			if (!hasText) return;

			auto resourceManager = GetGuiGraphicsResourceManager();
			CHECK_ERROR(resourceManager != nullptr, ERROR_MESSAGE_PREFIX L"GetGuiGraphicsResourceManager() returns null.");

			paragraph = resourceManager->GetLayoutProvider()->CreateParagraph(text, renderTarget, this);

			// Replay cached full runs so styling survives render-target recreation.
			mergedRuns.Clear();
			elements::MergeRuns(textRuns, inlineObjectRuns, mergedRuns);

			for (auto [range, props] : mergedRuns)
			{
				auto start = range.caretBegin;
				auto length = range.caretEnd - range.caretBegin;
				if (length <= 0) continue;

				props.Apply(Overloading(
					[&](const remoteprotocol::DocumentTextRunProperty& textProp)
					{
						paragraph->SetFont(start, length, textProp.fontProperties.fontFamily);
						paragraph->SetSize(start, length, textProp.fontProperties.size);

						IGuiGraphicsParagraph::TextStyle style = (IGuiGraphicsParagraph::TextStyle)0;
						if (textProp.fontProperties.bold) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Bold);
						if (textProp.fontProperties.italic) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Italic);
						if (textProp.fontProperties.underline) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Underline);
						if (textProp.fontProperties.strikeline) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Strikeline);
						paragraph->SetStyle(start, length, style);

						paragraph->SetColor(start, length, textProp.textColor);
						paragraph->SetBackgroundColor(start, length, textProp.backgroundColor);
					},
					[&](const remoteprotocol::DocumentInlineObjectRunProperty& inlineProp)
					{
						Ptr<IGuiGraphicsElement> background;
						if (inlineProp.backgroundElementId != -1)
						{
							vint index = owner->availableElements.Keys().IndexOf(inlineProp.backgroundElementId);
							CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"backgroundElementId not found.");
							background = owner->availableElements.Values()[index];
						}

						IGuiGraphicsParagraph::InlineObjectProperties props;
						props.size = inlineProp.size;
						props.baseline = inlineProp.baseline;
						props.breakCondition = (IGuiGraphicsParagraph::BreakCondition)inlineProp.breakCondition;
						props.callbackId = inlineProp.callbackId;
						props.backgroundImage = background;
						CHECK_ERROR(paragraph->SetInlineObject(start, length, props), ERROR_MESSAGE_PREFIX L"SetInlineObject failed.");
					}
				));
			}

			// Restore caret state after recreation.
			if (caret)
			{
				auto& c = caret.Value();
				paragraph->OpenCaret(c.caret, c.caretColor, c.frontSide);
			}
#undef ERROR_MESSAGE_PREFIX
		}

		bool TryGetInlineObjectRunProperty(vint callbackId, remoteprotocol::DocumentInlineObjectRunProperty& outProp) const
		{
			vint index = inlineObjectProps.Keys().IndexOf(callbackId);
			if (index == -1) return false;
			outProp = inlineObjectProps.Values()[index];
			return true;
		}

		void EnsureCreatedFromFirstUpdate(const remoteprotocol::ElementDesc_DocumentParagraph& arguments)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteDocumentParagraphElement::EnsureCreatedFromFirstUpdate(const remoteprotocol::ElementDesc_DocumentParagraph&)#"
			CHECK_ERROR(renderTarget != nullptr, ERROR_MESSAGE_PREFIX L"Render target is not available.");

			if (!paragraph)
			{
				// First update: capture text and reset cached run maps.
				if (arguments.text)
				{
					text = arguments.text.Value();
					hasText = true;
					textRuns.Clear();
					inlineObjectRuns.Clear();
					mergedRuns.Clear();
				}
				else
				{
					CHECK_ERROR(hasText, ERROR_MESSAGE_PREFIX L"First update must contain text.");
				}

				RecreateParagraphIfPossible();
			}
			else
			{
				CHECK_ERROR(!arguments.text, ERROR_MESSAGE_PREFIX L"Text is only allowed on first update.");
			}
#undef ERROR_MESSAGE_PREFIX
		}

		void ApplyUpdateAndFillResponse(const remoteprotocol::ElementDesc_DocumentParagraph& arguments, remoteprotocol::UpdateElement_DocumentParagraphResponse& response)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteDocumentParagraphElement::ApplyUpdateAndFillResponse(const remoteprotocol::ElementDesc_DocumentParagraph&, remoteprotocol::UpdateElement_DocumentParagraphResponse&)#"
			EnsureCreatedFromFirstUpdate(arguments);

			// Always apply layout settings
			paragraph->SetWrapLine(arguments.wrapLine);
			paragraph->SetMaxWidth(arguments.maxWidth);
			paragraph->SetParagraphAlignment(owner->GetAlignment(arguments.alignment));

			// Clear bounds; will be repopulated by a measurement render pass
			inlineObjectBounds.Clear();

			// 1) removedInlineObjects first (mirror UnitTestUtilities mock ordering)
			if (arguments.removedInlineObjects)
			{
				for (auto callbackId : *arguments.removedInlineObjects.Obj())
				{
					// Update cached inline object run map (source-of-truth for replay)
					for (auto [range, prop] : inlineObjectRuns)
					{
						if (prop.callbackId == callbackId)
						{
							CHECK_ERROR(elements::ResetInlineObjectRun(inlineObjectRuns, range), ERROR_MESSAGE_PREFIX L"ResetInlineObjectRun failed.");
							break;
						}
					}

					// Apply to the live paragraph and clear callback caches
					vint index = inlineObjectRanges.Keys().IndexOf(callbackId);
					CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"Removed inline object not found.");
					auto range = inlineObjectRanges.Values()[index];
					CHECK_ERROR(paragraph->ResetInlineObject(range.caretBegin, range.caretEnd - range.caretBegin), ERROR_MESSAGE_PREFIX L"ResetInlineObject failed.");
					inlineObjectRanges.Remove(callbackId);
					inlineObjectProps.Remove(callbackId);
					inlineObjectBounds.Remove(callbackId);
				}
			}

			// 2) apply runsDiff + update cached full state
			if (arguments.runsDiff)
			{
				for (auto&& run : *arguments.runsDiff.Obj())
				{
					elements::CaretRange range{ run.caretBegin, run.caretEnd };
					auto start = run.caretBegin;
					auto length = run.caretEnd - run.caretBegin;
					if (length <= 0) continue;

					run.props.Apply(Overloading(
						[&](const remoteprotocol::DocumentTextRunProperty& textProp)
						{
							// Cache (for render-target recreation)
							elements::DocumentTextRunPropertyOverrides overrides;
							overrides.textColor = textProp.textColor;
							overrides.backgroundColor = textProp.backgroundColor;
							overrides.fontFamily = textProp.fontProperties.fontFamily;
							overrides.size = textProp.fontProperties.size;

							IGuiGraphicsParagraph::TextStyle style = (IGuiGraphicsParagraph::TextStyle)0;
							if (textProp.fontProperties.bold) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Bold);
							if (textProp.fontProperties.italic) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Italic);
							if (textProp.fontProperties.underline) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Underline);
							if (textProp.fontProperties.strikeline) style = (IGuiGraphicsParagraph::TextStyle)((vint)style | (vint)IGuiGraphicsParagraph::TextStyle::Strikeline);
							overrides.textStyle = style;
							elements::AddTextRun(textRuns, range, overrides);

							// Apply to the live paragraph
							paragraph->SetFont(start, length, textProp.fontProperties.fontFamily);
							paragraph->SetSize(start, length, textProp.fontProperties.size);
							paragraph->SetStyle(start, length, style);
							paragraph->SetColor(start, length, textProp.textColor);
							paragraph->SetBackgroundColor(start, length, textProp.backgroundColor);
						},
						[&](const remoteprotocol::DocumentInlineObjectRunProperty& inlineProp)
						{
							// Cache (for render-target recreation)
							CHECK_ERROR(elements::AddInlineObjectRun(inlineObjectRuns, range, inlineProp), ERROR_MESSAGE_PREFIX L"AddInlineObjectRun failed.");

							// Capture background element id before mutating any maps
							Ptr<IGuiGraphicsElement> background;
							if (inlineProp.backgroundElementId != -1)
							{
								vint index = owner->availableElements.Keys().IndexOf(inlineProp.backgroundElementId);
								CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"backgroundElementId not found.");
								background = owner->availableElements.Values()[index];
							}

							IGuiGraphicsParagraph::InlineObjectProperties props;
							props.size = inlineProp.size;
							props.baseline = inlineProp.baseline;
							props.breakCondition = (IGuiGraphicsParagraph::BreakCondition)inlineProp.breakCondition;
							props.callbackId = inlineProp.callbackId;
							props.backgroundImage = background;
							CHECK_ERROR(paragraph->SetInlineObject(start, length, props), ERROR_MESSAGE_PREFIX L"SetInlineObject failed.");

							// Update callback caches for removals / hit-test / callback size
							auto cb = inlineProp.callbackId;
							if (inlineObjectProps.Keys().Contains(cb))
								inlineObjectProps.Set(cb, inlineProp);
							else
								inlineObjectProps.Add(cb, inlineProp);

							elements::CaretRange cbRange;
							cbRange.caretBegin = start;
							cbRange.caretEnd = start + length;
							if (inlineObjectRanges.Keys().Contains(cb))
								inlineObjectRanges.Set(cb, cbRange);
							else
								inlineObjectRanges.Add(cb, cbRange);
						}
					));
				}
			}

			// Maintain merged full state for replay (text styles persistence)
			mergedRuns.Clear();
			elements::MergeRuns(textRuns, inlineObjectRuns, mergedRuns);

			// Respond with size + inline object bounds
			response.documentSize = paragraph->GetSize();

			if (inlineObjectRuns.Count() > 0)
			{
				// Inline object bounds are only observable via OnRenderInlineObject; do a measurement render pass.
				renderTarget->StartRendering();
				paragraph->Render(Rect(Point(0, 0), response.documentSize));
				auto failure = renderTarget->StopRendering();
				(void)failure;

				if (inlineObjectBounds.Count() > 0)
				{
					response.inlineObjectBounds = Ptr(new Dictionary<vint, Rect>);
					CopyFrom(*response.inlineObjectBounds.Obj(), inlineObjectBounds);
				}
			}
#undef ERROR_MESSAGE_PREFIX
		}

		void OpenCaretAndStore(const remoteprotocol::OpenCaretRequest& arguments)
		{
			caret = arguments;
			if (paragraph)
			{
				paragraph->OpenCaret(arguments.caret, arguments.caretColor, arguments.frontSide);
			}
		}

		void CloseCaretAndStore()
		{
			caret.Reset();
			if (paragraph)
			{
				paragraph->CloseCaret();
			}
		}
	};

	Ptr<IGuiGraphicsElement> GuiRemoteRendererSingle::CreateRemoteDocumentParagraphElement()
	{
		return Ptr(new GuiRemoteDocumentParagraphElement(this));
	}
}
```

Notes embedded in code:
- We intentionally treat `arguments.text` after the paragraph exists as a protocol invariant break (fail fast), matching `GuiRemoteGraphicsParagraph::EnsureRemoteParagraphSynced()` behavior.
- We do `removedInlineObjects` before `runsDiff` to mirror `UnitTestUtilities\GuiUnitTestProtocol_Rendering_Document.cpp`.

## STEP 2: Create / store the wrapper in `availableElements`

### What to change

Update `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp` inside `GuiRemoteRendererSingle::RequestRendererCreated(...)` to create the wrapper when `rc.type == remoteprotocol::RendererType::DocumentParagraph`.

Concrete code change (add a switch case):

```cpp
case RendererType::DocumentParagraph:
	element = CreateRemoteDocumentParagraphElement();
	break;
```

Keep the existing behavior that immediately binds a render target:

```cpp
element->GetRenderer()->SetRenderTarget(GetGuiGraphicsResourceManager()->GetRenderTarget(window));
```

### Why

- This makes the paragraph resolvable by id in all paragraph-related protocol handlers (update + queries) without extra maps.
- It ensures the first update can immediately create a render-target-bound `IGuiGraphicsParagraph`.

## STEP 3: Implement `RendererUpdateElement_DocumentParagraph` (client)

### What to change

Replace the `CHECK_FAIL(L"Not implemented.")` in `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering_Document.cpp` for:

- `GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph`

Implementation requirements:
- Must run on the UI thread (render-target interaction + paragraph creation).
- Must resolve wrapper only by `arguments.id` from `availableElements`.
- Must respond with `remoteprotocol::UpdateElement_DocumentParagraphResponse` (size + inlineObjectBounds).

Concrete handler skeleton:

```cpp
void GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph(vint id, const ElementDesc_DocumentParagraph& arguments)
{
	auto proc = [&]()
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph(vint, const remoteprotocol::ElementDesc_DocumentParagraph&)#Paragraph not created.");

		auto wrapper = availableElements.Values()[index].Cast<GuiRemoteDocumentParagraphElement>();
		CHECK_ERROR(wrapper, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph(vint, const remoteprotocol::ElementDesc_DocumentParagraph&)#Unexpected element type.");

		UpdateElement_DocumentParagraphResponse response;
		wrapper->ApplyUpdateAndFillResponse(arguments, response);
		events->RespondRendererUpdateElement_DocumentParagraph(id, response);
	};

	auto async = GetCurrentController()->AsyncService();
	if (async->IsInMainThread(window))
	{
		proc();
	}
	else
	{
		async->InvokeInMainThreadAndWait(window, proc);
	}
}
```

### Why

- `UpdateElement_DocumentParagraphResponse` is used by core-side `GuiRemoteGraphicsParagraph::EnsureRemoteParagraphSynced()` to cache size and inline object bounds. Without this, paragraphs cannot be measured and hit-tested.
- UI-thread marshalling prevents subtle render-target usage races.

## STEP 4: Implement caret / hit-test query handlers

### What to change

Implement the following functions in `GuiRemoteRendererSingle_Rendering_Document.cpp` by forwarding to the wrapper’s `IGuiGraphicsParagraph`:

- `RequestDocumentParagraph_GetCaret`
- `RequestDocumentParagraph_GetCaretBounds`
- `RequestDocumentParagraph_GetInlineObjectFromPoint`
- `RequestDocumentParagraph_GetNearestCaretFromTextPos`
- `RequestDocumentParagraph_IsValidCaret`
- `RequestDocumentParagraph_OpenCaret`
- `RequestDocumentParagraph_CloseCaret`

### Why

- Core-side `GuiRemoteGraphicsParagraph` depends on these for editing/navigation.
- Returning defaults instead of failing for missing paragraphs makes the client more robust during transient states (e.g. render target recreation), while still allowing strict failures for true protocol misuse in update flows.

### Concrete mappings (code sketches)

All handlers should follow this structure:
- Marshal to UI thread using `AsyncService()->IsInMainThread(window)` + `InvokeInMainThreadAndWait(window, ...)`.
- Resolve wrapper by `arguments.id` from `availableElements`.

#### `GetCaret`

```cpp
void GuiRemoteRendererSingle::RequestDocumentParagraph_GetCaret(vint id, const GetCaretRequest& arguments)
{
	auto proc = [&]()
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestDocumentParagraph_GetCaret()#Paragraph not created.");
		auto wrapper = availableElements.Values()[index].Cast<GuiRemoteDocumentParagraphElement>();
		CHECK_ERROR(wrapper, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestDocumentParagraph_GetCaret()#Unexpected element type.");

		GetCaretResponse response;
		response.preferFrontSide = true;

		if (!wrapper->HasParagraph())
		{
			response.newCaret = arguments.caret;
		}
		else
		{
			bool preferFrontSide = true;
			auto caret = wrapper->GetParagraph()->GetCaret(arguments.caret, arguments.relativePosition, preferFrontSide);
			response.newCaret = caret;
			response.preferFrontSide = preferFrontSide;
		}
		events->RespondDocumentParagraph_GetCaret(id, response);
	};

	auto async = GetCurrentController()->AsyncService();
	if (async->IsInMainThread(window)) proc(); else async->InvokeInMainThreadAndWait(window, proc);
}
```

#### `GetCaretBounds`

```cpp
void GuiRemoteRendererSingle::RequestDocumentParagraph_GetCaretBounds(vint id, const GetCaretBoundsRequest& arguments)
{
	auto proc = [&]()
	{
		Rect bounds;
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index != -1)
		{
			auto wrapper = availableElements.Values()[index].Cast<GuiRemoteDocumentParagraphElement>();
			if (wrapper && wrapper->HasParagraph())
			{
				bounds = wrapper->GetParagraph()->GetCaretBounds(arguments.caret, arguments.frontSide);
			}
		}
		events->RespondDocumentParagraph_GetCaretBounds(id, bounds);
	};

	auto async = GetCurrentController()->AsyncService();
	if (async->IsInMainThread(window)) proc(); else async->InvokeInMainThreadAndWait(window, proc);
}
```

#### `GetNearestCaretFromTextPos`

Schema uses `GetCaretBoundsRequest`; treat `arguments.caret` as `textPos`.

```cpp
void GuiRemoteRendererSingle::RequestDocumentParagraph_GetNearestCaretFromTextPos(vint id, const GetCaretBoundsRequest& arguments)
{
	auto proc = [&]()
	{
		vint result = arguments.caret;
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index != -1)
		{
			auto wrapper = availableElements.Values()[index].Cast<GuiRemoteDocumentParagraphElement>();
			if (wrapper && wrapper->HasParagraph())
			{
				result = wrapper->GetParagraph()->GetNearestCaretFromTextPos(arguments.caret, arguments.frontSide);
			}
		}
		events->RespondDocumentParagraph_GetNearestCaretFromTextPos(id, result);
	};

	auto async = GetCurrentController()->AsyncService();
	if (async->IsInMainThread(window)) proc(); else async->InvokeInMainThreadAndWait(window, proc);
}
```

#### `IsValidCaret`

```cpp
void GuiRemoteRendererSingle::RequestDocumentParagraph_IsValidCaret(vint id, const IsValidCaretRequest& arguments)
{
	auto proc = [&]()
	{
		bool valid = false;
		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index != -1)
		{
			auto wrapper = availableElements.Values()[index].Cast<GuiRemoteDocumentParagraphElement>();
			if (wrapper && wrapper->HasParagraph())
			{
				valid = wrapper->GetParagraph()->IsValidCaret(arguments.caret);
			}
		}
		events->RespondDocumentParagraph_IsValidCaret(id, valid);
	};

	auto async = GetCurrentController()->AsyncService();
	if (async->IsInMainThread(window)) proc(); else async->InvokeInMainThreadAndWait(window, proc);
}
```

#### `GetInlineObjectFromPoint`

Return a `DocumentRun?` whose `props` is the inline-object property.

```cpp
void GuiRemoteRendererSingle::RequestDocumentParagraph_GetInlineObjectFromPoint(vint id, const GetInlineObjectFromPointRequest& arguments)
{
	auto proc = [&]()
	{
		Nullable<DocumentRun> result;

		vint index = availableElements.Keys().IndexOf(arguments.id);
		if (index != -1)
		{
			auto wrapper = availableElements.Values()[index].Cast<GuiRemoteDocumentParagraphElement>();
			if (wrapper && wrapper->HasParagraph())
			{
				vint start = 0;
				vint length = 0;
				auto props = wrapper->GetParagraph()->GetInlineObjectFromPoint(arguments.point, start, length);
				if (props)
				{
					vint cb = props.Value().callbackId;
					remoteprotocol::DocumentInlineObjectRunProperty inlineProp;
					if (wrapper->TryGetInlineObjectRunProperty(cb, inlineProp))
					{
						DocumentRun run;
						run.caretBegin = start;
						run.caretEnd = start + length;
						run.props = inlineProp;
						result = run;
					}
				}
			}
		}

		events->RespondDocumentParagraph_GetInlineObjectFromPoint(id, result);
	};

	auto async = GetCurrentController()->AsyncService();
	if (async->IsInMainThread(window)) proc(); else async->InvokeInMainThreadAndWait(window, proc);
}
```

Use `GuiRemoteDocumentParagraphElement::TryGetInlineObjectRunProperty(callbackId, outProp)` to avoid reaching into wrapper internals.

#### `OpenCaret` / `CloseCaret`

```cpp
void GuiRemoteRendererSingle::RequestDocumentParagraph_OpenCaret(const OpenCaretRequest& arguments)
{
	auto proc = [&]()
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestDocumentParagraph_OpenCaret()#Paragraph not created.");
		auto wrapper = availableElements.Values()[index].Cast<GuiRemoteDocumentParagraphElement>();
		CHECK_ERROR(wrapper, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestDocumentParagraph_OpenCaret()#Unexpected element type.");
		wrapper->OpenCaretAndStore(arguments);
	};

	auto async = GetCurrentController()->AsyncService();
	if (async->IsInMainThread(window)) proc(); else async->InvokeInMainThreadAndWait(window, proc);
}

void GuiRemoteRendererSingle::RequestDocumentParagraph_CloseCaret(const vint& arguments)
{
	auto proc = [&]()
	{
		vint index = availableElements.Keys().IndexOf(arguments);
		CHECK_ERROR(index != -1, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestDocumentParagraph_CloseCaret()#Paragraph not created.");
		auto wrapper = availableElements.Values()[index].Cast<GuiRemoteDocumentParagraphElement>();
		CHECK_ERROR(wrapper, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestDocumentParagraph_CloseCaret()#Unexpected element type.");
		wrapper->CloseCaretAndStore();
	};

	auto async = GetCurrentController()->AsyncService();
	if (async->IsInMainThread(window)) proc(); else async->InvokeInMainThreadAndWait(window, proc);
}
```

## STEP 5: Document hygiene (encoding / mojibake cleanup)

### What to change

When implementing the above code, ensure:
- All added `CHECK_ERROR` / `CHECK_FAIL` messages are plain ASCII + wide literals (`L"..."`) without mojibake artifacts.
- Limit the hygiene cleanup scope to the files touched by this task (avoid repo-wide churn):
  - `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering_Document.cpp`
  - `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`
  - `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle.h`
  - `Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_Document.txt`
- Before/after search patterns (example `rg` patterns):
  - `rg -n "\uFFFD|Ã|Â|â" <files>`
  - `rg -n "[^\x00-\x7F]" <files>` (only after confirming files are intended to be ASCII-only)

### Why

- This task is heavily grep-driven and relies on error message prefixes to debug protocol mismatches; mojibake breaks searchability and slows down follow-up maintenance.

## STEP 6: Test plan (repo tooling)

### Compile/build validation

Acceptance criteria requires compiling:
- `Test\GacUISrc\GacUISrc.sln`
- and ensuring `Test\GacUISrc\UnitTest\UnitTest.vcxproj` is included (solution build + unit test build coverage)

Using repo scripts only:

1) Stop debugger (safe if not running):

- `& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1`

2) Build solution:

- `cd Test\GacUISrc`
- `& REPO-ROOT\.github\Scripts\copilotBuild.ps1`
- Verify `REPO-ROOT\.github\Scripts\Build.log` ends with:
  - `Build succeeded.`
  - `0 Warning(s)` (warnings allowed if baseline has them, but note any new ones)
  - `0 Error(s)`

### Runtime-level sanity via unit test runner

Even if no end-to-end remote connection is available, run the unit test executable to catch missing symbols / handler wiring:

- `cd Test\GacUISrc`
- `& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Executable UnitTest`
- Verify `REPO-ROOT\.github\Scripts\Execute.log` ends with:
  - `Passed test files: X/X`
  - `Passed test cases: Y/Y`

### Focused regression checks

After build/test, do quick protocol/behavior spot-checks by reading code paths:
- `Source\PlatformProviders\Remote\GuiRemoteGraphics_Document.cpp`:
  - confirm core-side only sends `text` when `committedRuns.Count() == 0`.
- `Source\UnitTestUtilities\GuiUnitTestProtocol_Rendering_Document.cpp`:
  - confirm ordering: `text` (first update only) → layout props → `removedInlineObjects` → `runsDiff` → respond size/bounds.

# !!!FINISHED!!!

