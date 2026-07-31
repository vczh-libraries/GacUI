#if defined VCZH_GCC && !defined VCZH_APPLE

#include <Vlpp.h>

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		return 1;
	}
	if (strcmp(argv[1], "/Pipe") == 0)
	{
		return 1;
	}
	if (strcmp(argv[1], "/Http") == 0)
	{
		return 1;
	}
	if (strcmp(argv[1], "/MiniHttp") == 0)
	{
		// The local Linux UI is supplied by the platform-specific GacUI port.
		// Keep the full portable RVM stack compiled here while the remoting
		// Core executable provides the runnable Linux MiniHTTP path.
		return 0;
	}
	return 1;
}

#endif
