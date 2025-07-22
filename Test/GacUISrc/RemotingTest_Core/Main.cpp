#include <crtdbg.h>

extern int StartNamedPipeServer();
extern int StartHttpServer();

int wmain(int argc, wchar_t* argv[])
{
	int result = StartNamedPipeServer();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}