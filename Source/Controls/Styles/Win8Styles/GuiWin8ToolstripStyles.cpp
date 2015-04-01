#include "GuiWin8ToolstripStyles.h"

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
Win8WindowStyle
***********************************************************************/

			Win8ToolstripToolBarStyle::Win8ToolstripToolBarStyle()
			{
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Win8GetSystemWindowColor());
				
					boundsComposition=new GuiBoundsComposition;
					boundsComposition->SetOwnedElement(element);
					boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				}
				{
					Gui3DSplitterElement* element=Gui3DSplitterElement::Create();
					element->SetColors(Color(160, 160, 160), Color(255, 255, 255));
					element->SetDirection(Gui3DSplitterElement::Horizontal);

					GuiBoundsComposition* composition=new GuiBoundsComposition;
					composition->SetPreferredMinSize(Size(0, 2));
					composition->SetAlignmentToParent(Margin(0, 0, 0, -1));
					composition->SetOwnedElement(element);
					boundsComposition->AddChild(composition);
				}
				{
					containerComposition=new GuiBoundsComposition;
					containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					containerComposition->SetAlignmentToParent(Margin(0, 2, 0, 0));
					boundsComposition->AddChild(containerComposition);
				}
			}

			Win8ToolstripToolBarStyle::~Win8ToolstripToolBarStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8ToolstripToolBarStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8ToolstripToolBarStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win8ToolstripToolBarStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8ToolstripToolBarStyle::SetText(const WString& value)
			{
			}

			void Win8ToolstripToolBarStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8ToolstripToolBarStyle::SetVisuallyEnabled(bool value)
			{
			}

/***********************************************************************
Win8ToolstripButtonDropdownStyle
***********************************************************************/

			void Win8ToolstripButtonDropdownStyle::TransferInternal(controls::GuiButton::ControlState value, bool enabled)
			{
				splitterComposition->SetVisible(value!=GuiButton::Normal && enabled);
			}

			Win8ToolstripButtonDropdownStyle::Win8ToolstripButtonDropdownStyle()
				:isVisuallyEnabled(false)
				,controlState(GuiButton::Normal)
			{
				boundsComposition=new GuiBoundsComposition;
				boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				{
					GuiSolidBorderElement* splitterElement=GuiSolidBorderElement::Create();
					splitterElement->SetColor(Color(132, 132, 132));

					splitterComposition=new GuiBoundsComposition;
					splitterComposition->SetAlignmentToParent(Margin(0, 3, -1, 3));
					splitterComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
					splitterComposition->SetOwnedElement(splitterElement);
					splitterComposition->SetPreferredMinSize(Size(1, 0));
					splitterComposition->SetVisible(false);
					boundsComposition->AddChild(splitterComposition);
				}
				{
					containerComposition=new GuiBoundsComposition;
					containerComposition->SetAlignmentToParent(Margin(4, 0, 4, 0));
					containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					boundsComposition->AddChild(containerComposition);
				}
			}

			Win8ToolstripButtonDropdownStyle::~Win8ToolstripButtonDropdownStyle()
			{
			}
				
			compositions::GuiBoundsComposition* Win8ToolstripButtonDropdownStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8ToolstripButtonDropdownStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win8ToolstripButtonDropdownStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8ToolstripButtonDropdownStyle::SetText(const WString& value)
			{
			}

			void Win8ToolstripButtonDropdownStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8ToolstripButtonDropdownStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					TransferInternal(controlState, isVisuallyEnabled);
				}
			}

			void Win8ToolstripButtonDropdownStyle::Transfer(controls::GuiButton::ControlState value)
			{
				if(controlState!=value)
				{
					controlState=value;
					TransferInternal(controlState, isVisuallyEnabled);
				}
			}

/***********************************************************************
Win8ToolstripButtonStyle
***********************************************************************/

			IMPLEMENT_TRANSFERRING_ANIMATION(Win8ButtonColors, Win8ToolstripButtonStyle)
			{
				colorCurrent=Win8ButtonColors::Blend(colorBegin, colorEnd, currentPosition, totalLength);
				style->elements.Apply(colorCurrent);
			}

			void Win8ToolstripButtonStyle::TransferInternal(GuiButton::ControlState value, bool enabled, bool selected, bool menuOpening)
			{
				Win8ButtonColors targetColor;
				if(enabled)
				{
					if(menuOpening)
					{
						value=GuiButton::Pressed;
					}
					switch(value)
					{
					case GuiButton::Normal:
						targetColor=selected?Win8ButtonColors::ToolstripButtonSelected():Win8ButtonColors::ToolstripButtonNormal();
						break;
					case GuiButton::Active:
						targetColor=selected?Win8ButtonColors::ToolstripButtonSelected():Win8ButtonColors::ToolstripButtonActive();
						break;
					case GuiButton::Pressed:
						targetColor=Win8ButtonColors::ToolstripButtonPressed();
						break;
					}
				}
				else
				{
					targetColor=Win8ButtonColors::ToolstripButtonDisabled();
				}
				transferringAnimation->Transfer(targetColor);
			}

			Win8ToolstripButtonStyle::Win8ToolstripButtonStyle(ButtonStyle _buttonStyle)
				:controlStyle(GuiButton::Normal)
				,isVisuallyEnabled(true)
				,isSelected(false)
				,isOpening(false)
				,buttonStyle(_buttonStyle)
				,subMenuHost(0)
			{
				elements=Win8ButtonElements::Create(Alignment::Center, Alignment::Center);
				elements.Apply(Win8ButtonColors::ToolstripButtonNormal());
				transferringAnimation=new TransferringAnimation(this, Win8ButtonColors::ToolstripButtonNormal());

				elements.textComposition->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
				imageElement=GuiImageFrameElement::Create();
				imageComposition=new GuiBoundsComposition;
				imageComposition->SetOwnedElement(imageElement);
				imageComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
				imageComposition->SetAlignmentToParent(Margin(4, 4, 4, 4));

				if(_buttonStyle==CommandButton)
				{
					GetContainerComposition()->AddChild(imageComposition);
				}
				else
				{
					GuiTableComposition* table=new GuiTableComposition;
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetRowsAndColumns(1, 2);
					table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
					table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
					table->SetColumnOption(1, GuiCellOption::MinSizeOption());

					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 0, 1, 1);

						cell->AddChild(imageComposition);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 1, 1);
						GuiPolygonElement* arrow=0;
						GuiBoundsComposition* arrowComposition=common_styles::CommonFragmentBuilder::BuildDownArrow(arrow);

						switch(_buttonStyle)
						{
						case DropdownButton:
							{
								arrowComposition->SetAlignmentToParent(Margin(0, 0, 4, 0));
								cell->AddChild(arrowComposition);
							}
							break;
						case SplitButton:
							{

								subMenuHost=new GuiButton(new Win8ToolstripButtonDropdownStyle);
								subMenuHost->GetContainerComposition()->AddChild(arrowComposition);
								subMenuHost->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
								cell->AddChild(subMenuHost->GetBoundsComposition());
							}
							break;
						default:;
						}
					}
					GetContainerComposition()->AddChild(table);
				}
			}

			Win8ToolstripButtonStyle::~Win8ToolstripButtonStyle()
			{
				transferringAnimation->Disable();
			}

			compositions::GuiBoundsComposition* Win8ToolstripButtonStyle::GetBoundsComposition()
			{
				return elements.mainComposition;
			}

			compositions::GuiGraphicsComposition* Win8ToolstripButtonStyle::GetContainerComposition()
			{
				return elements.mainComposition;
			}

			void Win8ToolstripButtonStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8ToolstripButtonStyle::SetText(const WString& value)
			{
			}

			void Win8ToolstripButtonStyle::SetFont(const FontProperties& value)
			{
				Win8SetFont(elements.textElement, elements.textComposition, value);
			}

			void Win8ToolstripButtonStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					imageElement->SetEnabled(value);
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			void Win8ToolstripButtonStyle::SetSelected(bool value)
			{
				if(isSelected!=value)
				{
					isSelected=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			controls::GuiMenu::IStyleController* Win8ToolstripButtonStyle::CreateSubMenuStyleController()
			{
				return new Win8MenuStyle;
			}

			void Win8ToolstripButtonStyle::SetSubMenuExisting(bool value)
			{
			}

			void Win8ToolstripButtonStyle::SetSubMenuOpening(bool value)
			{
				if(isOpening!=value)
				{
					isOpening=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			controls::GuiButton* Win8ToolstripButtonStyle::GetSubMenuHost()
			{
				return subMenuHost;
			}

			void Win8ToolstripButtonStyle::SetImage(Ptr<GuiImageData> value)
			{
				if(value)
				{
					imageElement->SetImage(value->GetImage(), value->GetFrameIndex());
				}
				else
				{
					imageElement->SetImage(0, 0);
				}
			}

			void Win8ToolstripButtonStyle::SetShortcutText(const WString& value)
			{
			}

			void Win8ToolstripButtonStyle::Transfer(controls::GuiButton::ControlState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

/***********************************************************************
Win8ToolstripSplitterStyle
***********************************************************************/

			Win8ToolstripSplitterStyle::Win8ToolstripSplitterStyle()
			{
				GuiBoundsComposition* bounds=new GuiBoundsComposition;
				bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				boundsComposition=bounds;

				GuiSolidBackgroundElement* backgroundElement=GuiSolidBackgroundElement::Create();
				bounds->SetOwnedElement(backgroundElement);
				backgroundElement->SetColor(Win8GetSystemWindowColor());

				GuiBoundsComposition* splitterComposition=new GuiBoundsComposition;
				bounds->AddChild(splitterComposition);
				splitterComposition->SetPreferredMinSize(Size(1, 0));
				splitterComposition->SetAlignmentToParent(Margin(3, 3, 3, 3));

				GuiSolidBorderElement* splitterElement=GuiSolidBorderElement::Create();
				splitterComposition->SetOwnedElement(splitterElement);
				splitterElement->SetColor(Color(132, 132, 132));
			}

			Win8ToolstripSplitterStyle::~Win8ToolstripSplitterStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8ToolstripSplitterStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8ToolstripSplitterStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win8ToolstripSplitterStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8ToolstripSplitterStyle::SetText(const WString& value)
			{
			}

			void Win8ToolstripSplitterStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8ToolstripSplitterStyle::SetVisuallyEnabled(bool value)
			{
			}
		}
	}
}