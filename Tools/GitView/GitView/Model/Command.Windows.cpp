#include "Git.h"

#ifdef VCZH_MSVC
#include <Windows.h>

namespace gitview
{
	using namespace vl;
	using namespace vl::collections;

	class CommandPipeReader : public Thread
	{
	public:
		HANDLE							handle = nullptr;
		stream::MemoryStream			bytes;
		DWORD							error = ERROR_SUCCESS;

		void Run() override
		{
			char buffer[4096];
			DWORD count = 0;
			while (ReadFile(handle, buffer, sizeof(buffer), &count, nullptr) && count)
			{
				bytes.Write(buffer, count);
			}
			error = GetLastError();
		}
	};

	CommandResult RunCliCommand(const WString& executable, const List<WString>& arguments)
	{
		stream::MemoryStream commandBytes;
		stream::StreamWriter writer(commandBytes);
		auto quote = [&](const WString& argument)
		{
			if (argument.Length() && argument.IndexOf(L' ') == -1 && argument.IndexOf(L'\t') == -1 && argument.IndexOf(L'\n') == -1 && argument.IndexOf(L'\r') == -1 && argument.IndexOf(L'"') == -1)
			{
				writer.WriteString(argument);
				return;
			}
			writer.WriteChar(L'"');
			vint slashes = 0;
			for (vint i = 0; i < argument.Length(); i++)
			{
				auto c = argument[i];
				if (c == L'\\') { slashes++; continue; }
				for (vint j = 0; j < slashes * (c == L'"' ? 2 : 1); j++) writer.WriteChar(L'\\');
				slashes = 0;
				if (c == L'"') writer.WriteChar(L'\\');
				writer.WriteChar(c);
			}
			for (vint j = 0; j < slashes * 2; j++) writer.WriteChar(L'\\');
			writer.WriteChar(L'"');
		};
		quote(executable);
		for (auto&& argument : arguments) { writer.WriteChar(L' '); quote(argument); }
		writer.WriteChar(0);

		SECURITY_ATTRIBUTES security = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
		CommandPipeReader output;
		CommandPipeReader error;
		HANDLE outputWrite = nullptr;
		HANDLE errorWrite = nullptr;
		if (!CreatePipe(&output.handle, &outputWrite, &security, 0)) throw Exception(WString::Unmanaged(L"CreatePipe failed."));
		if (!CreatePipe(&error.handle, &errorWrite, &security, 0)) throw Exception(WString::Unmanaged(L"CreatePipe failed."));
		SetHandleInformation(output.handle, HANDLE_FLAG_INHERIT, 0);
		SetHandleInformation(error.handle, HANDLE_FLAG_INHERIT, 0);
		auto input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING, 0, nullptr);
		STARTUPINFOW startup = {};
		startup.cb = sizeof(startup);
		startup.dwFlags = STARTF_USESTDHANDLES;
		startup.hStdInput = input;
		startup.hStdOutput = outputWrite;
		startup.hStdError = errorWrite;
		PROCESS_INFORMATION process = {};
		auto created = CreateProcessW(nullptr, static_cast<wchar_t*>(commandBytes.GetInternalBuffer()), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
		auto launchError = GetLastError();
		CloseHandle(input);
		CloseHandle(outputWrite);
		CloseHandle(errorWrite);
		if (!created)
		{
			CloseHandle(output.handle);
			CloseHandle(error.handle);
			throw Exception(WString::Unmanaged(L"Cannot start ") + executable + WString::Unmanaged(L": Windows error ") + itow(launchError));
		}
		error.Start();
		output.Run();
		error.Wait();
		WaitForSingleObject(process.hProcess, INFINITE);
		DWORD exitCode = 0;
		GetExitCodeProcess(process.hProcess, &exitCode);
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		CloseHandle(output.handle);
		CloseHandle(error.handle);
		if ((output.error != ERROR_BROKEN_PIPE && output.error != ERROR_SUCCESS) || (error.error != ERROR_BROKEN_PIPE && error.error != ERROR_SUCCESS))
		{
			throw Exception(WString::Unmanaged(L"Reading command output failed."));
		}
		return { static_cast<vint>(exitCode), DecodeCommandOutput(output.bytes), DecodeCommandOutput(error.bytes) };
	}
}
#endif
