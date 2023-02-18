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
			using ButtonItem = INativeDialogService::MessageBoxButtonsOutput;
			using ButtonItemList = collections::List<Ptr<IMessageBoxDialogAction>>;

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
		};

/***********************************************************************
FakeDialogServiceBase
***********************************************************************/

		/// <summary>
		/// A general <see cref="INativeDialogService"/> implementation.
		/// </summary>
		class FakeDialogServiceBase
			: public Object
			, public INativeDialogService
		{
		protected:

			/// <summary>
			/// A callback to be overrided by the sub class to create a message box from the given view model.
			/// </summary>
			/// <param name="viewModel">The given view model.</param>
			/// <returns>The created window to be displayed.</returns>
			virtual controls::GuiWindow*	CreateMessageBoxDialog(Ptr<IMessageBoxDialogViewModel> viewModel) = 0;

			/// <summary>
			/// A callback to be overrided by the sub class to create a color dialog from the given view model.
			/// </summary>
			/// <param name="viewModel">The given view model.</param>
			/// <returns>The created window to be displayed.</returns>
			virtual controls::GuiWindow*	CreateColorDialog(Ptr<IColorDialogViewModel> viewModel) = 0;

			/// <summary>
			/// A callback to be overrided by the sub class to create a simple font dialog from the given view model.
			/// </summary>
			/// <param name="viewModel">The given view model.</param>
			/// <returns>The created window to be displayed.</returns>
			virtual controls::GuiWindow*	CreateSimpleFontDialog(Ptr<ISimpleFontDialogViewModel> viewModel) = 0;

			/// <summary>
			/// A callback to be overrided by the sub class to create a full font dialog from the given view model.
			/// </summary>
			/// <param name="viewModel">The given view model.</param>
			/// <returns>The created window to be displayed.</returns>
			virtual controls::GuiWindow*	CreateFullFontDialog(Ptr<IFullFontDialogViewModel> viewModel) = 0;

			void							ShowModalDialogAndDelete(controls::GuiWindow* owner, controls::GuiWindow* dialog);

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