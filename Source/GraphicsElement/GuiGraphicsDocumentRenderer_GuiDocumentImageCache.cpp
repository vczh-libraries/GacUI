#include "GuiGraphicsDocumentRenderer.h"

namespace vl
{
	using namespace collections;

	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
GuiDocumentImageCache
***********************************************************************/

			GuiDocumentImageCache::GuiDocumentImageCache()
			{
			}

			GuiDocumentImageCache::~GuiDocumentImageCache()
			{
			}

			void GuiDocumentImageCache::Initialize(GuiDocumentElement* _element)
			{
				element = _element;
			}

			void GuiDocumentImageCache::RenderTargetChanged(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget)
			{
			}

			void GuiDocumentImageCache::ResetCache()
			{
				caches.Resize(0);
				if (auto document = element->GetDocument())
				{
					caches.Resize(document->paragraphs.Count());
				}
			}

			void GuiDocumentImageCache::ResetTextCache(vint index, vint oldCount, vint newCount)
			{
				pg::ParagraphImageCacheArray oldCaches;
				CopyFrom(oldCaches, caches);

				vint paragraphCount = element->GetDocument()->paragraphs.Count();
				caches.Resize(paragraphCount);

				for (vint i = 0; i < paragraphCount; i++)
				{
					if (i < index)
					{
						caches[i] = oldCaches[i];
					}
					else if (i < index + newCount)
					{
						caches[i] = {};
					}
					else
					{
						caches[i] = oldCaches[i - (newCount - oldCount)];
					}
				}
			}

			Ptr<IGuiGraphicsElement> GuiDocumentImageCache::GetImageElement(Ptr<INativeImage> image, vint frameIndex, vint paragraphIndex, vint start)
			{
				auto cache = caches[paragraphIndex];
				if (!cache)
				{
					cache = Ptr(new pg::ParagraphImageCache);
					caches[paragraphIndex] = cache;
				}

				auto key = Tuple(image.Obj(), frameIndex, start);
				vint index = cache->elements.Keys().IndexOf(key);
				if (index == -1)
				{
					auto element = Ptr(GuiImageFrameElement::Create());
					element->SetImage(image, frameIndex);
					element->SetStretch(true);
					cache->elements.Add(key, element);
					return element;
				}
				else
				{
					return cache->elements.Values()[index];
				}
			}
		}
	}
}