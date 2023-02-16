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
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}