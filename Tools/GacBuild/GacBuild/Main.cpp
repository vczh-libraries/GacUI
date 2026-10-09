#include "GacBuild.h"
#include <clocale>

using namespace vl;

#if defined VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
#elif defined VCZH_GCC
int main(int argc, char* argv[])
#endif
{
	setlocale(LC_ALL, "");
	try
	{
		collections::List<WString> arguments;
		for (vint i = 1; i < argc; i++)
		{
#if defined VCZH_MSVC
			arguments.Add(argv[i]);
#elif defined VCZH_GCC
			arguments.Add(u8tow(U8String(reinterpret_cast<const char8_t*>(argv[i]))));
#endif
		}
		gacbuild::Run(arguments);
		return 0;
	}
	catch (const Exception& ex)
	{
		console::Console::WriteLine(L"GacBuild: " + ex.Message());
	}
	catch (const Error& ex)
	{
		console::Console::WriteLine(WString(L"GacBuild: ") + ex.Description());
	}
	return 1;
}
