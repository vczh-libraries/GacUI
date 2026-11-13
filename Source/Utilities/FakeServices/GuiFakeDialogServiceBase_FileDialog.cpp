#include "GuiFakeDialogServiceBase.h"
#include "../../Application/Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace regex;
		using namespace controls;

/***********************************************************************
View Model (IFileDialogFilter)
***********************************************************************/

		class FileDialogFilter : public Object, public virtual IFileDialogFilter
		{
		public:
			WString						name;
			WString						filter;
			Nullable<WString>			defaultExtension;
			Ptr<Regex>					regexFilter;

			WString GetName() override
			{
				return name;
			}

			WString GetFilter() override
			{
				return filter;
			}

			Nullable<WString> GetDefaultExtension() override
			{
				return defaultExtension;
			}

			bool FilterFile(Ptr<IFileDialogFile> file) override
			{
				if (file->GetType() != FileDialogFileType::File) return true;
				auto name = file->GetName();
				auto match = regexFilter->MatchHead(name);
				return match && match->Result().Length() == name.Length();
			}
		};

/***********************************************************************
FileDialogTaskQueue
***********************************************************************/

		class FileDialogTaskQueue
			: public Object
			, public Description<FileDialogTaskQueue>
		{
		protected:
			SpinLock						lock;
			List<Func<void()>>				tasks;
			bool							executing = false;

			void Execute()
			{
				auto self = Ptr(this);
				GetApplication()->InvokeAsync([self]()
				{
					while (true)
					{
						Func<void()> task;
						bool completed = false;
						SPIN_LOCK(self->lock)
						{
							if (self->tasks.Count() == 0)
							{
								self->executing = false;
								completed = true;
							}
							else
							{
								task = self->tasks[0];
								self->tasks.RemoveAt(0);
							}
						}
						if (completed) return;
						task();
					}
				});
			}

		public:
			void Queue(const Func<void()>& task)
			{
				bool execute = false;
				SPIN_LOCK(lock)
				{
					tasks.Add(task);
					if (!executing)
					{
						executing = true;
						execute = true;
					}
				}
				if (execute)
				{
					Execute();
				}
			}
		};

/***********************************************************************
View Model (IFileDialogFolder)
***********************************************************************/

		class FileDialogFolder : public Object, public virtual IFileDialogFolder
		{
			using FolderMap = Dictionary<WString, Ptr<FileDialogFolder>>;
		protected:
			Ptr<FileDialogTaskQueue>		taskQueue;
			vint						taskId = 0;
			bool						taskFired = false;
			bool						enumerationRequested = false;

		public:
			FileDialogFolder*			parent = nullptr;
			FileDialogFolderType		type = FileDialogFolderType::Placeholder;
			filesystem::Folder			folder;
			WString						name;
			vint						index = -1;
			WString						textLoadingFolders;

			// the placeholder node will always be the last one
			Folders						children;
			FolderMap					childrenByName;

			void AddChild(Ptr<FileDialogFolder> child)
			{
				child->textLoadingFolders = textLoadingFolders;
				child->parent = this;
				if (child->type == FileDialogFolderType::Folder)
				{
					childrenByName.Add(child->name, child);
				}
				child->index = HasPlaceholderChild() ? children.Count() - 1 : children.Count();
				if (HasPlaceholderChild()) children[children.Count() - 1].Cast<FileDialogFolder>()->index = children.Count();
				children.Insert(child->index, child);
			}

			bool HasPlaceholderChild()
			{
				return children.Count() > 0 && children[children.Count() - 1].Cast<FileDialogFolder>()->type == FileDialogFolderType::Placeholder;
			}

			void AddPlaceholderChild()
			{
				if (HasPlaceholderChild()) return;
				CHECK_ERROR(textLoadingFolders.Length() > 0, L"vl::presentation::FileDialogFolder::AddPlaceholderChild()#textLoadingFolders is not initialized.");
				auto child = Ptr(new FileDialogFolder(taskQueue));
				child->name = textLoadingFolders;
				AddChild(child);
			}

			void RemovePlaceholderChild()
			{
				if (HasPlaceholderChild())
				{
					children[children.Count() - 1].Cast<FileDialogFolder>()->parent = nullptr;
					children[children.Count() - 1].Cast<FileDialogFolder>()->index = -1;
					children.RemoveAt(children.Count() - 1);
				}
			}

			void InvalidateTasks()
			{
				++taskId;
				for (auto child : children)
				{
					auto folder = child.Cast<FileDialogFolder>();
					folder->taskFired = false;
					folder->InvalidateTasks();
				}
			}

			void ReconcileChildren(const List<filesystem::Folder>& folders)
			{
				++taskId;
				enumerationRequested = true;
				RemovePlaceholderChild();
				SortedList<WString> names;
				for (auto folder : folders)
				{
					names.Add(folder.GetFilePath().GetName());
				}
				for (vint i = children.Count() - 1; i >= 0; i--)
				{
					auto child = children[i].Cast<FileDialogFolder>();
					if (!names.Contains(child->name))
					{
						child->InvalidateTasks();
						child->parent = nullptr;
						child->index = -1;
						childrenByName.Remove(child->name);
						for (vint j = i + 1; j < children.Count(); j++)
						{
							children[j].Cast<FileDialogFolder>()->index = j - 1;
						}
						children.RemoveAt(i);
					}
				}
				for (auto folder : folders)
				{
					if (!childrenByName.Keys().Contains(folder.GetFilePath().GetName()))
					{
						auto child = Ptr(new FileDialogFolder(folder, taskQueue));
						AddChild(child);
						child->AddPlaceholderChild();
					}
				}
			}

			void RefreshChildren(bool recursively, const Func<void()>& refreshed = {})
			{
				enumerationRequested = true;
				auto taskFolder = Ptr(this);
				vint taskFolderId = ++taskId;
				taskQueue->Queue([taskFolder, taskFolderId, refreshed]()
				{
					auto subFolders = Ptr(new List<filesystem::Folder>);
					if (!taskFolder->folder.GetFolders(*subFolders.Obj()))
					{
						subFolders->Clear();
					}
					GetApplication()->InvokeInMainThread(nullptr, [taskFolder, taskFolderId, subFolders, refreshed]()
					{
						if (taskFolder->taskId == taskFolderId)
						{
							taskFolder->ReconcileChildren(*subFolders.Obj());
							if (refreshed) refreshed();
						}
					});
				});
				if (recursively)
				{
					for (auto child : children)
					{
						auto folder = child.Cast<FileDialogFolder>();
						if (folder->type == FileDialogFolderType::Folder && folder->enumerationRequested)
						{
							folder->RefreshChildren(true, refreshed);
						}
					}
				}
			}

			FileDialogFolder(Ptr<FileDialogTaskQueue> _taskQueue)
				: taskQueue(_taskQueue)
			{
			}

			FileDialogFolder(const filesystem::Folder& _folder, Ptr<FileDialogTaskQueue> _taskQueue)
				: taskQueue(_taskQueue)
				, type(FileDialogFolderType::Folder)
				, folder(_folder)
				, name(_folder.GetFilePath().GetName())
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
					if (parent) parent->RefreshChildren(false);
				}
				return type;
			}

			WString GetFullPath() override
			{
				return folder.GetFilePath().GetFullPath();
			}

			WString GetName() override
			{
				// Text-only tree rows must also start visible placeholder enumeration.
				GetType();
				return name;
			}

			vint GetIndex() override
			{
				return index;
			}

			Folders& GetFolders() override
			{
				return children;
			}

			Ptr<IFileDialogFolder> TryGetFolder(const WString& name) override
			{
				vint index = childrenByName.Keys().IndexOf(name);
				if (index == -1) return {};
				return childrenByName.Values()[index];
			}
		};

/***********************************************************************
View Model (IFileDialogFile)
***********************************************************************/

		class FileDialogFile : public Object, public virtual IFileDialogFile
		{
		public:
			FileDialogFileType			type = FileDialogFileType::Placeholder;
			Ptr<FileDialogFolder>		associatedFolder;
			WString						name;
			filesystem::FilePath		path;

			FileDialogFileType GetType() override
			{
				return type;
			}

			Ptr<IFileDialogFolder> GetAssociatedFolder() override
			{
				return associatedFolder;
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
			using ConfirmedSelection = collections::List<WString>;
		protected:
			WString						textLoadingFolders;
			WString						textLoadingFiles;
			WString						dialogErrorEmptySelection;
			WString						dialogErrorFileNotExist;
			WString						dialogErrorFileExpected;
			WString						dialogErrorFolderNotExist;
			WString						dialogErrorMultipleSelectionNotEnabled;
			WString						dialogAskCreateFile;
			WString						dialogAskOverrideFile;

			bool						isLoadingFiles = false;
			bool						filesAvailable = false;
			bool						isMutatingFiles = false;
			filesystem::FilePath		loadingPath;
			vint						loadingTaskId = 0;
			Files						files;
			Regex						regexDisplayString{ L";" };
			Ptr<FileDialogTaskQueue>		taskQueue = Ptr(new FileDialogTaskQueue);

		public:
			FileDialogViewModel()
			{
				rootFolder = Ptr(new FileDialogFolder(taskQueue));
				rootFolder->type = FileDialogFolderType::Root;
			}

			FakeDialogServiceBase*      dialogService = nullptr;
			bool						active = true;

			bool						selectToSave = false;
			bool						confirmed = false;
			ConfirmedSelection			confirmedSelection;

			WString						title;
			bool						enabledMultipleSelection = false;
			bool						fileMustExist = false;
			bool						folderMustExist = false;
			bool						promptCreateFile = false;
			bool						promptOverriteFile = false;
			WString						defaultExtension;

			Filters						filters;
			Ptr<FileDialogFilter>		selectedFilter;

			WString						initialDirectory;
			Ptr<FileDialogFolder>		rootFolder;
			Ptr<FileDialogFolder>		selectedFolder;

			void Deactivate()
			{
				active = false;
				++loadingTaskId;
				rootFolder->InvalidateTasks();
			}

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
						SelectedFilterChanged();
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

			void SetSelectedFolderInternal(Ptr<IFileDialogFolder> value, bool refreshFile)
			{
				if (auto folder = value.Cast<FileDialogFolder>())
				{
					if (selectedFolder != folder)
					{
						selectedFolder = folder;
						SelectedFolderChanged();
						FileOperationsChanged();

						if (refreshFile && loadingPath != selectedFolder->folder.GetFilePath())
						{
							RefreshFiles();
						}
					}
				}
			}

			void SetSelectedFolderInternal(filesystem::FilePath path, bool refreshFile)
			{
				List<filesystem::FilePath> fragments;
				while (!path.IsRoot())
				{
					fragments.Add(path);
					path = path.GetFolder();
				}

				auto folder = rootFolder;
				// TODO: (enumerable) foreach:reversed
				for (vint i = fragments.Count() - 1; i >= 0; i--)
				{
					auto fragment = fragments[i];
					vint index = folder->childrenByName.Keys().IndexOf(fragment.GetName());
					if (index == -1)
					{
						auto child = Ptr(new FileDialogFolder(fragment, taskQueue));
						folder->AddChild(child);
						child->AddPlaceholderChild();
						folder = child;
					}
					else
					{
						folder = folder->childrenByName.Values()[index];
					}
				}

				SetSelectedFolderInternal(folder, refreshFile);
			}

			void SetSelectedFolder(Ptr<IFileDialogFolder> value) override
			{
				SetSelectedFolderInternal(value, true);
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
				if (!active || !selectedFolder) return;

				vint taskId = ++loadingTaskId;
				auto taskFolder = selectedFolder;
				auto taskPath = (loadingPath = selectedFolder->folder.GetFilePath());
				filesAvailable = false;
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
				FileOperationsChanged();

				auto vm = Ptr(this);
				taskQueue->Queue([taskId, taskFolder, taskPath, vm]()
				{
					auto folders = Ptr(new List<filesystem::Folder>);
					auto files = Ptr(new List<filesystem::File>);
					filesystem::Folder loadingFolder = taskPath;
					auto available = loadingFolder.GetFolders(*folders.Obj()) && loadingFolder.GetFiles(*files.Obj());
					if (!available)
					{
						folders->Clear();
						files->Clear();
					}

					GetApplication()->InvokeInMainThread(nullptr, [taskId, taskFolder, folders, files, available, vm]()
					{
						if (vm->active && vm->loadingTaskId == taskId && vm->selectedFolder == taskFolder)
						{
							vm->files.Clear();
							taskFolder->ReconcileChildren(*folders.Obj());

							for (auto folder : *folders.Obj())
							{
								auto item = Ptr(new FileDialogFile);
								item->type = FileDialogFileType::Folder;
								item->name = folder.GetFilePath().GetName();
								item->path = folder.GetFilePath();
								item->associatedFolder = taskFolder->childrenByName[item->name];

								vm->files.Add(item);
							}

							for (auto file : *files.Obj())
							{
								auto item = Ptr(new FileDialogFile);
								item->type = FileDialogFileType::File;
								item->name = file.GetFilePath().GetName();
								item->path = file.GetFilePath();
								vm->files.Add(item);
							}

							vm->isLoadingFiles = false;
							vm->filesAvailable = available;
							vm->SelectedFolderChanged();
							vm->IsLoadingFilesChanged();
							vm->FileOperationsChanged();
						}
					});
				});
			}

			bool GetCanModifyFolder() override
			{
				return active && filesAvailable && !isLoadingFiles && !isMutatingFiles
					&& selectedFolder && selectedFolder->type != FileDialogFolderType::Placeholder
					&& selectedFolder->folder.GetFilePath() == loadingPath
					&& loadingPath.GetFullPath().Length() > 0;
			}

			bool GetIsMutatingFiles() override
			{
				return isMutatingFiles;
			}

			bool CanDeleteFiles(LazyList<Ptr<IFileDialogFile>> selection) override
			{
				if (!GetCanModifyFolder()) return false;
				bool selected = false;
				for (auto item : selection)
				{
					auto file = item.Cast<FileDialogFile>();
					if (!file || !files.Contains(file.Obj()) || file->type == FileDialogFileType::Placeholder
						|| file->path.GetFolder() != loadingPath || file->path.IsRoot()
						|| file->path.GetFolder().GetFullPath().Length() == 0)
					{
						return false;
					}
					selected = true;
				}
				return selected;
			}

			void RefreshAfterMutation(Ptr<FileDialogFolder> folder)
			{
				auto vm = Ptr(this);
				folder->RefreshChildren(true, [vm]()
				{
					if (vm->active) vm->SelectedFolderChanged();
				});
				auto path = selectedFolder->folder.GetFilePath();
				while (true)
				{
					if (path == folder->folder.GetFilePath())
					{
						RefreshFiles();
						break;
					}
					if (path.IsRoot()) break;
					path = path.GetFolder();
				}
			}

			void CreateFolder(
				GuiWindow* owner,
				Ptr<IFileDialogFolder> folder,
				const WString& name,
				const WString& alreadyExistsMessage,
				const WString& failureMessage,
				const Func<void(bool)>& callback
			) override
			{
				if (!GetCanModifyFolder() || folder != selectedFolder) return;
				auto taskFolder = selectedFolder;
				auto path = taskFolder->folder.GetFilePath();
				auto ownerFlag = owner->GetDisposedFlag();
				auto vm = Ptr(this);
				isMutatingFiles = true;
				FileOperationsChanged();

				taskQueue->Queue([vm, taskFolder, path, name, owner, ownerFlag, alreadyExistsMessage, failureMessage, callback]()
				{
					bool created = false;
					bool alreadyExists = false;
					Nullable<filesystem::FilePath> candidate;
					try
					{
						bool valid = name.Length() > 0 && name != L"." && name != L"..";
						auto delimiters = filesystem::FilePath::GetCompatibleDelimiters();
						for (vint i = 0; i < name.Length(); i++)
						{
							auto c = name[i];
							if (c == filesystem::FilePath::GetPathDelimiter() || delimiters.IndexOf(c) != -1 || c == L'\0')
							{
								valid = false;
							}
						}
						if (valid && !filesystem::FilePath::IsAbsolutePath(name))
						{
							auto child = path / name;
							if (child != path && child.GetFolder() == path)
							{
								candidate = child;
							}
						}
					}
					catch (const ArgumentException&)
					{
						// Invalid or overlong user input can fail before Folder::Create.
					}
					if (candidate)
					{
						filesystem::Folder child(candidate.Value());
						alreadyExists = child.Exists();
						if (!alreadyExists) created = child.Create(false);
					}
					GetApplication()->InvokeInMainThread(nullptr, [vm, taskFolder, owner, ownerFlag, alreadyExistsMessage, failureMessage, callback, created, alreadyExists]()
					{
						if (!vm->active) return;
						if (created)
						{
							taskFolder->InvalidateTasks();
							vm->RefreshAfterMutation(taskFolder);
						}
						else if (!ownerFlag->IsDisposed())
						{
							vm->dialogService->ShowMessageBox(
								owner->GetNativeWindow(),
								alreadyExists ? alreadyExistsMessage : failureMessage,
								owner->GetText(),
								INativeDialogService::DisplayOK,
								INativeDialogService::DefaultFirst,
								INativeDialogService::IconError,
								INativeDialogService::ModalWindow
							);
						}
						vm->isMutatingFiles = false;
						if (!vm->active) return;
						vm->FileOperationsChanged();
						if (!ownerFlag->IsDisposed()) callback(created);
					});
				});
			}

			static bool CanDeleteDirectory(const filesystem::Folder& folder)
			{
				List<filesystem::Folder> pending;
				pending.Add(folder);
				while (pending.Count() > 0)
				{
					auto current = pending[pending.Count() - 1];
					pending.RemoveAt(pending.Count() - 1);
					filesystem::FileInfo info;
					try
					{
						info = current.GetFileInfo();
					}
					catch (const Error&)
					{
						// The metadata API throws when an entry disappears or cannot be inspected.
						return false;
					}
					if (info.isSymbolicLink || info.isReparsePoint) return false;
					List<filesystem::Folder> folders;
					List<filesystem::File> files;
					try
					{
						if (!current.GetFolders(folders) || !current.GetFiles(files)) return false;
					}
					catch (const ArgumentException&)
					{
						// A descendant path that cannot be represented makes traversal incomplete.
						return false;
					}
					for (auto child : folders) pending.Add(child);
				}
				return true;
			}

			void DeleteFiles(
				GuiWindow* owner,
				LazyList<Ptr<IFileDialogFile>> selection,
				const Func<WString(const WString&, const WString&)>& confirmationMessage,
				const Func<WString(const WString&, const WString&)>& failureMessage
			) override
			{
				auto selected = Ptr(new List<Ptr<IFileDialogFile>>);
				CopyFrom(*selected.Obj(), selection);
				if (!CanDeleteFiles(From(*selected.Obj()))) return;
				struct Target
				{
					filesystem::FilePath	path;
					WString					name;
					FileDialogFileType		type;
				};
				auto targets = Ptr(new List<Target>);
				for (auto selectedFile : *selected.Obj())
				{
					auto file = selectedFile.Cast<FileDialogFile>();
					targets->Add({ file->path,file->name,file->type });
				}
				auto taskFolder = selectedFolder;
				auto path = taskFolder->folder.GetFilePath();
				auto names = stream::GenerateToStream([&](stream::TextWriter& writer)
				{
					for (auto [target, index] : indexed(*targets.Obj()))
					{
						if (index > 0) writer.WriteString(L"\n");
						writer.WriteString(target.name);
					}
				});
				auto ownerFlag = owner->GetDisposedFlag();
				isMutatingFiles = true;
				FileOperationsChanged();
				auto answer = dialogService->ShowMessageBox(
					owner->GetNativeWindow(),
					confirmationMessage(path.GetFullPath(), names),
					owner->GetText(),
					INativeDialogService::DisplayOKCancel,
					INativeDialogService::DefaultSecond,
					INativeDialogService::IconQuestion,
					INativeDialogService::ModalWindow
				);
				if (!active || ownerFlag->IsDisposed()) return;
				if (answer != INativeDialogService::SelectOK)
				{
					isMutatingFiles = false;
					FileOperationsChanged();
					return;
				}

				taskFolder->InvalidateTasks();
				if (selectedFolder == taskFolder) ++loadingTaskId;
				auto vm = Ptr(this);
				taskQueue->Queue([vm, taskFolder, path, targets, owner, ownerFlag, failureMessage]()
				{
					auto failures = stream::GenerateToStream([&](stream::TextWriter& writer)
					{
						bool first = true;
						for (auto target : *targets.Obj())
						{
							bool deleted = false;
							if (target.path.GetFolder() == path && !target.path.IsRoot() && path.GetFullPath().Length() > 0)
							{
								if (target.type == FileDialogFileType::File)
								{
									deleted = filesystem::File(target.path).Delete();
								}
								else
								{
									filesystem::Folder folder(target.path);
									deleted = CanDeleteDirectory(folder) && folder.Delete(true);
								}
							}
							if (!deleted)
							{
								if (!first) writer.WriteString(L"\n");
								writer.WriteString(target.name);
								first = false;
							}
						}
					});
					GetApplication()->InvokeInMainThread(nullptr, [vm, taskFolder, path, failures, owner, ownerFlag, failureMessage]()
					{
						if (!vm->active) return;
						taskFolder->InvalidateTasks();
						vm->RefreshAfterMutation(taskFolder);
						if (!ownerFlag->IsDisposed() && failures.Length() > 0)
						{
							vm->dialogService->ShowMessageBox(
								owner->GetNativeWindow(),
								failureMessage(path.GetFullPath(), failures),
								owner->GetText(),
								INativeDialogService::DisplayOK,
								INativeDialogService::DefaultFirst,
								INativeDialogService::IconError,
								INativeDialogService::ModalWindow
							);
						}
						vm->isMutatingFiles = false;
						if (vm->active && !ownerFlag->IsDisposed()) vm->FileOperationsChanged();
					});
				});
			}

			WString GetDisplayString(collections::LazyList<Ptr<IFileDialogFile>> files) override
			{
				return From(files)
					.FindType<FileDialogFile>()
					.Where([](auto file) { return file->type != FileDialogFileType::Placeholder; })
					.Select([](auto file) { return WString::Unmanaged(L"\"") + file->name + WString::Unmanaged(L"\""); })
					.Aggregate(WString::Empty, [](auto&& a, auto&& b)
					{
						return a == WString::Empty ? b : a + WString::Unmanaged(L";") + b;
					});
			}

			Selection ParseDisplayString(const WString& displayString) override
			{
				auto items = Ptr(new RegexMatch::List);
				regexDisplayString.Split(displayString, false, *items.Obj());
				return From(LazyList<Ptr<RegexMatch>>(items))
					.Select([](auto item) { return item->Result().Value(); })
					.Select([](auto item)
					{
						if (item.Length() >= 2 && item[0] == L'\"' && item[item.Length() - 1] == L'\"')
						{
							return item.Sub(1, item.Length() - 2);
						}
						else
						{
							return item;
						}
					});
			}

			bool TryConfirm(controls::GuiWindow* owner, Selection selectedPaths) override
			{
				if (!active || isMutatingFiles) return false;
				CHECK_ERROR(dialogService, L"vl::presentation::FileDialogViewModel::TryConfirm()#dialogService is not initialized.");
				auto wd = selectedFolder->folder.GetFilePath();
				List<filesystem::FilePath> rawPaths;
				CopyFrom(
					rawPaths,
					selectedPaths.Select([wd](auto path)
					{
						return filesystem::FilePath::IsAbsolutePath(path) ? filesystem::FilePath(path) : wd / path;
					})
					);

				if (rawPaths.Count() == 0)
				{
					dialogService->ShowMessageBox(
						owner ? owner->GetNativeWindow() : nullptr,
						dialogErrorEmptySelection,
						owner->GetText(),
						INativeDialogService::DisplayOK,
						INativeDialogService::DefaultFirst,
						INativeDialogService::IconError,
						INativeDialogService::ModalWindow
						);
					return false;
				}

				// folder navigation must use the raw selection (no extension append)
				if (rawPaths.Count() == 1)
				{
					auto path = rawPaths[0];
					if (filesystem::Folder(path).Exists())
					{
						SetSelectedFolderInternal(path, true);
						return false;
					}
				}
				else if (!enabledMultipleSelection)
				{
					dialogService->ShowMessageBox(
						owner ? owner->GetNativeWindow() : nullptr,
						dialogErrorMultipleSelectionNotEnabled,
						owner->GetText(),
						INativeDialogService::DisplayOK,
						INativeDialogService::DefaultFirst,
						INativeDialogService::IconError,
						INativeDialogService::ModalWindow
						);
					return false;
				}

				Nullable<WString> extension;
				bool extensionFromFilter = false;
				if (selectedFilter)
				{
					extension = selectedFilter->GetDefaultExtension();
					extensionFromFilter = extension;
				}

				if (!extensionFromFilter && defaultExtension != WString::Empty)
				{
					// Only use the dialog default extension if it matches the current filter
					if (selectedFilter && selectedFilter->regexFilter)
					{
						auto testName = WString::Unmanaged(L"x.") + defaultExtension;
						auto match = selectedFilter->regexFilter->MatchHead(testName);
						if (match && match->Result().Length() == testName.Length())
						{
							extension = defaultExtension;
						}
					}
					else
					{
						extension = defaultExtension;
					}
				}

				auto normalized = [&](filesystem::FilePath path)
				{
					if (!extension) return path;

					auto&& sExt = WString::Unmanaged(L".") + extension.Value();
					vint lExt = sExt.Length();
					auto full = path.GetFullPath();

					auto selectedFileName = path.GetName();
					if (selectedFilter && selectedFilter->regexFilter)
					{
						auto match = selectedFilter->regexFilter->MatchHead(selectedFileName);
						if (match && match->Result().Length() == selectedFileName.Length())
						{
							return path;
						}
					}
					if (INVLOC.FindFirst(selectedFileName, WString::Unmanaged(L"."), Locale::None).key != -1)
					{
						return path;
					}
					return filesystem::FilePath(full + sExt);
				};

				List<filesystem::FilePath> paths;
				CopyFrom(paths, From(rawPaths).Select([&](auto path) { return normalized(path); }));

				List<vint> files, folders, unexistings;
				for (auto [path, index] : indexed(paths))
				{
					if (filesystem::File(path).Exists())
					{
						files.Add(index);
					}
					else if (filesystem::Folder(path).Exists())
					{
						folders.Add(index);
					}
					else
					{
						unexistings.Add(index);
					}
				}

				if (folders.Count() > 0)
				{
					auto message = stream::GenerateToStream([&](stream::TextWriter& writer)
					{
						writer.WriteString(dialogErrorFileExpected);
						for (vint index : folders)
						{
							writer.WriteLine(WString::Empty);
							writer.WriteString(L"  ");
							writer.WriteString(wd.GetRelativePathFor(paths[index]));
						}
					});
					dialogService->ShowMessageBox(
						owner ? owner->GetNativeWindow() : nullptr,
						message,
						owner->GetText(),
						INativeDialogService::DisplayOK,
						INativeDialogService::DefaultFirst,
						INativeDialogService::IconError,
						INativeDialogService::ModalWindow
						);
					return false;
				}

				if (unexistings.Count() > 0)
				{
					if (fileMustExist)
					{
						auto message = stream::GenerateToStream([&](stream::TextWriter& writer)
						{
							writer.WriteString(dialogErrorFileNotExist);
							for (vint index : unexistings)
							{
								writer.WriteLine(WString::Empty);
								writer.WriteString(L"  ");
								writer.WriteString(wd.GetRelativePathFor(paths[index]));
							}
						});
						dialogService->ShowMessageBox(
							owner ? owner->GetNativeWindow() : nullptr,
							message,
							owner->GetText(),
							INativeDialogService::DisplayOK,
							INativeDialogService::DefaultFirst,
							INativeDialogService::IconError,
							INativeDialogService::ModalWindow
							);
						return false;
					}

					if (folderMustExist)
					{
						SortedList<filesystem::FilePath> folderOfUnexistings;
						for (vint index : unexistings)
						{
							auto path = paths[index].GetFolder();
							if(!folderOfUnexistings.Contains(path))
							{
								folderOfUnexistings.Add(path);
							}
						}

						// TODO: (enumerable) foreach:indexed(alterable(reversed))
						for (vint i = folderOfUnexistings.Count() - 1; i >= 0; i--)
						{
							if (filesystem::Folder(folderOfUnexistings[i]).Exists())
							{
								folderOfUnexistings.RemoveAt(i);
							}
						}

						if (folderOfUnexistings.Count() > 0)
						{
							auto message = stream::GenerateToStream([&](stream::TextWriter& writer)
							{
								writer.WriteString(dialogErrorFolderNotExist);
								for (auto path : folderOfUnexistings)
								{
									writer.WriteLine(WString::Empty);
									writer.WriteString(L"  ");
									writer.WriteString(wd.GetRelativePathFor(path));
								}
							});
							dialogService->ShowMessageBox(
								owner ? owner->GetNativeWindow() : nullptr,
								message,
								owner->GetText(),
								INativeDialogService::DisplayOK,
								INativeDialogService::DefaultFirst,
								INativeDialogService::IconError,
								INativeDialogService::ModalWindow
								);
							return false;
						}
					}
				}

				WString questionMessage;
				List<vint>* questionFiles = nullptr;
				if (selectToSave && promptOverriteFile)
				{
					questionMessage = dialogAskOverrideFile;
					questionFiles = &files;
				}
				if (!selectToSave && promptCreateFile && unexistings.Count() > 0)
				{
					questionMessage = dialogAskCreateFile;
					questionFiles = &unexistings;
				}

				if (questionFiles && questionFiles->Count() > 0)
				{
					auto message = stream::GenerateToStream([&](stream::TextWriter& writer)
					{
						writer.WriteString(questionMessage);
						for (vint index : *questionFiles)
						{
							writer.WriteLine(WString::Empty);
							writer.WriteString(L"  ");
							writer.WriteString(wd.GetRelativePathFor(paths[index]));
						}
					});

					auto result = dialogService->ShowMessageBox(
						owner ? owner->GetNativeWindow() : nullptr,
						message,
						owner->GetText(),
						INativeDialogService::DisplayOKCancel,
						INativeDialogService::DefaultThird,
						INativeDialogService::IconQuestion,
						INativeDialogService::ModalWindow
						);

					if (result == INativeDialogService::SelectCancel)
					{
						return false;
					}
				}

				CopyFrom(
					confirmedSelection,
					From(paths).Select([](auto path) { return path.GetFullPath(); })
					);

				confirmed = true;
				return true;
			}

			void InitLocalizedText(
				const WString& _textLoadingFolders,
				const WString& _textLoadingFiles,
				const WString& _dialogErrorEmptySelection,
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
				dialogErrorEmptySelection = _dialogErrorEmptySelection;
				dialogErrorFileNotExist = _dialogErrorFileNotExist;
				dialogErrorFileExpected = _dialogErrorFileExpected;
				dialogErrorFolderNotExist = _dialogErrorFolderNotExist;
				dialogErrorMultipleSelectionNotEnabled = _dialogErrorMultipleSelectionNotEnabled;
				dialogAskCreateFile = _dialogAskCreateFile;
				dialogAskOverrideFile = _dialogAskOverrideFile;

				rootFolder->textLoadingFolders = textLoadingFolders;
				rootFolder->AddPlaceholderChild();

				if (initialDirectory != WString::Empty)
				{
					filesystem::FilePath path = initialDirectory;
					if (filesystem::Folder(path).Exists())
					{
						SetSelectedFolderInternal(path, false);
						return;
					}
				}
				SetSelectedFolderInternal(rootFolder, false);
			}
		};

/***********************************************************************
FakeDialogServiceBase
***********************************************************************/

		void FakeDialogServiceBase::ParseFileDialogFilter(const WString& filter, collections::List<FilterDesc>& descs)
		{
			auto GetSingleWildcardDefaultExtension = [](const WString& wildcard) -> Nullable<WString>
			{
				// Accept only "*.<ext>" where <ext> is non-empty and contains no '*', '?', or ';'.
				if (wildcard.Length() < 3) return {};
				if (wildcard[0] != L'*' || wildcard[1] != L'.') return {};
				auto ext = wildcard.Right(wildcard.Length() - 2);
				for (vint i = 0; i < ext.Length(); i++)
				{
					switch (ext[i])
					{
					case L'*':
					case L'?':
					case L';':
						return {};
					}
				}
				return Nullable<WString>(ext);
			};

			auto GetFilterDefaultExtension = [&](const WString& wildcardExpr) -> Nullable<WString>
			{
				// Split by ';' and return the first qualifying extension
				vint start = 0;
				while (start <= wildcardExpr.Length())
				{
					vint semicolonPos = -1;
					for (vint i = start; i < wildcardExpr.Length(); i++)
					{
						if (wildcardExpr[i] == L';')
						{
							semicolonPos = i;
							break;
						}
					}
					auto part = (semicolonPos == -1)
						? wildcardExpr.Right(wildcardExpr.Length() - start)
						: wildcardExpr.Sub(start, semicolonPos - start);
					auto ext = GetSingleWildcardDefaultExtension(part);
					if (ext) return ext;
					if (semicolonPos == -1) break;
					start = semicolonPos + 1;
				}
				return {};
			};

			Regex regexWildcard(L"[*?;]");
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

				FilterDesc desc;
				desc.name = filter.Sub(filterStart, first - filterStart);
				desc.filter = filter.Sub(first + 1, (second == -1 ? count : second) - first - 1);
				desc.defaultExtensionOverride = GetFilterDefaultExtension(desc.filter);
				desc.regexFilterCode = stream::GenerateToStream([&](stream::TextWriter& writer)
				{
					writer.WriteString(L"^(");
					List<Ptr<RegexMatch>> matches;
					regexWildcard.Cut(desc.filter, false, matches);
					for (auto match : matches)
					{
						if (match->Success())
						{
							auto wildcard = match->Result().Value()[0];
							switch (wildcard)
							{
							case L'*':
								writer.WriteString(WString::Unmanaged(L"/.*"));
								break;
							case L'?':
								writer.WriteString(WString::Unmanaged(L"/."));
								break;
							case L';':
								writer.WriteString(WString::Unmanaged(L"|"));
								break;
							}
						}
						else
						{
							writer.WriteString(u32tow(regex_internal::EscapeTextForRegex(wtou32(match->Result().Value()))));
						}
					}
					writer.WriteString(L")$");
				});
				desc.regexFilter = Ptr(new Regex(desc.regexFilterCode));
				descs.Add(std::move(desc));

				if (second == -1) break;
				filterStart = second + 1;
			}
		}

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
			vm->dialogService = this;
			vm->title = title;
			vm->enabledMultipleSelection = (options & INativeDialogService::FileDialogAllowMultipleSelection) != 0;
			vm->fileMustExist = (options & INativeDialogService::FileDialogFileMustExist) != 0;
			vm->folderMustExist = (options & INativeDialogService::FileDialogDirectoryMustExist) != 0;
			vm->promptCreateFile = (options & INativeDialogService::FileDialogPromptCreateFile) != 0;
			vm->promptOverriteFile = (options & INativeDialogService::FileDialogPromptOverwriteFile) != 0;
			vm->defaultExtension = defaultExtension;

			List<FilterDesc> filterDescs;
			ParseFileDialogFilter(filter, filterDescs);
			for (auto&& desc : filterDescs)
			{
				auto filterItem = Ptr(new FileDialogFilter);
				filterItem->name = desc.name;
				filterItem->filter = desc.filter;
				filterItem->defaultExtension = desc.defaultExtensionOverride;
				filterItem->regexFilter = desc.regexFilter;
				vm->filters.Add(filterItem);
			}

			if (vm->filters.Count() > 0)
			{
				if (selectionFilterIndex < 0 || vm->filters.Count() <= selectionFilterIndex)
				{
					selectionFilterIndex = 0;
				}
				vm->selectedFilter = vm->filters[selectionFilterIndex].Cast<FileDialogFilter>();
			}

			vm->initialDirectory = initialDirectory;

			switch (dialogType)
			{
			case INativeDialogService::FileDialogOpen:
			case INativeDialogService::FileDialogOpenPreview:
				{
					vm->selectToSave = false;
					auto owner = GetApplication()->GetWindowFromNative(window);
					auto dialog = CreateOpenFileDialog(vm, initialFileName);
					ShowModalDialogAndDelete(vm, owner, dialog);
				}
				break;
			case INativeDialogService::FileDialogSave:
			case INativeDialogService::FileDialogSavePreview:
				{
					vm->selectToSave = true;
					auto owner = GetApplication()->GetWindowFromNative(window);
					auto dialog = CreateSaveFileDialog(vm, initialFileName);
					ShowModalDialogAndDelete(vm, owner, dialog);
				}
				break;
			}

			vm->Deactivate();

			if (vm->filters.Count() > 0)
			{
				selectionFilterIndex = vm->filters.IndexOf(vm->selectedFilter.Obj());
			}

			if (vm->confirmed)
			{
				CopyFrom(selectionFileNames, vm->confirmedSelection);
			}
			return vm->confirmed;
		}
	}
}
