/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDIALOGS
#define VCZH_PRESENTATION_CONTROLS_GUIDIALOGS

#include "GuiBasicControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiWindow;

/***********************************************************************
Dialogs
***********************************************************************/

			/// <summary>Base class for dialogs.</summary>
			class GuiDialogBase abstract : public GuiComponent, public Description<GuiDialogBase>
			{
			protected:
				GuiInstanceRootObject*								rootObject = nullptr;

				GuiWindow*											GetHostWindow();
			public:
				GuiDialogBase();
				~GuiDialogBase();

				void												Attach(GuiInstanceRootObject* _rootObject);
				void												Detach(GuiInstanceRootObject* _rootObject);
			};
			
			/// <summary>Message dialog.</summary>
			class GuiMessageDialog : public GuiDialogBase, public Description<GuiMessageDialog>
			{
			protected:
				INativeDialogService::MessageBoxButtonsInput		input = INativeDialogService::DisplayOK;
				INativeDialogService::MessageBoxDefaultButton		defaultButton = INativeDialogService::DefaultFirst;
				INativeDialogService::MessageBoxIcons				icon = INativeDialogService::IconNone;
				INativeDialogService::MessageBoxModalOptions		modalOption = INativeDialogService::ModalWindow;
				WString												text;
				WString												title;

			public:
				/// <summary>Create a message dialog.</summary>
				GuiMessageDialog();
				~GuiMessageDialog();

				/// <summary>Get the button combination that appear on the dialog.</summary>
				/// <returns>The button combination.</returns>
				INativeDialogService::MessageBoxButtonsInput		GetInput();
				/// <summary>Set the button combination that appear on the dialog.</summary>
				/// <param name="value">The button combination.</param>
				void												SetInput(INativeDialogService::MessageBoxButtonsInput value);
				
				/// <summary>Get the default button for the selected button combination.</summary>
				/// <returns>The default button.</returns>
				INativeDialogService::MessageBoxDefaultButton		GetDefaultButton();
				/// <summary>Set the default button for the selected button combination.</summary>
				/// <param name="value">The default button.</param>
				void												SetDefaultButton(INativeDialogService::MessageBoxDefaultButton value);

				/// <summary>Get the icon that appears on the dialog.</summary>
				/// <returns>The icon.</returns>
				INativeDialogService::MessageBoxIcons				GetIcon();
				/// <summary>Set the icon that appears on the dialog.</summary>
				/// <param name="value">The icon.</param>
				void												SetIcon(INativeDialogService::MessageBoxIcons value);

				/// <summary>Get the way that how this dialog disable windows of the current process.</summary>
				/// <returns>The way that how this dialog disable windows of the current process.</returns>
				INativeDialogService::MessageBoxModalOptions		GetModalOption();
				/// <summary>Set the way that how this dialog disable windows of the current process.</summary>
				/// <param name="value">The way that how this dialog disable windows of the current process.</param>
				void												SetModalOption(INativeDialogService::MessageBoxModalOptions value);

				/// <summary>Get the text for the dialog.</summary>
				/// <returns>The text.</returns>
				const WString&										GetText();
				/// <summary>Set the text for the dialog.</summary>
				/// <param name="value">The text.</param>
				void												SetText(const WString& value);

				/// <summary>Get the title for the dialog.</summary>
				/// <returns>The title.</returns>
				const WString&										GetTitle();
				/// <summary>Set the title for the dialog. If the title is empty, the dialog will use the title of the window that host this dialog.</summary>
				/// <param name="value">The title.</param>
				void												SetTitle(const WString& value);
				
				/// <summary>Show the dialog.</summary>
				/// <returns>Returns the clicked button.</returns>
				INativeDialogService::MessageBoxButtonsOutput		ShowDialog();
			};
			
			/// <summary>Color dialog.</summary>
			class GuiColorDialog : public GuiDialogBase, public Description<GuiColorDialog>
			{
			protected:
				bool												enabledCustomColor = true;
				bool												openedCustomColor = false;
				Color												selectedColor;
				bool												showSelection = true;
				collections::List<Color>							customColors;

			public:
				/// <summary>Create a color dialog.</summary>
				GuiColorDialog();
				~GuiColorDialog();

				/// <summary>Selected color changed event.</summary>
				compositions::GuiNotifyEvent						SelectedColorChanged;
				
				/// <summary>Get if the custom color panel is enabled for the dialog.</summary>
				/// <returns>Returns true if the color panel is enabled for the dialog.</returns>
				bool												GetEnabledCustomColor();
				/// <summary>Set if custom color panel is enabled for the dialog.</summary>
				/// <param name="value">Set to true to enable the custom color panel for the dialog.</param>
				void												SetEnabledCustomColor(bool value);
				
				/// <summary>Get if the custom color panel is opened by default when it is enabled.</summary>
				/// <returns>Returns true if the custom color panel is opened by default.</returns>
				bool												GetOpenedCustomColor();
				/// <summary>Set if the custom color panel is opened by default when it is enabled.</summary>
				/// <param name="value">Set to true to open custom color panel by default if it is enabled.</param>
				void												SetOpenedCustomColor(bool value);
				
				/// <summary>Get the selected color.</summary>
				/// <returns>The selected color.</returns>
				Color												GetSelectedColor();
				/// <summary>Set the selected color.</summary>
				/// <param name="value">The selected color.</param>
				void												SetSelectedColor(Color value);
				
				/// <summary>Get the list to access 16 selected custom colors on the palette. Colors in the list is guaranteed to have exactly 16 items after the dialog is closed.</summary>
				/// <returns>The list to access custom colors on the palette.</returns>
				collections::List<Color>&							GetCustomColors();
				
				/// <summary>Show the dialog.</summary>
				/// <returns>Returns true if the "OK" button is clicked.</returns>
				bool												ShowDialog();
			};
			
			/// <summary>Font dialog.</summary>
			class GuiFontDialog : public GuiDialogBase, public Description<GuiFontDialog>
			{
			protected:
				FontProperties										selectedFont;
				Color												selectedColor;
				bool												showSelection = true;
				bool												showEffect = true;
				bool												forceFontExist = true;

			public:
				/// <summary>Create a font dialog.</summary>
				GuiFontDialog();
				~GuiFontDialog();

				/// <summary>Selected font changed event.</summary>
				compositions::GuiNotifyEvent						SelectedFontChanged;
				/// <summary>Selected color changed event.</summary>
				compositions::GuiNotifyEvent						SelectedColorChanged;
				
				/// <summary>Get the selected font.</summary>
				/// <returns>The selected font.</returns>
				const FontProperties&								GetSelectedFont();
				/// <summary>Set the selected font.</summary>
				/// <param name="value">The selected font.</param>
				void												SetSelectedFont(const FontProperties& value);
				
				/// <summary>Get the selected color.</summary>
				/// <returns>The selected color.</returns>
				Color												GetSelectedColor();
				/// <summary>Set the selected color.</summary>
				/// <param name="value">The selected color.</param>
				void												SetSelectedColor(Color value);
				
				/// <summary>Get if the selected font is already selected on the dialog when it is opened.</summary>
				/// <returns>Returns true if the selected font is already selected on the dialog when it is opened.</returns>
				bool												GetShowSelection();
				/// <summary>Set if the selected font is already selected on the dialog when it is opened.</summary>
				/// <param name="value">Set to true to select the selected font when the dialog is opened.</param>
				void												SetShowSelection(bool value);
				
				/// <summary>Get if the font preview is enabled.</summary>
				/// <returns>Returns true if the font preview is enabled.</returns>
				bool												GetShowEffect();
				/// <summary>Set if the font preview is enabled.</summary>
				/// <param name="value">Set to true to enable the font preview.</param>
				void												SetShowEffect(bool value);
				
				/// <summary>Get if the dialog only accepts an existing font.</summary>
				/// <returns>Returns true if the dialog only accepts an existing font.</returns>
				bool												GetForceFontExist();
				/// <summary>Set if the dialog only accepts an existing font.</summary>
				/// <param name="value">Set to true to let the dialog only accept an existing font.</param>
				void												SetForceFontExist(bool value);
				
				/// <summary>Show the dialog.</summary>
				/// <returns>Returns true if the "OK" button is clicked.</returns>
				bool												ShowDialog();
			};
			
			/// <summary>Base class for file dialogs.</summary>
			class GuiFileDialogBase abstract : public GuiDialogBase, public Description<GuiFileDialogBase>
			{
			protected:
				WString												filter = L"All Files (*.*)|*.*";
				vint												filterIndex = 0;
				bool												enabledPreview = false;
				WString												title;
				WString												fileName;
				WString												directory;
				WString												defaultExtension;
				INativeDialogService::FileDialogOptions				options;

			public:
				GuiFileDialogBase();
				~GuiFileDialogBase();

				/// <summary>File name changed event.</summary>
				compositions::GuiNotifyEvent						FileNameChanged;
				/// <summary>Filter index changed event.</summary>
				compositions::GuiNotifyEvent						FilterIndexChanged;
				
				/// <summary>Get the filter.</summary>
				/// <returns>The filter.</returns>
				const WString&										GetFilter();
				/// <summary>Set the filter. The filter is formed by pairs of filter name and wildcard concatenated by "|", like "Text Files (*.txt)|*.txt|All Files (*.*)|*.*".</summary>
				/// <param name="value">The filter.</param>
				void												SetFilter(const WString& value);
				
				/// <summary>Get the filter index.</summary>
				/// <returns>The filter index.</returns>
				vint												GetFilterIndex();
				/// <summary>Set the filter index.</summary>
				/// <param name="value">The filter index.</param>
				void												SetFilterIndex(vint value);
				
				/// <summary>Get if the file preview is enabled.</summary>
				/// <returns>Returns true if the file preview is enabled.</returns>
				bool												GetEnabledPreview();
				/// <summary>Set if the file preview is enabled.</summary>
				/// <param name="value">Set to true to enable the file preview.</param>
				void												SetEnabledPreview(bool value);
				
				/// <summary>Get the title.</summary>
				/// <returns>The title.</returns>
				WString												GetTitle();
				/// <summary>Set the title.</summary>
				/// <param name="value">The title.</param>
				void												SetTitle(const WString& value);
				
				/// <summary>Get the selected file name.</summary>
				/// <returns>The selected file name.</returns>
				WString												GetFileName();
				/// <summary>Set the selected file name.</summary>
				/// <param name="value">The selected file name.</param>
				void												SetFileName(const WString& value);
				
				/// <summary>Get the default folder.</summary>
				/// <returns>The default folder.</returns>
				WString												GetDirectory();
				/// <summary>Set the default folder.</summary>
				/// <param name="value">The default folder.</param>
				void												SetDirectory(const WString& value);
				
				/// <summary>Get the default file extension.</summary>
				/// <returns>The default file extension.</returns>
				WString												GetDefaultExtension();
				/// <summary>Set the default file extension like "txt". If the user does not specify a file extension, the default file extension will be appended using "." after the file name.</summary>
				/// <param name="value">The default file extension.</param>
				void												SetDefaultExtension(const WString& value);
				
				/// <summary>Get the dialog options.</summary>
				/// <returns>The dialog options.</returns>
				INativeDialogService::FileDialogOptions				GetOptions();
				/// <summary>Set the dialog options.</summary>
				/// <param name="value">The dialog options.</param>
				void												SetOptions(INativeDialogService::FileDialogOptions value);
			};
			
			/// <summary>Open file dialog.</summary>
			class GuiOpenFileDialog : public GuiFileDialogBase, public Description<GuiOpenFileDialog>
			{
			protected:
				collections::List<WString>							fileNames;

			public:
				/// <summary>Create a open file dialog.</summary>
				GuiOpenFileDialog();
				~GuiOpenFileDialog();
				
				/// <summary>Get the list to access multiple selected file names.</summary>
				/// <returns>The list to access multiple selected file names.</returns>
				collections::List<WString>&							GetFileNames();
				
				/// <summary>Show the dialog.</summary>
				/// <returns>Returns true if the "Open" button is clicked.</returns>
				bool												ShowDialog();
			};
			
			/// <summary>Save file dialog.</summary>
			class GuiSaveFileDialog : public GuiFileDialogBase, public Description<GuiSaveFileDialog>
			{
			public:
				/// <summary>Create a save file dialog.</summary>
				GuiSaveFileDialog();
				~GuiSaveFileDialog();

				/// <summary>Show the dialog.</summary>
				/// <returns>Returns true if the "Save" button is clicked.</returns>
				bool												ShowDialog();
			};
		}
	}
}

#endif
