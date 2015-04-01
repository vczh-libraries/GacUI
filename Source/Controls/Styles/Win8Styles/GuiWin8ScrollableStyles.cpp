#include "GuiWin8ScrollableStyles.h"

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
Win8ScrollHandleButtonStyle
***********************************************************************/

			void Win8ScrollHandleButtonStyle::TransferInternal(GuiButton::ControlState value, bool enabled, bool selected)
			{
				Win8ButtonColors targetColor;
				if(enabled)
				{
					switch(value)
					{
					case GuiButton::Normal:
						targetColor=Win8ButtonColors::ScrollHandleNormal();
						break;
					case GuiButton::Active:
						targetColor=Win8ButtonColors::ScrollHandleActive();
						break;
					case GuiButton::Pressed:
						targetColor=Win8ButtonColors::ScrollHandlePressed();
						break;
					}
				}
				else
				{
					targetColor=Win8ButtonColors::ScrollHandleDisabled();
				}
				transferringAnimation->Transfer(targetColor);
			}

			Win8ScrollHandleButtonStyle::Win8ScrollHandleButtonStyle()
				:Win8ButtonStyleBase(Win8ButtonColors::ScrollHandleNormal(), Alignment::Center, Alignment::Center)
			{
			}

			Win8ScrollHandleButtonStyle::~Win8ScrollHandleButtonStyle()
			{
			}

/***********************************************************************
Win8ScrollArrowButtonStyle
***********************************************************************/

			void Win8ScrollArrowButtonStyle::TransferInternal(GuiButton::ControlState value, bool enabled, bool selected)
			{
				Win8ButtonColors targetColor;
				if(enabled)
				{
					switch(value)
					{
					case GuiButton::Normal:
						targetColor=Win8ButtonColors::ScrollArrowNormal();
						break;
					case GuiButton::Active:
						targetColor=Win8ButtonColors::ScrollArrowActive();
						break;
					case GuiButton::Pressed:
						targetColor=Win8ButtonColors::ScrollArrowPressed();
						break;
					}
				}
				else
				{
					targetColor=Win8ButtonColors::ScrollArrowDisabled();
				}
				transferringAnimation->Transfer(targetColor);
			}

			void Win8ScrollArrowButtonStyle::AfterApplyColors(const Win8ButtonColors& colors)
			{
				Win8ButtonStyleBase::AfterApplyColors(colors);
				arrowElement->SetBorderColor(colors.textColor);
				arrowElement->SetBackgroundColor(colors.textColor);
			}

			Win8ScrollArrowButtonStyle::Win8ScrollArrowButtonStyle(common_styles::CommonScrollStyle::Direction direction, bool increaseButton)
				:Win8ButtonStyleBase(Win8ButtonColors::ScrollArrowNormal(), Alignment::Center, Alignment::Center)
			{
				switch(direction)
				{
				case common_styles::CommonScrollStyle::Horizontal:
					if(increaseButton)
					{
						GetContainerComposition()->AddChild(common_styles::CommonFragmentBuilder::BuildRightArrow(arrowElement));
					}
					else
					{
						GetContainerComposition()->AddChild(common_styles::CommonFragmentBuilder::BuildLeftArrow(arrowElement));
					}
					break;
				case common_styles::CommonScrollStyle::Vertical:
					if(increaseButton)
					{
						GetContainerComposition()->AddChild(common_styles::CommonFragmentBuilder::BuildDownArrow(arrowElement));
					}
					else
					{
						GetContainerComposition()->AddChild(common_styles::CommonFragmentBuilder::BuildUpArrow(arrowElement));
					}
					break;
				}
			}

			Win8ScrollArrowButtonStyle::~Win8ScrollArrowButtonStyle()
			{
			}

/***********************************************************************
Win8ScrollStyle
***********************************************************************/

			controls::GuiButton::IStyleController* Win8ScrollStyle::CreateDecreaseButtonStyle(Direction direction)
			{
				Win8ScrollArrowButtonStyle* decreaseButtonStyle=new Win8ScrollArrowButtonStyle(direction, false);
				return decreaseButtonStyle;
			}

			controls::GuiButton::IStyleController* Win8ScrollStyle::CreateIncreaseButtonStyle(Direction direction)
			{
				Win8ScrollArrowButtonStyle* increaseButtonStyle=new Win8ScrollArrowButtonStyle(direction, true);
				return increaseButtonStyle;
			}

			controls::GuiButton::IStyleController* Win8ScrollStyle::CreateHandleButtonStyle(Direction direction)
			{
				Win8ScrollHandleButtonStyle* handleButtonStyle=new Win8ScrollHandleButtonStyle;
				return handleButtonStyle;
			}

			compositions::GuiBoundsComposition* Win8ScrollStyle::InstallBackground(compositions::GuiBoundsComposition* boundsComposition, Direction direction)
			{
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Win8GetSystemWindowColor());
					
					boundsComposition->SetOwnedElement(element);
				}

				return boundsComposition;
			}

			Win8ScrollStyle::Win8ScrollStyle(Direction _direction)
				:CommonScrollStyle(_direction)
			{
				BuildStyle(DefaultSize, ArrowSize);
			}

			Win8ScrollStyle::~Win8ScrollStyle()
			{
			}

/***********************************************************************
Win8TrackStyle
***********************************************************************/

			controls::GuiButton::IStyleController* Win8TrackStyle::CreateHandleButtonStyle(Direction direction)
			{
				Win8ButtonStyle* handleButtonStyle=new Win8ButtonStyle;
				return handleButtonStyle;
			}

			void Win8TrackStyle::InstallBackground(compositions::GuiGraphicsComposition* boundsComposition, Direction direction)
			{
				GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
				element->SetColor(Win8GetSystemWindowColor());
				boundsComposition->SetOwnedElement(element);
			}

			void Win8TrackStyle::InstallTrack(compositions::GuiGraphicsComposition* trackComposition, Direction direction)
			{
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Color(231, 234, 234));
					trackComposition->SetOwnedElement(element);
				}
				{
					GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
					element->SetColor(Color(214, 214, 214));
					
					GuiBoundsComposition* composition=new GuiBoundsComposition;
					composition->SetOwnedElement(element);
					composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
					trackComposition->AddChild(composition);
				}
			}

			Win8TrackStyle::Win8TrackStyle(Direction _direction)
				:CommonTrackStyle(_direction)
			{
				BuildStyle(TrackThickness, TrackPadding, HandleLong, HandleShort);
			}

			Win8TrackStyle::~Win8TrackStyle()
			{
			}

/***********************************************************************
Win8ProgressBarStyle
***********************************************************************/

			void Win8ProgressBarStyle::UpdateProgressBar()
			{
				vint max=totalSize-pageSize;
				if(position<0)
				{
					progressComposition->SetWidthPageSize(0);
				}
				else if(position>=max)
				{
					progressComposition->SetWidthPageSize(1);
				}
				else
				{
					progressComposition->SetWidthPageSize((double)position/max);
				}
			}

			Win8ProgressBarStyle::Win8ProgressBarStyle()
				:totalSize(1)
				,pageSize(0)
				,position(0)
			{
				{
					boundsComposition=new GuiBoundsComposition;
				}
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Color(230, 230, 230));

					containerComposition=new GuiBoundsComposition;
					containerComposition->SetAlignmentToParent(Margin(1, 1, 1, 1));
					containerComposition->SetOwnedElement(element);
					boundsComposition->AddChild(containerComposition);
				}
				{
					GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
					element->SetColor(Color(188, 188, 188));

					GuiBoundsComposition* borderComposition=new GuiBoundsComposition;
					borderComposition->SetOwnedElement(element);
					borderComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
					boundsComposition->AddChild(borderComposition);
				}
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(Color(6, 176, 37));

					progressComposition=new GuiPartialViewComposition;
					progressComposition->SetWidthRatio(0);
					progressComposition->SetWidthPageSize(0);
					progressComposition->SetHeightRatio(0);
					progressComposition->SetHeightPageSize(1);
					progressComposition->SetOwnedElement(element);
					containerComposition->AddChild(progressComposition);
				}
			}

			Win8ProgressBarStyle::~Win8ProgressBarStyle()
			{
			}

			compositions::GuiBoundsComposition* Win8ProgressBarStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win8ProgressBarStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win8ProgressBarStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8ProgressBarStyle::SetText(const WString& value)
			{
			}

			void Win8ProgressBarStyle::SetFont(const FontProperties& value)
			{
			}

			void Win8ProgressBarStyle::SetVisuallyEnabled(bool value)
			{
			}

			void Win8ProgressBarStyle::SetCommandExecutor(controls::GuiScroll::ICommandExecutor* value)
			{
			}

			void Win8ProgressBarStyle::SetTotalSize(vint value)
			{
				totalSize=value;
				UpdateProgressBar();
			}

			void Win8ProgressBarStyle::SetPageSize(vint value)
			{
				pageSize=value;
				UpdateProgressBar();
			}

			void Win8ProgressBarStyle::SetPosition(vint value)
			{
				position=value;
				UpdateProgressBar();
			}

/***********************************************************************
Win8ScrollViewProvider
***********************************************************************/

			Win8ScrollViewProvider::Win8ScrollViewProvider()
			{
			}

			Win8ScrollViewProvider::~Win8ScrollViewProvider()
			{
			}

			void Win8ScrollViewProvider::AssociateStyleController(controls::GuiControl::IStyleController* controller)
			{
			}
			
			void Win8ScrollViewProvider::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win8ScrollViewProvider::SetText(const WString& value)
			{
			}

			void Win8ScrollViewProvider::SetFont(const FontProperties& value)
			{
			}

			void Win8ScrollViewProvider::SetVisuallyEnabled(bool value)
			{
			}

			controls::GuiScroll::IStyleController* Win8ScrollViewProvider::CreateHorizontalScrollStyle()
			{
				return new Win8ScrollStyle(Win8ScrollStyle::Horizontal);
			}

			controls::GuiScroll::IStyleController* Win8ScrollViewProvider::CreateVerticalScrollStyle()
			{
				return new Win8ScrollStyle(Win8ScrollStyle::Vertical);
			}

			vint Win8ScrollViewProvider::GetDefaultScrollSize()
			{
				return Win8ScrollStyle::DefaultSize;
			}

			compositions::GuiGraphicsComposition* Win8ScrollViewProvider::InstallBackground(compositions::GuiBoundsComposition* boundsComposition)
			{
				GuiSolidBorderElement* border=GuiSolidBorderElement::Create();
				border->SetColor(Win8GetSystemBorderColor());
				boundsComposition->SetOwnedElement(border);
				boundsComposition->SetInternalMargin(Margin(1, 1, 1, 1));
				
				GuiSolidBackgroundElement* background=GuiSolidBackgroundElement::Create();
				background->SetColor(Win8GetSystemWindowColor());

				GuiBoundsComposition* backgroundComposition=new GuiBoundsComposition;
				boundsComposition->AddChild(backgroundComposition);
				backgroundComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				backgroundComposition->SetOwnedElement(background);

				return boundsComposition;
			}

/***********************************************************************
Win8TextBoxBackground
***********************************************************************/

#define HOST_GETTER_BY_FOCUSABLE_COMPOSITION(STYLE) (style->focusableComposition->GetRelatedGraphicsHost())

			IMPLEMENT_TRANSFERRING_ANIMATION_BASE(Win8TextBoxColors, Win8TextBoxBackground, HOST_GETTER_BY_FOCUSABLE_COMPOSITION)
			{
				colorCurrent=Win8TextBoxColors::Blend(colorBegin, colorEnd, currentPosition, totalLength);
				style->Apply(colorCurrent);
			}

			void Win8TextBoxBackground::UpdateStyle()
			{
				if(!isVisuallyEnabled)
				{
					transferringAnimation->Transfer(Win8TextBoxColors::Disabled());
				}
				else if(isFocused)
				{
					transferringAnimation->Transfer(Win8TextBoxColors::Focused());
				}
				else if(isMouseEnter)
				{
					transferringAnimation->Transfer(Win8TextBoxColors::Active());
				}
				else
				{
					transferringAnimation->Transfer(Win8TextBoxColors::Normal());
				}
			}

			void Win8TextBoxBackground::Apply(const Win8TextBoxColors& colors)
			{
				borderElement->SetColor(colors.borderColor);
				backgroundElement->SetColor(colors.backgroundColor);
			}

			void Win8TextBoxBackground::OnBoundsMouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				isMouseEnter=true;
				UpdateStyle();
			}

			void Win8TextBoxBackground::OnBoundsMouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				isMouseEnter=false;
				UpdateStyle();
			}

			void Win8TextBoxBackground::OnBoundsGotFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				isFocused=true;
				UpdateStyle();
			}

			void Win8TextBoxBackground::OnBoundsLostFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				isFocused=false;
				UpdateStyle();
			}

			Win8TextBoxBackground::Win8TextBoxBackground()
				:backgroundElement(0)
				,borderElement(0)
				,focusableComposition(0)
				,isMouseEnter(false)
				,isFocused(false)
				,isVisuallyEnabled(false)
				,styleController(0)
				,textElement(0)
			{
				transferringAnimation=new TransferringAnimation(this, Win8TextBoxColors::Normal());
			}

			Win8TextBoxBackground::~Win8TextBoxBackground()
			{
				transferringAnimation->Disable();
			}

			void Win8TextBoxBackground::AssociateStyleController(controls::GuiControl::IStyleController* controller)
			{
				styleController=controller;
			}
			
			void Win8TextBoxBackground::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
				focusableComposition=value;
				focusableComposition->GetEventReceiver()->mouseEnter.AttachMethod(this, &Win8TextBoxBackground::OnBoundsMouseEnter);
				focusableComposition->GetEventReceiver()->mouseLeave.AttachMethod(this, &Win8TextBoxBackground::OnBoundsMouseLeave);
				focusableComposition->GetEventReceiver()->gotFocus.AttachMethod(this, &Win8TextBoxBackground::OnBoundsGotFocus);
				focusableComposition->GetEventReceiver()->lostFocus.AttachMethod(this, &Win8TextBoxBackground::OnBoundsLostFocus);
			}

			void Win8TextBoxBackground::SetVisuallyEnabled(bool value)
			{
				isVisuallyEnabled=value;
				UpdateStyle();
			}

			compositions::GuiGraphicsComposition* Win8TextBoxBackground::InstallBackground(compositions::GuiBoundsComposition* boundsComposition)
			{
				{
					GuiSolidBackgroundElement* background=GuiSolidBackgroundElement::Create();
					background->SetColor(Color(255, 255, 255));

					GuiBoundsComposition* backgroundComposition=new GuiBoundsComposition;
					boundsComposition->AddChild(backgroundComposition);
					backgroundComposition->SetAlignmentToParent(Margin(1, 1, 1, 1));
					backgroundComposition->SetOwnedElement(background);
				}
				{
					GuiSolidBackgroundElement* background=GuiSolidBackgroundElement::Create();
					background->SetColor(Color(255, 255, 255));

					GuiBoundsComposition* backgroundComposition=new GuiBoundsComposition;
					boundsComposition->AddChild(backgroundComposition);
					backgroundComposition->SetAlignmentToParent(Margin(2, 2, 2, 2));
					backgroundComposition->SetOwnedElement(background);
					backgroundElement=background;
				}
				{
					GuiSolidBorderElement* border=GuiSolidBorderElement::Create();
					border->SetColor(Win8GetSystemBorderColor());
					borderElement=border;

					GuiBoundsComposition* borderComposition=new GuiBoundsComposition;
					boundsComposition->AddChild(borderComposition);
					borderComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
					borderComposition->SetOwnedElement(border);
				}
				Apply(Win8TextBoxColors::Normal());
				{
					GuiBoundsComposition* containerComposition=new GuiBoundsComposition;
					boundsComposition->AddChild(containerComposition);
					containerComposition->SetAlignmentToParent(Margin(2, 2, 2, 2));
					return containerComposition;
				}
			}

			void Win8TextBoxBackground::InitializeTextElement(elements::GuiColorizedTextElement* _textElement)
			{
				textElement=_textElement;

				Array<text::ColorEntry> colors;
				colors.Resize(1);
				{
					colors[0]=Win8GetTextBoxTextColor();
				}
				textElement->SetColors(colors);
				textElement->SetCaretColor(Color(0, 0, 0));
			}

/***********************************************************************
Win8MultilineTextBoxProvider
***********************************************************************/

			Win8MultilineTextBoxProvider::Win8MultilineTextBoxProvider()
				:styleController(0)
			{
			}

			Win8MultilineTextBoxProvider::~Win8MultilineTextBoxProvider()
			{
			}

			void Win8MultilineTextBoxProvider::AssociateStyleController(controls::GuiControl::IStyleController* controller)
			{
				styleController=controller;
				background.AssociateStyleController(controller);
			}
			
			void Win8MultilineTextBoxProvider::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
				background.SetFocusableComposition(value);
				GuiMultilineTextBox::StyleController* textBoxController=dynamic_cast<GuiMultilineTextBox::StyleController*>(styleController);
				if(textBoxController)
				{
					background.InitializeTextElement(textBoxController->GetTextElement());
				}
			}

			void Win8MultilineTextBoxProvider::SetVisuallyEnabled(bool value)
			{
				background.SetVisuallyEnabled(value);
			}

			compositions::GuiGraphicsComposition* Win8MultilineTextBoxProvider::InstallBackground(compositions::GuiBoundsComposition* boundsComposition)
			{
				return background.InstallBackground(boundsComposition);
			}

/***********************************************************************
Win8SinglelineTextBoxProvider
***********************************************************************/

			Win8SinglelineTextBoxProvider::Win8SinglelineTextBoxProvider()
				:styleController(0)
			{
			}

			Win8SinglelineTextBoxProvider::~Win8SinglelineTextBoxProvider()
			{
			}

			void Win8SinglelineTextBoxProvider::AssociateStyleController(controls::GuiControl::IStyleController* controller)
			{
				styleController=controller;
				background.AssociateStyleController(controller);
			}
			
			void Win8SinglelineTextBoxProvider::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
				background.SetFocusableComposition(value);
				GuiSinglelineTextBox::StyleController* textBoxController=dynamic_cast<GuiSinglelineTextBox::StyleController*>(styleController);
				background.InitializeTextElement(textBoxController->GetTextElement());
			}

			void Win8SinglelineTextBoxProvider::SetText(const WString& value)
			{
			}

			void Win8SinglelineTextBoxProvider::SetFont(const FontProperties& value)
			{
			}

			void Win8SinglelineTextBoxProvider::SetVisuallyEnabled(bool value)
			{
				background.SetVisuallyEnabled(value);
			}

			compositions::GuiGraphicsComposition* Win8SinglelineTextBoxProvider::InstallBackground(compositions::GuiBoundsComposition* boundsComposition)
			{
				return background.InstallBackground(boundsComposition);
			}

/***********************************************************************
Win8DocumentViewerStyle
***********************************************************************/

			Win8DocumentViewerStyle::Win8DocumentViewerStyle()
			{
			}

			Win8DocumentViewerStyle::~Win8DocumentViewerStyle()
			{
			}

			Ptr<DocumentModel> Win8DocumentViewerStyle::GetBaselineDocument()
			{
				return nullptr;
			}

/***********************************************************************
Win8DocumentlabelStyle
***********************************************************************/

			Win8DocumentlabelStyle::Win8DocumentlabelStyle()
			{
			}

			Win8DocumentlabelStyle::~Win8DocumentlabelStyle()
			{
			}

			Ptr<DocumentModel> Win8DocumentlabelStyle::GetBaselineDocument()
			{
				return nullptr;
			}
		}
	}
}