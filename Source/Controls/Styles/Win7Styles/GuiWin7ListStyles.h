/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows7 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN7STYLES_GUIWIN7LISTSTYLES
#define VCZH_PRESENTATION_CONTROLS_WIN7STYLES_GUIWIN7LISTSTYLES

#include "GuiWin7StylesCommon.h"
#include "GuiWin7ButtonStyles.h"
#include "GuiWin7ScrollableStyles.h"
#include "GuiWin7ControlStyles.h"
#include "GuiWin7MenuStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win7
		{

/***********************************************************************
List Control Buttons
***********************************************************************/
			
			/// <summary>Selectable item style (Windows 7). Generally for list box item background.</summary>
			class Win7SelectableItemStyle : public Win7ButtonStyleBase, public Description<Win7SelectableItemStyle>
			{
			protected:
				void										TransferInternal(controls::ButtonState value, bool enabled, bool selected)override;
			public:
				/// <summary>Create the style.</summary>
				Win7SelectableItemStyle();
				~Win7SelectableItemStyle();
			};
			
			/// <summary>List view column drop down button style (Windows 7).</summary>
			class Win7ListViewColumnDropDownStyle : public Object, public virtual controls::GuiSelectableButton::IStyleController, public Description<Win7ListViewColumnDropDownStyle>
			{
			protected:
				controls::ButtonState						controlStyle;
				bool										isVisuallyEnabled;
				bool										isSelected;

				compositions::GuiBoundsComposition*			mainComposition;
				compositions::GuiBoundsComposition*			leftBorderComposition;
				compositions::GuiBoundsComposition*			borderComposition;
				compositions::GuiBoundsComposition*			gradientComposition;
				compositions::GuiBoundsComposition*			arrowComposition;

				elements::GuiGradientBackgroundElement*		leftBorderElement;
				elements::GuiSolidBorderElement*			borderElement;
				elements::GuiGradientBackgroundElement*		gradientElement;
				elements::GuiPolygonElement*				arrowElement;

				void										TransferInternal(controls::ButtonState value, bool enabled, bool selected);
			public:
				/// <summary>Create the style.</summary>
				Win7ListViewColumnDropDownStyle();
				~Win7ListViewColumnDropDownStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				void										SetSelected(bool value)override;
				void										Transfer(controls::ButtonState value)override;
			};
			
			/// <summary>List view column header style (Windows 7).</summary>
			class Win7ListViewColumnHeaderStyle : public Object, public virtual controls::GuiListViewColumnHeader::IStyleController, public Description<Win7ListViewColumnHeaderStyle>
			{
			protected:
				controls::ButtonState						controlStyle;
				bool										isVisuallyEnabled;
				bool										isSubMenuExisting;
				bool										isSubMenuOpening;

				compositions::GuiBoundsComposition*			mainComposition;
				compositions::GuiBoundsComposition*			rightBorderComposition;
				compositions::GuiBoundsComposition*			borderComposition;
				compositions::GuiBoundsComposition*			gradientComposition;
				compositions::GuiBoundsComposition*			textComposition;
				compositions::GuiBoundsComposition*			arrowComposition;

				elements::GuiSolidBackgroundElement*		backgroundElement;
				elements::GuiGradientBackgroundElement*		rightBorderElement;
				elements::GuiSolidBorderElement*			borderElement;
				elements::GuiGradientBackgroundElement*		gradientElement;
				elements::GuiSolidLabelElement*				textElement;
				elements::GuiPolygonElement*				arrowElement;

				controls::GuiButton*						dropdownButton;

				void										TransferInternal(controls::ButtonState value, bool enabled, bool subMenuExisting, bool subMenuOpening);
			public:
				/// <summary>Create the style.</summary>
				Win7ListViewColumnHeaderStyle();
				~Win7ListViewColumnHeaderStyle();

				compositions::GuiBoundsComposition*							GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*						GetContainerComposition()override;
				void														SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void														SetText(const WString& value)override;
				void														SetFont(const FontProperties& value)override;
				void														SetVisuallyEnabled(bool value)override;
				void														SetSelected(bool value)override;
				void														Transfer(controls::ButtonState value)override;
				controls::GuiMenu::IStyleController*						CreateSubMenuStyleController()override;
				void														SetSubMenuExisting(bool value)override;
				void														SetSubMenuOpening(bool value)override;
				controls::GuiButton*										GetSubMenuHost()override;
				void														SetImage(Ptr<GuiImageData> value)override;
				void														SetShortcutText(const WString& value)override;
				void														SetColumnSortingState(controls::ColumnSortingState value)override;
			};
			
			/// <summary>Tree view expanding button style (Windows 7). Show the triangle to indicate the expanding state of a tree view item.</summary>
			class Win7TreeViewExpandingButtonStyle : public Object, public virtual controls::GuiSelectableButton::IStyleController, public Description<Win7TreeViewExpandingButtonStyle>
			{
			protected:
				controls::ButtonState						controlStyle;
				bool										isVisuallyEnabled;
				bool										isSelected;

				compositions::GuiBoundsComposition*			mainComposition;
				elements::GuiPolygonElement*				polygonElement;

				void										TransferInternal(controls::ButtonState value, bool enabled, bool selected);
			public:
				/// <summary>Create the style.</summary>
				Win7TreeViewExpandingButtonStyle();
				~Win7TreeViewExpandingButtonStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				void										SetSelected(bool value)override;
				void										Transfer(controls::ButtonState value)override;
			};

/***********************************************************************
ComboBox
***********************************************************************/
			
#pragma warning(push)
#pragma warning(disable:4250)
			/// <summary>Drop down combo box style (Windows 7).</summary>
			class Win7DropDownComboBoxStyle : public Win7ButtonStyle, public virtual controls::GuiComboBoxListControl::IStyleController, public Description<Win7DropDownComboBoxStyle>
			{
			protected:
				controls::IComboBoxCommandExecutor*				commandExecutor;
				compositions::GuiTableComposition*				table;
				compositions::GuiCellComposition*				textComposition;
				compositions::GuiCellComposition*				dropDownComposition;
				elements::GuiPolygonElement*					dropDownElement;
				WString											text;
				bool											textVisible;

				void											TransferInternal(controls::ButtonState value, bool enabled, bool selected)override;
				void											AfterApplyColors(const Win7ButtonColors& colors)override;
			public:
				/// <summary>Create the style.</summary>
				Win7DropDownComboBoxStyle();
				~Win7DropDownComboBoxStyle();
				
				compositions::GuiGraphicsComposition*						GetContainerComposition()override;
				
				controls::GuiMenu::IStyleController*						CreateSubMenuStyleController()override;
				void														SetSubMenuExisting(bool value)override;
				void														SetSubMenuOpening(bool value)override;
				controls::GuiButton*										GetSubMenuHost()override;
				void														SetImage(Ptr<GuiImageData> value)override;
				void														SetShortcutText(const WString& value)override;
				void														SetCommandExecutor(controls::IComboBoxCommandExecutor* value)override;
				void														OnItemSelected()override;
				void														SetText(const WString& value)override;
				void														SetTextVisible(bool value)override;
			};
#pragma warning(pop)

/***********************************************************************
List
***********************************************************************/

#pragma warning(push)
#pragma warning(disable:4250)
			
			/// <summary>Multiline text box style (Windows 7).</summary>
			class Win7TextListProvider : public Win7MultilineTextBoxProvider, public virtual controls::GuiVirtualTextList::IStyleProvider, public Description<Win7TextListProvider>
			{
			public:
				/// <summary>Create the style.</summary>
				Win7TextListProvider();
				~Win7TextListProvider();

				virtual Color												GetTextColor()override;
			};

			/// <summary>List view style (Windows 7).</summary>
			class Win7ListViewProvider : public Win7MultilineTextBoxProvider, public virtual controls::GuiListView::IStyleProvider, public Description<Win7ListViewProvider>
			{
			public:
				/// <summary>Create the style.</summary>
				Win7ListViewProvider();
				~Win7ListViewProvider();

				controls::GuiSelectableButton::IStyleController*		CreateItemBackground()override;
				controls::GuiListViewColumnHeader::IStyleController*	CreateColumnStyle()override;
				Color													GetPrimaryTextColor()override;
				Color													GetSecondaryTextColor()override;
				Color													GetItemSeparatorColor()override;
			};
			
			/// <summary>Tree view style (Windows 7).</summary>
			class Win7TreeViewProvider : public Win7MultilineTextBoxProvider, public virtual controls::GuiTreeView::IStyleProvider, public Description<Win7TreeViewProvider>
			{
			public:
				/// <summary>Create the style.</summary>
				Win7TreeViewProvider();
				~Win7TreeViewProvider();

				controls::GuiSelectableButton::IStyleController*		CreateItemBackground()override;
				controls::GuiSelectableButton::IStyleController*		CreateItemExpandingDecorator()override;
				Color													GetTextColor()override;
			};
#pragma warning(pop)
		}
	}
}

#endif