#include "GuiControlTemplates.h"
#include "../Templates/GuiThemeStyleFactory.h"
#include "../GuiWindowControls.h"
#include "../GuiScrollControls.h"
#include "../ListControlPackage/GuiListControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace reflection::description;
			using namespace compositions;

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

			GuiSinglelineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiSinglelineTextBoxTemplate::GuiSinglelineTextBoxTemplate()
			{
				GuiSinglelineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiSinglelineTextBoxTemplate::~GuiSinglelineTextBoxTemplate()
			{
				FinalizeAggregation();
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

			GuiMenuTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiMenuTemplate::GuiMenuTemplate()
			{
				GuiMenuTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
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
				, CustomFrameEnabled_(true)
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
				:Commands_(nullptr)
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
				:Commands_(nullptr)
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

			void GuiScrollViewTemplate::UpdateTable()
			{
				if (horizontalScroll->GetEnabled() || horizontalAlwaysVisible)
				{
					tableComposition->SetRowOption(1, GuiCellOption::AbsoluteOption(GetDefaultScrollSize()));
				}
				else
				{
					tableComposition->SetRowOption(1, GuiCellOption::AbsoluteOption(0));
				}
				if (verticalScroll->GetEnabled() || verticalAlwaysVisible)
				{
					tableComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(GetDefaultScrollSize()));
				}
				else
				{
					tableComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(0));
				}
				tableComposition->UpdateCellBounds();
			}

			GuiScrollViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiScrollViewTemplate::GuiScrollViewTemplate()
				:DefaultScrollSize_(0)
				, Commands_(nullptr)
			{
				GuiScrollViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiScrollViewTemplate::~GuiScrollViewTemplate()
			{
				FinalizeAggregation();
			}

			void GuiScrollViewTemplate::AdjustView(Size fullSize)
			{
				Size viewSize = containerComposition->GetBounds().GetSize();
				if (fullSize.x <= viewSize.x)
				{
					horizontalScroll->SetEnabled(false);
					horizontalScroll->SetPosition(0);
				}
				else
				{
					horizontalScroll->SetEnabled(true);
					horizontalScroll->SetTotalSize(fullSize.x);
					horizontalScroll->SetPageSize(viewSize.x);
				}
				if (fullSize.y <= viewSize.y)
				{
					verticalScroll->SetEnabled(false);
					verticalScroll->SetPosition(0);
				}
				else
				{
					verticalScroll->SetEnabled(true);
					verticalScroll->SetTotalSize(fullSize.y);
					verticalScroll->SetPageSize(viewSize.y);
				}
				UpdateTable();
			}

			GuiScroll* GuiScrollViewTemplate::GetHorizontalScroll()
			{
				return horizontalScroll;
			}

			GuiScroll* GuiScrollViewTemplate::GetVerticalScroll()
			{
				return verticalScroll;
			}

			bool GuiScrollViewTemplate::GetHorizontalAlwaysVisible()
			{
				return horizontalAlwaysVisible;
			}

			void GuiScrollViewTemplate::SetHorizontalAlwaysVisible(bool value)
			{
				if (horizontalAlwaysVisible != value)
				{
					horizontalAlwaysVisible = value;
					Commands_->CalculateView();
				}
			}

			bool GuiScrollViewTemplate::GetVerticalAlwaysVisible()
			{
				return verticalAlwaysVisible;
			}

			void GuiScrollViewTemplate::SetVerticalAlwaysVisible(bool value)
			{
				if (verticalAlwaysVisible != value)
				{
					verticalAlwaysVisible = value;
					Commands_->CalculateView();
				}
			}

			void GuiScrollViewTemplate::Initialize()
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
				SetContainerComposition(containerComposition);
			}

/***********************************************************************
GuiMultilineTextBoxTemplate
***********************************************************************/

			GuiMultilineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiMultilineTextBoxTemplate::GuiMultilineTextBoxTemplate()
				:Commands_(nullptr)
			{
				GuiMultilineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiMultilineTextBoxTemplate::~GuiMultilineTextBoxTemplate()
			{
				FinalizeAggregation();
			}

/***********************************************************************
GuiListControlTemplate
***********************************************************************/

			GuiListControlTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiListControlTemplate::GuiListControlTemplate()
			{
				GuiListControlTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiListControlTemplate::~GuiListControlTemplate()
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
				:Commands_(nullptr)
				, SelectedTabPage_(nullptr)
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

			GuiDatePickerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiDatePickerTemplate::GuiDatePickerTemplate()
				:Commands_(nullptr)
			{
				GuiDatePickerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiDatePickerTemplate::~GuiDatePickerTemplate()
			{
				FinalizeAggregation();
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