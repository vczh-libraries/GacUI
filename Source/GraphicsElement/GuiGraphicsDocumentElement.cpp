#include "GuiGraphicsDocumentElement.h"

namespace vl
{
	using namespace collections;

	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
GuiDocumentElement
***********************************************************************/

			Ptr<IGuiDocumentElementRenderer> GuiDocumentElement::GetElementRenderer()
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::GetElementRenderer()#"
				if (!renderer) return nullptr;
				auto elementRenderer = renderer.Cast<IGuiDocumentElementRenderer>();
				CHECK_ERROR(elementRenderer, ERROR_MESSAGE_PREFIX L"The registered renderer for GuiDocumentElement must implement IGuiDocumentElementRenderer.");
				return elementRenderer;
#undef ERROR_MESSAGE_PREFIX
			}

			void GuiDocumentElement::UpdateCaret()
			{
				if (auto elementRenderer = GetElementRenderer())
				{
					elementRenderer->SetSelection(caretBegin, caretEnd);
					if (caretVisible)
					{
						elementRenderer->OpenCaret(caretEnd, caretColor, caretFrontSide);
					}
					else
					{
						elementRenderer->CloseCaret(caretEnd);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			GuiDocumentElement::GuiDocumentElement()
				:passwordChar(0)
				,caretVisible(false)
				,caretFrontSide(false)
			{
			}

			IGuiDocumentElementCallback* GuiDocumentElement::GetCallback()
			{
				return callback;
			}

			void GuiDocumentElement::SetCallback(IGuiDocumentElementCallback* value)
			{
				callback = value;
			}

			Ptr<DocumentModel> GuiDocumentElement::GetDocument()
			{
				return document;
			}

			void GuiDocumentElement::SetDocument(Ptr<DocumentModel> value)
			{
				document=value;
				InvokeOnElementStateChanged();
				SetCaret(TextPos(), TextPos(), false);
			}

			bool GuiDocumentElement::GetParagraphPadding()
			{
				return paragraphPadding;
			}

			void GuiDocumentElement::SetParagraphPadding(bool value)
			{
				if (paragraphPadding != value)
				{
					paragraphPadding = value;
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphPaddingUpdated(paragraphPadding);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			bool GuiDocumentElement::GetWrapLine()
			{
				return wrapLine;
			}

			void GuiDocumentElement::SetWrapLine(bool value)
			{
				wrapLine = value;
				InvokeOnElementStateChanged();
			}

			wchar_t GuiDocumentElement::GetPasswordChar()
			{
				return passwordChar;
			}

			void GuiDocumentElement::SetPasswordChar(wchar_t value)
			{
				if(passwordChar!=value)
				{
					passwordChar=value;
					InvokeOnElementStateChanged();
				}
			}

			bool GuiDocumentElement::GetParagraphRecycle()
			{
				return paragraphRecycle;
			}

			void GuiDocumentElement::SetParagraphRecycle(bool value)
			{
				paragraphRecycle = value;
			}

			TextPos GuiDocumentElement::GetCaretBegin()
			{
				return caretBegin;
			}

			TextPos GuiDocumentElement::GetCaretEnd()
			{
				return caretEnd;
			}

			bool GuiDocumentElement::IsCaretEndPreferFrontSide()
			{
				return caretFrontSide;
			}

			void GuiDocumentElement::SetCaret(TextPos begin, TextPos end, bool frontSide)
			{
				caretBegin=begin;
				caretEnd=end;
				if(caretBegin<caretEnd)
				{
					caretFrontSide=true;
				}
				else if(caretBegin>caretEnd)
				{
					caretFrontSide=false;
				}
				else
				{
					caretFrontSide=frontSide;
				}
				UpdateCaret();
			}

			bool GuiDocumentElement::GetCaretVisible()
			{
				return caretVisible;
			}

			void GuiDocumentElement::SetCaretVisible(bool value)
			{
				caretVisible=value;
				UpdateCaret();
			}

			Color GuiDocumentElement::GetCaretColor()
			{
				return caretColor;
			}

			void GuiDocumentElement::SetCaretColor(Color value)
			{
				caretColor=value;
				UpdateCaret();
			}

			TextPos GuiDocumentElement::CalculateCaret(TextPos comparingCaret, IGuiGraphicsParagraph::CaretRelativePosition position, bool& preferFrontSide)
			{
				if (auto elementRenderer = GetElementRenderer())
				{
					TextPos caret = elementRenderer->CalculateCaret(comparingCaret, position, preferFrontSide);
					return caret.column == -1 ? comparingCaret : caret;
				}
				return {};
			}

			TextPos GuiDocumentElement::CalculateCaretFromPoint(Point point)
			{
				if (auto elementRenderer = GetElementRenderer())
				{
					return elementRenderer->CalculateCaretFromPoint(point);
				}
				return {};
			}

			Rect GuiDocumentElement::GetCaretBounds(TextPos caret, bool frontSide)
			{
				if (auto elementRenderer = GetElementRenderer())
				{
					return elementRenderer->GetCaretBounds(caret, frontSide);
				}
				return {};
			}
			
			void GuiDocumentElement::NotifyParagraphUpdated(vint index, vint oldCount, vint newCount, bool updatedText)
			{
				if (auto elementRenderer = GetElementRenderer())
				{
					elementRenderer->NotifyParagraphUpdated(index, oldCount, newCount, updatedText);
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiDocumentElement::EditRun(TextPos begin, TextPos end, Ptr<DocumentModel> model, bool copy)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				vint newRows = document->EditRun(begin, end, model, copy);
				if (newRows != -1)
				{
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, newRows, true);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			void GuiDocumentElement::EditText(TextPos begin, TextPos end, bool frontSide, const collections::Array<WString>& text)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				vint newRows = document->EditText(begin, end, frontSide, text);
				if (newRows != -1)
				{
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, newRows, true);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			void GuiDocumentElement::EditStyle(TextPos begin, TextPos end, Ptr<DocumentStyleProperties> style)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				if (document->EditStyle(begin, end, style))
				{
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, end.row - begin.row + 1, false);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			void GuiDocumentElement::EditImage(TextPos begin, TextPos end, Ptr<GuiImageData> image)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				if (document->EditImage(begin, end, image))
				{
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, 1, true);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			void GuiDocumentElement::EditHyperlink(vint paragraphIndex, vint begin, vint end, const WString& reference, const WString& normalStyleName, const WString& activeStyleName)
			{
				if (begin > end)
				{
					vint temp = begin;
					begin = end;
					end = temp;
				}

				if (document->EditHyperlink(paragraphIndex, begin, end, reference, normalStyleName, activeStyleName))
				{
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(paragraphIndex, 1, 1, false);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			void GuiDocumentElement::RemoveHyperlink(vint paragraphIndex, vint begin, vint end)
			{
				if (begin > end)
				{
					vint temp = begin;
					begin = end;
					end = temp;
				}

				if (document->RemoveHyperlink(paragraphIndex, begin, end))
				{
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(paragraphIndex, 1, 1, false);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			void GuiDocumentElement::EditStyleName(TextPos begin, TextPos end, const WString& styleName)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				if (document->EditStyleName(begin, end, styleName))
				{
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, end.row - begin.row + 1, false);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			void GuiDocumentElement::RemoveStyleName(TextPos begin, TextPos end)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				if (document->RemoveStyleName(begin, end))
				{
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, end.row - begin.row + 1, false);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			void GuiDocumentElement::RenameStyle(const WString& oldStyleName, const WString& newStyleName)
			{
				document->RenameStyle(oldStyleName, newStyleName);
			}

			void GuiDocumentElement::ClearStyle(TextPos begin, TextPos end)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				if (document->ClearStyle(begin, end))
				{
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, end.row - begin.row + 1, false);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			void GuiDocumentElement::ConvertToPlainText(TextPos begin, TextPos end)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				if (document->ConvertToPlainText(begin, end))
				{
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, end.row - begin.row + 1, false);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			Ptr<DocumentStyleProperties> GuiDocumentElement::SummarizeStyle(TextPos begin, TextPos end)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				return document->SummarizeStyle(begin, end);
			}

			Nullable<WString> GuiDocumentElement::SummarizeStyleName(TextPos begin, TextPos end)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				return document->SummarizeStyleName(begin, end);
			}

			void GuiDocumentElement::SetParagraphAlignment(TextPos begin, TextPos end, const collections::Array<Nullable<Alignment>>& alignments)
			{
				vint first = begin.row;
				vint last = end.row;
				if (first > last)
				{
					vint temp = first;
					first = last;
					last = temp;
				}

				if (0 <= first && first < document->paragraphs.Count() && 0 <= last && last < document->paragraphs.Count() && last - first + 1 == alignments.Count())
				{
					for (vint i = first; i <= last; i++)
					{
						document->paragraphs[i]->alignment = alignments[i - first];
					}
					if (auto elementRenderer = GetElementRenderer())
					{
						elementRenderer->NotifyParagraphUpdated(first, alignments.Count(), alignments.Count(), false);
						InvokeOnCompositionStateChanged();
					}
				}
			}

			Nullable<Alignment> GuiDocumentElement::SummarizeParagraphAlignment(TextPos begin, TextPos end)
			{
				if (begin > end)
				{
					TextPos temp = begin;
					begin = end;
					end = temp;
				}

				return document->SummarizeParagraphAlignment(begin, end);
			}

			Ptr<DocumentHyperlinkRun::Package> GuiDocumentElement::GetHyperlinkFromPoint(Point point)
			{
				if (auto elementRenderer = GetElementRenderer())
				{
					return elementRenderer->GetHyperlinkFromPoint(point);
				}
				return {};
			}
		}
	}
}