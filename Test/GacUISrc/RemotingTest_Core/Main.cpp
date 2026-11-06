#include <Vlpp.h>
#include <VlppOS.h>
#include "../SharedArguments.h"
#include <clocale>
#ifdef VCZH_MSVC
#include <crtdbg.h>
#endif

using namespace vl;
using namespace vl::console;

#ifdef VCZH_MSVC
extern int StartNamedPipeServer(vint index, const WString& cliCommand);
extern int StartHttpServer(vint index, const WString& cliCommand);
#define ARGUMENT_COMPARE wcscmp
#define ARGUMENT_NCOMPARE wcsncmp
#define ARGUMENT_TEXT(TEXT) L##TEXT
#else
#define ARGUMENT_COMPARE strcmp
#define ARGUMENT_NCOMPARE strncmp
#define ARGUMENT_TEXT(TEXT) TEXT
#endif
extern int StartMiniHttpServer(vint index, const WString& cliCommand);

#ifdef VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef VCZH_MSVC
	_set_abort_behavior(0, _WRITE_ABORT_MSG);
#else
	CHECK_ERROR(std::setlocale(LC_CTYPE, "") != nullptr, L"main()#Failed to initialize the character locale.");
#endif
	int result = 1;
	vint index = -1; // 0 = FullControlTest (/FCT), 1 = RemoteProtocolTest (/RPT), 2 = RemoteViewModelTest (/RVMT)
	int transport = -1; // 0 = Pipe, 1 = Http, 2 = MiniHTTP
	bool cliSpecified = false;
	WString cliCommand;

	for (int i = 1; i < argc; i++)
	{
#ifdef VCZH_MSVC
		auto argument = WString(argv[i]);
#else
		auto argument = u8tow(U8String(reinterpret_cast<const char8_t*>(argv[i])));
#endif
		auto consumed = gacui_test::remoteInjectionArguments.Consume(argument);
		if (consumed == 0) consumed = gacui_test::automationArguments.Consume(argument);
		if (consumed < 0) { Console::WriteLine(L"Error: invalid or duplicate option; /AsPort requires a decimal port in 1..65535."); return result; }
		if (consumed > 0) continue;
		if (ARGUMENT_COMPARE(argv[i], ARGUMENT_TEXT("/FCT")) == 0)
		{
			if (index != -1)
			{
				Console::WriteLine(L"Error: /FCT, /RPT and /RVMT are exclusive.");
				return result;
			}
			index = 0;
		}
		else if (ARGUMENT_COMPARE(argv[i], ARGUMENT_TEXT("/RPT")) == 0)
		{
			if (index != -1)
			{
				Console::WriteLine(L"Error: /FCT, /RPT and /RVMT are exclusive.");
				return result;
			}
			index = 1;
		}
		else if (ARGUMENT_COMPARE(argv[i], ARGUMENT_TEXT("/RVMT")) == 0)
		{
			if (index != -1)
			{
				Console::WriteLine(L"Error: /FCT, /RPT and /RVMT are exclusive.");
				return result;
			}
			index = 2;
		}
		else if (ARGUMENT_COMPARE(argv[i], ARGUMENT_TEXT("/Pipe")) == 0)
		{
			if (transport != -1)
			{
				Console::WriteLine(L"Error: /Pipe, /Http and /MiniHttp are exclusive.");
				return result;
			}
			transport = 0;
		}
		else if (ARGUMENT_COMPARE(argv[i], ARGUMENT_TEXT("/Http")) == 0)
		{
			if (transport != -1)
			{
				Console::WriteLine(L"Error: /Pipe, /Http and /MiniHttp are exclusive.");
				return result;
			}
			transport = 1;
		}
		else if (ARGUMENT_COMPARE(argv[i], ARGUMENT_TEXT("/MiniHttp")) == 0)
		{
			if (transport != -1)
			{
				Console::WriteLine(L"Error: /Pipe, /Http and /MiniHttp are exclusive.");
				return result;
			}
			transport = 2;
		}
		else if (ARGUMENT_NCOMPARE(argv[i], ARGUMENT_TEXT("/Cli:"), 5) == 0)
		{
			if (cliSpecified || !argv[i][5])
			{
				Console::WriteLine(L"Error: /Cli must be specified once with a nonempty host command.");
				return result;
			}
			cliSpecified = true;
			cliCommand = argument.Sub(5, argument.Length() - 5);
		}
		else
		{
			Console::WriteLine(L"Error: Unknown command line argument.");
			return result;
		}
	}

	if (cliSpecified && index != 2)
	{
		Console::WriteLine(L"Error: /Cli can only be used with explicit /RVMT.");
		return result;
	}

	if (index == -1)
	{
		index = 0;
	}

	if (transport == -1)
	{
		Console::WriteLine(L"Error: Either /Pipe, /Http or /MiniHttp must be provided.");
		return result;
	}

	if (transport == 0)
	{
#ifdef VCZH_MSVC
		result = StartNamedPipeServer(index, cliCommand);
#else
		Console::WriteLine(L"Error: /Pipe is only supported on Windows.");
#endif
	}
	else if (transport == 1)
	{
#ifdef VCZH_MSVC
		result = StartHttpServer(index, cliCommand);
#else
		Console::WriteLine(L"Error: /Http is only supported on Windows.");
#endif
	}
	else
	{
		result = StartMiniHttpServer(index, cliCommand);
	}
#if defined VCZH_MSVC && VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

#undef ARGUMENT_COMPARE
#undef ARGUMENT_NCOMPARE
#undef ARGUMENT_TEXT
