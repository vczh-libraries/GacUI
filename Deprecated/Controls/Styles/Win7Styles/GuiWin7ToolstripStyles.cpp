#include "GuiWin7ToolstripStyles.h"

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
Win7WindowStyle
***********************************************************************/

			Win7ToolstripToolBarStyle::Win7ToolstripToolBarStyle()
				:Win7EmptyStyle(Win7GetSystemWindowColor())
			{
				boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			}

			Win7ToolstripToolBarStyle::~Win7ToolstripToolBarStyle()
			{
			}

/***********************************************************************
Win7ToolstripButtonDropdownStyle
***********************************************************************/

			void Win7ToolstripButtonDropdownStyle::TransferInternal(controls::ButtonState value, bool enabled)
			{
				splitterComposition->SetVisible(value!= ButtonState::Normal && enabled);
			}

			Win7ToolstripButtonDropdownStyle::Win7ToolstripButtonDropdownStyle()
				:isVisuallyEnabled(false)
				,controlState(ButtonState::Normal)
			{
				boundsComposition=new GuiBoundsComposition;
				boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				{
					Gui3DSplitterElement* splitterElement=Gui3DSplitterElement::Create();
					splitterElement->SetColors(Win7GetSystemBorderSinkColor(), Win7GetSystemBorderRaiseColor());
					splitterElement->SetDirection(Gui3DSplitterElement::Vertical);

					splitterComposition=new GuiBoundsComposition;
					splitterComposition->SetAlignmentToParent(Margin(0, 3, -1, 3));
					splitterComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
					splitterComposition->SetOwnedElement(splitterElement);
					splitterComposition->SetPreferredMinSize(Size(2, 0));
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

			Win7ToolstripButtonDropdownStyle::~Win7ToolstripButtonDropdownStyle()
			{
			}
				
			compositions::GuiBoundsComposition* Win7ToolstripButtonDropdownStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7ToolstripButtonDropdownStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win7ToolstripButtonDropdownStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7ToolstripButtonDropdownStyle::SetText(const WString& value)
			{
			}

			void Win7ToolstripButtonDropdownStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7ToolstripButtonDropdownStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					TransferInternal(controlState, isVisuallyEnabled);
				}
			}

			void Win7ToolstripButtonDropdownStyle::Transfer(controls::ButtonState value)
			{
				if(controlState!=value)
				{
					controlState=value;
					TransferInternal(controlState, isVisuallyEnabled);
				}
			}

/***********************************************************************
Win7ToolstripButtonStyle
***********************************************************************/

			IMPLEMENT_TRANSFERRING_ANIMATION(Win7ButtonColors, Win7ToolstripButtonStyle)
			{
				colorCurrent=Win7ButtonColors::Blend(colorBegin, colorEnd, currentPosition, totalLength);
				style->elements.Apply(colorCurrent);
			}

			void Win7ToolstripButtonStyle::TransferInternal(ButtonState value, bool enabled, bool selected, bool menuOpening)
			{
				Win7ButtonColors targetColor;
				if(enabled)
				{
					if(menuOpening)
					{
						value= ButtonState::Pressed;
					}
					switch(value)
					{
					case ButtonState::Normal:
						targetColor=selected?Win7ButtonColors::ToolstripButtonSelected():Win7ButtonColors::ToolstripButtonNormal();
						break;
					case ButtonState::Active:
						targetColor=selected?Win7ButtonColors::ToolstripButtonSelected():Win7ButtonColors::ToolstripButtonActive();
						break;
					case ButtonState::Pressed:
						targetColor=Win7ButtonColors::ToolstripButtonPressed();
						break;
					}
				}
				else
				{
					targetColor=Win7ButtonColors::ToolstripButtonDisabled();
				}
				transferringAnimation->Transfer(targetColor);
			}

			Win7ToolstripButtonStyle::Win7ToolstripButtonStyle(ButtonStyle _buttonStyle)
				:controlStyle(ButtonState::Normal)
				,isVisuallyEnabled(true)
				,isSelected(false)
				,isOpening(false)
				,buttonStyle(_buttonStyle)
				,subMenuHost(0)
			{
				elements=Win7ButtonElements::Create(true, true, Alignment::Center, Alignment::Center);
				elements.Apply(Win7ButtonColors::ToolstripButtonNormal());
				transferringAnimation=new TransferringAnimation(this, Win7ButtonColors::ToolstripButtonNormal());

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

								subMenuHost=new GuiButton(new Win7ToolstripButtonDropdownStyle);
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

			Win7ToolstripButtonStyle::~Win7ToolstripButtonStyle()
			{
				transferringAnimation->Disable();
			}

			compositions::GuiBoundsComposition* Win7ToolstripButtonStyle::GetBoundsComposition()
			{
				return elements.mainComposition;
			}

			compositions::GuiGraphicsComposition* Win7ToolstripButtonStyle::GetContainerComposition()
			{
				return elements.mainComposition;
			}

			void Win7ToolstripButtonStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7ToolstripButtonStyle::SetText(const WString& value)
			{
			}

			void Win7ToolstripButtonStyle::SetFont(const FontProperties& value)
			{
				Win7SetFont(elements.textElement, elements.textComposition, value);
			}

			void Win7ToolstripButtonStyle::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					imageElement->SetEnabled(value);
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			void Win7ToolstripButtonStyle::SetSelected(bool value)
			{
				if(isSelected!=value)
				{
					isSelected=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			controls::GuiMenu::IStyleController* Win7ToolstripButtonStyle::CreateSubMenuStyleController()
			{
				return new Win7MenuStyle;
			}

			void Win7ToolstripButtonStyle::SetSubMenuExisting(bool value)
			{
			}

			void Win7ToolstripButtonStyle::SetSubMenuOpening(bool value)
			{
				if(isOpening!=value)
				{
					isOpening=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

			controls::GuiButton* Win7ToolstripButtonStyle::GetSubMenuHost()
			{
				return subMenuHost;
			}

			void Win7ToolstripButtonStyle::SetImage(Ptr<GuiImageData> value)
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

			void Win7ToolstripButtonStyle::SetShortcutText(const WString& value)
			{
			}

			void Win7ToolstripButtonStyle::Transfer(controls::ButtonState value)
			{
				if(controlStyle!=value)
				{
					controlStyle=value;
					TransferInternal(controlStyle, isVisuallyEnabled, isSelected, isOpening);
				}
			}

/***********************************************************************
Win7ToolstripSplitterStyle
***********************************************************************/

			Win7ToolstripSplitterStyle::Win7ToolstripSplitterStyle()
			{
				Color dark=Win7GetSystemBorderSinkColor();
				Color bright=Win7GetSystemBorderRaiseColor();

				GuiBoundsComposition* bounds=new GuiBoundsComposition;
				bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				boundsComposition=bounds;

				GuiSolidBackgroundElement* backgroundElement=GuiSolidBackgroundElement::Create();
				bounds->SetOwnedElement(backgroundElement);
				backgroundElement->SetColor(Win7GetSystemWindowColor());

				GuiBoundsComposition* splitterComposition=new GuiBoundsComposition;
				bounds->AddChild(splitterComposition);
				splitterComposition->SetAlignmentToParent(Margin(3, 3, 3, 3));

				Gui3DSplitterElement* splitterElement=Gui3DSplitterElement::Create();
				splitterComposition->SetOwnedElement(splitterElement);
				splitterElement->SetColors(dark, bright);
				splitterElement->SetDirection(Gui3DSplitterElement::Vertical);
			}

			Win7ToolstripSplitterStyle::~Win7ToolstripSplitterStyle()
			{
			}

			compositions::GuiBoundsComposition* Win7ToolstripSplitterStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7ToolstripSplitterStyle::GetContainerComposition()
			{
				return boundsComposition;
			}

			void Win7ToolstripSplitterStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7ToolstripSplitterStyle::SetText(const WString& value)
			{
			}

			void Win7ToolstripSplitterStyle::SetFont(const FontProperties& value)
			{
			}

			void Win7ToolstripSplitterStyle::SetVisuallyEnabled(bool value)
			{
			}
		}
	}
}