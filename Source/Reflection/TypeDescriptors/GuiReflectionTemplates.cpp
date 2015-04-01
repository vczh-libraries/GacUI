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

#ifndef VCZH_DEBUG_NO_REFLECTION

			GUIREFLECTIONTEMPLATES_TYPELIST(IMPL_VL_TYPE_INFO)

/***********************************************************************
Type Declaration
***********************************************************************/

#define _ ,

#define INTERFACE_EXTERNALCTOR(CONTROL, INTERFACE)\
	CLASS_MEMBER_EXTERNALCTOR(decltype(interface_proxy::CONTROL##_##INTERFACE::Create(0))(Ptr<IValueInterfaceProxy>), {L"proxy"}, &interface_proxy::CONTROL##_##INTERFACE::Create)

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
			
			BEGIN_CLASS_MEMBER(GuiTemplate::IFactory)
				INTERFACE_EXTERNALCTOR(GuiTemplate, IFactory)
				CLASS_MEMBER_EXTERNALCTOR(Ptr<GuiTemplate::IFactory>(const List<ITypeDescriptor*>&), { L"types" }, &GuiTemplate::IFactory::CreateTemplateFactory)

				CLASS_MEMBER_METHOD(CreateTemplate, NO_PARAMETER)
			END_CLASS_MEMBER(GuiTemplate::IFactory)

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