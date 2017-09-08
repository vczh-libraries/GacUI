#include "GuiTextControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace elements::text;
			using namespace compositions;

/***********************************************************************
GuiMultilineTextBox::DefaultTextElementOperatorCallback
***********************************************************************/

			GuiMultilineTextBox::TextElementOperatorCallback::TextElementOperatorCallback(GuiMultilineTextBox* _textControl)
				:GuiTextBoxCommonInterface::DefaultCallback(
					dynamic_cast<IStyleController*>(_textControl->GetStyleController())->GetTextElement(),
					dynamic_cast<IStyleController*>(_textControl->GetStyleController())->GetTextComposition()
					)
				,textControl(_textControl)
				,textController(dynamic_cast<IStyleController*>(_textControl->GetStyleController()))
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
				Point oldPoint(textControl->GetHorizontalScroll()->GetPosition(), textControl->GetVerticalScroll()->GetPosition());
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
				textControl->GetHorizontalScroll()->SetPosition(point.x+marginX);
				textControl->GetVerticalScroll()->SetPosition(point.y+marginY);
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

			void GuiMultilineTextBox::CalculateViewAndSetScroll()
			{
				CalculateView();
				vint smallMove=styleController->GetTextElement()->GetLines().GetRowHeight();
				vint bigMove=smallMove*5;
				styleController->GetHorizontalScroll()->SetSmallMove(smallMove);
				styleController->GetHorizontalScroll()->SetBigMove(bigMove);
				styleController->GetVerticalScroll()->SetSmallMove(smallMove);
				styleController->GetVerticalScroll()->SetBigMove(bigMove);
			}

			void GuiMultilineTextBox::OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)
			{
				CalculateViewAndSetScroll();
				GuiScrollView::OnRenderTargetChanged(renderTarget);
			}

			Size GuiMultilineTextBox::QueryFullSize()
			{
				text::TextLines& lines=styleController->GetTextElement()->GetLines();
				return Size(lines.GetMaxWidth()+TextMargin*2, lines.GetMaxHeight()+TextMargin*2);
			}

			void GuiMultilineTextBox::UpdateView(Rect viewBounds)
			{
				styleController->GetTextElement()->SetViewPosition(viewBounds.LeftTop()-Size(TextMargin, TextMargin));
			}

			void GuiMultilineTextBox::OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					boundsComposition->GetRelatedGraphicsHost()->SetFocus(boundsComposition);
				}
			}

			GuiMultilineTextBox::GuiMultilineTextBox(IStyleController* _styleController)
				:GuiScrollView(_styleController)
				, styleController(_styleController)
			{
				callback = new TextElementOperatorCallback(this);
				commandExecutor = new CommandExecutor(this);

				styleController->SetCommandExecutor(commandExecutor.Obj());
				SetFocusableComposition(boundsComposition);
				Install(styleController->GetTextElement(), styleController->GetTextComposition(), this);
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
				text = styleController->GetEditingText();
				return text;
			}

			void GuiMultilineTextBox::SetText(const WString& value)
			{
				GuiScrollView::SetText(value);
				CalculateView();
			}

			void GuiMultilineTextBox::SetFont(const FontProperties& value)
			{
				GuiScrollView::SetFont(value);
				CalculateViewAndSetScroll();
			}

/***********************************************************************
GuiSinglelineTextBox::DefaultTextElementOperatorCallback
***********************************************************************/

			GuiSinglelineTextBox::TextElementOperatorCallback::TextElementOperatorCallback(GuiSinglelineTextBox* _textControl)
				:GuiTextBoxCommonInterface::DefaultCallback(
					dynamic_cast<IStyleController*>(_textControl->GetStyleController())->GetTextElement(),
					dynamic_cast<IStyleController*>(_textControl->GetStyleController())->GetTextComposition()
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
GuiSinglelineTextBox::CommandExecutor
***********************************************************************/

			GuiSinglelineTextBox::CommandExecutor::CommandExecutor(GuiSinglelineTextBox* _textBox)
				:textBox(_textBox)
			{
			}

			GuiSinglelineTextBox::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiSinglelineTextBox::CommandExecutor::UnsafeSetText(const WString& value)
			{
				textBox->UnsafeSetText(value);
			}

/***********************************************************************
GuiSinglelineTextBox
***********************************************************************/

			void GuiSinglelineTextBox::OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)
			{
				GuiControl::OnRenderTargetChanged(renderTarget);
				styleController->RearrangeTextElement();
			}

			void GuiSinglelineTextBox::OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					boundsComposition->GetRelatedGraphicsHost()->SetFocus(boundsComposition);
				}
			}

			GuiSinglelineTextBox::GuiSinglelineTextBox(GuiSinglelineTextBox::IStyleController* _styleController)
				:GuiControl(_styleController)
				, styleController(_styleController)
			{
				callback = new TextElementOperatorCallback(this);
				commandExecutor = new CommandExecutor(this);

				styleController->SetCommandExecutor(commandExecutor.Obj());
				SetFocusableComposition(boundsComposition);
				Install(styleController->GetTextElement(), styleController->GetTextComposition(), this);
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
				text = styleController->GetEditingText();
				return text;
			}

			void GuiSinglelineTextBox::SetFont(const FontProperties& value)
			{
				GuiControl::SetFont(value);
				styleController->RearrangeTextElement();
			}

			wchar_t GuiSinglelineTextBox::GetPasswordChar()
			{
				return styleController->GetTextElement()->GetPasswordChar();
			}

			void GuiSinglelineTextBox::SetPasswordChar(wchar_t value)
			{
				styleController->GetTextElement()->SetPasswordChar(value);
			}
		}
	}
}