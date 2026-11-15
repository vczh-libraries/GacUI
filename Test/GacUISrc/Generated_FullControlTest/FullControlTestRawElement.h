#ifndef VCZH_DEMO_FULLCONTROLTESTRAWELEMENT
#define VCZH_DEMO_FULLCONTROLTESTRAWELEMENT

#include "../../../Source/GacUI.h"

namespace demo
{
	// Keep the returned Touching attachment alive while drawing is needed. Destruction detaches its handlers.
	// Data: centerX,centerY,axisX,axisY,count followed by count triples of pointId,x,y.
	// The axis is a complex rotation/uniform-scale transform of a 160 by 100 rectangle.
	extern vl::Ptr<vl::Object> AttachFullControlTestTouchingElement(vl::Ptr<vl::presentation::elements::GuiRawElementBase> element);
}

#endif
