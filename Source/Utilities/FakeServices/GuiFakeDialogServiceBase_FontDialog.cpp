#include "GuiFakeDialogServiceBase.h"
#include "../../Application/Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace controls;

/***********************************************************************
View Model (ICommonFontDialogViewModel)
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

/***********************************************************************
View Model (ISimpleFontDialogViewModel)
***********************************************************************/

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

/***********************************************************************
View Model (IFullFontDialogViewModel)
***********************************************************************/

		class FakeFullFontDialogViewModel : public FakeFontDialogViewModel<IFullFontDialogViewModel>
		{
		public:
			Color					color;
			FontProperties			font;

			Color					GetColor() override								{ return color; }
			void					SetColor(Color value) override					{ color = value; }

			FontProperties			GetFont() override								{ return font; }
			void					SetFont(const FontProperties& value) override	{ font = value; }

			bool SelectColor(controls::GuiWindow* owner) override
			{
				return GetCurrentController()->DialogService()->ShowColorDialog(
					owner->GetNativeWindow(),
					color
					);
			}
		};

/***********************************************************************
FakeDialogServiceBase
***********************************************************************/

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
					Sort(&vm->fontList[0], vm->fontList.Count());
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
					ShowModalDialogAndDelete(vm, owner, dialog);
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
					ShowModalDialogAndDelete(vm, owner, dialog);
				}
				if (vm->confirmed)
				{
					selectionFont.fontFamily = vm->fontFamily;
					selectionFont.size = vm->fontSize;
				}
				return vm->confirmed;
			}
		}
	}
}