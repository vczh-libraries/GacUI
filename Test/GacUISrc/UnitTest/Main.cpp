#include "../../../Source/GacUI.h"
#include "../../../Source/UnitTestUtilities/GuiUnitTestUtilities.h"
#if defined VCZH_MSVC
#include <Windows.h>
#endif

using namespace vl;
using namespace vl::unittest;
using namespace vl::presentation::unittest;

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
	return WString::CopyFrom(buffer, pos + 1);
}
#endif

namespace compiler_error_tests
{
	WString GetTestResourcePath()
	{
	#if defined VCZH_MSVC
	#ifdef _WIN64
		return GetExePath() + L"..\\..\\..\\Resources\\CompilerErrorTests";
	#else
		return GetExePath() + L"..\\..\\Resources\\CompilerErrorTests";
	#endif
	#elif defined VCZH_GCC
		return L"../../Resources/CompilerErrorTests";
	#endif
	}
	
	WString GetTestBaselinePath()
	{
	#if defined VCZH_MSVC
	#ifdef _WIN64
		return GetExePath() + L"..\\..\\..\\Resources\\CompilerErrorTests\\Baseline_x64";
	#else
		return GetExePath() + L"..\\..\\Resources\\CompilerErrorTests\\Baseline_x86";
	#endif
	#elif defined VCZH_GCC
		return L"../../Resources/CompilerErrorTests/Baseline_x64";
	#endif
	}
	
	WString GetTestOutputPath()
	{
	#if defined VCZH_MSVC
	#ifdef _WIN64
		return GetExePath() + L"..\\..\\..\\Output\\CompilerErrorTests\\x64";
	#else
		return GetExePath() + L"..\\..\\Output\\CompilerErrorTests\\x86";
	#endif
	#elif defined VCZH_GCC
		return L"../../Output/CompilerErrorTests";
	#endif
	}
}

namespace hosted_window_manager_tests
{
	WString GetTestBaselinePath()
	{
	#if defined VCZH_MSVC
	#ifdef _WIN64
		return GetExePath() + L"..\\..\\..\\Resources\\HostedWindowManagerTests";
	#else
		return GetExePath() + L"..\\..\\Resources\\HostedWindowManagerTests";
	#endif
	#elif defined VCZH_GCC
		return L"../../Resources/HostedWindowManagerTests";
	#endif
	}
}

namespace unittest_framework_tests
{
	WString GetTestSnapshotPath()
	{
	#if defined VCZH_MSVC
	#ifdef _WIN64
		return GetExePath() + L"..\\..\\..\\Resources\\UnitTestSnapshots";
	#else
		return GetExePath() + L"..\\..\\Resources\\UnitTestSnapshots";
	#endif
	#elif defined VCZH_GCC
		return L"../../Resources/UnitTestSnapshots";
	#endif
	}

	WString GetTestDataPath()
	{
	#if defined VCZH_MSVC
	#ifdef _WIN64
		return GetExePath() + L"..\\..\\..\\Resources\\UnitTestResources";
	#else
		return GetExePath() + L"..\\..\\Resources\\UnitTestResources";
	#endif
	#elif defined VCZH_GCC
		return L"../../Resources/UnitTestResources";
	#endif
	}
}

#if defined VCZH_MSVC
TEST_FILE
{
	{
		Folder folder(compiler_error_tests::GetTestOutputPath());
		if (!folder.Exists())
		{
			TEST_CASE_ASSERT(folder.Create(true) == true);
		}
	}
}
#endif

using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::reflection::description;

void SetGuiMainProxy(const Func<void()>& proxy)
{
	if (proxy)
	{
		GacUIUnitTest_SetGuiMainProxy([proxy](auto&&...)
		{
			proxy();
		});
	}
	else
	{
		GacUIUnitTest_SetGuiMainProxy({});
	}
}

template<typename T>
int UnitTestMain(int argc, T* argv[])
{
	UnitTestFrameworkConfig config;
	config.snapshotFolder = unittest_framework_tests::GetTestSnapshotPath();
	config.resourceFolder = unittest_framework_tests::GetTestDataPath();

	GacUIUnitTest_Initialize(&config);
	int result = UnitTest::RunAndDisposeTests(argc, argv);
	GacUIUnitTest_Finalize();
	return result;
}

#if defined VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
{
	int result = UnitTestMain(argc, argv);
#if defined VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
#elif defined VCZH_GCC
int main(int argc, char* argv[])
{
	return UnitTestMain(argc, argv);
}
#endif