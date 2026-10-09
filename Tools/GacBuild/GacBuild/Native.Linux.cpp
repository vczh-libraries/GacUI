#include "GacBuild.h"

#if defined VCZH_GCC
#include <cerrno>
#include <fcntl.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

namespace gacbuild
{
	bool IsAbsolutePath(const WString& path)
	{
		return path.Length() > 0 && path[0] == L'/';
	}

	void ValidateExecutable(const WString& path)
	{
		Require(IsAbsolutePath(path) && FilePath(path).IsFile(), L"Expected an absolute executable file path: " + path);
		auto utf8 = wtou8(path);
		Require(access(reinterpret_cast<const char*>(utf8.Buffer()), X_OK) == 0, L"File is not executable: " + path);
	}

	Nullable<FileInfo> GetFileInfo(const FilePath& path)
	{
		auto utf8 = wtou8(path.GetFullPath());
		struct stat data;
		if (stat(reinterpret_cast<const char*>(utf8.Buffer()), &data) != 0)
		{
			Require(errno == ENOENT || errno == ENOTDIR, L"Cannot inspect file: " + path.GetFullPath());
			return {};
		}
		if (!S_ISREG(data.st_mode)) return {};
#if defined VCZH_APPLE
		auto modified = data.st_mtimespec;
#elif !defined VCZH_APPLE
		auto modified = data.st_mtim;
#endif
		return FileInfo{ .size = static_cast<vuint64_t>(data.st_size), .modified = static_cast<vuint64_t>(modified.tv_sec) * 1000000000 + modified.tv_nsec };
	}

	void CopyFileNative(const FilePath& source, const FilePath& destination)
	{
		auto sourceUtf8 = wtou8(source.GetFullPath());
		auto destinationUtf8 = wtou8(destination.GetFullPath());
		auto input = open(reinterpret_cast<const char*>(sourceUtf8.Buffer()), O_RDONLY);
		Require(input != -1, L"Cannot read: " + source.GetFullPath());
		auto output = open(reinterpret_cast<const char*>(destinationUtf8.Buffer()), O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (output == -1) { close(input); throw Exception(L"Cannot write: " + destination.GetFullPath()); }
		bool succeeded = true;
		char buffer[65536];
		while (succeeded)
		{
			auto count = read(input, buffer, sizeof(buffer));
			if (count == -1 && errno == EINTR) continue;
			if (count == 0) break;
			if (count < 0) { succeeded = false; break; }
			ssize_t offset = 0;
			while (offset < count)
			{
				auto written = write(output, buffer + offset, count - offset);
				if (written == -1 && errno == EINTR) continue;
				if (written <= 0) { succeeded = false; break; }
				offset += written;
			}
		}
		if (close(input) != 0) succeeded = false;
		if (close(output) != 0) succeeded = false;
		Require(succeeded, L"Cannot copy " + source.GetFullPath() + L" to " + destination.GetFullPath());
	}

	void RunProcess(const FilePath& executable, const List<WString>& arguments)
	{
		List<U8String> strings;
		strings.Add(wtou8(executable.GetFullPath()));
		for (auto&& argument : arguments) strings.Add(wtou8(argument));
		Array<char*> argv(strings.Count() + 1);
		for (vint i = 0; i < strings.Count(); i++) argv[i] = const_cast<char*>(reinterpret_cast<const char*>(strings[i].Buffer()));
		argv[strings.Count()] = nullptr;
		pid_t child;
		Require(posix_spawn(&child, argv[0], nullptr, nullptr, &argv[0], environ) == 0, L"Cannot launch: " + executable.GetFullPath());
		int status;
		pid_t result;
		do { result = waitpid(child, &status, 0); } while (result == -1 && errno == EINTR);
		Require(result == child, L"Cannot wait for: " + executable.GetFullPath());
		Require(WIFEXITED(status) && WEXITSTATUS(status) == 0, L"Child failed: " + executable.GetFullPath());
	}
}
#endif
