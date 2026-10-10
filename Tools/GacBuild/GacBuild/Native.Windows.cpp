#include "GacBuild.h"

#if defined VCZH_MSVC
#include <Windows.h>

namespace gacbuild
{
	void ValidateExecutable(const WString& path)
	{
		Require(FilePath::IsAbsolutePath(path) && FilePath(path).IsFile(), L"Expected an absolute executable file path: " + path);
		DWORD binaryType;
		Require(GetBinaryTypeW(path.Buffer(), &binaryType) != FALSE, L"Not a Windows executable: " + path);
	}

	void RunProcess(const FilePath& executable, const List<WString>& arguments)
	{
		stream::MemoryStream buffer;
		stream::StreamWriter writer(buffer);
		// Quote each CRT argument independently, including trailing backslashes.
		auto quote = [&](const WString& argument)
		{
			writer.WriteChar(L'"');
			vint slashes = 0;
			for (vint i = 0; i < argument.Length(); i++)
			{
				auto c = argument[i];
				if (c == L'\\') { slashes++; continue; }
				for (vint j = 0; j < slashes * (c == L'"' ? 2 : 1); j++) writer.WriteChar(L'\\');
				if (c == L'"') writer.WriteChar(L'\\');
				writer.WriteChar(c);
				slashes = 0;
			}
			for (vint j = 0; j < slashes * 2; j++) writer.WriteChar(L'\\');
			writer.WriteChar(L'"');
		};
		quote(executable.GetFullPath());
		for (auto&& argument : arguments) { writer.WriteChar(L' '); quote(argument); }
		writer.WriteChar(0);

		STARTUPINFOW startup = {};
		startup.cb = sizeof(startup);
		startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		startup.wShowWindow = SW_HIDE;
		startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		startup.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		PROCESS_INFORMATION process = {};
		Require(CreateProcessW(executable.GetFullPath().Buffer(), static_cast<wchar_t*>(buffer.GetInternalBuffer()),
			nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process) != FALSE,
			L"Cannot launch: " + executable.GetFullPath());
		CloseHandle(process.hThread);
		auto waitResult = WaitForSingleObject(process.hProcess, INFINITE);
		DWORD exitCode = 1;
		auto gotExitCode = GetExitCodeProcess(process.hProcess, &exitCode);
		CloseHandle(process.hProcess);
		Require(waitResult == WAIT_OBJECT_0 && gotExitCode, L"Cannot wait for: " + executable.GetFullPath());
		Require(exitCode == 0, L"Child failed (" + itow(exitCode) + L"): " + executable.GetFullPath());
	}
}
#endif
