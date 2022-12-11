/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLSHARED
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLSHARED

#include "../../GraphicsComposition/IncludeForward.h"
#include "../../Resources/GuiResource.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiControlHost;
			class GuiCustomControl;

			/// <summary>The visual state for button.</summary>
			enum class ButtonState
			{
				/// <summary>Normal state.</summary>
				Normal,
				/// <summary>Active state (when the cursor is hovering on a button).</summary>
				Active,
				/// <summary>Pressed state (when the buttin is being pressed).</summary>
				Pressed,
			};

			/// <summary>Represents the sorting state of list view items related to this column.</summary>
			enum class ColumnSortingState
			{
				/// <summary>Not sorted.</summary>
				NotSorted,
				/// <summary>Ascending.</summary>
				Ascending,
				/// <summary>Descending.</summary>
				Descending,
			};

			/// <summary>Represents the order of tab pages.</summary>
			enum class TabPageOrder
			{
				/// <summary>Unknown.</summary>
				Unknown,
				/// <summary>Left to right.</summary>
				LeftToRight,
				/// <summary>Right to left.</summary>
				RightToLeft,
				/// <summary>Top to bottom.</summary>
				TopToBottom,
				/// <summary>Bottom to top.</summary>
				BottomToTop,
			};

			/// <summary>A command executor for the combo box to change the control state.</summary>
			class ITextBoxCommandExecutor : public virtual IDescriptable, public Description<ITextBoxCommandExecutor>
			{
			public:
				/// <summary>Override the text content in the control.</summary>
				/// <param name="value">The new text content.</param>
				virtual void						UnsafeSetText(const WString& value) = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IScrollCommandExecutor : public virtual IDescriptable, public Description<IScrollCommandExecutor>
			{
			public:
				/// <summary>Do small decrement.</summary>
				virtual void						SmallDecrease() = 0;
				/// <summary>Do small increment.</summary>
				virtual void						SmallIncrease() = 0;
				/// <summary>Do big decrement.</summary>
				virtual void						BigDecrease() = 0;
				/// <summary>Do big increment.</summary>
				virtual void						BigIncrease() = 0;

				/// <summary>Change to total size of the scroll.</summary>
				/// <param name="value">The total size.</param>
				virtual void						SetTotalSize(vint value) = 0;
				/// <summary>Change to page size of the scroll.</summary>
				/// <param name="value">The page size.</param>
				virtual void						SetPageSize(vint value) = 0;
				/// <summary>Change to position of the scroll.</summary>
				/// <param name="value">The position.</param>
				virtual void						SetPosition(vint value) = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class ITabCommandExecutor : public virtual IDescriptable, public Description<ITabCommandExecutor>
			{
			public:
				/// <summary>Select a tab page.</summary>
				/// <param name="index">The specified position for the tab page.</param>
				/// <param name="setFocus">Set to true to set focus to the tab control.</param>
				virtual void						ShowTab(vint index, bool setFocus) = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IDatePickerCommandExecutor : public virtual IDescriptable, public Description<IDatePickerCommandExecutor>
			{
			public:
				/// <summary>Called when the date has been changed.</summary>
				virtual void						NotifyDateChanged() = 0;
				/// <summary>Called when navigated to a date.</summary>
				virtual void						NotifyDateNavigated() = 0;
				/// <summary>Called when selected a date.</summary>
				virtual void						NotifyDateSelected() = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IRibbonGroupCommandExecutor : public virtual IDescriptable, public Description<IRibbonGroupCommandExecutor>
			{
			public:
				/// <summary>Called when the expand button is clicked.</summary>
				virtual void						NotifyExpandButtonClicked() = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IRibbonGalleryCommandExecutor : public virtual IDescriptable, public Description<IRibbonGalleryCommandExecutor>
			{
			public:
				/// <summary>Called when the scroll up button is clicked.</summary>
				virtual void						NotifyScrollUp() = 0;
				/// <summary>Called when the scroll down button is clicked.</summary>
				virtual void						NotifyScrollDown() = 0;
				/// <summary>Called when the dropdown button is clicked.</summary>
				virtual void						NotifyDropdown() = 0;
			};
		}
	}
}

#endif