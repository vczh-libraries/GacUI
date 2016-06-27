#include "GuiWin7ListStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win7
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;
			using namespace controls;

/***********************************************************************
Win7SelectableItemStyle
***********************************************************************/

			void Win7SelectableItemStyle::TransferInternal(GuiButton::ControlState value, bool enabled, bool selected)
			{
				if(!enabled)
				{
					transferringAnimation->Transfer(Win7ButtonColors::ItemDisabled());
				}
				else if(selected)
				{
					transferringAnimation->Transfer(Win7ButtonColors::ItemSelected());
				}
				else
				{
					switch(value)
					{
					case GuiButton::Normal:
						transferringAnimation->Transfer(Win7ButtonColors::ItemNormal());
						break;
					case GuiButton::Active:
						transferringAnimation->Transfer(Win7ButtonColors::ItemActive());
						break;
					case GuiButton::Pressed:
						transferringAnimation->Transfer(Win7ButtonColors::ItemSelected());
						break;
					}
				}
			}

			Win7SelectableItemStyle::Win7SelectableItemStyle()
				:Win7ButtonStyleBase(true, true, Win7ButtonColors::ItemNormal(), Alignment::Left, Alignment::Center)
			{
				transferringAnimation->SetEnableAnimation(false);
			}

			Win7SelectableItemStyle::~Win7SelectableItemStyle()
			{
			}

/***********************************************************************
Win7ListViewColumnDropDownStyle
***********************************************************************/

			void Win7ListViewColumnDropDownStyle::TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected)
			{
				if(!enabled) value=GuiButton::Normal;
				switch(value)
				{
				case GuiButton::Normal:
					{
						leftBorderComposition->SetVisible(true);
						borderComposition->SetVisible(false);
						gradientComposition->SetVisible(false);

						leftBorderElement->SetColors(Color(223, 234, 247), Color(252, 252, 252));
					}
					break;
				case GuiButton::Active:
				case GuiButton::Pressed:
					{
						leftBorderComposition->SetVisible(false);
						borderComposition->SetVisible(true);
						gradientComposition->SetVisible(true);

						borderElement->SetColor(Color(192, 203, 217));
						gradientElement->SetColors(Color(230, 241, 255), Color(224, 226, 253));
					}
					break;
				}
			}

			Win7ListViewColumnDropDownStyle::Win7ListViewColumnDropDownStyle()
				:controlStyle(GuiButton::Normal)
				,isVisuallyEnabled(true)
				,isSelected(false)
			{
				mainComposition=new GuiBoundsComposition;
				mainComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

				leftBorderElement=GuiGradientBackgroundElement::Create();
				leftBorderElement->SetDirection(GuiGradientBackgroundElement::Vertical);
				leftBorderComposition=new GuiBoundsComposition;
				leftBorderComposition->SetOwnedElement(leftBorderElement);
				leftBorderComposition->SetAlignmentToParent(Margin(0, 1, -1, 1));
				leftBorderComposition->SetPreferredMinSize(Size(1, 1));
				mainComposition->AddChild(leftBorderComposition);

				borderElement=GuiSolidBorderElement::Create();
				borderComposition=new GuiBoundsComposition;
				borderComposition->SetOwnedElement(borderElement);
				borderComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				mainComposition->AddChild(borderComposition);

				gradientElement=GuiGradientBackgroundElement::Create();
				gradientElement->SetDirection(GuiGradientBackgroundElement::Vertical);
				gradientComposition=new GuiBoundsComposition;
				gradientComposition->SetOwnedElement(gradientElement);
				gradientComposition->SetAlignmentToParent(Margin(2, 2, 2, 2));
				gradientComposition->SetPreferredMinSize(Size(0, 4));
				mainComposition->AddChild(gradientComposition);

				arrowElement=common_styles::CommonFragmentBuilder::BuildDownArrow();
				arrowElement->SetBackgroundColor(Color(76, 96, 122));
				arrowElement->SetBorderColor(Color(76, 96, 122));
				arrowComposition=new GuiBoundsComposition;
				arrowComposition->SetOwnedElement(arrowElement);
				arrowComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				arrowComposition->SetAlignmentToParent(Margin(3, 3, 3, 3));
				mainComposition->AddChild(arrowComposition);

				TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
			}

			Win7ListViewColumnDropDownStyle::~Win7ListViewColumnDropDownStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7ListViewColumnDropDownStyle::GetBoundsComposition()
			{
				return mainComposition;
			}

			compositions::GuiGraphicsComposition* Win7ListViewColumnDropDownStyle::GetContainerComposition()
			{
				return mainComposition;
			}

			void Win7ListViewColumnDropDownStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7ListViewColumnDropDownStyle::SetText(const WString& value)
			{
			}

			void Win7ListViewColumnDropDownStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7ListViewColumnDropDownStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

			void Win7ListViewColumnDropDownStyle::SetSelected(bool value)
			{
				if(isSelected!=value)
				{
					isSelected=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

			void Win7ListViewColumnDropDownStyle::Transfer(GuiButton::ControlState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

/***********************************************************************
Win7ListViewColumnHeaderStyle
***********************************************************************/

			void Win7ListViewColumnHeaderStyle::TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool subMenuExisting, bool subMenuOpening)
			{
				if(!enabled) value=GuiButton::Normal;
				switch(value)
				{
				case GuiButton::Normal:
					{
						rightBorderComposition->SetVisible(true);
						borderComposition->SetVisible(false);
						gradientComposition->SetVisible(false);

						backgroundElement->SetColor(Color(252, 252, 252));
						rightBorderElement->SetColors(Color(223, 234, 247), Color(252, 252, 252));

						dropdownButton->SetVisible(subMenuOpening);
					}
					break;
				case GuiButton::Active:
					{
						rightBorderComposition->SetVisible(false);
						borderComposition->SetVisible(true);
						gradientComposition->SetVisible(true);
						gradientComposition->SetAlignmentToParent(Margin(2, 2, 2, 2));

						backgroundElement->SetColor(Color(252, 252, 252));
						borderElement->SetColor(Color(223, 233, 246));
						gradientElement->SetColors(Color(243, 248, 253), Color(239, 243, 249));

						dropdownButton->SetVisible(isSubMenuExisting);
					}
					break;
				case GuiButton::Pressed:
					{
						rightBorderComposition->SetVisible(false);
						borderComposition->SetVisible(true);
						gradientComposition->SetVisible(true);
						gradientComposition->SetAlignmentToParent(Margin(1, 0, 1, -1));

						backgroundElement->SetColor(Color(246, 247, 248));
						borderElement->SetColor(Color(192, 203, 217));
						gradientElement->SetColors(Color(193, 204, 218), Color(252, 252, 252));

						dropdownButton->SetVisible(isSubMenuExisting);
					}
					break;
				}
			}

			Win7ListViewColumnHeaderStyle::Win7ListViewColumnHeaderStyle()
				:controlStyle(GuiButton::Normal)
				,isVisuallyEnabled(true)
				,isSubMenuExisting(false)
				,isSubMenuOpening(false)
			{
				backgroundElement=GuiSolidBackgroundElement::Create();
				mainComposition=new GuiBoundsComposition;
				mainComposition->SetOwnedElement(backgroundElement);
				mainComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

				rightBorderElement=GuiGradientBackgroundElement::Create();
				rightBorderElement->SetDirection(GuiGradientBackgroundElement::Vertical);
				rightBorderComposition=new GuiBoundsComposition;
				rightBorderComposition->SetOwnedElement(rightBorderElement);
				rightBorderComposition->SetAlignmentToParent(Margin(-1, 0, 0, 0));
				rightBorderComposition->SetPreferredMinSize(Size(1, 1));
				mainComposition->AddChild(rightBorderComposition);

				borderElement=GuiSolidBorderElement::Create();
				borderComposition=new GuiBoundsComposition;
				borderComposition->SetOwnedElement(borderElement);
				borderComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				mainComposition->AddChild(borderComposition);

				gradientElement=GuiGradientBackgroundElement::Create();
				gradientElement->SetDirection(GuiGradientBackgroundElement::Vertical);
				gradientComposition=new GuiBoundsComposition;
				gradientComposition->SetOwnedElement(gradientElement);
				gradientComposition->SetAlignmentToParent(Margin(2, 2, 2, 2));
				gradientComposition->SetPreferredMinSize(Size(0, 4));
				mainComposition->AddChild(gradientComposition);

				textElement=GuiSolidLabelElement::Create();
				textElement->SetColor(Color(76, 96, 122));
				textComposition=new GuiBoundsComposition;
				textComposition->SetOwnedElement(textElement);
				textComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				textComposition->SetAlignmentToParent(Margin(15, 7, 18, 5));
				mainComposition->AddChild(textComposition);

				arrowElement=GuiPolygonElement::Create();
				arrowElement->SetBackgroundColor(Color(76, 96, 122));
				arrowElement->SetBorderColor(Color(76, 96, 122));
				arrowComposition=new GuiBoundsComposition;
				arrowComposition->SetOwnedElement(arrowElement);
				arrowComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
				arrowComposition->SetAlignmentToParent(Margin(0, 2, 0, -1));
				mainComposition->AddChild(arrowComposition);

				dropdownButton=new GuiButton(new Win7ListViewColumnDropDownStyle);
				dropdownButton->GetBoundsComposition()->SetAlignmentToParent(Margin(-1, 0, 0, 0));
				dropdownButton->SetVisible(false);
				mainComposition->AddChild(dropdownButton->GetBoundsComposition());

				TransferInternal(controlStyle, isVisuallyEnabled, isSubMenuExisting, isSubMenuOpening);
			}

			Win7ListViewColumnHeaderStyle::~Win7ListViewColumnHeaderStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7ListViewColumnHeaderStyle::GetBoundsComposition()
			{
				return mainComposition;
			}

			compositions::GuiGraphicsComposition* Win7ListViewColumnHeaderStyle::GetContainerComposition()
			{
				return mainComposition;
			}

			void Win7ListViewColumnHeaderStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7ListViewColumnHeaderStyle::SetText(const WString& value)
			{
				textElement->SetText(value);
			}

			void Win7ListViewColumnHeaderStyle::SetFont(const FontProperties& value)
			{
				textElement->SetFont(value);
			}

			void Win7ListViewColumnHeaderStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSubMenuExisting, isSubMenuOpening);
				}
			}

			void Win7ListViewColumnHeaderStyle::SetSelected(bool value)
			{
			}

			void Win7ListViewColumnHeaderStyle::Transfer(GuiButton::ControlState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSubMenuExisting, isSubMenuOpening);
				}
			}

			controls::GuiMenu::IStyleController* Win7ListViewColumnHeaderStyle::CreateSubMenuStyleController()
			{
				return new Win7MenuStyle;
			}

			void Win7ListViewColumnHeaderStyle::SetSubMenuExisting(bool value)
			{
				if(isSubMenuExisting!=value)
				{
					isSubMenuExisting=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSubMenuExisting, isSubMenuOpening);
				}
			}

			void Win7ListViewColumnHeaderStyle::SetSubMenuOpening(bool value)
			{
				if(isSubMenuOpening!=value)
				{
					isSubMenuOpening=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSubMenuExisting, isSubMenuOpening);
				}
			}

			controls::GuiButton* Win7ListViewColumnHeaderStyle::GetSubMenuHost()
			{
				return dropdownButton;
			}

			void Win7ListViewColumnHeaderStyle::SetImage(Ptr<GuiImageData> value)
			{
			}

			void Win7ListViewColumnHeaderStyle::SetShortcutText(const WString& value)
			{
			}

			void Win7ListViewColumnHeaderStyle::SetColumnSortingState(controls::GuiListViewColumnHeader::ColumnSortingState value)
			{
				Margin margin=arrowComposition->GetAlignmentToParent();
				switch(value)
				{
				case controls::GuiListViewColumnHeader::NotSorted:
					arrowElement->SetPoints(0, 0);
					break;
				case controls::GuiListViewColumnHeader::Ascending:
					common_styles::CommonFragmentBuilder::FillUpArrow(arrowElement);
					break;
				case controls::GuiListViewColumnHeader::Descending:
					common_styles::CommonFragmentBuilder::FillDownArrow(arrowElement);
					break;
				}
			}

/***********************************************************************
Win7TreeViewExpandingButtonStyle
***********************************************************************/

			void Win7TreeViewExpandingButtonStyle::TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected)
			{
				bool expanding=false;
				bool activated=false;
				if(isVisuallyEnabled)
				{
					expanding=selected;
					activated=value!=GuiButton::Normal;
				}

				Point points[3];
				if(expanding)
				{
					points[0]=Point(5, 2);
					points[1]=Point(5, 7);
					points[2]=Point(0, 7);
				}
				else
				{
					points[0]=Point(0, 1);
					points[1]=Point(4, 5);
					points[2]=Point(0, 9);
				}
				polygonElement->SetPoints(points, sizeof(points)/sizeof(*points));

				if(activated)
				{
					polygonElement->SetBorderColor(Color(28, 196, 247));
					polygonElement->SetBackgroundColor(Color(130, 223, 251));
				}
				else if(expanding)
				{
					polygonElement->SetBorderColor(Color(38, 38, 38));
					polygonElement->SetBackgroundColor(Color(89, 89, 89));
				}
				else
				{
					polygonElement->SetBorderColor(Color(166, 166, 166));
					polygonElement->SetBackgroundColor(Color(255, 255, 255));
				}
			}

			Win7TreeViewExpandingButtonStyle::Win7TreeViewExpandingButtonStyle()
				:controlStyle(GuiButton::Normal)
				,isVisuallyEnabled(true)
				,isSelected(false)
			{
				polygonElement=GuiPolygonElement::Create();
				polygonElement->SetSize(Size(6, 11));

				mainComposition=new GuiBoundsComposition;
				mainComposition->SetOwnedElement(polygonElement);
				mainComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);

				TransferInternal(controlStyle, true, isSelected);
			}

			Win7TreeViewExpandingButtonStyle::~Win7TreeViewExpandingButtonStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7TreeViewExpandingButtonStyle::GetBoundsComposition()
			{
				return mainComposition;
			}

			compositions::GuiGraphicsComposition* Win7TreeViewExpandingButtonStyle::GetContainerComposition()
			{
				return mainComposition;
			}

			void Win7TreeViewExpandingButtonStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7TreeViewExpandingButtonStyle::SetText(const WString& value)
			{
			}

			void Win7TreeViewExpandingButtonStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7TreeViewExpandingButtonStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

			void Win7TreeViewExpandingButtonStyle::SetSelected(bool value)
			{
				if(isSelected!=value)
				{
					isSelected=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

			void Win7TreeViewExpandingButtonStyle::Transfer(GuiButton::ControlState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

/***********************************************************************
Win7DropDownComboBoxStyle
***********************************************************************/

			void Win7DropDownComboBoxStyle::TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected)
			{
				Win7ButtonColors targetColor;
				if(enabled)
				{
					if(selected) value=GuiButton::Pressed;
					switch(value)
					{
					case GuiButton::Normal:
						targetColor=Win7ButtonColors::ButtonNormal();
						break;
					case GuiButton::Active:
						targetColor=Win7ButtonColors::ButtonActive();
						break;
					case GuiButton::Pressed:
						targetColor=Win7ButtonColors::ButtonPressed();
						break;
					}
				}
				else
				{
					targetColor=Win7ButtonColors::ButtonDisabled();
				}
				transferringAnimation->Transfer(targetColor);
			}

			void Win7DropDownComboBoxStyle::AfterApplyColors(const Win7ButtonColors& colors)
			{
				Win7ButtonStyle::AfterApplyColors(colors);
				dropDownElement->SetBorderColor(colors.textColor);
				dropDownElement->SetBackgroundColor(colors.textColor);
			}

			Win7DropDownComboBoxStyle::Win7DropDownComboBoxStyle()
				:Win7ButtonStyle(true)
				, commandExecutor(0)
				, textVisible(true)
			{
				table=new GuiTableComposition;
				table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				table->SetAlignmentToParent(Margin(0, 0, 0, 0));
				table->SetRowsAndColumns(3, 2);
				table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
				table->SetRowOption(1, GuiCellOption::MinSizeOption());
				table->SetRowOption(2, GuiCellOption::PercentageOption(1.0));
				table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
				table->SetColumnOption(1, GuiCellOption::MinSizeOption());
				elements.textComposition->AddChild(table);
				elements.textComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

				textComposition=new GuiCellComposition;
				table->AddChild(textComposition);
				textComposition->SetSite(1, 0, 1, 1);

				Ptr<IGuiGraphicsElement> element=elements.textComposition->GetOwnedElement();
				elements.textComposition->SetOwnedElement(0);
				textComposition->SetOwnedElement(element);
				elements.textElement->SetEllipse(true);
				elements.textElement->SetAlignments(Alignment::Left, Alignment::Center);

				dropDownElement=common_styles::CommonFragmentBuilder::BuildDownArrow();

				dropDownComposition=new GuiCellComposition;
				table->AddChild(dropDownComposition);
				dropDownComposition->SetSite(1, 1, 1, 1);
				dropDownComposition->SetOwnedElement(dropDownElement);
				dropDownComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
				dropDownComposition->SetMargin(Margin(3, 0, 3, 0));
			}

			Win7DropDownComboBoxStyle::~Win7DropDownComboBoxStyle()
			{
			}

			compositions::GuiGraphicsComposition* Win7DropDownComboBoxStyle::GetContainerComposition()
			{
				return textComposition;
			}

			GuiMenu::IStyleController* Win7DropDownComboBoxStyle::CreateSubMenuStyleController()
			{
				return new Win7MenuStyle;
			}

			void Win7DropDownComboBoxStyle::SetSubMenuExisting(bool value)
			{
			}

			void Win7DropDownComboBoxStyle::SetSubMenuOpening(bool value)
			{
				SetSelected(value);
			}

			GuiButton* Win7DropDownComboBoxStyle::GetSubMenuHost()
			{
				return 0;
			}

			void Win7DropDownComboBoxStyle::SetImage(Ptr<GuiImageData> value)
			{
			}

			void Win7DropDownComboBoxStyle::SetShortcutText(const WString& value)
			{
			}

			void Win7DropDownComboBoxStyle::SetCommandExecutor(controls::GuiComboBoxBase::ICommandExecutor* value)
			{
				commandExecutor=value;
			}

			void Win7DropDownComboBoxStyle::OnItemSelected()
			{
			}

			void Win7DropDownComboBoxStyle::SetText(const WString& value)
			{
				text = value;
				if (textVisible)
				{
					Win7ButtonStyle::SetText(text);
				}
			}

			void Win7DropDownComboBoxStyle::SetTextVisible(bool value)
			{
				if (textVisible != value)
				{
					if ((textVisible = value))
					{
						Win7ButtonStyle::SetText(text);
					}
					else
					{
						Win7ButtonStyle::SetText(L"");
					}
				}
			}

/***********************************************************************
Win7TextListProvider
***********************************************************************/
			
			Win7TextListProvider::Win7TextListProvider()
			{
			}

			Win7TextListProvider::~Win7TextListProvider()
			{
			}

			controls::GuiSelectableButton::IStyleController* Win7TextListProvider::CreateBackgroundStyleController()
			{
				return new Win7SelectableItemStyle;
			}

			controls::GuiSelectableButton::IStyleController* Win7TextListProvider::CreateBulletStyleController()
			{
				return 0;
			}

			Color Win7TextListProvider::GetTextColor()
			{
				return Win7GetSystemTextColor(true);
			}

/***********************************************************************
Win7CheckTextListProvider
***********************************************************************/

			Win7CheckTextListProvider::Win7CheckTextListProvider()
			{
			}

			Win7CheckTextListProvider::~Win7CheckTextListProvider()
			{
			}

			controls::GuiSelectableButton::IStyleController* Win7CheckTextListProvider::CreateBulletStyleController()
			{
				return new Win7CheckBoxStyle(Win7CheckBoxStyle::CheckBox, false);
			}

/***********************************************************************
Win7RadioTextListProvider
***********************************************************************/

			Win7RadioTextListProvider::Win7RadioTextListProvider()
			{
			}

			Win7RadioTextListProvider::~Win7RadioTextListProvider()
			{
			}

			controls::GuiSelectableButton::IStyleController* Win7RadioTextListProvider::CreateBulletStyleController()
			{
				return new Win7CheckBoxStyle(Win7CheckBoxStyle::RadioButton, false);
			}

/***********************************************************************
Win7ListViewProvider
***********************************************************************/

			Win7ListViewProvider::Win7ListViewProvider()
			{
			}

			Win7ListViewProvider::~Win7ListViewProvider()
			{
			}

			controls::GuiSelectableButton::IStyleController* Win7ListViewProvider::CreateItemBackground()
			{
				return new Win7SelectableItemStyle;
			}

			controls::GuiListViewColumnHeader::IStyleController* Win7ListViewProvider::CreateColumnStyle()
			{
				return new Win7ListViewColumnHeaderStyle;
			}

			Color Win7ListViewProvider::GetPrimaryTextColor()
			{
				return Win7GetSystemTextColor(true);
			}

			Color Win7ListViewProvider::GetSecondaryTextColor()
			{
				return Win7GetSystemTextColor(false);
			}

			Color Win7ListViewProvider::GetItemSeparatorColor()
			{
				return Color(220, 220, 220);
			}

/***********************************************************************
Win7TreeViewProvider
***********************************************************************/

			Win7TreeViewProvider::Win7TreeViewProvider()
			{
			}

			Win7TreeViewProvider::~Win7TreeViewProvider()
			{
			}

			controls::GuiSelectableButton::IStyleController* Win7TreeViewProvider::CreateItemBackground()
			{
				return new Win7SelectableItemStyle;
			}

			controls::GuiSelectableButton::IStyleController* Win7TreeViewProvider::CreateItemExpandingDecorator()
			{
				return new Win7TreeViewExpandingButtonStyle;
			}

			Color Win7TreeViewProvider::GetTextColor()
			{
				return Win7GetSystemTextColor(true);
			}
		}
	}
}