#ifndef VCZH_DEMO_FULLCONTROLTESTRAWCLOCKINTERPRETOR
#define VCZH_DEMO_FULLCONTROLTESTRAWCLOCKINTERPRETOR

#include "../../../Source/GacUI.h"

namespace demo
{
	// Data is HH:mm:ss (24-hour time); painting never reads the local clock.
	// Concrete interpretors borrow their element and own their event handlers.
	class RawClockInterpretorBase : public vl::Object, public vl::presentation::elements::IGuiGraphicsRawInterpretor
	{
	protected:
		vl::presentation::elements::GuiRawElementBase*
										element = nullptr;

		bool							ReadTime(vl::vint& hour, vl::vint& minute, vl::vint& second);
		void							DrawClock(vl::presentation::Rect bounds);
		virtual void					DrawLine(vl::vint color, double x1, double y1, double x2, double y2) = 0;
		virtual void					DrawEllipse(double cx, double cy, double radius) = 0;
	};
}

#endif
