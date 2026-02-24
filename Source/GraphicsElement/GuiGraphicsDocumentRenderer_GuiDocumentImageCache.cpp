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
			}

			Ptr<IGuiGraphicsElement> GuiDocumentImageCache::GetImageElement(Ptr<INativeImage> image, vint frameIndex)
			{
				auto element = Ptr(GuiImageFrameElement::Create());
				element->SetImage(image, frameIndex);
				element->SetStretch(true);
				return element;
			}
		}
	}
}