/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_FAKESERVICES_FAKEDIALOGSERVICEBASE
#define VCZH_PRESENTATION_UTILITIES_FAKESERVICES_FAKEDIALOGSERVICEBASE

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		class FakeDialogServiceBase
			: public Object
			, public INativeDialogService
		{
		public:
			FakeDialogServiceBase();
			~FakeDialogServiceBase();

			MessageBoxButtonsOutput	ShowMessageBox(
										INativeWindow* window,
										const WString& text,
										const WString& title,
										MessageBoxButtonsInput buttons,
										MessageBoxDefaultButton defaultButton,
										MessageBoxIcons icon,
										MessageBoxModalOptions modal
										) override;

			bool					ShowColorDialog(
										INativeWindow* window,
										Color& selection,
										bool selected,
										ColorDialogCustomColorOptions customColorOptions,
										Color* customColors
										) override;

			bool					ShowFontDialog(
										INativeWindow* window,
										FontProperties& selectionFont,
										Color& selectionColor,
										bool selected,
										bool showEffect,
										bool forceFontExist
										) override;

			bool					ShowFileDialog(
										INativeWindow* window,
										collections::List<WString>& selectionFileNames,
										vint& selectionFilterIndex,
										FileDialogTypes dialogType,
										const WString& title,
										const WString& initialFileName,
										const WString& initialDirectory,
										const WString& defaultExtension,
										const WString& filter,
										FileDialogOptions options
										) override;
		};
	}
}

#endif