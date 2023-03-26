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

			auto movingFlag = Ptr(new bool(false));
			dialog->ControlSignalTrigerred.AttachLambda([=](GuiGraphicsComposition* sender, GuiControlSignalEventArgs& arguments)
			{
				if (arguments.controlSignal == ControlSignal::UpdateFullfilled)
				{
					if (!*movingFlag.Obj())
					{
						*movingFlag.Obj() = true;
						dialog->MoveToScreenCenter();
					}
				}
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