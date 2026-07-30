#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GAC_HEADER_USE_NAMESPACE
#include "../../../Source/GacUI.h"
#define _WINSOCKAPI_
#include <Windows.h>
#include <crtdbg.h>

using namespace vl;
using namespace vl::console;

extern int StartNamedPipeServer();
extern int StartHttpServer();
extern int StartMiniHttpServer();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
	int result = 1;
	if (strcmp(lpCmdLine, "/Pipe") == 0)
	{
		result = StartNamedPipeServer();
	}
	else if (strcmp(lpCmdLine, "/Http") == 0)
	{
		result = StartHttpServer();
	}
	else if (strcmp(lpCmdLine, "/MiniHttp") == 0)
	{
		result = StartMiniHttpServer();
	}
	else
	{
		Console::WriteLine(L"Error: exactly one of /Pipe, /Http or /MiniHttp must be provided.");
	}

#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
