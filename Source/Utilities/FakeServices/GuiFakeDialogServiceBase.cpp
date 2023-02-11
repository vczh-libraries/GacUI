#include "GuiFakeDialogServiceBase.h"

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
			WString					text;
			WString					title;
			Icon					icon;
			ButtonItemList			buttons;
			ButtonItem				defaultButton;
			ButtonItem				result;

			WString					GetText() { return text; }
			WString					GetTitle() { return title; }
			Icon					GetIcon() { return icon; }
			const ButtonItemList&	GetButtons() { return buttons; }
			ButtonItem				GetDefaultButton() { return defaultButton; }
			ButtonItem				GetResult() { return result; }
			void					SetResult(ButtonItem value) { result = value; }
		};

/***********************************************************************
FakeDialogServiceBase
***********************************************************************/

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

#define USE_BUTTON(NAME) vm->buttons.Add(INativeDialogService::Select##NAME)
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
			CHECK_FAIL(L"Not Implemented!");
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