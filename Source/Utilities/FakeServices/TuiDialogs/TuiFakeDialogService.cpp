#include "TuiFakeDialogService.h"
#include "Source/TuiFakeDialogServiceUIIncludes.h"

namespace vl
{
	namespace presentation
	{
		using namespace controls;

/***********************************************************************
FakeTuiDialogService
***********************************************************************/

		controls::GuiWindow* FakeTuiDialogService::CreateMessageBoxDialog(Ptr<IMessageBoxDialogViewModel> viewModel)
		{
			return new tui_controls::TuiMessageBoxWindow(viewModel);
		}

		controls::GuiWindow* FakeTuiDialogService::CreateColorDialog(Ptr<IColorDialogViewModel> viewModel)
		{
			return new tui_controls::TuiColorDialogWindow(viewModel);
		}

		controls::GuiWindow* FakeTuiDialogService::CreateSimpleFontDialog(Ptr<ISimpleFontDialogViewModel> viewModel)
		{
			return new tui_controls::TuiSimpleFontDialogWindow(viewModel);
		}

		controls::GuiWindow* FakeTuiDialogService::CreateFullFontDialog(Ptr<IFullFontDialogViewModel> viewModel)
		{
			return new tui_controls::TuiFullFontDialogWindow(viewModel);
		}

		controls::GuiWindow* FakeTuiDialogService::CreateOpenFileDialog(Ptr<IFileDialogViewModel> viewModel)
		{
			auto dialog = new tui_controls::TuiFileDialogWindow(viewModel);
			dialog->MakeOpenFileDialog();
			return dialog;
		}

		controls::GuiWindow* FakeTuiDialogService::CreateSaveFileDialog(Ptr<IFileDialogViewModel> viewModel)
		{
			auto dialog = new tui_controls::TuiFileDialogWindow(viewModel);
			dialog->MakeSaveFileDialog();
			return dialog;
		}

		FakeTuiDialogService::FakeTuiDialogService()
		{
		}

		FakeTuiDialogService::~FakeTuiDialogService()
		{
		}
	}
}
