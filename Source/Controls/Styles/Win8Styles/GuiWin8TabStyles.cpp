#include "GuiWin8TabStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win8
		{
			using namespace controls;

/***********************************************************************
Win8TabPageHeaderStyle
***********************************************************************/

			void Win8TabPageHeaderStyle::TransferInternal(ButtonState value, bool enabled, bool selected)
			{
				if(selected)
				{
					transferringAnimation->Transfer(Win8ButtonColors::TabPageHeaderSelected());
				}
				else
				{
					switch(value)
					{
					case ButtonState::Normal:
						transferringAnimation->Transfer(Win8ButtonColors::TabPageHeaderNormal());
						break;
					case ButtonState::Active:
					case ButtonState::Pressed:
						transferringAnimation->Transfer(Win8ButtonColors::TabPageHeaderActive());
						break;
					}
				}
			}

			Win8TabPageHeaderStyle::Win8TabPageHeaderStyle()
				:Win8ButtonStyleBase(Win8ButtonColors::TabPageHeaderNormal(), Alignment::Left, Alignment::Center)
			{
				transferringAnimation->SetEnableAnimation(false);
				{
					Margin margin=elements.backgroundComposition->GetAlignmentToParent();
					margin.bottom=0;
					elements.backgroundComposition->SetAlignmentToParent(margin);
				}
			}

			Win8TabPageHeaderStyle::~Win8TabPageHeaderStyle()
			{
			}

			void Win8TabPageHeaderStyle::SetFont(const FontProperties& value)
			{
				Win8ButtonStyleBase::SetFont(value);
				Margin margin=elements.textComposition->GetMargin();
				margin.left*=2;
				margin.right*=2;
				elements.textComposition->SetMargin(margin);
			}

/***********************************************************************
Win8TabStyle
***********************************************************************/

			controls::GuiSelectableButton::IStyleController* Win8TabStyle::CreateHeaderStyleController()
			{
				return new Win8TabPageHeaderStyle;
			}

			controls::GuiButton::IStyleController* Win8TabStyle::CreateMenuButtonStyleController()
			{
				return new Win8ButtonStyle;
			}

			controls::GuiToolstripMenu::IStyleController* Win8TabStyle::CreateMenuStyleController()
			{
				return new Win8MenuStyle;
			}

			controls::GuiToolstripButton::IStyleController* Win8TabStyle::CreateMenuItemStyleController()
			{
				return new Win8MenuItemButtonStyle;
			}

			Color Win8TabStyle::GetBorderColor()
			{
				return Win8ButtonColors::TabPageHeaderNormal().borderColor;
			}

			Color Win8TabStyle::GetBackgroundColor()
			{
				return Win8GetSystemTabContentColor();
			}

			Win8TabStyle::Win8TabStyle()
				:Win7TabStyle(false)
			{
				Initialize();
			}

			Win8TabStyle::~Win8TabStyle()
			{
			}
		}
	}
}