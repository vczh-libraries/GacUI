#include "GuiWin7ButtonStyles.h"

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
Win7ButtonStyleBase
***********************************************************************/

			IMPLEMENT_TRANSFERRING_ANIMATION(Win7ButtonColors, Win7ButtonStyleBase)
			{
				colorCurrent=Win7ButtonColors::Blend(colorBegin, colorEnd, currentPosition, totalLength);
				style->elements.Apply(colorCurrent);
				style->AfterApplyColors(colorCurrent);
			}

			void Win7ButtonStyleBase::AfterApplyColors(const Win7ButtonColors& colors)
			{
			}

			Win7ButtonStyleBase::Win7ButtonStyleBase(bool verticalGradient, bool roundBorder, const Win7ButtonColors& initialColor, Alignment horizontal, Alignment vertical)
				:controlStyle(GuiButton::Normal)
				,isVisuallyEnabled(true)
				,isSelected(false)
				,transparentWhenInactive(false)
				,transparentWhenDisabled(false)
			{
				elements=Win7ButtonElements::Create(verticalGradient, roundBorder, horizontal, vertical);
				elements.Apply(initialColor);
				transferringAnimation=new TransferringAnimation(this, initialColor);
			}

			Win7ButtonStyleBase::~Win7ButtonStyleBase()
			{
				transferringAnimation->Disable();
			}

			compositions::GuiBoundsComposition* Win7ButtonStyleBase::GetBoundsComposition()
			{
				return elements.mainComposition;
			}

			compositions::GuiGraphicsComposition* Win7ButtonStyleBase::GetContainerComposition()
			{
				return elements.mainComposition;
			}

			void Win7ButtonStyleBase::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7ButtonStyleBase::SetText(const WString& value)
			{
				elements.textElement->SetText(value);
			}

			void Win7ButtonStyleBase::SetFont(const FontProperties& value)
			{
				Win7SetFont(elements.textElement, elements.textComposition, value);
			}

			void Win7ButtonStyleBase::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

			void Win7ButtonStyleBase::SetSelected(bool value)
			{
				if(isSelected!=value)
				{
					isSelected=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

			void Win7ButtonStyleBase::Transfer(GuiButton::ControlState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

			bool Win7ButtonStyleBase::GetTransparentWhenInactive()
			{
				return transparentWhenInactive;
			}

			void Win7ButtonStyleBase::SetTransparentWhenInactive(bool value)
			{
				transparentWhenInactive=value;
				TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
			}

			bool Win7ButtonStyleBase::GetTransparentWhenDisabled()
			{
				return transparentWhenDisabled;
			}

			void Win7ButtonStyleBase::SetTransparentWhenDisabled(bool value)
			{
				transparentWhenDisabled=value;
				TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
			}

			bool Win7ButtonStyleBase::GetAutoSizeForText()
			{
				return elements.textComposition->GetMinSizeLimitation()!=GuiGraphicsComposition::NoLimit;
			}

			void Win7ButtonStyleBase::SetAutoSizeForText(bool value)
			{
				if(value)
				{
					elements.textComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
				}
				else
				{
					elements.textComposition->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
				}
			}

/***********************************************************************
Win7ButtonStyle
***********************************************************************/

			void Win7ButtonStyle::TransferInternal(GuiButton::ControlState value, bool enabled, bool selected)
			{
				Win7ButtonColors targetColor;
				if(enabled)
				{
					switch(value)
					{
					case GuiButton::Normal:
						targetColor=Win7ButtonColors::ButtonNormal();
						if(transparentWhenInactive)
						{
							targetColor.SetAlphaWithoutText(0);
						}
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
					if(transparentWhenDisabled)
					{
						targetColor.SetAlphaWithoutText(0);
					}
				}
				transferringAnimation->Transfer(targetColor);
			}

			Win7ButtonStyle::Win7ButtonStyle(bool verticalGradient)
				:Win7ButtonStyleBase(verticalGradient, true, Win7ButtonColors::ButtonNormal(), Alignment::Center, Alignment::Center)
			{
			}

			Win7ButtonStyle::~Win7ButtonStyle()
			{
			}

/***********************************************************************
Win7CheckBoxStyle
***********************************************************************/

			IMPLEMENT_TRANSFERRING_ANIMATION(Win7ButtonColors, Win7CheckBoxStyle)
			{
				colorCurrent=Win7ButtonColors::Blend(colorBegin, colorEnd, currentPosition, totalLength);
				style->elements.Apply(colorCurrent);
			}

			void Win7CheckBoxStyle::TransferInternal(GuiButton::ControlState value, bool enabled, bool selected)
			{
				if(enabled)
				{
					switch(value)
					{
					case GuiButton::Normal:
						transferringAnimation->Transfer(Win7ButtonColors::CheckedNormal(selected));
						break;
					case GuiButton::Active:
						transferringAnimation->Transfer(Win7ButtonColors::CheckedActive(selected));
						break;
					case GuiButton::Pressed:
						transferringAnimation->Transfer(Win7ButtonColors::CheckedPressed(selected));
						break;
					}
				}
				else
				{
					transferringAnimation->Transfer(Win7ButtonColors::CheckedDisabled(selected));
				}
			}

			Win7CheckBoxStyle::Win7CheckBoxStyle(BulletStyle bulletStyle, bool backgroundVisible)
				:controlStyle(GuiButton::Normal)
				,isVisuallyEnabled(true)
				,isSelected(false)
			{
				Win7ButtonColors initialColor=Win7ButtonColors::CheckedNormal(isSelected);
				elements=Win7CheckedButtonElements::Create(bulletStyle==CheckBox?ElementShape::Rectangle:ElementShape::Ellipse, backgroundVisible);
				elements.Apply(initialColor);
				transferringAnimation=new TransferringAnimation(this, initialColor);
			}

			Win7CheckBoxStyle::~Win7CheckBoxStyle()
			{
				transferringAnimation->Disable();
			}

			compositions::GuiBoundsComposition* Win7CheckBoxStyle::GetBoundsComposition()
			{
				return elements.mainComposition;
			}

			compositions::GuiGraphicsComposition* Win7CheckBoxStyle::GetContainerComposition()
			{
				return elements.mainComposition;
			}

			void Win7CheckBoxStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7CheckBoxStyle::SetText(const WString& value)
			{
				elements.textElement->SetText(value);
			}

			void Win7CheckBoxStyle::SetFont(const FontProperties& value)
			{
				Win7SetFont(elements.textElement, elements.textComposition, value);
			}

			void Win7CheckBoxStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

			void Win7CheckBoxStyle::SetSelected(bool value)
			{
				if(isSelected!=value)
				{
					isSelected=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}

			void Win7CheckBoxStyle::Transfer(GuiButton::ControlState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected);
				}
			}
		}
	}
}