#include "../GitView/ViewModel/GitViewModel.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::reflection::description;
using namespace vl::unittest;
using namespace gitview;

namespace gitview_tests
{
	class Fixture
	{
	public:
		FilePath root;

		Fixture()
			: root(FilePath(L".") / (L"GitViewFixture-" + u64tow(DateTime::LocalTime().osMilliseconds)))
		{
			TEST_ASSERT(!Folder(root).Exists());
			TEST_ASSERT(Folder(root).Create(false));
			Git({ L"init", L"-b", L"main", L"--template=" });
			Git({ L"config", L"user.name", L"GitView Test" });
			Git({ L"config", L"user.email", L"gitview@example.invalid" });
			Git({ L"config", L"commit.gpgsign", L"false" });
			Git({ L"config", L"core.autocrlf", L"false" });
		}

		WString Git(std::initializer_list<WString> arguments)
		{
			auto full = MakeArguments({ L"-C", root.GetFullPath() });
			for (auto&& argument : arguments) full.Add(argument);
			auto result = RunCliCommand(WString::Unmanaged(L"git"), full);
			if (result.exitCode != 0) throw Exception(result.error + result.output);
			return result.output;
		}

		void Write(const WString& path, const WString& content)
		{
			TEST_ASSERT(File(root / path).WriteAllText(content, false, stream::BomEncoder::Utf8));
		}

		void Commit()
		{
			Git({ L"add", L"--all" });
			Git({ L"commit", L"-m", L"Test commit" });
		}

		void Delete()
		{
			// Git object files are read-only on Windows. Change attributes only in this owned fixture.
#ifdef VCZH_MSVC
			auto result = RunCliCommand(WString::Unmanaged(L"attrib.exe"), MakeArguments({ L"-R", L"-S", L"-H", (root / L"*").GetFullPath(), L"/S", L"/D" }));
			TEST_ASSERT(result.exitCode == 0);
			TEST_ASSERT(result.output.Length() == 0 && result.error.Length() == 0);
#endif
			TEST_ASSERT(Folder(root).Delete(true));
		}
	};

	bool Contains(const WString& text, const WString& part)
	{
		for (vint i = 0; i + part.Length() <= text.Length(); i++)
		{
			if (text.Sub(i, part.Length()) == part) return true;
		}
		return false;
	}
}

using namespace gitview_tests;

TEST_FILE
{
	TEST_CASE(L"Command arguments preserve Unicode, quotes, shell syntax and trailing slashes")
	{
		auto value = WString::Unmanaged(L"中文 \"quoted\" & %PATH% $(literal) trailing \\");
		auto result = RunCliCommand(WString::Unmanaged(L"git"), MakeArguments({ L"-c", L"gitview.test=" + value, L"config", L"--get", L"gitview.test" }));
		TEST_ASSERT(result.exitCode == 0 && result.output == value + L"\n" && result.error.Length() == 0);
		result = RunCliCommand(WString::Unmanaged(L"git"), MakeArguments({ L"--gitview-invalid-option" }));
		TEST_ASSERT(result.exitCode != 0 && result.output.Length() == 0 && result.error.Length() > 0);
	});

	TEST_CASE(L"NUL status preserves rename source, whitespace and separate index/worktree entries")
	{
		const wchar_t raw[] = L"MM both.txt\0R  new\tname.txt\0old\nname.txt\0?? unicode-中文.txt\0UU conflict.txt\0";
		List<GitFile> files;
		ParseStatus(WString::CopyFrom(raw, sizeof(raw) / sizeof(wchar_t) - 1), files);
		TEST_ASSERT(files.Count() == 5);
		TEST_ASSERT(files[0].kind == FileKind::Staged && files[1].kind == FileKind::Unstaged);
		TEST_ASSERT(files[2].path == L"new\tname.txt" && files[2].oldPath == L"old\nname.txt");
		TEST_ASSERT(files[3].kind == FileKind::Untracked && files[3].path == L"unicode-中文.txt");
		TEST_ASSERT(files[4].kind == FileKind::Unstaged);
	});

	TEST_CASE(L"Diff headers stay neutral while hunk additions and deletions are colored")
	{
		List<DiffLine> lines;
		ParseDiff(L"--- a/file\n+++ b/file\n@@ -1 +1 @@\n-old\n+new\n context\n", lines);
		TEST_ASSERT(lines.Count() == 6);
		TEST_ASSERT(lines[0].change == 0 && lines[1].change == 0 && lines[2].change == 0);
		TEST_ASSERT(lines[3].change == -1 && lines[4].change == 1 && lines[5].change == 0);
	});

	TEST_CASE(L"Real repository discovery, unborn history, filenames, diffs and view-model selections")
	{
		Fixture fixture;
		Folder(fixture.root / L"nested").Create(false);
		GitRepository repo(fixture.root / L"nested");
		TEST_ASSERT(repo.GetRoot() == fixture.root);
		List<GitCommit> history;
		repo.History(L"main", history);
		TEST_ASSERT(history.Count() == 0);
		fixture.Write(L"tracked.txt", L"one\ntwo\nthree\nfour\nfive\nsix\nseven\neight\nnine\nten\n");
		fixture.Commit();
		fixture.Git({ L"branch", L"other" });
		fixture.Git({ L"tag", L"main" });
		TEST_ASSERT(repo.CurrentBranch() == L"main");
		fixture.Write(L"tracked.txt", L"one\ntwo\nthree\nfour\nSTAGED\nsix\nseven\neight\nnine\nten\n");
		fixture.Git({ L"add", L"tracked.txt" });
		fixture.Write(L"tracked.txt", L"one\ntwo\nthree\nfour\nWORKTREE\nsix\nseven\neight\nnine\nten\n");
		auto unusual = WString::Unmanaged(L"中文 & percent% $(literal) [x].txt");
		fixture.Write(unusual, L"new file\n");
		List<GitFile> changes;
		repo.Changes(changes);
		TEST_ASSERT(changes.Count() == 3);
		TEST_ASSERT(Contains(repo.Diff(changes[0]), L"+STAGED"));
		TEST_ASSERT(Contains(repo.Diff(changes[1]), L"+WORKTREE"));
		TEST_ASSERT(Contains(repo.Diff(changes[0]), L"@@ -2,7 +2,7 @@"));
		TEST_ASSERT(changes[2].path == unusual);
		TEST_ASSERT(Contains(repo.Diff(changes[2]), L"+new file"));
		TEST_ASSERT(repo.Pull(L"other", false).exitCode != 0);
		TEST_ASSERT(repo.Pull(L"main", true).exitCode != 0);
		repo.History(L"main", history);
		TEST_ASSERT(history.Count() == 1);
		List<GitFile> rootFiles;
		repo.CommitFiles(history[0].hash, rootFiles);
		TEST_ASSERT(rootFiles.Count() == 1 && rootFiles[0].path == L"tracked.txt");
		TEST_ASSERT(Contains(repo.Diff(rootFiles[0], history[0].hash), L"+one"));
		GitViewModel model(fixture.root);
		model.Refresh();
		TEST_ASSERT(model.GetBranches()->GetCount() == 2 && model.GetCanPull());
		TEST_ASSERT(model.GetChanges()->GetChildren()->GetCount() == 3);
		auto group = UnboxValue<Ptr<IEntry>>(model.GetChanges()->GetChildren()->Get(0));
		model.SelectChange(UnboxValue<Ptr<IEntry>>(group->GetChildren()->Get(0)));
		TEST_ASSERT(model.GetChangeDiff()->GetCount() > 0);
		model.SelectCommit(UnboxValue<Ptr<IEntry>>(model.GetCommits()->Get(0)));
		TEST_ASSERT(model.GetFiles()->GetCount() == 1);
		model.SelectFile(UnboxValue<Ptr<IEntry>>(model.GetFiles()->Get(0)));
		TEST_ASSERT(model.GetHistoryDiff()->GetCount() > 0);
		model.SetBranchIndex(1);
		TEST_ASSERT(!model.GetCanPull() && repo.CurrentBranch() == L"main");
		TEST_ASSERT(model.GetFiles()->GetCount() == 0 && model.GetHistoryDiff()->GetCount() == 0);
		model.Refresh();
		TEST_ASSERT(model.GetChangeDiff()->GetCount() == 0 && model.GetFiles()->GetCount() == 0);
		fixture.Commit();
		fixture.Git({ L"mv", L"tracked.txt", L"renamed.txt" });
		changes.Clear(); repo.Changes(changes);
		TEST_ASSERT(changes.Count() == 1 && changes[0].oldPath == L"tracked.txt");
		TEST_ASSERT(Contains(repo.Diff(changes[0]), L"rename to renamed.txt"));
		fixture.Commit();
		history.Clear(); repo.History(L"main", history);
		rootFiles.Clear(); repo.CommitFiles(history[0].hash, rootFiles);
		TEST_ASSERT(rootFiles.Count() == 1 && rootFiles[0].oldPath == L"tracked.txt");
		fixture.Git({ L"checkout", L"--detach" });
		TEST_ASSERT(repo.CurrentBranch().Length() == 0);
		history.Clear(); repo.History(L"(detached HEAD)", history);
		TEST_ASSERT(history.Count() == 3);
		fixture.Delete();
	});

	TEST_CASE(L"Submodules and worktrees discover their own root; merge history uses first parent")
	{
		Fixture child;
		child.Write(L"child.txt", L"submodule\n"); child.Commit();
		Fixture parent;
		parent.Write(L"parent.txt", L"parent\n"); parent.Commit();
		parent.Git({ L"-c", L"protocol.file.allow=always", L"submodule", L"add", child.root.GetFullPath(), L"sub" });
		parent.Commit();
		GitRepository submodule(parent.root / L"sub");
		TEST_ASSERT(submodule.GetRoot() == parent.root / L"sub");
		parent.Git({ L"checkout", L"-b", L"side" });
		parent.Write(L"side.txt", L"side\n"); parent.Commit();
		parent.Git({ L"checkout", L"main" });
		parent.Write(L"main.txt", L"main\n"); parent.Commit();
		parent.Git({ L"merge", L"--no-ff", L"--no-edit", L"side" });
		GitRepository repo(parent.root);
		List<GitCommit> history;
		repo.History(L"main", history);
		List<GitFile> files;
		repo.CommitFiles(history[0].hash, files);
		TEST_ASSERT(files.Count() == 1 && files[0].path == L"side.txt");
		TEST_ASSERT(Contains(repo.Diff(files[0], history[0].hash), L"+side"));
		parent.Git({ L"worktree", L"add", L"-b", L"linked", L"linked" });
		GitRepository worktree(parent.root / L"linked");
		TEST_ASSERT(worktree.GetRoot() == parent.root / L"linked");
		TEST_ASSERT(worktree.CurrentBranch() == L"linked");
		TEST_ASSERT(Contains(worktree.Pull(L"linked", false).error, L"origin"));
		parent.Delete(); child.Delete();
	});

	TEST_CASE(L"Pull uses only local remotes and retains conflicts for external resolution")
	{
		Fixture remote;
		remote.Write(L"conflict.txt", L"base\n"); remote.Commit();
		remote.Git({ L"config", L"receive.denyCurrentBranch", L"ignore" });
		Fixture local;
		local.Git({ L"remote", L"add", L"origin", remote.root.GetFullPath() });
		GitRepository repo(local.root);
		TEST_ASSERT(repo.Pull(L"main", false).exitCode == 0);
		TEST_ASSERT(File(local.root / L"conflict.txt").Exists());
		local.Write(L"conflict.txt", L"local\n"); local.Commit();
		remote.Write(L"conflict.txt", L"remote\n"); remote.Commit();
		TEST_ASSERT(repo.Pull(L"main", false).exitCode != 0);
		TEST_ASSERT(File(local.root / L".git/MERGE_HEAD").Exists());
		TEST_ASSERT(Contains(repo.Pull(L"main", true).error, L"existing Git operation"));
		local.Git({ L"merge", L"--abort" });
		TEST_ASSERT(repo.Pull(L"main", true).exitCode != 0);
		TEST_ASSERT(Folder(local.root / L".git/rebase-merge").Exists());
		TEST_ASSERT(!File(local.root / L".git/MERGE_HEAD").Exists());
		TEST_ASSERT(Contains(local.Git({ L"status", L"--porcelain=v1" }), L"UU conflict.txt"));
		local.Git({ L"rebase", L"--abort" });
		local.Delete();
		remote.Delete();
	});
}
