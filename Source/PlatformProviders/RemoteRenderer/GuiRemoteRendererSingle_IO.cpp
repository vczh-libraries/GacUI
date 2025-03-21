#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	void GuiRemoteRendererSingle::RequestIOUpdateGlobalShortcutKey(const Ptr<collections::List<remoteprotocol::GlobalShortcutKey>>& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestIORequireCapture()
	{
	}

	void GuiRemoteRendererSingle::RequestIOReleaseCapture()
	{
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