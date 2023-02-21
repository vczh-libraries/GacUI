#include "GuiFakeDialogServiceBase.h"
#include "../../Application/Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace compositions;
		using namespace controls;

/***********************************************************************
View Model (IMessageBoxDialogViewModel)
***********************************************************************/

		class FakeMessageBoxDialogViewModel : public Object, public virtual IMessageBoxDialogViewModel
		{
		public:
			class Action : public Object, public virtual IMessageBoxDialogAction
			{
			public:
				FakeMessageBoxDialogViewModel*	viewModel;
				ButtonItem						button;

				Action(FakeMessageBoxDialogViewModel* _viewModel, ButtonItem _button)
					: viewModel(_viewModel)
					, button(_button)
				{
				}

				ButtonItem			GetButton() override					{ return button; }
				void				PerformAction() override				{ viewModel->result = button; }
			};

			WString					text;
			WString					title;
			Icon					icon;
			ButtonItemList			buttons;
			ButtonItem				defaultButton;
			ButtonItem				result;

			WString					GetText() override						{ return text; }
			WString					GetTitle() override						{ return title; }
			Icon					GetIcon() override						{ return icon; }
			const ButtonItemList&	GetButtons() override					{ return buttons; }
			ButtonItem				GetDefaultButton() override				{ return defaultButton; }
			ButtonItem				GetResult() override					{ return result; }
		};

/***********************************************************************
View Model (IColorDialogViewModel)
***********************************************************************/

		class FakeColorDialogViewModel : public Object, public virtual IColorDialogViewModel
		{
		public:
			bool					confirmed = false;
			Color					color;

			bool					GetConfirmed() override					{ return confirmed; }
			void					SetConfirmed(bool value) override		{ confirmed = value; }

			Color					GetColor() override						{ return color; }
			void					SetColor(Color value) override			{ color = value; }
		};

/***********************************************************************
View Model (ISimpleFontDialogViewModel and IFullFontDialogViewModel)
***********************************************************************/

		template<typename IViewModel>
		class FakeFontDialogViewModel : public Object, public virtual IViewModel
		{
		public:
			using FontList = collections::List<WString>;

			bool					confirmed = false;
			bool					fontMustExist = false;
			FontList				fontList;

			bool					GetConfirmed() override					{ return confirmed; }
			void					SetConfirmed(bool value) override		{ confirmed = value; }

			bool					GetFontMustExist() override				{ return fontMustExist; }
			const FontList&			GetFontList() override					{ return fontList; }
		};

		class FakeSimpleFontDialogViewModel : public FakeFontDialogViewModel<ISimpleFontDialogViewModel>
		{
		public:
			WString					fontFamily;
			vint					fontSize = 0;

			WString					GetFontFamily() override						{ return fontFamily; }
			void					SetFontFamily(const WString& value) override	{ fontFamily = value; }

			vint					GetFontSize() override							{ return fontSize; }
			void					SetFontSize(vint value) override				{ fontSize = value; }
		};

		class FakeFullFontDialogViewModel : public FakeFontDialogViewModel<IFullFontDialogViewModel>
		{
		public:
			Color					color;
			FontProperties			font;

			Color					GetColor() override								{ return color; }
			void					SetColor(Color value) override					{ color = value; }

			FontProperties			GetFont() override								{ return font; }
			void					SetFont(const FontProperties& value) override	{ font = value; }
		};

/***********************************************************************
View Model (IFileDialogViewModel)
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

		void FakeDialogServiceBase::ShowModalDialogAndDelete(controls::GuiWindow* owner, controls::GuiWindow* dialog)
		{
			auto app = GetApplication();
			bool exit = false;
			dialog->WindowOpened.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				dialog->ForceCalculateSizeImmediately();
				dialog->MoveToScreenCenter();
			});
			dialog->ShowModalAndDelete(owner, [&exit]() {exit = true; });
			while (!exit && app->RunOneCycle());
		}

		FakeDialogServiceBase::FakeDialogServiceBase()
		{
		}

		FakeDialogServiceBase::~FakeDialogServiceBase()
		{
		}

		FakeDialogServiceBase::MessageBoxButtonsOutput	FakeDialogServiceBase::ShowMessageBox(
			INativeWindow* window,
			const WString& text,
			const WString& title,
			MessageBoxButtonsInput buttons,
			MessageBoxDefaultButton defaultButton,
			MessageBoxIcons icon,
			MessageBoxModalOptions modal
		)
		{
			auto vm = Ptr(new FakeMessageBoxDialogViewModel);
			vm->text = text;
			vm->title = title;
			vm->icon = icon;

#define USE_BUTTON(NAME) vm->buttons.Add(Ptr(new FakeMessageBoxDialogViewModel::Action(vm.Obj(), INativeDialogService::Select##NAME)))
			switch (buttons)
			{
			case DisplayOK:						USE_BUTTON(OK);														break;
			case DisplayOKCancel:				USE_BUTTON(OK);		USE_BUTTON(Cancel);								break;
			case DisplayYesNo:					USE_BUTTON(Yes);	USE_BUTTON(No);									break;
			case DisplayYesNoCancel:			USE_BUTTON(Yes);	USE_BUTTON(No);			USE_BUTTON(Cancel);		break;
			case DisplayRetryCancel:			USE_BUTTON(Retry);	USE_BUTTON(Cancel);								break;
			case DisplayAbortRetryIgnore:		USE_BUTTON(Abort);	USE_BUTTON(Retry);		USE_BUTTON(Ignore);		break;
			case DisplayCancelTryAgainContinue:	USE_BUTTON(Cancel);	USE_BUTTON(TryAgain);	USE_BUTTON(Continue);	break;
			default:							USE_BUTTON(OK);														break;
			}
#undef USE_BUTTON

#define USE_DEFAULT_BUTTON(INDEX) if (vm->buttons.Count() > INDEX) vm->defaultButton = vm->buttons[INDEX]->GetButton()
			USE_DEFAULT_BUTTON(0);
			switch (defaultButton)
			{
			case DefaultSecond:					USE_DEFAULT_BUTTON(1); break;
			case DefaultThird:					USE_DEFAULT_BUTTON(2); break;
			default:;
			}
#undef USE_DEFAULT_BUTTON

			vm->result = vm->defaultButton;
			{
				auto owner = GetApplication()->GetWindowFromNative(window);
				auto dialog = CreateMessageBoxDialog(vm);
				ShowModalDialogAndDelete(owner, dialog);
			}
			return vm->result;
		}

		bool FakeDialogServiceBase::ShowColorDialog(
			INativeWindow* window,
			Color& selection,
			bool selected,
			ColorDialogCustomColorOptions customColorOptions,
			Color* customColors
		)
		{
			auto vm = Ptr(new FakeColorDialogViewModel);
			vm->color = selection;
			{
				auto owner = GetApplication()->GetWindowFromNative(window);
				auto dialog = CreateColorDialog(vm);
				ShowModalDialogAndDelete(owner, dialog);
			}
			if (vm->confirmed)
			{
				selection = vm->color;
			}
			return vm->confirmed;
		}

		bool FakeDialogServiceBase::ShowFontDialog(
			INativeWindow* window,
			FontProperties& selectionFont,
			Color& selectionColor,
			bool selected,
			bool showEffect,
			bool forceFontExist
		)
		{
			auto initVm = [=](auto vm)
			{
				vm->fontMustExist = forceFontExist;
				GetCurrentController()->ResourceService()->EnumerateFonts(vm->fontList);
				if (vm->fontList.Count() > 0)
				{
					Sort(
						&vm->fontList[0],
						vm->fontList.Count(),
						Func([](WString a, WString b) {return WString::Compare(a, b); })
						);
				}
			};

			if (showEffect)
			{
				auto vm = Ptr(new FakeFullFontDialogViewModel);
				initVm(vm);

				vm->font = selectionFont;
				vm->color = selectionColor;
				{
					auto owner = GetApplication()->GetWindowFromNative(window);
					auto dialog = CreateFullFontDialog(vm);
					ShowModalDialogAndDelete(owner, dialog);
				}
				if (vm->confirmed)
				{
					selectionFont = vm->font;
					selectionColor = vm->color;
				}
				return vm->confirmed;
			}
			else
			{
				auto vm = Ptr(new FakeSimpleFontDialogViewModel);
				initVm(vm);

				vm->fontFamily = selectionFont.fontFamily;
				vm->fontSize = selectionFont.size;
				{
					auto owner = GetApplication()->GetWindowFromNative(window);
					auto dialog = CreateSimpleFontDialog(vm);
					ShowModalDialogAndDelete(owner, dialog);
				}
				if (vm->confirmed)
				{
					selectionFont.fontFamily = vm->fontFamily;
					selectionFont.size = vm->fontSize;
				}
				return vm->confirmed;
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
				if (0 <= selectionFilterIndex && selectionFilterIndex < vm->filters.Count())
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
					ShowModalDialogAndDelete(owner, dialog);
				}
				break;
			case INativeDialogService::FileDialogSave:
			case INativeDialogService::FileDialogSavePreview:
				{
					auto owner = GetApplication()->GetWindowFromNative(window);
					auto dialog = CreateSaveFileDialog(vm);
					ShowModalDialogAndDelete(owner, dialog);
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