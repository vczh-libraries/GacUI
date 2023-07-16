#include "GuiGraphicsEventReceiver.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
Event Receiver
***********************************************************************/

			GuiGraphicsEventReceiver::GuiGraphicsEventReceiver(GuiGraphicsComposition* _sender)
				:sender(_sender)
				,leftButtonDown(_sender)
				,leftButtonUp(_sender)
				,leftButtonDoubleClick(_sender)
				,middleButtonDown(_sender)
				,middleButtonUp(_sender)
				,middleButtonDoubleClick(_sender)
				,rightButtonDown(_sender)
				,rightButtonUp(_sender)
				,rightButtonDoubleClick(_sender)
				,horizontalWheel(_sender)
				,verticalWheel(_sender)
				,mouseMove(_sender)
				,mouseEnter(_sender)
				,mouseLeave(_sender)
				,previewKey(_sender)
				,keyDown(_sender)
				,keyUp(_sender)
				,previewCharInput(_sender)
				,charInput(_sender)
				,gotFocus(_sender)
				,lostFocus(_sender)
				,caretNotify(_sender)
				,clipboardNotify(_sender)
				,renderTargetChanged(_sender)
			{
			}

			GuiGraphicsEventReceiver::~GuiGraphicsEventReceiver()
			{
			}

			GuiGraphicsComposition* GuiGraphicsEventReceiver::GetAssociatedComposition()
			{
				return sender;
			}
		}
	}
}