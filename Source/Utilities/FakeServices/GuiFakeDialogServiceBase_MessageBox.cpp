#include "GuiFakeDialogServiceBase.h"
#include "../../Application/Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
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
				void				PerformAction() override				{ viewModel->result = Ptr(this); }
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
FakeDialogServiceBase
***********************************************************************/

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

#define USE_DEFAULT_BUTTON(INDEX) if (vm->buttons.Count() > INDEX) vm->defaultButton = vm->buttons[INDEX]
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
				ShowModalDialogAndDelete(vm, owner, dialog);
			}
			return vm->result->GetButton();
		}
	}
}