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

			BEGIN_INTERFACE_MEMBER_NOPROXY(IScrollViewCommandExecutor)
				CLASS_MEMBER_BASE(IDescriptable)
				CLASS_MEMBER_METHOD(CalculateView, NO_PARAMETER)
			END_INTERFACE_MEMBER(IScrollViewCommandExecutor)

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

			BEGIN_CLASS_MEMBER(GuiControlTemplate)
				CLASS_MEMBER_BASE(GuiTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiControlTemplate*(), NO_PARAMETER)

				GuiControlTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
				CLASS_MEMBER_METHOD(Initialize, NO_PARAMETER)
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
				CLASS_MEMBER_PROPERTY_READONLY_FAST(EditingText)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(TextElement)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(TextComposition)
				CLASS_MEMBER_METHOD(RearrangeTextElement, NO_PARAMETER)
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
				CLASS_MEMBER_PROPERTY_READONLY_FAST(EditingText)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(TextElement)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(TextComposition)
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

			BEGIN_CLASS_MEMBER(GuiScrollTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiScrollTemplate*(), NO_PARAMETER)

				GuiScrollTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiScrollTemplate)

			BEGIN_CLASS_MEMBER(GuiScrollViewTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiScrollViewTemplate*(), NO_PARAMETER)

				GuiScrollViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)

				CLASS_MEMBER_METHOD(AdjustView, {L"fullSize"})
				CLASS_MEMBER_PROPERTY_READONLY_FAST(HorizontalScroll)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(VerticalScroll)
				CLASS_MEMBER_PROPERTY_READONLY_FAST(InternalContainerComposition)
				CLASS_MEMBER_PROPERTY_FAST(HorizontalAlwaysVisible)
				CLASS_MEMBER_PROPERTY_FAST(VerticalAlwaysVisible)
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

			BEGIN_CLASS_MEMBER(GuiDatePickerTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiDatePickerTemplate*(), NO_PARAMETER)

				GuiDatePickerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
				CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(DateLocale)
				CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(Date)
			END_CLASS_MEMBER(GuiDatePickerTemplate)

			BEGIN_CLASS_MEMBER(GuiDateComboBoxTemplate)
				CLASS_MEMBER_BASE(GuiComboBoxTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiDateComboBoxTemplate*(), NO_PARAMETER)

				GuiDateComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiDateComboBoxTemplate)

			BEGIN_CLASS_MEMBER(GuiListItemTemplate)
				CLASS_MEMBER_BASE(GuiTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiListItemTemplate*(), NO_PARAMETER)

				GuiListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiListItemTemplate)

			BEGIN_CLASS_MEMBER(GuiTextListItemTemplate)
				CLASS_MEMBER_BASE(GuiListItemTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiTextListItemTemplate*(), NO_PARAMETER)

				GuiTextListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiTextListItemTemplate)

			BEGIN_CLASS_MEMBER(GuiTreeItemTemplate)
				CLASS_MEMBER_BASE(GuiTextListItemTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiTreeItemTemplate*(), NO_PARAMETER)

				GuiTreeItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiTreeItemTemplate)

			BEGIN_CLASS_MEMBER(GuiGridCellTemplate)
				CLASS_MEMBER_BASE(GuiControlTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiGridCellTemplate*(), NO_PARAMETER)

				GuiGridCellTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiGridCellTemplate)

			BEGIN_CLASS_MEMBER(GuiGridVisualizerTemplate)
				CLASS_MEMBER_BASE(GuiGridCellTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiGridVisualizerTemplate*(), NO_PARAMETER)

				GuiGridVisualizerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiGridVisualizerTemplate)

			BEGIN_CLASS_MEMBER(GuiGridEditorTemplate)
				CLASS_MEMBER_BASE(GuiGridCellTemplate)
				CLASS_MEMBER_CONSTRUCTOR(GuiGridEditorTemplate*(), NO_PARAMETER)

				GuiGridEditorTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_REFLECTION)
			END_CLASS_MEMBER(GuiGridEditorTemplate)

			BEGIN_CLASS_MEMBER(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControl::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiControlTemplate_StyleProvider*(TemplateProperty<GuiControlTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiControlTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiLabelTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiLabel::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiLabelTemplate_StyleProvider*(TemplateProperty<GuiLabelTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiLabelTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiSinglelineTextBoxTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiSinglelineTextBox::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiSinglelineTextBoxTemplate_StyleProvider*(TemplateProperty<GuiSinglelineTextBoxTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiSinglelineTextBoxTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiDocumentLabelTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiDocumentLabel::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiDocumentLabelTemplate_StyleProvider*(TemplateProperty<GuiDocumentLabelTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiDocumentLabelTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiMultilineTextBoxTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate_StyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiMultilineTextBoxTemplate_StyleProvider*(TemplateProperty<GuiMultilineTextBoxTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiMultilineTextBoxTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiDocumentViewerTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiDocumentViewer::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiDocumentViewerTemplate_StyleProvider*(TemplateProperty<GuiDocumentViewerTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiDocumentViewerTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiMenuTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiWindow::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiMenuTemplate_StyleProvider*(TemplateProperty<GuiMenuTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiMenuTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiWindowTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiWindow::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiWindowTemplate_StyleProvider*(TemplateProperty<GuiWindowTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiWindowTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiButton::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiButtonTemplate_StyleProvider*(TemplateProperty<GuiButtonTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiButtonTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiSelectableButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiSelectableButton::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiSelectableButtonTemplate_StyleProvider*(TemplateProperty<GuiSelectableButtonTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiSelectableButtonTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiToolstripButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiMenuButton::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiToolstripButtonTemplate_StyleProvider*(TemplateProperty<GuiToolstripButtonTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiToolstripButtonTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiListViewColumnHeaderTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiToolstripButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiListViewColumnHeader::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiListViewColumnHeaderTemplate_StyleProvider*(TemplateProperty<GuiListViewColumnHeaderTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiListViewColumnHeaderTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiComboBoxTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiToolstripButtonTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiComboBoxListControl::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiComboBoxTemplate_StyleProvider*(TemplateProperty<GuiComboBoxTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiComboBoxTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiDatePickerTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiDatePicker::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiDatePickerTemplate_StyleProvider*(TemplateProperty<GuiDatePickerTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiDatePickerTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiDateComboBoxTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiComboBoxTemplate_StyleProvider)

				CLASS_MEMBER_CONSTRUCTOR(GuiDateComboBoxTemplate_StyleProvider*(TemplateProperty<GuiDateComboBoxTemplate>), { L"factory" })
				CLASS_MEMBER_METHOD(CreateArgument, NO_PARAMETER)
			END_CLASS_MEMBER(GuiDateComboBoxTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiScrollTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScroll::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiScrollTemplate_StyleProvider*(TemplateProperty<GuiScrollTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiScrollTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiScrollViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollView::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiScrollViewTemplate_StyleProvider*(TemplateProperty<GuiScrollViewTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiScrollViewTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiTextListTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiVirtualTextList::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiTextListTemplate_StyleProvider*(TemplateProperty<GuiTextListTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiTextListTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiListViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiListViewBase::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiListViewTemplate_StyleProvider*(TemplateProperty<GuiListViewTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiListViewTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiTreeViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiScrollViewTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiVirtualTreeView::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiTreeViewTemplate_StyleProvider*(TemplateProperty<GuiTreeViewTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiTreeViewTemplate_StyleProvider)

			BEGIN_CLASS_MEMBER(GuiTabTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiControlTemplate_StyleProvider)
				CLASS_MEMBER_BASE(GuiTab::IStyleController)

				CLASS_MEMBER_CONSTRUCTOR(GuiTabTemplate_StyleProvider*(TemplateProperty<GuiTabTemplate>), { L"factory" })
			END_CLASS_MEMBER(GuiTabTemplate_StyleProvider)

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