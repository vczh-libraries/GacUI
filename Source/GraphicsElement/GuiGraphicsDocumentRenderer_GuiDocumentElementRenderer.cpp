#include "GuiGraphicsDocumentRenderer.h"

namespace vl
{
	using namespace collections;

	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
GuiDocumentElementRenderer
***********************************************************************/

			Size GuiDocumentElementRenderer::OnRenderInlineObject(vint callbackId, Rect location)
			{
				if (auto callback = element->GetCallback())
				{
					auto cache = pgCache.GetParagraphCache(renderingParagraph, true);
					auto relativeLocation = Rect(Point(location.x1 + renderingParagraphOffset.x, location.y1 + renderingParagraphOffset.y), location.GetSize());
					auto eo = cache->embeddedObjects[callbackId];
					auto size = callback->OnRenderEmbeddedObject(eo->name, relativeLocation);
					eo->resized = eo->size != size;
					eo->size = size;
					return eo->size;
				}
				else
				{
					return Size();
				}
			}

			void GuiDocumentElementRenderer::InitializeInternal()
			{
				pgCache.Initialize(element);
				imageCache.Initialize(element);
				NotifyParagraphPaddingUpdated(element->GetParagraphPadding());
			}

			void GuiDocumentElementRenderer::FinalizeInternal()
			{
			}

			void GuiDocumentElementRenderer::RenderTargetChangedInternal(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget)
			{
				pgCache.RenderTargetChanged(oldRenderTarget, newRenderTarget);
				imageCache.RenderTargetChanged(oldRenderTarget, newRenderTarget);
			}

			Ptr<pg::ParagraphCache> GuiDocumentElementRenderer::EnsureParagraph(vint paragraphIndex)
			{
				lastTotalHeightWithoutParagraphDistance += pgCache.EnsureParagraph(paragraphIndex, lastMaxWidth);
				vint width = pgCache.GetParagraphSize(paragraphIndex).x;
				if (lastTotalWidth < width)
				{
					lastTotalWidth = width;
				}
				FixMinSize();
				return pgCache.GetParagraphCache(paragraphIndex, true);
			}

			void GuiDocumentElementRenderer::FixMinSize()
			{
				minSize = { lastTotalWidth,lastTotalHeightWithoutParagraphDistance };
				if (pgCache.GetParagraphCount() > 0)
				{
					minSize.y += paragraphDistance * (pgCache.GetParagraphCount() - 1);
				}

				if (minSize.x < 1) minSize.x = 1;
				if (minSize.y < 1) minSize.y = 1;
			}

			void GuiDocumentElementRenderer::UpdateRenderRange(vint index, vint oldCount, vint newCount)
			{
				// Range adjustment logic for paragraph recycling
				if (previousRenderBegin != -1)
				{
					// Handle the three cases for range adjustment when text is actually updated
					vint prevEnd = previousRenderBegin + previousRenderCount;
					vint oldEnd = index + oldCount;
					
					if (previousRenderBegin + previousRenderCount <= index)
					{
						// Case 1: Previous range is completely before the updated area - no adjustment needed
					}
					else if (previousRenderBegin >= oldEnd)
					{
						// Case 2: Previous range is completely after the updated area - adjust begin position
						previousRenderBegin += (newCount - oldCount);
					}
					else
					{
						// Case 3: Previous range overlaps with updated area - use union approach
						vint newBegin = previousRenderBegin < index ? previousRenderBegin : index;
						vint unionEnd = prevEnd > oldEnd ? prevEnd : oldEnd;
						vint newCountAdjustment = unionEnd - newBegin + (newCount - oldCount);
						
						previousRenderBegin = newBegin;
						previousRenderCount = newCountAdjustment;
					}
					
					// Reset to invalid state when count becomes 0 (per UPDATE guidance)
					if (previousRenderCount <= 0)
					{
						previousRenderBegin = -1;
						previousRenderCount = 0;
					}
				}
			}

			void GuiDocumentElementRenderer::UpdateRenderRangeAndCleanUp(vint currentBegin, vint currentCount)
			{
				// Apply recycling logic if we have valid previous range
				if (previousRenderBegin != -1 && element->GetParagraphRecycle())
				{
					if (currentBegin == -1)
					{
						// Release everything when no paragraphs are currently visible
						pgCache.ReleaseParagraphs(previousRenderBegin, previousRenderCount);
					}
					else
					{
						vint prevEnd = previousRenderBegin + previousRenderCount;
						vint currEnd = currentBegin + currentCount;
						
						// Release paragraphs before current range
						if (previousRenderBegin < currentBegin)
						{
							vint releaseEnd = prevEnd < currentBegin ? prevEnd : currentBegin;
							pgCache.ReleaseParagraphs(previousRenderBegin, releaseEnd - previousRenderBegin);
						}
						
						// Release paragraphs after current range
						if (prevEnd > currEnd)
						{
							vint releaseBegin = currEnd > previousRenderBegin ? currEnd : previousRenderBegin;
							pgCache.ReleaseParagraphs(releaseBegin, prevEnd - releaseBegin);
						}
					}
				}
				
				// Update tracking variables for next render cycle
				previousRenderBegin = currentBegin;
				previousRenderCount = currentCount;
				
				// Reset to invalid state when no paragraphs are visible (per UPDATE guidance)
				if (previousRenderCount == 0)
				{
					previousRenderBegin = -1;
				}
			}

			void GuiDocumentElementRenderer::ApplyPropertiesOnParagraph(vint paragraphIndex, vint start, vint end, vint maxWidth)
			{
				auto cache = pgCache.GetParagraphCache(paragraphIndex, true);
				auto paragraph = element->GetDocument()->paragraphs[paragraphIndex];
				visitors::SetProperties(
					element->GetDocument().Obj(),
					&pgCache,
					&imageCache,
					cache,
					paragraph,
					cache->selectionBegin,
					cache->selectionEnd,
					start,
					end
				);
				cache->graphicsParagraph->SetParagraphAlignment(paragraph->alignment ? paragraph->alignment.Value() : Alignment::Left);
				cache->graphicsParagraph->SetWrapLine(element->GetWrapLine());
				cache->graphicsParagraph->SetMaxWidth(maxWidth);
			}

			GuiDocumentElementRenderer::GuiDocumentElementRenderer()
				: pgCache(this)
			{
			}

			GuiDocumentElementRenderer::~GuiDocumentElementRenderer()
			{
			}

			void GuiDocumentElementRenderer::Render(Rect bounds)
			{
				List<vint> paragraphsToReset;
				if (auto callback = element->GetCallback())
				{
					callback->OnStartRender();
				}
				renderTarget->PushClipper(bounds, element);
				if (!renderTarget->IsClipperCoverWholeTarget())
				{
					vint maxWidth = bounds.Width();
					Rect clipper = renderTarget->GetClipper();
					vint cx = bounds.Left();
					vint cy = bounds.Top();
					vint y1 = clipper.Top() - bounds.Top();
					vint y2 = y1 + clipper.Height();

					lastMaxWidth = maxWidth;

					// TODO: (enumerable) foreach
					vint paragraphCount = pgCache.GetParagraphCount();
					auto document = element->GetDocument();
					if (paragraphCount > document->paragraphs.Count())
					{
						paragraphCount = document->paragraphs.Count();
					}

					vint startParagraph = pgCache.GetParagraphFromY(y1, paragraphDistance);
					for (vint i = startParagraph; i < paragraphCount; i++)
					{
						Ptr<DocumentParagraphRun> paragraph = document->paragraphs[i];
						auto cache = pgCache.TryGetParagraphCache(i);
						bool paragraphAlreadyCreated = cache && cache->graphicsParagraph;

						cache = EnsureParagraph(i);
						if (!paragraphAlreadyCreated && i == lastCaret.row && element->GetCaretVisible())
						{
							cache->graphicsParagraph->OpenCaret(lastCaret.column, lastCaretColor, lastCaretFrontSide);
						}

						vint y = pgCache.GetParagraphTop(i, paragraphDistance);
						if (y >= y2)
						{
							break;
						}

						renderingParagraph = i;
						renderingParagraphOffset = Point(cx - bounds.x1, cy + y - bounds.y1);
						cache->graphicsParagraph->Render(Rect(Point(cx, cy + y), Size(maxWidth, pgCache.GetParagraphSize(i).y)));
						renderingParagraph = -1;

						bool resized = false;
						for (auto eo : cache->embeddedObjects.Values())
						{
							if (eo->resized)
							{
								eo->resized = false;
								resized = true;
							}
						}

						if (resized)
						{
							paragraphsToReset.Add(i);
						}
					}
				}
				renderTarget->PopClipper(element);
				if (auto callback = element->GetCallback())
				{
					callback->OnFinishRender();
				}

				// Paragraph recycling logic
				if (element->GetParagraphRecycle())
				{
					// Calculate current visible range (currentBegin, currentCount)
					vint currentBegin = -1;
					vint currentCount = 0;

					// Determine currentBegin using existing GetParagraphFromY logic
					Rect clipper = renderTarget->GetClipper();
					vint y1 = clipper.Top() - bounds.Top();
					vint y2 = y1 + clipper.Height();

					if (y1 < y2) // Only if there's visible area
					{
						currentBegin = pgCache.GetParagraphFromY(y1, paragraphDistance);
						if (currentBegin != -1)
						{
							// Count consecutive visible paragraphs
							vint y = pgCache.GetParagraphTop(currentBegin, paragraphDistance);
							for (vint i = currentBegin; i < pgCache.GetParagraphCount() && y < y2; i++)
							{
								currentCount++;
								y += pgCache.GetParagraphSize(i).y + paragraphDistance;
							}
						}
					}

					UpdateRenderRangeAndCleanUp(currentBegin, currentCount);
				}

				FixMinSize();

				for (auto p : paragraphsToReset)
				{
					NotifyParagraphStyleUpdated(p, 1);
				}
			}

			void GuiDocumentElementRenderer::NotifyParagraphPaddingUpdated(bool value)
			{
				vint defaultHeight = GuiDocumentParagraphCache::GetDefaultHeight();
				paragraphDistance = element->GetParagraphPadding() ? defaultHeight : 0;
			}

			void GuiDocumentElementRenderer::OnElementStateChanged()
			{
				lastTotalWidth = 0;
				lastTotalHeightWithoutParagraphDistance = pgCache.ResetCache();
				FixMinSize();
			}

			void GuiDocumentElementRenderer::NotifyParagraphUpdateLastTotalWidth(vint index, vint count)
			{
				for (vint i = 0; i < count; i++)
				{
					vint width = pgCache.GetParagraphSize(index + i).x;
					if (lastTotalWidth == width)
					{
						lastTotalWidth = 0;
						break;
					}
				}
			}

			void GuiDocumentElementRenderer::NotifyParagraphTextUpdated(vint index, vint oldCount, vint newCount)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::GuiDocumentElementRenderer::NotifyParagraphTextUpdated(vint, vint, vint)#"
				vint oldParagraphCount = pgCache.GetParagraphCount();
				vint newParagraphCount = element->GetDocument()->paragraphs.Count();

				CHECK_ERROR(oldCount >= 0, ERROR_MESSAGE_PREFIX L"oldCount cannot be negative.");
				CHECK_ERROR(newCount >= 0, ERROR_MESSAGE_PREFIX L"newCount cannot be negative.");
				CHECK_ERROR(0 <= index && index + oldCount <= oldParagraphCount, ERROR_MESSAGE_PREFIX L"index + oldCount is out of range.");
				CHECK_ERROR(0 <= index && index + newCount <= newParagraphCount, ERROR_MESSAGE_PREFIX L"index + newCount is out of range.");
				CHECK_ERROR(newParagraphCount - oldParagraphCount == newCount - oldCount, ERROR_MESSAGE_PREFIX L"newCount - oldCount does not reflect the actual paragraph count changing.");

				imageCache.ResetTextCache(index, oldCount, newCount);
				NotifyParagraphUpdateLastTotalWidth(index, oldCount);
				lastTotalHeightWithoutParagraphDistance += pgCache.ResetTextCache(index, oldCount, newCount);
				FixMinSize();
				UpdateRenderRange(index, oldCount, newCount);

#undef ERROR_MESSAGE_PREFIX
			}

			void GuiDocumentElementRenderer::NotifyParagraphStyleUpdated(TextPos begin, TextPos end)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::GuiDocumentElementRenderer::NotifyParagraphStyleUpdated(vint, TextPos, TextPos)#"
				vint paragraphCount = pgCache.GetParagraphCount();
				CHECK_ERROR(paragraphCount == element->GetDocument()->paragraphs.Count(), ERROR_MESSAGE_PREFIX L"This function can only be called when only text style is updated.");
				CHECK_ERROR(0 <= begin.row && begin.row < paragraphCount, ERROR_MESSAGE_PREFIX L"begin.row is out of range.");
				CHECK_ERROR(0 <= end.row && end.row < paragraphCount, ERROR_MESSAGE_PREFIX L"end.row is out of range.");

				vint count = end.row - begin.row + 1;
				NotifyParagraphUpdateLastTotalWidth(begin.row, count);
				lastTotalHeightWithoutParagraphDistance += pgCache.ResetStyleCache(begin, end);
				FixMinSize();

#undef ERROR_MESSAGE_PREFIX
			}

			void GuiDocumentElementRenderer::NotifyParagraphStyleUpdated(vint index, vint count)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::GuiDocumentElementRenderer::NotifyParagraphTextUpdated(vint, vint, vint)#"
				vint oldParagraphCount = pgCache.GetParagraphCount();
				vint newParagraphCount = element->GetDocument()->paragraphs.Count();

				CHECK_ERROR(count >= 0, ERROR_MESSAGE_PREFIX L"count cannot be negative.");
				CHECK_ERROR(0 <= index && index + count <= newParagraphCount, ERROR_MESSAGE_PREFIX L"index + count is out of range.");
				NotifyParagraphUpdateLastTotalWidth(index, count);
				lastTotalHeightWithoutParagraphDistance += pgCache.ResetStyleCache(index, count);
				FixMinSize();
#undef ERROR_MESSAGE_PREFIX
			}

			Ptr<DocumentHyperlinkRun::Package> GuiDocumentElementRenderer::GetHyperlinkFromPoint(Point point)
			{
				if (!renderTarget) return nullptr;
				vint index = pgCache.GetParagraphFromY(point.y, paragraphDistance);
				vint top = pgCache.GetParagraphTop(index, paragraphDistance);

				auto document = element->GetDocument();
				auto cache = EnsureParagraph(index);
				Point paragraphPoint(point.x, point.y - top);

				vint start = -1;
				vint length = 0;
				if (cache->graphicsParagraph->GetInlineObjectFromPoint(paragraphPoint, start, length))
				{
					return document->GetHyperlink(index, start, start + length);
				}

				vint caret = cache->graphicsParagraph->GetCaretFromPoint(paragraphPoint);
				return document->GetHyperlink(index, caret, caret);
			}

			void GuiDocumentElementRenderer::OpenCaret(TextPos caret, Color color, bool frontSide)
			{
				CloseCaret(caret);
				lastCaret = caret;
				lastCaretColor = color;
				lastCaretFrontSide = frontSide;

				auto cache = pgCache.TryGetParagraphCache(lastCaret.row);
				if (cache && cache->graphicsParagraph)
				{
					cache->graphicsParagraph->OpenCaret(lastCaret.column, lastCaretColor, lastCaretFrontSide);
				}
			}

			void GuiDocumentElementRenderer::CloseCaret(TextPos caret)
			{
				if (lastCaret != TextPos(-1, -1))
				{
					auto cache = pgCache.TryGetParagraphCache(lastCaret.row);
					if (cache && cache->graphicsParagraph)
					{
						cache->graphicsParagraph->CloseCaret();
					}
				}
				lastCaret = caret;
			}

			void GuiDocumentElementRenderer::SetSelection(TextPos begin, TextPos end)
			{
				if (begin > end)
				{
					TextPos t = begin;
					begin = end;
					end = t;
				}
				if (begin == end)
				{
					begin = TextPos(-1, -1);
					end = TextPos(-1, -1);
				}

				if (!renderTarget) return;
				vint paragraphCount = pgCache.GetParagraphCount();
				for (vint i = 0; i < paragraphCount; i++)
				{
					if (begin.row <= i && i <= end.row)
					{
						if (auto cache = pgCache.TryGetParagraphCache(i))
						{
							vint newBegin = i == begin.row ? begin.column : 0;
							vint newEnd = i == end.row ? end.column : cache->fullText.Length();

							if (cache->selectionBegin != newBegin || cache->selectionEnd != newEnd)
							{
								vint selectionBegin = cache->selectionBegin;
								vint selectionEnd = cache->selectionEnd;
								if (newBegin < selectionBegin || selectionBegin == -1) selectionBegin = newBegin;
								if (newEnd > selectionEnd || selectionEnd == -1) selectionEnd = newEnd;
								cache->selectionBegin = newBegin;
								cache->selectionEnd = newEnd;
								if (cache->graphicsParagraph)
								{
									NotifyParagraphStyleUpdated({ i,selectionBegin }, { i,selectionEnd });
								}
							}
						}
					}
					else
					{
						if (auto cache = pgCache.TryGetParagraphCache(i))
						{
							if (cache->selectionBegin != -1 || cache->selectionEnd != -1)
							{
								vint selectionBegin = cache->selectionBegin;
								vint selectionEnd = cache->selectionEnd;
								cache->selectionBegin = -1;
								cache->selectionEnd = -1;
								if (cache->graphicsParagraph)
								{
									NotifyParagraphStyleUpdated({ i,selectionBegin }, { i,selectionEnd });
								}
							}
						}
					}
				}
			}

			TextPos GuiDocumentElementRenderer::CalculateCaret(TextPos comparingCaret, IGuiGraphicsParagraph::CaretRelativePosition position, bool& preferFrontSide)
			{
				if (!renderTarget) return comparingCaret;
				if (auto cache = EnsureParagraph(comparingCaret.row))
				{
					switch (position)
					{
					case IGuiGraphicsParagraph::CaretFirst:
						{
							preferFrontSide = false;
							vint caret = cache->graphicsParagraph->GetCaret(0, IGuiGraphicsParagraph::CaretFirst, preferFrontSide);
							return TextPos(comparingCaret.row, caret);
						}
					case IGuiGraphicsParagraph::CaretLast:
						{
							preferFrontSide = true;
							vint caret = cache->graphicsParagraph->GetCaret(0, IGuiGraphicsParagraph::CaretLast, preferFrontSide);
							return TextPos(comparingCaret.row, caret);
						}
					case IGuiGraphicsParagraph::CaretLineFirst:
						{
							preferFrontSide = false;
							vint caret = cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretLineFirst, preferFrontSide);
							return TextPos(comparingCaret.row, caret);
						}
					case IGuiGraphicsParagraph::CaretLineLast:
						{
							preferFrontSide = true;
							vint caret = cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretLineLast, preferFrontSide);
							return TextPos(comparingCaret.row, caret);
						}
					case IGuiGraphicsParagraph::CaretMoveUp:
						{
							vint caret = cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretMoveUp, preferFrontSide);
							if (caret == comparingCaret.column && comparingCaret.row > 0)
							{
								Rect caretBounds = cache->graphicsParagraph->GetCaretBounds(comparingCaret.column, preferFrontSide);
								auto anotherCache = EnsureParagraph(comparingCaret.row - 1);
								vint height = anotherCache->graphicsParagraph->GetSize().y;
								caret = anotherCache->graphicsParagraph->GetCaretFromPoint(Point(caretBounds.x1, height));
								return TextPos(comparingCaret.row - 1, caret);
							}
							else
							{
								return TextPos(comparingCaret.row, caret);
							}
						}
					case IGuiGraphicsParagraph::CaretMoveDown:
						{
							vint caret = cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretMoveDown, preferFrontSide);
							if (caret == comparingCaret.column && comparingCaret.row < pgCache.GetParagraphCount() - 1)
							{
								Rect caretBounds = cache->graphicsParagraph->GetCaretBounds(comparingCaret.column, preferFrontSide);
								auto anotherCache = EnsureParagraph(comparingCaret.row + 1);
								caret = anotherCache->graphicsParagraph->GetCaretFromPoint(Point(caretBounds.x1, 0));
								return TextPos(comparingCaret.row + 1, caret);
							}
							else
							{
								return TextPos(comparingCaret.row, caret);
							}
						}
					case IGuiGraphicsParagraph::CaretMoveLeft:
						{
							preferFrontSide = false;
							vint caret = cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretMoveLeft, preferFrontSide);
							if (caret == comparingCaret.column && comparingCaret.row > 0)
							{
								auto anotherCache = EnsureParagraph(comparingCaret.row - 1);
								caret = anotherCache->graphicsParagraph->GetCaret(0, IGuiGraphicsParagraph::CaretLast, preferFrontSide);
								return TextPos(comparingCaret.row - 1, caret);
							}
							else
							{
								return TextPos(comparingCaret.row, caret);
							}
						}
					case IGuiGraphicsParagraph::CaretMoveRight:
						{
							preferFrontSide = true;
							vint caret = cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretMoveRight, preferFrontSide);
							if (caret == comparingCaret.column && comparingCaret.row < pgCache.GetParagraphCount() - 1)
							{
								auto anotherCache = EnsureParagraph(comparingCaret.row + 1);
								caret = anotherCache->graphicsParagraph->GetCaret(0, IGuiGraphicsParagraph::CaretFirst, preferFrontSide);
								return TextPos(comparingCaret.row + 1, caret);
							}
							else
							{
								return TextPos(comparingCaret.row, caret);
							}
						}
					}
				}
				return comparingCaret;
			}

			TextPos GuiDocumentElementRenderer::CalculateCaretFromPoint(Point point)
			{
				if (!renderTarget) return TextPos(-1, -1);
				vint index = pgCache.GetParagraphFromY(point.y, paragraphDistance);
				vint top = pgCache.GetParagraphTop(index, paragraphDistance);

				auto cache = EnsureParagraph(index);
				Point paragraphPoint(point.x, point.y - top);
				vint caret = cache->graphicsParagraph->GetCaretFromPoint(paragraphPoint);
				return TextPos(index, caret);
			}

			Rect GuiDocumentElementRenderer::GetCaretBounds(TextPos caret, bool frontSide)
			{
				if (!renderTarget) return Rect();
				auto cache = EnsureParagraph(caret.row);
				if (cache)
				{
					Rect bounds = cache->graphicsParagraph->GetCaretBounds(caret.column, frontSide);
					if (bounds != Rect())
					{
						vint y = pgCache.GetParagraphTop(caret.row, paragraphDistance);
						bounds.y1 += y;
						bounds.y2 += y;
						return bounds;
					}
				}
				return Rect();
			}
		}
	}
}