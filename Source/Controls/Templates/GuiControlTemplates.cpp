#include "GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{
			using namespace description;
			using namespace collections;
			using namespace controls;
			using namespace compositions;

/***********************************************************************
GuiTemplate::IFactory
***********************************************************************/

			class GuiTemplateReflectableFactory : public Object, public virtual GuiTemplate::IFactory
			{
			protected:
				List<ITypeDescriptor*>			types;

			public:
				GuiTemplateReflectableFactory(const List<ITypeDescriptor*>& _types)
				{
					CopyFrom(types, _types);
				}

				GuiTemplate* CreateTemplate(const description::Value& viewModel)override
				{
					FOREACH(ITypeDescriptor*, type, types)
					{
						auto group = type->GetConstructorGroup();
						vint count = group->GetMethodCount();
						for (vint i = 0; i < count; i++)
						{
							auto ctor = group->GetMethod(i);
							if (ctor->GetReturn()->GetDecorator() == ITypeInfo::RawPtr && ctor->GetParameterCount() <= 1)
							{
								Array<Value> arguments(ctor->GetParameterCount());
								if (ctor->GetParameterCount() == 1)
								{
									if (!viewModel.CanConvertTo(ctor->GetParameter(0)->GetType()))
									{
										continue;
									}
									arguments[0] = viewModel;
								}
								return dynamic_cast<GuiTemplate*>(ctor->Invoke(Value(), arguments).GetRawPtr());
							}
						}
					}
					
					WString message = L"Unable to create a template from types {";
					FOREACH_INDEXER(ITypeDescriptor*, type, index, types)
					{
						if (index > 0) message += L", ";
						message += type->GetTypeName();
					}
					message += L"} using view model: ";
					if (viewModel.IsNull())
					{
						message += L"null.";
					}
					else
					{
						message += viewModel.GetTypeDescriptor()->GetTypeName() + L".";
					}

					throw ArgumentException(message);
				}
			};

			Ptr<GuiTemplate::IFactory> GuiTemplate::IFactory::CreateTemplateFactory(const collections::List<description::ITypeDescriptor*>& types)
			{
				return new GuiTemplateReflectableFactory(types);
			}

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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
			}

/***********************************************************************
GuiMenuTemplate
***********************************************************************/

			GuiMenuTemplate::GuiMenuTemplate()
			{
			}

			GuiMenuTemplate::~GuiMenuTemplate()
			{
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
			}

/***********************************************************************
GuiComboBoxTemplate
***********************************************************************/

			GuiComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)

			GuiComboBoxTemplate::GuiComboBoxTemplate()
				:Commands_(0)
			{
				GuiComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)
			}

			GuiComboBoxTemplate::~GuiComboBoxTemplate()
			{
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
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
				FinalizeInstance();
			}
		}
	}
}