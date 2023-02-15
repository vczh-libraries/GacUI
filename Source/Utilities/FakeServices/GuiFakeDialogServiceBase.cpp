#include "GuiFakeDialogServiceBase.h"
#include "../../Application/Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
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
			if (vm->confirmed) selection = vm->color;
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
			CHECK_FAIL(L"Not Implemented!");
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
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}