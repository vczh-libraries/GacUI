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

	TEST_CASE(L"Diff rendering uses numbered source rows and three rows of context")
	{
		List<DiffLine> lines;
		ParseDiff(L"diff --git a/file b/file\nindex 123..456 100644\n--- a/file\n+++ b/file\n@@ -5 +5,2 @@\n--- old source\n++++ new source\n+@@ source text\n", L"1\n2\n3\n4\n+++ new source\n@@ source text\n7\n8\n9\n10\n", lines);
		TEST_ASSERT(lines.Count() == 9);
		TEST_ASSERT(lines[0].text == L"   2 2" && lines[0].change == 0);
		TEST_ASSERT(lines[2].text == L"   4 4" && lines[2].change == 0);
		TEST_ASSERT(lines[3].text == L"   5 -- old source" && lines[3].change == -1);
		TEST_ASSERT(lines[4].text == L"   5 +++ new source" && lines[4].change == 1);
		TEST_ASSERT(lines[5].text == L"   6 @@ source text" && lines[5].change == 1);
		TEST_ASSERT(lines[8].text == L"   9 9" && lines[8].change == 0);
	});

	TEST_CASE(L"Nearby diff context is merged and distant unchanged source is omitted")
	{
		List<DiffLine> lines;
		ParseDiff(L"@@ -2 +2 @@\n-old2\n+2\n@@ -6 +6 @@\n-old6\n+6\n@@ -18 +18 @@\n-old18\n+18\n", L"1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n", lines);
		TEST_ASSERT(lines.Count() == 20);
		TEST_ASSERT(lines[0].text == L"   1 1");
		TEST_ASSERT(lines[5].text == L"   5 5" && lines[5].change == 0);
		TEST_ASSERT(lines[6].text == L"   6 old6" && lines[6].change == -1);
		TEST_ASSERT(lines[10].text == L"   9 9");
		TEST_ASSERT(lines[11].separator && lines[11].text.Length() == 0 && lines[11].change == 0);
		TEST_ASSERT(lines[12].text == L"  15 15");
		TEST_ASSERT(lines[19].text == L"  21 21");
		for (auto [line, i] : indexed(lines)) TEST_ASSERT(line.separator == (i == 11));
		DiffLineModel separator(lines[11]);
		TEST_ASSERT(separator.GetIsSeparator() && separator.GetBackground() == vl::presentation::Color(0, 0, 0));
	});

	TEST_CASE(L"Touching diff contexts stay joined across replacements, insertions and deletions")
	{
		const wchar_t* patches[] =
		{
			L"@@ -2 +2 @@\n-old2\n+2\n@@ -9 +9 @@\n-old9\n+9\n",
			L"@@ -1,0 +2,2 @@\n+2\n+3\n@@ -8 +10 @@\n-old10\n+10\n",
			L"@@ -2,2 +1,0 @@\n-old2\n-old3\n@@ -10 +8 @@\n-old10\n+8\n",
		};
		for (auto patch : patches)
		{
			List<DiffLine> lines;
			ParseDiff(WString::Unmanaged(patch), L"1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n", lines);
			TEST_ASSERT(lines.Count() == 14);
			for (auto&& line : lines) TEST_ASSERT(!line.separator);
		}
	});

	TEST_CASE(L"Even one omitted source line separates diff groups")
	{
		List<DiffLine> lines;
		ParseDiff(L"@@ -2 +2 @@\n-old2\n+2\n@@ -10 +10 @@\n-old10\n+10\n", L"1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n", lines);
		TEST_ASSERT(lines.Count() == 15);
		TEST_ASSERT(lines[5].text == L"   5 5");
		TEST_ASSERT(lines[6].separator);
		TEST_ASSERT(lines[7].text == L"   7 7");
		for (auto [line, i] : indexed(lines)) TEST_ASSERT(line.separator == (i == 6));
	});

	TEST_CASE(L"Diff rendering handles empty files, no final newline, CRLF and large line numbers")
	{
		List<DiffLine> lines;
		ParseDiff(L"@@ -0,0 +1 @@\n+new\n\\ No newline at end of file\n", L"new", lines);
		TEST_ASSERT(lines.Count() == 1 && lines[0].text == L"   1 new" && lines[0].change == 1);
		lines.Clear();
		ParseDiff(L"@@ -1 +0,0 @@\n-old\n\\ No newline at end of file\n", L"", lines);
		TEST_ASSERT(lines.Count() == 1 && lines[0].text == L"   1 old" && lines[0].change == -1);
		lines.Clear();
		ParseDiff(L"@@ -1 +1 @@\n-old\r\n+new\r\n", L"new\r\n\r\nafter\r\n", lines);
		TEST_ASSERT(lines.Count() == 4 && lines[2].text == L"   2 " && lines[3].text == L"   3 after");
		lines.Clear();
		stream::MemoryStream source;
		stream::StreamWriter writer(source);
		for (vint i = 0; i < 9999; i++) writer.WriteString(L"context\n");
		source.SeekFromBegin(0);
		stream::StreamReader reader(source);
		ParseDiff(L"@@ -10000 +9999,0 @@\n-old\n", reader.ReadToEnd(), lines);
		TEST_ASSERT(lines.Count() == 4 && lines[3].text == L"10000 old");
		lines.Clear();
		ParseDiff(L"Binary files a/file and b/file differ\n", L"", lines);
		TEST_ASSERT(lines.Count() == 1 && lines[0].text == L"Binary file changed." && lines[0].change == 0);
		lines.Clear();
		ParseDiff(L"diff --git a/old b/new\nsimilarity index 100%\nrename from old\nrename to new\n", L"", lines);
		TEST_ASSERT(lines.Count() == 1 && lines[0].text == L"No textual changes.");
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
		auto staged = repo.Diff(changes[0]);
		TEST_ASSERT(staged.Count() == 8);
		TEST_ASSERT(staged[0].text == L"   2 two" && staged[0].change == 0);
		TEST_ASSERT(staged[3].text == L"   5 five" && staged[3].change == -1);
		TEST_ASSERT(staged[4].text == L"   5 STAGED" && staged[4].change == 1);
		TEST_ASSERT(staged[7].text == L"   8 eight" && staged[7].change == 0);
		auto working = repo.Diff(changes[1]);
		TEST_ASSERT(working.Count() == 8);
		TEST_ASSERT(working[3].text == L"   5 STAGED" && working[3].change == -1);
		TEST_ASSERT(working[4].text == L"   5 WORKTREE" && working[4].change == 1);
		TEST_ASSERT(changes[2].path == unusual);
		auto untracked = repo.Diff(changes[2]);
		TEST_ASSERT(untracked.Count() == 1 && untracked[0].text == L"   1 new file" && untracked[0].change == 1);
		TEST_ASSERT(repo.Pull(L"other", false).exitCode != 0);
		TEST_ASSERT(repo.Pull(L"main", true).exitCode != 0);
		repo.History(L"main", history);
		TEST_ASSERT(history.Count() == 1);
		TEST_ASSERT(history[0].text == L"Test commit" && history[0].author == L"GitView Test" && history[0].time.Length() > 0);
		List<GitFile> rootFiles;
		repo.CommitFiles(history[0].hash, rootFiles);
		TEST_ASSERT(rootFiles.Count() == 1 && rootFiles[0].path == L"tracked.txt");
		auto historical = repo.Diff(rootFiles[0], history[0].hash);
		TEST_ASSERT(historical.Count() == 10);
		TEST_ASSERT(historical[0].text == L"   1 one" && historical[0].change == 1);
		TEST_ASSERT(historical[4].text == L"   5 five" && historical[9].text == L"  10 ten");
		GitViewModel model(fixture.root);
		model.Refresh();
		TEST_ASSERT(model.GetBranches()->GetCount() == 2 && model.GetCanPull());
		TEST_ASSERT(model.GetChanges()->GetChildren()->GetCount() == 3);
		auto group = UnboxValue<Ptr<IEntry>>(model.GetChanges()->GetChildren()->Get(0));
		model.SelectChange(UnboxValue<Ptr<IEntry>>(group->GetChildren()->Get(0)));
		TEST_ASSERT(model.GetChangeDiff()->GetCount() > 0);
		auto commit = UnboxValue<Ptr<IEntry>>(model.GetCommits()->Get(0));
		TEST_ASSERT(commit->GetText() == L"Test commit");
		model.SelectCommit(commit);
		TEST_ASSERT(model.GetFiles()->GetCount() == 1);
		auto details = history[0].hash + L" (GitView Test) " + history[0].time;
		TEST_ASSERT(model.GetStatus() == details);
		model.SelectFile(UnboxValue<Ptr<IEntry>>(model.GetFiles()->Get(0)));
		TEST_ASSERT(model.GetHistoryDiff()->GetCount() > 0);
		TEST_ASSERT(model.GetStatus() == details + L"\r\nA tracked.txt");
		model.SetBranchIndex(1);
		TEST_ASSERT(model.GetCanPull() && repo.CurrentBranch() == L"main");
		TEST_ASSERT(model.GetFiles()->GetCount() == 0 && model.GetHistoryDiff()->GetCount() == 0);
		model.Refresh();
		TEST_ASSERT(model.GetChangeDiff()->GetCount() == 0 && model.GetFiles()->GetCount() == 0);
		fixture.Commit();
		fixture.Git({ L"mv", L"tracked.txt", L"renamed.txt" });
		changes.Clear(); repo.Changes(changes);
		TEST_ASSERT(changes.Count() == 1 && changes[0].oldPath == L"tracked.txt");
		auto rename = repo.Diff(changes[0]);
		TEST_ASSERT(rename.Count() == 1 && rename[0].text == L"No textual changes.");
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

	TEST_CASE(L"Real deleted, binary and UTF-8 BOM files render without patch metadata")
	{
		Fixture fixture;
		fixture.Write(L"text.txt", L"old\n"); fixture.Commit();
		GitRepository repo(fixture.root);
		TEST_ASSERT(File(fixture.root / L"text.txt").Delete());
		List<GitFile> files;
		repo.Changes(files);
		auto removed = repo.Diff(files[0]);
		TEST_ASSERT(removed.Count() == 1 && removed[0].text == L"   1 old" && removed[0].change == -1);
		fixture.Git({ L"add", L"--all" });
		files.Clear(); repo.Changes(files);
		removed = repo.Diff(files[0]);
		TEST_ASSERT(removed.Count() == 1 && removed[0].text == L"   1 old" && removed[0].change == -1);
		fixture.Commit();
		List<GitCommit> history;
		repo.History(L"main", history);
		files.Clear(); repo.CommitFiles(history[0].hash, files);
		removed = repo.Diff(files[0], history[0].hash);
		TEST_ASSERT(removed.Count() == 1 && removed[0].text == L"   1 old" && removed[0].change == -1);
		{
			stream::FileStream file((fixture.root / L"binary.dat").GetFullPath(), stream::FileStream::WriteOnly);
			uint8_t binary[] = { 0, 1, 2, 3 };
			TEST_ASSERT(file.Write(binary, sizeof(binary)) == sizeof(binary));
		}
		TEST_ASSERT(File(fixture.root / L"bom.txt").WriteAllText(L"new\n", true, stream::BomEncoder::Utf8));
		files.Clear(); repo.Changes(files);
		TEST_ASSERT(files.Count() == 2);
		auto binaryDiff = repo.Diff(files[0]);
		TEST_ASSERT(binaryDiff.Count() == 1 && binaryDiff[0].text == L"Binary file changed.");
		auto bom = repo.Diff(files[1]);
		TEST_ASSERT(bom.Count() == 1 && bom[0].text == L"   1 \xFEFFnew" && bom[0].change == 1);
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
		auto merged = repo.Diff(files[0], history[0].hash);
		TEST_ASSERT(merged.Count() == 1 && merged[0].text == L"   1 side" && merged[0].change == 1);
		parent.Git({ L"worktree", L"add", L"-b", L"linked", L"linked" });
		GitRepository worktree(parent.root / L"linked");
		TEST_ASSERT(worktree.GetRoot() == parent.root / L"linked");
		TEST_ASSERT(worktree.CurrentBranch() == L"linked");
		TEST_ASSERT(Contains(worktree.Pull(L"linked", false).error, L"origin"));
		parent.Delete(); child.Delete();
	});

	TEST_CASE(L"Pull reports selected and actual branch mismatches for both menu actions")
	{
		Fixture remote;
		remote.Write(L"tracked.txt", L"base\n"); remote.Commit();
		Fixture local;
		local.Git({ L"remote", L"add", L"origin", remote.root.GetFullPath() });
		local.Git({ L"pull", L"origin", L"main" });
		local.Git({ L"branch", L"other" });
		remote.Write(L"tracked.txt", L"remote update\n"); remote.Commit();
		GitRepository repo(local.root);
		GitViewModel model(local.root);
		auto head = local.Git({ L"rev-parse", L"HEAD" });
		auto index = local.Git({ L"write-tree" });
		auto fetchHead = local.Git({ L"rev-parse", L"FETCH_HEAD" });
		for (auto rebase : { false, true })
		{
			for (vint scenario = 0; scenario < 4; scenario++)
			{
				local.Git({ L"checkout", L"main" });
				model.Refresh();
				if (scenario == 0) model.SetBranchIndex(1);
				else if (scenario == 1) local.Git({ L"checkout", L"other" });
				else local.Git({ L"checkout", L"--detach" });
				if (scenario == 3) model.Refresh();
				auto actual = repo.CurrentBranch();
				TEST_ASSERT(model.GetCanPull());
				model.Pull(rebase);
				TEST_ASSERT(Contains(model.GetStatus(), L"Pull failed."));
				TEST_ASSERT(Contains(model.GetStatus(), L"No pull was performed."));
				TEST_ASSERT(!Contains(model.GetStatus(), L"Resolve conflicts"));
				if (scenario < 2)
				{
					TEST_ASSERT(Contains(model.GetStatus(), scenario == 0 ? L"Selected branch 'other'" : L"Selected branch 'main'"));
					TEST_ASSERT(Contains(model.GetStatus(), scenario == 0 ? L"checked-out branch 'main'" : L"checked-out branch 'other'"));
				}
				else TEST_ASSERT(Contains(model.GetStatus(), L"HEAD is detached"));
				TEST_ASSERT(repo.CurrentBranch() == actual);
				TEST_ASSERT(local.Git({ L"rev-parse", L"HEAD" }) == head);
				TEST_ASSERT(local.Git({ L"write-tree" }) == index);
				TEST_ASSERT(local.Git({ L"rev-parse", L"FETCH_HEAD" }) == fetchHead);
				TEST_ASSERT(local.Git({ L"status", L"--porcelain=v1" }).Length() == 0);
				WString content;
				TEST_ASSERT(File(local.root / L"tracked.txt").ReadAllTextByBom(content));
				TEST_ASSERT(content == L"base\n");
			}
		}
		local.Git({ L"checkout", L"main" });
		model.Refresh();
		model.Pull(false);
		TEST_ASSERT(Contains(model.GetStatus(), L"Pull completed."));
		TEST_ASSERT(local.Git({ L"rev-parse", L"HEAD" }) == remote.Git({ L"rev-parse", L"HEAD" }));
		local.Delete();
		remote.Delete();
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
		List<GitFile> conflicts;
		repo.Changes(conflicts);
		TEST_ASSERT(conflicts.Count() == 1);
		auto conflict = repo.Diff(conflicts[0]);
		TEST_ASSERT(conflict.Count() > 0 && conflict[0].text == L"   1 <<<<<<< HEAD" && conflict[0].change == 1);
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
