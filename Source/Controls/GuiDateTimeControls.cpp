#include "GuiDateTimeControls.h"
#include "Templates/GuiThemeStyleFactory.h"

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
GuiDatePicker::CommandExecutor
***********************************************************************/

			GuiDatePicker::CommandExecutor::CommandExecutor(GuiDatePicker* _datePicker)
				:datePicker(_datePicker)
			{
			}

			GuiDatePicker::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiDatePicker::CommandExecutor::NotifyDateChanged()
			{
				datePicker->date = datePicker->GetControlTemplateObject(true)->GetDate();
				datePicker->UpdateText();
				datePicker->DateChanged.Execute(datePicker->GetNotifyEventArguments());
			}

			void GuiDatePicker::CommandExecutor::NotifyDateNavigated()
			{
				datePicker->DateNavigated.Execute(datePicker->GetNotifyEventArguments());
			}

			void GuiDatePicker::CommandExecutor::NotifyDateSelected()
			{
				datePicker->DateSelected.Execute(datePicker->GetNotifyEventArguments());
			}

/***********************************************************************
GuiDatePicker
***********************************************************************/

			void GuiDatePicker::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject(false);
				if (!ct) return;

				ct->SetCommands(nullptr);
			}

			void GuiDatePicker::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				ct->SetCommands(commandExecutor.Obj());
				ct->SetDate(date);
				ct->SetDateLocale(dateLocale);
				UpdateText();
			}

			void GuiDatePicker::UpdateText()
			{
				GuiControl::SetText(dateLocale.FormatDate(dateFormat, date));
			}

			bool GuiDatePicker::IsAltAvailable()
			{
				if (nestedAlt)
				{
					return alt != L"";
				}
				else
				{
					return GuiControl::IsAltAvailable();
				}
			}

			compositions::IGuiAltActionHost* GuiDatePicker::GetActivatingAltHost()
			{
				if (nestedAlt)
				{
					return this;
				}
				else
				{
					return GuiControl::GetActivatingAltHost();
				}
			}

			GuiDatePicker::GuiDatePicker(theme::ThemeName themeName, bool _nestedAlt)
				:GuiControl(themeName)
				, nestedAlt(_nestedAlt)
			{
				commandExecutor = new CommandExecutor(this);
				SetDateLocale(Locale::UserDefault());
				SetDate(DateTime::LocalTime());
				SetAltComposition(boundsComposition);
				SetAltControl(this, false);

				DateChanged.SetAssociatedComposition(boundsComposition);
				DateNavigated.SetAssociatedComposition(boundsComposition);
				DateSelected.SetAssociatedComposition(boundsComposition);
				DateFormatChanged.SetAssociatedComposition(boundsComposition);
				DateLocaleChanged.SetAssociatedComposition(boundsComposition);

				commandExecutor->NotifyDateChanged();
			}

			GuiDatePicker::~GuiDatePicker()
			{
			}

			const DateTime& GuiDatePicker::GetDate()
			{
				return date;
			}

			void GuiDatePicker::SetDate(const DateTime& value)
			{
				if (date != value)
				{
					date = value;
					GetControlTemplateObject(true)->SetDate(value);
				}
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
				GetControlTemplateObject(true)->SetDateLocale(dateLocale);

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

			void GuiDateComboBox::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiDateComboBox::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				datePicker->SetControlTemplate(ct->GetDatePickerTemplate());
			}

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
				NotifyUpdateSelectedDate();
			}

			GuiDateComboBox::GuiDateComboBox(theme::ThemeName themeName)
				:GuiComboBoxBase(themeName)
			{
				SelectedDateChanged.SetAssociatedComposition(GetBoundsComposition());
				
				datePicker = new GuiDatePicker(theme::ThemeName::DatePicker, false);
				datePicker->DateSelected.AttachMethod(this, &GuiDateComboBox::datePicker_DateSelected);
				datePicker->DateLocaleChanged.AttachMethod(this, &GuiDateComboBox::datePicker_DateLocaleChanged);
				datePicker->DateFormatChanged.AttachMethod(this, &GuiDateComboBox::datePicker_DateFormatChanged);
				datePicker->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));

				GetSubMenu()->GetContainerComposition()->AddChild(datePicker->GetBoundsComposition());
				GetSubMenu()->SetHideOnDeactivateAltHost(false);

				selectedDate=datePicker->GetDate();
				SubMenuOpeningChanged.AttachMethod(this, &GuiDateComboBox::OnSubMenuOpeningChanged);
				SetFont(GetFont());
				SetText(datePicker->GetText());
			}

			GuiDateComboBox::~GuiDateComboBox()
			{
			}

			void GuiDateComboBox::SetFont(const Nullable<FontProperties>& value)
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