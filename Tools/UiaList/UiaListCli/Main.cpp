#include "GacUI.Windows.h"
#undef GetRoleText
#include "../UiaList/ViewModel/CliViewModel.h"
#include <cstdlib>

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;

namespace uialist::cli
{
	class ConsoleInput : public Thread
	{
	public:
		CliViewModel& model;
		EventObject response;
		bool stopping = false;
		bool failed = false;

		ConsoleInput(CliViewModel& value) : model(value)
		{
			response.CreateAutoUnsignal(false);
		}

		void Write(const WString& text)
		{
			if (!text.Length()) return;
			auto line = text + L"\n";
			auto handle = GetStdHandle(STD_OUTPUT_HANDLE); DWORD mode = 0, written = 0;
			if (GetConsoleMode(handle, &mode))
			{
				for (vint offset = 0; offset < line.Length(); offset += written)
					CHECK_ERROR(WriteConsoleW(handle, line.Buffer() + offset, static_cast<DWORD>((std::min)(line.Length() - offset, static_cast<vint>(16384))), &written, nullptr) && written, L"uialist::cli::ConsoleInput::Write#Console output failed.");
			}
			else
			{
				auto bytes = wtou8(line);
				for (vint offset = 0; offset < bytes.Length(); offset += written)
					CHECK_ERROR(WriteFile(handle, bytes.Buffer() + offset, static_cast<DWORD>(bytes.Length() - offset), &written, nullptr) && written, L"uialist::cli::ConsoleInput::Write#Redirected output failed.");
			}
		}

		void Run() override
		{
			auto handle = GetStdHandle(STD_INPUT_HANDLE); DWORD mode = 0; bool console = GetConsoleMode(handle, &mode) != FALSE;
			while (!stopping)
			{
				collections::List<wchar_t> wide;
				collections::List<char8_t> utf8;
				bool eof = false;
				for (;;)
				{
					DWORD count = 0; wchar_t wc = 0; char8_t byte = 0;
					auto ok = console ? ReadConsoleW(handle, &wc, 1, &count, nullptr) : ReadFile(handle, &byte, 1, &count, nullptr);
					if (!ok || !count) { eof = true; break; }
					if (console && wc == 26 && !wide.Count()) { eof = true; break; }
					if (console ? wc == L'\n' : byte == u8'\n') break;
					if (console) wide.Add(wc); else utf8.Add(byte);
				}
				bool validUtf8 = true;
				WString line;
				if (console) line = wide.Count() ? WString::CopyFrom(&wide[0], wide.Count()) : WString();
				else if (utf8.Count())
				{
					auto bytes = reinterpret_cast<const char*>(&utf8[0]);
					auto length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, static_cast<int>(utf8.Count()), nullptr, 0);
					validUtf8 = length > 0;
					if (validUtf8)
					{
						collections::Array<wchar_t> buffer(length);
						MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, static_cast<int>(utf8.Count()), &buffer[0], length);
						line = WString::CopyFrom(&buffer[0], length);
					}
				}
				if (eof && !line.Length() && validUtf8)
				{
					GetApplication()->InvokeInMainThread(nullptr, [this]() { model.root.RequestClose(); stopping = true; response.Signal(); PostQuitMessage(0); });
					response.Wait(); return;
				}
				GetApplication()->InvokeInMainThread(nullptr, [this, line, validUtf8]()
				{
					try
					{
						if (validUtf8) model.Execute(line);
						else
						{
							model.command = {}; model.target = {}; model.phase = L"validation";
							model.Fail(L"InvalidEncoding", L"Redirected command lines must be valid UTF-8.");
						}
					}
					catch (const Exception& error) { model.Fail(L"Fatal", error.Message(), L"Execute command", E_FAIL, true); }
					catch (const Error& error) { model.Fail(L"Fatal", error.Description(), L"Execute command", E_FAIL, true); }
				});
				response.Wait();
			}
		}
	};
}

int cliExitCode = 0;

void GuiMain()
{
	const auto locale = WString::Unmanaged(L"en-US");
	uialist::UiaListViewModel root(uialist::Strings::Get(Locale(locale)), locale);
	uialist::cli::CliViewModel model(root);
	uialist::cli::ConsoleInput input(model);
	model.output = [&](const WString& text, bool exit, bool fatal)
	{
		input.Write(text);
		if (exit) { input.stopping = true; input.failed = fatal; PostQuitMessage(0); }
		input.response.Signal();
	};
	input.Start();
	while (GetApplication()->RunOneCycle());
	input.Wait();
	root.RequestClose();
	cliExitCode = input.failed ? 1 : 0;
}

int wmain()
{
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
	_set_error_mode(_OUT_TO_STDERR);
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif
	try { SetupWindowsDirect2DRenderer(); }
	catch (const Exception& error) { OutputDebugStringW(error.Message().Buffer()); return 1; }
	catch (const Error& error) { OutputDebugStringW(error.Description()); return 1; }
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return cliExitCode;
}
