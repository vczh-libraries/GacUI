#include "GuiWin8MenuStyles.h"

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
Win8MenuStyle
***********************************************************************/

			Win8MenuStyle::Win8MenuStyle()
			{
				{
					GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
					element->SetColor(Win8GetMenuBorderColor());
					boundsComposition=new GuiBoundsComposition;
					boundsComposition->SetOwnedElement(element);
				}
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Win8GetSystemWindowColor());
					GuiBoundsComposition* subBorder=new GuiBoundsComposition;
					subBorder->SetOwnedElement(element);
					subBorder->SetAlignmentToParent(Margin(1, 1, 1, 1));
					boundsComposition->AddChild(subBorder);
				}
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Win8GetSystemWindowColor());
					containerComposition=new GuiBoundsComposition;
					containerComposition->SetOwnedElement(element);
					containerComposition->SetAlignmentToParent(Margin(3, 3, 3, 3));
					containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					boundsComposition->AddChild(containerComposition);
				}
			}

			Win8MenuStyle::~Win8MenuStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8MenuStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8MenuStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win8MenuStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8MenuStyle::SetText(const WString& value)
			{
			}

			void Win8MenuStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8MenuStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win8MenuBarStyle
***********************************************************************/

			Win8MenuBarStyle::Win8MenuBarStyle()
			{
				boundsComposition=new GuiBoundsComposition;
				{
					GuiSolidBackgroundElement* solid=GuiSolidBackgroundElement::Create();
					solid->SetColor(Color(245, 246, 247));
					boundsComposition->SetOwnedElement(solid);
					boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				}
				{
					Gui3DSplitterElement* element=Gui3DSplitterElement::Create();
					element->SetColors(Color(232, 233, 234), Color(240, 240, 240));
					element->SetDirection(Gui3DSplitterElement::Horizontal);

					GuiBoundsComposition* composition=new GuiBoundsComposition;
					composition->SetOwnedElement(element);
					composition->SetPreferredMinSize(Size(0, 2));
					composition->SetAlignmentToParent(Margin(0, -1, 0, 0));
					boundsComposition->AddChild(composition);
				}
				containerComposition=new GuiBoundsComposition;
				{
					containerComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
					containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					boundsComposition->AddChild(containerComposition);
				}
			}

			Win8MenuBarStyle::~Win8MenuBarStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8MenuBarStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8MenuBarStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win8MenuBarStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8MenuBarStyle::SetText(const WString& value)
			{
			}

			void Win8MenuBarStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8MenuBarStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win8MenuBarButtonStyle
***********************************************************************/

			void Win8MenuBarButtonStyle::TransferInternal(GuiButton::ControlState value, bool enabled, bool opening)
			{
				Win8ButtonColors targetColor;
				if(!enabled)
				{
					targetColor=Win8ButtonColors::MenuBarButtonDisabled();
					targetColor.SetAlphaWithoutText(0);
				}
				else if(opening)
				{
					targetColor=Win8ButtonColors::MenuBarButtonPressed();
				}
				else
				{
					switch(value)
					{
					case GuiButton::Normal:
						targetColor=Win8ButtonColors::MenuBarButtonNormal();
						targetColor.SetAlphaWithoutText(0);
						break;
					case GuiButton::Active:
						targetColor=Win8ButtonColors::MenuBarButtonActive();
						break;
					case GuiButton::Pressed:
						targetColor=Win8ButtonColors::MenuBarButtonPressed();
						break;
					}
				}
				elements.Apply(targetColor);
			}

			Win8MenuBarButtonStyle::Win8MenuBarButtonStyle()
				:controlStyle(GuiButton::Normal)
				,isVisuallyEnabled(true)
				,isOpening(false)
			{
				Win8ButtonColors initialColor=Win8ButtonColors::MenuBarButtonNormal();
				initialColor.SetAlphaWithoutText(0);

				elements=Win8ButtonElements::Create(Alignment::Center, Alignment::Center);
				elements.Apply(initialColor);
			}

			Win8MenuBarButtonStyle::~Win8MenuBarButtonStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8MenuBarButtonStyle::GetBoundsComposition()
			{
				return elements.mainComposition;
			}

			compositions::GuiGraphicsComposition* Win8MenuBarButtonStyle::GetContainerComposition()
			{
				return elements.mainComposition;
			}

			void Win8MenuBarButtonStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8MenuBarButtonStyle::SetText(const WString& value)
			{
				elements.textElement->SetText(value);
			}

			void Win8MenuBarButtonStyle::SetFont(const FontProperties& value)
			{
				Win8SetFont(elements.textElement, elements.textComposition, value);
				Margin margin=elements.textComposition->GetMargin();
				margin.left*=3;
				margin.right*=3;
				elements.textComposition->SetMargin(margin);
			}

			void Win8MenuBarButtonStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isOpening);
				}
			}

			void Win8MenuBarButtonStyle::SetSelected(bool value)
			{
			}

			controls::GuiMenu::IStyleController* Win8MenuBarButtonStyle::CreateSubMenuStyleController()
			{
				return new Win8MenuStyle;
			}

			void Win8MenuBarButtonStyle::SetSubMenuExisting(bool value)
			{
			}

			void Win8MenuBarButtonStyle::SetSubMenuOpening(bool value)
			{
				if(isOpening!=value)
				{
					isOpening=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isOpening);
				}
			}

			controls::GuiButton* Win8MenuBarButtonStyle::GetSubMenuHost()
			{
				return 0;
			}

			void Win8MenuBarButtonStyle::SetImage(Ptr<GuiImageData> value)
			{
			}

			void Win8MenuBarButtonStyle::SetShortcutText(const WString& value)
			{
			}

			void Win8MenuBarButtonStyle::Transfer(GuiButton::ControlState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isOpening);
				}
			}

/***********************************************************************
Win8MenuItemButtonStyle
***********************************************************************/

			void Win8MenuItemButtonStyle::TransferInternal(GuiButton::ControlState value, bool enabled, bool selected, bool opening)
			{
				Win8ButtonColors targetColor;
				bool active=false;
				if(enabled)
				{
					if(opening)
					{
						targetColor=Win8ButtonColors::MenuItemButtonNormalActive();
						active=true;
					}
					else
					{
						switch(value)
						{
						case GuiButton::Normal:
							targetColor=selected?Win8ButtonColors::MenuItemButtonSelected():Win8ButtonColors::MenuItemButtonNormal();
							break;
						case GuiButton::Active:
						case GuiButton::Pressed:
							targetColor=selected?Win8ButtonColors::MenuItemButtonSelectedActive():Win8ButtonColors::MenuItemButtonNormalActive();
							active=true;
							break;
						}
					}
				}
				else
				{
					switch(value)
					{
					case GuiButton::Normal:
						targetColor=Win8ButtonColors::MenuItemButtonDisabled();
						break;
					case GuiButton::Active:
					case GuiButton::Pressed:
						targetColor=Win8ButtonColors::MenuItemButtonDisabledActive();
						active=true;
						break;
					}
				}
				elements.Apply(targetColor);
				elements.SetActive(active || selected);
			}

			Win8MenuItemButtonStyle::Win8MenuItemButtonStyle()
				:controlStyle(GuiButton::Normal)
				,isVisuallyEnabled(true)
				,isSelected(false)
				,isOpening(false)
			{
				elements=Win8MenuItemButtonElements::Create();
				elements.Apply(Win8ButtonColors::MenuItemButtonNormal());
			}

			Win8MenuItemButtonStyle::~Win8MenuItemButtonStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8MenuItemButtonStyle::GetBoundsComposition()
			{
				return elements.mainComposition;
			}

			compositions::GuiGraphicsComposition* Win8MenuItemButtonStyle::GetContainerComposition()
			{
				return elements.mainComposition;
			}

			void Win8MenuItemButtonStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8MenuItemButtonStyle::SetText(const WString& value)
			{
				elements.textElement->SetText(value);
			}

			void Win8MenuItemButtonStyle::SetFont(const FontProperties& value)
			{
				Win8SetFont(elements.textElement, elements.textComposition, value);
				Win8SetFont(elements.shortcutElement, elements.shortcutComposition, value);
			}

			void Win8MenuItemButtonStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					elements.imageElement->SetEnabled(value);
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			void Win8MenuItemButtonStyle::SetSelected(bool value)
			{
				if(isSelected!=value)
				{
					isSelected=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			controls::GuiMenu::IStyleController* Win8MenuItemButtonStyle::CreateSubMenuStyleController()
			{
				return new Win8MenuStyle;
			}

			void Win8MenuItemButtonStyle::SetSubMenuExisting(bool value)
			{
				elements.SetSubMenuExisting(value);
			}

			void Win8MenuItemButtonStyle::SetSubMenuOpening(bool value)
			{
				if(isOpening!=value)
				{
					isOpening=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			controls::GuiButton* Win8MenuItemButtonStyle::GetSubMenuHost()
			{
				return 0;
			}

			void Win8MenuItemButtonStyle::SetImage(Ptr<GuiImageData> value)
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

			void Win8MenuItemButtonStyle::SetShortcutText(const WString& value)
			{
				elements.shortcutElement->SetText(value);
			}

			void Win8MenuItemButtonStyle::Transfer(GuiButton::ControlState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

/***********************************************************************
Win8MenuSplitterStyle
***********************************************************************/

			Win8MenuSplitterStyle::Win8MenuSplitterStyle()
			{
				boundsComposition=new GuiBoundsComposition;
				boundsComposition->SetPreferredMinSize(Size(26, 5));
				{
					GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
					element->SetColor(Win8GetMenuSplitterColor());

					GuiBoundsComposition* composition=new GuiBoundsComposition;
					composition->SetPreferredMinSize(Size(1, 0));
					composition->SetOwnedElement(element);
					composition->SetAlignmentToParent(Margin(26, 0, -1, 0));
					boundsComposition->AddChild(composition);
				}
				{
					GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
					element->SetColor(Win8GetMenuSplitterColor());

					GuiBoundsComposition* composition=new GuiBoundsComposition;
					composition->SetPreferredMinSize(Size(0, 1));
					composition->SetOwnedElement(element);
					composition->SetAlignmentToParent(Margin(27, 2, 0, 2));
					boundsComposition->AddChild(composition);
				}
			}

			Win8MenuSplitterStyle::~Win8MenuSplitterStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8MenuSplitterStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8MenuSplitterStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win8MenuSplitterStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8MenuSplitterStyle::SetText(const WString& value)
			{
			}

			void Win8MenuSplitterStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8MenuSplitterStyle::SetVisuallyEnabled(bool value)
			{
			}
		}
	}
}