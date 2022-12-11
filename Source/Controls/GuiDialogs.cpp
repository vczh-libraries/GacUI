#include "GuiDialogs.h"
#include "../Application/Controls/GuiWindowControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace collections;
			using namespace reflection::description;

/***********************************************************************
GuiDialogBase
***********************************************************************/

			GuiWindow* GuiDialogBase::GetHostWindow()
			{
				if (rootObject)
				{
					if (auto control = dynamic_cast<GuiControl*>(rootObject))
					{
						if (auto host = control->GetRelatedControlHost())
						{
							return dynamic_cast<GuiWindow*>(host);
						}
					}
					else if (auto composition = dynamic_cast<GuiGraphicsComposition*>(rootObject))
					{
						if (auto host = composition->GetRelatedControlHost())
						{
							return dynamic_cast<GuiWindow*>(host);
						}
					}
				}
				return nullptr;
			}

			GuiDialogBase::GuiDialogBase()
			{
			}

			GuiDialogBase::~GuiDialogBase()
			{
			}

			void GuiDialogBase::Attach(GuiInstanceRootObject* _rootObject)
			{
				rootObject = _rootObject;
			}

			void GuiDialogBase::Detach(GuiInstanceRootObject* _rootObject)
			{
				rootObject = nullptr;
			}

/***********************************************************************
GuiMessageDialog
***********************************************************************/

			GuiMessageDialog::GuiMessageDialog()
			{
			}

			GuiMessageDialog::~GuiMessageDialog()
			{
			}

			INativeDialogService::MessageBoxButtonsInput GuiMessageDialog::GetInput()
			{
				return input;
			}

			void GuiMessageDialog::SetInput(INativeDialogService::MessageBoxButtonsInput value)
			{
				input = value;
			}

			INativeDialogService::MessageBoxDefaultButton GuiMessageDialog::GetDefaultButton()
			{
				return defaultButton;
			}

			void GuiMessageDialog::SetDefaultButton(INativeDialogService::MessageBoxDefaultButton value)
			{
				defaultButton = value;
			}

			INativeDialogService::MessageBoxIcons GuiMessageDialog::GetIcon()
			{
				return icon;
			}

			void GuiMessageDialog::SetIcon(INativeDialogService::MessageBoxIcons value)
			{
				icon = value;
			}

			INativeDialogService::MessageBoxModalOptions GuiMessageDialog::GetModalOption()
			{
				return modalOption;
			}

			void GuiMessageDialog::SetModalOption(INativeDialogService::MessageBoxModalOptions value)
			{
				modalOption = value;
			}

			const WString& GuiMessageDialog::GetText()
			{
				return text;
			}

			void GuiMessageDialog::SetText(const WString& value)
			{
				text = value;
			}

			const WString& GuiMessageDialog::GetTitle()
			{
				return title;
			}

			void GuiMessageDialog::SetTitle(const WString& value)
			{
				title = value;
			}

			INativeDialogService::MessageBoxButtonsOutput GuiMessageDialog::ShowDialog()
			{
				auto service = GetCurrentController()->DialogService();
				return service->ShowMessageBox(GetHostWindow()->GetNativeWindow(), text, title, input, defaultButton, icon, modalOption);
			}

/***********************************************************************
GuiColorDialog
***********************************************************************/

			GuiColorDialog::GuiColorDialog()
			{
				for (vint i = 0; i < 16; i++)
				{
					customColors.Add(Color());
				}
			}

			GuiColorDialog::~GuiColorDialog()
			{
			}

			bool GuiColorDialog::GetEnabledCustomColor()
			{
				return enabledCustomColor;
			}

			void GuiColorDialog::SetEnabledCustomColor(bool value)
			{
				enabledCustomColor = value;
			}

			bool GuiColorDialog::GetOpenedCustomColor()
			{
				return openedCustomColor;
			}

			void GuiColorDialog::SetOpenedCustomColor(bool value)
			{
				openedCustomColor = value;
			}

			Color GuiColorDialog::GetSelectedColor()
			{
				return selectedColor;
			}

			void GuiColorDialog::SetSelectedColor(Color value)
			{
				if (selectedColor != value)
				{
					selectedColor = value;
					SelectedColorChanged.Execute(GuiEventArgs());
				}
			}

			collections::List<Color>& GuiColorDialog::GetCustomColors()
			{
				return customColors;
			}

			bool GuiColorDialog::ShowDialog()
			{
				Array<Color> colors;
				CopyFrom(colors, customColors);
				colors.Resize(16);

				INativeDialogService::ColorDialogCustomColorOptions options =
					!enabledCustomColor ? INativeDialogService::CustomColorDisabled :
					!openedCustomColor ? INativeDialogService::CustomColorEnabled :
					INativeDialogService::CustomColorOpened;

				auto service = GetCurrentController()->DialogService();
				if (!service->ShowColorDialog(GetHostWindow()->GetNativeWindow(), selectedColor, showSelection, options, &colors[0]))
				{
					return false;
				}

				CopyFrom(customColors, colors);
				SelectedColorChanged.Execute(GuiEventArgs());
				return true;
			}

/***********************************************************************
GuiFontDialog
***********************************************************************/

			GuiFontDialog::GuiFontDialog()
			{
			}

			GuiFontDialog::~GuiFontDialog()
			{
			}

			const FontProperties& GuiFontDialog::GetSelectedFont()
			{
				return selectedFont;
			}

			void GuiFontDialog::SetSelectedFont(const FontProperties& value)
			{
				if (selectedFont != value)
				{
					selectedFont = value;
					SelectedFontChanged.Execute(GuiEventArgs());
				}
			}

			Color GuiFontDialog::GetSelectedColor()
			{
				return selectedColor;
			}

			void GuiFontDialog::SetSelectedColor(Color value)
			{
				if (selectedColor != value)
				{
					selectedColor = value;
					SelectedColorChanged.Execute(GuiEventArgs());
				}
			}

			bool GuiFontDialog::GetShowSelection()
			{
				return showSelection;
			}

			void GuiFontDialog::SetShowSelection(bool value)
			{
				showSelection = value;
			}

			bool GuiFontDialog::GetShowEffect()
			{
				return showEffect;
			}

			void GuiFontDialog::SetShowEffect(bool value)
			{
				showEffect = value;
			}

			bool GuiFontDialog::GetForceFontExist()
			{
				return forceFontExist;
			}

			void GuiFontDialog::SetForceFontExist(bool value)
			{
				forceFontExist = value;
			}

			bool GuiFontDialog::ShowDialog()
			{
				auto service = GetCurrentController()->DialogService();
				if (!service->ShowFontDialog(GetHostWindow()->GetNativeWindow(), selectedFont, selectedColor, showSelection, showEffect, forceFontExist))
				{
					return false;
				}

				SelectedColorChanged.Execute(GuiEventArgs());
				SelectedFontChanged.Execute(GuiEventArgs());
				return true;
			}

/***********************************************************************
GuiFileDialogBase
***********************************************************************/

			GuiFileDialogBase::GuiFileDialogBase()
			{
			}

			GuiFileDialogBase::~GuiFileDialogBase()
			{
			}

			const WString& GuiFileDialogBase::GetFilter()
			{
				return filter;
			}

			void GuiFileDialogBase::SetFilter(const WString& value)
			{
				filter = value;
			}

			vint GuiFileDialogBase::GetFilterIndex()
			{
				return filterIndex;
			}

			void GuiFileDialogBase::SetFilterIndex(vint value)
			{
				if (filterIndex != value)
				{
					filterIndex = value;
					FilterIndexChanged.Execute(GuiEventArgs());
				}
			}

			bool GuiFileDialogBase::GetEnabledPreview()
			{
				return enabledPreview;
			}

			void GuiFileDialogBase::SetEnabledPreview(bool value)
			{
				enabledPreview = value;
			}

			WString GuiFileDialogBase::GetTitle()
			{
				return title;
			}

			void GuiFileDialogBase::SetTitle(const WString& value)
			{
				title = value;
			}

			WString GuiFileDialogBase::GetFileName()
			{
				return fileName;
			}

			void GuiFileDialogBase::SetFileName(const WString& value)				
			{
				if (fileName != value)
				{
					FileNameChanged.Execute(GuiEventArgs());
				}
			}

			WString GuiFileDialogBase::GetDirectory()
			{
				return directory;
			}

			void GuiFileDialogBase::SetDirectory(const WString& value)
			{
				directory = value;
			}

			WString GuiFileDialogBase::GetDefaultExtension()
			{
				return defaultExtension;
			}

			void GuiFileDialogBase::SetDefaultExtension(const WString& value)
			{
				defaultExtension = value;
			}

			INativeDialogService::FileDialogOptions GuiFileDialogBase::GetOptions()
			{
				return options;
			}

			void GuiFileDialogBase::SetOptions(INativeDialogService::FileDialogOptions value)
			{
				options = value;
			}

/***********************************************************************
GuiOpenFileDialog
***********************************************************************/

			GuiOpenFileDialog::GuiOpenFileDialog()
			{
			}

			GuiOpenFileDialog::~GuiOpenFileDialog()
			{
			}

			collections::List<WString>& GuiOpenFileDialog::GetFileNames()
			{
				return fileNames;
			}

			bool GuiOpenFileDialog::ShowDialog()
			{
				fileNames.Clear();
				auto service = GetCurrentController()->DialogService();
				if (!service->ShowFileDialog(
					GetHostWindow()->GetNativeWindow(),
					fileNames,
					filterIndex,
					(enabledPreview ? INativeDialogService::FileDialogOpenPreview : INativeDialogService::FileDialogOpen),
					title,
					fileName,
					directory,
					defaultExtension,
					filter,
					options))
				{
					return false;
				}

				if (fileNames.Count() > 0)
				{
					fileName = fileNames[0];
					FileNameChanged.Execute(GuiEventArgs());
					FilterIndexChanged.Execute(GuiEventArgs());
				}
				return true;
			}

/***********************************************************************
GuiSaveFileDialog
***********************************************************************/

			GuiSaveFileDialog::GuiSaveFileDialog()
			{
			}

			GuiSaveFileDialog::~GuiSaveFileDialog()
			{
			}

			bool GuiSaveFileDialog::ShowDialog()
			{
				List<WString> fileNames;
				auto service = GetCurrentController()->DialogService();
				if (!service->ShowFileDialog(
					GetHostWindow()->GetNativeWindow(),
					fileNames,
					filterIndex,
					(enabledPreview ? INativeDialogService::FileDialogSavePreview : INativeDialogService::FileDialogSave),
					title,
					fileName,
					directory,
					defaultExtension,
					filter,
					options))
				{
					return false;
				}

				if (fileNames.Count() > 0)
				{
					fileName = fileNames[0];
					FileNameChanged.Execute(GuiEventArgs());
					FilterIndexChanged.Execute(GuiEventArgs());
				}
				return true;
			}
		}
	}
}