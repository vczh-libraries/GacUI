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
			class GuiDatePicker : public GuiControl, public Description<GuiDatePicker>
			{
			public:
				/// <summary>Style provider interface for <see cref="GuiDatePicker"/>.</summary>
				class IStyleProvider : public virtual GuiControl::IStyleProvider, public Description<IStyleProvider>
				{
				public:
					/// <summary>Create a style for date button for choosing "day".</summary>
					/// <returns>The created style.</returns>
					virtual GuiSelectableButton::IStyleController*		CreateDateButtonStyle()=0;
					/// <summary>Create a text list for candidate "year" and "month".</summary>
					/// <returns>The created control.</returns>
					virtual GuiTextList*								CreateTextList()=0;
					/// <summary>Create a combo box style for "year" and "month".</summary>
					/// <returns>The created style.</returns>
					virtual GuiComboBoxListControl::IStyleController*	CreateComboBoxStyle()=0;

					/// <summary>Get the color for background.</summary>
					/// <returns>The color.</returns>
					virtual Color										GetBackgroundColor()=0;
					/// <summary>Get the color for "day" that in the current month.</summary>
					/// <returns>The color.</returns>
					virtual Color										GetPrimaryTextColor()=0;
					/// <summary>Get the color for "day" that not in the current month.</summary>
					/// <returns>The color.</returns>
					virtual Color										GetSecondaryTextColor()=0;
				};

				/// <summary>Style controller for <see cref="GuiDatePicker"/>.</summary>
				class StyleController : public Object, public virtual GuiControl::IStyleController, public Description<StyleController>
				{
				protected:
					static const vint									DaysOfWeek=7;
					static const vint									DayRows=6;
					static const vint									DayRowStart=2;
					static const vint									YearFirst=1900;
					static const vint									YearLast=2099;

					IStyleProvider*										styleProvider;
					GuiDatePicker*										datePicker;
					DateTime											currentDate;
					Locale												dateLocale;
					compositions::GuiTableComposition*					boundsComposition;
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
				public:
					/// <summary>Create a style controller with a specified style provider.</summary>
					/// <param name="_styleProvider">The style provider.</param>
					StyleController(IStyleProvider* _styleProvider);
					~StyleController();

					compositions::GuiBoundsComposition*					GetBoundsComposition()override;
					compositions::GuiGraphicsComposition*				GetContainerComposition()override;
					void												SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
					void												SetText(const WString& value)override;
					void												SetFont(const FontProperties& value)override;
					void												SetVisuallyEnabled(bool value)override;
					
					/// <summary>Set the data picker that owns this style controller.</summary>
					/// <param name="_datePicker">The date picker.</param>
					void												SetDatePicker(GuiDatePicker* _datePicker);
					/// <summary>Set the locale to display texts.</summary>
					/// <param name="_dateLocale">The locale.</param>
					void												SetDateLocale(const Locale& _dateLocale);
					/// <summary>Get the displayed date.</summary>
					/// <returns>The date.</returns>
					const DateTime&										GetDate();
					/// <summary>Display a date.</summary>
					/// <param name="value">The date.</param>
					/// <param name="forceUpdate">Set to true to refill all data in the control whatever cached or not.</param>
					void												SetDate(const DateTime& value, bool forceUpdate=false);
				};

			protected:
				StyleController*										styleController;
				WString													dateFormat;
				Locale													dateLocale;

				void													UpdateText();
				void													NotifyDateChanged();
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="styleProvider">The style provider.</param>
				GuiDatePicker(IStyleProvider* _styleProvider);
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
				/// <param name="styleProvider">The style provider.</param>
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