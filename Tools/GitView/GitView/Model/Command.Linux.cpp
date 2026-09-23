#include "Git.h"

#ifdef VCZH_GCC
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

extern char** environ;

namespace gitview
{
	using namespace vl;
	using namespace vl::collections;

	CommandResult RunCliCommand(const WString& executable, const List<WString>& arguments)
	{
		List<U8String> encoded;
		encoded.Add(wtou8(executable));
		for (auto&& argument : arguments) encoded.Add(wtou8(argument));
		Array<char*> argv(encoded.Count() + 1);
		for (vint i = 0; i < encoded.Count(); i++) argv[i] = reinterpret_cast<char*>(const_cast<char8_t*>(encoded[i].Buffer()));
		argv[encoded.Count()] = nullptr;
		int output[2];
		int error[2];
		if (pipe(output) != 0 || pipe(error) != 0) throw Exception(WString::Unmanaged(L"Cannot create command pipes."));
		posix_spawn_file_actions_t actions;
		posix_spawn_file_actions_init(&actions);
		posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
		posix_spawn_file_actions_adddup2(&actions, output[1], STDOUT_FILENO);
		posix_spawn_file_actions_adddup2(&actions, error[1], STDERR_FILENO);
		for (auto fd : { output[0], output[1], error[0], error[1] }) posix_spawn_file_actions_addclose(&actions, fd);
		pid_t pid = 0;
		auto status = posix_spawnp(&pid, argv[0], &actions, nullptr, &argv[0], environ);
		posix_spawn_file_actions_destroy(&actions);
		close(output[1]);
		close(error[1]);
		if (status != 0)
		{
			close(output[0]); close(error[0]);
			throw Exception(WString::Unmanaged(L"Cannot start ") + executable + WString::Unmanaged(L": POSIX error ") + itow(status));
		}
		stream::MemoryStream outputBytes;
		stream::MemoryStream errorBytes;
		pollfd pipes[] = { {output[0], POLLIN, 0}, {error[0], POLLIN, 0} };
		while (pipes[0].fd != -1 || pipes[1].fd != -1)
		{
			if (poll(pipes, 2, -1) == -1)
			{
				if (errno == EINTR) continue;
				throw Exception(WString::Unmanaged(L"Polling command output failed."));
			}
			for (vint i = 0; i < 2; i++)
			{
				if (pipes[i].fd == -1 || !pipes[i].revents) continue;
				char buffer[4096];
				auto count = read(pipes[i].fd, buffer, sizeof(buffer));
				if (count > 0) (i == 0 ? outputBytes : errorBytes).Write(buffer, count);
				else if (count == 0) { close(pipes[i].fd); pipes[i].fd = -1; }
				else if (errno != EINTR) throw Exception(WString::Unmanaged(L"Reading command output failed."));
			}
		}
		while (waitpid(pid, &status, 0) == -1)
		{
			if (errno != EINTR) throw Exception(WString::Unmanaged(L"Waiting for command failed."));
		}
		return { WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status),
			DecodeCommandOutput(outputBytes), DecodeCommandOutput(errorBytes) };
	}
}
#endif
