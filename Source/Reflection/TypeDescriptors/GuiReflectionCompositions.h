/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Compositions

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONCOMPOSITIONS
#define VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONCOMPOSITIONS

#include "GuiReflectionBasic.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
Type List
***********************************************************************/

#define GUIREFLECTIONCOMPOSITION_TYPELIST(F)\
			F(presentation::compositions::GuiStackComposition)\
			F(presentation::compositions::GuiStackComposition::Direction)\
			F(presentation::compositions::GuiStackItemComposition)\
			F(presentation::compositions::GuiCellOption)\
			F(presentation::compositions::GuiCellOption::ComposeType)\
			F(presentation::compositions::GuiTableComposition)\
			F(presentation::compositions::GuiCellComposition)\
			F(presentation::compositions::GuiSideAlignedComposition)\
			F(presentation::compositions::GuiSideAlignedComposition::Direction)\
			F(presentation::compositions::GuiPartialViewComposition)\
			F(presentation::compositions::GuiSharedSizeItemComposition)\
			F(presentation::compositions::GuiSharedSizeRootComposition)\
			F(presentation::compositions::IGuiGraphicsAnimation)\
			F(presentation::compositions::GuiGraphicsAnimationManager)\
			F(presentation::compositions::IGuiShortcutKeyItem)\
			F(presentation::compositions::IGuiShortcutKeyManager)\
			F(presentation::compositions::GuiShortcutKeyManager)\
			F(presentation::compositions::IGuiAltAction)\
			F(presentation::compositions::IGuiAltActionContainer)\
			F(presentation::compositions::IGuiAltActionHost)\

			GUIREFLECTIONCOMPOSITION_TYPELIST(DECL_TYPE_INFO)

/***********************************************************************
Interface Proxy
***********************************************************************/

#pragma warning(push)
#pragma warning(disable:4250)
			namespace interface_proxy
			{
				class composition_IGuiGraphicsAnimation : public ValueInterfaceRoot, public virtual IGuiGraphicsAnimation
				{
				public:
					composition_IGuiGraphicsAnimation(Ptr<IValueInterfaceProxy> _proxy)
						:ValueInterfaceRoot(_proxy)
					{
					}

					static Ptr<IGuiGraphicsAnimation> Create(Ptr<IValueInterfaceProxy> proxy)
					{
						return new composition_IGuiGraphicsAnimation(proxy);
					}

					vint GetTotalLength()override
					{
						return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetTotalLength);
					}

					vint GetCurrentPosition()override
					{
						return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetCurrentPosition);
					}

					void Play(vint currentPosition, vint totalLength)override
					{
						INVOKE_INTERFACE_PROXY(Play, currentPosition, totalLength);
					}

					void Stop()override
					{
						INVOKE_INTERFACE_PROXY_NOPARAMS(Stop);
					}
				};
			}
#pragma warning(pop)

/***********************************************************************
Type Loader
***********************************************************************/

#endif

			extern bool						LoadGuiCompositionTypes();
		}
	}
}

#endif