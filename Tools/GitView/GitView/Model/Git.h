#ifndef VCZH_GITVIEW_GIT
#define VCZH_GITVIEW_GIT

#include "VlppOS.h"
#include <initializer_list>

namespace gitview
{
	struct CommandResult
	{
		vl::vint						exitCode = 0;
		vl::WString						output;
		vl::WString						error;
	};

	// Arguments are passed directly to the program, without a command shell.
	extern CommandResult				RunCliCommand(const vl::WString& executable, const vl::collections::List<vl::WString>& arguments);
	extern vl::collections::List<vl::WString> MakeArguments(std::initializer_list<vl::WString> arguments);
	extern vl::WString					DecodeCommandOutput(vl::stream::MemoryStream& bytes);

	enum class FileKind { Group, Staged, Unstaged, Untracked, History, Commit };

	struct GitFile
	{
		FileKind						kind = FileKind::Group;
		vl::WString						path;
		vl::WString						oldPath;
		vl::WString						status;
	};

	struct GitCommit
	{
		vl::WString						hash;
		vl::WString						text;
		vl::WString						author;
		vl::WString						time;
	};

	struct DiffLine
	{
		vl::WString						text;
		vl::vint						change = 0;
		bool							separator = false;
	};

	extern void							SplitRecords(const vl::WString& text, wchar_t separator, vl::collections::List<vl::WString>& records);
	extern void							ParseStatus(const vl::WString& text, vl::collections::List<GitFile>& files);
	extern void							ParseDiff(const vl::WString& patch, const vl::WString& newText, vl::collections::List<DiffLine>& lines);

	class GitRepository
	{
	protected:
		vl::filesystem::FilePath			root;

	public:
										GitRepository(const vl::filesystem::FilePath& directory);
		vl::filesystem::FilePath			GetRoot() const;
		CommandResult					Run(const vl::collections::List<vl::WString>& arguments) const;
		CommandResult					Run(std::initializer_list<vl::WString> arguments) const;
		vl::WString						Read(std::initializer_list<vl::WString> arguments) const;
		vl::WString						CurrentBranch() const;
		void							Branches(vl::collections::List<vl::WString>& branches) const;
		void							Changes(vl::collections::List<GitFile>& files) const;
		void							History(const vl::WString& branch, vl::collections::List<GitCommit>& commits) const;
		void							CommitFiles(const vl::WString& commit, vl::collections::List<GitFile>& files) const;
		vl::collections::List<DiffLine>	Diff(const GitFile& file, const vl::WString& commit = {}) const;
		CommandResult					Pull(const vl::WString& branch, bool rebaseOnConflict) const;
	};
}

#endif
