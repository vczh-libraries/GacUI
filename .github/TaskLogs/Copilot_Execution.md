# !!!EXECUTION!!!

# UPDATES

# AFFECTED PROJECTS

- Build the solution in folder Test\GacUISrc
  - Run Test Project UnitTest

# EXECUTION PLAN

## STEP 1: Add factory method declaration (client) [DONE]

### File

- `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle.h`

### Change

In `class vl::presentation::remote_renderer::GuiRemoteRendererSingle`, in the `protected:` helper-method region, add the factory method declaration right after `void CheckDom();`:

```cpp
		remoteprotocol::ImageMetadata			CreateImage(const remoteprotocol::ImageCreation& arguments);
		void									CheckDom();
		Ptr<elements::IGuiGraphicsElement>	CreateRemoteDocumentParagraphElement();
```

## STEP 2: Implement the paragraph wrapper element (client) [DONE]

### File

- `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering_Document.cpp`

### Change

Add the wrapper implementation + factory method implementation (keep in this file; do not create new files):

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

## STEP 3: Create/store the wrapper in `availableElements` [DONE]

### File

- `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`

### Change

In `GuiRemoteRendererSingle::RequestRendererCreated(...)`, extend the `switch (rc.type)` to handle `RendererType::DocumentParagraph`:

```cpp
case RendererType::DocumentParagraph:
	element = CreateRemoteDocumentParagraphElement();
	break;
```

Keep the existing behavior that binds the render target immediately:

```cpp
element->GetRenderer()->SetRenderTarget(GetGuiGraphicsResourceManager()->GetRenderTarget(window));
```

## STEP 4: Implement paragraph update + query handlers [DONE]

### File

- `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering_Document.cpp`

### Change A: `RequestRendererUpdateElement_DocumentParagraph`

Replace the `CHECK_FAIL` body with:

```cpp
void GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph(vint requestId, const ElementDesc_DocumentParagraph& arguments)
{
	auto proc = [&]()
	{
		vint index = availableElements.Keys().IndexOf(arguments.id);
		CHECK_ERROR(index != -1, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph(vint, const remoteprotocol::ElementDesc_DocumentParagraph&)#Paragraph not created.");

		auto wrapper = availableElements.Values()[index].Cast<GuiRemoteDocumentParagraphElement>();
		CHECK_ERROR(wrapper, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph(vint, const remoteprotocol::ElementDesc_DocumentParagraph&)#Unexpected element type.");

		UpdateElement_DocumentParagraphResponse response;
		wrapper->ApplyUpdateAndFillResponse(arguments, response);
		events->RespondRendererUpdateElement_DocumentParagraph(requestId, response);
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

### Change B: query handlers

Replace each `CHECK_FAIL` body with the following implementations.

#### `RequestDocumentParagraph_GetCaret`

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
	if (async->IsInMainThread(window)) proc(); else async->InvokeInMainThreadAndWait(window, proc, -1);
}
```

#### `RequestDocumentParagraph_GetCaretBounds`

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

#### `RequestDocumentParagraph_GetNearestCaretFromTextPos`

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

#### `RequestDocumentParagraph_IsValidCaret`

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

#### `RequestDocumentParagraph_GetInlineObjectFromPoint`

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

#### `RequestDocumentParagraph_OpenCaret` / `RequestDocumentParagraph_CloseCaret`

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

## STEP 5: Document hygiene (encoding / mojibake cleanup) [DONE]

Limit to files touched by this task:

- `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering_Document.cpp`
- `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp`
- `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle.h`
- `Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_Document.txt`

Search patterns:

- `rg -n "\uFFFD|Ã|Â|â" <files>`

## STEP 6: Build/test validation (repo scripts only) [DONE]

1) Stop debugger (safe if not running):

- `& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1`

2) Build solution:

- `cd Test\GacUISrc`
- `& REPO-ROOT\.github\Scripts\copilotBuild.ps1`

3) Run unit tests (sanity):

- `cd Test\GacUISrc`
- `& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Executable UnitTest`

# FIXING ATTEMPTS

## Fixing attempt No.1

- The build failed with an ambiguous `DocumentRun` reference in `Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering_Document.cpp`.
- I qualified the protocol type with `remoteprotocol::DocumentRun` (including `Nullable<remoteprotocol::DocumentRun>`) to disambiguate between the protocol struct and the document model class.
- This resolves the type ambiguity and allows the build to complete.

# !!!FINISHED!!!

