#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	GuiRemoteRendererSingle::GuiRemoteRendererSingle()
	{
	}

	GuiRemoteRendererSingle::~GuiRemoteRendererSingle()
	{
	}

	WString GuiRemoteRendererSingle::GetExecutablePath()
	{
		CHECK_FAIL(L"This function should not be called!");
	}

	void GuiRemoteRendererSingle::Initialize(IGuiRemoteProtocolEvents* events)
	{
	}

	void GuiRemoteRendererSingle::Submit(bool& disconnected)
	{
		CHECK_FAIL(L"This function should not be called!");
	}

	void GuiRemoteRendererSingle::ProcessRemoteEvents()
	{
		CHECK_FAIL(L"This function should not be called!");
	}
}