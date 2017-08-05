#include "GuiWin8ControlStyles.h"
#include "GuiWin8ListStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win8
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;
			using namespace controls;

/***********************************************************************
Win8EmptyStyle
***********************************************************************/

			Win8EmptyStyle::Win8EmptyStyle(Color color)
			{
				GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
				element->SetColor(color);
				
				boundsComposition=new GuiBoundsComposition;
				boundsComposition->SetOwnedElement(element);
			}

			Win8EmptyStyle::~Win8EmptyStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8EmptyStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8EmptyStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win8EmptyStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8EmptyStyle::SetText(const WString& value)
			{
			}

			void Win8EmptyStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8EmptyStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win8WindowStyle
***********************************************************************/

			Win8WindowStyle::Win8WindowStyle()
			{
				GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
				element->SetColor(Win8GetSystemWindowColor());
				
				boundsComposition=new GuiBoundsComposition;
				boundsComposition->SetOwnedElement(element);
			}

			Win8WindowStyle::~Win8WindowStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8WindowStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8WindowStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win8WindowStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8WindowStyle::SetText(const WString& value)
			{
			}

			void Win8WindowStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8WindowStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win8TooltipStyle
***********************************************************************/

			Win8TooltipStyle::Win8TooltipStyle()
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

			Win8TooltipStyle::~Win8TooltipStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8TooltipStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8TooltipStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win8TooltipStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8TooltipStyle::SetText(const WString& value)
			{
			}

			void Win8TooltipStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8TooltipStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win8LabelStyle
***********************************************************************/

			Win8LabelStyle::Win8LabelStyle(bool forShortcutKey)
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

			Win8LabelStyle::~Win8LabelStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8LabelStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8LabelStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win8LabelStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8LabelStyle::SetText(const WString& value)
			{
				textElement->SetText(value);
			}

			void Win8LabelStyle::SetFont(const FontProperties& value)
			{
				textElement->SetFont(value);
			}

			void Win8LabelStyle::SetVisuallyEnabled(bool value)
			{
			}

			Color Win8LabelStyle::GetDefaultTextColor()
			{
				return Win8GetSystemTextColor(true);
			}

			void Win8LabelStyle::SetTextColor(Color value)
			{
				textElement->SetColor(value);
			}

/***********************************************************************
Win8GroupBoxStyle
***********************************************************************/
			
			IMPLEMENT_TRANSFERRING_ANIMATION(Color, Win8GroupBoxStyle)
			{
				colorCurrent=BlendColor(colorBegin, colorEnd, currentPosition, totalLength);
				style->textElement->SetColor(colorCurrent);
			}

			void Win8GroupBoxStyle::SetMargins(vint fontSize)
			{
				fontSize+=4;
				vint half=fontSize/2;
				borderComposition->SetAlignmentToParent(Margin(0, half, 0, 0));
				containerComposition->SetAlignmentToParent(Margin(1, fontSize, 1, 1));
				textBackgroundComposition->SetAlignmentToParent(Margin(half, 2, -1, -1));
			}

			Win8GroupBoxStyle::Win8GroupBoxStyle()
			{
				boundsComposition=new GuiBoundsComposition;
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Win8GetSystemWindowColor());

					boundsComposition->SetOwnedElement(element);
					boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				}

				borderComposition=new GuiBoundsComposition;
				{
					GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
					element->SetColor(Color(221, 221, 221));

					borderComposition->SetOwnedElement(element);
					boundsComposition->AddChild(borderComposition);
				}

				textBackgroundComposition=new GuiBoundsComposition;
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Win8GetSystemWindowColor());

					textBackgroundComposition->SetOwnedElement(element);
					textBackgroundComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					boundsComposition->AddChild(textBackgroundComposition);
				}

				textComposition=new GuiBoundsComposition;
				{
					GuiSolidLabelElement* element=GuiSolidLabelElement::Create();
					element->SetColor(Win8GetSystemTextColor(true));
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
				transferringAnimation=new TransferringAnimation(this, Win8GetSystemTextColor(true));
			}

			Win8GroupBoxStyle::~Win8GroupBoxStyle()
			{
				transferringAnimation->Disable();
			}

			compositions::GuiBoundsComposition* Win8GroupBoxStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8GroupBoxStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win8GroupBoxStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8GroupBoxStyle::SetText(const WString& value)
			{
				textElement->SetText(value);
			}

			void Win8GroupBoxStyle::SetFont(const FontProperties& value)
			{
				textElement->SetFont(value);
				SetMargins(value.size);
			}

			void Win8GroupBoxStyle::SetVisuallyEnabled(bool value)
			{
				if(value)
				{
					transferringAnimation->Transfer(Win8GetSystemTextColor(true));
				}
				else
				{
					transferringAnimation->Transfer(Win8GetSystemTextColor(false));
				}
			}

/***********************************************************************
Win8DatePickerStyle
***********************************************************************/

			Win8DatePickerStyle::Win8DatePickerStyle()
			{
			}

			Win8DatePickerStyle::~Win8DatePickerStyle()
			{
			}

			void Win8DatePickerStyle::AssociateStyleController(controls::GuiControl::IStyleController* controller)
			{
			}

			void Win8DatePickerStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8DatePickerStyle::SetText(const WString& value)
			{
			}

			void Win8DatePickerStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8DatePickerStyle::SetVisuallyEnabled(bool value)
			{
			}

			controls::GuiSelectableButton::IStyleController* Win8DatePickerStyle::CreateDateButtonStyle()
			{
				return new Win8SelectableItemStyle;
			}

			controls::GuiTextList* Win8DatePickerStyle::CreateTextList()
			{
				return new GuiTextList(new Win8TextListProvider);
			}

			controls::GuiComboBoxListControl::IStyleController* Win8DatePickerStyle::CreateComboBoxStyle()
			{
				return new Win8DropDownComboBoxStyle;
			}

			Color Win8DatePickerStyle::GetBackgroundColor()
			{
				return Win8GetSystemWindowColor();
			}

			Color Win8DatePickerStyle::GetPrimaryTextColor()
			{
				return Win8GetSystemTextColor(true);
			}

			Color Win8DatePickerStyle::GetSecondaryTextColor()
			{
				return Win8GetSystemTextColor(false);
			}
		}
	}
}