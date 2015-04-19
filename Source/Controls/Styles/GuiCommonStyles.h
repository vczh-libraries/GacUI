/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Common Style Helpers

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUICOMMONSTYLES
#define VCZH_PRESENTATION_CONTROLS_GUICOMMONSTYLES

#include "GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace common_styles
		{

/***********************************************************************
CommonScrollStyle
***********************************************************************/

			/// <summary>A general implementation for <see cref="controls::GuiScroll::IStyleController"/> to make a scroll control.</summary>
			class CommonScrollStyle : public Object, public virtual controls::GuiScroll::IStyleController, public Description<CommonScrollStyle>
			{
			public:
				/// <summary>Scroll direction.</summary>
				enum Direction
				{
					/// <summary>Horizontal scroll.</summary>
					Horizontal,
					/// <summary>Vertical scroll.</summary>
					Vertical,
				};
			protected:
				Direction											direction;
				controls::GuiScroll::ICommandExecutor*				commandExecutor;
				controls::GuiButton*								decreaseButton;
				controls::GuiButton*								increaseButton;
				controls::GuiButton*								handleButton;
				compositions::GuiPartialViewComposition*			handleComposition;
				compositions::GuiBoundsComposition*					boundsComposition;
				compositions::GuiBoundsComposition*					containerComposition;

				vint												totalSize;
				vint												pageSize;
				vint												position;
				Point												draggingStartLocation;
				bool												draggingHandle;

				void												UpdateHandle();
				void												OnDecreaseButtonClicked(compositions::GuiGraphicsComposition* sender,compositions::GuiEventArgs& arguments);
				void												OnIncreaseButtonClicked(compositions::GuiGraphicsComposition* sender,compositions::GuiEventArgs& arguments);
				void												OnHandleMouseDown(compositions::GuiGraphicsComposition* sender,compositions::GuiMouseEventArgs& arguments);
				void												OnHandleMouseMove(compositions::GuiGraphicsComposition* sender,compositions::GuiMouseEventArgs& arguments);
				void												OnHandleMouseUp(compositions::GuiGraphicsComposition* sender,compositions::GuiMouseEventArgs& arguments);;
				void												OnBigMoveMouseDown(compositions::GuiGraphicsComposition* sender,compositions::GuiMouseEventArgs& arguments);

				/// <summary>Callback to create a style controller for the decrement button.</summary>
				/// <returns>The created style controller.</returns>
				/// <param name="direction">The direction of the scroll.</param>
				virtual controls::GuiButton::IStyleController*		CreateDecreaseButtonStyle(Direction direction)=0;
				/// <summary>Callback to create a style controller for the increment button.</summary>
				/// <returns>The created style controller.</returns>
				/// <param name="direction">The direction of the scroll.</param>
				virtual controls::GuiButton::IStyleController*		CreateIncreaseButtonStyle(Direction direction)=0;
				/// <summary>Callback to create a style controller for the handle button.</summary>
				/// <returns>The created style controller.</returns>
				/// <param name="direction">The direction of the scroll.</param>
				virtual controls::GuiButton::IStyleController*		CreateHandleButtonStyle(Direction direction)=0;
				/// <summary>Install necessary compositions and elements to the background.</summary>
				/// <returns>The created container composition.</returns>
				/// <param name="boundsComposition">The background composition.</param>
				/// <param name="direction">The direction of the scroll.</param>
				virtual compositions::GuiBoundsComposition*			InstallBackground(compositions::GuiBoundsComposition* boundsComposition, Direction direction)=0;
				/// <summary>Build the style. This function is supposed to be called in the contructor of the sub class.</summary>
				/// <param name="defaultSize">The size of the increment button and decrement button.</param>
				/// <param name="arrowSize">The size of the arrows in the increment button and decrement button.</param>
				void												BuildStyle(vint defaultSize, vint arrowSize);
			public:
				/// <summary>Create the style controller using a specified direction.</summary>
				/// <param name="_direction">The specified direction</param>
				CommonScrollStyle(Direction _direction);
				~CommonScrollStyle();

				compositions::GuiBoundsComposition*					GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*				GetContainerComposition()override;
				void												SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void												SetText(const WString& value)override;
				void												SetFont(const FontProperties& value)override;
				void												SetVisuallyEnabled(bool value)override;
				void												SetCommandExecutor(controls::GuiScroll::ICommandExecutor* value)override;
				void												SetTotalSize(vint value)override;
				void												SetPageSize(vint value)override;
				void												SetPosition(vint value)override;
			};

/***********************************************************************
CommonTrackStyle
***********************************************************************/
			
			/// <summary>A general implementation for <see cref="controls::GuiScroll::IStyleController"/> to make a tracker control (or a slide bar).</summary>
			class CommonTrackStyle : public Object, public virtual controls::GuiScroll::IStyleController, public Description<CommonTrackStyle>
			{
			public:
				/// <summary>Track direction.</summary>
				enum Direction
				{
					/// <summary>Horizontal scroll.</summary>
					Horizontal,
					/// <summary>Vertical scroll.</summary>
					Vertical,
				};
			protected:
				Direction											direction;
				controls::GuiScroll::ICommandExecutor*				commandExecutor;
				compositions::GuiBoundsComposition*					boundsComposition;
				controls::GuiButton*								handleButton;
				compositions::GuiTableComposition*					handleComposition;

				vint												totalSize;
				vint												pageSize;
				vint												position;
				Point												draggingStartLocation;
				bool												draggingHandle;

				void												UpdateHandle();
				void												OnHandleMouseDown(compositions::GuiGraphicsComposition* sender,compositions::GuiMouseEventArgs& arguments);
				void												OnHandleMouseMove(compositions::GuiGraphicsComposition* sender,compositions::GuiMouseEventArgs& arguments);
				void												OnHandleMouseUp(compositions::GuiGraphicsComposition* sender,compositions::GuiMouseEventArgs& arguments);
				
				/// <summary>Callback to create a style controller for the handle button.</summary>
				/// <returns>The created style controller.</returns>
				/// <param name="direction">The direction of the tracker.</param>
				virtual controls::GuiButton::IStyleController*		CreateHandleButtonStyle(Direction direction)=0;
				/// <summary>Install necessary compositions and elements to the background.</summary>
				/// <param name="boundsComposition">The background composition.</param>
				/// <param name="direction">The direction of the tracker.</param>
				virtual void										InstallBackground(compositions::GuiGraphicsComposition* boundsComposition, Direction direction)=0;
				/// <summary>Install necessary compositions and elements to the tracker groove.</summary>
				/// <param name="boundsComposition">The tracker groove composition.</param>
				/// <param name="direction">The direction of the tracker.</param>
				virtual void										InstallTrack(compositions::GuiGraphicsComposition* trackComposition, Direction direction)=0;
				/// <summary>Build the style. This function is supposed to be called in the contructor of the sub class.</summary>
				/// <param name="trackThickness">The thickness of the tracker troove.</param>
				/// <param name="trackPadding">The padding between the tracker groove to the control.</param>
				/// <param name="handleLong">The size of the long size of the handle button. Horizontal: height; Vertical: width.</param>
				/// <param name="handleShort">The size of the short size of the handle button. Horizontal: width; Vertical: height.</param>
				void												BuildStyle(vint trackThickness, vint trackPadding, vint handleLong, vint handleShort);
			public:
				/// <summary>Create the style controller using a specified direction.</summary>
				/// <param name="_direction">The specified direction</param>
				CommonTrackStyle(Direction _direction);
				~CommonTrackStyle();

				compositions::GuiBoundsComposition*					GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*				GetContainerComposition()override;
				void												SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void												SetText(const WString& value)override;
				void												SetFont(const FontProperties& value)override;
				void												SetVisuallyEnabled(bool value)override;
				void												SetCommandExecutor(controls::GuiScroll::ICommandExecutor* value)override;
				void												SetTotalSize(vint value)override;
				void												SetPageSize(vint value)override;
				void												SetPosition(vint value)override;
			};

/***********************************************************************
CommonFragmentBuilder
***********************************************************************/

			class CommonFragmentBuilder
			{
			private:
				static compositions::GuiBoundsComposition*			BuildDockedElementContainer(elements::IGuiGraphicsElement* element);
			public:
				static void											FillUpArrow(elements::GuiPolygonElement* element);
				static void											FillDownArrow(elements::GuiPolygonElement* element);
				static void											FillLeftArrow(elements::GuiPolygonElement* element);
				static void											FillRightArrow(elements::GuiPolygonElement* element);

				static elements::GuiPolygonElement*					BuildUpArrow();
				static elements::GuiPolygonElement*					BuildDownArrow();
				static elements::GuiPolygonElement*					BuildLeftArrow();
				static elements::GuiPolygonElement*					BuildRightArrow();

				static compositions::GuiBoundsComposition*			BuildUpArrow(elements::GuiPolygonElement*& elementOut);
				static compositions::GuiBoundsComposition*			BuildDownArrow(elements::GuiPolygonElement*& elementOut);
				static compositions::GuiBoundsComposition*			BuildLeftArrow(elements::GuiPolygonElement*& elementOut);
				static compositions::GuiBoundsComposition*			BuildRightArrow(elements::GuiPolygonElement*& elementOut);
			};
		}

/***********************************************************************
Helper Functions
***********************************************************************/
			
		extern unsigned char							IntToColor(vint color);
		extern Color									BlendColor(Color c1, Color c2, vint currentPosition, vint totalLength);

/***********************************************************************
Animation
***********************************************************************/

#define DEFINE_TRANSFERRING_ANIMATION(TSTATE, TSTYLECONTROLLER)\
				class TransferringAnimation : public compositions::GuiTimeBasedAnimation\
				{\
				protected:\
					TSTATE									colorBegin;\
					TSTATE									colorEnd;\
					TSTATE									colorCurrent;\
					TSTYLECONTROLLER*						style;\
					bool									stopped;\
					bool									disabled;\
					bool									enableAnimation;\
					void									PlayInternal(vint currentPosition, vint totalLength);\
				public:\
					TransferringAnimation(TSTYLECONTROLLER* _style, const TSTATE& begin);\
					void									Disable();\
					void									Play(vint currentPosition, vint totalLength)override;\
					void									Stop()override;\
					bool									GetEnableAnimation();\
					void									SetEnableAnimation(bool value);\
					void									Transfer(const TSTATE& end);\
				};\

/***********************************************************************
Animation Implementation
***********************************************************************/

#define DEFAULT_TRANSFERRING_ANIMATION_HOST_GETTER(STYLE) (STYLE->GetBoundsComposition()->GetRelatedGraphicsHost())

#define IMPLEMENT_TRANSFERRING_ANIMATION_BASE(TSTATE, TSTYLECONTROLLER, HOST_GETTER)\
			TSTYLECONTROLLER::TransferringAnimation::TransferringAnimation(TSTYLECONTROLLER* _style, const TSTATE& begin)\
				:GuiTimeBasedAnimation(0)\
				,colorBegin(begin)\
				,colorEnd(begin)\
				,colorCurrent(begin)\
				,style(_style)\
				,stopped(true)\
				,disabled(false)\
				,enableAnimation(true)\
			{\
			}\
			void TSTYLECONTROLLER::TransferringAnimation::Disable()\
			{\
				disabled=true;\
			}\
			void TSTYLECONTROLLER::TransferringAnimation::Play(vint currentPosition, vint totalLength)\
			{\
				if(!disabled)\
				{\
					PlayInternal(currentPosition, totalLength);\
				}\
			}\
			void TSTYLECONTROLLER::TransferringAnimation::Stop()\
			{\
				stopped=true;\
			}\
			bool TSTYLECONTROLLER::TransferringAnimation::GetEnableAnimation()\
			{\
				return enableAnimation;\
			}\
			void TSTYLECONTROLLER::TransferringAnimation::SetEnableAnimation(bool value)\
			{\
				enableAnimation=value;\
			}\
			void TSTYLECONTROLLER::TransferringAnimation::Transfer(const TSTATE& end)\
			{\
				if(colorEnd!=end)\
				{\
					GuiGraphicsHost* host=HOST_GETTER(style);\
					if(enableAnimation && host)\
					{\
						Restart(120);\
						if(stopped)\
						{\
							colorBegin=colorEnd;\
							colorEnd=end;\
							host->GetAnimationManager()->AddAnimation(style->transferringAnimation);\
							stopped=false;\
						}\
						else\
						{\
							colorBegin=colorCurrent;\
							colorEnd=end;\
						}\
					}\
					else\
					{\
						colorBegin=end;\
						colorEnd=end;\
						colorCurrent=end;\
						Play(1, 1);\
					}\
				}\
			}\
			void TSTYLECONTROLLER::TransferringAnimation::PlayInternal(vint currentPosition, vint totalLength)\

#define IMPLEMENT_TRANSFERRING_ANIMATION(TSTATE, TSTYLECONTROLLER)\
	IMPLEMENT_TRANSFERRING_ANIMATION_BASE(TSTATE, TSTYLECONTROLLER, DEFAULT_TRANSFERRING_ANIMATION_HOST_GETTER)
	}
}

#endif