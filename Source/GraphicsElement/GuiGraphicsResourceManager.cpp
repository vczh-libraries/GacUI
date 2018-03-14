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

			bool GuiGraphicsResourceManager::RegisterElementFactory(IGuiGraphicsElementFactory* factory)
			{
				if(elementFactories.Keys().Contains(factory->GetElementTypeName()))
				{
					return false;
				}
				else
				{
					elementFactories.Add(factory->GetElementTypeName(), factory);
					return true;
				}
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

			IGuiGraphicsElementFactory* GuiGraphicsResourceManager::GetElementFactory(const WString& elementTypeName)
			{
				vint index=elementFactories.Keys().IndexOf(elementTypeName);
				return index==-1?0:elementFactories.Values().Get(index).Obj();
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

			bool RegisterFactories(IGuiGraphicsElementFactory* elementFactory, IGuiGraphicsRendererFactory* rendererFactory)
			{
				if(guiGraphicsResourceManager && elementFactory && rendererFactory)
				{
					if(guiGraphicsResourceManager->RegisterElementFactory(elementFactory))
					{
						if(guiGraphicsResourceManager->RegisterRendererFactory(elementFactory->GetElementTypeName(), rendererFactory))
						{
							return true;
						}
					}
				}
				return false;
			}
		}
	}
}