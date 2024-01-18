#define GAC_HEADER_USE_NAMESPACE

#include "../../../Source/Compiler/RemoteProtocol/GuiRemoteProtocolCompiler.h"
#ifdef VCZH_MSVC
#include <Windows.h>
#endif

using namespace vl;
using namespace vl::presentation;

#if defined VCZH_MSVC
WString GetExePath()
{
	wchar_t buffer[65536];
	GetModuleFileName(NULL, buffer, sizeof(buffer) / sizeof(*buffer));
	vint pos = -1;
	vint index = 0;
	while (buffer[index])
	{
		if (buffer[index] == L'\\' || buffer[index] == L'/')
		{
			pos = index;
		}
		index++;
	}
	return WString::CopyFrom(buffer, pos + 1);
}

WString GetRemoteProtocolPath()
{
#ifdef _WIN64
	return GetExePath() + L"../../../../Source/PlatformProviders/Remote/Protocol/";
#else
	return GetExePath() + L"../../../Source/PlatformProviders/Remote/Protocol/";
#endif
}
#elif defined VCZH_GCC
WString GetRemoteProtocolPath()
{
	return L"../../../Source/PlatformProviders/Remote/Protocol/";
}
#endif

#if defined VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
#elif defined VCZH_GCC
int main(int argc, char* argv[])
#endif
{
	return 0;
}