#include "GacUI.h"
#ifdef VCZH_GCC
#include <clocale>
#endif

using namespace vl;

void GuiMain() {}

#ifdef VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef VCZH_GCC
	CHECK_ERROR(std::setlocale(LC_CTYPE, "") != nullptr, L"GitViewTests::main#Failed to initialize the character locale.");
#endif
	auto result = vl::unittest::UnitTest::RunAndDisposeTests(argc, argv);
	vl::presentation::DestroyPluginManager();
	vl::FinalizeGlobalStorage();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
