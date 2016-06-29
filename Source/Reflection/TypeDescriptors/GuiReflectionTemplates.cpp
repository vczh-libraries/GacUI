#include "GuiReflectionTemplates.h"
#include "GuiReflectionEvents.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			using namespace collections;
			using namespace parsing;
			using namespace parsing::tabling;
			using namespace parsing::xml;
			using namespace stream;
			using namespace presentation;
			using namespace presentation::compositions;
			using namespace presentation::controls;
			using namespace presentation::templates;

#ifndef VCZH_DEBUG_NO_REFLECTION

			GUIREFLECTIONTEMPLATES_TYPELIST(IMPL_VL_TYPE_INFO)

/***********************************************************************
Type Declaration
***********************************************************************/

#define _ ,

#define GUI_TEMPLATE_PROPERTY_REFLECTION(CLASS, TYPE, NAME)\
	CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(NAME)

			BEGIN_ENUM_ITEM(BoolOption)
				ENUM_CLASS_ITEM(AlwaysTrue)
				ENUM_CLASS_ITEM(AlwaysFalse)
				ENUM_CLASS_ITEM(Customizable)
			END_ENUM_ITEM(BoolOption)

			BEGIN_CLASS_MEMBER(GuiTemplate)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_BASE(GuiInstanceRootObject)
				CLASS_MEMBER_CONSTRUCTOR(GuiTemplate*(), NO_PARAMETER)

				GuiTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiTemplate)
			
			BEGIN_INTERFACE_MEMBER(GuiTemplate::IFactory)
				CLASS_MEMBER_EXTERNALCTOR(Ptr<GuiTemplate::IFactory>(const List<ITypeDescriptor*>&), { L"types" }, &GuiTemplate::IFactory::CreateTemplateFactory)

				CLASS_MEMBER_METHOD(CreateTemplate, NO_PARAMETER)
			END_INTERFACE_MEMBER(GuiTemplate::IFactory)

			BEGIN_CLASS_MEMBER(GuiControlTemplate)
				CLASS_MEMBER_BASE(GuiTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiControlTemplate*(), NO_PARAMETER)

				GuiControlTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiControlTemplate)

			BEGIN_CLASS_MEMBER(GuiLabelTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiLabelTemplate*(), NO_PARAMETER)

				GuiLabelTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiLabelTemplate)

			BEGIN_CLASS_MEMBER(GuiSinglelineTextBoxTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiSinglelineTextBoxTemplate*(), NO_PARAMETER)

				GuiSinglelineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiSinglelineTextBoxTemplate)

			BEGIN_CLASS_MEMBER(GuiDocumentLabelTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiDocumentLabelTemplate*(), NO_PARAMETER)

				GuiDocumentLabelTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiDocumentLabelTemplate)

			BEGIN_CLASS_MEMBER(GuiMultilineTextBoxTemplate)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiMultilineTextBoxTemplate*(), NO_PARAMETER)

				GuiMultilineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiMultilineTextBoxTemplate)

			BEGIN_CLASS_MEMBER(GuiDocumentViewerTemplate)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiDocumentViewerTemplate*(), NO_PARAMETER)

				GuiDocumentViewerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiDocumentViewerTemplate)

			BEGIN_CLASS_MEMBER(GuiMenuTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiMenuTemplate*(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiMenuTemplate)

			BEGIN_CLASS_MEMBER(GuiWindowTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiWindowTemplate*(), NO_PARAMETER)

				GuiWindowTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiWindowTemplate)

			BEGIN_CLASS_MEMBER(GuiButtonTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiButtonTemplate*(), NO_PARAMETER)

				GuiButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiButtonTemplate)

			BEGIN_CLASS_MEMBER(GuiSelectableButtonTemplate)
				CLASS_MEMBER_BASE(GuiButtonTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiSelectableButtonTemplate*(), NO_PARAMETER)

				GuiSelectableButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiSelectableButtonTemplate)

			BEGIN_CLASS_MEMBER(GuiToolstripButtonTemplate)
				CLASS_MEMBER_BASE(GuiSelectableButtonTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiToolstripButtonTemplate*(), NO_PARAMETER)

				GuiToolstripButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiToolstripButtonTemplate)

			BEGIN_CLASS_MEMBER(GuiListViewColumnHeaderTemplate)
				CLASS_MEMBER_BASE(GuiToolstripButtonTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiListViewColumnHeaderTemplate*(), NO_PARAMETER)

				GuiListViewColumnHeaderTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiListViewColumnHeaderTemplate)

			BEGIN_CLASS_MEMBER(GuiComboBoxTemplate)
				CLASS_MEMBER_BASE(GuiToolstripButtonTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiComboBoxTemplate*(), NO_PARAMETER)

				GuiComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiComboBoxTemplate)

			BEGIN_CLASS_MEMBER(GuiDatePickerTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiDatePickerTemplate*(), NO_PARAMETER)

				GuiDatePickerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiDatePickerTemplate)

			BEGIN_CLASS_MEMBER(GuiDateComboBoxTemplate)
				CLASS_MEMBER_BASE(GuiComboBoxTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiDateComboBoxTemplate*(), NO_PARAMETER)

				GuiDateComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiDateComboBoxTemplate)

			BEGIN_CLASS_MEMBER(GuiScrollTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiScrollTemplate*(), NO_PARAMETER)

				GuiScrollTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiScrollTemplate)

			BEGIN_CLASS_MEMBER(GuiScrollViewTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiScrollViewTemplate*(), NO_PARAMETER)

				GuiScrollViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiScrollViewTemplate)

			BEGIN_CLASS_MEMBER(GuiTextListTemplate)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiTextListTemplate*(), NO_PARAMETER)

				GuiTextListTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiTextListTemplate)

			BEGIN_CLASS_MEMBER(GuiListViewTemplate)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiListViewTemplate*(), NO_PARAMETER)

				GuiListViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiListViewTemplate)

			BEGIN_CLASS_MEMBER(GuiTreeViewTemplate)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiTreeViewTemplate*(), NO_PARAMETER)

				GuiTreeViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiTreeViewTemplate)

			BEGIN_CLASS_MEMBER(GuiTabTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiTabTemplate*(), NO_PARAMETER)

				GuiTabTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiTabTemplate)

			BEGIN_CLASS_MEMBER(GuiListItemTemplate)
				CLASS_MEMBER_BASE(GuiTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiListItemTemplate*(), NO_PARAMETER)

				GuiListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiListItemTemplate)

			BEGIN_CLASS_MEMBER(GuiTreeItemTemplate)
				CLASS_MEMBER_BASE(GuiListItemTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiTreeItemTemplate*(), NO_PARAMETER)

				GuiTreeItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiTreeItemTemplate)

			BEGIN_CLASS_MEMBER(GuiGridVisualizerTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiGridVisualizerTemplate*(), NO_PARAMETER)

				GuiGridVisualizerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiGridVisualizerTemplate)

			BEGIN_CLASS_MEMBER(GuiGridEditorTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiGridEditorTemplate*(), NO_PARAMETER)

				GuiGridEditorTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiGridEditorTemplate)

			BEGIN_CLASS_MEMBER(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControl::IStyleController)
				CLASS_MEMBER_BASE(GuiControl::IStyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiControlTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiControlTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiLabelTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiLabel::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiLabelTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiLabelTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiSinglelineTextBoxTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiSinglelineTextBox::IStyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiSinglelineTextBoxTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiSinglelineTextBoxTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiDocumentLabelTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiDocumentLabel::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiDocumentLabelTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiDocumentLabelTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiMultilineTextBoxTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate_StyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiMultilineTextBoxTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiMultilineTextBoxTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiDocumentViewerTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiDocumentViewer::IStyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiDocumentViewerTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiDocumentViewerTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiMenuTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiWindow::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiMenuTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiMenuTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiWindowTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiWindow::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiWindowTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiWindowTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiButton::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiButtonTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiButtonTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiSelectableButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiSelectableButton::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiSelectableButtonTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiSelectableButtonTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiToolstripButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiMenuButton::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiToolstripButtonTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiToolstripButtonTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiListViewColumnHeaderTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiToolstripButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiListViewColumnHeader::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiListViewColumnHeaderTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiListViewColumnHeaderTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiComboBoxTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiToolstripButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiComboBoxListControl::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiComboBoxTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiComboBoxTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiDatePickerTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiDatePicker::IStyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiDatePickerTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiDatePickerTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiDateComboBoxTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiComboBoxTemplate_StyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiDateComboBoxTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
				CLASS_MEMBER_METHOD(CreateArgument, NO_PARAMETER)
			END_CLASS_MEMBER(GuiDateComboBoxTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiScrollTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScroll::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiScrollTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiScrollTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiScrollViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollView::IStyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiScrollViewTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiScrollViewTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiTextListTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollView::IStyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiTextListTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
				CLASS_MEMBER_METHOD(CreateArgument, NO_PARAMETER)
			END_CLASS_MEMBER(GuiTextListTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiListViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiListViewBase::IStyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiListViewTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiListViewTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiTreeViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiVirtualTreeView::IStyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiTreeViewTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiTreeViewTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiTabTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiTab::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiTabTemplate_StyleProvider*(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiTabTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiControlTemplate_ItemStyleProvider)
				CLASS_MEMBER_BASE(GuiComboBoxListControl::IItemStyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiControlTemplate_ItemStyleProvider>(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiControlTemplate_ItemStyleProvider)

			BEGIN_CLASS_MEMBER(GuiListItemTemplate_ItemStyleController)
				CLASS_MEMBER_BASE(GuiListControl::IItemStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiListItemTemplate_ItemStyleController*(GuiListItemTemplate_ItemStyleProvider*), { L"itemStyleProvider" })
			END_CLASS_MEMBER(GuiListItemTemplate_ItemStyleController)

			BEGIN_CLASS_MEMBER(GuiListItemTemplate_ItemStyleProvider)
				CLASS_MEMBER_BASE(GuiSelectableListControl::IItemStyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiListItemTemplate_ItemStyleProvider>(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiListItemTemplate_ItemStyleProvider)

			BEGIN_CLASS_MEMBER(GuiTreeItemTemplate_ItemStyleProvider)
				CLASS_MEMBER_BASE(tree::INodeItemStyleProvider)
				CLASS_MEMBER_BASE(tree::INodeProviderCallback)

				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiTreeItemTemplate_ItemStyleProvider>(Ptr<GuiTemplate::IFactory>), { L"factory" })
			END_CLASS_MEMBER(GuiTreeItemTemplate_ItemStyleProvider)

			BEGIN_CLASS_MEMBER(GuiBindableDataVisualizer)
				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiBindableDataVisualizer>(), NO_PARAMETER)
				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiBindableDataVisualizer>(Ptr<list::IDataVisualizer>), { L"decoratedVisualizer" })
			END_CLASS_MEMBER(GuiBindableDataVisualizer)

			BEGIN_CLASS_MEMBER(GuiBindableDataVisualizer::Factory)
				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiBindableDataVisualizer::Factory>(Ptr<GuiTemplate::IFactory>, list::BindableDataColumn*), { L"templateFactory" _ L"ownerColumn" })
			END_CLASS_MEMBER(GuiBindableDataVisualizer::Factory)

			BEGIN_CLASS_MEMBER(GuiBindableDataVisualizer::DecoratedFactory)
				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiBindableDataVisualizer::DecoratedFactory>(Ptr<GuiTemplate::IFactory>, list::BindableDataColumn*, Ptr<list::IDataVisualizerFactory>), { L"templateFactory" _ L"ownerColumn" _ L"decoratedFactory" })
			END_CLASS_MEMBER(GuiBindableDataVisualizer::DecoratedFactory)

			BEGIN_CLASS_MEMBER(GuiBindableDataEditor)
				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiBindableDataEditor>(), NO_PARAMETER)
			END_CLASS_MEMBER(GuiBindableDataEditor)

			BEGIN_CLASS_MEMBER(GuiBindableDataEditor::Factory)
				CLASS_MEMBER_CONSTRUCTOR(Ptr<GuiBindableDataEditor::Factory>(Ptr<GuiTemplate::IFactory>, list::BindableDataColumn*), { L"templateFactory" _ L"ownerColumn" })
			END_CLASS_MEMBER(GuiBindableDataEditor::Factory)

#undef INTERFACE_EXTERNALCTOR
#undef _

/***********************************************************************
Type Loader
***********************************************************************/

			class GuiTemplateTypeLoader : public Object, public ITypeLoader
			{
			public:
				void Load(ITypeManager* manager)
				{
					GUIREFLECTIONTEMPLATES_TYPELIST(ADD_TYPE_INFO)
				}

				void Unload(ITypeManager* manager)
				{
				}
			};

#endif

			bool LoadGuiTemplateTypes()
			{
#ifndef VCZH_DEBUG_NO_REFLECTION
				ITypeManager* manager=GetGlobalTypeManager();
				if(manager)
				{
					Ptr<ITypeLoader> loader=new GuiTemplateTypeLoader;
					return manager->AddTypeLoader(loader);
				}
#endif
				return false;
			}
		}
	}
}