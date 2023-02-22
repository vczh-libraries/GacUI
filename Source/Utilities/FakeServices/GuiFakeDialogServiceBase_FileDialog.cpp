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
		public:
			FileDialogFolder*			parent = nullptr;
			FileDialogFolderType		type = FileDialogFolderType::Placeholder;
			filesystem::Folder			folder;
			WString						name;
			Folders						children;

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

			bool						isLoadingFile = false;
			Files						files;

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
						RefreshFiles();
					}
				}
			}

			bool GetIsLoadingFiles() override
			{
				return isLoadingFile;
			}

			Files& GetFiles() override
			{
				return files;
			}

			void RefreshFiles() override
			{
				if (!isLoadingFile)
				{
					isLoadingFile = true;
					IsLoadingFilesChanged();

					{
						auto folder = Ptr(new FileDialogFolder);
						folder->parent = rootFolder.Obj();
						folder->name = textLoadingFolders;
						rootFolder->children.Clear();
						rootFolder->children.Add(folder);
					}
					{
						auto file = Ptr(new FileDialogFile);
						file->name = textLoadingFiles;
						files.Clear();
						files.Add(file);
					}
				}
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