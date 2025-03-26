#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	void GuiRemoteRendererSingle::Opened()
	{
		events->OnControllerConnect();
	}

	GuiRemoteRendererSingle::GuiRemoteRendererSingle()
	{
	}

	GuiRemoteRendererSingle::~GuiRemoteRendererSingle()
	{
	}

	void GuiRemoteRendererSingle::RegisterMainWindow(INativeWindow* _window)
	{
		window = _window;
		window->InstallListener(this);
	}

	void GuiRemoteRendererSingle::UnregisterMainWindow()
	{
		window->UninstallListener(this);
		window = nullptr;
	}

	WString GuiRemoteRendererSingle::GetExecutablePath()
	{
		CHECK_FAIL(L"This function should not be called!");
	}

	void GuiRemoteRendererSingle::Initialize(IGuiRemoteProtocolEvents* _events)
	{
		events = _events;
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