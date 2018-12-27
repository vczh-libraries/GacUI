/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDATETIMECONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIDATETIMECONTROLS

#include "ListControlPackage/GuiComboControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
DatePicker
***********************************************************************/

			/// <summary>Date picker control that display a calendar.</summary>
			class GuiDatePicker : public GuiControl, protected compositions::GuiAltActionHostBase, public Description<GuiDatePicker>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(DatePickerTemplate, GuiControl)
			protected:
				class CommandExecutor : public Object, public IDatePickerCommandExecutor
				{
				protected:
					GuiDatePicker*										datePicker;
				public:
					CommandExecutor(GuiDatePicker* _datePicker);
					~CommandExecutor();

					void												NotifyDateChanged()override;
					void												NotifyDateNavigated()override;
					void												NotifyDateSelected()override;
				};

				Ptr<CommandExecutor>									commandExecutor;
				DateTime												date;
				WString													dateFormat;
				Locale													dateLocale;
				compositions::IGuiAltActionHost*						previousAltHost = nullptr;
				bool													nestedAlt = false;

				void													UpdateText();
				bool													IsAltAvailable()override;
				compositions::IGuiAltActionHost*						GetActivatingAltHost()override;

			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_nestedAlt">Set to true to make this date picker an <see cref="compositions::IGuiAltActionHost"/>.</param>
				GuiDatePicker(theme::ThemeName themeName, bool _nestedAlt = true);
				~GuiDatePicker();

				/// <summary>Date changed event.</summary>
				compositions::GuiNotifyEvent							DateChanged;
				/// <summary>Date navigated event. Called when the current month is changed.</summary>
				compositions::GuiNotifyEvent							DateNavigated;
				/// <summary>Date selected event. Called when a day button is selected.</summary>
				compositions::GuiNotifyEvent							DateSelected;
				/// <summary>Date format changed event.</summary>
				compositions::GuiNotifyEvent							DateFormatChanged;
				/// <summary>Date locale changed event.</summary>
				compositions::GuiNotifyEvent							DateLocaleChanged;
				
				/// <summary>Get the displayed date.</summary>
				/// <returns>The date.</returns>
				const DateTime&											GetDate();
				/// <summary>Display a date.</summary>
				/// <param name="value">The date.</param>
				void													SetDate(const DateTime& value);
				/// <summary>Get the format.</summary>
				/// <returns>The format.</returns>
				const WString&											GetDateFormat();
				/// <summary>Set the format for the text of this control.</summary>
				/// <param name="value">The format.</param>
				void													SetDateFormat(const WString& value);
				/// <summary>Get the locale.</summary>
				/// <returns>The locale.</returns>
				const Locale&											GetDateLocale();
				/// <summary>Set the locale to display texts.</summary>
				/// <param name="value">The locale.</param>
				void													SetDateLocale(const Locale& value);

				void													SetText(const WString& value)override;
			};

/***********************************************************************
DateComboBox
***********************************************************************/
			
			/// <summary>A combo box control with a date picker control.</summary>
			class GuiDateComboBox : public GuiComboBoxBase, public Description<GuiDateComboBox>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(DateComboBoxTemplate, GuiComboBoxBase)
			protected:
				GuiDatePicker*											datePicker;
				DateTime												selectedDate;
				
				void													UpdateText();
				void													NotifyUpdateSelectedDate();
				void													OnSubMenuOpeningChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													datePicker_DateLocaleChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													datePicker_DateFormatChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													datePicker_DateSelected(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiDateComboBox(theme::ThemeName themeName);
				~GuiDateComboBox();
				
				/// <summary>Selected data changed event.</summary>
				compositions::GuiNotifyEvent							SelectedDateChanged;
				
				void													SetFont(const Nullable<FontProperties>& value)override;
				/// <summary>Get the displayed date.</summary>
				/// <returns>The date.</returns>
				const DateTime&											GetSelectedDate();
				/// <summary>Display a date.</summary>
				/// <param name="value">The date.</param>
				void													SetSelectedDate(const DateTime& value);
				/// <summary>Get the date picker control.</summary>
				/// <returns>The date picker control.</returns>
				GuiDatePicker*											GetDatePicker();
			};
		}
	}
}

#endif