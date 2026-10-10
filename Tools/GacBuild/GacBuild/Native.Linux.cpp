#include "GacBuild.h"

#if defined VCZH_GCC
#include <cerrno>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

namespace gacbuild
{
	void ValidateExecutable(const WString& path)
	{
		Require(FilePath::IsAbsolutePath(path) && FilePath(path).IsFile(), L"Expected an absolute executable file path: " + path);
		auto utf8 = wtou8(path);
		Require(access(reinterpret_cast<const char*>(utf8.Buffer()), X_OK) == 0, L"File is not executable: " + path);
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
