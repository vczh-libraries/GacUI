#include "GuiAnimation.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiTimedAnimation
***********************************************************************/

			class GuiTimedAnimation : public Object, public virtual IGuiAnimation
			{
			protected:
				DateTime						startTime;
				vuint64_t						time;
				bool							running = false;

			public:
				GuiTimedAnimation()
				{
				}

				~GuiTimedAnimation()
				{
				}

				void Start()override
				{
					startTime = DateTime::LocalTime();
					time = 0;
					running = true;
				}

				void Pause()override
				{
					time = GetTime();
					running = false;
				}

				void Resume()override
				{
					startTime = DateTime::LocalTime();
					running = true;
				}

				vuint64_t GetTime()
				{
					if (running)
					{
						return time + (DateTime::LocalTime().totalMilliseconds - startTime.totalMilliseconds);
					}
					else
					{
						return time;
					}
				}
			};

/***********************************************************************
GuiFiniteAnimation
***********************************************************************/

			class GuiFiniteAnimation : public GuiTimedAnimation
			{
			protected:
				vuint64_t						length = 0;
				Func<void(vuint64_t)>			run;

			public:
				GuiFiniteAnimation(const Func<void(vuint64_t)>& _run, vuint64_t _length)
					:run(_run)
					, length(_length)
				{
				}

				~GuiFiniteAnimation()
				{
				}

				void Run()override
				{
					auto currentTime = GetTime();
					if (currentTime < length && run)
					{
						run(currentTime);
					}
				}

				bool GetStopped()override
				{
					return GetTime() >= length;
				}
			};

/***********************************************************************
GuiInfiniteAnimation
***********************************************************************/

			class GuiInfiniteAnimation : public GuiTimedAnimation
			{
			protected:
				Func<void(vuint64_t)>			run;

			public:
				GuiInfiniteAnimation(const Func<void(vuint64_t)>& _run)
					:run(_run)
				{
				}

				~GuiInfiniteAnimation()
				{
				}

				void Run()override
				{
					if (run)
					{
						run(GetTime());
					}
				}

				bool GetStopped()override
				{
					return false;
				}
			};

/***********************************************************************
IGuiAnimation
***********************************************************************/

			Ptr<IGuiAnimation> IGuiAnimation::CreateAnimation(const Func<void(vuint64_t)>& run, vuint64_t milliseconds)
			{
				return new GuiFiniteAnimation(run, milliseconds);
			}

			Ptr<IGuiAnimation> IGuiAnimation::CreateAnimation(const Func<void(vuint64_t)>& run)
			{
				return new GuiInfiniteAnimation(run);
			}
		}
	}
}