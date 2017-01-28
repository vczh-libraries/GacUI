/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATES
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATES

#include "../Styles/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{

#define GUI_TEMPLATE_PROPERTY_DECL(CLASS, TYPE, NAME)\
			private:\
				TYPE NAME##_;\
			public:\
				TYPE Get##NAME();\
				void Set##NAME(TYPE const& value);\
				compositions::GuiNotifyEvent NAME##Changed;\

#define GUI_TEMPLATE_PROPERTY_IMPL(CLASS, TYPE, NAME)\
			TYPE CLASS::Get##NAME()\
			{\
				return NAME##_;\
			}\
			void CLASS::Set##NAME(TYPE const& value)\
			{\
				if (NAME##_ != value)\
				{\
					NAME##_ = value;\
					NAME##Changed.Execute(compositions::GuiEventArgs(this));\
				}\
			}\

#define GUI_TEMPLATE_PROPERTY_EVENT_INIT(CLASS, TYPE, NAME)\
			NAME##Changed.SetAssociatedComposition(this);

			/// <summary>Represents a user customizable template.</summary>
			class GuiTemplate : public compositions::GuiBoundsComposition, public controls::GuiInstanceRootObject, public Description<GuiTemplate>
			{
			public:
				/// <summary>Factory interface for creating <see cref="GuiTemplate"/> instances.</summary>
				class IFactory : public IDescriptable, public Description<IFactory>
				{
				public:
					/// <summary>Create a <see cref="GuiTemplate"/> instance.</summary>
					/// <returns>The created template.</returns>
					/// <param name="viewModel">The view model for binding.</param>
					virtual GuiTemplate*				CreateTemplate(const description::Value& viewModel) = 0;
				};

				/// <summary>Create a template.</summary>
				GuiTemplate();
				~GuiTemplate();
				
#define GuiTemplate_PROPERTIES(F)\
				F(GuiTemplate, FontProperties, Font)\
				F(GuiTemplate, bool, VisuallyEnabled)\

				GuiTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

/***********************************************************************
Control Template
***********************************************************************/

			class GuiControlTemplate : public GuiTemplate, public AggregatableDescription<GuiControlTemplate>
			{
			public:
				GuiControlTemplate();
				~GuiControlTemplate();
				
#define GuiControlTemplate_PROPERTIES(F)\
				F(GuiControlTemplate, WString, Text)\
				F(GuiControlTemplate, compositions::GuiGraphicsComposition*, ContainerComposition)\
				F(GuiControlTemplate, compositions::GuiGraphicsComposition*, FocusableComposition)\

				GuiControlTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiLabelTemplate :public GuiControlTemplate, public AggregatableDescription<GuiLabelTemplate>
			{
			public:
				GuiLabelTemplate();
				~GuiLabelTemplate();

#define GuiLabelTemplate_PROPERTIES(F)\
				F(GuiLabelTemplate, Color, DefaultTextColor)\
				F(GuiLabelTemplate, Color, TextColor)\

				GuiLabelTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiSinglelineTextBoxTemplate : public GuiControlTemplate, public AggregatableDescription<GuiSinglelineTextBoxTemplate>
			{
			public:
				GuiSinglelineTextBoxTemplate();
				~GuiSinglelineTextBoxTemplate();

#define GuiSinglelineTextBoxTemplate_PROPERTIES(F)\
				F(GuiSinglelineTextBoxTemplate, elements::text::ColorEntry, TextColor)\
				F(GuiSinglelineTextBoxTemplate, Color, CaretColor)\

				GuiSinglelineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiDocumentLabelTemplate : public GuiControlTemplate, public AggregatableDescription<GuiDocumentLabelTemplate>
			{
			public:
				GuiDocumentLabelTemplate();
				~GuiDocumentLabelTemplate();

#define GuiDocumentLabelTemplate_PROPERTIES(F)\
				F(GuiDocumentLabelTemplate, Ptr<DocumentModel>, BaselineDocument)\

				GuiDocumentLabelTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiMenuTemplate : public GuiControlTemplate, public AggregatableDescription<GuiMenuTemplate>
			{
			public:
				GuiMenuTemplate();
				~GuiMenuTemplate();
			};

			enum class BoolOption
			{
				AlwaysTrue,
				AlwaysFalse,
				Customizable,
			};

			class GuiWindowTemplate : public GuiControlTemplate, public AggregatableDescription<GuiWindowTemplate>
			{
			public:
				GuiWindowTemplate();
				~GuiWindowTemplate();

#define GuiWindowTemplate_PROPERTIES(F)\
				F(GuiWindowTemplate, BoolOption, MaximizedBoxOption)\
				F(GuiWindowTemplate, BoolOption, MinimizedBoxOption)\
				F(GuiWindowTemplate, BoolOption, BorderOption)\
				F(GuiWindowTemplate, BoolOption, SizeBoxOption)\
				F(GuiWindowTemplate, BoolOption, IconVisibleOption)\
				F(GuiWindowTemplate, BoolOption, TitleBarOption)\
				F(GuiWindowTemplate, bool, MaximizedBox)\
				F(GuiWindowTemplate, bool, MinimizedBox)\
				F(GuiWindowTemplate, bool, Border)\
				F(GuiWindowTemplate, bool, SizeBox)\
				F(GuiWindowTemplate, bool, IconVisible)\
				F(GuiWindowTemplate, bool, TitleBar)\
				F(GuiWindowTemplate, bool, CustomizedBorder)\
				F(GuiWindowTemplate, bool, Maximized)\
				F(GuiWindowTemplate, Ptr<GuiTemplate::IFactory>, TooltipTemplate)\
				F(GuiWindowTemplate, Ptr<GuiTemplate::IFactory>, ShortcutKeyTemplate)

				GuiWindowTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiButtonTemplate : public GuiControlTemplate, public AggregatableDescription<GuiButtonTemplate>
			{
			public:
				GuiButtonTemplate();
				~GuiButtonTemplate();

#define GuiButtonTemplate_PROPERTIES(F)\
				F(GuiButtonTemplate, controls::GuiButton::ControlState, State)\

				GuiButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiSelectableButtonTemplate : public GuiButtonTemplate, public AggregatableDescription<GuiSelectableButtonTemplate>
			{
			public:
				GuiSelectableButtonTemplate();
				~GuiSelectableButtonTemplate();

#define GuiSelectableButtonTemplate_PROPERTIES(F)\
				F(GuiSelectableButtonTemplate, bool, Selected)\

				GuiSelectableButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiToolstripButtonTemplate : public GuiSelectableButtonTemplate, public AggregatableDescription<GuiToolstripButtonTemplate>
			{
			public:
				GuiToolstripButtonTemplate();
				~GuiToolstripButtonTemplate();

#define GuiToolstripButtonTemplate_PROPERTIES(F)\
				F(GuiToolstripButtonTemplate, Ptr<GuiTemplate::IFactory>, SubMenuTemplate)\
				F(GuiToolstripButtonTemplate, bool, SubMenuExisting)\
				F(GuiToolstripButtonTemplate, bool, SubMenuOpening)\
				F(GuiToolstripButtonTemplate, controls::GuiButton*, SubMenuHost)\
				F(GuiToolstripButtonTemplate, Ptr<GuiImageData>, Image)\
				F(GuiToolstripButtonTemplate, WString, ShortcutText)\

				GuiToolstripButtonTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiListViewColumnHeaderTemplate :public GuiToolstripButtonTemplate, public AggregatableDescription<GuiListViewColumnHeaderTemplate>
			{
			public:
				GuiListViewColumnHeaderTemplate();
				~GuiListViewColumnHeaderTemplate();

#define GuiListViewColumnHeaderTemplate_PROPERTIES(F)\
				F(GuiListViewColumnHeaderTemplate, controls::GuiListViewColumnHeader::ColumnSortingState, SortingState)\

				GuiListViewColumnHeaderTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiComboBoxTemplate : public GuiToolstripButtonTemplate, public AggregatableDescription<GuiComboBoxTemplate>
			{
			public:
				GuiComboBoxTemplate();
				~GuiComboBoxTemplate();

#define GuiComboBoxTemplate_PROPERTIES(F)\
				F(GuiComboBoxTemplate, controls::GuiComboBoxBase::ICommandExecutor*, Commands)\
				F(GuiComboBoxTemplate, bool, TextVisible)\

				GuiComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiDatePickerTemplate : public GuiControlTemplate, public AggregatableDescription<GuiDatePickerTemplate>
			{
			public:
				GuiDatePickerTemplate();
				~GuiDatePickerTemplate();

#define GuiDatePickerTemplate_PROPERTIES(F)\
				F(GuiDatePickerTemplate, Ptr<GuiTemplate::IFactory>, DateButtonTemplate)\
				F(GuiDatePickerTemplate, Ptr<GuiTemplate::IFactory>, DateTextListTemplate)\
				F(GuiDatePickerTemplate, Ptr<GuiTemplate::IFactory>, DateComboBoxTemplate)\
				F(GuiDatePickerTemplate, Color, BackgroundColor)\
				F(GuiDatePickerTemplate, Color, PrimaryTextColor)\
				F(GuiDatePickerTemplate, Color, SecondaryTextColor)\

				GuiDatePickerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiDateComboBoxTemplate : public GuiComboBoxTemplate, public AggregatableDescription<GuiDateComboBoxTemplate>
			{
			public:
				GuiDateComboBoxTemplate();
				~GuiDateComboBoxTemplate();

#define GuiDateComboBoxTemplate_PROPERTIES(F)\
				F(GuiDateComboBoxTemplate, Ptr<GuiTemplate::IFactory>, DatePickerTemplate)\

				GuiDateComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiScrollTemplate : public GuiControlTemplate, public AggregatableDescription<GuiScrollTemplate>
			{
			public:
				GuiScrollTemplate();
				~GuiScrollTemplate();

#define GuiScrollTemplate_PROPERTIES(F)\
				F(GuiScrollTemplate, controls::GuiScroll::ICommandExecutor*, Commands)\
				F(GuiScrollTemplate, vint, TotalSize)\
				F(GuiScrollTemplate, vint, PageSize)\
				F(GuiScrollTemplate, vint, Position)\

				GuiScrollTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiScrollViewTemplate : public GuiControlTemplate, public AggregatableDescription<GuiScrollViewTemplate>
			{
			public:
				GuiScrollViewTemplate();
				~GuiScrollViewTemplate();

#define GuiScrollViewTemplate_PROPERTIES(F)\
				F(GuiScrollViewTemplate, Ptr<GuiTemplate::IFactory>, HScrollTemplate)\
				F(GuiScrollViewTemplate, Ptr<GuiTemplate::IFactory>, VScrollTemplate)\
				F(GuiScrollViewTemplate, vint, DefaultScrollSize)\

				GuiScrollViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiMultilineTextBoxTemplate : public GuiScrollViewTemplate, public AggregatableDescription<GuiMultilineTextBoxTemplate>
			{
			public:
				GuiMultilineTextBoxTemplate();
				~GuiMultilineTextBoxTemplate();

#define GuiMultilineTextBoxTemplate_PROPERTIES(F)\
				F(GuiMultilineTextBoxTemplate, elements::text::ColorEntry, TextColor)\
				F(GuiMultilineTextBoxTemplate, Color, CaretColor)\

				GuiMultilineTextBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiDocumentViewerTemplate : public GuiScrollViewTemplate, public AggregatableDescription<GuiDocumentViewerTemplate>
			{
			public:
				GuiDocumentViewerTemplate();
				~GuiDocumentViewerTemplate();

#define GuiDocumentViewerTemplate_PROPERTIES(F)\
				F(GuiDocumentViewerTemplate, Ptr<DocumentModel>, BaselineDocument)\

				GuiDocumentViewerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiTextListTemplate : public GuiScrollViewTemplate, public AggregatableDescription<GuiTextListTemplate>
			{
			public:
				GuiTextListTemplate();
				~GuiTextListTemplate();

#define GuiTextListTemplate_PROPERTIES(F)\
				F(GuiTextListTemplate, Ptr<GuiTemplate::IFactory>, BackgroundTemplate)\
				F(GuiTextListTemplate, Ptr<GuiTemplate::IFactory>, BulletTemplate)\
				F(GuiTextListTemplate, Color, TextColor)\

				GuiTextListTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiListViewTemplate : public GuiScrollViewTemplate, public AggregatableDescription<GuiListViewTemplate>
			{
			public:
				GuiListViewTemplate();
				~GuiListViewTemplate();

#define GuiListViewTemplate_PROPERTIES(F)\
				F(GuiListViewTemplate, Ptr<GuiTemplate::IFactory>, BackgroundTemplate)\
				F(GuiListViewTemplate, Ptr<GuiTemplate::IFactory>, ColumnHeaderTemplate)\
				F(GuiListViewTemplate, Color, PrimaryTextColor)\
				F(GuiListViewTemplate, Color, SecondaryTextColor)\
				F(GuiListViewTemplate, Color, ItemSeparatorColor)\

				GuiListViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiTreeViewTemplate : public GuiScrollViewTemplate, public AggregatableDescription<GuiTreeViewTemplate>
			{
			public:
				GuiTreeViewTemplate();
				~GuiTreeViewTemplate();

#define GuiTreeViewTemplate_PROPERTIES(F)\
				F(GuiTreeViewTemplate, Ptr<GuiTemplate::IFactory>, BackgroundTemplate)\
				F(GuiTreeViewTemplate, Ptr<GuiTemplate::IFactory>, ExpandingDecoratorTemplate)\
				F(GuiTreeViewTemplate, Color, TextColor)\

				GuiTreeViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiTabTemplate : public GuiControlTemplate, public AggregatableDescription<GuiTabTemplate>
			{
			public:
				GuiTabTemplate();
				~GuiTabTemplate();

#define GuiTabTemplate_PROPERTIES(F)\
				F(GuiTabTemplate, Ptr<GuiTemplate::IFactory>, HeaderTemplate)\
				F(GuiTabTemplate, Ptr<GuiTemplate::IFactory>, DropdownTemplate)\
				F(GuiTabTemplate, Ptr<GuiTemplate::IFactory>, MenuTemplate)\
				F(GuiTabTemplate, Ptr<GuiTemplate::IFactory>, MenuItemTemplate)\
				F(GuiTabTemplate, vint, HeaderPadding)\
				F(GuiTabTemplate, compositions::GuiGraphicsComposition*, HeaderComposition)\

				GuiTabTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

/***********************************************************************
Item Template
***********************************************************************/

			class GuiListItemTemplate : public GuiTemplate, public AggregatableDescription<GuiListItemTemplate>
			{
			public:
				GuiListItemTemplate();
				~GuiListItemTemplate();
				
#define GuiListItemTemplate_PROPERTIES(F)\
				F(GuiListItemTemplate, bool, Selected)\
				F(GuiListItemTemplate, vint, Index)\

				GuiListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiTextListItemTemplate : public GuiListItemTemplate, public AggregatableDescription<GuiTextListItemTemplate>
			{
			public:
				GuiTextListItemTemplate();
				~GuiTextListItemTemplate();
				
#define GuiTextListItemTemplate_PROPERTIES(F)\
				F(GuiTextListItemTemplate, Color, TextColor)\

				GuiTextListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiTreeItemTemplate : public GuiTextListItemTemplate, public AggregatableDescription<GuiTreeItemTemplate>
			{
			public:
				GuiTreeItemTemplate();
				~GuiTreeItemTemplate();
				
#define GuiTreeItemTemplate_PROPERTIES(F)\
				F(GuiTreeItemTemplate, bool, Expanding)\

				GuiTreeItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiGridVisualizerTemplate : public GuiControlTemplate , public AggregatableDescription<GuiGridVisualizerTemplate>
			{
			public:
				GuiGridVisualizerTemplate();
				~GuiGridVisualizerTemplate();

#define GuiGridVisualizerTemplate_PROPERTIES(F)\
				F(GuiGridVisualizerTemplate, description::Value, RowValue)\
				F(GuiGridVisualizerTemplate, description::Value, CellValue)\
				F(GuiGridVisualizerTemplate, bool, Selected)\

				GuiGridVisualizerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiGridEditorTemplate : public GuiControlTemplate , public AggregatableDescription<GuiGridEditorTemplate>
			{
			public:
				GuiGridEditorTemplate();
				~GuiGridEditorTemplate();

#define GuiGridEditorTemplate_PROPERTIES(F)\
				F(GuiGridEditorTemplate, description::Value, RowValue)\
				F(GuiGridEditorTemplate, description::Value, CellValue)\

				GuiGridEditorTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};
		}
	}
}

#endif