#define GAC_HEADER_USE_NAMESPACE
#include "../../../Source/GacUI.h"
#include <Windows.h>

using namespace vl;
using namespace vl::presentation;

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