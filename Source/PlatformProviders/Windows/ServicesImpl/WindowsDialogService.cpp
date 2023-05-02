#include "WindowsDialogService.h"
#include <Vfw.h>

#pragma comment(lib, "Vfw32.lib")

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			using namespace collections;

/***********************************************************************
WindowsDialogService
***********************************************************************/

			WindowsDialogService::WindowsDialogService(HandleRetriver _handleRetriver)
				:handleRetriver(_handleRetriver)
			{
			}

			INativeDialogService::MessageBoxButtonsOutput WindowsDialogService::ShowMessageBox(
				INativeWindow* window,
				const WString& text,
				const WString& title,
				MessageBoxButtonsInput buttons,
				MessageBoxDefaultButton defaultButton,
				MessageBoxIcons icon,
				MessageBoxModalOptions modal)
			{
				WString realTitle=title;
				if(title==L"" && window!=0)
				{
					realTitle=window->GetTitle();
				}
				HWND hWnd=handleRetriver(window);
				LPCTSTR lpText=text.Buffer();
				LPCTSTR lpCaption=realTitle.Buffer();
				UINT uType=0;
				
#define MAP(A, B) case A: uType|=(B); break
				switch(buttons)
				{
					MAP(DisplayOK, MB_OK);
					MAP(DisplayOKCancel, MB_OKCANCEL);
					MAP(DisplayYesNo, MB_YESNO);
					MAP(DisplayYesNoCancel, MB_YESNOCANCEL);
					MAP(DisplayRetryCancel, MB_RETRYCANCEL);
					MAP(DisplayAbortRetryIgnore, MB_ABORTRETRYIGNORE);
					MAP(DisplayCancelTryAgainContinue, MB_CANCELTRYCONTINUE);
				}
				switch(defaultButton)
				{
					MAP(DefaultFirst, MB_DEFBUTTON1);
					MAP(DefaultSecond, MB_DEFBUTTON2);
					MAP(DefaultThird, MB_DEFBUTTON3);
				}
				switch(icon)
				{
					MAP(IconError, MB_ICONERROR);
					MAP(IconQuestion, MB_ICONQUESTION);
					MAP(IconWarning, MB_ICONWARNING);
					MAP(IconInformation, MB_ICONINFORMATION);
				}
				switch(modal)
				{
					MAP(ModalWindow, MB_APPLMODAL);
					MAP(ModalSystem, MB_SYSTEMMODAL);
					MAP(ModalTask, MB_TASKMODAL);
				}
#undef MAP

				vint result=MessageBox(hWnd, lpText, lpCaption, uType);
				switch(result)
				{
				case IDABORT: return SelectAbort;
				case IDCANCEL: return SelectCancel;
				case IDCONTINUE: return SelectContinue;
				case IDIGNORE: return SelectIgnore;
				case IDNO: return SelectNo;
				case IDOK: return SelectOK;
				case IDRETRY: return SelectRetry;
				case IDTRYAGAIN: return SelectTryAgain;
				case IDYES: return SelectYes;
				default: return SelectOK;
				}
			}

			bool WindowsDialogService::ShowColorDialog(INativeWindow* window, Color& selection, bool selected, ColorDialogCustomColorOptions customColorOptions, Color* customColors)
			{
				CHOOSECOLOR chooseColor;
				ZeroMemory(&chooseColor, sizeof(chooseColor));
				COLORREF customColorsBuffer[16]={0};
				if(customColors)
				{
					for(vint i=0;i<sizeof(customColorsBuffer)/sizeof(*customColorsBuffer);i++)
					{
						customColorsBuffer[i]=RGB(customColors[i].r, customColors[i].g, customColors[i].b);
					}
				}

				chooseColor.lStructSize=sizeof(chooseColor);
				chooseColor.hwndOwner=handleRetriver(window);
				chooseColor.rgbResult=RGB(selection.r, selection.g, selection.b);
				chooseColor.lpCustColors=customColorsBuffer;
				chooseColor.Flags=CC_ANYCOLOR;
				
#define MAP(A, B) case A: chooseColor.Flags|=(B); break
				switch(customColorOptions)
				{
					MAP(CustomColorDisabled, CC_PREVENTFULLOPEN);
					MAP(CustomColorOpened, CC_FULLOPEN);
				}
#undef MAP
				if(selected)
				{
					chooseColor.Flags|=CC_RGBINIT;
				}

				BOOL result=ChooseColor(&chooseColor);
				if(result)
				{
					selection=Color(GetRValue(chooseColor.rgbResult), GetGValue(chooseColor.rgbResult), GetBValue(chooseColor.rgbResult));
					if(customColors)
					{
						for(vint i=0;i<sizeof(customColorsBuffer)/sizeof(*customColorsBuffer);i++)
						{
							COLORREF color=customColorsBuffer[i];
							customColors[i]=Color(GetRValue(color), GetGValue(color), GetBValue(color));
						}
					}
				}
				return result!=FALSE;
			}

			bool WindowsDialogService::ShowFontDialog(INativeWindow* window, FontProperties& selectionFont, Color& selectionColor, bool selected, bool showEffect, bool forceFontExist)
			{
				LOGFONT logFont;
				ZeroMemory(&logFont, sizeof(logFont));
				logFont.lfHeight=-(int)selectionFont.size;
				logFont.lfWeight=selectionFont.bold?FW_BOLD:FW_REGULAR;
				logFont.lfItalic=selectionFont.italic?TRUE:FALSE;
				logFont.lfUnderline=selectionFont.underline?TRUE:FALSE;
				logFont.lfStrikeOut=selectionFont.strikeline?TRUE:FALSE;
				wcscpy_s(logFont.lfFaceName, selectionFont.fontFamily.Buffer());

				CHOOSEFONT chooseFont;
				ZeroMemory(&chooseFont, sizeof(chooseFont));
				chooseFont.lStructSize=sizeof(chooseFont);
				chooseFont.hwndOwner=handleRetriver(window);
				chooseFont.lpLogFont=&logFont;
				chooseFont.iPointSize=0;
				chooseFont.Flags=(showEffect?CF_EFFECTS:0) | (forceFontExist?CF_FORCEFONTEXIST:0) | (selected?CF_INITTOLOGFONTSTRUCT:0);
				chooseFont.rgbColors=RGB(selectionColor.r, selectionColor.g, selectionColor.b);

				BOOL result=ChooseFont(&chooseFont);
				if(result)
				{
					selectionFont.fontFamily=logFont.lfFaceName;
					selectionFont.bold=logFont.lfWeight>FW_REGULAR;
					selectionFont.italic=logFont.lfItalic!=FALSE;
					selectionFont.underline=logFont.lfUnderline!=FALSE;
					selectionFont.strikeline=logFont.lfStrikeOut!=FALSE;
					selectionFont.size=-logFont.lfHeight;

					selectionColor=Color(GetRValue(chooseFont.rgbColors), GetGValue(chooseFont.rgbColors), GetBValue(chooseFont.rgbColors));
				}
				return result!=FALSE;
			}

			bool WindowsDialogService::ShowFileDialog(INativeWindow* window, collections::List<WString>& selectionFileNames, vint& selectionFilterIndex, FileDialogTypes dialogType, const WString& title, const WString& initialFileName, const WString& initialDirectory, const WString& defaultExtension, const WString& filter, FileDialogOptions options)
			{
				Array<wchar_t> fileNamesBuffer(65536>initialFileName.Length()+1?65536:initialFileName.Length()+1);
				wcscpy_s(&fileNamesBuffer[0], fileNamesBuffer.Count(), initialFileName.Buffer());

				OPENFILENAME ofn;
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize=sizeof(ofn);
				ofn.hwndOwner=handleRetriver(window);
				ofn.hInstance=NULL;
				ofn.lpstrCustomFilter=NULL;
				ofn.nMaxCustFilter=0;
				ofn.nFilterIndex=(int)selectionFilterIndex+1;
				ofn.lpstrFile=&fileNamesBuffer[0];
				ofn.nMaxFile=(int)fileNamesBuffer.Count();
				ofn.lpstrFileTitle=NULL;
				ofn.nMaxFileTitle=0;
				ofn.lpstrInitialDir=initialDirectory==L""?NULL:initialDirectory.Buffer();
				ofn.lpstrTitle=title==L""?NULL:title.Buffer();
				ofn.lpstrDefExt=defaultExtension==L""?NULL:defaultExtension.Buffer();

				List<vint> filterSeparators;
				for(vint i=0;i<filter.Length();i++)
				{
					if(filter[i]==L'|')
					{
						filterSeparators.Add(i);
					}
				}
				if(filterSeparators.Count()%2==1)
				{
					filterSeparators.Add(filter.Length());
				}

				Array<wchar_t> filterBuffer(filter.Length()+2);
				vint index=0;
				// TODO: (enumerable) foreach
				for(vint i=0;i<filterSeparators.Count();i++)
				{
					vint end=filterSeparators[i];
					wcsncpy_s(&filterBuffer[index], filterBuffer.Count()-index, filter.Buffer()+index, end-index);
					filterBuffer[end]=0;
					index=end+1;
				}
				filterBuffer[index]=0;
				ofn.lpstrFilter=&filterBuffer[0];

				ofn.Flags=OFN_ENABLESIZING | OFN_EXPLORER | OFN_LONGNAMES;
				if(options&FileDialogAllowMultipleSelection)	ofn.Flags|=OFN_ALLOWMULTISELECT;
				if(options&FileDialogFileMustExist)				ofn.Flags|=OFN_FILEMUSTEXIST;
				if(!(options&FileDialogShowReadOnlyCheckBox))	ofn.Flags|=OFN_HIDEREADONLY;
				if(!(options&FileDialogDereferenceLinks))		ofn.Flags|=OFN_NODEREFERENCELINKS;
				if(!(options&FileDialogShowNetworkButton))		ofn.Flags|=OFN_NONETWORKBUTTON;
				if(options&FileDialogPromptCreateFile)			ofn.Flags|=OFN_CREATEPROMPT;
				if(options&FileDialogPromptOverwriteFile)		ofn.Flags|=OFN_OVERWRITEPROMPT;
				if(options&FileDialogDirectoryMustExist)		ofn.Flags|=OFN_PATHMUSTEXIST;
				if(!(options&FileDialogAddToRecent))			ofn.Flags|=OFN_DONTADDTORECENT;

				BOOL result=FALSE;
				switch(dialogType)
				{
				case FileDialogOpen:
					result=GetOpenFileName(&ofn);
					break;
				case FileDialogOpenPreview:
					result=GetOpenFileNamePreview(&ofn);
					break;
				case FileDialogSave:
					result=GetSaveFileName(&ofn);
					break;
				case FileDialogSavePreview:
					result=GetSaveFileNamePreview(&ofn);
					break;
				}
				if(result)
				{
					selectionFilterIndex=ofn.nFilterIndex-1;
					selectionFileNames.Clear();
					if(options&FileDialogAllowMultipleSelection)
					{
						const wchar_t* reading=&fileNamesBuffer[0];
						WString directory=reading;
						reading+=directory.Length()+1;
						while(*reading)
						{
							WString fileName=reading;
							reading+=fileName.Length()+1;
							selectionFileNames.Add(directory+L"\\"+fileName);
						}
						if(selectionFileNames.Count()==0)
						{
							selectionFileNames.Add(directory);
						}
					}
					else
					{
						selectionFileNames.Add(&fileNamesBuffer[0]);
					}
				}
				return result!=FALSE;
			}
		}
	}
}