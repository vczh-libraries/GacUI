#include "GuiScrollControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace collections;
			using namespace reflection::description;

/***********************************************************************
GuiScroll::CommandExecutor
***********************************************************************/

			GuiScroll::CommandExecutor::CommandExecutor(GuiScroll* _scroll)
				:scroll(_scroll)
			{
			}

			GuiScroll::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiScroll::CommandExecutor::SmallDecrease()
			{
				scroll->SetPosition(scroll->GetPosition()-scroll->GetSmallMove());
			}

			void GuiScroll::CommandExecutor::SmallIncrease()
			{
				scroll->SetPosition(scroll->GetPosition()+scroll->GetSmallMove());
			}

			void GuiScroll::CommandExecutor::BigDecrease()
			{
				scroll->SetPosition(scroll->GetPosition()-scroll->GetBigMove());
			}

			void GuiScroll::CommandExecutor::BigIncrease()
			{
				scroll->SetPosition(scroll->GetPosition()+scroll->GetBigMove());
			}
			
			void GuiScroll::CommandExecutor::SetTotalSize(vint value)
			{
				scroll->SetTotalSize(value);
			}

			void GuiScroll::CommandExecutor::SetPageSize(vint value)
			{
				scroll->SetPageSize(value);
			}

			void GuiScroll::CommandExecutor::SetPosition(vint value)
			{
				scroll->SetPosition(value);
			}

/***********************************************************************
GuiScroll
***********************************************************************/

			void GuiScroll::OnActiveAlt()
			{
				if (autoFocus)
				{
					GuiControl::OnActiveAlt();
				}
			}

			bool GuiScroll::IsTabAvailable()
			{
				return autoFocus && GuiControl::IsTabAvailable();
			}

			void GuiScroll::OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments)
			{
				if (arguments.eventSource == focusableComposition)
				{
					switch (arguments.code)
					{
					case VKEY::KEY_HOME:
						SetPosition(GetMinPosition());
						arguments.handled = true;
						break;
					case VKEY::KEY_END:
						SetPosition(GetMaxPosition());
						arguments.handled = true;
						break;
					case VKEY::KEY_PRIOR:
						commandExecutor->BigDecrease();
						arguments.handled = true;
						break;
					case VKEY::KEY_NEXT:
						commandExecutor->BigIncrease();
						arguments.handled = true;
						break;
					case VKEY::KEY_LEFT:
					case VKEY::KEY_UP:
						commandExecutor->SmallDecrease();
						arguments.handled = true;
						break;
					case VKEY::KEY_RIGHT:
					case VKEY::KEY_DOWN:
						commandExecutor->SmallIncrease();
						arguments.handled = true;
						break;
					default:;
					}
				}
			}

			void GuiScroll::OnMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if (autoFocus)
				{
					SetFocused();
				}
			}

			void GuiScroll::BeforeControlTemplateUninstalled_()
			{
				auto ct = TypedControlTemplateObject(false);
				if (!ct) return;

				ct->SetCommands(nullptr);
			}

			void GuiScroll::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = TypedControlTemplateObject(true);
				ct->SetCommands(commandExecutor.Obj());
				ct->SetPageSize(pageSize);
				ct->SetTotalSize(totalSize);
				ct->SetPosition(position);
			}

			GuiScroll::GuiScroll(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				if (themeName == theme::ThemeName::ProgressBar)
				{
					autoFocus = false;
				}
				else
				{
					SetFocusableComposition(boundsComposition);
				}

				TotalSizeChanged.SetAssociatedComposition(boundsComposition);
				PageSizeChanged.SetAssociatedComposition(boundsComposition);
				PositionChanged.SetAssociatedComposition(boundsComposition);
				SmallMoveChanged.SetAssociatedComposition(boundsComposition);
				BigMoveChanged.SetAssociatedComposition(boundsComposition);

				commandExecutor = Ptr(new CommandExecutor(this));
				boundsComposition->GetEventReceiver()->keyDown.AttachMethod(this, &GuiScroll::OnKeyDown);
				boundsComposition->GetEventReceiver()->leftButtonDown.AttachMethod(this, &GuiScroll::OnMouseDown);
				boundsComposition->GetEventReceiver()->rightButtonDown.AttachMethod(this, &GuiScroll::OnMouseDown);
			}

			GuiScroll::~GuiScroll()
			{
			}

			vint GuiScroll::GetTotalSize()
			{
				return totalSize;
			}

			void GuiScroll::SetTotalSize(vint value)
			{
				if(totalSize!=value && 0<value)
				{
					totalSize=value;
					if(pageSize>totalSize)
					{
						SetPageSize(totalSize);
					}
					if(position>GetMaxPosition())
					{
						SetPosition(GetMaxPosition());
					}
					TypedControlTemplateObject(true)->SetTotalSize(totalSize);
					TotalSizeChanged.Execute(GetNotifyEventArguments());
				}
			}

			vint GuiScroll::GetPageSize()
			{
				return pageSize;
			}

			void GuiScroll::SetPageSize(vint value)
			{
				if(pageSize!=value && 0<=value && value<=totalSize)
				{
					pageSize=value;
					if(position>GetMaxPosition())
					{
						SetPosition(GetMaxPosition());
					}
					TypedControlTemplateObject(true)->SetPageSize(pageSize);
					PageSizeChanged.Execute(GetNotifyEventArguments());
				}
			}

			vint GuiScroll::GetPosition()
			{
				return position;
			}

			void GuiScroll::SetPosition(vint value)
			{
				vint min=GetMinPosition();
				vint max=GetMaxPosition();
				vint newPosition=
					value<min?min:
					value>max?max:
					value;
				if(position!=newPosition)
				{
					position=newPosition;
					TypedControlTemplateObject(true)->SetPosition(position);
					PositionChanged.Execute(GetNotifyEventArguments());
				}
			}

			vint GuiScroll::GetSmallMove()
			{
				return smallMove;
			}

			void GuiScroll::SetSmallMove(vint value)
			{
				if(value>0 && smallMove!=value)
				{
					smallMove=value;
					SmallMoveChanged.Execute(GetNotifyEventArguments());
				}
			}

			vint GuiScroll::GetBigMove()
			{
				return bigMove;
			}

			void GuiScroll::SetBigMove(vint value)
			{
				if(value>0 && bigMove!=value)
				{
					bigMove=value;
					BigMoveChanged.Execute(GetNotifyEventArguments());
				}
			}

			vint GuiScroll::GetMinPosition()
			{
				return 0;
			}

			vint GuiScroll::GetMaxPosition()
			{
				return totalSize-pageSize;
			}

			bool GuiScroll::GetAutoFocus()
			{
				return autoFocus;
			}

			void GuiScroll::SetAutoFocus(bool value)
			{
				autoFocus = value;
			}
		}
	}
}