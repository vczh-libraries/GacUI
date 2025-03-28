#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace collections;
	using namespace remoteprotocol;

	void GuiRemoteRendererSingle::RequestControllerGetFontConfig(vint id)
	{
		FontConfig response;
		auto rs = GetCurrentController()->ResourceService();
		response.defaultFont = rs->GetDefaultFont();
		response.supportedFonts = Ptr(new List<WString>);
		rs->EnumerateFonts(*response.supportedFonts.Obj());
		events->RespondControllerGetFontConfig(id, response);
	}

	void GuiRemoteRendererSingle::RequestControllerGetScreenConfig(vint id)
	{
		ScreenConfig response;
		auto primary = GetCurrentController()->ScreenService()->GetScreen((vint)0);
		response.bounds = primary->GetBounds();
		response.clientBounds = primary->GetClientBounds();
		response.scalingX = primary->GetScalingX();
		response.scalingY = primary->GetScalingY();
		events->RespondControllerGetScreenConfig(id, response);
	}

	void GuiRemoteRendererSingle::RequestControllerConnectionEstablished()
	{
	}

	void GuiRemoteRendererSingle::RequestControllerConnectionStopped()
	{
		if (window)
		{
			window->ReleaseCapture();
		}
	}
}
