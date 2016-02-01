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

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::compositions::IGuiGraphicsAnimation)
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
			END_INTERFACE_PROXY(presentation::compositions::IGuiGraphicsAnimation)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::compositions::IGuiAxis)
				presentation::Size RealSizeToVirtualSize(presentation::Size size)override
				{
					return INVOKEGET_INTERFACE_PROXY(RealSizeToVirtualSize, size);
				}

				presentation::Size VirtualSizeToRealSize(presentation::Size size)override
				{
					return INVOKEGET_INTERFACE_PROXY(VirtualSizeToRealSize, size);
				}

				presentation::Point RealPointToVirtualPoint(presentation::Size realFullSize, presentation::Point point)override
				{
					return INVOKEGET_INTERFACE_PROXY(RealPointToVirtualPoint, realFullSize, point);
				}

				presentation::Point VirtualPointToRealPoint(presentation::Size realFullSize, presentation::Point point)override
				{
					return INVOKEGET_INTERFACE_PROXY(VirtualPointToRealPoint, realFullSize, point);
				}

				presentation::Rect RealRectToVirtualRect(presentation::Size realFullSize, presentation::Rect rect)override
				{
					return INVOKEGET_INTERFACE_PROXY(RealRectToVirtualRect, realFullSize, rect);
				}

				presentation::Rect VirtualRectToRealRect(presentation::Size realFullSize, presentation::Rect rect)override
				{
					return INVOKEGET_INTERFACE_PROXY(VirtualRectToRealRect, realFullSize, rect);
				}

				presentation::Margin RealMarginToVirtualMargin(presentation::Margin margin)override
				{
					return INVOKEGET_INTERFACE_PROXY(RealMarginToVirtualMargin, margin);
				}

				presentation::Margin VirtualMarginToRealMargin(presentation::Margin margin)override
				{
					return INVOKEGET_INTERFACE_PROXY(VirtualMarginToRealMargin, margin);
				}

				presentation::compositions::KeyDirection RealKeyDirectionToVirtualKeyDirection(presentation::compositions::KeyDirection key)override
				{
					return INVOKEGET_INTERFACE_PROXY(RealKeyDirectionToVirtualKeyDirection, key);
				}
			END_INTERFACE_PROXY(presentation::compositions::IGuiAxis)

/***********************************************************************
Type Loader
***********************************************************************/

#endif

			extern bool						LoadGuiCompositionTypes();
		}
	}
}

#endif