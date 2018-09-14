/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSAXIS
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSAXIS

#include "../GuiTypes.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
Axis Interface
***********************************************************************/

			/// <summary>Represents the four directions that is accessable by keyboard.</summary>
			enum class KeyDirection
			{
				/// <summary>The up direction.</summary>
				Up,
				/// <summary>The down direction.</summary>
				Down,
				/// <summary>The left direction.</summary>
				Left,
				/// <summary>The right direction.</summary>
				Right,
				/// <summary>The home direction.</summary>
				Home,
				/// <summary>The end direction.</summary>
				End,
				/// <summary>The page up direction.</summary>
				PageUp,
				/// <summary>The page down direction.</summary>
				PageDown,
				/// <summary>The page left direction.</summary>
				PageLeft,
				/// <summary>The page right direction.</summary>
				PageRight,
			};
				
			/// <summary>Item coordinate transformer for a <see cref="GuiListControl"/>. In all functions in this interface, real coordinate is in the list control's container space, virtual coordinate is in a space that the transformer created.</summary>
			class IGuiAxis : public virtual IDescriptable, public Description<IGuiAxis>
			{
			public:
				/// <summary>Translate real size to virtual size.</summary>
				/// <returns>The virtual size.</returns>
				/// <param name="size">The real size.</param>
				virtual Size								RealSizeToVirtualSize(Size size)=0;
				/// <summary>Translate virtual size to real size.</summary>
				/// <returns>The real size.</returns>
				/// <param name="size">The virtual size.</param>
				virtual Size								VirtualSizeToRealSize(Size size)=0;
				/// <summary>Translate real point to virtual point.</summary>
				/// <returns>The virtual point.</returns>
				/// <param name="realFullSize">The real full size.</param>
				/// <param name="point">The real point.</param>
				virtual Point								RealPointToVirtualPoint(Size realFullSize, Point point)=0;
				/// <summary>Translate virtual point to real point.</summary>
				/// <returns>The real point.</returns>
				/// <param name="realFullSize">The real full size.</param>
				/// <param name="point">The virtual point.</param>
				virtual Point								VirtualPointToRealPoint(Size realFullSize, Point point)=0;
				/// <summary>Translate real bounds to virtual bounds.</summary>
				/// <returns>The virtual bounds.</returns>
				/// <param name="realFullSize">The real full size.</param>
				/// <param name="rect">The real bounds.</param>
				virtual Rect								RealRectToVirtualRect(Size realFullSize, Rect rect)=0;
				/// <summary>Translate virtual bounds to real bounds.</summary>
				/// <returns>The real bounds.</returns>
				/// <param name="realFullSize">The real full size.</param>
				/// <param name="rect">The virtual bounds.</param>
				virtual Rect								VirtualRectToRealRect(Size realFullSize, Rect rect)=0;
				/// <summary>Translate real margin to margin size.</summary>
				/// <returns>The virtual margin.</returns>
				/// <param name="margin">The real margin.</param>
				virtual Margin								RealMarginToVirtualMargin(Margin margin)=0;
				/// <summary>Translate virtual margin to margin size.</summary>
				/// <returns>The real margin.</returns>
				/// <param name="margin">The virtual margin.</param>
				virtual Margin								VirtualMarginToRealMargin(Margin margin)=0;
				/// <summary>Translate real key direction to virtual key direction.</summary>
				/// <returns>The virtual key direction.</returns>
				/// <param name="key">The real key direction.</param>
				virtual KeyDirection						RealKeyDirectionToVirtualKeyDirection(KeyDirection key)=0;
			};

/***********************************************************************
Axis Implementation
***********************************************************************/

			/// <summary>Default item coordinate transformer. This transformer doesn't transform any coordinate.</summary>
			class GuiDefaultAxis : public Object, virtual public IGuiAxis, public Description<GuiDefaultAxis>
			{
			public:
				/// <summary>Create the transformer.</summary>
				GuiDefaultAxis();
				~GuiDefaultAxis();

				Size										RealSizeToVirtualSize(Size size)override;
				Size										VirtualSizeToRealSize(Size size)override;
				Point										RealPointToVirtualPoint(Size realFullSize, Point point)override;
				Point										VirtualPointToRealPoint(Size realFullSize, Point point)override;
				Rect										RealRectToVirtualRect(Size realFullSize, Rect rect)override;
				Rect										VirtualRectToRealRect(Size realFullSize, Rect rect)override;
				Margin										RealMarginToVirtualMargin(Margin margin)override;
				Margin										VirtualMarginToRealMargin(Margin margin)override;
				KeyDirection								RealKeyDirectionToVirtualKeyDirection(KeyDirection key)override;
			};
				
			/// <summary>Axis aligned item coordinate transformer. This transformer transforms coordinates by changing the axis direction.</summary>
			class GuiAxis : public Object, virtual public IGuiAxis, public Description<GuiAxis>
			{
			protected:
				AxisDirection								axisDirection;

			public:
				/// <summary>Create the transformer with a specified axis direction.</summary>
				/// <param name="_axisDirection">The specified axis direction.</param>
				GuiAxis(AxisDirection _axisDirection);
				~GuiAxis();

				/// <summary>Get the specified axis direction.</summary>
				/// <returns>The specified axis direction.</returns>
				AxisDirection								GetDirection();
				Size										RealSizeToVirtualSize(Size size)override;
				Size										VirtualSizeToRealSize(Size size)override;
				Point										RealPointToVirtualPoint(Size realFullSize, Point point)override;
				Point										VirtualPointToRealPoint(Size realFullSize, Point point)override;
				Rect										RealRectToVirtualRect(Size realFullSize, Rect rect)override;
				Rect										VirtualRectToRealRect(Size realFullSize, Rect rect)override;
				Margin										RealMarginToVirtualMargin(Margin margin)override;
				Margin										VirtualMarginToRealMargin(Margin margin)override;
				KeyDirection								RealKeyDirectionToVirtualKeyDirection(KeyDirection key)override;
			};
		}
	}
}

#endif