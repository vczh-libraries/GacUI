#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GAC_HEADER_USE_NAMESPACE
#include "../../../Source/GacUI.h"
#define _WINSOCKAPI_
#include <Windows.h>
#include <Shellapi.h>
#include <crtdbg.h>

#pragma comment(lib, "Shell32.lib")

extern int StartNamedPipeServer();
extern int StartHttpServer();
extern int StartMiniHttpServer();
extern int StartCliServer(const vl::WString& hostPath);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
	int result = 1;
	int argc = 0;
	auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (!argv)
	{
		return result;
	}

	if (argc == 2 && wcscmp(argv[1], L"/Pipe") == 0)
	{
		result = StartNamedPipeServer();
	}
	else if (argc == 2 && wcscmp(argv[1], L"/Http") == 0)
	{
		result = StartHttpServer();
	}
	else if (argc == 2 && wcscmp(argv[1], L"/MiniHttp") == 0)
	{
		result = StartMiniHttpServer();
	}
	else if (argc == 2 && wcsncmp(argv[1], L"/Cli:", 5) == 0 && argv[1][5])
	{
		result = StartCliServer(vl::WString::CopyFrom(argv[1] + 5, wcslen(argv[1] + 5)));
	}
	LocalFree(argv);
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
