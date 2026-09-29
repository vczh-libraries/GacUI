#include "../../../Source/GacUI.h"
#include "../../../Source/UnitTestUtilities/GuiUnitTestUtilities.h"
#if defined VCZH_MSVC
#define _WINSOCKAPI_
#include <Windows.h>
#elif defined VCZH_GCC
#include <iostream>
#include <locale>
#endif

using namespace vl;
using namespace vl::unittest;
using namespace vl::presentation::unittest;

using namespace vl::filesystem;

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
	#elif defined VCZH_WASM
		return L"/Resources/CompilerErrorTests";
	#endif
	}
	
	WString GetTestSharedBaselinePath()
	{
	#if defined VCZH_MSVC
	#ifdef _WIN64
		return GetExePath() + L"..\\..\\..\\Resources\\CompilerErrorTests\\Baseline";
	#else
		return GetExePath() + L"..\\..\\Resources\\CompilerErrorTests\\Baseline";
	#endif
	#elif defined VCZH_GCC
		return L"../../Resources/CompilerErrorTests/Baseline";
	#elif defined VCZH_WASM
		return L"/Resources/CompilerErrorTests/Baseline";
	#endif
	}
	
	WString GetTestPlatformBaselinePath()
	{
	#if defined VCZH_MSVC
	#ifdef _WIN64
		return GetExePath() + L"..\\..\\..\\Resources\\CompilerErrorTests\\Baseline_x64";
	#else
		return GetExePath() + L"..\\..\\Resources\\CompilerErrorTests\\Baseline_x86";
	#endif
	#elif defined VCZH_GCC
		return L"../../Resources/CompilerErrorTests/Baseline_x64";
	#elif defined VCZH_WASM
		return L"/Resources/CompilerErrorTests/Baseline_x86";
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
	#elif defined VCZH_WASM
		return L"/Output/CompilerErrorTests";
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
	#elif defined VCZH_WASM
		return L"/Resources/HostedWindowManagerTests";
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
	#elif defined VCZH_WASM
		return L"/Resources/UnitTestSnapshots";
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
	#elif defined VCZH_WASM
		return L"/Resources/UnitTestResources";
	#endif
	}
}

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

using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::reflection::description;

remoteprotocol::ControllerGlobalConfig MakeGlobalConfig()
{
	vl::presentation::remoteprotocol::ControllerGlobalConfig globalConfig;
	globalConfig.osSuperKeyName = WString::Unmanaged(L"Super");
#if defined VCZH_WCHAR_UTF16
	globalConfig.documentCaretFromEncoding = vl::presentation::remoteprotocol::CharacterEncoding::UTF16;
#elif defined VCZH_WCHAR_UTF32
	globalConfig.documentCaretFromEncoding = vl::presentation::remoteprotocol::CharacterEncoding::UTF32;
#endif
	return globalConfig;
}

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
	EnUsLocaleImpl unitTestLocaleImpl;
	InjectLocaleImpl(&unitTestLocaleImpl);
	UnitTestFrameworkConfig config;
	config.snapshotFolder = unittest_framework_tests::GetTestSnapshotPath();
	config.resourceFolder = unittest_framework_tests::GetTestDataPath();

	GacUIUnitTest_Initialize(&config);
	int result = UnitTest::RunAndDisposeTests(argc, argv);
	GacUIUnitTest_Finalize();
	EjectLocaleImpl(nullptr);
	return result;
}

#if defined VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
{
	int result = UnitTestMain(argc, argv);
	UnitTest::DumpMemoryLeak(argc, argv);
	return result;
}
#elif defined VCZH_GCC
int main(int argc, char* argv[])
{
	std::wcout.imbue(std::locale(""));
	return UnitTestMain(argc, argv);
}
#endif

#if defined VCZH_WASM
#include <emscripten.h>
#include <emscripten/bind.h>

EM_JS(int, WasmReportFailure, (const char16_t* text, vint length), {
	return globalThis["vlConsoleFailure"](HEAPU16, text, length);
});

vint WasmMain()
{
	wchar_t name[] = L"UnitTest";
	wchar_t mode[] = L"/D";
	wchar_t* arguments[] = { name, mode };
	return UnitTestMain(2, arguments);
}

vint wasm_main()
{
	try
	{
		WString message;
		try
		{
			return WasmMain();
		}
		catch (const vl::unittest::UnitTestAssertError& error)
		{
			message = error.message;
		}
		catch (const vl::unittest::UnitTestConfigError& error)
		{
			message = error.message;
		}
		catch (const vl::unittest::UnitTestJustCrashError&)
		{
			message = L"The unit test framework stopped after a failure.";
		}
		catch (const Error& error)
		{
			message = error.Description();
		}
		catch (const Exception& error)
		{
			message = error.Message();
		}
		catch (const std::exception& error)
		{
			message = atow(error.what());
		}
		catch (...)
		{
			message = L"Unknown C++ exception.";
		}
		auto text = wtou16(message);
		WasmReportFailure(text.Buffer(), text.Length());
	}
	catch (...)
	{
		// Diagnostics can allocate too; no C++ exception may cross this boundary.
		constexpr char16_t text[] = u"Unable to format the C++ failure diagnostic.";
		WasmReportFailure(text, sizeof(text) / sizeof(*text) - 1);
	}
	return 1;
}

EMSCRIPTEN_BINDINGS(CppApplication)
{
	emscripten::function("wasm_main", &wasm_main);
}
#endif
