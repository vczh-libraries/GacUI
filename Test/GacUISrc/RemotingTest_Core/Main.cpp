#include <Vlpp.h>
#include <crtdbg.h>

using namespace vl;

extern int StartNamedPipeServer();
extern int StartHttpServer();

int main(int argc, char* argv[])
{
	int result = -1;
	CHECK_ERROR(argc == 2, L"main(...)#An command line argument must be provided.");
	if (strcmp(argv[1], "/Pipe") == 0)
	{
		result = StartNamedPipeServer();
	}
	else if (strcmp(argv[1], "/Http") == 0)
	{
		result = StartHttpServer();
	}
	else
	{
		CHECK_FAIL(L"main(...)#The command line argument must be either /Pipe or /Http.");
	}
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}