#include "GuiGraphicsDocumentRenderer.h"

namespace vl
{
	using namespace collections;

	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
GuiDocumentParagraphCache
***********************************************************************/

			GuiDocumentParagraphCache::GuiDocumentParagraphCache(IGuiGraphicsParagraphCallback* _callback)
				: callback(_callback)
				, layoutProvider(GetGuiGraphicsResourceManager()->GetLayoutProvider())
			{
			}

			GuiDocumentParagraphCache::~GuiDocumentParagraphCache()
			{
			}

			vint GuiDocumentParagraphCache::GetDefaultHeight()
			{
				vint defaultHeight = GetCurrentController()->ResourceService()->GetDefaultFont().size;
				if (defaultHeight < 8) defaultHeight = 8;
				return defaultHeight;
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
						cache->invalidation = true;
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
				if (requireParagraph)
				{
					CHECK_ERROR(cache && cache->graphicsParagraph && cache->invalidation.TryGet<bool>() && cache->invalidation.Get<bool>() == false, ERROR_MESSAGE_PREFIX L"The specified paragraph is not created.");
				}
				else
				{
					return cache;
				}
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

					vint defaultHeight = GetDefaultHeight();
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

			vint GuiDocumentParagraphCache::ResetTextCache(vint index, vint oldCount, vint newCount)
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

				if (oldCount == newCount)
				{
					for (vint i = 0; i < oldCount; i++)
					{
						paragraphCaches[index + i] = nullptr;
					}
					return 0;
				}
				else
				{
					vint defaultHeight = GetDefaultHeight();
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

			vint GuiDocumentParagraphCache::ResetStyleCache(TextPos begin, TextPos end)
			{
				for (vint i = begin.row; i <= end.row; i++)
				{
					if (auto cache = paragraphCaches[i])
					{
						cache->invalidation = Pair(
							(i == begin.row ? begin.column : 0),
							(i == end.row ? end.column : cache->fullText.Length())
						);
					}
				}
				return 0;
			}

			vint GuiDocumentParagraphCache::ResetStyleCache(vint index, vint count)
			{
				for (vint i = 0; i < count; i++)
				{
					if (auto cache = paragraphCaches[index + i])
					{
						cache->invalidation = true;
					}
				}
				return 0;
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
					cache->invalidation = true;
				}

				cache->invalidation.Apply(Overloading(
					[&, this](bool value)
					{
						if (value)
						{
							visitors::SetProperties(element->GetDocument().Obj(), this, cache, paragraph, cache->selectionBegin, cache->selectionEnd, 0, cache->fullText.Length());
							cache->graphicsParagraph->SetParagraphAlignment(paragraph->alignment ? paragraph->alignment.Value() : Alignment::Left);
							cache->graphicsParagraph->SetWrapLine(element->GetWrapLine());
							cache->graphicsParagraph->SetMaxWidth(maxWidth);
						}
					},
					[&, this](collections::Pair<vint, vint> range)
					{
						visitors::SetProperties(element->GetDocument().Obj(), this, cache, paragraph, cache->selectionBegin, cache->selectionEnd, range.key, range.value);
						cache->graphicsParagraph->SetParagraphAlignment(paragraph->alignment ? paragraph->alignment.Value() : Alignment::Left);
						cache->graphicsParagraph->SetWrapLine(element->GetWrapLine());
						cache->graphicsParagraph->SetMaxWidth(maxWidth);
					}
				));
				cache->invalidation = false;

				auto& cachedSize = paragraphSizes[paragraphIndex];
				Size oldSize = cachedSize.cachedSize;
				Size newSize = cache->graphicsParagraph->GetSize();

				vint defaultHeight = GetDefaultHeight();
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
		}
	}
}