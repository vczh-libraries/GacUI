#include "GuiDateTimeControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace compositions;
			using namespace elements;

/***********************************************************************
GuiDatePicker
***********************************************************************/

			void GuiDatePicker::UpdateText()
			{
				GuiControl::SetText(dateLocale.FormatDate(dateFormat, styleController->GetDate()));
			}

			void GuiDatePicker::NotifyDateChanged()
			{
				UpdateText();
				DateChanged.Execute(GetNotifyEventArguments());
			}

			void GuiDatePicker::NotifyDateNavigated()
			{
				DateNavigated.Execute(GetNotifyEventArguments());
			}

			void GuiDatePicker::NotifyDateSelected()
			{
				DateSelected.Execute(GetNotifyEventArguments());
			}

			GuiDatePicker::GuiDatePicker(IStyleController* _styleController)
				:GuiControl(_styleController)
				, styleController(_styleController)
			{
				styleController->SetCommandExecutor(this);
				SetDateLocale(Locale::UserDefault());
				SetDate(DateTime::LocalTime());

				DateChanged.SetAssociatedComposition(GetBoundsComposition());
				DateNavigated.SetAssociatedComposition(GetBoundsComposition());
				DateSelected.SetAssociatedComposition(GetBoundsComposition());
				DateFormatChanged.SetAssociatedComposition(GetBoundsComposition());
				DateLocaleChanged.SetAssociatedComposition(GetBoundsComposition());

				NotifyDateChanged();
			}

			GuiDatePicker::~GuiDatePicker()
			{
			}

			const DateTime& GuiDatePicker::GetDate()
			{
				return styleController->GetDate();
			}

			void GuiDatePicker::SetDate(const DateTime& value)
			{
				styleController->SetDate(value);
			}

			const WString& GuiDatePicker::GetDateFormat()
			{
				return dateFormat;
			}

			void GuiDatePicker::SetDateFormat(const WString& value)
			{
				dateFormat=value;
				UpdateText();
				DateFormatChanged.Execute(GetNotifyEventArguments());
			}

			const Locale& GuiDatePicker::GetDateLocale()
			{
				return dateLocale;
			}

			void GuiDatePicker::SetDateLocale(const Locale& value)
			{
				dateLocale=value;
				List<WString> formats;
				dateLocale.GetLongDateFormats(formats);
				if(formats.Count()>0)
				{
					dateFormat=formats[0];
				}
				styleController->SetDateLocale(dateLocale);

				UpdateText();
				DateFormatChanged.Execute(GetNotifyEventArguments());
				DateLocaleChanged.Execute(GetNotifyEventArguments());
			}

			void GuiDatePicker::SetText(const WString& value)
			{
			}

/***********************************************************************
GuiDateComboBox
***********************************************************************/

			void GuiDateComboBox::UpdateText()
			{
				SetText(datePicker->GetDateLocale().FormatDate(datePicker->GetDateFormat(), selectedDate));
			}

			void GuiDateComboBox::NotifyUpdateSelectedDate()
			{
				UpdateText();
				SelectedDateChanged.Execute(GetNotifyEventArguments());
			}

			void GuiDateComboBox::OnSubMenuOpeningChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				datePicker->SetDate(selectedDate);
			}

			void GuiDateComboBox::datePicker_DateLocaleChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				UpdateText();
			}

			void GuiDateComboBox::datePicker_DateFormatChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				UpdateText();
			}

			void GuiDateComboBox::datePicker_DateSelected(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				selectedDate=datePicker->GetDate();
				GetSubMenu()->Hide();
				SelectItem();
				NotifyUpdateSelectedDate();
			}

			GuiDateComboBox::GuiDateComboBox(IStyleController* _styleController, GuiDatePicker* _datePicker)
				:GuiComboBoxBase(_styleController)
				,datePicker(_datePicker)
			{
				SelectedDateChanged.SetAssociatedComposition(GetBoundsComposition());

				datePicker->DateSelected.AttachMethod(this, &GuiDateComboBox::datePicker_DateSelected);
				datePicker->DateLocaleChanged.AttachMethod(this, &GuiDateComboBox::datePicker_DateLocaleChanged);
				datePicker->DateFormatChanged.AttachMethod(this, &GuiDateComboBox::datePicker_DateFormatChanged);
				datePicker->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				GetSubMenu()->GetContainerComposition()->AddChild(datePicker->GetBoundsComposition());

				selectedDate=datePicker->GetDate();
				SubMenuOpeningChanged.AttachMethod(this, &GuiDateComboBox::OnSubMenuOpeningChanged);
				SetFont(GetFont());
				SetText(datePicker->GetText());
			}

			GuiDateComboBox::~GuiDateComboBox()
			{
			}

			void GuiDateComboBox::SetFont(const FontProperties& value)
			{
				GuiComboBoxBase::SetFont(value);
				datePicker->SetFont(value);
			}

			const DateTime& GuiDateComboBox::GetSelectedDate()
			{
				return selectedDate;
			}

			void GuiDateComboBox::SetSelectedDate(const DateTime& value)
			{
				selectedDate=value;
				NotifyUpdateSelectedDate();
			}

			GuiDatePicker* GuiDateComboBox::GetDatePicker()
			{
				return datePicker;
			}
		}
	}
}