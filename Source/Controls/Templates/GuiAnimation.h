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
			class IGuiAnimationCoroutine : public Object, public Description<IGuiAnimationCoroutine>
			{
			public:
				class IImpl : public virtual IGuiAnimation, public Description<IImpl>
				{
				public:
					virtual void			OnPlayAndWait(Ptr<IGuiAnimation> animation) = 0;
					virtual void			OnPlayInGroup(Ptr<IGuiAnimation> animation, vint groupId) = 0;
					virtual void			OnWaitForGroup(vint groupId) = 0;
				};

				typedef Func<Ptr<description::ICoroutine>(IImpl*)>	Creator;

				static void					WaitAndPause(IImpl* impl, vuint64_t milliseconds);
				static void					PlayAndWaitAndPause(IImpl* impl, Ptr<IGuiAnimation> animation);
				static void					PlayInGroupAndPause(IImpl* impl, Ptr<IGuiAnimation> animation, vint groupId);
				static void					WaitForGroupAndPause(IImpl* impl, vint groupId);
				static void					ReturnAndExit(IImpl* impl);
				static Ptr<IGuiAnimation>	Create(const Creator& creator);
			};
		}
	}
}

#endif