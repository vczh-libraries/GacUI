/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATES
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATES

#include "../../GraphicsElement/GuiGraphicsTextElement.h"
#include "../../Application/Controls/GuiThemeManager.h"
#include "../../GraphicsComposition/IncludeForward.h"

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

		namespace controls
		{
			class GuiControlHost;
			class GuiCustomControl;

			/// <summary>The visual state for button.</summary>
			enum class ButtonState
			{
				/// <summary>Normal state.</summary>
				Normal,
				/// <summary>Active state (when the cursor is hovering on a button).</summary>
				Active,
				/// <summary>Pressed state (when the buttin is being pressed).</summary>
				Pressed,
			};

			/// <summary>Represents the sorting state of list view items related to this column.</summary>
			enum class ColumnSortingState
			{
				/// <summary>Not sorted.</summary>
				NotSorted,
				/// <summary>Ascending.</summary>
				Ascending,
				/// <summary>Descending.</summary>
				Descending,
			};

			/// <summary>Represents the order of tab pages.</summary>
			enum class TabPageOrder
			{
				/// <summary>Unknown.</summary>
				Unknown,
				/// <summary>Left to right.</summary>
				LeftToRight,
				/// <summary>Right to left.</summary>
				RightToLeft,
				/// <summary>Top to bottom.</summary>
				TopToBottom,
				/// <summary>Bottom to top.</summary>
				BottomToTop,
			};

			/// <summary>A command executor for the combo box to change the control state.</summary>
			class ITextBoxCommandExecutor : public virtual IDescriptable, public Description<ITextBoxCommandExecutor>
			{
			public:
				/// <summary>Override the text content in the control.</summary>
				/// <param name="value">The new text content.</param>
				virtual void						UnsafeSetText(const WString& value) = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IScrollCommandExecutor : public virtual IDescriptable, public Description<IScrollCommandExecutor>
			{
			public:
				/// <summary>Do small decrement.</summary>
				virtual void						SmallDecrease() = 0;
				/// <summary>Do small increment.</summary>
				virtual void						SmallIncrease() = 0;
				/// <summary>Do big decrement.</summary>
				virtual void						BigDecrease() = 0;
				/// <summary>Do big increment.</summary>
				virtual void						BigIncrease() = 0;

				/// <summary>Change to total size of the scroll.</summary>
				/// <param name="value">The total size.</param>
				virtual void						SetTotalSize(vint value) = 0;
				/// <summary>Change to page size of the scroll.</summary>
				/// <param name="value">The page size.</param>
				virtual void						SetPageSize(vint value) = 0;
				/// <summary>Change to position of the scroll.</summary>
				/// <param name="value">The position.</param>
				virtual void						SetPosition(vint value) = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class ITabCommandExecutor : public virtual IDescriptable, public Description<ITabCommandExecutor>
			{
			public:
				/// <summary>Select a tab page.</summary>
				/// <param name="index">The specified position for the tab page.</param>
				/// <param name="setFocus">Set to true to set focus to the tab control.</param>
				virtual void						ShowTab(vint index, bool setFocus) = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IDatePickerCommandExecutor : public virtual IDescriptable, public Description<IDatePickerCommandExecutor>
			{
			public:
				/// <summary>Called when the date has been changed.</summary>
				virtual void						NotifyDateChanged() = 0;
				/// <summary>Called when navigated to a date.</summary>
				virtual void						NotifyDateNavigated() = 0;
				/// <summary>Called when selected a date.</summary>
				virtual void						NotifyDateSelected() = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IRibbonGroupCommandExecutor : public virtual IDescriptable, public Description<IRibbonGroupCommandExecutor>
			{
			public:
				/// <summary>Called when the expand button is clicked.</summary>
				virtual void						NotifyExpandButtonClicked() = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IRibbonGalleryCommandExecutor : public virtual IDescriptable, public Description<IRibbonGalleryCommandExecutor>
			{
			public:
				/// <summary>Called when the scroll up button is clicked.</summary>
				virtual void						NotifyScrollUp() = 0;
				/// <summary>Called when the scroll down button is clicked.</summary>
				virtual void						NotifyScrollDown() = 0;
				/// <summary>Called when the dropdown button is clicked.</summary>
				virtual void						NotifyDropdown() = 0;
			};
		}

/***********************************************************************
Templates
***********************************************************************/

		namespace templates
		{

#define GUI_CONTROL_TEMPLATE_DECL(F)\
			F(GuiSinglelineTextBoxTemplate,		GuiControlTemplate)			\
			F(GuiDocumentLabelTemplate,			GuiControlTemplate)			\
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
			F(GuiRibbonGroupMenuTemplate,		GuiMenuTemplate)			\
			F(GuiRibbonIconLabelTemplate,		GuiControlTemplate)			\
			F(GuiRibbonButtonsTemplate,			GuiControlTemplate)			\
			F(GuiRibbonToolstripsTemplate,		GuiControlTemplate)			\
			F(GuiRibbonToolstripMenuTemplate,	GuiMenuTemplate)			\
			F(GuiRibbonGalleryTemplate,			GuiControlTemplate)			\
			F(GuiRibbonGalleryListTemplate,		GuiRibbonGalleryTemplate)	\

#define GUI_ITEM_TEMPLATE_DECL(F)\
			F(GuiListItemTemplate,				GuiTemplate)				\
			F(GuiTextListItemTemplate,			GuiListItemTemplate)		\
			F(GuiTreeItemTemplate,				GuiListItemTemplate)		\
			F(GuiGridCellTemplate,				GuiControlTemplate)			\
			F(GuiGridVisualizerTemplate,		GuiGridCellTemplate)		\
			F(GuiGridEditorTemplate,			GuiGridCellTemplate)		\

/***********************************************************************
Control Template
***********************************************************************/

#define GuiSinglelineTextBoxTemplate_PROPERTIES(F)\
				F(GuiSinglelineTextBoxTemplate, elements::text::ColorEntry, TextColor, {})\
				F(GuiSinglelineTextBoxTemplate, Color, CaretColor, {})\

#define GuiDocumentLabelTemplate_PROPERTIES(F)\
				F(GuiDocumentLabelTemplate, Ptr<DocumentModel>, BaselineDocument, {})\
				F(GuiDocumentLabelTemplate, Color, CaretColor, {})\

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
				F(GuiRibbonGroupTemplate, TemplateProperty<GuiRibbonGroupMenuTemplate>, SubMenuTemplate, {})\

#define GuiRibbonGroupMenuTemplate_PROPERTIES(F)\
				F(GuiRibbonGroupMenuTemplate, controls::IRibbonGroupCommandExecutor*, Commands, nullptr)\
				F(GuiRibbonGroupMenuTemplate, bool, Expandable, false)\

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
				F(GuiListItemTemplate, controls::GuiListControl*, AssociatedListControl, nullptr)\

#define GuiTextListItemTemplate_PROPERTIES(F)\
				F(GuiTextListItemTemplate, Color, TextColor, {})\
				F(GuiTextListItemTemplate, bool, Checked, false)\
				
#define GuiTreeItemTemplate_PROPERTIES(F)\
				F(GuiTreeItemTemplate, Color, TextColor, {})\
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

			GUI_CONTROL_TEMPLATE_DECL(GUI_TEMPLATE_CLASS_FORWARD_DECL)
			GUI_ITEM_TEMPLATE_DECL(GUI_TEMPLATE_CLASS_FORWARD_DECL)

			GUI_CONTROL_TEMPLATE_DECL(GUI_TEMPLATE_CLASS_DECL)
			GUI_ITEM_TEMPLATE_DECL(GUI_TEMPLATE_CLASS_DECL)
		}
	}
}

#endif