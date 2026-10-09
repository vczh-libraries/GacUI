#include "GacBuild.h"

#if defined VCZH_MSVC
#include <Windows.h>

namespace gacbuild
{
	bool IsAbsolutePath(const WString& path)
	{
		return (path.Length() >= 3 && path[1] == L':' && (path[2] == L'\\' || path[2] == L'/'))
			|| (path.Length() >= 3 && path[0] == L'\\' && path[1] == L'\\');
	}

	void ValidateExecutable(const WString& path)
	{
		Require(IsAbsolutePath(path) && FilePath(path).IsFile(), L"Expected an absolute executable file path: " + path);
		DWORD binaryType;
		Require(GetBinaryTypeW(path.Buffer(), &binaryType) != FALSE, L"Not a Windows executable: " + path);
	}

	Nullable<FileInfo> GetFileInfo(const FilePath& path)
	{
		WIN32_FILE_ATTRIBUTE_DATA data;
		if (!GetFileAttributesExW(path.GetFullPath().Buffer(), GetFileExInfoStandard, &data))
		{
			auto error = GetLastError();
			Require(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND, L"Cannot inspect file: " + path.GetFullPath());
			return {};
		}
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return {};
		return FileInfo{
			.size = (static_cast<vuint64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow,
			.modified = (static_cast<vuint64_t>(data.ftLastWriteTime.dwHighDateTime) << 32) | data.ftLastWriteTime.dwLowDateTime,
		};
	}

	void CopyFileNative(const FilePath& source, const FilePath& destination)
	{
		auto succeeded = CopyFileW(source.GetFullPath().Buffer(), destination.GetFullPath().Buffer(), FALSE) != FALSE;
		auto error = GetLastError();
		Require(succeeded, L"Cannot copy " + source.GetFullPath() + L" to " + destination.GetFullPath() + L" (Windows error " + itow(error) + L").");
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
