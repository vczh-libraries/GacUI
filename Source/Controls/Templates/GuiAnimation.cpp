#include "GuiAnimation.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiWaitAnimation
***********************************************************************/

			GuiWaitAnimation::GuiWaitAnimation(vuint64_t _length)
			{

			}

			GuiWaitAnimation::~GuiWaitAnimation()
			{
			}

			void GuiWaitAnimation::Start()
			{
				startTime = DateTime::LocalTime();
				running = true;
			}

			void GuiWaitAnimation::Pause()
			{
				running = false;
				auto currentTime = DateTime::LocalTime();
				auto delta = currentTime.totalMilliseconds - startTime.totalMilliseconds;
				if (length > delta)
				{
					length -= delta;
				}
				else
				{
					length = 0;
				}
			}

			void GuiWaitAnimation::Resume()
			{
				startTime = DateTime::LocalTime();
				running = true;
			}

			void GuiWaitAnimation::Run()
			{
			}

			bool GuiWaitAnimation::GetStopped()
			{
				if (running)
				{
					auto currentTime = DateTime::LocalTime();
					auto delta = currentTime.totalMilliseconds - startTime.totalMilliseconds;
					return length <= delta;
				}
				else
				{
					return length == 0;
				}
			}
		}
	}
}