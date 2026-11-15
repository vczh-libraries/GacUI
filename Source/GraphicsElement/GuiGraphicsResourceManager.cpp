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
				elementTypes.Add(elementTypeName);
				// GuiElementBase caches type IDs across resource-manager lifetimes.
				static vint nextElementType = 0;
				return nextElementType++;
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

			void GuiGraphicsResourceManager::RegisterRawInterpretorFactory(const WString& name, Ptr<IGuiGraphicsRawInterpretorFactory> factory)
			{
#define ERROR_MESSAGE_PREFIX L"GuiGraphicsResourceManager::RegisterRawInterpretorFactory(const WString&, Ptr<IGuiGraphicsRawInterpretorFactory>)#"
				CHECK_ERROR(!rawInterpretorFactories.Keys().Contains(name), ERROR_MESSAGE_PREFIX L"This interpretor name has already been registered.");
				rawInterpretorFactories.Add(name, factory);
#undef ERROR_MESSAGE_PREFIX
			}

			IGuiGraphicsRawInterpretorFactory* GuiGraphicsResourceManager::GetRawInterpretorFactory(const WString& name)
			{
				auto index = rawInterpretorFactories.Keys().IndexOf(name);
				return index == -1 ? nullptr : rawInterpretorFactories.Values()[index].Obj();
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