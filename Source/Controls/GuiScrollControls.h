/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUISCROLLCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUISCROLLCONTROLS

#include "GuiBasicControls.h"

namespace vl
{
	namespace presentation
	{

		namespace controls
		{

/***********************************************************************
Scrolls
***********************************************************************/

			/// <summary>A scroll control, which represents a one dimension sub range of a whole range.</summary>
			class GuiScroll : public GuiControl, public Description<GuiScroll>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(ScrollTemplate, GuiControl)
			protected:
				class CommandExecutor : public Object, public IScrollCommandExecutor
				{
				protected:
					GuiScroll*							scroll;
				public:
					CommandExecutor(GuiScroll* _scroll);
					~CommandExecutor();

					void								SmallDecrease()override;
					void								SmallIncrease()override;
					void								BigDecrease()override;
					void								BigIncrease()override;

					void								SetTotalSize(vint value)override;
					void								SetPageSize(vint value)override;
					void								SetPosition(vint value)override;
				};

				Ptr<CommandExecutor>					commandExecutor;
				vint									totalSize = 100;
				vint									pageSize = 10;
				vint									position = 0;
				vint									smallMove = 1;
				vint									bigMove = 10;
				bool									autoFocus = true;

				void									OnActiveAlt()override;
				bool									IsTabAvailable()override;
				void									OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments);
				void									OnMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiScroll(theme::ThemeName themeName);
				~GuiScroll();
				
				/// <summary>Total size changed event.</summary>
				compositions::GuiNotifyEvent			TotalSizeChanged;
				/// <summary>Page size changed event.</summary>
				compositions::GuiNotifyEvent			PageSizeChanged;
				/// <summary>Position changed event.</summary>
				compositions::GuiNotifyEvent			PositionChanged;
				/// <summary>Small move changed event.</summary>
				compositions::GuiNotifyEvent			SmallMoveChanged;
				/// <summary>Big move changed event.</summary>
				compositions::GuiNotifyEvent			BigMoveChanged;
				
				/// <summary>Get the total size.</summary>
				/// <returns>The total size.</returns>
				virtual vint							GetTotalSize();
				/// <summary>Set the total size.</summary>
				/// <param name="value">The total size.</param>
				virtual void							SetTotalSize(vint value);
				/// <summary>Get the page size.</summary>
				/// <returns>The page size.</returns>
				virtual vint							GetPageSize();
				/// <summary>Set the page size.</summary>
				/// <param name="value">The page size.</param>
				virtual void							SetPageSize(vint value);
				/// <summary>Get the position.</summary>
				/// <returns>The position.</returns>
				virtual vint							GetPosition();
				/// <summary>Set the position.</summary>
				/// <param name="value">The position.</param>
				virtual void							SetPosition(vint value);
				/// <summary>Get the small move.</summary>
				/// <returns>The small move.</returns>
				virtual vint							GetSmallMove();
				/// <summary>Set the small move.</summary>
				/// <param name="value">The small move.</param>
				virtual void							SetSmallMove(vint value);
				/// <summary>Get the big move.</summary>
				/// <returns>The big move.</returns>
				virtual vint							GetBigMove();
				/// <summary>Set the big move.</summary>
				/// <param name="value">The big move.</param>
				virtual void							SetBigMove(vint value);
				
				/// <summary>Get the minimum possible position.</summary>
				/// <returns>The minimum possible position.</returns>
				vint									GetMinPosition();
				/// <summary>Get the maximum possible position.</summary>
				/// <returns>The maximum possible position.</returns>
				vint									GetMaxPosition();

				/// <summary>Test if the scroll gets focus when it is clicked.</summary>
				/// <returns>Returns true if the scroll gets focus when it is clicked</returns>
				bool									GetAutoFocus();
				/// <summary>Set if the scroll gets focus when it is clicked.</summary>
				/// <param name="value">Set to true to make this scroll get focus when it is clicked.</param>
				void									SetAutoFocus(bool value);
			};
		}
	}
}

#endif
