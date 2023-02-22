#include "GuiFakeDialogServiceBase.h"
#include "../../Application/Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace controls;

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
				ShowModalDialogAndDelete(vm, owner, dialog);
			}
			if (vm->confirmed)
			{
				selection = vm->color;
			}
			return vm->confirmed;
		}
	}
}