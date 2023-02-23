#include "GuiFakeDialogServiceBase.h"
#include "../../Application/Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace controls;

/***********************************************************************
View Model (IFileDialogFilter)
***********************************************************************/

		class FileDialogFilter : public Object, public virtual IFileDialogFilter
		{
		public:
			WString						name;
			WString						filter;

			WString GetName() override
			{
				return name;
			}

			WString GetFilter() override
			{
				return filter;
			}
		};

/***********************************************************************
View Model (IFileDialogFolder)
***********************************************************************/

		class FileDialogFolder : public Object, public virtual IFileDialogFolder
		{
		protected:
			vint						taskId = 0;
			bool						taskFired = false;

		public:
			FileDialogFolder*			parent = nullptr;
			FileDialogFolderType		type = FileDialogFolderType::Placeholder;
			filesystem::Folder			folder;
			WString						name;
			WString						textLoadingFolders;
			Folders						children;

			void AddChild(Ptr<FileDialogFolder> child)
			{
				child->textLoadingFolders = textLoadingFolders;
				child->parent = this;
				children.Add(child);
			}

			void AddPlaceholderChild()
			{
				auto child = Ptr(new FileDialogFolder);
				child->name = textLoadingFolders;
				AddChild(child);
			}

			FileDialogFolder() = default;

			FileDialogFolder(const filesystem::FilePath& filePath)
				: folder(filePath)
			{
			}

			Ptr<IFileDialogFolder> GetParent() override
			{
				return Ptr(static_cast<IFileDialogFolder*>(parent));
			}

			FileDialogFolderType GetType() override
			{
				if (type == FileDialogFolderType::Placeholder && !taskFired)
				{
					taskFired = true;
					auto taskFolder = Ptr(parent);
					vint taskFolderId = ++taskFolder->taskId;
					GetApplication()->InvokeAsync([taskFolder, taskFolderId]()
					{
						auto subFolders = Ptr(new List<filesystem::Folder>);
						if (!taskFolder->folder.GetFolders(*subFolders.Obj()))
						{
							subFolders->Clear();
						}

						GetApplication()->InvokeInMainThread(nullptr, [taskFolder, taskFolderId, subFolders]()
						{
							if (taskFolder->taskId == taskFolderId)
							{
								taskFolder->children.Clear();
								for (auto subFolder : *subFolders.Obj())
								{
									auto child = Ptr(new FileDialogFolder);
									child->type = FileDialogFolderType::Folder;
									child->folder = subFolder;
									child->name = subFolder.GetFilePath().GetName();
									child->AddPlaceholderChild();
									taskFolder->AddChild(child);
								}
							}
						});
					});
				}
				return type;
			}

			WString GetFullPath() override
			{
				return folder.GetFilePath().GetFullPath();
			}

			WString GetName() override
			{
				return name;
			}

			Folders& GetFolders() override
			{
				return children;
			}
		};

/***********************************************************************
View Model (IFileDialogFile)
***********************************************************************/

		class FileDialogFile : public Object, public virtual IFileDialogFile
		{
		public:
			FileDialogFileType			type = FileDialogFileType::Placeholder;
			Ptr<FileDialogFolder>		folder;
			WString						name;

			FileDialogFileType GetType() override
			{
				return type;
			}

			Ptr<IFileDialogFolder> GetAssociatedFolder() override
			{
				return folder;
			}

			WString GetName() override
			{
				return name;
			}
		};

/***********************************************************************
View Model (IFileDialogViewModel)
***********************************************************************/

		class FileDialogViewModel : public Object, public virtual IFileDialogViewModel
		{
		protected:
			WString						textLoadingFolders;
			WString						textLoadingFiles;
			WString						dialogErrorFileNotExist;
			WString						dialogErrorFileExpected;
			WString						dialogErrorFolderNotExist;
			WString						dialogErrorMultipleSelectionNotEnabled;
			WString						dialogAskCreateFile;
			WString						dialogAskOverrideFile;

			bool						isLoadingFiles = false;
			filesystem::FilePath		loadingPath;
			vint						loadingTaskId = 0;
			Files						files;

		public:
			bool						confirmed = false;
			WString						title;
			bool						enabledMultipleSelection = false;
			bool						fileMustExist = false;
			bool						folderMustExist = false;
			bool						promptCreateFile = false;
			bool						promptOverriteFile = false;
			WString						defaultExtension;

			Filters						filters;
			Ptr<FileDialogFilter>		selectedFilter;

			Ptr<FileDialogFolder>		rootFolder;
			Ptr<FileDialogFolder>		selectedFolder;

			WString GetTitle() override
			{
				return title;
			}

			bool GetEnabledMultipleSelection() override
			{
				return enabledMultipleSelection;
			}

			WString GetDefaultExtension() override
			{
				return defaultExtension;
			}

			const Filters& GetFilters() override
			{
				return filters;
			}

			Ptr<IFileDialogFilter> GetSelectedFilter() override
			{
				return selectedFilter;
			}

			void SetSelectedFilter(Ptr<IFileDialogFilter> value) override
			{
				if (auto filter = value.Cast<FileDialogFilter>())
				{
					if (selectedFilter != filter && filters.Contains(value.Obj()))
					{
						selectedFilter = filter;
						RefreshFiles();
					}
				}
			}

			Ptr<IFileDialogFolder> GetRootFolder() override
			{
				return rootFolder;
			}

			Ptr<IFileDialogFolder> GetSelectedFolder() override
			{
				return selectedFolder;
			}

			void SetSelectedFolder(Ptr<IFileDialogFolder> value) override
			{
				if (auto folder = value.Cast<FileDialogFolder>())
				{
					if (selectedFolder != folder)
					{
						selectedFolder = folder;
						SelectedFolderChanged();

						if (loadingPath != selectedFolder->folder.GetFilePath())
						{
							RefreshFiles();
						}
					}
				}
			}

			bool GetIsLoadingFiles() override
			{
				return isLoadingFiles;
			}

			Files& GetFiles() override
			{
				return files;
			}

			void RefreshFiles() override
			{
				if (!selectedFolder) return;

				vint taskId = ++loadingTaskId;
				auto taskFolder = selectedFolder;
				auto taskPath = (loadingPath = selectedFolder->folder.GetFilePath());
				if (!isLoadingFiles)
				{
					isLoadingFiles = true;
					IsLoadingFilesChanged();

					files.Clear();
					{
						auto item = Ptr(new FileDialogFile);
						item->name = textLoadingFiles;
						files.Add(item);
					}
				}

				auto vm = Ptr(this);
				GetApplication()->InvokeAsync([taskId, taskFolder, taskPath, vm]()
				{
					auto folders = Ptr(new List<filesystem::Folder>);
					auto files = Ptr(new List<filesystem::File>);
					filesystem::Folder loadingFolder = taskPath;
					if (!loadingFolder.GetFolders(*folders.Obj()) || !loadingFolder.GetFiles(*files.Obj()))
					{
						folders->Clear();
						files->Clear();
					}

					GetApplication()->InvokeInMainThread(nullptr, [taskId, taskFolder, folders, files, vm]()
					{
						if (vm->loadingTaskId == taskId)
						{
							vm->files.Clear();

							for (auto folder : *folders.Obj())
							{
								auto item = Ptr(new FileDialogFile);
								item->type = FileDialogFileType::Folder;
								item->folder = taskFolder;
								item->name = folder.GetFilePath().GetName();
								vm->files.Add(item);
							}

							for (auto file : *files.Obj())
							{
								auto item = Ptr(new FileDialogFile);
								item->type = FileDialogFileType::File;
								item->folder = taskFolder;
								item->name = file.GetFilePath().GetName();
								vm->files.Add(item);
							}

							vm->isLoadingFiles = false;
							vm->IsLoadingFilesChanged();
						}
					});
				});
			}

			bool TryConfirm(const collections::List<WString>& selectedPaths) override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void InitLocalizedText(
				const WString& _textLoadingFolders,
				const WString& _textLoadingFiles,
				const WString& _dialogErrorFileNotExist,
				const WString& _dialogErrorFileExpected,
				const WString& _dialogErrorFolderNotExist,
				const WString& _dialogErrorMultipleSelectionNotEnabled,
				const WString& _dialogAskCreateFile,
				const WString& _dialogAskOverrideFile
			) override
			{
				textLoadingFolders = _textLoadingFolders;
				textLoadingFiles = _textLoadingFiles;
				dialogErrorFileNotExist = _dialogErrorFileNotExist;
				dialogErrorFileExpected = _dialogErrorFileExpected;
				dialogErrorFolderNotExist = _dialogErrorFolderNotExist;
				dialogErrorMultipleSelectionNotEnabled = _dialogErrorMultipleSelectionNotEnabled;
				dialogAskCreateFile = _dialogAskCreateFile;
				dialogAskOverrideFile = _dialogAskOverrideFile;

				rootFolder->textLoadingFolders = textLoadingFolders;
				rootFolder->AddPlaceholderChild();
			}
		};

/***********************************************************************
FakeDialogServiceBase
***********************************************************************/

		bool FakeDialogServiceBase::ShowFileDialog(
			INativeWindow* window,
			collections::List<WString>& selectionFileNames,
			vint& selectionFilterIndex,
			FileDialogTypes dialogType,
			const WString& title,
			const WString& initialFileName,
			const WString& initialDirectory,
			const WString& defaultExtension,
			const WString& filter,
			FileDialogOptions options
		)
		{
			auto vm = Ptr(new FileDialogViewModel);
			vm->title = title;
			vm->enabledMultipleSelection = (options | INativeDialogService::FileDialogAllowMultipleSelection) != 0;
			vm->fileMustExist = (options | INativeDialogService::FileDialogFileMustExist) != 0;
			vm->folderMustExist = (options | INativeDialogService::FileDialogDirectoryMustExist) != 0;
			vm->promptCreateFile = (options | INativeDialogService::FileDialogPromptCreateFile) != 0;
			vm->promptOverriteFile = (options | INativeDialogService::FileDialogPromptOverwriteFile) != 0;
			vm->defaultExtension = defaultExtension;

			vint filterStart = 0;
			while (true)
			{
				vint first = -1;
				vint second = -1;
				vint count = filter.Length();

				for (vint i = filterStart; i < count; i++)
				{
					if (filter[i] == L'|')
					{
						first = i;
						break;
					}
				}
				if (first == -1) break;

				for (vint i = first + 1; i < count; i++)
				{
					if (filter[i] == L'|')
					{
						second = i;
						break;
					}
				}

				auto filterItem = Ptr(new FileDialogFilter);
				filterItem->name = filter.Sub(filterStart, first - filterStart);
				filterItem->filter = filter.Sub(first + 1, (second == -1 ? count : second) - first - 1);
				vm->filters.Add(filterItem);

				if (second == -1) break;
				filterStart = second + 1;
			}

			if (vm->filters.Count() > 0)
			{
				if (selectionFilterIndex < 0 || vm->filters.Count() <= selectionFilterIndex)
				{
					selectionFilterIndex = 0;
				}
				vm->selectedFilter = vm->filters[selectionFilterIndex].Cast<FileDialogFilter>();
			}

			vm->rootFolder = Ptr(new FileDialogFolder);
			vm->rootFolder->type = FileDialogFolderType::Root;

			// TODO: initialDirectory -> selectedFolder
			vm->SetSelectedFolder(vm->rootFolder);

			switch (dialogType)
			{
			case INativeDialogService::FileDialogOpen:
			case INativeDialogService::FileDialogOpenPreview:
				{
					auto owner = GetApplication()->GetWindowFromNative(window);
					auto dialog = CreateOpenFileDialog(vm);
					ShowModalDialogAndDelete(vm, owner, dialog);
				}
				break;
			case INativeDialogService::FileDialogSave:
			case INativeDialogService::FileDialogSavePreview:
				{
					auto owner = GetApplication()->GetWindowFromNative(window);
					auto dialog = CreateSaveFileDialog(vm);
					ShowModalDialogAndDelete(vm, owner, dialog);
				}
				break;
			}

			if (vm->filters.Count() > 0)
			{
				selectionFilterIndex = vm->filters.IndexOf(vm->selectedFilter.Obj());
			}
			return vm->confirmed;
		}
	}
}