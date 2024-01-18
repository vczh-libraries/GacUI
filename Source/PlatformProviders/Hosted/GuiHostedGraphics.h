/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Hosted Window

Interfaces:
  IGuiGraphicsResourceManager

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIHOSTEDCONTROLLER_GUIHOSTEDGRAPHICS
#define VCZH_PRESENTATION_GUIHOSTEDCONTROLLER_GUIHOSTEDGRAPHICS

#include "../../GraphicsElement/GuiGraphicsResourceManager.h"

namespace vl
{
	namespace presentation
	{
		class GuiHostedController;

		namespace elements
		{

/***********************************************************************
GuiHostedGraphicsResourceManager
***********************************************************************/

			class GuiHostedGraphicsResourceManager : public Object, public virtual IGuiGraphicsResourceManager
			{
				friend class vl::presentation::GuiHostedController;
			protected:
				GuiHostedController*				hostedController = nullptr;
				IGuiGraphicsResourceManager*		nativeManager = nullptr;

			public:
				GuiHostedGraphicsResourceManager(GuiHostedController* _hostedController, IGuiGraphicsResourceManager* _nativeManager);
				~GuiHostedGraphicsResourceManager();

				vint								RegisterElementType(const WString& elementTypeName) override;
				void								RegisterRendererFactory(vint elementType, Ptr<IGuiGraphicsRendererFactory> factory) override;
				IGuiGraphicsRendererFactory*		GetRendererFactory(vint elementType) override;
				IGuiGraphicsRenderTarget*			GetRenderTarget(INativeWindow* window) override;
				void								RecreateRenderTarget(INativeWindow* window) override;
				void								ResizeRenderTarget(INativeWindow* window) override;
				IGuiGraphicsLayoutProvider*			GetLayoutProvider() override;
			};
		}
	}
}

#endif