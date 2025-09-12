#include "GuiGraphicsDocumentRenderer.h"

namespace vl
{
	using namespace collections;

	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
SetPropertiesVisitor
***********************************************************************/

			namespace visitors
			{
				class SetPropertiesVisitor : public Object, public DocumentRun::IVisitor
				{
					typedef DocumentModel::ResolvedStyle					ResolvedStyle;
				public:
					vint							start;
					vint							length;
					vint							selectionBegin;
					vint							selectionEnd;
					List<ResolvedStyle>				styles;

					DocumentModel*					model;
					GuiDocumentElementRenderer*		renderer;
					Ptr<pg::ParagraphCache>			cache;
					IGuiGraphicsParagraph*			paragraph;

					SetPropertiesVisitor(DocumentModel* _model, GuiDocumentElementRenderer* _renderer, Ptr<pg::ParagraphCache> _cache, vint _selectionBegin, vint _selectionEnd)
						:start(0)
						,length(0)
						,model(_model)
						,renderer(_renderer)
						,cache(_cache)
						,paragraph(_cache->graphicsParagraph.Obj())
						,selectionBegin(_selectionBegin)
						,selectionEnd(_selectionEnd)
					{
						ResolvedStyle style;
						style=model->GetStyle(DocumentModel::DefaultStyleName, style);
						styles.Add(style);
					}

					void VisitContainer(DocumentContainerRun* run)
					{
						for (auto subRun : run->runs)
						{
							subRun->Accept(this);
						}
					}

					void ApplyStyle(vint start, vint length, const ResolvedStyle& style)
					{
						paragraph->SetFont(start, length, style.style.fontFamily);
						paragraph->SetSize(start, length, style.style.size);
						paragraph->SetStyle(start, length, 
							(IGuiGraphicsParagraph::TextStyle)
							( (style.style.bold?IGuiGraphicsParagraph::Bold:0)
							| (style.style.italic?IGuiGraphicsParagraph::Italic:0)
							| (style.style.underline?IGuiGraphicsParagraph::Underline:0)
							| (style.style.strikeline?IGuiGraphicsParagraph::Strikeline:0)
							));
					}

					void ApplyColor(vint start, vint length, const ResolvedStyle& style)
					{
						paragraph->SetColor(start, length, style.color);
						paragraph->SetBackgroundColor(start, length, style.backgroundColor);
					}

					void Visit(DocumentTextRun* run)override
					{
						length=run->GetRepresentationText().Length();
						if(length>0)
						{
							ResolvedStyle style=styles[styles.Count()-1];
							ApplyStyle(start, length, style);
							ApplyColor(start, length, style);

							vint styleStart=start;
							vint styleEnd=styleStart+length;
							if(styleStart<selectionEnd && selectionBegin<styleEnd)
							{
								vint s2=styleStart>selectionBegin?styleStart:selectionBegin;
								vint s3=selectionEnd<styleEnd?selectionEnd:styleEnd;

								if(s2<s3)
								{
									ResolvedStyle selectionStyle=model->GetStyle(DocumentModel::SelectionStyleName, style);
									ApplyColor(s2, s3-s2, selectionStyle);
								}
							}
						}
						start+=length;
					}

					void Visit(DocumentStylePropertiesRun* run)override
					{
						ResolvedStyle style=styles[styles.Count()-1];
						style=model->GetStyle(run->style, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count()-1);
					}

					void Visit(DocumentStyleApplicationRun* run)override
					{
						ResolvedStyle style=styles[styles.Count()-1];
						style=model->GetStyle(run->styleName, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count()-1);
					}

					void Visit(DocumentHyperlinkRun* run)override
					{
						ResolvedStyle style=styles[styles.Count()-1];
						style=model->GetStyle(run->styleName, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count()-1);
					}

					void Visit(DocumentImageRun* run)override
					{
						length=run->GetRepresentationText().Length();

						auto element=Ptr(GuiImageFrameElement::Create());
						element->SetImage(run->image, run->frameIndex);
						element->SetStretch(true);

						IGuiGraphicsParagraph::InlineObjectProperties properties;
						properties.size=run->size;
						properties.baseline=run->baseline;
						properties.breakCondition=IGuiGraphicsParagraph::Alone;
						properties.backgroundImage = element;

						paragraph->SetInlineObject(start, length, properties);

						if(start<selectionEnd && selectionBegin<start+length)
						{
							ResolvedStyle style=styles[styles.Count()-1];
							ResolvedStyle selectionStyle=model->GetStyle(DocumentModel::SelectionStyleName, style);
							ApplyColor(start, length, selectionStyle);
						}
						start+=length;
					}

					void Visit(DocumentEmbeddedObjectRun* run)override
					{
						length=run->GetRepresentationText().Length();

						IGuiGraphicsParagraph::InlineObjectProperties properties;
						properties.breakCondition=IGuiGraphicsParagraph::Alone;

						if (run->name != L"")
						{
							vint index = renderer->nameCallbackIdMap.Keys().IndexOf(run->name);
							if (index != -1)
							{
								auto id = renderer->nameCallbackIdMap.Values()[index];
								index = cache->embeddedObjects.Keys().IndexOf(id);
								if (index != -1)
								{
									auto eo = cache->embeddedObjects.Values()[index];
									if (eo->start == start)
									{
										properties.size = eo->size;
										properties.callbackId = id;
									}
								}
							}
							else
							{
								auto eo = Ptr(new pg::EmbeddedObject);
								eo->name = run->name;
								eo->size = Size(0, 0);
								eo->start = start;

								vint id = -1;
								vint count = renderer->freeCallbackIds.Count();
								if (count > 0)
								{
									id = renderer->freeCallbackIds[count - 1];
									renderer->freeCallbackIds.RemoveAt(count - 1);
								}
								else
								{
									id = renderer->usedCallbackIds++;
								}

								renderer->nameCallbackIdMap.Add(eo->name, id);
								cache->embeddedObjects.Add(id, eo);
								properties.callbackId = id;
							}
						}

						paragraph->SetInlineObject(start, length, properties);

						if(start<selectionEnd && selectionBegin<start+length)
						{
							ResolvedStyle style=styles[styles.Count()-1];
							ResolvedStyle selectionStyle=model->GetStyle(DocumentModel::SelectionStyleName, style);
							ApplyColor(start, length, selectionStyle);
						}
						start+=length;
					}

					void Visit(DocumentParagraphRun* run)override
					{
						VisitContainer(run);
					}

					static vint SetProperty(DocumentModel* model, GuiDocumentElementRenderer* renderer, auto cache, Ptr<DocumentParagraphRun> run, vint selectionBegin, vint selectionEnd)
					{
						SetPropertiesVisitor visitor(model, renderer, cache, selectionBegin, selectionEnd);
						run->Accept(&visitor);
						return visitor.length;
					}
				};
			}
			using namespace visitors;

/***********************************************************************
GuiDocumentParagraphCache
***********************************************************************/

			GuiDocumentParagraphCache::GuiDocumentParagraphCache()
				: layoutProvider(GetGuiGraphicsResourceManager()->GetLayoutProvider())
			{
			}

			GuiDocumentParagraphCache::~GuiDocumentParagraphCache()
			{
			}

			void GuiDocumentParagraphCache::Initialize(GuiDocumentElement* _element)
			{
				element = _element;
			}

			void GuiDocumentParagraphCache::RenderTargetChanged(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget)
			{
				renderTarget = newRenderTarget;
				// TODO: (enumerable) foreach
				for (vint i = 0; i < paragraphCaches.Count(); i++)
				{
					if (auto cache = paragraphCaches[i].Obj())
					{
						cache->graphicsParagraph = 0;
					}
				}
			}

			vint GuiDocumentParagraphCache::GetParagraphCount()
			{
				return paragraphCaches.Count();
			}

			Ptr<pg::ParagraphCache> GuiDocumentParagraphCache::TryGetParagraphCache(vint paragraphIndex)
			{
				if (paragraphIndex < 0 || paragraphIndex >= paragraphCaches.Count()) return nullptr;
				return paragraphCaches[paragraphIndex];
			}

			Ptr<pg::ParagraphCache> GuiDocumentParagraphCache::GetParagraphCache(vint paragraphIndex, bool requireParagraph)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::GuiDocumentParagraphCache::GetParagraphCache(vint)#"
				auto cache = paragraphCaches[paragraphIndex];
				CHECK_ERROR(cache && (!requireParagraph || cache->graphicsParagraph), ERROR_MESSAGE_PREFIX L"The specified paragraph is not created.");
				return cache;
#undef ERROR_MESSAGE_PREFIX
			}

			Size GuiDocumentParagraphCache::GetParagraphSize(vint paragraphIndex)
			{
				return paragraphSizes[paragraphIndex].cachedSize;
			}

			vint GuiDocumentParagraphCache::GetParagraphTop(vint paragraphIndex, vint paragraphDistance)
			{
				if (paragraphIndex < validCachedTops)
				{
					vint currentTop = 0;
					if (validCachedTops > 0)
					{
						auto size = paragraphSizes[validCachedTops - 1];
						currentTop = size.cachedTopWithoutParagraphDistance + size.cachedSize.y;
					}

					for (vint i = validCachedTops; i <= paragraphIndex; i++)
					{
						auto& size = paragraphSizes[i];
						size.cachedTopWithoutParagraphDistance = currentTop;
						currentTop += size.cachedSize.y;
					}

					validCachedTops = paragraphIndex + 1;
				}
				return paragraphSizes[paragraphIndex].cachedTopWithoutParagraphDistance + (paragraphIndex == 0 ? 0 : (paragraphIndex - 1) * paragraphDistance;
			}

			void GuiDocumentParagraphCache::InvalidCachedTops(vint firstParagraphIndex)
			{
				validCachedTops = firstParagraphIndex;
			}

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
			}

			void GuiDocumentElementRenderer::FinalizeInternal()
			{
			}

			void GuiDocumentElementRenderer::RenderTargetChangedInternal(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget)
			{
				pgCache.RenderTargetChanged(oldRenderTarget, newRenderTarget);
			}

			Ptr<pg::ParagraphCache> GuiDocumentElementRenderer::EnsureAndGetCache(vint paragraphIndex, bool createParagraph)
			{
				if (paragraphIndex < 0 || paragraphIndex >= paragraphCaches.Count()) return 0;
				Ptr<DocumentParagraphRun> paragraph = element->GetDocument()->paragraphs[paragraphIndex];
				auto cache = paragraphCaches[paragraphIndex];
				if (!cache)
				{
					cache = Ptr(new pg::ParagraphCache);
					cache->fullText = paragraph->GetTextForCaret();
					paragraphCaches[paragraphIndex] = cache;
				}

				if (createParagraph)
				{
					if (!cache->graphicsParagraph)
					{
						auto paragraphText = cache->fullText;
						if (auto passwordChar = element->GetPasswordChar())
						{
							Array<wchar_t> passwordText(paragraphText.Length() + 1);
							for (vint i = 0; i < paragraphText.Length(); i++)
							{
								passwordText[i] = passwordChar;
							}
							passwordText[paragraphText.Length()] = 0;
							paragraphText = &passwordText[0];
						}
						cache->graphicsParagraph = layoutProvider->CreateParagraph(paragraphText, renderTarget, this);
						cache->graphicsParagraph->SetParagraphAlignment(paragraph->alignment ? paragraph->alignment.Value() : Alignment::Left);
						cache->graphicsParagraph->SetWrapLine(element->GetWrapLine());
						SetPropertiesVisitor::SetProperty(element->GetDocument().Obj(), this, cache, paragraph, cache->selectionBegin, cache->selectionEnd);
					}
					if (cache->graphicsParagraph->GetMaxWidth() != lastMaxWidth)
					{
						cache->graphicsParagraph->SetMaxWidth(lastMaxWidth);
					}

					Size cachedSize = paragraphSizes[paragraphIndex];
					Size realSize = cache->graphicsParagraph->GetSize();
					if (lastTotalSize.x < realSize.x)
					{
						lastTotalSize.x = realSize.x;
					}
					if (cachedSize.y != realSize.y)
					{
						lastTotalSize.y += realSize.y - cachedSize.y;
					}
					paragraphSizes[paragraphIndex] = realSize;
					minSize = lastTotalSize;
				}

				return cache;
			}

			bool GuiDocumentElementRenderer::GetParagraphIndexFromPoint(Point point, vint& top, vint& index)
			{
				vint y = 0;
				// TODO: (enumerable) foreach
				for (vint i = 0; i < paragraphSizes.Count(); i++)
				{
					vint paragraphHeight = paragraphSizes[i].y;
					vint nextY = y + paragraphHeight + paragraphDistance;
					top = y;
					index = i;

					if (nextY <= point.y)
					{
						y = nextY;
						continue;
					}
					else
					{
						break;
					}
				}
				return true;
			}

			GuiDocumentElementRenderer::GuiDocumentElementRenderer()
			{
			}

			void GuiDocumentElementRenderer::Render(Rect bounds)
			{
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
					vint y = 0;

					lastMaxWidth = maxWidth;

					// TODO: (enumerable) foreach
					vint paragraphCount = paragraphSizes.Count();
					auto document = element->GetDocument();
					if (paragraphCount > document->paragraphs.Count())
					{
						paragraphCount = document->paragraphs.Count();
					}
					for (vint i = 0; i < paragraphCount; i++)
					{
						Size cachedSize = paragraphSizes[i];
						if (y + cachedSize.y <= y1)
						{
							y += cachedSize.y + paragraphDistance;
							continue;
						}
						else if (y >= y2)
						{
							break;
						}
						else
						{
							Ptr<DocumentParagraphRun> paragraph = document->paragraphs[i];
							auto cache = paragraphCaches[i];
							bool created = cache && cache->graphicsParagraph;
							cache = EnsureAndGetCache(i, true);
							if (!created && i == lastCaret.row && element->GetCaretVisible())
							{
								cache->graphicsParagraph->OpenCaret(lastCaret.column, lastCaretColor, lastCaretFrontSide);
							}

							cachedSize = cache->graphicsParagraph->GetSize();

							renderingParagraph = i;
							renderingParagraphOffset = Point(cx - bounds.x1, cy + y - bounds.y1);
							cache->graphicsParagraph->Render(Rect(Point(cx, cy + y), Size(maxWidth, cachedSize.y)));
							renderingParagraph = -1;

							bool resized = false;
							// TODO: (enumerable) foreach
							for (vint j = 0; j < cache->embeddedObjects.Count(); j++)
							{
								auto eo = cache->embeddedObjects.Values()[j];
								if (eo->resized)
								{
									eo->resized = false;
									resized = true;
								}
							}

							if (resized)
							{
								cache->graphicsParagraph = 0;
							}
						}

						y += cachedSize.y + paragraphDistance;
					}
				}
				renderTarget->PopClipper(element);
				if (auto callback = element->GetCallback())
				{
					callback->OnFinishRender();
				}
			}

			void GuiDocumentElementRenderer::NotifyParagraphPaddingUpdated(bool value)
			{
				vint defaultHeight = GetCurrentController()->ResourceService()->GetDefaultFont().size;
				paragraphDistance = element->GetParagraphPadding() ? defaultHeight : 0;
			}

			void GuiDocumentElementRenderer::OnElementStateChanged()
			{
				lastTotalSize = { 1,1 };
				auto document = element->GetDocument();
				if (document && document->paragraphs.Count() > 0)
				{
					vint defaultHeight = GetCurrentController()->ResourceService()->GetDefaultFont().size;

					paragraphCaches.Resize(document->paragraphs.Count());
					paragraphSizes.Resize(document->paragraphs.Count());

					for (vint i = 0; i < paragraphCaches.Count(); i++)
					{
						paragraphCaches[i] = 0;
					}
					for (vint i = 0; i < paragraphSizes.Count(); i++)
					{
						paragraphSizes[i] = { 0,defaultHeight };
					}

					lastTotalSize.y = paragraphSizes.Count() * (defaultHeight + paragraphDistance);
					if (paragraphSizes.Count() > 0)
					{
						lastTotalSize.y -= paragraphDistance;
					}
				}
				else
				{
					paragraphCaches.Resize(0);
					paragraphSizes.Resize(0);
				}
				minSize = lastTotalSize;

				nameCallbackIdMap.Clear();
				freeCallbackIds.Clear();
				usedCallbackIds = 0;
			}

			void GuiDocumentElementRenderer::NotifyParagraphUpdated(vint index, vint oldCount, vint newCount, bool updatedText)
			{
				if (0 <= index && index < paragraphCaches.Count() && 0 <= oldCount && index + oldCount <= paragraphCaches.Count() && 0 <= newCount)
				{
					vint paragraphCount = element->GetDocument()->paragraphs.Count();
					CHECK_ERROR(updatedText || oldCount == newCount, L"GuiDocumentlement::GuiDocumentElementRenderer::NotifyParagraphUpdated(vint, vint, vint, bool)#Illegal values of oldCount and newCount.");
					CHECK_ERROR(paragraphCount - paragraphCaches.Count() == newCount - oldCount, L"GuiDocumentElementRenderer::NotifyParagraphUpdated(vint, vint, vint, bool)#Illegal values of oldCount and newCount.");

					pg::ParagraphCacheArray oldCaches;
					CopyFrom(oldCaches, paragraphCaches);
					paragraphCaches.Resize(paragraphCount);

					pg::ParagraphSizeArray oldSizes;
					CopyFrom(oldSizes, paragraphSizes);
					paragraphSizes.Resize(paragraphCount);

					vint defaultHeight = GetCurrentController()->ResourceService()->GetDefaultFont().size;
					lastTotalSize = { 1,1 };

					for (vint i = 0; i < paragraphCount; i++)
					{
						if (i < index)
						{
							paragraphCaches[i] = oldCaches[i];
							paragraphSizes[i] = oldSizes[i];
						}
						else if (i < index + newCount)
						{
							paragraphCaches[i] = 0;
							paragraphSizes[i] = { 0,defaultHeight };
							if (!updatedText && i < index + oldCount)
							{
								auto cache = oldCaches[i];
								if (cache)
								{
									cache->graphicsParagraph = 0;
								}
								paragraphCaches[i] = cache;
								paragraphSizes[i] = oldSizes[i];
							}
						}
						else
						{
							paragraphCaches[i] = oldCaches[i - (newCount - oldCount)];
							paragraphSizes[i] = oldSizes[i - (newCount - oldCount)];
						}

						auto cachedSize = paragraphSizes[i];
						if (lastTotalSize.x < cachedSize.x)
						{
							lastTotalSize.x = cachedSize.x;
						}
						lastTotalSize.y += cachedSize.y + paragraphDistance;
					}
					if (paragraphCount > 0)
					{
						lastTotalSize.y -= paragraphDistance;
					}

					if (updatedText)
					{
						vint count = oldCount < newCount ? oldCount : newCount;
						for (vint i = 0; i < count; i++)
						{
							if (auto cache = oldCaches[index + i])
							{
								// TODO: (enumerable) foreach on dictionary
								for (vint j = 0; j < cache->embeddedObjects.Count(); j++)
								{
									auto id = cache->embeddedObjects.Keys()[j];
									auto name = cache->embeddedObjects.Values()[j]->name;
									nameCallbackIdMap.Remove(name);
									freeCallbackIds.Add(id);
								}
							}
						}
					}
					minSize = lastTotalSize;
				}
			}

			Ptr<DocumentHyperlinkRun::Package> GuiDocumentElementRenderer::GetHyperlinkFromPoint(Point point)
			{
				if (!renderTarget) return nullptr;
				vint top = 0;
				vint index = -1;
				if (GetParagraphIndexFromPoint(point, top, index))
				{
					auto document = element->GetDocument();
					auto cache = EnsureAndGetCache(index, true);
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
				return nullptr;
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
				// TODO: (enumerable) foreach:indexed
				for (vint i = 0; i < paragraphCaches.Count(); i++)
				{
					if (begin.row <= i && i <= end.row)
					{
						auto cache = EnsureAndGetCache(i, false);
						vint newBegin = i == begin.row ? begin.column : 0;
						vint newEnd = i == end.row ? end.column : cache->fullText.Length();

						if (cache->selectionBegin != newBegin || cache->selectionEnd != newEnd)
						{
							cache->selectionBegin = newBegin;
							cache->selectionEnd = newEnd;
							NotifyParagraphUpdated(i, 1, 1, false);
						}
					}
					else
					{
						auto cache = paragraphCaches[i];
						if (cache)
						{
							if (cache->selectionBegin != -1 || cache->selectionEnd != -1)
							{
								cache->selectionBegin = -1;
								cache->selectionEnd = -1;
								NotifyParagraphUpdated(i, 1, 1, false);
							}
						}
					}
				}
			}

			TextPos GuiDocumentElementRenderer::CalculateCaret(TextPos comparingCaret, IGuiGraphicsParagraph::CaretRelativePosition position, bool& preferFrontSide)
			{
				if (!renderTarget) return comparingCaret;
				auto cache = EnsureAndGetCache(comparingCaret.row, true);
				if (cache)
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
								auto anotherCache = EnsureAndGetCache(comparingCaret.row - 1, true);
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
								auto anotherCache = EnsureAndGetCache(comparingCaret.row + 1, true);
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
								auto anotherCache = EnsureAndGetCache(comparingCaret.row - 1, true);
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
								auto anotherCache = EnsureAndGetCache(comparingCaret.row + 1, true);
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
				vint top = 0;
				vint index = -1;
				if (GetParagraphIndexFromPoint(point, top, index))
				{
					auto cache = EnsureAndGetCache(index, true);
					Point paragraphPoint(point.x, point.y - top);
					vint caret = cache->graphicsParagraph->GetCaretFromPoint(paragraphPoint);
					return TextPos(index, caret);
				}
				return TextPos(-1, -1);
			}

			Rect GuiDocumentElementRenderer::GetCaretBounds(TextPos caret, bool frontSide)
			{
				if (!renderTarget) return Rect();
				auto cache = EnsureAndGetCache(caret.row, true);
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