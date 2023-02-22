#include "GuiFakeDialogServiceBase.h"
#include "../../Application/Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace compositions;
		using namespace controls;

/***********************************************************************
FakeDialogServiceBase
***********************************************************************/

		void FakeDialogServiceBase::ShowModalDialogAndDelete(Ptr<IDescriptable> viewModel, controls::GuiWindow* owner, controls::GuiWindow* dialog)
		{
			auto app = GetApplication();
			bool exit = false;
			dialog->WindowOpened.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				dialog->ForceCalculateSizeImmediately();
				dialog->MoveToScreenCenter();
			});
			dialog->ShowModalAndDelete(owner, [&exit]() { exit = true; }, [viewModel]() { (void)viewModel; });
			while (!exit && app->RunOneCycle());
		}

		FakeDialogServiceBase::FakeDialogServiceBase()
		{
		}

		FakeDialogServiceBase::~FakeDialogServiceBase()
		{
		}
	}
}