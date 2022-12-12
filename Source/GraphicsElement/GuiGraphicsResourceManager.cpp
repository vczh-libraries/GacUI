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

			vint GuiGraphicsResourceManager::RegisterElementType(const WString& elementTypeName)
			{
				CHECK_ERROR(!elementTypes.Contains(elementTypeName), L"GuiGraphicsResourceManager::RegisterElementType(const WString&)#This element type has already been registered.");
				return elementTypes.Add(elementTypeName);
			}

			void GuiGraphicsResourceManager::RegisterRendererFactory(vint elementType, Ptr<IGuiGraphicsRendererFactory> factory)
			{
				if (rendererFactories.Count() <= elementType)
				{
					rendererFactories.Resize(elementType + 1);
					rendererFactories[elementType] = factory;
				}
				else
				{
					CHECK_ERROR(!rendererFactories[elementType], L"GuiGraphicsResourceManager::RegisterRendererFactory(vint, Ptr<IGuiGraphicsRendererFactory>)#This element type has already been binded a renderer factory.");
					rendererFactories[elementType] = factory;
				}
			}

			IGuiGraphicsRendererFactory* GuiGraphicsResourceManager::GetRendererFactory(vint elementType)
			{
				return rendererFactories.Count() > elementType ? rendererFactories[elementType].Obj() : nullptr;
			}

			IGuiGraphicsResourceManager* guiGraphicsResourceManager = nullptr;

			IGuiGraphicsResourceManager* GetGuiGraphicsResourceManager()
			{
				return guiGraphicsResourceManager;
			}

			void SetGuiGraphicsResourceManager(IGuiGraphicsResourceManager* resourceManager)
			{
				guiGraphicsResourceManager=resourceManager;
			}
		}
	}
}