/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATES
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATES

#include "../../GraphicsComposition/GuiGraphicsComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiSelectableButton;
			class GuiListControl;
			class GuiComboBoxListControl;
			class GuiTextList;
			class GuiControlHost;
			class GuiCustomControl;
			class GuiTabPage;
			class GuiScroll;

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

			/// <summary>A command executor for the combo box to change the control state.</summary>
			class ITextBoxCommandExecutor : public virtual IDescriptable, public Description<ITextBoxCommandExecutor>
			{
			public:
				/// <summary>Override the text content in the control.</summary>
				/// <param name="value">The new text content.</param>
				virtual void						UnsafeSetText(const WString& value) = 0;
			};

			/// <summary>A command executor for the combo box to change the control state.</summary>
			class IComboBoxCommandExecutor : public virtual IDescriptable, public Description<IComboBoxCommandExecutor>
			{
			public:
				/// <summary>Notify that an item is selected, the combo box should close the popup and show the text of the selected item.</summary>
				virtual void						SelectItem() = 0;
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
				virtual void						ShowTab(vint index) = 0;
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

			class GuiInstanceRootObject;

			/// <summary>
			/// Represnets a component.
			/// </summary>
			class GuiComponent : public Object, public Description<GuiComponent>
			{
			public:
				GuiComponent();
				~GuiComponent();

				virtual void							Attach(GuiInstanceRootObject* rootObject);
				virtual void							Detach(GuiInstanceRootObject* rootObject);
			};

			/// <summary>Represnets a root GUI object.</summary>
			class GuiInstanceRootObject abstract : public Description<GuiInstanceRootObject>
			{
				typedef collections::List<Ptr<description::IValueSubscription>>		SubscriptionList;
			protected:
				Ptr<GuiResourcePathResolver>					resourceResolver;
				SubscriptionList								subscriptions;
				collections::SortedList<GuiComponent*>			components;
				bool											finalized = false;

			public:
				GuiInstanceRootObject();
				~GuiInstanceRootObject();

				/// <summary>Clear all subscriptions and components.</summary>
				void											FinalizeInstance();

				/// <summary>Test has the object been finalized.</summary>
				/// <returns>Returns true if this object has been finalized.</returns>
				bool											IsFinalized();

				void											FinalizeInstanceRecursively(templates::GuiTemplate* thisObject);
				void											FinalizeInstanceRecursively(GuiCustomControl* thisObject);
				void											FinalizeInstanceRecursively(GuiControlHost* thisObject);
				void											FinalizeGeneralInstance(GuiInstanceRootObject* thisObject);

				/// <summary>Set the resource resolver to connect the current root object to the resource creating it.</summary>
				/// <param name="resolver">The resource resolver</param>
				void											SetResourceResolver(Ptr<GuiResourcePathResolver> resolver);
				/// <summary>Resolve a resource using the current resource resolver.</summary>
				/// <returns>The loaded resource. Returns null if failed to load.</returns>
				/// <param name="protocol">The protocol.</param>
				/// <param name="path">The path.</param>
				/// <param name="ensureExist">Set to true and it will throw an exception if the resource doesn't exist.</param>
				Ptr<DescriptableObject>							ResolveResource(const WString& protocol, const WString& path, bool ensureExist);

				/// <summary>Add a subscription. When this control host is disposing, all attached subscriptions will be deleted.</summary>
				/// <returns>Returns null if this operation failed.</returns>
				/// <param name="subscription">The subscription to test.</param>
				Ptr<description::IValueSubscription>			AddSubscription(Ptr<description::IValueSubscription> subscription);
				/// <summary>Clear all subscriptions.</summary>
				void											UpdateSubscriptions();

				/// <summary>Add a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="component">The component to add.</param>
				bool											AddComponent(GuiComponent* component);

				/// <summary>Add a control host as a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="controlHost">The controlHost to add.</param>
				bool											AddControlHostComponent(GuiControlHost* controlHost);
			};

			class GuiButton;
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
			public:
				/// <summary>Create a template.</summary>
				GuiTemplate();
				~GuiTemplate();
				
#define GuiTemplate_PROPERTIES(F)\
				F(GuiTemplate,	FontProperties,	Font,				{}	)\
				F(GuiTemplate,	WString,		Text,				{}	)\
				F(GuiTemplate,	bool,			VisuallyEnabled,	true)\

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
				F(GuiWindowTemplate, bool, CustomizedBorder, false)\
				F(GuiWindowTemplate, bool, Maximized, false)\
				F(GuiWindowTemplate, TemplateProperty<GuiWindowTemplate>, TooltipTemplate, {})\
				F(GuiWindowTemplate, TemplateProperty<GuiLabelTemplate>, ShortcutKeyTemplate, {})\
				F(GuiWindowTemplate, bool, CustomFrameEnabled, true)\

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
				F(GuiToolstripButtonTemplate, Ptr<GuiImageData>, Image, {})\
				F(GuiToolstripButtonTemplate, WString, ShortcutText, {})\

#define GuiListViewColumnHeaderTemplate_PROPERTIES(F)\
				F(GuiListViewColumnHeaderTemplate, controls::ColumnSortingState, SortingState, controls::ColumnSortingState::NotSorted)\

#define GuiComboBoxTemplate_PROPERTIES(F)\
				F(GuiComboBoxTemplate, controls::IComboBoxCommandExecutor*, Commands, nullptr)\
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

#define GuiDatePickerTemplate_PROPERTIES(F)\
				F(GuiDatePickerTemplate, controls::IDatePickerCommandExecutor*, Commands, nullptr)\
				F(GuiDatePickerTemplate, Locale, DateLocale, {})\
				F(GuiDatePickerTemplate, DateTime, Date, {})\

#define GuiDateComboBoxTemplate_PROPERTIES(F)\
				F(GuiDateComboBoxTemplate, TemplateProperty<GuiDatePickerTemplate>, DatePickerTemplate, {})\
				
#define GuiListItemTemplate_PROPERTIES(F)\
				F(GuiListItemTemplate, bool, Selected, false)\
				F(GuiListItemTemplate, vint, Index, 0)\

/***********************************************************************
Item Template
***********************************************************************/

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