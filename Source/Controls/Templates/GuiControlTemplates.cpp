#include "GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{
			using namespace collections;
			using namespace controls;
			using namespace compositions;

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
				FinalizeInstance();
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
				:State_(GuiButton::Normal)
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
				:SortingState_(GuiListViewColumnHeader::NotSorted)
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
				:HeaderPadding_(0)
				, HeaderComposition_(0)
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
			{
				GuiTreeItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiTreeItemTemplate::~GuiTreeItemTemplate()
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