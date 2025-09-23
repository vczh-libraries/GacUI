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
					GuiDocumentParagraphCache*		paragraphCache;
					Ptr<pg::ParagraphCache>			cache;
					IGuiGraphicsParagraph*			paragraph;

					SetPropertiesVisitor(DocumentModel* _model, GuiDocumentParagraphCache* _paragraphCache, Ptr<pg::ParagraphCache> _cache, vint _selectionBegin, vint _selectionEnd)
						: start(0)
						, length(0)
						, model(_model)
						, paragraphCache(_paragraphCache)
						, cache(_cache)
						, paragraph(_cache->graphicsParagraph.Obj())
						, selectionBegin(_selectionBegin)
						, selectionEnd(_selectionEnd)
					{
						ResolvedStyle style;
						style = model->GetStyle(DocumentModel::DefaultStyleName, style);
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
							((style.style.bold ? IGuiGraphicsParagraph::Bold : 0)
							| (style.style.italic ? IGuiGraphicsParagraph::Italic : 0)
							| (style.style.underline ? IGuiGraphicsParagraph::Underline : 0)
							| (style.style.strikeline ? IGuiGraphicsParagraph::Strikeline : 0)
							));
					}

					void ApplyColor(vint start, vint length, const ResolvedStyle& style)
					{
						paragraph->SetColor(start, length, style.color);
						paragraph->SetBackgroundColor(start, length, style.backgroundColor);
					}

					void Visit(DocumentTextRun* run)override
					{
						length = run->GetRepresentationText().Length();
						if (length > 0)
						{
							ResolvedStyle style = styles[styles.Count() - 1];
							ApplyStyle(start, length, style);
							ApplyColor(start, length, style);

							vint styleStart = start;
							vint styleEnd = styleStart + length;
							if (styleStart < selectionEnd && selectionBegin < styleEnd)
							{
								vint s2 = styleStart > selectionBegin ? styleStart : selectionBegin;
								vint s3 = selectionEnd < styleEnd ? selectionEnd : styleEnd;

								if (s2 < s3)
								{
									ResolvedStyle selectionStyle = model->GetStyle(DocumentModel::SelectionStyleName, style);
									ApplyColor(s2, s3 - s2, selectionStyle);
								}
							}
						}
						start += length;
					}

					void Visit(DocumentStylePropertiesRun* run)override
					{
						ResolvedStyle style = styles[styles.Count() - 1];
						style = model->GetStyle(run->style, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count() - 1);
					}

					void Visit(DocumentStyleApplicationRun* run)override
					{
						ResolvedStyle style = styles[styles.Count() - 1];
						style = model->GetStyle(run->styleName, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count() - 1);
					}

					void Visit(DocumentHyperlinkRun* run)override
					{
						ResolvedStyle style = styles[styles.Count() - 1];
						style = model->GetStyle(run->styleName, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count() - 1);
					}

					void Visit(DocumentImageRun* run)override
					{
						length = run->GetRepresentationText().Length();

						auto element = Ptr(GuiImageFrameElement::Create());
						element->SetImage(run->image, run->frameIndex);
						element->SetStretch(true);

						IGuiGraphicsParagraph::InlineObjectProperties properties;
						properties.size = run->size;
						properties.baseline = run->baseline;
						properties.breakCondition = IGuiGraphicsParagraph::Alone;
						properties.backgroundImage = element;

						paragraph->SetInlineObject(start, length, properties);

						if (start < selectionEnd && selectionBegin < start + length)
						{
							ResolvedStyle style = styles[styles.Count() - 1];
							ResolvedStyle selectionStyle = model->GetStyle(DocumentModel::SelectionStyleName, style);
							ApplyColor(start, length, selectionStyle);
						}
						start += length;
					}

					void Visit(DocumentEmbeddedObjectRun* run)override
					{
						length = run->GetRepresentationText().Length();

						IGuiGraphicsParagraph::InlineObjectProperties properties;
						properties.breakCondition = IGuiGraphicsParagraph::Alone;

						if (run->name != L"")
						{
							vint index = paragraphCache->nameCallbackIdMap.Keys().IndexOf(run->name);
							if (index != -1)
							{
								auto id = paragraphCache->nameCallbackIdMap.Values()[index];
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
								vint count = paragraphCache->freeCallbackIds.Count();
								if (count > 0)
								{
									id = paragraphCache->freeCallbackIds[count - 1];
									paragraphCache->freeCallbackIds.RemoveAt(count - 1);
								}
								else
								{
									id = paragraphCache->usedCallbackIds++;
								}

								paragraphCache->nameCallbackIdMap.Add(eo->name, id);
								cache->embeddedObjects.Add(id, eo);
								properties.callbackId = id;
							}
						}

						paragraph->SetInlineObject(start, length, properties);

						if (start < selectionEnd && selectionBegin < start + length)
						{
							ResolvedStyle style = styles[styles.Count() - 1];
							ResolvedStyle selectionStyle = model->GetStyle(DocumentModel::SelectionStyleName, style);
							ApplyColor(start, length, selectionStyle);
						}
						start += length;
					}

					void Visit(DocumentParagraphRun* run)override
					{
						VisitContainer(run);
					}

					static vint SetProperty(DocumentModel* model, GuiDocumentParagraphCache* paragraphCache, Ptr<pg::ParagraphCache> cache, Ptr<DocumentParagraphRun> run, vint selectionBegin, vint selectionEnd)
					{
						SetPropertiesVisitor visitor(model, paragraphCache, cache, selectionBegin, selectionEnd);
						run->Accept(&visitor);
						return visitor.length;
					}
				};
			}
			using namespace visitors;

/***********************************************************************
GuiDocumentParagraphCache
***********************************************************************/

			GuiDocumentParagraphCache::GuiDocumentParagraphCache(IGuiGraphicsParagraphCallback* _callback)
				: callback(_callback)
				, layoutProvider(GetGuiGraphicsResourceManager()->GetLayoutProvider())
				, defaultHeight(GetCurrentController()->ResourceService()->GetDefaultFont().size)
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
						cache->graphicsParagraph = nullptr;
						cache->outdatedStyles = true;
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
				CHECK_ERROR(cache && (!requireParagraph || (cache->graphicsParagraph && !cache->outdatedStyles)), ERROR_MESSAGE_PREFIX L"The specified paragraph is not created.");
				return cache;
#undef ERROR_MESSAGE_PREFIX
			}

			Size GuiDocumentParagraphCache::GetParagraphSize(vint paragraphIndex)
			{
				return paragraphSizes[paragraphIndex].cachedSize;
			}

			vint GuiDocumentParagraphCache::GetParagraphTopWithoutParagraphDistance(vint paragraphIndex)
			{
				if (paragraphIndex >= validCachedTops)
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
				return paragraphSizes[paragraphIndex].cachedTopWithoutParagraphDistance;
			}

			vint GuiDocumentParagraphCache::GetParagraphTop(vint paragraphIndex, vint paragraphDistance)
			{
				return GetParagraphTopWithoutParagraphDistance(paragraphIndex) + paragraphIndex * paragraphDistance;
			}

			vint GuiDocumentParagraphCache::ResetCache()
			{
				nameCallbackIdMap.Clear();
				freeCallbackIds.Clear();
				usedCallbackIds = 0;

				auto document = element ? element->GetDocument() : nullptr;
				if (document && document->paragraphs.Count() > 0)
				{
					paragraphCaches.Resize(0);
					paragraphCaches.Resize(document->paragraphs.Count());
					paragraphSizes.Resize(document->paragraphs.Count());

					for (vint i = 0; i < paragraphSizes.Count(); i++)
					{
						paragraphSizes[i] = { (i * defaultHeight),{0,defaultHeight}};
					}

					validCachedTops = document->paragraphs.Count();
					return document->paragraphs.Count() * defaultHeight;
				}
				else
				{
					paragraphCaches.Resize(0);
					paragraphSizes.Resize(0);
					validCachedTops = 0;
					return 0;
				}
			}

			vint GuiDocumentParagraphCache::ResetCache(vint index, vint oldCount, vint newCount, bool updatedText)
			{
				if (updatedText)
				{
					for (vint i = 0; i < oldCount; i++)
					{
						if (auto cache = paragraphCaches[i + index])
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

				if (oldCount == newCount)
				{
					for (vint i = 0; i < oldCount; i++)
					{
						if (updatedText)
						{
							paragraphCaches[index + i] = nullptr;
						}
						else if (auto cache = paragraphCaches[index + i])
						{
							cache->outdatedStyles = true;
						}
					}
					return 0;
				}
				else
				{
					pg::ParagraphCacheArray oldCaches;
					pg::ParagraphSizeArray oldSizes;

					CopyFrom(oldCaches, paragraphCaches);
					CopyFrom(oldSizes, paragraphSizes);

					vint paragraphCount = element->GetDocument()->paragraphs.Count();
					paragraphCaches.Resize(paragraphCount);
					paragraphSizes.Resize(paragraphCount);

					vint paragraphTop = GetParagraphTopWithoutParagraphDistance(index);
					for (vint i = 0; i < paragraphCount; i++)
					{
						if (i < index)
						{
							paragraphCaches[i] = oldCaches[i];
							paragraphSizes[i] = oldSizes[i];
						}
						else if (i < index + newCount)
						{
							// updateText must be true, ensured in GuiDocumentElementRenderer::NotifyParagraphUpdated
							paragraphCaches[i] = nullptr;
							paragraphSizes[i] = { (paragraphTop + (i - index) * defaultHeight),{0,defaultHeight} };
						}
						else
						{
							paragraphCaches[i] = oldCaches[i - (newCount - oldCount)];
							paragraphSizes[i] = oldSizes[i - (newCount - oldCount)];
						}
					}
					validCachedTops = index + newCount;

					vint oldUpdatedTotalHeight = 0;
					for (vint i = 0; i < oldCount; i++)
					{
						oldUpdatedTotalHeight += oldSizes[index + i].cachedSize.y;
					}
					return newCount * defaultHeight - oldUpdatedTotalHeight;
				}
			}

			vint GuiDocumentParagraphCache::EnsureParagraph(vint paragraphIndex, vint maxWidth)
			{
				auto paragraph = element->GetDocument()->paragraphs[paragraphIndex];
				auto cache = paragraphCaches[paragraphIndex];
				if (!cache)
				{
					cache = Ptr(new pg::ParagraphCache);
					cache->fullText = paragraph->GetTextForCaret();
					paragraphCaches[paragraphIndex] = cache;
				}

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
					cache->graphicsParagraph = layoutProvider->CreateParagraph(paragraphText, renderTarget, callback);
					cache->outdatedStyles = true;
				}

				if (cache->outdatedStyles)
				{
					cache->outdatedStyles = false;
					SetPropertiesVisitor::SetProperty(element->GetDocument().Obj(), this, cache, paragraph, cache->selectionBegin, cache->selectionEnd);
					cache->graphicsParagraph->SetParagraphAlignment(paragraph->alignment ? paragraph->alignment.Value() : Alignment::Left);
					cache->graphicsParagraph->SetWrapLine(element->GetWrapLine());
					cache->graphicsParagraph->SetMaxWidth(maxWidth);
				}

				auto& cachedSize = paragraphSizes[paragraphIndex];
				Size oldSize = cachedSize.cachedSize;
				Size newSize = cache->graphicsParagraph->GetSize();
				if(newSize.y < defaultHeight)
				{
					newSize.y = defaultHeight;
				}
				cachedSize.cachedSize = newSize;
				if (oldSize.y != newSize.y && validCachedTops > paragraphIndex + 1)
				{
					validCachedTops = paragraphIndex + 1;
				}
				return newSize.y - oldSize.y;
			}

			vint GuiDocumentParagraphCache::GetParagraphFromY(vint y, vint paragraphDistance)
			{
				auto document = element ? element->GetDocument() : nullptr;
				if (!document || document->paragraphs.Count() == 0) return -1;

				vint start = 0;
				vint end = paragraphSizes.Count() - 1;

				if (0 < validCachedTops && validCachedTops <= paragraphSizes.Count())
				{
					vint index = validCachedTops - 1;
					vint top = GetParagraphTop(index, paragraphDistance);
					auto size = paragraphSizes[index].cachedSize;
					if (y < top)
					{
						if (index < 1) return 0;
						end = index - 1;
					}
					else if (y < top + size.y + paragraphDistance)
					{
						return index;
					}
					else
					{
						if (index >= paragraphSizes.Count() - 1) return paragraphSizes.Count() - 1;
						start = validCachedTops;
					}
				}

				if (start >= end) return end;
				while (true)
				{
					vint mid = (start + end) / 2;
					vint top = GetParagraphTop(mid, paragraphDistance);
					auto size = paragraphSizes[mid].cachedSize;
					if (y < top)
					{
						end = mid - 1;
						if (start >= end) return start;
					}
					else if (y < top + size.y + paragraphDistance)
					{
						return mid;
					}
					else
					{
						start = mid + 1;
						if (start >= end) return end;
					}
				}
			}

			void GuiDocumentParagraphCache::ReleaseParagraphs(vint index, vint count)
			{
				for (vint i = 0; i < count; i++)
				{
					vint paragraphIndex = index + i;
					if (paragraphIndex >= 0 && paragraphIndex < paragraphCaches.Count())
					{
						auto cache = paragraphCaches[paragraphIndex];
						if (cache) // Check if cache itself is null per UPDATE guidance
						{
							cache->graphicsParagraph = nullptr; // Release only the rendering object
							// Preserve all other data: fullText, embeddedObjects, selectionBegin/selectionEnd
						}
					}
				}
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
				pgCache.Initialize(element);
				NotifyParagraphPaddingUpdated(element->GetParagraphPadding());
			}

			void GuiDocumentElementRenderer::FinalizeInternal()
			{
			}

			void GuiDocumentElementRenderer::RenderTargetChangedInternal(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget)
			{
				pgCache.RenderTargetChanged(oldRenderTarget, newRenderTarget);
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

			GuiDocumentElementRenderer::GuiDocumentElementRenderer()
				: pgCache(this)
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
						for(auto eo: cache->embeddedObjects.Values())
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

				for(auto p:paragraphsToReset)
				{
					NotifyParagraphUpdated(p, 1, 1, false);
				}
			}

			void GuiDocumentElementRenderer::NotifyParagraphPaddingUpdated(bool value)
			{
				vint defaultHeight = GetCurrentController()->ResourceService()->GetDefaultFont().size;
				paragraphDistance = element->GetParagraphPadding() ? defaultHeight : 0;
			}

			void GuiDocumentElementRenderer::OnElementStateChanged()
			{
				lastTotalWidth = 0;
				lastTotalHeightWithoutParagraphDistance = pgCache.ResetCache();
				FixMinSize();
			}

			void GuiDocumentElementRenderer::NotifyParagraphUpdated(vint index, vint oldCount, vint newCount, bool updatedText)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::GuiDocumentElementRenderer::NotifyParagraphUpdated(vint, vint, vint, bool)#"
				vint oldParagraphCount = pgCache.GetParagraphCount();
				vint newParagraphCount = element->GetDocument()->paragraphs.Count();

				CHECK_ERROR(oldCount >= 0, ERROR_MESSAGE_PREFIX L"oldCount cannot be negative.");
				CHECK_ERROR(newCount >= 0, ERROR_MESSAGE_PREFIX L"newCount cannot be negative.");
				CHECK_ERROR(0 <= index && index + oldCount <= oldParagraphCount, ERROR_MESSAGE_PREFIX L"index + oldCount is out of range.");
				CHECK_ERROR(0 <= index && index + newCount <= newParagraphCount, ERROR_MESSAGE_PREFIX L"index + newCount is out of range.");
				CHECK_ERROR(updatedText || oldCount == newCount, ERROR_MESSAGE_PREFIX L"updatedText must be true if oldCount is not equal to newCount.");
				CHECK_ERROR(newParagraphCount - oldParagraphCount == newCount - oldCount, ERROR_MESSAGE_PREFIX L"newCount - oldCount does not reflect the actual paragraph count changing.");

				for (vint i = 0; i < oldCount; i++)
				{
					vint width = pgCache.GetParagraphSize(index + i).x;
					if (lastTotalWidth == width)
					{
						lastTotalWidth = 0;
						break;
					}
				}
				lastTotalHeightWithoutParagraphDistance += pgCache.ResetCache(index, oldCount, newCount, updatedText);
				FixMinSize();

				// Update render range if text was actually updated
				if (updatedText)
				{
					UpdateRenderRange(index, oldCount, newCount);
				}

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
								cache->selectionBegin = newBegin;
								cache->selectionEnd = newEnd;
								if (cache->graphicsParagraph)
								{
									NotifyParagraphUpdated(i, 1, 1, false);
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
								cache->selectionBegin = -1;
								cache->selectionEnd = -1;
								if (cache->graphicsParagraph)
								{
									NotifyParagraphUpdated(i, 1, 1, false);
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