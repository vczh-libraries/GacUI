#define GAC_HEADER_USE_NAMESPACE
#include "../../../Source/GacUI.h"
#define _WINSOCKAPI_
#include <Windows.h>
#include <Shellapi.h>

#pragma comment(lib, "Shell32.lib")

using namespace vl;
using namespace vl::presentation;

extern int StartNamedPipeClient(vint automationHttpPort);
extern int StartHttpClient(vint automationHttpPort);
extern int StartMiniHttpClient(vint automationHttpPort);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	int result = -1;
	int argc = 0;
	auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (!argv)
	{
		return result;
	}

	vint transport = -1; // 0 = Pipe, 1 = Http, 2 = MiniHTTP
	vint automationHttpPort = 8889;
	bool portSpecified = false;
	for (int i = 1; i < argc; i++)
	{
		vint currentTransport = -1;
		if (wcscmp(argv[i], L"/Pipe") == 0)
		{
			currentTransport = 0;
		}
		else if (wcscmp(argv[i], L"/Http") == 0)
		{
			currentTransport = 1;
		}
		else if (wcscmp(argv[i], L"/MiniHttp") == 0)
		{
			currentTransport = 2;
		}
		else if (wcsncmp(argv[i], L"/port:", 6) == 0)
		{
			wchar_t* end = nullptr;
			auto port = wcstol(argv[i] + 6, &end, 10);
			if (portSpecified || end == argv[i] + 6 || *end || port < 1 || port > 65535)
			{
				LocalFree(argv);
				return result;
			}
			portSpecified = true;
			automationHttpPort = port;
			continue;
		}
		else
		{
			LocalFree(argv);
			return result;
		}

		if (transport != -1)
		{
			LocalFree(argv);
			return result;
		}
		transport = currentTransport;
	}
	LocalFree(argv);

	if (transport == 0)
	{
		result = StartNamedPipeClient(automationHttpPort);
	}
	else if (transport == 1)
	{
		result = StartHttpClient(automationHttpPort);
	}
	else if (transport == 2)
	{
		result = StartMiniHttpClient(automationHttpPort);
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
