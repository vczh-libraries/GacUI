#include <Vlpp.h>
#include <VlppOS.h>
#include <crtdbg.h>

using namespace vl;
using namespace vl::console;

extern int StartNamedPipeServer(vint index);
extern int StartHttpServer(vint index);

int main(int argc, char* argv[])
{
#ifdef VCZH_MSVC
	_set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
	int result = 1;
	vint index = -1; // 0 = FullControlTest (/FCT), 1 = RemoteProtocolTest (/RPT)
	int transport = -1; // 0 = Pipe, 1 = Http

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "/FCT") == 0)
		{
			if (index != -1)
			{
				Console::WriteLine(L"Error: /FCT and /RPT are exclusive.");
				return result;
			}
			index = 0;
		}
		else if (strcmp(argv[i], "/RPT") == 0)
		{
			if (index != -1)
			{
				Console::WriteLine(L"Error: /FCT and /RPT are exclusive.");
				return result;
			}
			index = 1;
		}
		else if (strcmp(argv[i], "/Pipe") == 0)
		{
			if (transport != -1)
			{
				Console::WriteLine(L"Error: /Pipe and /Http are exclusive.");
				return result;
			}
			transport = 0;
		}
		else if (strcmp(argv[i], "/Http") == 0)
		{
			if (transport != -1)
			{
				Console::WriteLine(L"Error: /Pipe and /Http are exclusive.");
				return result;
			}
			transport = 1;
		}
		else
		{
			Console::WriteLine(L"Error: Unknown command line argument.");
			return result;
		}
	}

	if (index == -1)
	{
		index = 0;
	}

	if (transport == -1)
	{
		Console::WriteLine(L"Error: Either /Pipe or /Http must be provided.");
		return result;
	}

	if (transport == 0)
	{
		result = StartNamedPipeServer(index);
	}
	else
	{
		result = StartHttpServer(index);
	}
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}