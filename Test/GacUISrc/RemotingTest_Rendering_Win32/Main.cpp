#define GAC_HEADER_USE_NAMESPACE
#include "../../../Source/GacUI.h"
#define _WINSOCKAPI_
#include <Windows.h>

using namespace vl;
using namespace vl::presentation;

extern int StartNamedPipeClient();
extern int StartHttpClient();
extern int StartMiniHttpClient();

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
	else if (strcmp(lpCmdLine, "/MiniHTTP") == 0)
	{
		result = StartMiniHttpClient();
	}
	else
	{
		return result;
	}
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
