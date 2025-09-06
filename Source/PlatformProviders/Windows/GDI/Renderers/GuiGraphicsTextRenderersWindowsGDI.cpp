#include "GuiGraphicsTextRenderersWindowsGDI.h"

namespace vl
{
	namespace presentation
	{
		namespace elements_windows_gdi
		{
			using namespace windows;
			using namespace collections;

/***********************************************************************
GuiColorizedTextElementRenderer
***********************************************************************/

			void GuiColorizedTextElementRenderer::DestroyColors()
			{
				auto resourceManager=GetWindowsGDIResourceManager();
				// TODO: (enumerable) foreach
				for(vint i=0;i<colors.Count();i++)
				{
					resourceManager->DestroyGdiBrush(colors[i].normal.background);
					resourceManager->DestroyGdiBrush(colors[i].selectedFocused.background);
					resourceManager->DestroyGdiBrush(colors[i].selectedUnfocused.background);
				}
			}

			void GuiColorizedTextElementRenderer::ColorChanged()
			{
				auto resourceManager=GetWindowsGDIResourceManager();
				ColorArray newColors;
				newColors.Resize(element->GetColors().Count());
				// TODO: (enumerable) foreach:indexed
				for(vint i=0;i<newColors.Count();i++)
				{
					text::ColorEntry entry=element->GetColors().Get(i);
					ColorEntryResource newEntry;

					newEntry.normal.text=entry.normal.text;
					newEntry.normal.background=entry.normal.background;
					newEntry.normal.backgroundBrush=resourceManager->CreateGdiBrush(newEntry.normal.background);
					newEntry.selectedFocused.text=entry.selectedFocused.text;
					newEntry.selectedFocused.background=entry.selectedFocused.background;
					newEntry.selectedFocused.backgroundBrush=resourceManager->CreateGdiBrush(newEntry.selectedFocused.background);
					newEntry.selectedUnfocused.text=entry.selectedUnfocused.text;
					newEntry.selectedUnfocused.background=entry.selectedUnfocused.background;
					newEntry.selectedUnfocused.backgroundBrush=resourceManager->CreateGdiBrush(newEntry.selectedUnfocused.background);
					newColors[i]=newEntry;
				}

				DestroyColors();
				CopyFrom(colors, newColors);
			}

			void GuiColorizedTextElementRenderer::FontChanged()
			{
				auto resourceManager = GetWindowsGDIResourceManager();
				if (font)
				{
					element->GetLines().SetCharMeasurer(nullptr);
					resourceManager->DestroyGdiFont(oldFont);
					resourceManager->DestroyCharMeasurer(oldFont);
					font = nullptr;
				}
				oldFont = element->GetFont();
				font = resourceManager->CreateGdiFont(oldFont);
				element->GetLines().SetCharMeasurer(resourceManager->CreateCharMeasurer(oldFont).Obj());
			}

			void GuiColorizedTextElementRenderer::InitializeInternal()
			{
				auto resourceManager=GetWindowsGDIResourceManager();
				oldCaretColor=element->GetCaretColor();
				caretPen=resourceManager->CreateGdiPen(oldCaretColor);
				element->SetCallback(this);
			}

			void GuiColorizedTextElementRenderer::FinalizeInternal()
			{
				element->SetCallback(nullptr);
				auto resourceManager=GetWindowsGDIResourceManager();
				if(font)
				{
					resourceManager->DestroyGdiFont(oldFont);
					resourceManager->DestroyCharMeasurer(oldFont);
				}
				resourceManager->DestroyGdiPen(oldCaretColor);
				DestroyColors();
			}

			void GuiColorizedTextElementRenderer::RenderTargetChangedInternal(IWindowsGDIRenderTarget* oldRenderTarget, IWindowsGDIRenderTarget* newRenderTarget)
			{
				element->GetLines().SetRenderTarget(newRenderTarget);
			}

			void GuiColorizedTextElementRenderer::Render(Rect bounds)
			{
				if (renderTarget)
				{
					WinDC* dc = renderTarget->GetDC();
					dc->SetFont(font);

					wchar_t passwordChar = element->GetPasswordChar();
					Point viewPosition = element->GetViewPosition();
					Rect viewBounds(viewPosition, bounds.GetSize());
					vint startRow = element->GetLines().GetTextPosFromPoint(Point(viewBounds.x1, viewBounds.y1)).row;
					vint endRow = element->GetLines().GetTextPosFromPoint(Point(viewBounds.x2, viewBounds.y2)).row;
					TextPos selectionBegin = element->GetCaretBegin() < element->GetCaretEnd() ? element->GetCaretBegin() : element->GetCaretEnd();
					TextPos selectionEnd = element->GetCaretBegin() > element->GetCaretEnd() ? element->GetCaretBegin() : element->GetCaretEnd();
					bool focused = element->GetFocused();
					Ptr<windows::WinBrush> lastBrush = 0;

					for (vint row = startRow; row <= endRow; row++)
					{
						Rect startRect = element->GetLines().GetRectFromTextPos(TextPos(row, 0));
						Point startPoint = startRect.LeftTop();
						vint startColumn = element->GetLines().GetTextPosFromPoint(Point(viewBounds.x1, startPoint.y)).column;
						vint endColumn = element->GetLines().GetTextPosFromPoint(Point(viewBounds.x2, startPoint.y)).column;

						text::TextLine& line = element->GetLines().GetLine(row);
						if (text::UTF16SPFirst(line.text[endColumn]) && endColumn + 1 < line.dataLength && text::UTF16SPSecond(line.text[startColumn + 1]))
						{
							endColumn++;
						}

						vint x = startColumn == 0 ? 0 : line.att[startColumn - 1].rightOffset;
						for (vint column = startColumn; column <= endColumn; column++)
						{
							bool inSelection = false;
							if (selectionBegin.row == selectionEnd.row)
							{
								inSelection = (row == selectionBegin.row && selectionBegin.column <= column && column < selectionEnd.column);
							}
							else if (row == selectionBegin.row)
							{
								inSelection = selectionBegin.column <= column;
							}
							else if (row == selectionEnd.row)
							{
								inSelection = column < selectionEnd.column;
							}
							else
							{
								inSelection = selectionBegin.row < row && row < selectionEnd.row;
							}

							bool crlf = column == line.dataLength;
							vint colorIndex = crlf ? 0 : line.att[column].colorIndex;
							if (colorIndex >= colors.Count())
							{
								colorIndex = 0;
							}
							ColorItemResource& color =
								!inSelection ? colors[colorIndex].normal :
								focused ? colors[colorIndex].selectedFocused :
								colors[colorIndex].selectedUnfocused;
							vint x2 = crlf ? x + startRect.Height() / 2 : line.att[column].rightOffset;
							vint tx = x - viewPosition.x + bounds.x1;
							vint ty = startPoint.y - viewPosition.y + bounds.y1;

							if (color.background.a)
							{
								if (lastBrush != color.backgroundBrush)
								{
									lastBrush = color.backgroundBrush;
									dc->SetBrush(lastBrush);
								}
								dc->FillRect(tx, ty, tx + (x2 - x), ty + startRect.Height());
							}
							if (!crlf)
							{
								vint count = text::UTF16SPFirst(line.text[column]) && column + 1 < line.dataLength && text::UTF16SPSecond(line.text[column + 1]) ? 2 : 1;
								if (color.text.a)
								{
									dc->SetTextColor(RGB(color.text.r, color.text.g, color.text.b));
									dc->DrawBuffer(tx, ty, (passwordChar ? &passwordChar : &line.text[column]), count);
								}
								if (count == 2) column++;
							}
							x = x2;
						}
					}

					if (element->GetCaretVisible() && element->GetLines().IsAvailable(element->GetCaretEnd()))
					{
						Point caretPoint = element->GetLines().GetPointFromTextPos(element->GetCaretEnd());
						vint height = element->GetLines().GetRowHeight();
						dc->SetPen(caretPen);
						dc->MoveTo(caretPoint.x - viewPosition.x + bounds.x1, caretPoint.y - viewPosition.y + bounds.y1 + 1);
						dc->LineTo(caretPoint.x - viewPosition.x + bounds.x1, caretPoint.y + height - viewPosition.y + bounds.y1 - 1);
						dc->MoveTo(caretPoint.x - 1 - viewPosition.x + bounds.x1, caretPoint.y - viewPosition.y + bounds.y1 + 1);
						dc->LineTo(caretPoint.x - 1 - viewPosition.x + bounds.x1, caretPoint.y + height - viewPosition.y + bounds.y1 - 1);
					}
				}
			}

			void GuiColorizedTextElementRenderer::OnElementStateChanged()
			{
				Color caretColor=element->GetCaretColor();
				if(oldCaretColor!=caretColor)
				{
					auto resourceManager=GetWindowsGDIResourceManager();
					resourceManager->DestroyGdiPen(oldCaretColor);
					oldCaretColor=caretColor;
					caretPen=resourceManager->CreateGdiPen(oldCaretColor);
				}
			}
		}
	}
}