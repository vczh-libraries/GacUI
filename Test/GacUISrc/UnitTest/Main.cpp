#include "../../../Source/GacUI.h"
#include <Windows.h>

using namespace vl;

#if defined VCZH_MSVC
using namespace vl::filesystem;
#endif

#if defined VCZH_MSVC
WString GetExePath()
{
	wchar_t buffer[65536];
	GetModuleFileName(NULL, buffer, sizeof(buffer) / sizeof(*buffer));
	vint pos = -1;
	vint index = 0;
	while (buffer[index])
	{
		if (buffer[index] == L'\\')
		{
			pos = index;
		}
		index++;
	}
	return WString(buffer, pos + 1);
}
#endif

WString GetTestResourcePath()
{
#if defined VCZH_MSVC
#ifdef _WIN64
	return GetExePath() + L"..\\..\\..\\Resources\\";
#else
	return GetExePath() + L"..\\..\\Resources\\";
#endif
#elif defined VCZH_GCC
	return L"../Resources/";
#endif
}

WString GetTestOutputPath()
{
#if defined VCZH_MSVC
#ifdef _WIN64
	return GetExePath() + L"..\\..\\..\\Output\\";
#else
	return GetExePath() + L"..\\..\\Output\\";
#endif
#elif defined VCZH_GCC
	return L"../Output/";
#endif
}

void GuiMain()
{
#if defined VCZH_MSVC
	{
		Folder folder(GetTestOutputPath());
		if (!folder.Exists())
		{
			TEST_ASSERT(folder.Create(false) == true);
		}
	}
#endif
	unittest::UnitTest::RunAndDisposeTests();
}

#if defined VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
#elif defined VCZH_GCC
int main()
#endif
{
	int result = SetupWindowsDirect2DRenderer();
#if defined VCZH_MSVC && defined VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
