/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_FAKESERVICES_FAKEDIALOGSERVICEBASE
#define VCZH_PRESENTATION_UTILITIES_FAKESERVICES_FAKEDIALOGSERVICEBASE

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiWindow;
		}

/***********************************************************************
View Models (MessageBox)
***********************************************************************/

		/// <summary>
		/// The view model for message box button. It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class IMessageBoxDialogAction : public virtual IDescriptable
		{
		public:
			using ButtonItem = INativeDialogService::MessageBoxButtonsOutput;

			/// <summary>
			/// Get the button that it stands for.
			/// </summary>
			/// <returns>The button.</returns>
			virtual ButtonItem				GetButton() = 0;
			/// <summary>
			/// Select this button.
			/// </summary>
			virtual void					PerformAction() = 0;
		};

		/// <summary>
		/// The view model for message box. It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class IMessageBoxDialogViewModel : public virtual IDescriptable
		{
		public:
			using Icon = INativeDialogService::MessageBoxIcons;
			using ButtonItem = Ptr<IMessageBoxDialogAction>;
			using ButtonItemList = collections::List<ButtonItem>;

			/// <summary>
			/// Get the text to display on the message box.
			/// </summary>
			/// <returns>The text.</returns>
			virtual WString					GetText() = 0;
			/// <summary>
			/// Get the title to display on the message box.
			/// </summary>
			/// <returns>The title.</returns>
			virtual WString					GetTitle() = 0;
			/// <summary>
			/// Get the icon to display on the message box.
			/// </summary>
			/// <returns>The icon.</returns>
			virtual Icon					GetIcon() = 0;
			/// <summary>
			/// Get all buttons to display on the message box.
			/// </summary>
			/// <returns>All buttons.</returns>
			virtual const ButtonItemList&	GetButtons() = 0;
			/// <summary>
			/// Get the button that should have the focus by default.
			/// </summary>
			/// <returns>The button to be focused.</returns>
			virtual ButtonItem				GetDefaultButton() = 0;
			/// <summary>
			/// Get the selected button. It is set by <see cref="IMessageBoxDialogAction::PerformAction"/>.
			/// </summary>
			/// <returns>The selected button.</returns>
			virtual ButtonItem				GetResult() = 0;
		};

/***********************************************************************
View Models (Confirmation)
***********************************************************************/

		/// <summary>
		/// The view model for all dialogs with "OK" and "Cancel" button.  It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class IDialogConfirmation : public virtual IDescriptable
		{
		public:
			/// <summary>
			/// Test is the OK button is selected.
			/// </summary>
			/// <returns>Returns true if the OK button is selected.</returns>
			virtual bool					GetConfirmed() = 0;
			/// <summary>
			/// Set the selected button.
			/// </summary>
			/// <param name="value">True for OK, false for Cancel.</param>
			virtual void					SetConfirmed(bool value) = 0;
		};

/***********************************************************************
View Models (ColorDialog)
***********************************************************************/

		/// <summary>
		/// The view model for color dialog. It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class IColorDialogViewModel : public virtual IDialogConfirmation
		{
		public:
			/// <summary>
			/// Get the selected color. When the dialog is opened, it returns the pre-selected color.
			/// </summary>
			/// <returns>The selected color.</returns>
			virtual Color					GetColor() = 0;
			/// <summary>
			/// Set the selected color.
			/// </summary>
			/// <param name="value">The selected color.</param>
			virtual void					SetColor(Color value) = 0;
		};

/***********************************************************************
View Models (FontDialog)
***********************************************************************/

		/// <summary>
		/// The view model for all font dialogs. It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class ICommonFontDialogViewModel : public virtual IDescriptable
		{
		public:
			using FontList = collections::List<WString>;

			/// <summary>
			/// Test if the selected font should be one in <see cref="GetFontList"/>. If it is true and the selected font does not exist, the OK button should be disabled.
			/// </summary>
			/// <returns>Returns true if the selected font should be one in <see cref="GetFontList"/>.</returns>
			virtual bool					GetFontMustExist() = 0;
			/// <summary>
			/// All registered fonts in the system.
			/// </summary>
			/// <returns>All registered fonts.</returns>
			virtual const FontList&			GetFontList() = 0;
		};

		/// <summary>
		/// The view model for simple font dialog. It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class ISimpleFontDialogViewModel : public virtual ICommonFontDialogViewModel, public virtual IDialogConfirmation
		{
		public:
			/// <summary>
			/// Get the selected font. When the dialog is opened, it returns the pre-selected font.
			/// </summary>
			/// <returns>The selected font.</returns>
			virtual WString					GetFontFamily() = 0;
			/// <summary>
			/// Set the selected font.
			/// </summary>
			/// <param name="fontface">The selected font.</param>
			virtual void					SetFontFamily(const WString& fontface) = 0;
			/// <summary>
			/// Get the selected size. When the dialog is opened, it returns the pre-selected size.
			/// </summary>
			/// <returns>The selected size.</returns>
			virtual vint					GetFontSize() = 0;
			/// <summary>
			/// Set the selected size.
			/// </summary>
			/// <param name="value">The selected size.</param>
			virtual void					SetFontSize(vint value) = 0;
		};

		/// <summary>
		/// The view model for full font dialog. It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class IFullFontDialogViewModel : public virtual ICommonFontDialogViewModel, public virtual IColorDialogViewModel
		{
		public:
			/// <summary>
			/// Get the selected font. When the dialog is opened, it returns the pre-selected font.
			/// </summary>
			/// <returns>The selected font.</returns>
			virtual FontProperties			GetFont() = 0;
			/// <summary>
			/// Set the selected font.
			/// </summary>
			/// <param name="value">The selected font.</param>
			virtual void					SetFont(const FontProperties& value) = 0;
			/// <summary>
			/// Display a color dialog and change the Color property in <see cref="IColorDialogViewModel"/>.
			/// </summary>
			/// <returns>Returns true when a color is selected.</returns>
			/// <param name="owner">A owner window for displaying color dialogs.</param>
			virtual bool					SelectColor(controls::GuiWindow* owner) = 0;
		};

/***********************************************************************
View Models (FileDialog)
***********************************************************************/

		/// <summary>
		/// Type of a folder in a file dialog.
		/// </summary>
		enum class FileDialogFolderType
		{
			/// <summary>
			/// The root folder, it does not render in the dialog.
			/// </summary>
			Root,
			/// <summary>
			/// A placeolder item, it means folders are being loaded.
			/// </summary>
			Placeholder,
			/// <summary>
			/// A folder.
			/// </summary>
			Folder,
		};

		/// <summary>
		/// The view model for a folder in a file dialog. It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class IFileDialogFolder : public virtual IDescriptable
		{
		public:
			using Folders = collections::ObservableList<Ptr<IFileDialogFolder>>;

			/// <summary>
			/// Get the parent folder of this folder.
			/// </summary>
			/// <returns>The parent folder. It returns null for the root folder.</returns>
			virtual Ptr<IFileDialogFolder>	GetParent() = 0;
			/// <summary>
			/// Get the type of this folder.
			/// </summary>
			/// <returns>The type.</returns>
			virtual FileDialogFolderType	GetType() = 0;
			/// <summary>
			/// Get the full path of this folder.
			/// </summary>
			/// <returns>The full path. It returns an empty string for root or placeholder.</returns>
			virtual WString					GetFullPath() = 0;
			/// <summary>
			/// Get the name of this folder.
			/// </summary>
			/// <returns>The name. It returns an empty string for root.</returns>
			virtual WString					GetName() = 0;
			/// <summary>
			/// Get the rendering position of this folder in its parent folder.
			/// </summary>
			/// <returns>The rendering position.</returns>
			virtual vint					GetIndex() = 0;
			/// <summary>
			/// Get all sub folders of this folder.
			/// </summary>
			/// <returns>All sub folders.</returns>
			virtual Folders&				GetFolders() = 0;
			/// <summary>
			/// Get a sub folder by its name.
			/// </summary>
			/// <param name="name">The name of the sub folder.</param>
			/// <returns>The sub folder. It returns null if the object has not been created yet, this doesn't mean the folder doesn't exist.</returns>
			virtual Ptr<IFileDialogFolder>	TryGetFolder(const WString& name) = 0;
		};

		/// <summary>
		/// Type of a file in a file dialog.
		/// </summary>
		enum class FileDialogFileType
		{
			/// <summary>
			/// A placeholder item, it means files and folders are being loaded.
			/// </summary>
			Placeholder,
			/// <summary>
			/// A folder.
			/// </summary>
			Folder,
			/// <summary>
			/// A file.
			/// </summary>
			File,
		};

		/// <summary>
		/// The view model for a file in a file dialog. It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class IFileDialogFile : public virtual IDescriptable
		{
		public:
			using Files = collections::ObservableList<Ptr<IFileDialogFile>>;

			/// <summary>
			/// Get the type of this file.
			/// </summary>
			/// <returns></returns>
			virtual FileDialogFileType		GetType() = 0;
			/// <summary>
			/// Get the associated folder of this file.
			/// </summary>
			/// <returns>The associated folder. It returns null for placeholder or file.</returns>
			virtual Ptr<IFileDialogFolder>	GetAssociatedFolder() = 0;
			/// <summary>
			/// Get the name of this file.
			/// </summary>
			/// <returns>The name.</returns>
			virtual WString					GetName() = 0;
		};

		/// <summary>
		/// The view model for a filter in a file dialog. It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class IFileDialogFilter : public virtual IDescriptable
		{
		public:
			using Filters = collections::List<Ptr<IFileDialogFilter>>;

			/// <summary>
			/// Get the name of this filter.
			/// </summary>
			/// <returns>The name.</returns>
			virtual WString					GetName() = 0;
			/// <summary>
			/// Get the wildcard of this filter.
			/// </summary>
			/// <returns>The wildcard.</returns>
			virtual WString					GetFilter() = 0;
			/// <summary>
			/// Get the default extension for this filter.
			/// </summary>
			/// <returns>The default extension. It returns null if it is not defined.</returns>
			virtual Nullable<WString>		GetDefaultExtension() = 0;
			/// <summary>
			/// Filter a file.
			/// </summary>
			/// <param name="file">The file to filter.</param>
			/// <returns>Returns true if the file satisfies the filter.</returns>
			virtual bool					FilterFile(Ptr<IFileDialogFile> file) = 0;
		};

		/// <summary>
		/// The view model for file dialog. It is implemented by <see cref="FakeDialogServiceBase"/>.
		/// </summary>
		class IFileDialogViewModel : public virtual IDescriptable
		{
		public:
			using Filters = IFileDialogFilter::Filters;
			using Folders = IFileDialogFolder::Folders;
			using Files = IFileDialogFile::Files;
			using Selection = collections::LazyList<WString>;

			/// <summary>
			/// Raised when the <see cref="GetSelectedFilter"/> is changed.
			/// </summary>
			Event<void()>					SelectedFilterChanged;
			/// <summary>
			/// Raised when the <see cref="GetSelectedFolder"/> is changed.
			/// </summary>
			Event<void()>					SelectedFolderChanged;
			/// <summary>
			/// Raised when the <see cref="GetIsLoadingFiles"/> property is changed.
			/// </summary>
			Event<void()>					IsLoadingFilesChanged;

			/// <summary>
			/// Get the title of this dialog.
			/// </summary>
			/// <returns></returns>
			virtual WString					GetTitle() = 0;
			/// <summary>
			/// Test if multiple selection is allowed.
			/// </summary>
			/// <returns>Returns true if multiple selection is allowed.</returns>
			virtual bool					GetEnabledMultipleSelection() = 0;

			/// <summary>
			/// Get the default extension.
			/// </summary>
			/// <returns>The default extension.</returns>
			virtual WString					GetDefaultExtension() = 0;
			/// <summary>
			/// Get all filters of this dialog.
			/// </summary>
			/// <returns>All filters.</returns>
			virtual const Filters&			GetFilters() = 0;
			/// <summary>
			/// Get the selected filter of this dialog.
			/// </summary>
			/// <returns>The selected filter of this dialog.</returns>
			virtual Ptr<IFileDialogFilter>	GetSelectedFilter() = 0;
			/// <summary>
			/// Set the selected filter of this dialog. It could cause folders and files to be refreshed.
			/// </summary>
			/// <param name="value">The selected filter of this dialog.</param>
			virtual void					SetSelectedFilter(Ptr<IFileDialogFilter> value) = 0;

			/// <summary>
			/// Get the root folder.
			/// </summary>
			/// <returns>The root folder.</returns>
			virtual Ptr<IFileDialogFolder>	GetRootFolder() = 0;
			/// <summary>
			/// Get the selected folder.
			/// </summary>
			/// <returns>The selected folder.</returns>
			virtual Ptr<IFileDialogFolder>	GetSelectedFolder() = 0;
			/// <summary>
			/// Set the selected folder.
			/// </summary>
			/// <param name="value">The selected folder.</param>
			virtual void					SetSelectedFolder(Ptr<IFileDialogFolder> value) = 0;

			/// <summary>
			/// Test if folders and files are being loaded.
			/// </summary>
			/// <returns>Returns true if folders and files are being loaded.</returns>
			virtual bool					GetIsLoadingFiles() = 0;
			/// <summary>
			/// Get all folders and files in the selected folder.
			/// </summary>
			/// <returns>All folders and files to display.</returns>
			virtual Files&					GetFiles() = 0;
			/// <summary>
			/// Refresh the folders and files list.
			/// </summary>
			virtual void					RefreshFiles() = 0;

			/// <summary>
			/// Convert files to a display string.
			/// </summary>
			/// <param name="files">The files.</param>
			/// <returns>The display string, items are separated by ";".</returns>
			virtual WString					GetDisplayString(collections::LazyList<Ptr<IFileDialogFile>> files) = 0;

			/// <summary>
			/// Split the display string to items.
			/// </summary>
			/// <param name="displayString">The display string.</param>
			/// <returns>The items, each item is either a relative path or an absolute path.</returns>
			virtual Selection				ParseDisplayString(const WString& displayString) = 0;
			/// <summary>
			/// Test if the selection is valid. Dialogs could be displayed and ask for input accordingly.
			/// </summary>
			/// <param name="owner">A owner window for displaying message boxes.</param>
			/// <param name="selectedPaths">All selected items in string format. Each of them could be either full path, relative path or file name.</param>
			/// <returns>Returns true if the selection is valid.</returns>
			virtual bool					TryConfirm(controls::GuiWindow* owner, Selection selection) = 0;

			/// <summary>
			/// Initialize the view model with localized texts.
			/// </summary>
			/// <param name="textLoadingFolders">The name for placeholder folder.</param>
			/// <param name="textLoadingFiles">The name for placeholder file.</param>
			/// <param name="dialogErrorEmptySelection">The message saying selection is empty.</param>
			/// <param name="dialogErrorFileNotExist">The message saying selected files do not exist.</param>
			/// <param name="dialogErrorFileExpected">The message saying selected files are expected but they are folders.</param>
			/// <param name="dialogErrorFolderNotExist">The message saying the selected folder do not exist.</param>
			/// <param name="dialogErrorMultipleSelectionNotEnabled">The message saying multiple selection is not allowed.</param>
			/// <param name="dialogAskCreateFile">The message asking if user wants to create a file.</param>
			/// <param name="dialogAskOverrideFile">The message asking if user wants to override a file.</param>
			virtual void					InitLocalizedText(
												const WString& textLoadingFolders,
												const WString& textLoadingFiles,
												const WString& dialogErrorEmptySelection,
												const WString& dialogErrorFileNotExist,
												const WString& dialogErrorFileExpected,
												const WString& dialogErrorFolderNotExist,
												const WString& dialogErrorMultipleSelectionNotEnabled,
												const WString& dialogAskCreateFile,
												const WString& dialogAskOverrideFile
											) = 0;
		};

/***********************************************************************
FakeDialogServiceBase
***********************************************************************/

		/// <summary>
		/// View model implementations for <see cref="INativeDialogService"/>.
		/// </summary>
		class FakeDialogServiceBase
			: public Object
			, public INativeDialogService
		{
		protected:

			/// <summary>
			/// A callback to create a message box from the given view model.
			/// </summary>
			/// <param name="viewModel">The given view model.</param>
			/// <returns>The created window to be displayed.</returns>
			virtual controls::GuiWindow*	CreateMessageBoxDialog(Ptr<IMessageBoxDialogViewModel> viewModel) = 0;

			/// <summary>
			/// A callback to create a color dialog from the given view model.
			/// </summary>
			/// <param name="viewModel">The given view model.</param>
			/// <returns>The created window to be displayed.</returns>
			virtual controls::GuiWindow*	CreateColorDialog(Ptr<IColorDialogViewModel> viewModel) = 0;

			/// <summary>
			/// A callback to create a simple font dialog from the given view model.
			/// </summary>
			/// <param name="viewModel">The given view model.</param>
			/// <returns>The created window to be displayed.</returns>
			virtual controls::GuiWindow*	CreateSimpleFontDialog(Ptr<ISimpleFontDialogViewModel> viewModel) = 0;

			/// <summary>
			/// A callback to create a full font dialog from the given view model.
			/// </summary>
			/// <param name="viewModel">The given view model.</param>
			/// <returns>The created window to be displayed.</returns>
			virtual controls::GuiWindow*	CreateFullFontDialog(Ptr<IFullFontDialogViewModel> viewModel) = 0;

			/// <summary>
			/// A callback to create a open file dialog from the given view model.
			/// </summary>
			/// <param name="viewModel">The given view model.</param>
			/// <returns>The created window to be displayed.</returns>
			virtual controls::GuiWindow*	CreateOpenFileDialog(Ptr<IFileDialogViewModel> viewModel) = 0;

			/// <summary>
			/// A callback to create a save file dialog from the given view model.
			/// </summary>
			/// <param name="viewModel">The given view model.</param>
			/// <returns>The created window to be displayed.</returns>
			virtual controls::GuiWindow*	CreateSaveFileDialog(Ptr<IFileDialogViewModel> viewModel) = 0;

			void							ShowModalDialogAndDelete(Ptr<IDescriptable> viewModel, controls::GuiWindow* owner, controls::GuiWindow* dialog);

		public:
			FakeDialogServiceBase();
			~FakeDialogServiceBase();

			MessageBoxButtonsOutput	ShowMessageBox(
										INativeWindow* window,
										const WString& text,
										const WString& title,
										MessageBoxButtonsInput buttons,
										MessageBoxDefaultButton defaultButton,
										MessageBoxIcons icon,
										MessageBoxModalOptions modal
										) override;

			bool					ShowColorDialog(
										INativeWindow* window,
										Color& selection,
										bool selected,
										ColorDialogCustomColorOptions customColorOptions,
										Color* customColors
										) override;

			bool					ShowFontDialog(
										INativeWindow* window,
										FontProperties& selectionFont,
										Color& selectionColor,
										bool selected,
										bool showEffect,
										bool forceFontExist
										) override;

			bool					ShowFileDialog(
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
										) override;
		};
	}
}

#endif