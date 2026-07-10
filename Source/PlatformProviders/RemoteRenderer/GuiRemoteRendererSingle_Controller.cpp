#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace collections;
	using namespace remoteprotocol;

	void GuiRemoteRendererSingle::RequestControllerGetFontConfig(vint id)
	{
		if (!CanSendEvents()) return;
		FontConfig response;
		auto rs = GetCurrentController()->ResourceService();
		response.defaultFont = rs->GetDefaultFont();
		response.supportedFonts = Ptr(new List<WString>);
		rs->EnumerateFonts(*response.supportedFonts.Obj());
		events->RespondControllerGetFontConfig(id, response);
	}

	void GuiRemoteRendererSingle::RequestControllerGetScreenConfig(vint id)
	{
		if (!CanSendEvents()) return;
		auto primary = screen ? screen : GetCurrentController()->ScreenService()->GetScreen((vint)0);
		events->RespondControllerGetScreenConfig(id, GetScreenConfig(primary));
	}

	void GuiRemoteRendererSingle::RequestControllerConnectionEstablished()
	{
	}

	void GuiRemoteRendererSingle::RequestControllerConnectionStopped()
	{
		if (window)
		{
			DisconnectFromCore();
			window->Hide(true);
		}
	}
}
