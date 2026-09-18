#ifndef GACUI_TEST_AUTOMATIONARGUMENTS
#define GACUI_TEST_AUTOMATIONARGUMENTS

#include <Vlpp.h>
#ifdef VCZH_MSVC
#include <Windows.h>
#include <Shellapi.h>
#pragma comment(lib, "Shell32.lib")
#endif

namespace gacui_test
{
	class AutomationArguments
	{
		bool specified = false;
	public:
		vl::vint port = 8888;

		// 0: another option; 1: consumed; -1: invalid automation option.
		vl::vint Consume(const vl::WString& argument)
		{
			if (argument.Length() < 7 || argument.Left(7) != L"/AsPort") return 0;
			if (specified || argument.Length() <= 8 || argument[7] != L':') return -1;
			vl::vint value = 0;
			for (vl::vint i = 8; i < argument.Length(); i++)
			{
				if (argument[i] < L'0' || argument[i] > L'9') return -1;
				value = value * 10 + argument[i] - L'0';
				if (value > 65535) return -1;
			}
			if (!value) return -1;
			specified = true;
			port = value;
			return 1;
		}

#ifdef VCZH_MSVC
		bool ParseWindowsCommandLine()
		{
			int count = 0;
			auto arguments = CommandLineToArgvW(GetCommandLineW(), &count);
			if (!arguments) return false;
			bool valid = true;
			for (int i = 1; i < count; i++) if (Consume(vl::WString(arguments[i])) < 0) valid = false;
			LocalFree(arguments);
			return valid;
		}
#endif
	};

	inline AutomationArguments automationArguments;
}
#endif
