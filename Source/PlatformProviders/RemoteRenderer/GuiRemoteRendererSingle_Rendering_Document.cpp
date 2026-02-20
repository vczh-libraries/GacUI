#include "GuiRemoteRendererSingle.h"
#include "../Remote/GuiRemoteGraphics_Document.h"

namespace vl::presentation::remote_renderer
{
	using namespace collections;
	using namespace elements;
	using namespace remoteprotocol;

/***********************************************************************
* Rendering (Elements -- Document)
***********************************************************************/

	class GuiRemoteDocumentParagraphElement
		: public Object
		, public IGuiGraphicsElement
		, protected IGuiGraphicsRenderer
		, protected IGuiGraphicsRendererFactory
		, protected IGuiGraphicsParagraphCallback
	{
	protected:
		GuiRemoteRendererSingle*											owner = nullptr;
		compositions::GuiGraphicsComposition*								ownerComposition = nullptr;
		IGuiGraphicsRenderTarget*											renderTarget = nullptr;

		Ptr<IGuiGraphicsParagraph>											paragraph;
		WString																text;
		bool																hasText = false;
		Nullable<remoteprotocol::OpenCaretRequest>							caret;

		elements::DocumentTextRunPropertyMap								textRuns;
		elements::DocumentInlineObjectRunPropertyMap						inlineObjectRuns;
		elements::DocumentRunPropertyMap									mergedRuns;

		Dictionary<vint, Rect>												inlineObjectBounds;
		Dictionary<vint, remoteprotocol::DocumentInlineObjectRunProperty>	inlineObjectProps;
		Dictionary<vint, elements::CaretRange>								inlineObjectRanges;

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
		}

		void Finalize() override
		{
		}

		void SetRenderTarget(IGuiGraphicsRenderTarget* _renderTarget) override
		{
			if (renderTarget != _renderTarget)
			{
				renderTarget = _renderTarget;
				paragraph = nullptr;
				inlineObjectBounds.Clear();
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

			mergedRuns.Clear();
			elements::MergeRuns(textRuns, inlineObjectRuns, mergedRuns);

			auto&& mergedKeys = mergedRuns.Keys();
			for (vint i = 0; i < mergedKeys.Count(); i++)
			{
				auto range = mergedKeys[i];
				auto props = mergedRuns.Values()[i];
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

			paragraph->SetWrapLine(arguments.wrapLine);
			paragraph->SetMaxWidth(arguments.maxWidth);
			paragraph->SetParagraphAlignment(owner->GetAlignment(arguments.alignment));

			inlineObjectBounds.Clear();

			if (arguments.removedInlineObjects)
			{
				for (auto callbackId : *arguments.removedInlineObjects.Obj())
				{
					auto&& inlineKeys = inlineObjectRuns.Keys();
					for (vint i = 0; i < inlineKeys.Count(); i++)
					{
						auto range = inlineKeys[i];
						auto prop = inlineObjectRuns.Values()[i];
						if (prop.callbackId == callbackId)
						{
							CHECK_ERROR(elements::ResetInlineObjectRun(inlineObjectRuns, range), ERROR_MESSAGE_PREFIX L"ResetInlineObjectRun failed.");
							break;
						}
					}

					vint index = inlineObjectRanges.Keys().IndexOf(callbackId);
					CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"Removed inline object not found.");
					auto range = inlineObjectRanges.Values()[index];
					CHECK_ERROR(paragraph->ResetInlineObject(range.caretBegin, range.caretEnd - range.caretBegin), ERROR_MESSAGE_PREFIX L"ResetInlineObject failed.");
					inlineObjectRanges.Remove(callbackId);
					inlineObjectProps.Remove(callbackId);
					inlineObjectBounds.Remove(callbackId);
				}
			}

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

							paragraph->SetFont(start, length, textProp.fontProperties.fontFamily);
							paragraph->SetSize(start, length, textProp.fontProperties.size);
							paragraph->SetStyle(start, length, style);
							paragraph->SetColor(start, length, textProp.textColor);
							paragraph->SetBackgroundColor(start, length, textProp.backgroundColor);
						},
						[&](const remoteprotocol::DocumentInlineObjectRunProperty& inlineProp)
						{
							CHECK_ERROR(elements::AddInlineObjectRun(inlineObjectRuns, range, inlineProp), ERROR_MESSAGE_PREFIX L"AddInlineObjectRun failed.");

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

			mergedRuns.Clear();
			elements::MergeRuns(textRuns, inlineObjectRuns, mergedRuns);

			response.documentSize = paragraph->GetSize();

			if (inlineObjectRuns.Count() > 0)
			{
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

#define PREPARE_DOCUMENT_WRAPPER_RAW(WRAPPER_NAME, ELEMENT_ID)																			\
	vint index = availableElements.Keys().IndexOf(ELEMENT_ID);																			\
	CHECK_ERROR(index != -1, L"GuiRemoteRendererSingle::Request*()#Failed to find IGuiGraphicsParagraph from element id.");				\
	auto WRAPPER_NAME = availableElements.Values()[index].Cast<GuiRemoteDocumentParagraphElement>();									\
	CHECK_ERROR(WRAPPER_NAME, L"GuiRemoteRendererSingle::Request*()#Failed to find IGuiGraphicsParagraph from element id.")				\

#define PREPARE_DOCUMENT_WRAPPER(WRAPPER_NAME, ELEMENT_ID)																				\
	PREPARE_DOCUMENT_WRAPPER_RAW(WRAPPER_NAME, ELEMENT_ID);																				\
	CHECK_ERROR(WRAPPER_NAME->HasParagraph(), L"GuiRemoteRendererSingle::Request*()#The IGuiGraphicsParagraph is not created yet.")		\

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph(vint id, const remoteprotocol::ElementDesc_DocumentParagraph& arguments)
	{
		PREPARE_DOCUMENT_WRAPPER_RAW(wrapper, arguments.id);
		UpdateElement_DocumentParagraphResponse response;
		wrapper->ApplyUpdateAndFillResponse(arguments, response);
		events->RespondRendererUpdateElement_DocumentParagraph(id, response);
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetCaret(vint id, const remoteprotocol::GetCaretRequest& arguments)
	{
		PREPARE_DOCUMENT_WRAPPER(wrapper, arguments.id);
		GetCaretResponse response;
		response.preferFrontSide = false;
		response.newCaret = wrapper->GetParagraph()->GetCaret(arguments.caret, arguments.relativePosition, response.preferFrontSide);
		events->RespondDocumentParagraph_GetCaret(id, response);
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetCaretBounds(vint id, const remoteprotocol::GetCaretBoundsRequest& arguments)
	{
		PREPARE_DOCUMENT_WRAPPER(wrapper, arguments.id);
		auto bounds = wrapper->GetParagraph()->GetCaretBounds(arguments.caret, arguments.frontSide);
		events->RespondDocumentParagraph_GetCaretBounds(id, bounds);
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetInlineObjectFromPoint(vint id, const remoteprotocol::GetInlineObjectFromPointRequest& arguments)
	{
		PREPARE_DOCUMENT_WRAPPER(wrapper, arguments.id);
		Nullable<remoteprotocol::DocumentRun> result;
		vint start = 0;
		vint length = 0;
		auto props = wrapper->GetParagraph()->GetInlineObjectFromPoint(arguments.point, start, length);
		if (props)
		{
			vint cb = props.Value().callbackId;
			remoteprotocol::DocumentInlineObjectRunProperty inlineProp;
			if (wrapper->TryGetInlineObjectRunProperty(cb, inlineProp))
			{
				remoteprotocol::DocumentRun run;
				run.caretBegin = start;
				run.caretEnd = start + length;
				run.props = inlineProp;
				result = run;
			}
		}
		events->RespondDocumentParagraph_GetInlineObjectFromPoint(id, result);
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetNearestCaretFromTextPos(vint id, const remoteprotocol::GetCaretBoundsRequest& arguments)
	{
		PREPARE_DOCUMENT_WRAPPER(wrapper, arguments.id);
		auto result = wrapper->GetParagraph()->GetNearestCaretFromTextPos(arguments.caret, arguments.frontSide);
		events->RespondDocumentParagraph_GetNearestCaretFromTextPos(id, result);
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_IsValidCaret(vint id, const remoteprotocol::IsValidCaretRequest& arguments)
	{
		PREPARE_DOCUMENT_WRAPPER(wrapper, arguments.id);
		auto result = wrapper->GetParagraph()->IsValidCaret(arguments.caret);
		events->RespondDocumentParagraph_IsValidCaret(id, result);
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_OpenCaret(const remoteprotocol::OpenCaretRequest& arguments)
	{
		PREPARE_DOCUMENT_WRAPPER(wrapper, arguments.id);
		wrapper->OpenCaretAndStore(arguments);
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_CloseCaret(const vint& arguments)
	{
		PREPARE_DOCUMENT_WRAPPER(wrapper, arguments);
		wrapper->CloseCaretAndStore();
	}

#undef PREPARE_DOCUMENT_WRAPPER
#undef PREPARE_DOCUMENT_WRAPPER_RAW
}
