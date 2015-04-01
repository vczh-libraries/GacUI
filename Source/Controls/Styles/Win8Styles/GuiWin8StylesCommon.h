/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows8 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWIN8STYLESCOMMON
#define VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWIN8STYLESCOMMON

#include "../GuiCommonStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win8
		{

/***********************************************************************
Button Configuration
***********************************************************************/
			
			struct Win8ButtonColors
			{
				Color										borderColor;
				Color										g1;
				Color										g2;
				Color										textColor;
				Color										bullet;

				bool operator==(const Win8ButtonColors& colors)
				{
					return
						borderColor == colors.borderColor &&
						g1 == colors.g1 &&
						g2 == colors.g2 &&
						textColor == colors.textColor &&
						bullet == colors.bullet;
				}

				bool operator!=(const Win8ButtonColors& colors)
				{
					return !(*this==colors);
				}

				void										SetAlphaWithoutText(unsigned char a);

				static Win8ButtonColors						Blend(const Win8ButtonColors& c1, const Win8ButtonColors& c2, vint ratio, vint total);

				static Win8ButtonColors						ButtonNormal();
				static Win8ButtonColors						ButtonActive();
				static Win8ButtonColors						ButtonPressed();
				static Win8ButtonColors						ButtonDisabled();
				
				static Win8ButtonColors						ItemNormal();
				static Win8ButtonColors						ItemActive();
				static Win8ButtonColors						ItemSelected();
				static Win8ButtonColors						ItemDisabled();
				
				static Win8ButtonColors						CheckedNormal(bool selected);
				static Win8ButtonColors						CheckedActive(bool selected);
				static Win8ButtonColors						CheckedPressed(bool selected);
				static Win8ButtonColors						CheckedDisabled(bool selected);

				static Win8ButtonColors						ToolstripButtonNormal();
				static Win8ButtonColors						ToolstripButtonActive();
				static Win8ButtonColors						ToolstripButtonPressed();
				static Win8ButtonColors						ToolstripButtonSelected();
				static Win8ButtonColors						ToolstripButtonDisabled();

				static Win8ButtonColors						ScrollHandleNormal();
				static Win8ButtonColors						ScrollHandleActive();
				static Win8ButtonColors						ScrollHandlePressed();
				static Win8ButtonColors						ScrollHandleDisabled();
				static Win8ButtonColors						ScrollArrowNormal();
				static Win8ButtonColors						ScrollArrowActive();
				static Win8ButtonColors						ScrollArrowPressed();
				static Win8ButtonColors						ScrollArrowDisabled();

				static Win8ButtonColors						MenuBarButtonNormal();
				static Win8ButtonColors						MenuBarButtonActive();
				static Win8ButtonColors						MenuBarButtonPressed();
				static Win8ButtonColors						MenuBarButtonDisabled();

				static Win8ButtonColors						MenuItemButtonNormal();
				static Win8ButtonColors						MenuItemButtonNormalActive();
				static Win8ButtonColors						MenuItemButtonSelected();
				static Win8ButtonColors						MenuItemButtonSelectedActive();
				static Win8ButtonColors						MenuItemButtonDisabled();
				static Win8ButtonColors						MenuItemButtonDisabledActive();

				static Win8ButtonColors						TabPageHeaderNormal();
				static Win8ButtonColors						TabPageHeaderActive();
				static Win8ButtonColors						TabPageHeaderSelected();
			};

			struct Win8ButtonElements
			{
				elements::GuiSolidBorderElement*			rectBorderElement;
				elements::GuiGradientBackgroundElement*		backgroundElement;
				elements::GuiSolidLabelElement*				textElement;
				compositions::GuiBoundsComposition*			textComposition;
				compositions::GuiBoundsComposition*			mainComposition;
				compositions::GuiBoundsComposition*			backgroundComposition;

				static Win8ButtonElements					Create(Alignment horizontal=Alignment::Center, Alignment vertical=Alignment::Center);
				void										Apply(const Win8ButtonColors& colors);
			};

			struct Win8CheckedButtonElements
			{
				elements::GuiSolidBorderElement*			bulletBorderElement;
				elements::GuiGradientBackgroundElement*		bulletBackgroundElement;
				elements::GuiSolidLabelElement*				bulletCheckElement;
				elements::GuiSolidBackgroundElement*		bulletRadioElement;
				elements::GuiSolidLabelElement*				textElement;
				compositions::GuiBoundsComposition*			textComposition;
				compositions::GuiBoundsComposition*			mainComposition;

				static Win8CheckedButtonElements			Create(elements::ElementShape shape, bool backgroundVisible);
				void										Apply(const Win8ButtonColors& colors);
			};

			struct Win8MenuItemButtonElements
			{
				elements::GuiSolidBorderElement*			borderElement;
				elements::GuiGradientBackgroundElement*		backgroundElement;
				elements::GuiSolidBorderElement*			splitterElement;
				compositions::GuiCellComposition*			splitterComposition;
				elements::GuiImageFrameElement*				imageElement;
				elements::GuiSolidLabelElement*				textElement;
				compositions::GuiSharedSizeItemComposition*	textComposition;
				elements::GuiSolidLabelElement*				shortcutElement;
				compositions::GuiSharedSizeItemComposition*	shortcutComposition;
				elements::GuiPolygonElement*				subMenuArrowElement;
				compositions::GuiGraphicsComposition*		subMenuArrowComposition;
				compositions::GuiBoundsComposition*			mainComposition;

				static Win8MenuItemButtonElements			Create();
				void										Apply(const Win8ButtonColors& colors);
				void										SetActive(bool value);
				void										SetSubMenuExisting(bool value);
			};

			struct Win8TextBoxColors
			{
				Color										borderColor;
				Color										backgroundColor;

				bool operator==(const Win8TextBoxColors& colors)
				{
					return
						borderColor == colors.borderColor &&
						backgroundColor == colors.backgroundColor;
				}

				bool operator!=(const Win8TextBoxColors& colors)
				{
					return !(*this==colors);
				}

				static Win8TextBoxColors					Blend(const Win8TextBoxColors& c1, const Win8TextBoxColors& c2, vint ratio, vint total);

				static Win8TextBoxColors					Normal();
				static Win8TextBoxColors					Active();
				static Win8TextBoxColors					Focused();
				static Win8TextBoxColors					Disabled();
			};

/***********************************************************************
Helper Functions
***********************************************************************/
			
			extern Color									Win8GetSystemWindowColor();
			extern Color									Win8GetSystemTabContentColor();
			extern Color									Win8GetSystemBorderColor();
			extern Color									Win8GetSystemTextColor(bool enabled);
			extern Color									Win8GetMenuBorderColor();
			extern Color									Win8GetMenuSplitterColor();
			extern void										Win8SetFont(elements::GuiSolidLabelElement* element, compositions::GuiBoundsComposition* composition, const FontProperties& fontProperties);
			extern void										Win8CreateSolidLabelElement(elements::GuiSolidLabelElement*& element, compositions::GuiBoundsComposition*& composition, Alignment horizontal, Alignment vertical);
			extern void										Win8CreateSolidLabelElement(elements::GuiSolidLabelElement*& element, compositions::GuiSharedSizeItemComposition*& composition, const WString& group, Alignment horizontal, Alignment vertical);
			extern elements::text::ColorEntry				Win8GetTextBoxTextColor();
		}
	}
}

#endif