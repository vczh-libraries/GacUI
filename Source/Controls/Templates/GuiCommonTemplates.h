/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICOMMONTEMPLATES
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICOMMONTEMPLATES

#include "GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{

/***********************************************************************
GuiCommonDatePickerLook
***********************************************************************/

			class GuiCommonDatePickerLook : public GuiTemplate, public Description<GuiCommonDatePickerLook>
			{
			protected:
				static const vint									DaysOfWeek = 7;
				static const vint									DayRows = 6;
				static const vint									DayRowStart = 2;
				static const vint									YearFirst = 1900;
				static const vint									YearLast = 2099;

				Color												backgroundColor;
				Color												primaryTextColor;
				Color												secondaryTextColor;
				DateTime											currentDate;
				Locale												dateLocale;
				FontProperties										font;

				TemplateProperty<GuiSelectableButtonTemplate>		dateButtonTemplate;
				TemplateProperty<GuiTextListTemplate>				dateTextListTemplate;
				TemplateProperty<GuiComboBoxTemplate>				dateComboBoxTemplate;

				controls::IDatePickerCommandExecutor*				commands = nullptr;
				bool												preventComboEvent = false;
				bool												preventButtonEvent = false;

				controls::GuiComboBoxListControl*					comboYear;
				controls::GuiTextList*								listYears;
				controls::GuiComboBoxListControl*					comboMonth;
				controls::GuiTextList*								listMonths;
				collections::Array<elements::GuiSolidLabelElement*>	labelDaysOfWeek;
				collections::Array<controls::GuiSelectableButton*>	buttonDays;
				collections::Array<elements::GuiSolidLabelElement*>	labelDays;
				collections::Array<DateTime>						dateDays;

				void												SetDay(const DateTime& day, vint& index, vint monthOffset);
				void												DisplayMonth(vint year, vint month);
				void												SelectDay(vint day);

				void												comboYearMonth_SelectedIndexChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void												buttonDay_SelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

			public:
				GuiCommonDatePickerLook(Color _backgroundColor, Color _primaryTextColor, Color _secondaryTextColor);
				~GuiCommonDatePickerLook();

				compositions::GuiNotifyEvent						DateChanged;

				controls::IDatePickerCommandExecutor*				GetCommands();
				void												SetCommands(controls::IDatePickerCommandExecutor* value);
				TemplateProperty<GuiSelectableButtonTemplate>		GetDateButtonTemplate();
				void												SetDateButtonTemplate(const TemplateProperty<GuiSelectableButtonTemplate>& value);
				TemplateProperty<GuiTextListTemplate>				GetDateTextListTemplate();
				void												SetDateTextListTemplate(const TemplateProperty<GuiTextListTemplate>& value);
				TemplateProperty<GuiComboBoxTemplate>				GetDateComboBoxTemplate();
				void												SetDateComboBoxTemplate(const TemplateProperty<GuiComboBoxTemplate>& value);

				const Locale&										GetDateLocale();
				void												SetDateLocale(const Locale& value);
				const DateTime&										GetDate();
				void												SetDate(const DateTime& value);
				const FontProperties&								GetFont();
				void												SetFont(const FontProperties& value);
			};

/***********************************************************************
GuiCommonScrollViewLook
***********************************************************************/

			class GuiCommonScrollViewLook : public GuiTemplate, public Description<GuiCommonScrollViewLook>
			{
			protected:
				controls::GuiScroll*								horizontalScroll = nullptr;
				controls::GuiScroll*								verticalScroll = nullptr;
				compositions::GuiTableComposition*					tableComposition = nullptr;
				compositions::GuiCellComposition*					containerCellComposition = nullptr;
				compositions::GuiBoundsComposition*					containerComposition = nullptr;

				vint												defaultScrollSize = 12;
				TemplateProperty<GuiScrollTemplate>					hScrollTemplate;
				TemplateProperty<GuiScrollTemplate>					vScrollTemplate;

				void												UpdateTable();
				void												hScroll_OnVisibleChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void												vScroll_OnVisibleChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				GuiCommonScrollViewLook(vint _defaultScrollSize);
				~GuiCommonScrollViewLook();

				controls::GuiScroll*								GetHScroll();
				controls::GuiScroll*								GetVScroll();
				compositions::GuiGraphicsComposition*				GetContainerComposition();

				TemplateProperty<GuiScrollTemplate>					GetHScrollTemplate();
				void												SetHScrollTemplate(const TemplateProperty<GuiScrollTemplate>& value);
				TemplateProperty<GuiScrollTemplate>					GetVScrollTemplate();
				void												SetVScrollTemplate(const TemplateProperty<GuiScrollTemplate>& value);
			};

/***********************************************************************
GuiCommonScrollBehavior
***********************************************************************/

			class GuiCommonScrollBehavior : public controls::GuiComponent, public Description<GuiCommonScrollBehavior>
			{
			protected:
				bool												dragging = false;
				Point												location = { 0,0 };
				GuiScrollTemplate*									scrollTemplate = nullptr;

				void												SetScroll(vint totalPixels, vint newOffset);
				void												AttachHandle(compositions::GuiGraphicsComposition* handle);
			public:
				GuiCommonScrollBehavior();
				~GuiCommonScrollBehavior();

				void												AttachScrollTemplate(GuiScrollTemplate* value);
				void												AttachDecreaseButton(controls::GuiButton* button);
				void												AttachIncreaseButton(controls::GuiButton* button);
				void												AttachHorizontalScrollHandle(compositions::GuiPartialViewComposition* partialView);
				void												AttachVerticalScrollHandle(compositions::GuiPartialViewComposition* partialView);
				void												AttachHorizontalTrackerHandle(compositions::GuiPartialViewComposition* partialView);
				void												AttachVerticalTrackerHandle(compositions::GuiPartialViewComposition* partialView);

				vint												GetHorizontalTrackerHandlerPosition(compositions::GuiBoundsComposition* handle, vint totalSize, vint pageSize, vint position);
				vint												GetVerticalTrackerHandlerPosition(compositions::GuiBoundsComposition* handle, vint totalSize, vint pageSize, vint position);
			};
		}
	}
}

#endif