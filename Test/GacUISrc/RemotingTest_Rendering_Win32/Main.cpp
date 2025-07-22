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
extern int StartHttpClient();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	int result = -1;
	if (strcmp(lpCmdLine, "/Pipe") == 0)
	{
		result = StartNamedPipeClient();
	}
	else if (strcmp(lpCmdLine, "/Http") == 0)
	{
		result = StartHttpClient();
	}
	else
	{
		CHECK_FAIL(L"WinMain(...)#The command line argument must be either /Pipe or /Http.");
	}
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}