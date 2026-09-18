#include "WindowsUIAutomationProvider.Windows.h"
#include <cwctype>

#ifdef VCZH_MSVC
namespace vl::presentation::windows
{
	using namespace collections;
	using namespace controls;
	using namespace compositions;

	struct WindowsUIAutomationTextSnapshot
	{
		WString text;
		List<TextPos> positions;
		Dictionary<WString, vint> objects;

		void Append(DocumentRun* run, vint row, vint& column)
		{
			if (auto container = dynamic_cast<DocumentContainerRun*>(run))
			{
				for (auto child : container->runs) Append(child.Obj(), row, column);
			}
			else if (auto content = dynamic_cast<DocumentContentRun*>(run))
			{
				if (auto plain = dynamic_cast<DocumentTextRun*>(run))
				{
					text += plain->text;
					for (vint i = 0; i < plain->text.Length(); i++) positions.Add(TextPos(row, column++));
				}
				else
				{
					if (auto embedded = dynamic_cast<DocumentEmbeddedObjectRun*>(run)) objects.Set(embedded->name, text.Length());
					text += L"\xFFFC";
					positions.Add(TextPos(row, column));
					column += content->GetRepresentationText().Length();
				}
			}
		}
		WindowsUIAutomationTextSnapshot(GuiDocumentCommonInterface* document)
		{
			auto model = document->GetDocument();
			vint column = 0;
			for (vint row = 0; row < model->paragraphs.Count(); row++)
			{
				if (row)
				{
					text += L"\r\n";
					positions.Add(TextPos(row - 1, column));
					positions.Add(TextPos(row - 1, column));
				}
				column = 0;
				Append(model->paragraphs[row].Obj(), row, column);
			}
			positions.Add(TextPos(max((vint)0, model->paragraphs.Count() - 1), column));
			// UIA offsets count UTF-16 code units; renderer carets cannot split a
			// CR/LF pair or a surrogate pair. Keep text intact and map those offsets
			// to the preceding valid caret, including pairs crossing run boundaries.
			for (vint i = 1; i < text.Length(); i++)
			{
				if (text[i] == L'\n' && text[i - 1] == L'\r' || text[i] >= 0xDC00 && text[i] <= 0xDFFF)
					positions.Set(i, positions[i - 1]);
			}
		}
		vint Offset(TextPos position)
		{
			for (vint i = 0; i < positions.Count(); i++) if (positions[i] >= position) return i;
			return text.Length();
		}
	};

	List<Ptr<WindowsUIAutomationNode>> UiaTextChildren(WindowsUIAutomationNode* node, vint begin, vint end)
	{
		List<Ptr<WindowsUIAutomationNode>> children;
		auto document = UiaDocument(node->control);
		WindowsUIAutomationTextSnapshot snapshot(document);
		auto& items = document->GetDocumentItems();
		auto objects = From(snapshot.objects.Keys()).OrderBy([&](const WString& a, const WString& b) { return snapshot.objects[a] <=> snapshot.objects[b]; });
		for (auto name : objects)
		{
			auto offset = snapshot.objects[name];
			if (offset < begin || end >= 0 && offset >= end) continue;
			auto index = items.Keys().IndexOf(name);
			// The renderer hides this container when its placeholder is offscreen.
			// Its semantic children still belong to the document and text range.
			if (index >= 0) UiaCollectChildren(node->context, items.Values()[index]->GetContainer(), children);
		}
		return children;
	}

	GuiGraphicsComposition* UiaTextComposition(GuiGraphicsComposition* composition)
	{
		if (dynamic_cast<elements::GuiDocumentElement*>(composition->GetOwnedElement().Obj())) return composition;
		for (auto child : composition->Children()) if (auto result = UiaTextComposition(child)) return result;
		return nullptr;
	}

	UiaRect UiaTextRectangle(Ptr<WindowsUIAutomationNode> node, Rect rectangle)
	{
		auto composition = UiaTextComposition(node->control->GetBoundsComposition());
		if (!composition) return {};
		auto native = node->Window()->GetNativeWindow();
		auto origin = composition->GetGlobalBounds().LeftTop();
		auto position = native->Convert(Point(origin.x + rectangle.x1, origin.y + rectangle.y1));
		auto size = native->Convert(Size(max((vint)1, rectangle.Width()), rectangle.Height()));
		auto window = node->ScreenOrigin();
		auto clip = node->Bounds();
		double left = max((double)(window.x.value + position.x.value), clip.left), top = max((double)(window.y.value + position.y.value), clip.top);
		double right = min((double)(window.x.value + position.x.value + size.x.value), clip.left + clip.width), bottom = min((double)(window.y.value + position.y.value + size.y.value), clip.top + clip.height);
		return right > left && bottom > top ? UiaRect{ left, top, right - left, bottom - top } : UiaRect{};
	}

	class WindowsUIAutomationTextRange : public ITextRangeProvider
	{
		std::atomic<ULONG> references = 1;
	public:
		Ptr<WindowsUIAutomationNode> node;
		Ptr<DocumentModel> document;
		WString previousText;
		vint begin, end;

		WindowsUIAutomationTextRange(Ptr<WindowsUIAutomationNode> target, vint first, vint last)
			: node(target), document(UiaDocument(target->control)->GetDocument()), begin(first), end(last)
		{
			WindowsUIAutomationTextSnapshot snapshot(UiaDocument(node->control));
			previousText = snapshot.text;
		}
		HRESULT Read(const Func<HRESULT(WindowsUIAutomationTextSnapshot&)>& action)
		{
			return node->dispatcher->Run([&]() -> HRESULT
			{
				if (!node->IsLive() || !node->Supports(UIA_TextPatternId) || UiaDocument(node->control)->GetDocument() != document) return UIA_E_ELEMENTNOTAVAILABLE;
				WindowsUIAutomationTextSnapshot snapshot(UiaDocument(node->control));
				Normalize(snapshot);
				return action(snapshot);
			});
		}
		void Normalize(WindowsUIAutomationTextSnapshot& snapshot)
		{
			if (snapshot.text != previousText)
			{
				vint prefix = 0, suffix = 0;
				while (prefix < snapshot.text.Length() && prefix < previousText.Length() && snapshot.text[prefix] == previousText[prefix]) prefix++;
				while (suffix < snapshot.text.Length() - prefix && suffix < previousText.Length() - prefix && snapshot.text[snapshot.text.Length() - suffix - 1] == previousText[previousText.Length() - suffix - 1]) suffix++;
				auto adjust = [&](vint position)
				{
					if (position <= prefix) return position;
					if (position >= previousText.Length() - suffix) return position + snapshot.text.Length() - previousText.Length();
					return prefix;
				};
				begin = adjust(begin); end = max(begin, adjust(end)); previousText = snapshot.text;
			}
			begin = min(begin, snapshot.text.Length()); end = min(max(begin, end), snapshot.text.Length());
		}
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result)override
		{
			if (!result) return E_POINTER;
			*result = nullptr;
			if (iid != __uuidof(IUnknown) && iid != __uuidof(ITextRangeProvider)) return E_NOINTERFACE;
			*result = static_cast<ITextRangeProvider*>(this); AddRef(); return S_OK;
		}
		ULONG STDMETHODCALLTYPE AddRef()override { return ++references; }
		ULONG STDMETHODCALLTYPE Release()override { auto count = --references; if (!count) delete this; return count; }
		HRESULT STDMETHODCALLTYPE Clone(ITextRangeProvider** result)override
		{
			if (!result) return E_POINTER;
			*result = nullptr;
			return Read([&](auto&) -> HRESULT { *result = new WindowsUIAutomationTextRange(node, begin, end); return S_OK; });
		}
		HRESULT STDMETHODCALLTYPE Compare(ITextRangeProvider* other, BOOL* result)override
		{
			if (!result) return E_POINTER;
			*result = FALSE;
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto range = dynamic_cast<WindowsUIAutomationTextRange*>(other);
				if (!range || range->node != node || range->document != document) return E_INVALIDARG;
				range->Normalize(snapshot); *result = begin == range->begin && end == range->end; return S_OK;
			});
		}
		HRESULT STDMETHODCALLTYPE CompareEndpoints(TextPatternRangeEndpoint endpoint, ITextRangeProvider* other, TextPatternRangeEndpoint otherEndpoint, int* result)override
		{
			if (!result) return E_POINTER;
			*result = 0;
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto range = dynamic_cast<WindowsUIAutomationTextRange*>(other);
				if (!range || range->node != node || range->document != document || endpoint < 0 || endpoint > 1 || otherEndpoint < 0 || otherEndpoint > 1) return E_INVALIDARG;
				range->Normalize(snapshot); *result = (int)((endpoint ? end : begin) - (otherEndpoint ? range->end : range->begin)); return S_OK;
			});
		}
		List<vint> Boundaries(WindowsUIAutomationTextSnapshot& snapshot, TextUnit unit)
		{
			List<vint> boundaries; boundaries.Add(0);
			for (vint i = 1; i < snapshot.text.Length(); i++)
			{
				bool boundary = false;
				switch (unit)
				{
				case TextUnit_Character: boundary = snapshot.positions[i] != snapshot.positions[i - 1]; break;
				case TextUnit_Format:
					{
						const TEXTATTRIBUTEID attributes[] = { UIA_FontNameAttributeId, UIA_FontSizeAttributeId, UIA_FontWeightAttributeId, UIA_IsItalicAttributeId, UIA_UnderlineStyleAttributeId, UIA_StrikethroughStyleAttributeId, UIA_ForegroundColorAttributeId, UIA_BackgroundColorAttributeId };
						for (auto attribute : attributes)
						{
							WindowsUIAutomationValue a, b;
							Attribute(snapshot, attribute, i - 1, i, &a.value);
							Attribute(snapshot, attribute, i, i + 1, &b.value);
							bool same = a.value.vt == VT_UNKNOWN && b.value.vt == VT_UNKNOWN ? a.value.punkVal == b.value.punkVal : VarCmp(&a.value, &b.value, LOCALE_INVARIANT, 0) == VARCMP_EQ;
							if (!same) { boundary = true; break; }
						}
						break;
					}
				case TextUnit_Word: boundary = iswalnum(snapshot.text[i]) != iswalnum(snapshot.text[i - 1]) || iswspace(snapshot.text[i - 1]) && !iswspace(snapshot.text[i]); break;
				case TextUnit_Line:
					boundary = snapshot.positions[i].row != snapshot.positions[i - 1].row || UiaDocument(node->control)->GetCaretBounds(snapshot.positions[i], false).y1 != UiaDocument(node->control)->GetCaretBounds(snapshot.positions[i - 1], false).y1;
					break;
				case TextUnit_Paragraph: boundary = snapshot.positions[i].row != snapshot.positions[i - 1].row; break;
				default: break;
				}
				if (boundary) boundaries.Add(i);
			}
			if (snapshot.text.Length()) boundaries.Add(snapshot.text.Length());
			return boundaries;
		}
		HRESULT STDMETHODCALLTYPE ExpandToEnclosingUnit(TextUnit unit)override
		{
			if (unit < TextUnit_Character || unit > TextUnit_Document) return E_INVALIDARG;
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto boundaries = Boundaries(snapshot, unit);
				for (vint i = 1; i < boundaries.Count(); i++) if (begin < boundaries[i] || i == boundaries.Count() - 1) { begin = boundaries[i - 1]; end = boundaries[i]; break; }
				return S_OK;
			});
		}
		HRESULT Attribute(WindowsUIAutomationTextSnapshot& snapshot, TEXTATTRIBUTEID attribute, vint first, vint last, VARIANT* result)
		{
			VariantInit(result);
			auto mixed = [&]() { result->vt = VT_UNKNOWN; return UiaGetReservedMixedAttributeValue(&result->punkVal); };
			auto document = UiaDocument(node->control);
			auto style = document->SummarizeStyle(snapshot.positions[first], snapshot.positions[last]);
			switch (attribute)
			{
			case UIA_IsReadOnlyAttributeId: result->vt = VT_BOOL; result->boolVal = document->GetEditMode() == GuiDocumentEditMode::Editable ? VARIANT_FALSE : VARIANT_TRUE; return S_OK;
			case UIA_HorizontalTextAlignmentAttributeId:
				{
					auto alignment = document->SummarizeParagraphAlignment(snapshot.positions[first], snapshot.positions[last]);
					if (!alignment) return mixed();
					result->vt = VT_I4;
					result->lVal = alignment.Value() == Alignment::Center ? HorizontalTextAlignment_Centered : alignment.Value() == Alignment::Right ? HorizontalTextAlignment_Right : HorizontalTextAlignment_Left;
					return S_OK;
				}
			case UIA_FontNameAttributeId: if (!style->face) return mixed(); result->vt = VT_BSTR; return UiaString(style->face.Value(), &result->bstrVal);
			case UIA_FontSizeAttributeId: if (!style->size) return mixed(); result->vt = VT_R8; result->dblVal = style->size.Value().size * 72.0 / 96.0; return S_OK;
			case UIA_FontWeightAttributeId: if (!style->bold) return mixed(); result->vt = VT_I4; result->lVal = style->bold.Value() ? FW_BOLD : FW_NORMAL; return S_OK;
			case UIA_IsItalicAttributeId: if (!style->italic) return mixed(); result->vt = VT_BOOL; result->boolVal = style->italic.Value() ? VARIANT_TRUE : VARIANT_FALSE; return S_OK;
			case UIA_UnderlineStyleAttributeId: if (!style->underline) return mixed(); result->vt = VT_I4; result->lVal = style->underline.Value() ? TextDecorationLineStyle_Single : TextDecorationLineStyle_None; return S_OK;
			case UIA_StrikethroughStyleAttributeId: if (!style->strikeline) return mixed(); result->vt = VT_I4; result->lVal = style->strikeline.Value() ? TextDecorationLineStyle_Single : TextDecorationLineStyle_None; return S_OK;
			case UIA_ForegroundColorAttributeId: case UIA_BackgroundColorAttributeId:
				{
					auto color = attribute == UIA_ForegroundColorAttributeId ? style->color : style->backgroundColor;
					if (!color) return mixed(); result->vt = VT_I4; result->lVal = RGB(color.Value().r, color.Value().g, color.Value().b); return S_OK;
				}
			default: result->vt = VT_UNKNOWN; return UiaGetReservedNotSupportedValue(&result->punkVal);
			}
		}
		HRESULT STDMETHODCALLTYPE GetAttributeValue(TEXTATTRIBUTEID attribute, VARIANT* result)override
		{
			if (!result) return E_POINTER;
			VariantInit(result);
			return Read([&](auto& snapshot) { return Attribute(snapshot, attribute, begin, end, result); });
		}
		HRESULT STDMETHODCALLTYPE FindAttribute(TEXTATTRIBUTEID attribute, VARIANT value, BOOL backward, ITextRangeProvider** result)override
		{
			if (!result) return E_POINTER;
			*result = nullptr;
			return Read([&](auto& snapshot) -> HRESULT
			{
				vint first = -1, last = -1;
				for (vint i = backward ? end - 1 : begin; i >= begin && i < end; i += backward ? -1 : 1)
				{
					WindowsUIAutomationValue candidate; Attribute(snapshot, attribute, i, i + 1, &candidate.value);
					if (VarCmp(&candidate.value, &value, LOCALE_INVARIANT, 0) == VARCMP_EQ) { if (first < 0) first = i; last = i; }
					else if (first >= 0) break;
				}
				if (first >= 0) *result = new WindowsUIAutomationTextRange(node, min(first, last), max(first, last) + 1);
				return S_OK;
			});
		}
		HRESULT STDMETHODCALLTYPE FindText(BSTR text, BOOL backward, BOOL ignoreCase, ITextRangeProvider** result)override
		{
			if (!result) return E_POINTER;
			*result = nullptr;
			if (!text || !SysStringLen(text)) return E_INVALIDARG;
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto length = (vint)SysStringLen(text);
				for (vint i = backward ? end - length : begin; i >= begin && i + length <= end; i += backward ? -1 : 1)
				{
					if (CompareStringOrdinal(snapshot.text.Buffer() + i, (int)length, text, (int)length, ignoreCase) == CSTR_EQUAL)
					{ *result = new WindowsUIAutomationTextRange(node, i, i + length); break; }
				}
				return S_OK;
			});
		}
		HRESULT STDMETHODCALLTYPE GetText(int maximum, BSTR* result)override
		{
			if (!result) return E_POINTER;
			*result = nullptr;
			if (maximum < -1) return E_INVALIDARG;
			return Read([&](auto& snapshot) { return UiaString(snapshot.text.Sub(begin, maximum < 0 ? end - begin : min((vint)maximum, end - begin)), result); });
		}
		HRESULT STDMETHODCALLTYPE GetEnclosingElement(IRawElementProviderSimple** result)override
		{
			if (!result) return E_POINTER;
			*result = nullptr;
			return Read([&](auto&) -> HRESULT { *result = node->Provider(); (*result)->AddRef(); return S_OK; });
		}
		HRESULT STDMETHODCALLTYPE GetChildren(SAFEARRAY** result)override
		{
			if (!result) return E_POINTER;
			*result = nullptr;
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto children = UiaTextChildren(node.Obj(), begin, end);
				return UiaNodeArray(children, result);
			});
		}
		HRESULT STDMETHODCALLTYPE GetBoundingRectangles(SAFEARRAY** result)override
		{
			if (!result) return E_POINTER;
			*result = nullptr;
			return Read([&](auto& snapshot) -> HRESULT
			{
				List<UiaRect> rectangles;
				auto document = UiaDocument(node->control);
				for (vint i = begin; i < end; i++)
				{
					if (snapshot.text[i] == L'\r' || snapshot.text[i] == L'\n') continue;
					auto a = document->GetCaretBounds(snapshot.positions[i], false), b = document->GetCaretBounds(snapshot.positions[i + 1], true);
					auto rect = UiaTextRectangle(node, Rect(min(a.x1, b.x1), min(a.y1, b.y1), max(a.x2, b.x2), max(a.y2, b.y2)));
					if (!rect.width || !rect.height) continue;
					if (rectangles.Count())
					{
						auto previous = rectangles[rectangles.Count() - 1];
						if (previous.top == rect.top && previous.height == rect.height && rect.left <= previous.left + previous.width + 1)
						{
							previous.width = max(previous.left + previous.width, rect.left + rect.width) - previous.left;
							rectangles.Set(rectangles.Count() - 1, previous); continue;
						}
					}
					rectangles.Add(rect);
				}
				*result = SafeArrayCreateVector(VT_R8, 0, (ULONG)rectangles.Count() * 4);
				LONG index = 0;
				for (auto rect : rectangles) for (double value : { rect.left, rect.top, rect.width, rect.height }) { SafeArrayPutElement(*result, &index, &value); index++; }
				return S_OK;
			});
		}
		HRESULT STDMETHODCALLTYPE MoveEndpointByUnit(TextPatternRangeEndpoint endpoint, TextUnit unit, int count, int* moved)override
		{
			if (!moved) return E_POINTER;
			*moved = 0;
			if (endpoint < 0 || endpoint > 1 || unit < 0 || unit > TextUnit_Document) return E_INVALIDARG;
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto boundaries = Boundaries(snapshot, unit); auto position = endpoint ? end : begin;
				while (*moved != count)
				{
					vint next = position;
					if (count > 0) { for (auto candidate : boundaries) if (candidate > position) { next = candidate; break; } }
					else { for (vint i = boundaries.Count() - 1; i >= 0; i--) if (boundaries[i] < position) { next = boundaries[i]; break; } }
					if (next == position) break; position = next; *moved += count > 0 ? 1 : -1;
				}
				if (endpoint) { end = position; begin = min(begin, end); } else { begin = position; end = max(begin, end); }
				return S_OK;
			});
		}
		HRESULT STDMETHODCALLTYPE Move(TextUnit unit, int count, int* moved)override
		{
			if (!moved) return E_POINTER;
			*moved = 0;
			if (unit < 0 || unit > TextUnit_Document) return E_INVALIDARG;
			return Read([&](auto& snapshot) -> HRESULT
			{
				if (!count) return S_OK;
				bool degenerate = begin == end;
				auto boundaries = Boundaries(snapshot, unit);
				vint index = 0;
				while (index + 1 < boundaries.Count() && boundaries[index + 1] <= begin) index++;
				auto maximum = max((vint)0, boundaries.Count() - (degenerate ? 1 : 2));
				auto next = min(maximum, max((vint)0, index + count));
				*moved = (int)(next - index); begin = boundaries[next]; end = degenerate || next + 1 >= boundaries.Count() ? begin : boundaries[next + 1];
				return S_OK;
			});
		}
		HRESULT STDMETHODCALLTYPE MoveEndpointByRange(TextPatternRangeEndpoint endpoint, ITextRangeProvider* other, TextPatternRangeEndpoint otherEndpoint)override
		{
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto range = dynamic_cast<WindowsUIAutomationTextRange*>(other);
				if (!range || range->node != node || range->document != document || endpoint < 0 || endpoint > 1 || otherEndpoint < 0 || otherEndpoint > 1) return E_INVALIDARG;
				range->Normalize(snapshot); auto position = otherEndpoint ? range->end : range->begin;
				if (endpoint) { end = position; begin = min(begin, end); } else { begin = position; end = max(begin, end); }
				return S_OK;
			});
		}
		HRESULT STDMETHODCALLTYPE Select()override
		{
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto document = UiaDocument(node->control);
				if (document->GetEditMode() == GuiDocumentEditMode::ViewOnly) return UIA_E_INVALIDOPERATION;
				if (!node->control->GetVisuallyEnabled()) return UIA_E_ELEMENTNOTENABLED;
				document->SetCaret(snapshot.positions[begin], snapshot.positions[end]); return S_OK;
			});
		}
		HRESULT STDMETHODCALLTYPE AddToSelection()override
		{
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto document = UiaDocument(node->control);
				auto a = snapshot.Offset(document->GetCaretBegin()), b = snapshot.Offset(document->GetCaretEnd());
				return a == b || min(a, b) == begin && max(a, b) == end ? Select() : UIA_E_INVALIDOPERATION;
			});
		}
		HRESULT STDMETHODCALLTYPE RemoveFromSelection()override
		{
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto document = UiaDocument(node->control);
				if (document->GetEditMode() == GuiDocumentEditMode::ViewOnly) return UIA_E_INVALIDOPERATION;
				auto a = snapshot.Offset(document->GetCaretBegin()), b = snapshot.Offset(document->GetCaretEnd());
				if (min(a, b) != begin || max(a, b) != end) return UIA_E_INVALIDOPERATION;
				document->SetCaret(snapshot.positions[begin], snapshot.positions[begin]); return S_OK;
			});
		}
		HRESULT STDMETHODCALLTYPE ScrollIntoView(BOOL alignToTop)override
		{
			return Read([&](auto& snapshot) -> HRESULT
			{
				auto rectangle = UiaDocument(node->control)->GetCaretBounds(snapshot.positions[alignToTop ? begin : end], alignToTop == FALSE);
				if (auto scroll = dynamic_cast<GuiScrollView*>(node->control))
				{
					auto position = scroll->GetViewPosition(); position.y = alignToTop ? rectangle.y1 : rectangle.y2 - scroll->GetViewSize().y;
					scroll->SetViewPosition(position);
				}
				return S_OK;
			});
		}
	};

	HRESULT UiaTextSelection(Ptr<WindowsUIAutomationNode> node, SAFEARRAY** result)
	{
		auto document = UiaDocument(node->control);
		if (document->GetEditMode() == GuiDocumentEditMode::ViewOnly) { *result = SafeArrayCreateVector(VT_UNKNOWN, 0, 0); return S_OK; }
		WindowsUIAutomationTextSnapshot snapshot(document);
		auto a = snapshot.Offset(document->GetCaretBegin()), b = snapshot.Offset(document->GetCaretEnd());
		*result = SafeArrayCreateVector(VT_UNKNOWN, 0, 1);
		ComPtr<ITextRangeProvider> range = new WindowsUIAutomationTextRange(node, min(a, b), max(a, b));
		LONG index = 0; return SafeArrayPutElement(*result, &index, range.Obj());
	}
	HRESULT WindowsUIAutomationProvider::get_DocumentRange(ITextRangeProvider** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT { WindowsUIAutomationTextSnapshot snapshot(UiaDocument(node->control)); *result = new WindowsUIAutomationTextRange(node, 0, snapshot.text.Length()); return S_OK; }, UIA_TextPatternId);
	}
	HRESULT WindowsUIAutomationProvider::get_SupportedTextSelection(SupportedTextSelection* result)
	{
		if (!result) return E_POINTER;
		return Read([&]() -> HRESULT { *result = UiaDocument(node->control)->GetEditMode() == GuiDocumentEditMode::ViewOnly ? SupportedTextSelection_None : SupportedTextSelection_Single; return S_OK; }, UIA_TextPatternId);
	}
	HRESULT WindowsUIAutomationProvider::RangeFromPoint(UiaPoint point, ITextRangeProvider** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			auto document = UiaDocument(node->control);
			auto native = node->Window()->GetNativeWindow();
			auto screen = node->ScreenOrigin();
			auto local = native->Convert(NativePoint((vint)point.x - screen.x.value, (vint)point.y - screen.y.value));
			auto composition = UiaTextComposition(node->control->GetBoundsComposition());
			if (!composition) return UIA_E_ELEMENTNOTAVAILABLE;
			auto origin = composition->GetGlobalBounds().LeftTop();
			WindowsUIAutomationTextSnapshot snapshot(document);
			auto position = snapshot.Offset(document->CalculateCaretFromPoint(Point(local.x - origin.x, local.y - origin.y)));
			*result = new WindowsUIAutomationTextRange(node, position, position); return S_OK;
		}, UIA_TextPatternId);
	}
	HRESULT WindowsUIAutomationProvider::RangeFromChild(IRawElementProviderSimple* child, ITextRangeProvider** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			auto provider = dynamic_cast<WindowsUIAutomationProvider*>(child);
			if (!provider || !provider->node->IsLive()) return E_INVALIDARG;
			auto document = UiaDocument(node->control);
			WindowsUIAutomationTextSnapshot snapshot(document);
			for (GuiGraphicsComposition* composition = provider->node->control->GetBoundsComposition(); composition; composition = composition->GetParent())
			{
				for (auto item : document->GetDocumentItems().Values())
				{
					if (item->GetContainer() != composition) continue;
					auto index = snapshot.objects.Keys().IndexOf(item->GetName());
					if (index < 0) return UIA_E_ELEMENTNOTAVAILABLE;
					auto offset = snapshot.objects.Values()[index]; *result = new WindowsUIAutomationTextRange(node, offset, offset + 1); return S_OK;
				}
			}
			return E_INVALIDARG;
		}, UIA_TextPatternId);
	}
	HRESULT WindowsUIAutomationProvider::GetVisibleRanges(SAFEARRAY** result)
	{
		if (!result) return E_POINTER;
		*result = nullptr;
		return Read([&]() -> HRESULT
		{
			auto document = UiaDocument(node->control);
			WindowsUIAutomationTextSnapshot snapshot(document);
			List<ComPtr<ITextRangeProvider>> ranges;
			vint first = -1;
			for (vint i = 0; i <= snapshot.text.Length(); i++)
			{
				auto rectangle = i < snapshot.text.Length() ? UiaTextRectangle(node, document->GetCaretBounds(snapshot.positions[i], false)) : UiaRect{};
				if (rectangle.height && rectangle.width) { if (first < 0) first = i; }
				else if (first >= 0) { ranges.Add(ComPtr<ITextRangeProvider>(new WindowsUIAutomationTextRange(node, first, i))); first = -1; }
			}
			*result = SafeArrayCreateVector(VT_UNKNOWN, 0, (ULONG)ranges.Count());
			for (LONG i = 0; i < ranges.Count(); i++) SafeArrayPutElement(*result, &i, ranges[i].Obj());
			return S_OK;
		}, UIA_TextPatternId);
	}
}
#endif
