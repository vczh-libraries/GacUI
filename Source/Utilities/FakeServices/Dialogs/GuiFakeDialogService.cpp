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
			return new gaclib_controls::ColorDialogWindow(viewModel);
		}

		controls::GuiWindow* FakeDialogService::CreateSimpleFontDialog(Ptr<ISimpleFontDialogViewModel> viewModel)
		{
			return new gaclib_controls::SimpleFontDialogWindow(viewModel);
		}

		controls::GuiWindow* FakeDialogService::CreateFullFontDialog(Ptr<IFullFontDialogViewModel> viewModel)
		{
			return new gaclib_controls::FullFontDialogWindow(viewModel);
		}

		controls::GuiWindow* FakeDialogService::CreateOpenFileDialog(Ptr<IFileDialogViewModel> viewModel)
		{
			auto dialog = new gaclib_controls::FileDialogWindow(viewModel);
			dialog->MakeOpenFileDialog();
			return dialog;
		}

		controls::GuiWindow* FakeDialogService::CreateSaveFileDialog(Ptr<IFileDialogViewModel> viewModel)
		{
			auto dialog = new gaclib_controls::FileDialogWindow(viewModel);
			dialog->MakeSaveFileDialog();
			return dialog;
		}

		FakeDialogService::FakeDialogService()
		{
		}

		FakeDialogService::~FakeDialogService()
		{
		}
	}
}