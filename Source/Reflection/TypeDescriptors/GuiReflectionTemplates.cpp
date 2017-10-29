#include "GuiReflectionPlugin.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			using namespace presentation;
			using namespace presentation::compositions;
			using namespace presentation::controls;
			using namespace presentation::templates;

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
Type Declaration
***********************************************************************/

#define _ ,

#define GUI_TEMPLATE_PROPERTY_REFLECTION(CLASS, TYPE, NAME)\
	CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(NAME)

			BEGIN_ENUM_ITEM(ButtonState)
				ENUM_CLASS_ITEM(Normal)
				ENUM_CLASS_ITEM(Active)
				ENUM_CLASS_ITEM(Pressed)
			END_ENUM_ITEM(ButtonState)

			BEGIN_ENUM_ITEM(ColumnSortingState)
				ENUM_CLASS_ITEM(NotSorted)
				ENUM_CLASS_ITEM(Ascending)
				ENUM_CLASS_ITEM(Descending)
			END_ENUM_ITEM(ColumnSortingState)

			BEGIN_ENUM_ITEM(BoolOption)
				ENUM_CLASS_ITEM(AlwaysTrue)
				ENUM_CLASS_ITEM(AlwaysFalse)
				ENUM_CLASS_ITEM(Customizable)
			END_ENUM_ITEM(BoolOption)

			BEGIN_INTERFACE_MEMBER_NOPROXY(ITextBoxCommandExecutor)
				CLASS_MEMBER_BASE(IDescriptable)

				CLASS_MEMBER_METHOD(UnsafeSetText, { L"value" })
			END_INTERFACE_MEMBER(ITextBoxCommandExecutor)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IComboBoxCommandExecutor)
				CLASS_MEMBER_BASE(IDescriptable)

				CLASS_MEMBER_METHOD(SelectItem, NO_PARAMETER)
			END_INTERFACE_MEMBER(IComboBoxCommandExecutor)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IScrollCommandExecutor)
				CLASS_MEMBER_METHOD(SmallDecrease, NO_PARAMETER)
				CLASS_MEMBER_METHOD(SmallIncrease, NO_PARAMETER)
				CLASS_MEMBER_METHOD(BigDecrease, NO_PARAMETER)
				CLASS_MEMBER_METHOD(BigIncrease, NO_PARAMETER)
				CLASS_MEMBER_METHOD(SetTotalSize, { L"value" })
				CLASS_MEMBER_METHOD(SetPageSize, { L"value" })
				CLASS_MEMBER_METHOD(SetPosition, { L"value" })
			END_INTERFACE_MEMBER(IScrollCommandExecutor)

			BEGIN_INTERFACE_MEMBER_NOPROXY(ITabCommandExecutor)
				CLASS_MEMBER_BASE(IDescriptable)
				CLASS_MEMBER_METHOD(ShowTab, { L"index" })
			END_INTERFACE_MEMBER(ITabCommandExecutor)

			BEGIN_INTERFACE_MEMBER_NOPROXY(IDatePickerCommandExecutor)
				CLASS_MEMBER_BASE(IDescriptable)
				CLASS_MEMBER_METHOD(NotifyDateChanged, NO_PARAMETER)
				CLASS_MEMBER_METHOD(NotifyDateNavigated, NO_PARAMETER)
				CLASS_MEMBER_METHOD(NotifyDateSelected, NO_PARAMETER)
			END_INTERFACE_MEMBER(IDatePickerCommandExecutor)

			BEGIN_CLASS_MEMBER(GuiComponent)
			END_CLASS_MEMBER(GuiComponent)

			BEGIN_CLASS_MEMBER(GuiInstanceRootObject)
				CLASS_MEMBER_METHOD_OVERLOAD(FinalizeInstanceRecursively, {L"thisObject"}, void(GuiInstanceRootObject::*)(GuiTemplate*))
				CLASS_MEMBER_METHOD_OVERLOAD(FinalizeInstanceRecursively, {L"thisObject"}, void(GuiInstanceRootObject::*)(GuiCustomControl*))
				CLASS_MEMBER_METHOD_OVERLOAD(FinalizeInstanceRecursively, {L"thisObject"}, void(GuiInstanceRootObject::*)(GuiControlHost*))
				CLASS_MEMBER_METHOD(FinalizeGeneralInstance, {L"thisObject"})
				CLASS_MEMBER_METHOD(SetResourceResolver, {L"resolver"})
				CLASS_MEMBER_METHOD(ResolveResource, {L"protocol" _ L"path" _ L"ensureExist"})

				CLASS_MEMBER_METHOD(AddSubscription, {L"subscription"})
				CLASS_MEMBER_METHOD(UpdateSubscriptions, NO_PARAMETER)
				CLASS_MEMBER_METHOD(AddComponent, {L"component"})
				CLASS_MEMBER_METHOD(AddControlHostComponent, {L"controlHost"})
			END_CLASS_MEMBER(GuiInstanceRootObject)

			BEGIN_CLASS_MEMBER(GuiTemplate)
				CLASS_MEMBER_BASE(GuiBoundsComposition)
				CLASS_MEMBER_BASE(GuiInstanceRootObject)
				CLASS_MEMBER_CONSTRUCTOR(GuiTemplate*(), NO_PARAMETER)

				GuiTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiTemplate)

#define GUI_CONTROL_TEMPLATE(NAME, BASE)\
			BEGIN_CLASS_MEMBER(NAME)\
				CLASS_MEMBER_BASE(BASE)\
				CLASS_MEMBER_CONSTRUCTOR(NAME*(), NO_PARAMETER)\
				NAME ## _PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)\
			END_CLASS_MEMBER(NAME)\

			GUI_CONTROL_TEMPLATE(GuiControlTemplate, GuiTemplate)
			GUI_CONTROL_TEMPLATE(GuiLabelTemplate, GuiControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiSinglelineTextBoxTemplate, GuiControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiDocumentLabelTemplate, GuiControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiMultilineTextBoxTemplate, GuiScrollViewTemplate)
			GUI_CONTROL_TEMPLATE(GuiDocumentViewerTemplate, GuiScrollViewTemplate)
			GUI_CONTROL_TEMPLATE(GuiMenuTemplate, GuiWindowTemplate)
			GUI_CONTROL_TEMPLATE(GuiWindowTemplate, GuiControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiButtonTemplate, GuiControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiSelectableButtonTemplate, GuiButtonTemplate)
			GUI_CONTROL_TEMPLATE(GuiToolstripButtonTemplate, GuiSelectableButtonTemplate)
			GUI_CONTROL_TEMPLATE(GuiListViewColumnHeaderTemplate, GuiToolstripButtonTemplate)
			GUI_CONTROL_TEMPLATE(GuiComboBoxTemplate, GuiToolstripButtonTemplate)
			GUI_CONTROL_TEMPLATE(GuiScrollTemplate, GuiControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiScrollViewTemplate, GuiControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiListControlTemplate, GuiScrollViewTemplate)
			GUI_CONTROL_TEMPLATE(GuiTextListTemplate, GuiListControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiListViewTemplate, GuiListControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiTreeViewTemplate, GuiListControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiTabTemplate, GuiControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiDatePickerTemplate, GuiControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiDateComboBoxTemplate, GuiComboBoxTemplate)
			GUI_CONTROL_TEMPLATE(GuiListItemTemplate, GuiTemplate)
			GUI_CONTROL_TEMPLATE(GuiTextListItemTemplate, GuiListItemTemplate)
			GUI_CONTROL_TEMPLATE(GuiTreeItemTemplate, GuiTextListItemTemplate)
			GUI_CONTROL_TEMPLATE(GuiGridCellTemplate, GuiControlTemplate)
			GUI_CONTROL_TEMPLATE(GuiGridVisualizerTemplate, GuiGridCellTemplate)
			GUI_CONTROL_TEMPLATE(GuiGridEditorTemplate, GuiGridCellTemplate)

			BEGIN_CLASS_MEMBER(GuiCommonDatePickerLook)
				CLASS_MEMBER_BASE(GuiTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiCommonDatePickerLook*(Color, Color, Color), { L"backgroundColor" _ L"primaryTextColor" _ L"secondaryTextColor" })

				CLASS_MEMBER_PROPERTY_FAST(Commands)
				CLASS_MEMBER_PROPERTY_FAST(DateButtonTemplate)
				CLASS_MEMBER_PROPERTY_FAST(DateTextListTemplate)
				CLASS_MEMBER_PROPERTY_FAST(DateComboBoxTemplate)
				CLASS_MEMBER_PROPERTY_FAST(DateLocale)
				CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(Date)
				CLASS_MEMBER_PROPERTY_FAST(Font)
			END_CLASS_MEMBER(GuiCommonDatePickerLook)

			BEGIN_CLASS_MEMBER(GuiCommonScrollViewLook)
				CLASS_MEMBER_BASE(GuiTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiCommonScrollViewLook*(vint), { L"defaultScrollSize" })
				
				CLASS_MEMBER_PROPERTY_FAST(HScrollTemplate)
				CLASS_MEMBER_PROPERTY_FAST(VScrollTemplate)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(HScroll)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(VScroll)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(ContainerComposition)
			END_CLASS_MEMBER(GuiCommonScrollViewLook)

			BEGIN_CLASS_MEMBER(GuiCommonScrollBehavior)
				CLASS_MEMBER_BASE(GuiComponent)
				CLASS_MEMBER_CONSTRUCTOR(GuiCommonScrollBehavior*(), NO_PARAMETER)
				
				CLASS_MEMBER_METHOD(AttachScrollTemplate, { L"value" })
				CLASS_MEMBER_METHOD(AttachDecreaseButton, { L"button" })
				CLASS_MEMBER_METHOD(AttachIncreaseButton, { L"button" })
				CLASS_MEMBER_METHOD(AttachHorizontalScrollHandle, { L"partialView" })
				CLASS_MEMBER_METHOD(AttachVerticalScrollHandle, { L"partialView" })
				CLASS_MEMBER_METHOD(AttachHorizontalTrackerHandle, { L"partialView" })
				CLASS_MEMBER_METHOD(AttachVerticalTrackerHandle, { L"partialView" })
				CLASS_MEMBER_METHOD(GetHorizontalTrackerHandlerPosition, { L"handle" _ L"totalSize" _ L"pageSize" _ L"position" })
				CLASS_MEMBER_METHOD(GetVerticalTrackerHandlerPosition, { L"handle" _ L"totalSize" _ L"pageSize" _ L"position" })
			END_CLASS_MEMBER(GuiCommonScrollBehavior)

#undef GUI_CONTROL_TEMPLATE
#undef GUI_TEMPLATE_PROPERTY_REFLECTION
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