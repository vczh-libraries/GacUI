#include "GuiGraphicsEventReceiver.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
Point Event Arguments
***********************************************************************/

			GuiPointEventArgs::GuiPointEventArgs(const collections::List<Nullable<Point>>& _points)
				:points(_points)
			{
			}

			GuiPointEventArgs::GuiPointEventArgs(GuiGraphicsComposition* composition, const collections::List<Nullable<Point>>& _points)
				:GuiEventArgs(composition)
				,points(_points)
			{
			}

			const collections::List<Nullable<Point>>& GuiPointEventArgs::GetPoints() const
			{
				return points;
			}

			GuiPointSequenceStartedEventArgs::GuiPointSequenceStartedEventArgs(const collections::List<Nullable<Point>>& _points)
				:GuiPointEventArgs(GuiPointEventsType::Started, _points)
			{
			}

			GuiPointSequenceStartedEventArgs::GuiPointSequenceStartedEventArgs(GuiGraphicsComposition* composition, const collections::List<Nullable<Point>>& _points)
				:GuiPointEventArgs(composition, GuiPointEventsType::Started, _points)
			{
			}

			GuiPointSequenceEndedEventArgs::GuiPointSequenceEndedEventArgs(const collections::List<Nullable<Point>>& _points)
				:points(_points)
			{
			}

			GuiPointSequenceEndedEventArgs::GuiPointSequenceEndedEventArgs(GuiGraphicsComposition* composition, const collections::List<Nullable<Point>>& _points)
				:GuiEventArgs(composition)
				,points(_points)
			{
			}

			const collections::List<Nullable<Point>>& GuiPointSequenceEndedEventArgs::GetPoints() const
			{
				return points;
			}

/***********************************************************************
Event Receiver
***********************************************************************/

			GuiGraphicsEventReceiver::GuiGraphicsEventReceiver(GuiGraphicsComposition* _sender)
				:sender(_sender)
				,mouseDown(_sender)
				,mouseUp(_sender)
				,mouseDoubleClick(_sender)
				,horizontalWheel(_sender)
				,verticalWheel(_sender)
				,mouseMove(_sender)
				,mouseEnter(_sender)
				,mouseLeave(_sender)
				,pointSequenceStarted(_sender)
				,pointDown(_sender)
				,pointUpdate(_sender)
				,pointUp(_sender)
				,pointEnter(_sender)
				,pointLeave(_sender)
				,pointSequenceEnded(_sender)
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
