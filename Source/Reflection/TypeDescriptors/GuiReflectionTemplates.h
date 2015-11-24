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

			GUIREFLECTIONTEMPLATES_TYPELIST(DECL_TYPE_INFO)

/***********************************************************************
Interface Proxy
***********************************************************************/

#pragma warning(push)
#pragma warning(disable:4250)
			namespace interface_proxy
			{
				using namespace presentation;
				using namespace presentation::templates;

				class GuiTemplate_IFactory : public ValueInterfaceRoot, public virtual GuiTemplate::IFactory
				{
				public:
					GuiTemplate_IFactory(Ptr<IValueInterfaceProxy> _proxy)
						:ValueInterfaceRoot(_proxy)
					{
					}

					static Ptr<GuiTemplate::IFactory> Create(Ptr<IValueInterfaceProxy> proxy)
					{
						return new GuiTemplate_IFactory(proxy);
					}

					GuiTemplate* CreateTemplate(const Value& viewModel)override
					{
						return INVOKEGET_INTERFACE_PROXY(CreateTemplate, viewModel);
					}
				};
			}
#pragma warning(pop)

/***********************************************************************
Type Loader
***********************************************************************/

#endif

			extern bool						LoadGuiTemplateTypes();
		}
	}
}

#endif