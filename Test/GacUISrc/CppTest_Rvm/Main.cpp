#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GAC_HEADER_USE_NAMESPACE
#include "../../../Source/GacUI.h"
#define _WINSOCKAPI_
#include <Windows.h>
#include <Shellapi.h>
#include <crtdbg.h>
#include "../../AutomationArguments.h"

#pragma comment(lib, "Shell32.lib")

extern int StartNamedPipeServer();
extern int StartHttpServer();
extern int StartMiniHttpServer();
extern int StartCliServer(const vl::WString& hostPath);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
	_set_abort_behavior(0, _WRITE_ABORT_MSG);
	int result = 1;
	int argc = 0;
	auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (!argv)
	{
		return result;
	}
	// Keep the existing transport grammar after consuming its independent automation option.
	int remaining = 1;
	for (int i = 1; i < argc; i++)
	{
		auto consumed = gacui_test::automationArguments.Consume(vl::WString(argv[i]));
		if (consumed < 0) { LocalFree(argv); return result; }
		if (!consumed) argv[remaining++] = argv[i];
	}
	argc = remaining;

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
		auto cliArgument = argv[1] + 5;
		auto cliLength = (vl::vint)wcslen(cliArgument);
		if (cliArgument[0] == L'"' || cliArgument[cliLength - 1] == L'"')
		{
			if (cliLength < 2 || cliArgument[0] != L'"' || cliArgument[cliLength - 1] != L'"')
			{
				LocalFree(argv);
				return result;
			}
			cliArgument++;
			cliLength -= 2;
		}
		if (cliLength > 0)
		{
			result = StartCliServer(vl::WString::CopyFrom(cliArgument, cliLength));
		}
	}
	LocalFree(argv);
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
