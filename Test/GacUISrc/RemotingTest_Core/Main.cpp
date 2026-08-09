#include <Vlpp.h>
#include <VlppOS.h>
#ifdef VCZH_MSVC
#include <crtdbg.h>
#endif

using namespace vl;
using namespace vl::console;

#ifdef VCZH_MSVC
extern int StartNamedPipeServer(vint index, const WString& cliPath);
extern int StartHttpServer(vint index, const WString& cliPath);
#define ARGUMENT_COMPARE wcscmp
#define ARGUMENT_NCOMPARE wcsncmp
#define ARGUMENT_TEXT(TEXT) L##TEXT
#else
#define ARGUMENT_COMPARE strcmp
#define ARGUMENT_NCOMPARE strncmp
#define ARGUMENT_TEXT(TEXT) TEXT
#endif
extern int StartMiniHttpServer(vint index, const WString& cliPath);

#ifdef VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef VCZH_MSVC
	_set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
	int result = 1;
	vint index = -1; // 0 = FullControlTest (/FCT), 1 = RemoteProtocolTest (/RPT), 2 = RemoteViewModelTest (/RVMT)
	int transport = -1; // 0 = Pipe, 1 = Http, 2 = MiniHTTP
	bool cliSpecified = false;
	WString cliPath;

	for (int i = 1; i < argc; i++)
	{
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
				Console::WriteLine(L"Error: /Cli must be specified once with a nonempty host path.");
				return result;
			}
			cliSpecified = true;
#ifdef VCZH_MSVC
			cliPath = WString::CopyFrom(argv[i] + 5, wcslen(argv[i] + 5));
#else
			cliPath = u8tow(U8String::CopyFrom(reinterpret_cast<const char8_t*>(argv[i] + 5), strlen(argv[i] + 5)));
#endif
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
		result = StartNamedPipeServer(index, cliPath);
#else
		Console::WriteLine(L"Error: /Pipe is only supported on Windows.");
#endif
	}
	else if (transport == 1)
	{
#ifdef VCZH_MSVC
		result = StartHttpServer(index, cliPath);
#else
		Console::WriteLine(L"Error: /Http is only supported on Windows.");
#endif
	}
	else
	{
		result = StartMiniHttpServer(index, cliPath);
	}
#if defined VCZH_MSVC && VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

#undef ARGUMENT_COMPARE
#undef ARGUMENT_NCOMPARE
#undef ARGUMENT_TEXT
