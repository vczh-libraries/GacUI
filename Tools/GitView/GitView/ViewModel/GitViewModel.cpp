#include "GitViewModel.h"

namespace gitview
{
	using namespace vl;
	using namespace vl::collections;
	using namespace vl::filesystem;
	using namespace vl::reflection::description;
	using namespace vl::presentation;

	Ptr<IValueList> MakeFileEntries(const List<GitFile>& source)
	{
		auto entries = IValueList::Create();
		for (auto&& file : source)
		{
			auto entry = Ptr(new Entry);
			entry->file = file;
			entry->text = file.status + WString::Unmanaged(L" ") + (file.oldPath.Length() ? file.oldPath + WString::Unmanaged(L" -> ") : WString::Empty) + file.path;
			entries->Add(BoxValue(Ptr<IEntry>(entry)));
		}
		return entries;
	}

/***********************************************************************
Entry / DiffLineModel
***********************************************************************/

	Entry::Entry() : children(IValueList::Create()) {}
	WString Entry::GetText() { return text; }
	Ptr<IValueList> Entry::GetChildren() { return children; }

	DiffLineModel::DiffLineModel(const DiffLine& value) : line(value) {}
	WString DiffLineModel::GetText() { return line.text; }
	bool DiffLineModel::GetIsSeparator() { return line.separator; }
	Color DiffLineModel::GetBackground()
	{
		return line.change > 0 ? Color(0, 64, 0) : line.change < 0 ? Color(96, 0, 0) : Color(0, 0, 0);
	}

/***********************************************************************
GitViewModel
***********************************************************************/

	GitViewModel::GitViewModel(const FilePath& directory)
		: repository(directory)
		, changes(Ptr(new Entry))
		, branches(IValueList::Create())
		, commits(IValueList::Create())
		, files(IValueList::Create())
		, changeDiff(IValueList::Create())
		, historyDiff(IValueList::Create())
	{
	}

	WString GitViewModel::GetRepositoryName() { return repository.GetRoot().GetName(); }
	Ptr<IValueList> GitViewModel::GetBranches() { return branches; }
	vint GitViewModel::GetBranchIndex() { return branchIndex; }
	bool GitViewModel::GetCanPull() { return branchIndex >= 0 && branchIndex < branches->GetCount(); }
	Ptr<IEntry> GitViewModel::GetChanges() { return changes; }
	Ptr<IValueList> GitViewModel::GetCommits() { return commits; }
	Ptr<IValueList> GitViewModel::GetFiles() { return files; }
	Ptr<IValueList> GitViewModel::GetChangeDiff() { return changeDiff; }
	Ptr<IValueList> GitViewModel::GetHistoryDiff() { return historyDiff; }
	WString GitViewModel::GetStatus() { return status; }
	vint GitViewModel::GetRevision() { return revision; }

	void GitViewModel::SetStatus(const WString& value)
	{
		status = value;
		StatusChanged();
	}

	void GitViewModel::LoadHistory()
	{
		selectedCommit = {};
		selectedCommitDetails = {};
		commits = IValueList::Create();
		files = IValueList::Create();
		historyDiff = IValueList::Create();
		if (branchIndex >= 0)
		{
			List<GitCommit> records;
			repository.History(UnboxValue<WString>(branches->Get(branchIndex)), records);
			for (auto&& record : records)
			{
				auto entry = Ptr(new Entry);
				entry->file.kind = FileKind::Commit;
				entry->text = record.text;
				entry->commit = record.hash;
				entry->details = record.hash + WString::Unmanaged(L" (") + record.author + WString::Unmanaged(L") ") + record.time;
				commits->Add(BoxValue(Ptr<IEntry>(entry)));
			}
		}
	}

	void GitViewModel::Refresh()
	{
		if (refreshing) return;
		refreshing = true;
		try
		{
			List<WString> names;
			repository.Branches(names);
			branches = IValueList::Create();
			for (auto&& name : names) branches->Add(BoxValue(name));
			branchIndex = 0;
			List<GitFile> records;
			repository.Changes(records);
			auto root = Ptr(new Entry);
			for (auto kind : { FileKind::Staged, FileKind::Unstaged, FileKind::Untracked })
			{
				auto group = Ptr(new Entry);
				group->text = kind == FileKind::Staged ? WString::Unmanaged(L"STAGED FILES") : kind == FileKind::Unstaged ? WString::Unmanaged(L"UNSTAGED FILES") : WString::Unmanaged(L"NEW FILES");
				List<GitFile> matching;
				for (auto&& record : records) if (record.kind == kind) matching.Add(record);
				group->children = MakeFileEntries(matching);
				root->children->Add(BoxValue(Ptr<IEntry>(group)));
			}
			changes = root;
			changeDiff = IValueList::Create();
			LoadHistory();
			SetStatus(WString::Unmanaged(L"CHANGES: working tree | HISTORY: selected branch | Select a file or commit. REFRESH reloads everything."));
		}
		catch (const Exception& error)
		{
			SetStatus(error.Message());
		}
		BranchesChanged(); BranchIndexChanged(); CanPullChanged(); ChangesChanged();
		CommitsChanged(); FilesChanged(); ChangeDiffChanged(); HistoryDiffChanged();
		revision++;
		RevisionChanged();
		refreshing = false;
	}

	void GitViewModel::SetBranchIndex(vint value)
	{
		if (refreshing || value == branchIndex || value < 0 || value >= branches->GetCount()) return;
		refreshing = true;
		branchIndex = value;
		try { LoadHistory(); SetStatus(WString::Unmanaged(L"Browsing history only. The checked-out branch and working tree are unchanged.")); }
		catch (const Exception& error) { SetStatus(error.Message()); }
		BranchIndexChanged(); CanPullChanged(); CommitsChanged(); FilesChanged(); HistoryDiffChanged();
		changeDiff = IValueList::Create();
		ChangeDiffChanged();
		revision++;
		RevisionChanged();
		refreshing = false;
	}

	void GitViewModel::LoadDiff(Ptr<Entry> entry, bool history)
	{
		auto result = IValueList::Create();
		if (entry && entry->file.kind != FileKind::Group)
		{
			auto lines = repository.Diff(entry->file, history ? selectedCommit : WString::Empty);
			for (auto&& line : lines) result->Add(BoxValue(Ptr<IDiffLine>(Ptr(new DiffLineModel(line)))));
			SetStatus(history ? selectedCommitDetails + WString::Unmanaged(L"\r\n") + entry->text : entry->text);
		}
		if (history) { historyDiff = result; HistoryDiffChanged(); }
		else { changeDiff = result; ChangeDiffChanged(); }
	}

	void GitViewModel::SelectChange(Ptr<IEntry> entry)
	{
		if (refreshing) return;
		changeDiff = IValueList::Create();
		ChangeDiffChanged();
		try { LoadDiff(entry.Cast<Entry>(), false); }
		catch (const Exception& error) { SetStatus(error.Message()); }
	}

	void GitViewModel::SelectCommit(Ptr<IEntry> entry)
	{
		if (refreshing) return;
		selectedCommit = {};
		selectedCommitDetails = {};
		files = IValueList::Create();
		historyDiff = IValueList::Create();
		try
		{
			if (entry)
			{
				selectedCommit = entry.Cast<Entry>()->commit;
				selectedCommitDetails = entry.Cast<Entry>()->details;
				List<GitFile> records;
				repository.CommitFiles(selectedCommit, records);
				files = MakeFileEntries(records);
				SetStatus(selectedCommitDetails);
			}
		}
		catch (const Exception& error) { SetStatus(error.Message()); }
		FilesChanged(); HistoryDiffChanged();
	}

	void GitViewModel::SelectFile(Ptr<IEntry> entry)
	{
		if (refreshing) return;
		historyDiff = IValueList::Create();
		HistoryDiffChanged();
		try { LoadDiff(entry.Cast<Entry>(), true); }
		catch (const Exception& error) { SetStatus(error.Message()); }
	}

	void GitViewModel::Pull(bool rebaseOnConflict)
	{
		if (!GetCanPull())
		{
			SetStatus(WString::Unmanaged(L"Pull failed. Select a branch first."));
			return;
		}
		auto result = repository.Pull(UnboxValue<WString>(branches->Get(branchIndex)), rebaseOnConflict);
		Refresh();
		SetStatus((result.exitCode == 0 ? WString::Unmanaged(L"Pull completed.\r\n") : WString::Unmanaged(L"Pull failed.\r\n")) + result.output + result.error);
	}
}
