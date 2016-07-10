#include "GuiWin7ControlStyles.h"
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
Win7EmptyStyle
***********************************************************************/

			Win7EmptyStyle::Win7EmptyStyle(Color color)
			{
				GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
				element->SetColor(color);
				
				boundsComposition=new GuiBoundsComposition;
				boundsComposition->SetOwnedElement(element);
			}

			Win7EmptyStyle::~Win7EmptyStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7EmptyStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7EmptyStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win7EmptyStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7EmptyStyle::SetText(const WString& value)
			{
			}

			void Win7EmptyStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7EmptyStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win7WindowStyle
***********************************************************************/

			Win7WindowStyle::Win7WindowStyle()
			{
				GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
				element->SetColor(Win7GetSystemWindowColor());
				
				boundsComposition=new GuiBoundsComposition;
				boundsComposition->SetOwnedElement(element);
			}

			Win7WindowStyle::~Win7WindowStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7WindowStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7WindowStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win7WindowStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7WindowStyle::SetText(const WString& value)
			{
			}

			void Win7WindowStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7WindowStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win7TooltipStyle
***********************************************************************/

			Win7TooltipStyle::Win7TooltipStyle()
			{
				boundsComposition=new GuiBoundsComposition;
				boundsComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				{
					GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
					element->SetColor(Color(100, 100, 100));
					boundsComposition->SetOwnedElement(element);
				}

				containerComposition=new GuiBoundsComposition;
				containerComposition->SetAlignmentToParent(Margin(1, 1, 1, 1));
				containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				boundsComposition->AddChild(containerComposition);
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Color(255, 255, 225));
					containerComposition->SetOwnedElement(element);
				}
			}

			Win7TooltipStyle::~Win7TooltipStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7TooltipStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7TooltipStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win7TooltipStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7TooltipStyle::SetText(const WString& value)
			{
			}

			void Win7TooltipStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7TooltipStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win7LabelStyle
***********************************************************************/

			Win7LabelStyle::Win7LabelStyle(bool forShortcutKey)
			{
				textElement=GuiSolidLabelElement::Create();
				textElement->SetColor(GetDefaultTextColor());
				
				boundsComposition=new GuiBoundsComposition;
				boundsComposition->SetMinSizeLimitation(GuiBoundsComposition::LimitToElementAndChildren);
				if (forShortcutKey)
				{
					{
						GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
						element->SetColor(Color(100, 100, 100));
						boundsComposition->SetOwnedElement(element);
					}

					auto containerComposition=new GuiBoundsComposition;
					containerComposition->SetAlignmentToParent(Margin(1, 1, 1, 1));
					containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					boundsComposition->AddChild(containerComposition);
					{
						GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
						element->SetColor(Color(255, 255, 225));
						containerComposition->SetOwnedElement(element);
					}

					auto labelComposition = new GuiBoundsComposition;
					labelComposition->SetAlignmentToParent(Margin(2, 2, 2, 2));
					labelComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
					labelComposition->SetOwnedElement(textElement);
					containerComposition->AddChild(labelComposition);
				}
				else
				{
					boundsComposition->SetOwnedElement(textElement);
				}
			}

			Win7LabelStyle::~Win7LabelStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7LabelStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7LabelStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win7LabelStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7LabelStyle::SetText(const WString& value)
			{
				textElement->SetText(value);
			}

			void Win7LabelStyle::SetFont(const FontProperties& value)
			{
				textElement->SetFont(value);
			}

			void Win7LabelStyle::SetVisuallyEnabled(bool value)
			{
			}

			Color Win7LabelStyle::GetDefaultTextColor()
			{
				return Win7GetSystemTextColor(true);
			}

			void Win7LabelStyle::SetTextColor(Color value)
			{
				textElement->SetColor(value);
			}

/***********************************************************************
Win7GroupBoxStyle
***********************************************************************/
			
			IMPLEMENT_TRANSFERRING_ANIMATION(Color, Win7GroupBoxStyle)
			{
				colorCurrent=BlendColor(colorBegin, colorEnd, currentPosition, totalLength);
				style->textElement->SetColor(colorCurrent);
			}

			void Win7GroupBoxStyle::SetMargins(vint fontSize)
			{
				fontSize+=4;
				vint half=fontSize/2;
				sinkBorderComposition->SetAlignmentToParent(Margin(0, half, 1, 1));
				raisedBorderComposition->SetAlignmentToParent(Margin(1, half+1, 0, 0));
				containerComposition->SetAlignmentToParent(Margin(2, fontSize, 2, 2));
				textBackgroundComposition->SetAlignmentToParent(Margin(half, 2, -1, -1));
			}

			Win7GroupBoxStyle::Win7GroupBoxStyle()
			{
				boundsComposition=new GuiBoundsComposition;
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Win7GetSystemWindowColor());

					boundsComposition->SetOwnedElement(element);
					boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				}

				sinkBorderComposition=new GuiBoundsComposition;
				{
					GuiRoundBorderElement* element=GuiRoundBorderElement::Create();
					element->SetRadius(3);
					element->SetColor(Color(213, 223, 229));

					sinkBorderComposition->SetOwnedElement(element);
					boundsComposition->AddChild(sinkBorderComposition);
				}

				raisedBorderComposition=new GuiBoundsComposition;
				{
					GuiRoundBorderElement* element=GuiRoundBorderElement::Create();
					element->SetRadius(3);
					element->SetColor(Color(255, 255, 255));

					raisedBorderComposition->SetOwnedElement(element);
					boundsComposition->AddChild(raisedBorderComposition);
				}

				textBackgroundComposition=new GuiBoundsComposition;
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Win7GetSystemWindowColor());

					textBackgroundComposition->SetOwnedElement(element);
					textBackgroundComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					boundsComposition->AddChild(textBackgroundComposition);
				}

				textComposition=new GuiBoundsComposition;
				{
					GuiSolidLabelElement* element=GuiSolidLabelElement::Create();
					element->SetColor(Win7GetSystemTextColor(true));
					textElement=element;

					textComposition->SetOwnedElement(element);
					textComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
					textComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
					textBackgroundComposition->AddChild(textComposition);
				}

				containerComposition=new GuiBoundsComposition;
				{
					boundsComposition->AddChild(containerComposition);
				}

				SetMargins(0);
				transferringAnimation=new TransferringAnimation(this, Win7GetSystemTextColor(true));
			}

			Win7GroupBoxStyle::~Win7GroupBoxStyle()
			{
				transferringAnimation->Disable();
			}

			compositions::GuiBoundsComposition* Win7GroupBoxStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7GroupBoxStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win7GroupBoxStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7GroupBoxStyle::SetText(const WString& value)
			{
				textElement->SetText(value);
			}

			void Win7GroupBoxStyle::SetFont(const FontProperties& value)
			{
				textElement->SetFont(value);
				SetMargins(value.size);
			}

			void Win7GroupBoxStyle::SetVisuallyEnabled(bool value)
			{
				if(value)
				{
					transferringAnimation->Transfer(Win7GetSystemTextColor(true));
				}
				else
				{
					transferringAnimation->Transfer(Win7GetSystemTextColor(false));
				}
			}

/***********************************************************************
Win7DatePickerStyle
***********************************************************************/

			Win7DatePickerStyle::Win7DatePickerStyle()
			{
			}

			Win7DatePickerStyle::~Win7DatePickerStyle()
			{
			}

			void Win7DatePickerStyle::AssociateStyleController(controls::GuiControl::IStyleController* controller)
			{
			}

			void Win7DatePickerStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7DatePickerStyle::SetText(const WString& value)
			{
			}

			void Win7DatePickerStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7DatePickerStyle::SetVisuallyEnabled(bool value)
			{
			}

			controls::GuiSelectableButton::IStyleController* Win7DatePickerStyle::CreateDateButtonStyle()
			{
				return new Win7SelectableItemStyle;
			}

			controls::GuiTextList* Win7DatePickerStyle::CreateTextList()
			{
				return new GuiTextList(new Win7TextListProvider, new Win7TextListItemProvider);
			}

			controls::GuiComboBoxListControl::IStyleController* Win7DatePickerStyle::CreateComboBoxStyle()
			{
				return new Win7DropDownComboBoxStyle;
			}

			Color Win7DatePickerStyle::GetBackgroundColor()
			{
				return Win7GetSystemWindowColor();
			}

			Color Win7DatePickerStyle::GetPrimaryTextColor()
			{
				return Win7GetSystemTextColor(true);
			}

			Color Win7DatePickerStyle::GetSecondaryTextColor()
			{
				return Win7GetSystemTextColor(false);
			}
		}
	}
}