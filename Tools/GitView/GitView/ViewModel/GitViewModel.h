#ifndef VCZH_GITVIEW_VIEWMODEL
#define VCZH_GITVIEW_VIEWMODEL

#include "../Source/GitViewPartialClasses.h"
#include "../Model/Git.h"

namespace gitview
{
	class Entry : public vl::Object, public virtual IEntry
	{
	public:
		GitFile							file;
		vl::WString						text;
		vl::WString						commit;
		vl::WString						details;
		vl::Ptr<vl::reflection::description::IValueList> children;

										Entry();
		vl::WString						GetText() override;
		vl::Ptr<vl::reflection::description::IValueList> GetChildren() override;
	};

	class DiffLineModel : public vl::Object, public virtual IDiffLine
	{
	public:
		DiffLine						line;
										DiffLineModel(const DiffLine& value);
		vl::WString						GetText() override;
		vl::presentation::Color			GetBackground() override;
		bool							GetIsSeparator() override;
	};

	class GitViewModel : public vl::Object, public virtual IGitViewModel
	{
	protected:
		GitRepository					repository;
		vl::Ptr<IEntry>					changes;
		vl::Ptr<vl::reflection::description::IValueList> branches;
		vl::Ptr<vl::reflection::description::IValueList> commits;
		vl::Ptr<vl::reflection::description::IValueList> files;
		vl::Ptr<vl::reflection::description::IValueList> changeDiff;
		vl::Ptr<vl::reflection::description::IValueList> historyDiff;
		vl::WString						selectedCommit;
		vl::WString						selectedCommitDetails;
		vl::WString						status;
		vl::vint						branchIndex = -1;
		vl::vint						revision = 0;
		bool							refreshing = false;

		void							LoadHistory();
		void							SetStatus(const vl::WString& value);
		void							LoadDiff(vl::Ptr<Entry> entry, bool history);

	public:
										GitViewModel(const vl::filesystem::FilePath& directory);
		vl::WString						GetRepositoryName() override;
		vl::Ptr<vl::reflection::description::IValueList> GetBranches() override;
		vl::vint						GetBranchIndex() override;
		void							SetBranchIndex(vl::vint value) override;
		bool							GetCanPull() override;
		vl::Ptr<IEntry>					GetChanges() override;
		vl::Ptr<vl::reflection::description::IValueList> GetCommits() override;
		vl::Ptr<vl::reflection::description::IValueList> GetFiles() override;
		vl::Ptr<vl::reflection::description::IValueList> GetChangeDiff() override;
		vl::Ptr<vl::reflection::description::IValueList> GetHistoryDiff() override;
		vl::WString						GetStatus() override;
		vl::vint						GetRevision() override;
		void							Refresh() override;
		void							SelectChange(vl::Ptr<IEntry> entry) override;
		void							SelectCommit(vl::Ptr<IEntry> entry) override;
		void							SelectFile(vl::Ptr<IEntry> entry) override;
		void							Pull(bool rebaseOnConflict) override;
	};
}

#endif
