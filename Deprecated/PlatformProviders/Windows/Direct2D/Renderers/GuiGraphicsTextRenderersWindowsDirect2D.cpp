#include <math.h>
#include "GuiGraphicsTextRenderersWindowsDirect2D.h"
#include "..\WinDirect2DApplication.h"

namespace vl
{
	namespace presentation
	{
		namespace elements_windows_d2d
		{
			using namespace collections;

/***********************************************************************
GuiColorizedTextElementRenderer
***********************************************************************/

			void GuiColorizedTextElementRenderer::CreateTextBrush(IWindowsDirect2DRenderTarget* _renderTarget)
			{
				if(_renderTarget)
				{
					colors.Resize(element->GetColors().Count());
					// TODO: (enumerable) foreach:indexed
					for(vint i=0;i<colors.Count();i++)
					{
						text::ColorEntry entry=element->GetColors().Get(i);
						ColorEntryResource newEntry;

						newEntry.normal.text=entry.normal.text;
						newEntry.normal.textBrush=_renderTarget->CreateDirect2DBrush(newEntry.normal.text);
						newEntry.normal.background=entry.normal.background;
						newEntry.normal.backgroundBrush=_renderTarget->CreateDirect2DBrush(newEntry.normal.background);
						newEntry.selectedFocused.text=entry.selectedFocused.text;
						newEntry.selectedFocused.textBrush=_renderTarget->CreateDirect2DBrush(newEntry.selectedFocused.text);
						newEntry.selectedFocused.background=entry.selectedFocused.background;
						newEntry.selectedFocused.backgroundBrush=_renderTarget->CreateDirect2DBrush(newEntry.selectedFocused.background);
						newEntry.selectedUnfocused.text=entry.selectedUnfocused.text;
						newEntry.selectedUnfocused.textBrush=_renderTarget->CreateDirect2DBrush(newEntry.selectedUnfocused.text);
						newEntry.selectedUnfocused.background=entry.selectedUnfocused.background;
						newEntry.selectedUnfocused.backgroundBrush=_renderTarget->CreateDirect2DBrush(newEntry.selectedUnfocused.background);
						colors[i]=newEntry;
					}
				}
			}

			void GuiColorizedTextElementRenderer::DestroyTextBrush(IWindowsDirect2DRenderTarget* _renderTarget)
			{
				if(_renderTarget)
				{
					// TODO: (enumerable) foreach
					for(vint i=0;i<colors.Count();i++)
					{
						_renderTarget->DestroyDirect2DBrush(colors[i].normal.text);
						_renderTarget->DestroyDirect2DBrush(colors[i].normal.background);
						_renderTarget->DestroyDirect2DBrush(colors[i].selectedFocused.text);
						_renderTarget->DestroyDirect2DBrush(colors[i].selectedFocused.background);
						_renderTarget->DestroyDirect2DBrush(colors[i].selectedUnfocused.text);
						_renderTarget->DestroyDirect2DBrush(colors[i].selectedUnfocused.background);
					}
					colors.Resize(0);
				}
			}

			void GuiColorizedTextElementRenderer::CreateCaretBrush(IWindowsDirect2DRenderTarget* _renderTarget)
			{
				if(_renderTarget)
				{
					oldCaretColor=element->GetCaretColor();
					caretBrush=_renderTarget->CreateDirect2DBrush(oldCaretColor);
				}
			}

			void GuiColorizedTextElementRenderer::DestroyCaretBrush(IWindowsDirect2DRenderTarget* _renderTarget)
			{
				if(_renderTarget)
				{
					if(caretBrush)
					{
						_renderTarget->DestroyDirect2DBrush(oldCaretColor);
						caretBrush=0;
					}
				}
			}

			void GuiColorizedTextElementRenderer::ColorChanged()
			{
				DestroyTextBrush(renderTarget);
				CreateTextBrush(renderTarget);
			}

			void GuiColorizedTextElementRenderer::FontChanged()
			{
				IWindowsDirect2DResourceManager* resourceManager = GetWindowsDirect2DResourceManager();
				if (textFormat)
				{
					element->GetLines().SetCharMeasurer(nullptr);
					resourceManager->DestroyDirect2DTextFormat(oldFont);
					resourceManager->DestroyDirect2DCharMeasurer(oldFont);
				}
				oldFont = element->GetFont();
				if (oldFont.fontFamily == L"") oldFont.fontFamily = windows::GetWindowsNativeController()->ResourceService()->GetDefaultFont().fontFamily;
				if (oldFont.size == 0) oldFont.size = 12;

				textFormat = resourceManager->CreateDirect2DTextFormat(oldFont);
				element->GetLines().SetCharMeasurer(resourceManager->CreateDirect2DCharMeasurer(oldFont).Obj());
			}

			text::CharMeasurer* GuiColorizedTextElementRenderer::GetCharMeasurer()
			{
				return 0;
			}

			void GuiColorizedTextElementRenderer::InitializeInternal()
			{
				textFormat=0;
				caretBrush=0;
				element->SetCallback(this);
			}

			void GuiColorizedTextElementRenderer::FinalizeInternal()
			{
				element->SetCallback(nullptr);
				DestroyTextBrush(renderTarget);
				DestroyCaretBrush(renderTarget);

				IWindowsDirect2DResourceManager* resourceManager=GetWindowsDirect2DResourceManager();
				if(textFormat)
				{
					resourceManager->DestroyDirect2DTextFormat(oldFont);
					resourceManager->DestroyDirect2DCharMeasurer(oldFont);
				}
			}

			void GuiColorizedTextElementRenderer::RenderTargetChangedInternal(IWindowsDirect2DRenderTarget* oldRenderTarget, IWindowsDirect2DRenderTarget* newRenderTarget)
			{
				DestroyTextBrush(oldRenderTarget);
				DestroyCaretBrush(oldRenderTarget);
				CreateTextBrush(newRenderTarget);
				CreateCaretBrush(newRenderTarget);
				element->GetLines().SetRenderTarget(newRenderTarget);
			}

			void GuiColorizedTextElementRenderer::Render(Rect bounds)
			{
				if (renderTarget)
				{
					ID2D1RenderTarget* d2dRenderTarget = renderTarget->GetDirect2DRenderTarget();
					wchar_t passwordChar = element->GetPasswordChar();
					Point viewPosition = element->GetViewPosition();
					Rect viewBounds(viewPosition, bounds.GetSize());
					vint startRow = element->GetLines().GetTextPosFromPoint(Point(viewBounds.x1, viewBounds.y1)).row;
					vint endRow = element->GetLines().GetTextPosFromPoint(Point(viewBounds.x2, viewBounds.y2)).row;
					TextPos selectionBegin = element->GetCaretBegin() < element->GetCaretEnd() ? element->GetCaretBegin() : element->GetCaretEnd();
					TextPos selectionEnd = element->GetCaretBegin() > element->GetCaretEnd() ? element->GetCaretBegin() : element->GetCaretEnd();
					bool focused = element->GetFocused();

					renderTarget->SetTextAntialias(oldFont.antialias, oldFont.verticalAntialias);

					for (vint row = startRow; row <= endRow; row++)
					{
						Rect startRect = element->GetLines().GetRectFromTextPos(TextPos(row, 0));
						Point startPoint = startRect.LeftTop();
						vint startColumn = element->GetLines().GetTextPosFromPoint(Point(viewBounds.x1, startPoint.y)).column;
						vint endColumn = element->GetLines().GetTextPosFromPoint(Point(viewBounds.x2, startPoint.y)).column;

						text::TextLine& line = element->GetLines().GetLine(row);
						if (endColumn + 1 < line.dataLength && text::UTF16SPFirst(line.text[endColumn]) && text::UTF16SPSecond(line.text[startColumn + 1]))
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

							if (color.background.a > 0)
							{
								d2dRenderTarget->FillRectangle(D2D1::RectF((FLOAT)tx, (FLOAT)ty, (FLOAT)(tx + (x2 - x)), (FLOAT)(ty + startRect.Height())), color.backgroundBrush);
							}
							if (!crlf)
							{
								UINT32 count = text::UTF16SPFirst(line.text[column]) && column + 1 < line.dataLength && text::UTF16SPSecond(line.text[column + 1]) ? 2 : 1;
								d2dRenderTarget->DrawText(
									(passwordChar ? &passwordChar : &line.text[column]),
									count,
									textFormat->textFormat.Obj(),
									D2D1::RectF((FLOAT)tx, (FLOAT)ty, (FLOAT)tx + 1, (FLOAT)ty + 1),
									color.textBrush,
									D2D1_DRAW_TEXT_OPTIONS_NO_SNAP,
									DWRITE_MEASURING_MODE_GDI_NATURAL
								);
								if (count == 2) column++;
							}
							x = x2;
						}
					}

					if (element->GetCaretVisible() && element->GetLines().IsAvailable(element->GetCaretEnd()))
					{
						Point caretPoint = element->GetLines().GetPointFromTextPos(element->GetCaretEnd());
						vint height = element->GetLines().GetRowHeight();
						Point p1(caretPoint.x - viewPosition.x + bounds.x1, caretPoint.y - viewPosition.y + bounds.y1 + 1);
						Point p2(caretPoint.x - viewPosition.x + bounds.x1, caretPoint.y + height - viewPosition.y + bounds.y1 - 1);
						d2dRenderTarget->DrawLine(
							D2D1::Point2F((FLOAT)p1.x + 0.5f, (FLOAT)p1.y),
							D2D1::Point2F((FLOAT)p2.x + 0.5f, (FLOAT)p2.y),
							caretBrush
						);
						d2dRenderTarget->DrawLine(
							D2D1::Point2F((FLOAT)p1.x - 0.5f, (FLOAT)p1.y),
							D2D1::Point2F((FLOAT)p2.x - 0.5f, (FLOAT)p2.y),
							caretBrush
						);
					}
				}
			}

			void GuiColorizedTextElementRenderer::OnElementStateChanged()
			{
				if(renderTarget)
				{
					Color caretColor=element->GetCaretColor();
					if(oldCaretColor!=caretColor)
					{
						DestroyCaretBrush(renderTarget);
						CreateCaretBrush(renderTarget);
					}
				}
			}
		}
	}
}