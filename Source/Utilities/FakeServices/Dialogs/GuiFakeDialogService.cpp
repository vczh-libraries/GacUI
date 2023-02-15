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

		controls::GuiWindow* FakeDialogService::CreateColorDialog(Ptr<IColorDialogViewModel> viewModel)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		controls::GuiWindow* FakeDialogService::CreateSimpleFontDialog(Ptr<ISimpleFontDialogViewModel> viewModel)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		controls::GuiWindow* FakeDialogService::CreateFullFontDialog(Ptr<IFullFontDialogViewModel> viewModel)
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