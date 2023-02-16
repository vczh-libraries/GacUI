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

		class IMessageBoxDialogAction : public virtual IDescriptable
		{
		public:
			using ButtonItem = INativeDialogService::MessageBoxButtonsOutput;

			virtual ButtonItem				GetButton() = 0;
			virtual void					PerformAction() = 0;
		};

		class IMessageBoxDialogViewModel : public virtual IDescriptable
		{
		public:
			using Icon = INativeDialogService::MessageBoxIcons;
			using ButtonItem = INativeDialogService::MessageBoxButtonsOutput;
			using ButtonItemList = collections::List<Ptr<IMessageBoxDialogAction>>;

			virtual WString					GetText() = 0;
			virtual WString					GetTitle() = 0;
			virtual Icon					GetIcon() = 0;
			virtual const ButtonItemList&	GetButtons() = 0;
			virtual ButtonItem				GetDefaultButton() = 0;
			virtual ButtonItem				GetResult() = 0;
		};

/***********************************************************************
View Models (Confirmation)
***********************************************************************/

		class IDialogConfirmation : public virtual IDescriptable
		{
		public:
			virtual bool					GetConfirmed() = 0;
			virtual void					SetConfirmed(bool value) = 0;
		};

/***********************************************************************
View Models (ColorDialog)
***********************************************************************/

		class IColorDialogViewModel : public virtual IDialogConfirmation
		{
		public:
			virtual Color					GetColor() = 0;
			virtual void					SetColor(Color value) = 0;
		};

/***********************************************************************
View Models (FontDialog)
***********************************************************************/

		class ICommonFontDialogViewModel : public virtual IDescriptable
		{
		public:
			using FontList = collections::List<WString>;

			virtual bool					GetFontMustExist() = 0;
			virtual const FontList&			GetFontList() = 0;
		};

		class ISimpleFontDialogViewModel : public virtual ICommonFontDialogViewModel, public virtual IDialogConfirmation
		{
		public:
			virtual WString					GetFontFamily() = 0;
			virtual void					SetFontFamily(const WString& fontface) = 0;
		};

		class IFullFontDialogViewModel : public virtual ICommonFontDialogViewModel, public virtual IColorDialogViewModel
		{
		public:
			virtual FontProperties			GetFont() = 0;
			virtual void					SetFont(const FontProperties& value) = 0;
		};

/***********************************************************************
FakeDialogServiceBase
***********************************************************************/

		class FakeDialogServiceBase
			: public Object
			, public INativeDialogService
		{
		protected:

			virtual controls::GuiWindow*	CreateMessageBoxDialog(Ptr<IMessageBoxDialogViewModel> viewModel) = 0;
			virtual controls::GuiWindow*	CreateColorDialog(Ptr<IColorDialogViewModel> viewModel) = 0;
			virtual controls::GuiWindow*	CreateSimpleFontDialog(Ptr<ISimpleFontDialogViewModel> viewModel) = 0;
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