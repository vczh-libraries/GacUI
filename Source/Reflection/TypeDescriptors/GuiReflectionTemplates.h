/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Basic

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIREFLECTIOTEMPLATES
#define VCZH_PRESENTATION_REFLECTION_GUIREFLECTIOTEMPLATES

#include "GuiReflectionControls.h"
#include "../../Controls/Templates/GuiControlTemplateStyles.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
Type List
***********************************************************************/

#define GUIREFLECTIONTEMPLATES_TYPELIST(F)\
			F(presentation::templates::BoolOption)\
			F(presentation::templates::GuiTemplate)\
			F(presentation::templates::GuiTemplate::IFactory)\
			F(presentation::templates::GuiControlTemplate)\
			F(presentation::templates::GuiLabelTemplate)\
			F(presentation::templates::GuiSinglelineTextBoxTemplate)\
			F(presentation::templates::GuiDocumentLabelTemplate)\
			F(presentation::templates::GuiMultilineTextBoxTemplate)\
			F(presentation::templates::GuiDocumentViewerTemplate)\
			F(presentation::templates::GuiMenuTemplate)\
			F(presentation::templates::GuiWindowTemplate)\
			F(presentation::templates::GuiButtonTemplate)\
			F(presentation::templates::GuiSelectableButtonTemplate)\
			F(presentation::templates::GuiToolstripButtonTemplate)\
			F(presentation::templates::GuiListViewColumnHeaderTemplate)\
			F(presentation::templates::GuiComboBoxTemplate)\
			F(presentation::templates::GuiDatePickerTemplate)\
			F(presentation::templates::GuiDateComboBoxTemplate)\
			F(presentation::templates::GuiScrollTemplate)\
			F(presentation::templates::GuiScrollViewTemplate)\
			F(presentation::templates::GuiTextListTemplate)\
			F(presentation::templates::GuiListViewTemplate)\
			F(presentation::templates::GuiTreeViewTemplate)\
			F(presentation::templates::GuiTabTemplate)\
			F(presentation::templates::GuiListItemTemplate)\
			F(presentation::templates::GuiTreeItemTemplate)\
			F(presentation::templates::GuiGridVisualizerTemplate)\
			F(presentation::templates::GuiGridEditorTemplate)\
			F(presentation::templates::GuiControlTemplate_StyleProvider)\
			F(presentation::templates::GuiLabelTemplate_StyleProvider)\
			F(presentation::templates::GuiSinglelineTextBoxTemplate_StyleProvider)\
			F(presentation::templates::GuiDocumentLabelTemplate_StyleProvider)\
			F(presentation::templates::GuiMultilineTextBoxTemplate_StyleProvider)\
			F(presentation::templates::GuiDocumentViewerTemplate_StyleProvider)\
			F(presentation::templates::GuiMenuTemplate_StyleProvider)\
			F(presentation::templates::GuiWindowTemplate_StyleProvider)\
			F(presentation::templates::GuiButtonTemplate_StyleProvider)\
			F(presentation::templates::GuiSelectableButtonTemplate_StyleProvider)\
			F(presentation::templates::GuiToolstripButtonTemplate_StyleProvider)\
			F(presentation::templates::GuiListViewColumnHeaderTemplate_StyleProvider)\
			F(presentation::templates::GuiComboBoxTemplate_StyleProvider)\
			F(presentation::templates::GuiDatePickerTemplate_StyleProvider)\
			F(presentation::templates::GuiDateComboBoxTemplate_StyleProvider)\
			F(presentation::templates::GuiScrollTemplate_StyleProvider)\
			F(presentation::templates::GuiScrollViewTemplate_StyleProvider)\
			F(presentation::templates::GuiTextListTemplate_StyleProvider)\
			F(presentation::templates::GuiListViewTemplate_StyleProvider)\
			F(presentation::templates::GuiTreeViewTemplate_StyleProvider)\
			F(presentation::templates::GuiTabTemplate_StyleProvider)\
			F(presentation::templates::GuiControlTemplate_ItemStyleProvider)\
			F(presentation::templates::GuiListItemTemplate_ItemStyleController)\
			F(presentation::templates::GuiListItemTemplate_ItemStyleProvider)\
			F(presentation::templates::GuiTreeItemTemplate_ItemStyleProvider)\
			F(presentation::templates::GuiBindableDataVisualizer)\
			F(presentation::templates::GuiBindableDataVisualizer::Factory)\
			F(presentation::templates::GuiBindableDataVisualizer::DecoratedFactory)\
			F(presentation::templates::GuiBindableDataEditor)\
			F(presentation::templates::GuiBindableDataEditor::Factory)\

			GUIREFLECTIONTEMPLATES_TYPELIST(DECL_TYPE_INFO)

/***********************************************************************
Interface Proxy
***********************************************************************/

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::templates::GuiTemplate::IFactory)
				presentation::templates::GuiTemplate* CreateTemplate(const Value& viewModel)override
				{
					INVOKEGET_INTERFACE_PROXY(CreateTemplate, viewModel);
				}
			END_INTERFACE_PROXY(presentation::templates::GuiTemplate::IFactory)

/***********************************************************************
Type Loader
***********************************************************************/

#endif

			extern bool						LoadGuiTemplateTypes();
		}
	}
}

#endif