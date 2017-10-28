#include "GuiCommonTemplates.h"
#include "GuiThemeStyleFactory.h"
#include "../ListControlPackage/GuiComboControls.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{
			using namespace elements;
			using namespace compositions;
			using namespace templates;
			using namespace controls;
			using namespace theme;

/***********************************************************************
GuiCommonDatePickerLook
***********************************************************************/

			vint GetDayCountForMonth(vint year, vint month)
			{
				bool isLeapYear = (year % 100 == 0) ? (year % 400 == 0) : (year % 4 == 0);
				switch (month)
				{
				case 1:case 3:case 5:case 7:case 8:case 10:case 12:
					return 31;
				case 4:case 6:case 9:case 11:
					return 30;
				default:
					return isLeapYear ? 29 : 28;
				}
			}

			void StepPreviousMonth(vint& year, vint& month)
			{
				if (month == 1)
				{
					year--;
					month = 12;
				}
				else
				{
					month--;
				}
			}

			void StepNextMonth(vint& year, vint& month)
			{
				if (month == 12)
				{
					year++;
					month = 1;
				}
				else
				{
					month++;
				}
			}

			void GuiCommonDatePickerLook::SetDay(const DateTime& day, vint& index, bool currentMonth)
			{
				dateDays[index] = day;
				GuiSolidLabelElement* label = labelDays[index];
				label->SetText(itow(day.day));
				label->SetColor(currentMonth ? primaryTextColor : secondaryTextColor);
				index++;
			}

			void GuiCommonDatePickerLook::comboYearMonth_SelectedIndexChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (!preventComboEvent)
				{
					if (comboYear->GetSelectedIndex() != -1 && comboMonth->GetSelectedIndex() != -1)
					{
						vint year = comboYear->GetSelectedIndex() + YearFirst;
						vint month = comboMonth->GetSelectedIndex() + 1;
						SetDate(DateTime::FromDateTime(year, month, 1));

						GuiEventArgs arguments(this);
						DateChanged.Execute(arguments);
						commands->NotifyDateChanged();
						commands->NotifyDateNavigated();
					}
				}
			}

			void GuiCommonDatePickerLook::buttonDay_SelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (!preventButtonEvent)
				{
					GuiSelectableButton* button = dynamic_cast<GuiSelectableButton*>(sender->GetRelatedControl());
					if (button->GetSelected())
					{
						vint index = buttonDays.IndexOf(button);
						if (index != -1)
						{
							DateTime day = dateDays[index];
							if (day.year != currentDate.year || day.month != currentDate.month)
							{
								SetDate(day);
							}
							else
							{
								currentDate = day;
							}

							GuiEventArgs arguments(this);
							DateChanged.Execute(arguments);
							commands->NotifyDateChanged();
							commands->NotifyDateSelected();
						}
					}
				}
			}

			void GuiCommonDatePickerLook::DisplayMonth(vint year, vint month)
			{
				if (YearFirst <= year && year <= YearLast && 1 <= month && month <= 12)
				{
					preventComboEvent = true;
					comboYear->SetSelectedIndex(year - YearFirst);
					comboMonth->SetSelectedIndex(month - 1);
					preventComboEvent = false;
				}

				vint yearPrev = year, yearNext = year, monthPrev = month, monthNext = month;
				StepPreviousMonth(yearPrev, monthPrev);
				StepNextMonth(yearNext, monthNext);

				vint countPrev = GetDayCountForMonth(yearPrev, monthPrev);
				vint count = GetDayCountForMonth(year, month);
				vint countNext = GetDayCountForMonth(yearNext, monthNext);

				DateTime firstDay = DateTime::FromDateTime(year, month, 1);
				vint showPrev = firstDay.dayOfWeek;
				if (showPrev == 0) showPrev = DaysOfWeek;
				vint show = count;
				vint showNext = DaysOfWeek*DayRows - showPrev - show;

				vint index = 0;
				for (vint i = 0; i < showPrev; i++)
				{
					DateTime day = DateTime::FromDateTime(yearPrev, monthPrev, countPrev - (showPrev - i - 1));
					SetDay(day, index, false);
				}
				for (vint i = 0; i < show; i++)
				{
					DateTime day = DateTime::FromDateTime(year, month, i + 1);
					SetDay(day, index, true);
				}
				for (vint i = 0; i < showNext; i++)
				{
					DateTime day = DateTime::FromDateTime(yearNext, monthNext, i + 1);
					SetDay(day, index, false);
				}
			}

			void GuiCommonDatePickerLook::SelectDay(vint day)
			{
				for (vint i = 0; i < dateDays.Count(); i++)
				{
					const DateTime& dt = dateDays[i];
					if (dt.year == currentDate.year && dt.month == currentDate.month && dt.day == day)
					{
						preventButtonEvent = true;
						buttonDays[i]->SetSelected(true);
						preventButtonEvent = false;
						break;
					}
				}
			}

			GuiCommonDatePickerLook::GuiCommonDatePickerLook(Color _backgroundColor, Color _primaryTextColor, Color _secondaryTextColor)
				:backgroundColor(_backgroundColor)
				, primaryTextColor(_primaryTextColor)
				, secondaryTextColor(_secondaryTextColor)
			{
				DateChanged.SetAssociatedComposition(this);
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

				GuiTableComposition* monthTable = 0;
				GuiTableComposition* dayTable = 0;
				{
					listYears = new GuiTextList(theme::ThemeName::TextList);
					listYears->SetHorizontalAlwaysVisible(false);
					for (vint i = YearFirst; i <= YearLast; i++)
					{
						listYears->GetItems().Add(new list::TextItem(itow(i)));
					}
					comboYear = new GuiComboBoxListControl(theme::ThemeName::ComboBox, listYears);
					comboYear->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 2, 0));
					comboYear->SelectedIndexChanged.AttachMethod(this, &GuiCommonDatePickerLook::comboYearMonth_SelectedIndexChanged);
				}
				{
					listMonths = new GuiTextList(theme::ThemeName::TextList);
					listMonths->SetHorizontalAlwaysVisible(false);
					comboMonth = new GuiComboBoxListControl(theme::ThemeName::ComboBox, listMonths);
					comboMonth->GetBoundsComposition()->SetAlignmentToParent(Margin(2, 0, 0, 0));
					comboMonth->SelectedIndexChanged.AttachMethod(this, &GuiCommonDatePickerLook::comboYearMonth_SelectedIndexChanged);
				}
				{
					monthTable = new GuiTableComposition;
					monthTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
					monthTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					monthTable->SetRowsAndColumns(1, 2);
					monthTable->SetRowOption(0, GuiCellOption::MinSizeOption());
					monthTable->SetColumnOption(0, GuiCellOption::PercentageOption(0.5));
					monthTable->SetColumnOption(1, GuiCellOption::PercentageOption(0.5));
					{
						GuiCellComposition* cell = new GuiCellComposition;
						monthTable->AddChild(cell);
						cell->SetSite(0, 0, 1, 1);
						cell->AddChild(comboYear->GetBoundsComposition());
					}
					{
						GuiCellComposition* cell = new GuiCellComposition;
						monthTable->AddChild(cell);
						cell->SetSite(0, 1, 1, 1);
						cell->AddChild(comboMonth->GetBoundsComposition());
					}
				}
				{
					dayTable = new GuiTableComposition;
					dayTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					dayTable->SetCellPadding(4);
					dayTable->SetRowsAndColumns(DayRows + DayRowStart, DaysOfWeek);

					for (vint i = 0; i < DayRowStart; i++)
					{
						dayTable->SetRowOption(i, GuiCellOption::MinSizeOption());
					}
					for (vint i = 0; i < DayRows; i++)
					{
						dayTable->SetRowOption(i + DayRowStart, GuiCellOption::PercentageOption(1.0));
					}
					for (vint i = 0; i < DaysOfWeek; i++)
					{
						dayTable->SetColumnOption(i, GuiCellOption::PercentageOption(1.0));
					}

					{
						GuiCellComposition* cell = new GuiCellComposition;
						dayTable->AddChild(cell);
						cell->SetSite(0, 0, 1, DaysOfWeek);
						cell->AddChild(monthTable);
					}

					labelDaysOfWeek.Resize(7);
					for (vint i = 0; i < DaysOfWeek; i++)
					{
						GuiCellComposition* cell = new GuiCellComposition;
						dayTable->AddChild(cell);
						cell->SetSite(1, i, 1, 1);

						GuiSolidLabelElement* element = GuiSolidLabelElement::Create();
						element->SetAlignments(Alignment::Center, Alignment::Center);
						element->SetColor(primaryTextColor);
						labelDaysOfWeek[i] = element;
						cell->SetOwnedElement(element);
					}

					buttonDays.Resize(DaysOfWeek*DayRows);
					labelDays.Resize(DaysOfWeek*DayRows);
					dateDays.Resize(DaysOfWeek*DayRows);

					auto dayMutexController = new GuiSelectableButton::MutexGroupController;
					AddComponent(dayMutexController);

					for (vint i = 0; i < DaysOfWeek; i++)
					{
						for (vint j = 0; j < DayRows; j++)
						{
							GuiCellComposition* cell = new GuiCellComposition;
							dayTable->AddChild(cell);
							cell->SetSite(j + DayRowStart, i, 1, 1);

							GuiSelectableButton* button = new GuiSelectableButton(theme::ThemeName::CheckBox);
							button->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
							button->SetGroupController(dayMutexController);
							button->SelectedChanged.AttachMethod(this, &GuiCommonDatePickerLook::buttonDay_SelectedChanged);
							cell->AddChild(button->GetBoundsComposition());
							buttonDays[j*DaysOfWeek + i] = button;

							GuiSolidLabelElement* element = GuiSolidLabelElement::Create();
							element->SetAlignments(Alignment::Center, Alignment::Center);
							element->SetText(L"0");
							labelDays[j*DaysOfWeek + i] = element;

							GuiBoundsComposition* elementBounds = new GuiBoundsComposition;
							elementBounds->SetOwnedElement(element);
							elementBounds->SetAlignmentToParent(Margin(0, 0, 0, 0));
							elementBounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
							button->GetContainerComposition()->AddChild(elementBounds);
						}
					}
				}
				{
					GuiSolidBackgroundElement* element = GuiSolidBackgroundElement::Create();
					element->SetColor(backgroundColor);
					dayTable->SetOwnedElement(element);
				}

				dayTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
				AddChild(dayTable);

				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				SetFont(font);
			}

			GuiCommonDatePickerLook::~GuiCommonDatePickerLook()
			{
				FinalizeInstanceRecursively(this);
			}

			controls::IDatePickerCommandExecutor* GuiCommonDatePickerLook::GetCommands()
			{
				return commands;
			}

			void GuiCommonDatePickerLook::SetCommands(controls::IDatePickerCommandExecutor* value)
			{
				commands = value;
			}

			TemplateProperty<GuiSelectableButtonTemplate> GuiCommonDatePickerLook::GetDateButtonTemplate()
			{
				return dateButtonTemplate;
			}

			void GuiCommonDatePickerLook::SetDateButtonTemplate(const TemplateProperty<GuiSelectableButtonTemplate>& value)
			{
				dateButtonTemplate = value;
				FOREACH(GuiSelectableButton*, button, buttonDays)
				{
					button->SetControlTemplate(value);
				}
			}

			TemplateProperty<GuiTextListTemplate> GuiCommonDatePickerLook::GetDateTextListTemplate()
			{
				return dateTextListTemplate;
			}

			void GuiCommonDatePickerLook::SetDateTextListTemplate(const TemplateProperty<GuiTextListTemplate>& value)
			{
				dateTextListTemplate = value;
				listYears->SetControlTemplate(value);
				listMonths->SetControlTemplate(value);
			}

			TemplateProperty<GuiComboBoxTemplate> GuiCommonDatePickerLook::GetDateComboBoxTemplate()
			{
				return dateComboBoxTemplate;
			}

			void GuiCommonDatePickerLook::SetDateComboBoxTemplate(const TemplateProperty<GuiComboBoxTemplate>& value)
			{
				dateComboBoxTemplate = value;
				comboYear->SetControlTemplate(value);
				comboMonth->SetControlTemplate(value);
			}

			const Locale& GuiCommonDatePickerLook::GetDateLocale()
			{
				return dateLocale;
			}

			void GuiCommonDatePickerLook::SetDateLocale(const Locale& value)
			{
				if (dateLocale != value)
				{
					dateLocale = value;
					for (vint i = 0; i < DaysOfWeek; i++)
					{
						labelDaysOfWeek[i]->SetText(dateLocale.GetShortDayOfWeekName(i));
					}

					listMonths->GetItems().Clear();
					for (vint i = 1; i <= 12; i++)
					{
						listMonths->GetItems().Add(new list::TextItem(dateLocale.GetLongMonthName(i)));
					}

					SetDate(currentDate);
				}
			}

			const DateTime& GuiCommonDatePickerLook::GetDate()
			{
				return currentDate;
			}

			void GuiCommonDatePickerLook::SetDate(const DateTime& value)
			{
				currentDate = value;
				DisplayMonth(value.year, value.month);
				SelectDay(value.day);
			}

			const FontProperties& GuiCommonDatePickerLook::GetFont()
			{
				return font;
			}

			void GuiCommonDatePickerLook::SetFont(const FontProperties& value)
			{
				if (font != value)
				{
					font = value;
					comboYear->SetFont(value);
					listYears->SetFont(value);
					comboMonth->SetFont(value);
					listMonths->SetFont(value);
					FOREACH(GuiSolidLabelElement*, label, From(labelDaysOfWeek).Concat(labelDays))
					{
						label->SetFont(value);
					}
				}
			}

/***********************************************************************
GuiCommonDatePickerLook
***********************************************************************/

			void GuiCommonScrollViewLook::UpdateTable()
			{
				if (horizontalScroll->GetVisible())
				{
					tableComposition->SetRowOption(1, GuiCellOption::AbsoluteOption(defaultScrollSize));
				}
				else
				{
					tableComposition->SetRowOption(1, GuiCellOption::AbsoluteOption(0));
				}

				if (verticalScroll->GetVisible())
				{
					tableComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(defaultScrollSize));
				}
				else
				{
					tableComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(0));
				}

				tableComposition->UpdateCellBounds();
			}

			void GuiCommonScrollViewLook::hScroll_OnVisibleChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				UpdateTable();
			}

			void GuiCommonScrollViewLook::vScroll_OnVisibleChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				UpdateTable();
			}

			GuiCommonScrollViewLook::GuiCommonScrollViewLook(vint _defaultScrollSize)
				:defaultScrollSize(_defaultScrollSize)
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

				horizontalScroll = new GuiScroll(theme::ThemeName::HScroll);
				horizontalScroll->SetControlTemplate(GetHScrollTemplate());
				horizontalScroll->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				horizontalScroll->SetEnabled(false);
				verticalScroll = new GuiScroll(theme::ThemeName::HScroll);
				verticalScroll->SetControlTemplate(GetVScrollTemplate());
				verticalScroll->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				verticalScroll->SetEnabled(false);

				tableComposition = new GuiTableComposition;
				AddChild(tableComposition);
				tableComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				tableComposition->SetRowsAndColumns(2, 2);
				tableComposition->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
				tableComposition->SetRowOption(1, GuiCellOption::MinSizeOption());
				tableComposition->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
				tableComposition->SetColumnOption(1, GuiCellOption::MinSizeOption());
				UpdateTable();
				{
					GuiCellComposition* cell = new GuiCellComposition;
					tableComposition->AddChild(cell);
					cell->SetSite(1, 0, 1, 1);
					cell->AddChild(horizontalScroll->GetBoundsComposition());
				}
				{
					GuiCellComposition* cell = new GuiCellComposition;
					tableComposition->AddChild(cell);
					cell->SetSite(0, 1, 1, 1);
					cell->AddChild(verticalScroll->GetBoundsComposition());
				}

				containerCellComposition = new GuiCellComposition;
				tableComposition->AddChild(containerCellComposition);
				containerCellComposition->SetSite(0, 0, 1, 1);

				containerComposition = new GuiBoundsComposition;
				containerComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				containerCellComposition->AddChild(containerComposition);

				horizontalScroll->VisibleChanged.AttachMethod(this, &GuiCommonScrollViewLook::hScroll_OnVisibleChanged);
				verticalScroll->VisibleChanged.AttachMethod(this, &GuiCommonScrollViewLook::vScroll_OnVisibleChanged);
				UpdateTable();
			}

			GuiCommonScrollViewLook::~GuiCommonScrollViewLook()
			{
			}

			controls::GuiScroll* GuiCommonScrollViewLook::GetHScroll()
			{
				return horizontalScroll;
			}

			controls::GuiScroll* GuiCommonScrollViewLook::GetVScroll()
			{
				return verticalScroll;
			}

			compositions::GuiGraphicsComposition* GuiCommonScrollViewLook::GetContainerComposition()
			{
				return containerComposition;
			}

			controls::IScrollViewCommandExecutor* GuiCommonScrollViewLook::GetCommands()
			{
				return commands;
			}

			void GuiCommonScrollViewLook::SetCommands(controls::IScrollViewCommandExecutor* value)
			{
				commands = value;
			}

			TemplateProperty<GuiScrollTemplate> GuiCommonScrollViewLook::GetHScrollTemplate()
			{
				return hScrollTemplate;
			}

			void GuiCommonScrollViewLook::SetHScrollTemplate(const TemplateProperty<GuiScrollTemplate>& value)
			{
				hScrollTemplate = value;
				horizontalScroll->SetControlTemplate(value);
			}

			TemplateProperty<GuiScrollTemplate> GuiCommonScrollViewLook::GetVScrollTemplate()
			{
				return vScrollTemplate;
			}

			void GuiCommonScrollViewLook::SetVScrollTemplate(const TemplateProperty<GuiScrollTemplate>& value)
			{
				vScrollTemplate = value;
				verticalScroll->SetControlTemplate(value);
			}
		}
	}
}