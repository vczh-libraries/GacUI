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

			void GuiDocumentImageCache::ResetTextCache(vint index, vint oldCount, vint newCount)
			{
				// TODO: Delete affected caches
				// array[index] -> (image, frameIndex)[]
			}

			Ptr<IGuiGraphicsElement> GuiDocumentImageCache::GetImageElement(Ptr<INativeImage> image, vint frameIndex, vint paragraphIndex, vint start)
			{
				// TODO: Cache create element with position
				// map[(image, frameIndex)] -> (&array[index])[]
				// map[(image, frameIndex)] -> element
				auto element = Ptr(GuiImageFrameElement::Create());
				element->SetImage(image, frameIndex);
				element->SetStretch(true);
				return element;
			}
		}
	}
}