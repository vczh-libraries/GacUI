#define GAC_HEADER_USE_NAMESPACE
#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include <Windows.h>

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remote_renderer;

GuiRemoteRendererSingle* remoteRenderer = nullptr;

void InstallRemoteRenderer(GuiRemoteRendererSingle* _remoteRenderer)
{
	remoteRenderer = _remoteRenderer;
}

void GuiMain()
{
}

namespace vl::presentation
{
	void GuiInitializeUtilities()
	{
	}

	void GuiFinalizeUtilities()
	{
	}
}

extern int StartNamedPipeClient();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	int result = StartNamedPipeClient();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}