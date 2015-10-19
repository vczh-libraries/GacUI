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
			F(presentation::compositions::KeyDirection)\
			F(presentation::compositions::IGuiAxis)\
			F(presentation::compositions::GuiDefaultAxis)\
			F(presentation::compositions::GuiAxis)\
			F(presentation::compositions::GuiStackComposition)\
			F(presentation::compositions::GuiStackComposition::Direction)\
			F(presentation::compositions::GuiStackItemComposition)\
			F(presentation::compositions::GuiCellOption)\
			F(presentation::compositions::GuiCellOption::ComposeType)\
			F(presentation::compositions::GuiTableComposition)\
			F(presentation::compositions::GuiCellComposition)\
			F(presentation::compositions::FlowAlignment)\
			F(presentation::compositions::GuiFlowComposition)\
			F(presentation::compositions::GuiFlowOption)\
			F(presentation::compositions::GuiFlowOption::BaselineType)\
			F(presentation::compositions::GuiFlowItemComposition)\
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

				class compositions_IGuiAxis : public ValueInterfaceRoot, public virtual IGuiAxis
				{
				public:
					compositions_IGuiAxis(Ptr<IValueInterfaceProxy> _proxy)
						:ValueInterfaceRoot(_proxy)
					{
					}

					static Ptr<IGuiAxis> Create(Ptr<IValueInterfaceProxy> proxy)
					{
						return new compositions_IGuiAxis(proxy);
					}

					Size RealSizeToVirtualSize(Size size)override
					{
						return INVOKEGET_INTERFACE_PROXY(RealSizeToVirtualSize, size);
					}

					Size VirtualSizeToRealSize(Size size)override
					{
						return INVOKEGET_INTERFACE_PROXY(VirtualSizeToRealSize, size);
					}

					Point RealPointToVirtualPoint(Size realFullSize, Point point)override
					{
						return INVOKEGET_INTERFACE_PROXY(RealPointToVirtualPoint, realFullSize, point);
					}

					Point VirtualPointToRealPoint(Size realFullSize, Point point)override
					{
						return INVOKEGET_INTERFACE_PROXY(VirtualPointToRealPoint, realFullSize, point);
					}

					Rect RealRectToVirtualRect(Size realFullSize, Rect rect)override
					{
						return INVOKEGET_INTERFACE_PROXY(RealRectToVirtualRect, realFullSize, rect);
					}

					Rect VirtualRectToRealRect(Size realFullSize, Rect rect)override
					{
						return INVOKEGET_INTERFACE_PROXY(VirtualRectToRealRect, realFullSize, rect);
					}

					Margin RealMarginToVirtualMargin(Margin margin)override
					{
						return INVOKEGET_INTERFACE_PROXY(RealMarginToVirtualMargin, margin);
					}

					Margin VirtualMarginToRealMargin(Margin margin)override
					{
						return INVOKEGET_INTERFACE_PROXY(VirtualMarginToRealMargin, margin);
					}

					KeyDirection RealKeyDirectionToVirtualKeyDirection(KeyDirection key)override
					{
						return INVOKEGET_INTERFACE_PROXY(RealKeyDirectionToVirtualKeyDirection, key);
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