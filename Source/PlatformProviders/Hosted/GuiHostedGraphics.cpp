#include "GuiHostedController.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
GuiHostedGraphicsResourceManager
***********************************************************************/
			
			GuiHostedGraphicsResourceManager::GuiHostedGraphicsResourceManager(GuiHostedController* _hostedController, IGuiGraphicsResourceManager* _nativeManager)
				: hostedController(_hostedController)
				, nativeManager(_nativeManager)
			{
				CHECK_ERROR(
					!hostedController->hostedResourceManager,
					L"vl::presentation::elements::GuiHostedGraphicsResourceManager::GuiHostedGraphicsResourceManager(GuiHostedController*, IGuiGraphicsResourceManager*)#"
					L"GuiHostedGraphicsResourceManager has been created for the same GuiHostedController");
				hostedController->hostedResourceManager = this;
			}

			GuiHostedGraphicsResourceManager::~GuiHostedGraphicsResourceManager()
			{
				hostedController->hostedResourceManager = nullptr;
			}

			vint GuiHostedGraphicsResourceManager::RegisterElementType(const WString& elementTypeName)
			{
				return nativeManager->RegisterElementType(elementTypeName);
			}

			void GuiHostedGraphicsResourceManager::RegisterRendererFactory(vint elementType, Ptr<IGuiGraphicsRendererFactory> factory)
			{
				nativeManager->RegisterRendererFactory(elementType, factory);
			}

			IGuiGraphicsRendererFactory* GuiHostedGraphicsResourceManager::GetRendererFactory(vint elementType)
			{
				return nativeManager->GetRendererFactory(elementType);
			}

			IGuiGraphicsRenderTarget* GuiHostedGraphicsResourceManager::GetRenderTarget(INativeWindow* window)
			{
				return nativeManager->GetRenderTarget(hostedController->nativeWindow);
			}

			void GuiHostedGraphicsResourceManager::RecreateRenderTarget(INativeWindow* window)
			{
			}

			void GuiHostedGraphicsResourceManager::ResizeRenderTarget(INativeWindow* window)
			{
			}

			IGuiGraphicsLayoutProvider* GuiHostedGraphicsResourceManager::GetLayoutProvider()
			{
				return nativeManager->GetLayoutProvider();
			}
		}
	}
}