#include "GuiFakeDialogService.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
FakeDialogService
***********************************************************************/

		Ptr<controls::GuiWindow> FakeDialogService::CreateMessageBoxDialog(Ptr< IMessageBoxDialogViewModel> viewModel)
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