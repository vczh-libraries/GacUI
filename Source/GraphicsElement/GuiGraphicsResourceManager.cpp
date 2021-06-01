#include "GuiGraphicsResourceManager.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{
			using namespace collections;

/***********************************************************************
GuiGraphicsResourceManager
***********************************************************************/

			GuiGraphicsResourceManager::GuiGraphicsResourceManager()
			{
			}

			GuiGraphicsResourceManager::~GuiGraphicsResourceManager()
			{
			}

			bool GuiGraphicsResourceManager::RegisterRendererFactory(const WString& elementTypeName, IGuiGraphicsRendererFactory* factory)
			{
				if(rendererFactories.Keys().Contains(elementTypeName))
				{
					return false;
				}
				else
				{
					rendererFactories.Add(elementTypeName, factory);
					return true;
				}
			}

			IGuiGraphicsRendererFactory* GuiGraphicsResourceManager::GetRendererFactory(const WString& elementTypeName)
			{
				vint index=rendererFactories.Keys().IndexOf(elementTypeName);
				return index==-1?nullptr:rendererFactories.Values().Get(index).Obj();
			}

			GuiGraphicsResourceManager* guiGraphicsResourceManager=0;

			GuiGraphicsResourceManager* GetGuiGraphicsResourceManager()
			{
				return guiGraphicsResourceManager;
			}

			void SetGuiGraphicsResourceManager(GuiGraphicsResourceManager* resourceManager)
			{
				guiGraphicsResourceManager=resourceManager;
			}
		}
	}
}