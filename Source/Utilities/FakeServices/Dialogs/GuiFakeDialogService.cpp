#include "GuiFakeDialogService.h"
#include "Source/GuiFakeDialogServiceUIIncludes.h"

namespace vl
{
	namespace presentation
	{
		using namespace controls;

/***********************************************************************
FakeDialogService
***********************************************************************/

		controls::GuiWindow* FakeDialogService::CreateMessageBoxDialog(Ptr< IMessageBoxDialogViewModel> viewModel)
		{
			return new gaclib_controls::MessageBoxWindow(viewModel);
		}

		controls::GuiWindow* CreateColorDialog(Ptr<IMessageBoxDialogViewModel> viewModel, Ptr<IDialogConfirmation> confirmation)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		controls::GuiWindow* CreateSimpleFontDialog(Ptr<IMessageBoxDialogViewModel> viewModel, Ptr<IDialogConfirmation> confirmation)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		controls::GuiWindow* CreateFullFontDialog(Ptr<IMessageBoxDialogViewModel> viewModel, Ptr<IDialogConfirmation> confirmation)
		{
			CHECK_FAIL(L"Not Implemented!");
		}


		FakeDialogService::FakeDialogService()
		{
		}

		FakeDialogService::~FakeDialogService()
		{
		}
	}
}