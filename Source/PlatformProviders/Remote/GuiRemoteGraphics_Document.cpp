#include "GuiRemoteGraphics_Document.h"
#include "GuiRemoteGraphics.h"

namespace vl::presentation::elements
{

/***********************************************************************
GuiRemoteGraphicsParagraph
***********************************************************************/

	GuiRemoteGraphicsParagraph::GuiRemoteGraphicsParagraph(const WString& _text, GuiRemoteController* _remote, GuiRemoteGraphicsResourceManager* _resourceManager, GuiRemoteGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _callback)
		: text(_text)
		, remote(_remote)
		, resourceManager(_resourceManager)
		, renderTarget(_renderTarget)
		, callback(_callback)
	{
	}

	GuiRemoteGraphicsParagraph::~GuiRemoteGraphicsParagraph()
	{
	}
}