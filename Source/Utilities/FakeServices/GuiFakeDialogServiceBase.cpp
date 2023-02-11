#include "GuiFakeDialogServiceBase.h"

namespace vl
{
	namespace presentation
	{
		using namespace controls;

/***********************************************************************
FakeDialogServiceBase
***********************************************************************/

		FakeDialogServiceBase::FakeDialogServiceBase()
		{
		}

		FakeDialogServiceBase::~FakeDialogServiceBase()
		{
		}

		FakeDialogServiceBase::MessageBoxButtonsOutput	FakeDialogServiceBase::ShowMessageBox(
			INativeWindow* window,
			const WString& text,
			const WString& title,
			MessageBoxButtonsInput buttons,
			MessageBoxDefaultButton defaultButton,
			MessageBoxIcons icon,
			MessageBoxModalOptions modal
		)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		bool FakeDialogServiceBase::ShowColorDialog(
			INativeWindow* window,
			Color& selection,
			bool selected,
			ColorDialogCustomColorOptions customColorOptions,
			Color* customColors
		)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		bool FakeDialogServiceBase::ShowFontDialog(
			INativeWindow* window,
			FontProperties& selectionFont,
			Color& selectionColor,
			bool selected,
			bool showEffect,
			bool forceFontExist
		)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		bool FakeDialogServiceBase::ShowFileDialog(
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
		)
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}