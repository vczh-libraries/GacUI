#include "GuiAnimation.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace reflection::description;

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

/***********************************************************************
IGuiAnimationCoroutine
***********************************************************************/

			class GuiCoroutineAnimation : public Object, public virtual IGuiAnimationCoroutine::IImpl
			{
			protected:
				IGuiAnimationCoroutine::Creator				creator;
				Ptr<ICoroutine>								coroutine;

				Ptr<IGuiAnimation>							waitingAnimation;
				vint										waitingGroup = -1;
				Group<vint, Ptr<IGuiAnimation>>				groupAnimations;

			public:
				GuiCoroutineAnimation(const IGuiAnimationCoroutine::Creator& _creator)
					:creator(_creator)
				{
				}

				~GuiCoroutineAnimation()
				{
				}

				void OnPlayAndWait(Ptr<IGuiAnimation> animation)override
				{
					CHECK_ERROR(!waitingAnimation && waitingGroup == -1, L"GuiCoroutineAnimation::OnPlayAndWait(Ptr<IGuiAnimation>)#Cannot be called when an animation or a group has already been waiting for.");
					waitingAnimation = animation;
					waitingAnimation->Start();
				}

				void OnPlayInGroup(Ptr<IGuiAnimation> animation, vint groupId)override
				{
					groupAnimations.Add(groupId, animation);
					animation->Start();
				}

				void OnWaitForGroup(vint groupId)override
				{
					CHECK_ERROR(!waitingAnimation && waitingGroup == -1, L"GuiCoroutineAnimation::OnWaitForGroup(vint)#Cannot be called when an animation or a group has already been waiting for.");
					if (groupAnimations.Keys().Contains(groupId))
					{
						waitingGroup = groupId;
					}
				}

				void Start()override
				{
					CHECK_ERROR(!coroutine, L"GuiCoroutineAnimation::Start()#Cannot be called more than once.");
					coroutine = creator(this);
				}

				void Pause()override
				{
					if (waitingAnimation)
					{
						waitingAnimation->Pause();
					}
					for (vint i = 0; i < groupAnimations.Count(); i++)
					{
						FOREACH(Ptr<IGuiAnimation>, animation, groupAnimations.GetByIndex(i))
						{
							animation->Pause();
						}
					}
				}

				void Resume()override
				{
					if (waitingAnimation)
					{
						waitingAnimation->Resume();
					}
					for (vint i = 0; i < groupAnimations.Count(); i++)
					{
						FOREACH(Ptr<IGuiAnimation>, animation, groupAnimations.GetByIndex(i))
						{
							animation->Resume();
						}
					}
				}

				void Run()override
				{
					CHECK_ERROR(coroutine, L"GuiCoroutineAnimation::Run()#Cannot be called before calling Start.");

					if (waitingAnimation)
					{
						waitingAnimation->Run();
						if (waitingAnimation->GetStopped())
						{
							waitingAnimation = nullptr;
						}
					}

					for (vint i = groupAnimations.Count() - 1; i >= 0; i--)
					{
						auto& animations = groupAnimations.GetByIndex(i);
						for (vint j = animations.Count() - 1; j >= 0; j--)
						{
							auto animation = animations[j];
							animation->Run();
							if (animation->GetStopped())
							{
								groupAnimations.Remove(i, animation.Obj());
							}
						}
					}

					if (waitingGroup != -1 && !groupAnimations.Keys().Contains(waitingGroup))
					{
						waitingGroup = -1;
					}

					if (coroutine->GetStatus() == CoroutineStatus::Waiting)
					{
						if (waitingAnimation || waitingGroup != -1)
						{
							return;
						}
						coroutine->Resume(true, nullptr);
					}
				}

				bool GetStopped()override
				{
					if (!coroutine) return false;
					if (coroutine->GetStatus() != CoroutineStatus::Stopped) return false;
					if (waitingAnimation || groupAnimations.Count() > 0) return false;
					return true;
				}
			};

			void IGuiAnimationCoroutine::WaitAndPause(IImpl* impl, vuint64_t milliseconds)
			{
				return PlayAndWaitAndPause(impl, IGuiAnimation::CreateAnimation({}, milliseconds));
			}

			void IGuiAnimationCoroutine::PlayAndWaitAndPause(IImpl* impl, Ptr<IGuiAnimation> animation)
			{
				impl->OnPlayAndWait(animation);
			}

			void IGuiAnimationCoroutine::PlayInGroupAndPause(IImpl* impl, Ptr<IGuiAnimation> animation, vint groupId)
			{
				impl->OnPlayInGroup(animation, groupId);
			}

			void IGuiAnimationCoroutine::WaitForGroupAndPause(IImpl* impl, vint groupId)
			{
				impl->OnWaitForGroup(groupId);
			}

			void IGuiAnimationCoroutine::ReturnAndExit(IImpl* impl)
			{
			}

			Ptr<IGuiAnimation> IGuiAnimationCoroutine::Create(const Creator& creator)
			{
				return new GuiCoroutineAnimation(creator);
			}
		}
	}
}