#include "GuiControlTemplates.h"
#include "GuiControlTemplateStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace reflection::description;

/***********************************************************************
GuiComponent
***********************************************************************/
			
			GuiComponent::GuiComponent()
			{
			}

			GuiComponent::~GuiComponent()
			{
			}

			void GuiComponent::Attach(GuiInstanceRootObject* rootObject)
			{
			}

			void GuiComponent::Detach(GuiInstanceRootObject* rootObject)
			{
			}

/***********************************************************************
GuiInstanceRootObject
***********************************************************************/

			GuiInstanceRootObject::GuiInstanceRootObject()
			{
			}

			GuiInstanceRootObject::~GuiInstanceRootObject()
			{
			}

			void GuiInstanceRootObject::FinalizeInstance()
			{
				if (!finalized)
				{
					finalized = true;

					FOREACH(Ptr<IValueSubscription>, subscription, subscriptions)
					{
						subscription->Close();
					}
					FOREACH(GuiComponent*, component, components)
					{
						component->Detach(this);
					}

					subscriptions.Clear();
					for (vint i = 0; i<components.Count(); i++)
					{
						delete components[i];
					}
					components.Clear();
				}
			}

			bool GuiInstanceRootObject::IsFinalized()
			{
				return finalized;
			}

			void GuiInstanceRootObject::FinalizeInstanceRecursively(templates::GuiTemplate* thisObject)
			{
				if (!finalized)
				{
					NotifyFinalizeInstance(thisObject);
				}
			}

			void GuiInstanceRootObject::FinalizeInstanceRecursively(GuiCustomControl* thisObject)
			{
				if (!finalized)
				{
					NotifyFinalizeInstance(thisObject);
				}
			}

			void GuiInstanceRootObject::FinalizeInstanceRecursively(GuiControlHost* thisObject)
			{
				if (!finalized)
				{
					NotifyFinalizeInstance(thisObject);
				}
			}

			void GuiInstanceRootObject::FinalizeGeneralInstance(GuiInstanceRootObject* thisObject)
			{
			}

			void GuiInstanceRootObject::SetResourceResolver(Ptr<GuiResourcePathResolver> resolver)
			{
				resourceResolver = resolver;
			}

			Ptr<DescriptableObject> GuiInstanceRootObject::ResolveResource(const WString& protocol, const WString& path, bool ensureExist)
			{
				Ptr<DescriptableObject> object;
				if (resourceResolver)
				{
					object = resourceResolver->ResolveResource(protocol, path);
				}
				if (ensureExist && !object)
				{
					throw ArgumentException(L"Resource \"" + protocol + L"://" + path + L"\" does not exist.");
				}
				return object;
			}

			Ptr<description::IValueSubscription> GuiInstanceRootObject::AddSubscription(Ptr<description::IValueSubscription> subscription)
			{
				CHECK_ERROR(finalized == false, L"GuiInstanceRootObject::AddSubscription(Ptr<IValueSubscription>)#Cannot add subscription after finalizing.");
				if (subscriptions.Contains(subscription.Obj()))
				{
					return nullptr;
				}
				else
				{
					subscriptions.Add(subscription);
					subscription->Open();
					subscription->Update();
					return subscription;
				}
			}

			void GuiInstanceRootObject::UpdateSubscriptions()
			{
				FOREACH(Ptr<IValueSubscription>, subscription, subscriptions)
				{
					subscription->Update();
				}
			}

			bool GuiInstanceRootObject::AddComponent(GuiComponent* component)
			{
				CHECK_ERROR(finalized == false, L"GuiInstanceRootObject::AddComponent(GuiComponent*>)#Cannot add component after finalizing.");
				if(components.Contains(component))
				{
					return false;
				}
				else
				{
					components.Add(component);
					component->Attach(this);
					return true;
				}
			}

			bool GuiInstanceRootObject::AddControlHostComponent(GuiControlHost* controlHost)
			{
				return AddComponent(new GuiObjectComponent<GuiControlHost>(controlHost));
			}
		}
		namespace templates
		{
			using namespace collections;
			using namespace controls;
			using namespace compositions;
			using namespace elements;

/***********************************************************************
GuiTemplate
***********************************************************************/

			GuiTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiTemplate::GuiTemplate()
				:VisuallyEnabled_(true)
			{
				GuiTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiTemplate::~GuiTemplate()
			{
				FinalizeInstanceRecursively(this);
			}

/***********************************************************************
GuiControlTemplate
***********************************************************************/

			GuiControlTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiControlTemplate::GuiControlTemplate()
				:ContainerComposition_(this)
				, FocusableComposition_(0)
			{
				GuiControlTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiControlTemplate::~GuiControlTemplate()
			{
				FinalizeAggregation();
			}

			void GuiControlTemplate::Initialize()
			{
			}

/***********************************************************************
GuiLabelTemplate
***********************************************************************/

			GuiLabelTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiLabelTemplate::GuiLabelTemplate()
			{
				GuiLabelTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiLabelTemplate::~GuiLabelTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiSinglelineTextBoxTemplate
***********************************************************************/

			void GuiSinglelineTextBoxTemplate::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				Commands_->UnsafeSetText(GetText());
				textElement->SetCaretBegin(TextPos(0, 0));
				textElement->SetCaretEnd(TextPos(0, 0));
			}

			void GuiSinglelineTextBoxTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				textElement->SetFont(GetFont());
			}

			void GuiSinglelineTextBoxTemplate::OnVisuallyEnabledChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				textElement->SetVisuallyEnabled(GetVisuallyEnabled());
			}

			GuiSinglelineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiSinglelineTextBoxTemplate::GuiSinglelineTextBoxTemplate()
			{
				TextChanged.AttachMethod(this, &GuiSinglelineTextBoxTemplate::OnTextChanged);
				FontChanged.AttachMethod(this, &GuiSinglelineTextBoxTemplate::OnFontChanged);
				VisuallyEnabledChanged.AttachMethod(this, &GuiSinglelineTextBoxTemplate::OnVisuallyEnabledChanged);
				GuiSinglelineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiSinglelineTextBoxTemplate::~GuiSinglelineTextBoxTemplate()
			{
				FinalizeAggregation();
			}

			WString GuiSinglelineTextBoxTemplate::GetEditingText()
			{
				return textElement->GetLines().GetText();
			}

			elements::GuiColorizedTextElement* GuiSinglelineTextBoxTemplate::GetTextElement()
			{
				return textElement;
			}

			compositions::GuiGraphicsComposition* GuiSinglelineTextBoxTemplate::GetTextComposition()
			{
				return textComposition;
			}

			void GuiSinglelineTextBoxTemplate::RearrangeTextElement()
			{
				textCompositionTable->SetRowOption(1, GuiCellOption::AbsoluteOption(textElement->GetLines().GetRowHeight() + 2 * GuiSinglelineTextBox::TextMargin));
			}

			void GuiSinglelineTextBoxTemplate::Initialize()
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

				textElement = GuiColorizedTextElement::Create();
				textElement->SetViewPosition(Point(-GuiSinglelineTextBox::TextMargin, -GuiSinglelineTextBox::TextMargin));

				textCompositionTable = new GuiTableComposition;
				textCompositionTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				textCompositionTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
				textCompositionTable->SetRowsAndColumns(3, 1);
				textCompositionTable->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
				textCompositionTable->SetRowOption(1, GuiCellOption::AbsoluteOption(0));
				textCompositionTable->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
				textCompositionTable->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
				AddChild(textCompositionTable);

				textComposition = new GuiCellComposition;
				textComposition->SetOwnedElement(textElement);
				textCompositionTable->AddChild(textComposition);
				textComposition->SetSite(1, 0, 1, 1);
			}

/***********************************************************************
GuiDocumentLabelTemplate
***********************************************************************/

			GuiDocumentLabelTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiDocumentLabelTemplate::GuiDocumentLabelTemplate()
			{
				GuiDocumentLabelTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiDocumentLabelTemplate::~GuiDocumentLabelTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiMenuTemplate
***********************************************************************/

			GuiMenuTemplate::GuiMenuTemplate()
			{
			}

			GuiMenuTemplate::~GuiMenuTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiWindowTemplate
***********************************************************************/

			GuiWindowTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiWindowTemplate::GuiWindowTemplate()
				:MaximizedBoxOption_(BoolOption::Customizable)
				, MinimizedBoxOption_(BoolOption::Customizable)
				, BorderOption_(BoolOption::Customizable)
				, SizeBoxOption_(BoolOption::Customizable)
				, IconVisibleOption_(BoolOption::Customizable)
				, TitleBarOption_(BoolOption::Customizable)
				, MaximizedBox_(true)
				, MinimizedBox_(true)
				, Border_(true)
				, SizeBox_(true)
				, IconVisible_(true)
				, TitleBar_(true)
				, CustomizedBorder_(false)
				, Maximized_(false)
			{
				GuiWindowTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiWindowTemplate::~GuiWindowTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiButtonTemplate
***********************************************************************/

			GuiButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiButtonTemplate::GuiButtonTemplate()
				:State_(ButtonState::Normal)
			{
				GuiButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiButtonTemplate::~GuiButtonTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiSelectableButtonTemplate
***********************************************************************/

			GuiSelectableButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiSelectableButtonTemplate::GuiSelectableButtonTemplate()
				:Selected_(false)
			{
				GuiSelectableButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiSelectableButtonTemplate::~GuiSelectableButtonTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiToolstripButtonTemplate
***********************************************************************/

			GuiToolstripButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiToolstripButtonTemplate::GuiToolstripButtonTemplate()
				:SubMenuExisting_(false)
				, SubMenuOpening_(false)
				, SubMenuHost_(0)
			{
				GuiToolstripButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiToolstripButtonTemplate::~GuiToolstripButtonTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiListViewColumnHeaderTemplate
***********************************************************************/

			GuiListViewColumnHeaderTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiListViewColumnHeaderTemplate::GuiListViewColumnHeaderTemplate()
				:SortingState_(ColumnSortingState::NotSorted)
			{
				GuiListViewColumnHeaderTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiListViewColumnHeaderTemplate::~GuiListViewColumnHeaderTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiComboBoxTemplate
***********************************************************************/

			GuiComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiComboBoxTemplate::GuiComboBoxTemplate()
				:Commands_(0)
				, TextVisible_(true)
			{
				GuiComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiComboBoxTemplate::~GuiComboBoxTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiScrollTemplate
***********************************************************************/

			GuiScrollTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiScrollTemplate::GuiScrollTemplate()
				:Commands_(0)
				, TotalSize_(100)
				, PageSize_(10)
				, Position_(0)
			{
				GuiScrollTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiScrollTemplate::~GuiScrollTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiScrollViewTemplate
***********************************************************************/

			GuiScrollViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiScrollViewTemplate::GuiScrollViewTemplate()
				:DefaultScrollSize_(0)
			{
				GuiScrollViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiScrollViewTemplate::~GuiScrollViewTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiMultilineTextBoxTemplate
***********************************************************************/

			GuiMultilineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiMultilineTextBoxTemplate::GuiMultilineTextBoxTemplate()
			{
				GuiMultilineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiMultilineTextBoxTemplate::~GuiMultilineTextBoxTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiTextListTemplate
***********************************************************************/

			GuiTextListTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiTextListTemplate::GuiTextListTemplate()
			{
				GuiTextListTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiTextListTemplate::~GuiTextListTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiDocumentViewerTemplate
***********************************************************************/

			GuiDocumentViewerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiDocumentViewerTemplate::GuiDocumentViewerTemplate()
			{
				GuiDocumentViewerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiDocumentViewerTemplate::~GuiDocumentViewerTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiListViewTemplate
***********************************************************************/

			GuiListViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiListViewTemplate::GuiListViewTemplate()
			{
				GuiListViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiListViewTemplate::~GuiListViewTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiTreeViewTemplate
***********************************************************************/

			GuiTreeViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiTreeViewTemplate::GuiTreeViewTemplate()
			{
				GuiTreeViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiTreeViewTemplate::~GuiTreeViewTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiTabTemplate
***********************************************************************/

			GuiTabTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiTabTemplate::GuiTabTemplate()
				:SelectedTabPage_(nullptr)
			{
				GuiTabTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiTabTemplate::~GuiTabTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiDatePickerTemplate
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

			void GuiDatePickerTemplate::SetDay(const DateTime& day, vint& index, bool currentMonth)
			{
				dateDays[index] = day;
				GuiSolidLabelElement* label = labelDays[index];
				label->SetText(itow(day.day));
				label->SetColor(currentMonth ? GetPrimaryTextColor() : GetSecondaryTextColor());
				index++;
			}

			void GuiDatePickerTemplate::comboYearMonth_SelectedIndexChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (!preventComboEvent)
				{
					if (comboYear->GetSelectedIndex() != -1 && comboMonth->GetSelectedIndex() != -1)
					{
						vint year = comboYear->GetSelectedIndex() + YearFirst;
						vint month = comboMonth->GetSelectedIndex() + 1;
						SetDate(DateTime::FromDateTime(year, month, 1));
						Commands_->NotifyDateChanged();
						Commands_->NotifyDateNavigated();
					}
				}
			}

			void GuiDatePickerTemplate::buttonDay_SelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
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
							Commands_->NotifyDateChanged();
							Commands_->NotifyDateSelected();
						}
					}
				}
			}

			void GuiDatePickerTemplate::DisplayMonth(vint year, vint month)
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

			void GuiDatePickerTemplate::SelectDay(vint day)
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

			void GuiDatePickerTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				auto value = GetFont();
				comboYear->SetFont(value);
				listYears->SetFont(value);
				comboMonth->SetFont(value);
				listMonths->SetFont(value);
				FOREACH(GuiSolidLabelElement*, label, From(labelDaysOfWeek).Concat(labelDays))
				{
					label->SetFont(value);
				}
			}

			void GuiDatePickerTemplate::UpdateData(const DateTime& value, bool forceUpdate)
			{
				bool dateChanged = currentDate.year != value.year || currentDate.month != value.month || currentDate.day != value.day;

				if (forceUpdate || dateChanged)
				{
					currentDate = value;
					DisplayMonth(value.year, value.month);
					SelectDay(value.day);
				}

				if (dateChanged)
				{
					GuiEventArgs arguments(this);
					DateChanged.Execute(arguments);
				}
			}

			GuiDatePickerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiDatePickerTemplate::GuiDatePickerTemplate()
				:Commands_(nullptr)
				, preventComboEvent(false)
				, preventButtonEvent(false)
			{
				CommandsChanged.SetAssociatedComposition(this);
				DateLocaleChanged.SetAssociatedComposition(this);
				DateChanged.SetAssociatedComposition(this);
				GuiDatePickerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiDatePickerTemplate::~GuiDatePickerTemplate()
			{
				FinalizeAggregation();
			}

			const Locale& GuiDatePickerTemplate::GetDateLocale()
			{
				return dateLocale;
			}

			void GuiDatePickerTemplate::SetDateLocale(const Locale& value)
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

					UpdateData(currentDate, true);

					GuiEventArgs arguments(this);
					DateLocaleChanged.Execute(arguments);
				}
			}

			const DateTime& GuiDatePickerTemplate::GetDate()
			{
				return currentDate;
			}

			void GuiDatePickerTemplate::SetDate(const DateTime& value)
			{
				UpdateData(value, false);
			}

			void GuiDatePickerTemplate::Initialize()
			{
				GuiTableComposition* monthTable = 0;
				GuiTableComposition* dayTable = 0;
				{
					listYears = new GuiTextList(new GuiTextListTemplate_StyleProvider(GetDateTextListTemplate()));
					listYears->SetHorizontalAlwaysVisible(false);
					for (vint i = YearFirst; i <= YearLast; i++)
					{
						listYears->GetItems().Add(new list::TextItem(itow(i)));
					}
					comboYear = new GuiComboBoxListControl(new GuiComboBoxTemplate_StyleProvider(GetDateComboBoxTemplate()), listYears);
					comboYear->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 2, 0));
					comboYear->SelectedIndexChanged.AttachMethod(this, &GuiDatePickerTemplate::comboYearMonth_SelectedIndexChanged);
				}
				{
					listMonths = new GuiTextList(new GuiTextListTemplate_StyleProvider(GetDateTextListTemplate()));
					listMonths->SetHorizontalAlwaysVisible(false);
					comboMonth = new GuiComboBoxListControl(new GuiComboBoxTemplate_StyleProvider(GetDateComboBoxTemplate()), listMonths);
					comboMonth->GetBoundsComposition()->SetAlignmentToParent(Margin(2, 0, 0, 0));
					comboMonth->SelectedIndexChanged.AttachMethod(this, &GuiDatePickerTemplate::comboYearMonth_SelectedIndexChanged);
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
						element->SetColor(GetPrimaryTextColor());
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

							GuiSelectableButton* button = new GuiSelectableButton(new GuiSelectableButtonTemplate_StyleProvider(GetDateButtonTemplate()));
							button->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
							button->SetGroupController(dayMutexController);
							button->SelectedChanged.AttachMethod(this, &GuiDatePickerTemplate::buttonDay_SelectedChanged);
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
					element->SetColor(GetBackgroundColor());
					dayTable->SetOwnedElement(element);
				}

				dayTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
				AddChild(dayTable);

				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				SetContainerComposition(this);
			}

/***********************************************************************
GuiDateComboBoxTemplate
***********************************************************************/

			GuiDateComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

				GuiDateComboBoxTemplate::GuiDateComboBoxTemplate()
			{
				GuiDateComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiDateComboBoxTemplate::~GuiDateComboBoxTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiListItemTemplate
***********************************************************************/

			void GuiListItemTemplate::OnInitialize()
			{
				CHECK_ERROR(initialized == false, L"GuiListItemTemplate::OnInitialize()#Cannot initialize a GuiListItemTemplate twice.");
			}

			GuiListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiListItemTemplate::GuiListItemTemplate()
				:Selected_(false)
				, Index_(0)
			{
				GuiListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiListItemTemplate::~GuiListItemTemplate()
			{
				FinalizeAggregation();
			}

			void GuiListItemTemplate::BeginEditListItem()
			{
				listControl->GetItemProvider()->PushEditing();
			}

			void GuiListItemTemplate::EndEditListItem()
			{
				CHECK_ERROR(listControl->GetItemProvider()->PopEditing(), L"GuiListItemTemplate::EndEditListItem()#BeginEditListItem and EndEditListItem calls are not paired.");
			}

			void GuiListItemTemplate::Initialize(controls::GuiListControl* _listControl)
			{
				CHECK_ERROR(listControl == nullptr, L"GuiListItemTemplate::Initialize(GuiListControl*)#This function can only be called once.");
				listControl = _listControl;
				OnInitialize();
			}

/***********************************************************************
GuiTextListItemTemplate
***********************************************************************/

			GuiTextListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiTextListItemTemplate::GuiTextListItemTemplate()
			{
				GuiTextListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiTextListItemTemplate::~GuiTextListItemTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiTreeItemTemplate
***********************************************************************/

			GuiTreeItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiTreeItemTemplate::GuiTreeItemTemplate()
				:Expanding_(false)
				, Expandable_(false)
				, Level_(0)
			{
				GuiTreeItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiTreeItemTemplate::~GuiTreeItemTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiGridCellTemplate
***********************************************************************/

			GuiGridCellTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiGridCellTemplate::GuiGridCellTemplate()
			{
				GuiGridCellTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiGridCellTemplate::~GuiGridCellTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiGridVisualizerTemplate
***********************************************************************/

			GuiGridVisualizerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiGridVisualizerTemplate::GuiGridVisualizerTemplate()
			{
				GuiGridVisualizerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiGridVisualizerTemplate::~GuiGridVisualizerTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiGridEditorTemplate
***********************************************************************/

			GuiGridEditorTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiGridEditorTemplate::GuiGridEditorTemplate()
				:CellValueSaved_(true)
				, FocusControl_(nullptr)
			{
				GuiGridEditorTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiGridEditorTemplate::~GuiGridEditorTemplate()
			{
				FinalizeAggregation();
			}
		}
	}
}