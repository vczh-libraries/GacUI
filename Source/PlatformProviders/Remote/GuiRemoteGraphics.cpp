#include "GuiRemoteGraphics.h"

namespace vl::presentation::elements
{

/***********************************************************************
GuiRemoteGraphicsResourceManager
***********************************************************************/

	GuiRemoteGraphicsResourceManager::GuiRemoteGraphicsResourceManager(GuiRemoteController* _remote)
		: remote(_remote)
	{
		// TODO: register element renderers;
	}

	GuiRemoteGraphicsResourceManager::~GuiRemoteGraphicsResourceManager()
	{
	}

	IGuiGraphicsRenderTarget* GuiRemoteGraphicsResourceManager::GetRenderTarget(INativeWindow* window)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteGraphicsResourceManager::RecreateRenderTarget(INativeWindow* window)
	{
	}

	void GuiRemoteGraphicsResourceManager::ResizeRenderTarget(INativeWindow* window)
	{
	}

	IGuiGraphicsLayoutProvider* GuiRemoteGraphicsResourceManager::GetLayoutProvider()
	{
		CHECK_FAIL(L"Not Implemented!");
	}
}