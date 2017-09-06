/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDATETIMECONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIDATETIMECONTROLS

#include "ListControlPackage/GuiComboControls.h"
#include "ListControlPackage/GuiTextListControls.h"

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
			class GuiDatePicker : public GuiControl, public IDatePickerCommandExecutor, public Description<GuiDatePicker>
			{
			public:
				/// <summary>Style provider interface for <see cref="GuiDatePicker"/>.</summary>
				class IStyleController : public virtual GuiControl::IStyleController, public Description<IStyleController>
				{
				public:
					virtual void										SetCommandExecutor(IDatePickerCommandExecutor* value) = 0;
					virtual void										SetDateLocale(const Locale& value) = 0;
					virtual const DateTime&								GetDate() = 0;
					virtual void										SetDate(const DateTime& value) = 0;
				};

				/// <summary>Style controller for <see cref="GuiDatePicker"/>.</summary>
				class ControlTemplate : public templates::GuiControlTemplate, public Description<ControlTemplate>
				{
				protected:
					static const vint									DaysOfWeek=7;
					static const vint									DayRows=6;
					static const vint									DayRowStart=2;
					static const vint									YearFirst=1900;
					static const vint									YearLast=2099;

					IDatePickerCommandExecutor*							commands = nullptr;
					DateTime											currentDate;
					Locale												dateLocale;
					bool												preventComboEvent;
					bool												preventButtonEvent;

					GuiComboBoxListControl*								comboYear;
					GuiTextList*										listYears;
					GuiComboBoxListControl*								comboMonth;
					GuiTextList*										listMonths;
					collections::Array<elements::GuiSolidLabelElement*>	labelDaysOfWeek;
					collections::Array<GuiSelectableButton*>			buttonDays;
					collections::Array<elements::GuiSolidLabelElement*>	labelDays;
					collections::Array<DateTime>						dateDays;
					Ptr<GuiSelectableButton::GroupController>			dayMutexController;

					void												SetDay(const DateTime& day, vint& index, bool currentMonth);
					void												DisplayMonth(vint year, vint month);
					void												SelectDay(vint day);

					void												comboYearMonth_SelectedIndexChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												buttonDay_SelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												UpdateData(const DateTime& value, bool forceUpdate);

				protected:

					virtual GuiSelectableButton::IStyleController*		CreateDateButtonStyle() = 0;
					virtual GuiTextList*								CreateTextList() = 0;
					virtual GuiComboBoxListControl::IStyleController*	CreateComboBoxStyle() = 0;
					virtual Color										GetBackgroundColor() = 0;
					virtual Color										GetPrimaryTextColor() = 0;
					virtual Color										GetSecondaryTextColor() = 0;
				public:
					ControlTemplate();
					~ControlTemplate();

					compositions::GuiNotifyEvent						DateLocaleChanged;
					compositions::GuiNotifyEvent						DateChanged;
					compositions::GuiNotifyEvent						CommandsChanged;

					IDatePickerCommandExecutor*							GetCommands();
					void												SetCommands(IDatePickerCommandExecutor* value);
					const Locale&										GetDateLocale();
					void												SetDateLocale(const Locale& value);
					const DateTime&										GetDate();
					void												SetDate(const DateTime& value);

					void												Initialize()override;
				};

			protected:
				IStyleController*										styleController;
				WString													dateFormat;
				Locale													dateLocale;

				void													UpdateText();
				void													NotifyDateChanged()override;
				void													NotifyDateNavigated()override;
				void													NotifyDateSelected()override;
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="_styleProvider">The style provider.</param>
				GuiDatePicker(IStyleController* _styleController);
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
				/// <param name="_styleController">The style provider.</param>
				/// <param name="_datePicker">The date picker control to show in the popup.</param>
				GuiDateComboBox(IStyleController* _styleController, GuiDatePicker* _datePicker);
				~GuiDateComboBox();
				
				/// <summary>Selected data changed event.</summary>
				compositions::GuiNotifyEvent							SelectedDateChanged;
				
				void													SetFont(const FontProperties& value)override;
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