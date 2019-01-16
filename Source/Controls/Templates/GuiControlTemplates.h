/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATES
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATES

#include "GuiControlShared.h"
#include "../../GraphicsElement/GuiGraphicsTextElement.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiButton;
			class GuiSelectableButton;
			class GuiListControl;
			class GuiComboBoxListControl;
			class GuiTextList;
			class GuiTabPage;
			class GuiScroll;
		}

		namespace templates
		{

#define GUI_TEMPLATE_PROPERTY_DECL(CLASS, TYPE, NAME, VALUE)\
			private:\
				TYPE NAME##_ = VALUE;\
			public:\
				TYPE Get##NAME();\
				void Set##NAME(TYPE const& value);\
				compositions::GuiNotifyEvent NAME##Changed;\

#define GUI_TEMPLATE_PROPERTY_IMPL(CLASS, TYPE, NAME, VALUE)\
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

#define GUI_TEMPLATE_PROPERTY_EVENT_INIT(CLASS, TYPE, NAME, VALUE)\
			NAME##Changed.SetAssociatedComposition(this);

#define GUI_TEMPLATE_CLASS_DECL(CLASS, BASE)\
			class CLASS : public BASE, public AggregatableDescription<CLASS>\
			{\
			public:\
				CLASS();\
				~CLASS();\
				CLASS ## _PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)\
			};\

#define GUI_TEMPLATE_CLASS_IMPL(CLASS, BASE)\
			CLASS ## _PROPERTIES(GUI_TEMPLATE_PROPERTY_IMPL)\
			CLASS::CLASS()\
			{\
				CLASS ## _PROPERTIES(GUI_TEMPLATE_PROPERTY_EVENT_INIT)\
			}\
			CLASS::~CLASS()\
			{\
				FinalizeAggregation();\
			}\

#define GUI_CONTROL_TEMPLATE_DECL(F)\
			F(GuiControlTemplate,				GuiTemplate)				\
			F(GuiLabelTemplate,					GuiControlTemplate)			\
			F(GuiSinglelineTextBoxTemplate,		GuiControlTemplate)			\
			F(GuiDocumentLabelTemplate,			GuiControlTemplate)			\
			F(GuiWindowTemplate,				GuiControlTemplate)			\
			F(GuiMenuTemplate,					GuiWindowTemplate)			\
			F(GuiButtonTemplate,				GuiControlTemplate)			\
			F(GuiSelectableButtonTemplate,		GuiButtonTemplate)			\
			F(GuiToolstripButtonTemplate,		GuiSelectableButtonTemplate)\
			F(GuiListViewColumnHeaderTemplate,	GuiToolstripButtonTemplate)	\
			F(GuiComboBoxTemplate,				GuiToolstripButtonTemplate)	\
			F(GuiScrollTemplate,				GuiControlTemplate)			\
			F(GuiScrollViewTemplate,			GuiControlTemplate)			\
			F(GuiMultilineTextBoxTemplate,		GuiScrollViewTemplate)		\
			F(GuiDocumentViewerTemplate,		GuiScrollViewTemplate)		\
			F(GuiListControlTemplate,			GuiScrollViewTemplate)		\
			F(GuiTextListTemplate,				GuiListControlTemplate)		\
			F(GuiListViewTemplate,				GuiListControlTemplate)		\
			F(GuiTreeViewTemplate,				GuiListControlTemplate)		\
			F(GuiTabTemplate,					GuiControlTemplate)			\
			F(GuiDatePickerTemplate,			GuiControlTemplate)			\
			F(GuiDateComboBoxTemplate,			GuiComboBoxTemplate)		\
			F(GuiRibbonTabTemplate,				GuiTabTemplate)				\
			F(GuiRibbonGroupTemplate,			GuiControlTemplate)			\
			F(GuiRibbonIconLabelTemplate,		GuiControlTemplate)			\
			F(GuiRibbonButtonsTemplate,			GuiControlTemplate)			\
			F(GuiRibbonToolstripsTemplate,		GuiControlTemplate)			\
			F(GuiRibbonToolstripMenuTemplate,	GuiMenuTemplate)			\
			F(GuiRibbonGalleryTemplate,			GuiControlTemplate)			\
			F(GuiRibbonGalleryListTemplate,		GuiRibbonGalleryTemplate)	\

#define GUI_ITEM_TEMPLATE_DECL(F)\
			F(GuiTextListItemTemplate,			GuiListItemTemplate)		\
			F(GuiTreeItemTemplate,				GuiTextListItemTemplate)	\
			F(GuiGridCellTemplate,				GuiControlTemplate)			\
			F(GuiGridVisualizerTemplate,		GuiGridCellTemplate)		\
			F(GuiGridEditorTemplate,			GuiGridCellTemplate)		\

/***********************************************************************
GuiTemplate
***********************************************************************/

			/// <summary>Represents a user customizable template.</summary>
			class GuiTemplate : public compositions::GuiBoundsComposition, public controls::GuiInstanceRootObject, public Description<GuiTemplate>
			{
			protected:
				controls::GuiControlHost*		GetControlHostForInstance()override;
				void							OnParentLineChanged()override;
			public:
				/// <summary>Create a template.</summary>
				GuiTemplate();
				~GuiTemplate();
				
#define GuiTemplate_PROPERTIES(F)\
				F(GuiTemplate,	FontProperties,		Font,				{}	)\
				F(GuiTemplate,	description::Value,	Context,			{}	)\
				F(GuiTemplate,	WString,			Text,				{}	)\
				F(GuiTemplate,	bool,				VisuallyEnabled,	true)\

				GuiTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

/***********************************************************************
GuiListItemTemplate
***********************************************************************/

			class GuiListItemTemplate : public GuiTemplate, public AggregatableDescription<GuiListItemTemplate>
			{
			protected:
				controls::GuiListControl*	listControl = nullptr;

				virtual void				OnInitialize();
			public:
				GuiListItemTemplate();
				~GuiListItemTemplate();

#define GuiListItemTemplate_PROPERTIES(F)\
				F(GuiListItemTemplate, bool, Selected, false)\
				F(GuiListItemTemplate, vint, Index, 0)\

				GuiListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)

				void						BeginEditListItem();
				void						EndEditListItem();
				void						Initialize(controls::GuiListControl* _listControl);
			};

/***********************************************************************
Control Template
***********************************************************************/

			enum class BoolOption
			{
				AlwaysTrue,
				AlwaysFalse,
				Customizable,
			};

#define GuiControlTemplate_PROPERTIES(F)\
				F(GuiControlTemplate, compositions::GuiGraphicsComposition*, ContainerComposition, this)\
				F(GuiControlTemplate, compositions::GuiGraphicsComposition*, FocusableComposition, nullptr)\
				F(GuiControlTemplate, bool, Focused, false)\

#define GuiLabelTemplate_PROPERTIES(F)\
				F(GuiLabelTemplate, Color, DefaultTextColor, {})\
				F(GuiLabelTemplate, Color, TextColor, {})\

#define GuiSinglelineTextBoxTemplate_PROPERTIES(F)\
				F(GuiSinglelineTextBoxTemplate, elements::text::ColorEntry, TextColor, {})\
				F(GuiSinglelineTextBoxTemplate, Color, CaretColor, {})\

#define GuiDocumentLabelTemplate_PROPERTIES(F)\
				F(GuiDocumentLabelTemplate, Ptr<DocumentModel>, BaselineDocument, {})\
				F(GuiDocumentLabelTemplate, Color, CaretColor, {})\

#define GuiWindowTemplate_PROPERTIES(F)\
				F(GuiWindowTemplate, BoolOption, MaximizedBoxOption, BoolOption::Customizable)\
				F(GuiWindowTemplate, BoolOption, MinimizedBoxOption, BoolOption::Customizable)\
				F(GuiWindowTemplate, BoolOption, BorderOption, BoolOption::Customizable)\
				F(GuiWindowTemplate, BoolOption, SizeBoxOption, BoolOption::Customizable)\
				F(GuiWindowTemplate, BoolOption, IconVisibleOption, BoolOption::Customizable)\
				F(GuiWindowTemplate, BoolOption, TitleBarOption, BoolOption::Customizable)\
				F(GuiWindowTemplate, bool, MaximizedBox, true)\
				F(GuiWindowTemplate, bool, MinimizedBox, true)\
				F(GuiWindowTemplate, bool, Border, true)\
				F(GuiWindowTemplate, bool, SizeBox, true)\
				F(GuiWindowTemplate, bool, IconVisible, true)\
				F(GuiWindowTemplate, bool, TitleBar, true)\
				F(GuiWindowTemplate, bool, Maximized, false)\
				F(GuiWindowTemplate, bool, Activated, false)\
				F(GuiWindowTemplate, TemplateProperty<GuiWindowTemplate>, TooltipTemplate, {})\
				F(GuiWindowTemplate, TemplateProperty<GuiLabelTemplate>, ShortcutKeyTemplate, {})\
				F(GuiWindowTemplate, bool, CustomFrameEnabled, true)\
				F(GuiWindowTemplate, Margin, CustomFramePadding, {})\
				F(GuiWindowTemplate, Ptr<GuiImageData>, Icon, {})\

#define GuiMenuTemplate_PROPERTIES(F)

#define GuiButtonTemplate_PROPERTIES(F)\
				F(GuiButtonTemplate, controls::ButtonState, State, controls::ButtonState::Normal)\

#define GuiSelectableButtonTemplate_PROPERTIES(F)\
				F(GuiSelectableButtonTemplate, bool, Selected, false)\

#define GuiToolstripButtonTemplate_PROPERTIES(F)\
				F(GuiToolstripButtonTemplate, TemplateProperty<GuiMenuTemplate>, SubMenuTemplate, {})\
				F(GuiToolstripButtonTemplate, bool, SubMenuExisting, false)\
				F(GuiToolstripButtonTemplate, bool, SubMenuOpening, false)\
				F(GuiToolstripButtonTemplate, controls::GuiButton*, SubMenuHost, nullptr)\
				F(GuiToolstripButtonTemplate, Ptr<GuiImageData>, LargeImage, {})\
				F(GuiToolstripButtonTemplate, Ptr<GuiImageData>, Image, {})\
				F(GuiToolstripButtonTemplate, WString, ShortcutText, {})\

#define GuiListViewColumnHeaderTemplate_PROPERTIES(F)\
				F(GuiListViewColumnHeaderTemplate, controls::ColumnSortingState, SortingState, controls::ColumnSortingState::NotSorted)\

#define GuiComboBoxTemplate_PROPERTIES(F)\
				F(GuiComboBoxTemplate, bool, TextVisible, true)\

#define GuiScrollTemplate_PROPERTIES(F)\
				F(GuiScrollTemplate, controls::IScrollCommandExecutor*, Commands, nullptr)\
				F(GuiScrollTemplate, vint, TotalSize, 100)\
				F(GuiScrollTemplate, vint, PageSize, 10)\
				F(GuiScrollTemplate, vint, Position, 0)\

#define GuiScrollViewTemplate_PROPERTIES(F)\
				F(GuiScrollViewTemplate, controls::GuiScroll*, HorizontalScroll, nullptr)\
				F(GuiScrollViewTemplate, controls::GuiScroll*, VerticalScroll, nullptr)\

#define GuiMultilineTextBoxTemplate_PROPERTIES(F)\
				F(GuiMultilineTextBoxTemplate, controls::ITextBoxCommandExecutor*, Commands, nullptr)\
				F(GuiMultilineTextBoxTemplate, elements::text::ColorEntry, TextColor, {})\
				F(GuiMultilineTextBoxTemplate, Color, CaretColor, {})\

#define GuiDocumentViewerTemplate_PROPERTIES(F)\
				F(GuiDocumentViewerTemplate, Ptr<DocumentModel>, BaselineDocument, {})\
				F(GuiDocumentViewerTemplate, Color, CaretColor, {})\

#define GuiListControlTemplate_PROPERTIES(F)\
				F(GuiListControlTemplate, TemplateProperty<GuiSelectableButtonTemplate>, BackgroundTemplate, {})\

#define GuiTextListTemplate_PROPERTIES(F)\
				F(GuiTextListTemplate, Color, TextColor, {})\
				F(GuiTextListTemplate, TemplateProperty<GuiSelectableButtonTemplate>, CheckBulletTemplate, {})\
				F(GuiTextListTemplate, TemplateProperty<GuiSelectableButtonTemplate>, RadioBulletTemplate, {})\

#define GuiListViewTemplate_PROPERTIES(F)\
				F(GuiListViewTemplate, TemplateProperty<GuiListViewColumnHeaderTemplate>, ColumnHeaderTemplate, {})\
				F(GuiListViewTemplate, Color, PrimaryTextColor, {})\
				F(GuiListViewTemplate, Color, SecondaryTextColor, {})\
				F(GuiListViewTemplate, Color, ItemSeparatorColor, {})\

#define GuiTreeViewTemplate_PROPERTIES(F)\
				F(GuiTreeViewTemplate, TemplateProperty<GuiSelectableButtonTemplate>, ExpandingDecoratorTemplate, {})\
				F(GuiTreeViewTemplate, Color, TextColor, {})\

#define GuiTabTemplate_PROPERTIES(F)\
				F(GuiTabTemplate, controls::ITabCommandExecutor*, Commands, nullptr)\
				F(GuiTabTemplate, Ptr<reflection::description::IValueObservableList>, TabPages, {})\
				F(GuiTabTemplate, controls::GuiTabPage*, SelectedTabPage, nullptr)\
				F(GuiTabTemplate, controls::TabPageOrder, TabOrder, controls::TabPageOrder::Unknown)\

#define GuiDatePickerTemplate_PROPERTIES(F)\
				F(GuiDatePickerTemplate, controls::IDatePickerCommandExecutor*, Commands, nullptr)\
				F(GuiDatePickerTemplate, Locale, DateLocale, {})\
				F(GuiDatePickerTemplate, DateTime, Date, {})\

#define GuiDateComboBoxTemplate_PROPERTIES(F)\
				F(GuiDateComboBoxTemplate, TemplateProperty<GuiDatePickerTemplate>, DatePickerTemplate, {})\

#define GuiRibbonTabTemplate_PROPERTIES(F)\
				F(GuiRibbonTabTemplate, compositions::GuiGraphicsComposition*, BeforeHeadersContainer, nullptr)\
				F(GuiRibbonTabTemplate, compositions::GuiGraphicsComposition*, AfterHeadersContainer, nullptr)\

#define GuiRibbonGroupTemplate_PROPERTIES(F)\
				F(GuiRibbonGroupTemplate, controls::IRibbonGroupCommandExecutor*, Commands, nullptr)\
				F(GuiRibbonGroupTemplate, bool, Expandable, false)\
				F(GuiRibbonGroupTemplate, bool, Collapsed, false)\
				F(GuiRibbonGroupTemplate, TemplateProperty<GuiToolstripButtonTemplate>, LargeDropdownButtonTemplate, {})\
				F(GuiRibbonGroupTemplate, TemplateProperty<GuiMenuTemplate>, SubMenuTemplate, {})\

#define GuiRibbonIconLabelTemplate_PROPERTIES(F)\
				F(GuiRibbonIconLabelTemplate, Ptr<GuiImageData>, Image, {})\

#define GuiRibbonButtonsTemplate_PROPERTIES(F)\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, LargeButtonTemplate, {})\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, LargeDropdownButtonTemplate, {})\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, LargeSplitButtonTemplate, {})\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, SmallButtonTemplate, {})\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, SmallDropdownButtonTemplate, {})\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, SmallSplitButtonTemplate, {})\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, SmallIconLabelTemplate, {})\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, IconButtonTemplate, {})\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, IconDropdownButtonTemplate, {})\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, IconSplitButtonTemplate, {})\
				F(GuiRibbonButtonsTemplate, TemplateProperty<GuiToolstripButtonTemplate>, IconLabelTemplate, {})\

#define GuiRibbonToolstripsTemplate_PROPERTIES(F)\
				F(GuiRibbonToolstripsTemplate, TemplateProperty<GuiControlTemplate>, ToolbarTemplate, {})\

#define GuiRibbonToolstripMenuTemplate_PROPERTIES(F)\
				F(GuiRibbonToolstripMenuTemplate, compositions::GuiGraphicsComposition*, ContentComposition, nullptr)\

#define GuiRibbonGalleryTemplate_PROPERTIES(F)\
				F(GuiRibbonGalleryTemplate, controls::IRibbonGalleryCommandExecutor*, Commands, nullptr)\
				F(GuiRibbonGalleryTemplate, bool, ScrollUpEnabled, true)\
				F(GuiRibbonGalleryTemplate, bool, ScrollDownEnabled, true)\

#define GuiRibbonGalleryListTemplate_PROPERTIES(F)\
				F(GuiRibbonGalleryListTemplate, TemplateProperty<GuiTextListTemplate>, ItemListTemplate, {})\
				F(GuiRibbonGalleryListTemplate, TemplateProperty<GuiRibbonToolstripMenuTemplate>, MenuTemplate, {})\
				F(GuiRibbonGalleryListTemplate, TemplateProperty<GuiControlTemplate>, HeaderTemplate, {})\
				F(GuiRibbonGalleryListTemplate, TemplateProperty<GuiSelectableButtonTemplate>, BackgroundTemplate, {})\
				F(GuiRibbonGalleryListTemplate, TemplateProperty<GuiScrollViewTemplate>, GroupContainerTemplate, {})\

/***********************************************************************
Item Template
***********************************************************************/
				
#define GuiListItemTemplate_PROPERTIES(F)\
				F(GuiListItemTemplate, bool, Selected, false)\
				F(GuiListItemTemplate, vint, Index, 0)\

#define GuiTextListItemTemplate_PROPERTIES(F)\
				F(GuiTextListItemTemplate, Color, TextColor, {})\
				F(GuiTextListItemTemplate, bool, Checked, false)\
				
#define GuiTreeItemTemplate_PROPERTIES(F)\
				F(GuiTreeItemTemplate, bool, Expanding, false)\
				F(GuiTreeItemTemplate, bool, Expandable, false)\
				F(GuiTreeItemTemplate, vint, Level, 0)\
				F(GuiTreeItemTemplate, Ptr<GuiImageData>, Image, {})\

#define GuiGridCellTemplate_PROPERTIES(F)\
				F(GuiGridCellTemplate, Color, PrimaryTextColor, {})\
				F(GuiGridCellTemplate, Color, SecondaryTextColor, {})\
				F(GuiGridCellTemplate, Color, ItemSeparatorColor, {})\
				F(GuiGridCellTemplate, Ptr<GuiImageData>, LargeImage, {})\
				F(GuiGridCellTemplate, Ptr<GuiImageData>, SmallImage, {})\

#define GuiGridVisualizerTemplate_PROPERTIES(F)\
				F(GuiGridVisualizerTemplate, description::Value, RowValue, {})\
				F(GuiGridVisualizerTemplate, description::Value, CellValue, {})\
				F(GuiGridVisualizerTemplate, bool, Selected, false)\

#define GuiGridEditorTemplate_PROPERTIES(F)\
				F(GuiGridEditorTemplate, description::Value, RowValue, {})\
				F(GuiGridEditorTemplate, description::Value, CellValue, {})\
				F(GuiGridEditorTemplate, bool, CellValueSaved, true)\
				F(GuiGridEditorTemplate, controls::GuiControl*, FocusControl, nullptr)\

/***********************************************************************
Template Declarations
***********************************************************************/

			GUI_CONTROL_TEMPLATE_DECL(GUI_TEMPLATE_CLASS_DECL)
			GUI_ITEM_TEMPLATE_DECL(GUI_TEMPLATE_CLASS_DECL)
		}
	}
}

#endif