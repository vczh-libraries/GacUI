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
				};

				typedef Func<Ptr<IGuiAnimation>(IImpl*)>	Creator;

				static void					Wait(IImpl* impl, vuint64_t milliseconds);
				static void					PlayAndWait(IImpl* impl, Ptr<IGuiAnimation> animation);
				static void					PlayInGroup(IImpl* impl, Ptr<IGuiAnimation> animation, vint groupId);
				static void					WaitForGroup(IImpl* impl, vint groupId);
				static Ptr<IGuiAnimation>	Create(const Creator& creator);
			};
		}
	}
}

#endif