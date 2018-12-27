#include "GuiTextControls.h"
#include "../../GraphicsComposition/GuiGraphicsTableComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace elements::text;
			using namespace compositions;
			using namespace collections;

/***********************************************************************
GuiMultilineTextBox::DefaultTextElementOperatorCallback
***********************************************************************/

			GuiMultilineTextBox::TextElementOperatorCallback::TextElementOperatorCallback(GuiMultilineTextBox* _textControl)
				:GuiTextBoxCommonInterface::DefaultCallback(
					_textControl->textElement,
					_textControl->textComposition
					)
				,textControl(_textControl)
			{
			}

			void GuiMultilineTextBox::TextElementOperatorCallback::AfterModify(TextPos originalStart, TextPos originalEnd, const WString& originalText, TextPos inputStart, TextPos inputEnd, const WString& inputText)
			{
				textControl->CalculateView();
			}
			
			void GuiMultilineTextBox::TextElementOperatorCallback::ScrollToView(Point point)
			{
				point.x+=TextMargin;
				point.y+=TextMargin;
				Point oldPoint = textControl->GetViewPosition();
				vint marginX=0;
				vint marginY=0;
				if(oldPoint.x<point.x)
				{
					marginX=TextMargin;
				}
				else if(oldPoint.x>point.x)
				{
					marginX=-TextMargin;
				}
				if(oldPoint.y<point.y)
				{
					marginY=TextMargin;
				}
				else if(oldPoint.y>point.y)
				{
					marginY=-TextMargin;
				}
				textControl->SetViewPosition(Point(point.x + marginX, point.y + marginY));
			}

			vint GuiMultilineTextBox::TextElementOperatorCallback::GetTextMargin()
			{
				return TextMargin;
			}

/***********************************************************************
GuiMultilineTextBox::CommandExecutor
***********************************************************************/

			GuiMultilineTextBox::CommandExecutor::CommandExecutor(GuiMultilineTextBox* _textBox)
				:textBox(_textBox)
			{
			}

			GuiMultilineTextBox::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiMultilineTextBox::CommandExecutor::UnsafeSetText(const WString& value)
			{
				textBox->UnsafeSetText(value);
			}

/***********************************************************************
GuiMultilineTextBox
***********************************************************************/

			void GuiMultilineTextBox::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject(false);
				if (!ct) return;

				ct->SetCommands(nullptr);
			}

			void GuiMultilineTextBox::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				Array<text::ColorEntry> colors(1);
				colors[0] = ct->GetTextColor();
				textElement->SetColors(colors);
				textElement->SetCaretColor(ct->GetCaretColor());
				ct->SetCommands(commandExecutor.Obj());
			}

			void GuiMultilineTextBox::UpdateVisuallyEnabled()
			{
				GuiControl::UpdateVisuallyEnabled();
				textElement->SetVisuallyEnabled(GetVisuallyEnabled());
			}

			void GuiMultilineTextBox::UpdateDisplayFont()
			{
				GuiControl::UpdateDisplayFont();
				textElement->SetFont(GetDisplayFont());
				CalculateViewAndSetScroll();
			}

			void GuiMultilineTextBox::OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)
			{
				CalculateViewAndSetScroll();
				GuiScrollView::OnRenderTargetChanged(renderTarget);
			}

			Size GuiMultilineTextBox::QueryFullSize()
			{
				text::TextLines& lines = textElement->GetLines();
				return Size(lines.GetMaxWidth() + TextMargin * 2, lines.GetMaxHeight() + TextMargin * 2);
			}

			void GuiMultilineTextBox::UpdateView(Rect viewBounds)
			{
				textElement->SetViewPosition(viewBounds.LeftTop() - Size(TextMargin, TextMargin));
			}

			void GuiMultilineTextBox::CalculateViewAndSetScroll()
			{
				auto ct = GetControlTemplateObject(true);
				CalculateView();
				vint smallMove = textElement->GetLines().GetRowHeight();
				vint bigMove = smallMove * 5;

				if (auto scroll = ct->GetHorizontalScroll())
				{
					scroll->SetSmallMove(smallMove);
					scroll->SetBigMove(bigMove);
				}

				if (auto scroll = ct->GetVerticalScroll())
				{
					scroll->SetSmallMove(smallMove);
					scroll->SetBigMove(bigMove);
				}
			}

			void GuiMultilineTextBox::OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					SetFocus();
				}
			}

			GuiMultilineTextBox::GuiMultilineTextBox(theme::ThemeName themeName)
				:GuiScrollView(themeName)
			{
				textElement = GuiColorizedTextElement::Create();
				textElement->SetFont(GetDisplayFont());

				textComposition = new GuiBoundsComposition;
				textComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				textComposition->SetOwnedElement(textElement);
				containerComposition->AddChild(textComposition);

				callback = new TextElementOperatorCallback(this);
				commandExecutor = new CommandExecutor(this);

				SetAcceptTabInput(true);
				SetFocusableComposition(boundsComposition);
				Install(textElement, textComposition, this, boundsComposition, focusableComposition);
				SetCallback(callback.Obj());

				boundsComposition->GetEventReceiver()->leftButtonDown.AttachMethod(this, &GuiMultilineTextBox::OnBoundsMouseButtonDown);
				boundsComposition->GetEventReceiver()->middleButtonDown.AttachMethod(this, &GuiMultilineTextBox::OnBoundsMouseButtonDown);
				boundsComposition->GetEventReceiver()->rightButtonDown.AttachMethod(this, &GuiMultilineTextBox::OnBoundsMouseButtonDown);
			}

			GuiMultilineTextBox::~GuiMultilineTextBox()
			{
			}

			const WString& GuiMultilineTextBox::GetText()
			{
				text = textElement->GetLines().GetText();
				return text;
			}

			void GuiMultilineTextBox::SetText(const WString& value)
			{
				UnsafeSetText(value);
				textElement->SetCaretBegin(TextPos(0, 0));
				textElement->SetCaretEnd(TextPos(0, 0));
				CalculateView();
			}

/***********************************************************************
GuiSinglelineTextBox::DefaultTextElementOperatorCallback
***********************************************************************/

			GuiSinglelineTextBox::TextElementOperatorCallback::TextElementOperatorCallback(GuiSinglelineTextBox* _textControl)
				:GuiTextBoxCommonInterface::DefaultCallback(
					_textControl->textElement,
					_textControl->textComposition
					)
			{
			}

			bool GuiSinglelineTextBox::TextElementOperatorCallback::BeforeModify(TextPos start, TextPos end, const WString& originalText, WString& inputText)
			{
				vint length=inputText.Length();
				const wchar_t* input=inputText.Buffer();
				for(vint i=0;i<length;i++)
				{
					if(*input==0 || *input==L'\r' || *input==L'\n')
					{
						length=i;
						break;
					}
				}
				if(length!=inputText.Length())
				{
					inputText=inputText.Left(length);
				}
				return true;
			}

			void GuiSinglelineTextBox::TextElementOperatorCallback::AfterModify(TextPos originalStart, TextPos originalEnd, const WString& originalText, TextPos inputStart, TextPos inputEnd, const WString& inputText)
			{
			}
			
			void GuiSinglelineTextBox::TextElementOperatorCallback::ScrollToView(Point point)
			{
				vint newX=point.x;
				vint oldX=textElement->GetViewPosition().x;
				vint marginX=0;
				if(oldX<newX)
				{
					marginX=TextMargin;
				}
				else if(oldX>newX)
				{
					marginX=-TextMargin;
				}

				newX+=marginX;
				vint minX=-TextMargin;
				vint maxX=textElement->GetLines().GetMaxWidth()+TextMargin-textComposition->GetBounds().Width();
				if(newX>=maxX)
				{
					newX=maxX-1;
				}
				if(newX<minX)
				{
					newX=minX;
				}
				textElement->SetViewPosition(Point(newX, -TextMargin));
			}

			vint GuiSinglelineTextBox::TextElementOperatorCallback::GetTextMargin()
			{
				return TextMargin;
			}

/***********************************************************************
GuiSinglelineTextBox
***********************************************************************/

			void GuiSinglelineTextBox::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiSinglelineTextBox::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				Array<text::ColorEntry> colors(1);
				colors[0] = ct->GetTextColor();
				textElement->SetColors(colors);
				textElement->SetCaretColor(ct->GetCaretColor());
			}

			void GuiSinglelineTextBox::UpdateVisuallyEnabled()
			{
				GuiControl::UpdateVisuallyEnabled();
				textElement->SetVisuallyEnabled(GetVisuallyEnabled());
			}

			void GuiSinglelineTextBox::UpdateDisplayFont()
			{
				GuiControl::UpdateDisplayFont();
				textElement->SetFont(GetDisplayFont());
				RearrangeTextElement();
			}

			void GuiSinglelineTextBox::RearrangeTextElement()
			{
				textCompositionTable->SetRowOption(
					1,
					GuiCellOption::AbsoluteOption(
						textElement->GetLines().GetRowHeight() + 2 * TextMargin)
				);
			}

			void GuiSinglelineTextBox::OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)
			{
				GuiControl::OnRenderTargetChanged(renderTarget);
				RearrangeTextElement();
			}

			void GuiSinglelineTextBox::OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					SetFocus();
				}
			}

			GuiSinglelineTextBox::GuiSinglelineTextBox(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				textElement = GuiColorizedTextElement::Create();
				textElement->SetFont(GetDisplayFont());
				textElement->SetViewPosition(Point(-GuiSinglelineTextBox::TextMargin, -GuiSinglelineTextBox::TextMargin));

				textCompositionTable = new GuiTableComposition;
				textCompositionTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				textCompositionTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
				textCompositionTable->SetRowsAndColumns(3, 1);
				textCompositionTable->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
				textCompositionTable->SetRowOption(1, GuiCellOption::AbsoluteOption(0));
				textCompositionTable->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
				textCompositionTable->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
				containerComposition->AddChild(textCompositionTable);

				textComposition = new GuiCellComposition;
				textComposition->SetOwnedElement(textElement);
				textCompositionTable->AddChild(textComposition);
				textComposition->SetSite(1, 0, 1, 1);

				callback = new TextElementOperatorCallback(this);
				SetAcceptTabInput(true);
				SetFocusableComposition(boundsComposition);
				Install(textElement, textComposition, this, boundsComposition, focusableComposition);
				SetCallback(callback.Obj());

				boundsComposition->GetEventReceiver()->leftButtonDown.AttachMethod(this, &GuiSinglelineTextBox::OnBoundsMouseButtonDown);
				boundsComposition->GetEventReceiver()->middleButtonDown.AttachMethod(this, &GuiSinglelineTextBox::OnBoundsMouseButtonDown);
				boundsComposition->GetEventReceiver()->rightButtonDown.AttachMethod(this, &GuiSinglelineTextBox::OnBoundsMouseButtonDown);
			}

			GuiSinglelineTextBox::~GuiSinglelineTextBox()
			{
			}

			const WString& GuiSinglelineTextBox::GetText()
			{
				text = textElement->GetLines().GetText();
				return text;
			}

			void GuiSinglelineTextBox::SetText(const WString& value)
			{
				UnsafeSetText(value);
				textElement->SetCaretBegin(TextPos(0, 0));
				textElement->SetCaretEnd(TextPos(0, 0));
			}

			wchar_t GuiSinglelineTextBox::GetPasswordChar()
			{
				return textElement->GetPasswordChar();
			}

			void GuiSinglelineTextBox::SetPasswordChar(wchar_t value)
			{
				textElement->SetPasswordChar(value);
			}
		}
	}
}