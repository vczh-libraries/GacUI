#ifndef GACUI_TEST_AUTOMATIONARGUMENTS
#define GACUI_TEST_AUTOMATIONARGUMENTS

#include <VlppOS.h>
#if defined VCZH_MSVC
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
		bool gdi = false;

		// 0: another option; 1: consumed; -1: invalid automation option.
		vl::vint Consume(const vl::WString& argument)
		{
			if (argument == L"/Gdi")
			{
				if (gdi) return -1;
				gdi = true;
				return 1;
			}
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

#if defined VCZH_MSVC
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

#if defined VCZH_MSVC || defined VCZH_GCC
	struct CoreHostArguments
	{
		bool specified = false;
		vl::WString host = vl::WString::Unmanaged(L"localhost");

		// Consume the fixed prefix only: the remaining colons belong to IPv6.
		vl::vint Consume(const vl::WString& argument)
		{
			if (argument.Length() < 9 || argument.Left(9) != L"/corehost") return 0;
			if (specified || argument.Length() <= 10 || argument[9] != L':') return -1;
			auto value = argument.Right(argument.Length() - 10);
			if (value.Length() >= 2 && value[0] == L'[' && value[value.Length() - 1] == L']')
			{
				value = value.Sub(1, value.Length() - 2);
				if (value.IndexOf(L':') == -1) return -1;
			}
			if (!vl::inter_process::async_tcp_socket::IsValidSocketHost(value)) return -1;
			host = value;
			specified = true;
			return 1;
		}
	};

#endif

	struct RemoteInjectionArguments
	{
		bool injectDateTime = false;
		bool injectLocale = false;
		bool injectFileSystem = false;

		vl::vint Consume(const vl::WString& argument)
		{
			bool* option = argument == L"/remoteDT" ? &injectDateTime
				: argument == L"/remoteLocale" ? &injectLocale
				: argument == L"/remoteFS" ? &injectFileSystem : nullptr;
			if (!option) return 0;
			if (*option) return -1;
			*option = true;
			return 1;
		}
	};

	inline RemoteInjectionArguments remoteInjectionArguments;
}
#endif
