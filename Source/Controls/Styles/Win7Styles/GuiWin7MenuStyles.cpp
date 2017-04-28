#include "GuiWin7MenuStyles.h"

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
Win7MenuStyle
***********************************************************************/

			Win7MenuStyle::Win7MenuStyle()
			{
				{
					GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
					element->SetColor(Color(151, 151, 151));
					boundsComposition=new GuiBoundsComposition;
					boundsComposition->SetOwnedElement(element);
				}
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Color(245, 245, 245));
					GuiBoundsComposition* subBorder=new GuiBoundsComposition;
					subBorder->SetOwnedElement(element);
					subBorder->SetAlignmentToParent(Margin(1, 1, 1, 1));
					boundsComposition->AddChild(subBorder);
				}
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Win7GetSystemWindowColor());
					containerComposition=new GuiBoundsComposition;
					containerComposition->SetOwnedElement(element);
					containerComposition->SetAlignmentToParent(Margin(3, 3, 3, 3));
					containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					boundsComposition->AddChild(containerComposition);
				}
			}

			Win7MenuStyle::~Win7MenuStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7MenuStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7MenuStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win7MenuStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7MenuStyle::SetText(const WString& value)
			{
			}

			void Win7MenuStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7MenuStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win7MenuBarStyle
***********************************************************************/

			Win7MenuBarStyle::Win7MenuBarStyle()
			{
				GuiTableComposition* table=new GuiTableComposition;
				table->SetAlignmentToParent(Margin(0, 0, 0, 1));
				table->SetRowsAndColumns(2, 1);
				table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
				table->SetRowOption(1, GuiCellOption::PercentageOption(0.5));
				table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
				{
					GuiGradientBackgroundElement* gradient=GuiGradientBackgroundElement::Create();
					gradient->SetDirection(GuiGradientBackgroundElement::Vertical);
					gradient->SetColors(Color(252, 253, 254), Color(229, 234, 245));

					GuiCellComposition* cell=new GuiCellComposition;
					table->AddChild(cell);
					cell->SetSite(0, 0, 1, 1);
					cell->SetOwnedElement(gradient);
				}
				{
					GuiGradientBackgroundElement* gradient=GuiGradientBackgroundElement::Create();
					gradient->SetDirection(GuiGradientBackgroundElement::Vertical);
					gradient->SetColors(Color(212, 219, 237), Color(225, 230, 246));

					GuiCellComposition* cell=new GuiCellComposition;
					table->AddChild(cell);
					cell->SetSite(1, 0, 1, 1);
					cell->SetOwnedElement(gradient);
				}

				boundsComposition=new GuiBoundsComposition;
				{
					GuiSolidBackgroundElement* solid=GuiSolidBackgroundElement::Create();
					solid->SetColor(Color(182, 188, 204));
					boundsComposition->SetOwnedElement(solid);
					boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				}
				boundsComposition->AddChild(table);
			}

			Win7MenuBarStyle::~Win7MenuBarStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7MenuBarStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7MenuBarStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win7MenuBarStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7MenuBarStyle::SetText(const WString& value)
			{
			}

			void Win7MenuBarStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7MenuBarStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win7MenuBarButtonStyle
***********************************************************************/

			void Win7MenuBarButtonStyle::TransferInternal(ButtonState value, bool enabled, bool opening)
			{
				Win7ButtonColors targetColor;
				if(!enabled)
				{
					targetColor=Win7ButtonColors::MenuBarButtonDisabled();
					targetColor.SetAlphaWithoutText(0);
				}
				else if(opening)
				{
					targetColor=Win7ButtonColors::MenuBarButtonPressed();
				}
				else
				{
					switch(value)
					{
					case ButtonState::Normal:
						targetColor=Win7ButtonColors::MenuBarButtonNormal();
						targetColor.SetAlphaWithoutText(0);
						break;
					case ButtonState::Active:
						targetColor=Win7ButtonColors::MenuBarButtonActive();
						break;
					case ButtonState::Pressed:
						targetColor=Win7ButtonColors::MenuBarButtonPressed();
						break;
					}
				}
				elements.Apply(targetColor);
			}

			Win7MenuBarButtonStyle::Win7MenuBarButtonStyle()
				:controlStyle(ButtonState::Normal)
				,isVisuallyEnabled(true)
				,isOpening(false)
			{
				Win7ButtonColors initialColor=Win7ButtonColors::MenuBarButtonNormal();
				initialColor.SetAlphaWithoutText(0);

				elements=Win7ButtonElements::Create(true, true, Alignment::Center, Alignment::Center);
				elements.Apply(initialColor);
			}

			Win7MenuBarButtonStyle::~Win7MenuBarButtonStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7MenuBarButtonStyle::GetBoundsComposition()
			{
				return elements.mainComposition;
			}

			compositions::GuiGraphicsComposition* Win7MenuBarButtonStyle::GetContainerComposition()
			{
				return elements.mainComposition;
			}

			void Win7MenuBarButtonStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7MenuBarButtonStyle::SetText(const WString& value)
			{
				elements.textElement->SetText(value);
			}

			void Win7MenuBarButtonStyle::SetFont(const FontProperties& value)
			{
				Win7SetFont(elements.textElement, elements.textComposition, value);
				Margin margin=elements.textComposition->GetMargin();
				margin.left*=3;
				margin.right*=3;
				elements.textComposition->SetMargin(margin);
			}

			void Win7MenuBarButtonStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isOpening);
				}
			}

			void Win7MenuBarButtonStyle::SetSelected(bool value)
			{
			}

			controls::GuiMenu::IStyleController* Win7MenuBarButtonStyle::CreateSubMenuStyleController()
			{
				return new Win7MenuStyle;
			}

			void Win7MenuBarButtonStyle::SetSubMenuExisting(bool value)
			{
			}

			void Win7MenuBarButtonStyle::SetSubMenuOpening(bool value)
			{
				if(isOpening!=value)
				{
					isOpening=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isOpening);
				}
			}

			controls::GuiButton* Win7MenuBarButtonStyle::GetSubMenuHost()
			{
				return 0;
			}

			void Win7MenuBarButtonStyle::SetImage(Ptr<GuiImageData> value)
			{
			}

			void Win7MenuBarButtonStyle::SetShortcutText(const WString& value)
			{
			}

			void Win7MenuBarButtonStyle::Transfer(ButtonState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isOpening);
				}
			}

/***********************************************************************
Win7MenuItemButtonStyle
***********************************************************************/

			void Win7MenuItemButtonStyle::TransferInternal(ButtonState value, bool enabled, bool selected, bool opening)
			{
				Win7ButtonColors targetColor;
				bool active=false;
				if(enabled)
				{
					if(opening)
					{
						targetColor=Win7ButtonColors::MenuItemButtonNormalActive();
						active=true;
					}
					else
					{
						switch(value)
						{
						case ButtonState::Normal:
							targetColor=selected?Win7ButtonColors::MenuItemButtonSelected():Win7ButtonColors::MenuItemButtonNormal();
							break;
						case ButtonState::Active:
						case ButtonState::Pressed:
							targetColor=selected?Win7ButtonColors::MenuItemButtonSelectedActive():Win7ButtonColors::MenuItemButtonNormalActive();
							active=true;
							break;
						}
					}
				}
				else
				{
					switch(value)
					{
					case ButtonState::Normal:
						targetColor=Win7ButtonColors::MenuItemButtonDisabled();
						break;
					case ButtonState::Active:
					case ButtonState::Pressed:
						targetColor=Win7ButtonColors::MenuItemButtonDisabledActive();
						active=true;
						break;
					}
				}
				elements.Apply(targetColor);
				elements.SetActive(active || selected);
			}

			Win7MenuItemButtonStyle::Win7MenuItemButtonStyle()
				:controlStyle(ButtonState::Normal)
				,isVisuallyEnabled(true)
				,isSelected(false)
				,isOpening(false)
			{
				elements=Win7MenuItemButtonElements::Create();
				elements.Apply(Win7ButtonColors::MenuItemButtonNormal());
			}

			Win7MenuItemButtonStyle::~Win7MenuItemButtonStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7MenuItemButtonStyle::GetBoundsComposition()
			{
				return elements.mainComposition;
			}

			compositions::GuiGraphicsComposition* Win7MenuItemButtonStyle::GetContainerComposition()
			{
				return elements.mainComposition;
			}

			void Win7MenuItemButtonStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7MenuItemButtonStyle::SetText(const WString& value)
			{
				elements.textElement->SetText(value);
			}

			void Win7MenuItemButtonStyle::SetFont(const FontProperties& value)
			{
				Win7SetFont(elements.textElement, elements.textComposition, value);
				Win7SetFont(elements.shortcutElement, elements.shortcutComposition, value);
			}

			void Win7MenuItemButtonStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					elements.imageElement->SetEnabled(value);
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			void Win7MenuItemButtonStyle::SetSelected(bool value)
			{
				if(isSelected!=value)
				{
					isSelected=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			controls::GuiMenu::IStyleController* Win7MenuItemButtonStyle::CreateSubMenuStyleController()
			{
				return new Win7MenuStyle;
			}

			void Win7MenuItemButtonStyle::SetSubMenuExisting(bool value)
			{
				elements.SetSubMenuExisting(value);
			}

			void Win7MenuItemButtonStyle::SetSubMenuOpening(bool value)
			{
				if(isOpening!=value)
				{
					isOpening=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			controls::GuiButton* Win7MenuItemButtonStyle::GetSubMenuHost()
			{
				return 0;
			}

			void Win7MenuItemButtonStyle::SetImage(Ptr<GuiImageData> value)
			{
				if(value)
				{
					elements.imageElement->SetImage(value->GetImage(), value->GetFrameIndex());
				}
				else
				{
					elements.imageElement->SetImage(0, 0);
				}
			}

			void Win7MenuItemButtonStyle::SetShortcutText(const WString& value)
			{
				elements.shortcutElement->SetText(value);
			}

			void Win7MenuItemButtonStyle::Transfer(ButtonState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

/***********************************************************************
Win7MenuSplitterStyle
***********************************************************************/

			Win7MenuSplitterStyle::Win7MenuSplitterStyle()
			{
				Color dark=Win7GetSystemBorderSinkColor();
				Color bright=Win7GetSystemBorderRaiseColor();

				GuiTableComposition* table=new GuiTableComposition;
				table->SetAlignmentToParent(Margin(0, 0, 0, 0));
				table->SetRowsAndColumns(1, 3);
				table->SetPreferredMinSize(Size(0, 6));

				table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
				table->SetColumnOption(0, GuiCellOption::AbsoluteOption(26));
				table->SetColumnOption(1, GuiCellOption::AbsoluteOption(2));
				table->SetColumnOption(2, GuiCellOption::MinSizeOption());

				{
					GuiCellComposition* cell=new GuiCellComposition;
					table->AddChild(cell);
					cell->SetSite(0, 1, 1, 1);

					Gui3DSplitterElement* element=Gui3DSplitterElement::Create();
					element->SetDirection(Gui3DSplitterElement::Vertical);
					element->SetColors(dark, bright);
					cell->SetOwnedElement(element);
				}
				{
					GuiCellComposition* cell=new GuiCellComposition;
					table->AddChild(cell);
					cell->SetSite(0, 2, 1, 1);

					Gui3DSplitterElement* element=Gui3DSplitterElement::Create();
					element->SetDirection(Gui3DSplitterElement::Horizontal);
					element->SetColors(dark, bright);
					cell->SetOwnedElement(element);
				}
				boundsComposition=table;
			}

			Win7MenuSplitterStyle::~Win7MenuSplitterStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7MenuSplitterStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7MenuSplitterStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win7MenuSplitterStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7MenuSplitterStyle::SetText(const WString& value)
			{
			}

			void Win7MenuSplitterStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7MenuSplitterStyle::SetVisuallyEnabled(bool value)
			{
			}
		}
	}
}