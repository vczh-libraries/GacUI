/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_FAKESERVICES_FAKETUIDIALOGSERVICE
#define VCZH_PRESENTATION_UTILITIES_FAKESERVICES_FAKETUIDIALOGSERVICE

#include "../GuiFakeDialogServiceBase.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiWindow;
		}

		/// <summary>
		/// UI implementations for <see cref="INativeDialogService"/>.
		/// </summary>
		class FakeTuiDialogService : public FakeDialogServiceBase
		{
		protected:

			controls::GuiWindow*	CreateMessageBoxDialog(Ptr< IMessageBoxDialogViewModel> viewModel) override;
			controls::GuiWindow*	CreateColorDialog(Ptr<IColorDialogViewModel> viewModel) override;
			controls::GuiWindow*	CreateSimpleFontDialog(Ptr<ISimpleFontDialogViewModel> viewModel) override;
			controls::GuiWindow*	CreateFullFontDialog(Ptr<IFullFontDialogViewModel> viewModel) override;
			controls::GuiWindow*	CreateOpenFileDialog(Ptr<IFileDialogViewModel> viewModel) override;
			controls::GuiWindow*	CreateSaveFileDialog(Ptr<IFileDialogViewModel> viewModel) override;

		public:
			FakeTuiDialogService();
			~FakeTuiDialogService();
		};
	}
}

#endif