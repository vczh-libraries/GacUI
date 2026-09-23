#include "GacUI.h"

void GuiMain() {}

#ifdef VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	auto result = vl::unittest::UnitTest::RunAndDisposeTests(argc, argv);
	vl::presentation::DestroyPluginManager();
	vl::FinalizeGlobalStorage();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
