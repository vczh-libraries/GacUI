#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	void GuiRemoteRendererSingle::RequestIOUpdateGlobalShortcutKey(const Ptr<collections::List<remoteprotocol::GlobalShortcutKey>>& arguments)
	{
		CHECK_ERROR(arguments->Count() == 0, L"Not Implemented");
	}

	void GuiRemoteRendererSingle::RequestIORequireCapture()
	{
		window->RequireCapture();
	}

	void GuiRemoteRendererSingle::RequestIOReleaseCapture()
	{
		window->ReleaseCapture();
	}

	void GuiRemoteRendererSingle::RequestIOIsKeyPressing(vint id, const VKEY& arguments)
	{
		CHECK_FAIL(L"Not Implemented");
	}

	void GuiRemoteRendererSingle::RequestIOIsKeyToggled(vint id, const VKEY& arguments)
	{
		CHECK_FAIL(L"Not Implemented");
	}
}