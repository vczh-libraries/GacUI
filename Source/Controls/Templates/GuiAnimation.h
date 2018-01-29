/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUIANIMATION
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUIANIMATION

#include "GuiControlShared.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiWaitAnimation abstract : public virtual IGuiAnimation, public Description<GuiWaitAnimation>
			{
			protected:
				DateTime						startTime;
				vuint64_t						length = 0;
				bool							running = false;

			public:
				GuiWaitAnimation(vuint64_t _length);
				~GuiWaitAnimation();

				void							Start()override;
				void							Pause()override;
				void							Resume()override;
				void							Run()override;
				bool							GetStopped()override;
			};
		}
	}
}

#endif