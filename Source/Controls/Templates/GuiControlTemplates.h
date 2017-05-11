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
		template<typename T>
		using ItemProperty = Func<T(const reflection::description::Value&)>;

		template<typename T>
		using WritableItemProperty = Func<T(const reflection::description::Value&, T, bool)>;

		template<typename T>
		using TemplateProperty = Func<T*(const reflection::description::Value&)>;

		namespace controls
		{
			class GuiListControl;

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

				void											FinalizeInstance();
			public:
				GuiInstanceRootObject();
				~GuiInstanceRootObject();

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
				/// <summary>Remove a subscription.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="subscription">The subscription to test.</param>
				bool											RemoveSubscription(Ptr<description::IValueSubscription> subscription);
				/// <summary>Test does the window contain the subscription.</summary>
				/// <returns>Returns true if the window contains the subscription.</returns>
				/// <param name="subscription">The subscription to test.</param>
				bool											ContainsSubscription(Ptr<description::IValueSubscription> subscription);
				/// <summary>Clear all subscriptions.</summary>
				void											ClearSubscriptions();

				/// <summary>Add a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="component">The component to add.</param>
				bool											AddComponent(GuiComponent* component);

				/// <summary>Add a control host as a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="controlHost">The controlHost to add.</param>
				bool											AddControlHostComponent(GuiControlHost* controlHost);
				/// <summary>Remove a component.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="component">The component to remove.</param>
				bool											RemoveComponent(GuiComponent* component);
				/// <summary>Test does the window contain the component.</summary>
				/// <returns>Returns true if the window contains the component.</returns>
				/// <param name="component">The component to test.</param>
				bool											ContainsComponent(GuiComponent* component);
				/// <summary>Clear all components.</summary>
				void											ClearComponents();
			};

			class GuiButton;
		}

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
				/// <summary>Create a template.</summary>
				GuiTemplate();
				~GuiTemplate();
				
#define GuiTemplate_PROPERTIES(F)\
				F(GuiTemplate, FontProperties, Font)\
				F(GuiTemplate, WString, Text)\
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
				F(GuiWindowTemplate, TemplateProperty<GuiWindowTemplate>, TooltipTemplate)\
				F(GuiWindowTemplate, TemplateProperty<GuiLabelTemplate>, ShortcutKeyTemplate)

				GuiWindowTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiButtonTemplate : public GuiControlTemplate, public AggregatableDescription<GuiButtonTemplate>
			{
			public:
				GuiButtonTemplate();
				~GuiButtonTemplate();

#define GuiButtonTemplate_PROPERTIES(F)\
				F(GuiButtonTemplate, controls::ButtonState, State)\

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
				F(GuiToolstripButtonTemplate, TemplateProperty<GuiMenuTemplate>, SubMenuTemplate)\
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
				F(GuiListViewColumnHeaderTemplate, controls::ColumnSortingState, SortingState)\

				GuiListViewColumnHeaderTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiComboBoxTemplate : public GuiToolstripButtonTemplate, public AggregatableDescription<GuiComboBoxTemplate>
			{
			public:
				GuiComboBoxTemplate();
				~GuiComboBoxTemplate();

#define GuiComboBoxTemplate_PROPERTIES(F)\
				F(GuiComboBoxTemplate, controls::IComboBoxCommandExecutor*, Commands)\
				F(GuiComboBoxTemplate, bool, TextVisible)\

				GuiComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiScrollTemplate : public GuiControlTemplate, public AggregatableDescription<GuiScrollTemplate>
			{
			public:
				GuiScrollTemplate();
				~GuiScrollTemplate();

#define GuiScrollTemplate_PROPERTIES(F)\
				F(GuiScrollTemplate, controls::IScrollCommandExecutor*, Commands)\
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
				F(GuiScrollViewTemplate, TemplateProperty<GuiScrollTemplate>, HScrollTemplate)\
				F(GuiScrollViewTemplate, TemplateProperty<GuiScrollTemplate>, VScrollTemplate)\
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
				F(GuiDocumentViewerTemplate, Color, CaretColor)\

				GuiDocumentViewerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiTextListTemplate : public GuiScrollViewTemplate, public AggregatableDescription<GuiTextListTemplate>
			{
			public:
				GuiTextListTemplate();
				~GuiTextListTemplate();

#define GuiTextListTemplate_PROPERTIES(F)\
				F(GuiTextListTemplate, Color, TextColor)\

				GuiTextListTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiListViewTemplate : public GuiScrollViewTemplate, public AggregatableDescription<GuiListViewTemplate>
			{
			public:
				GuiListViewTemplate();
				~GuiListViewTemplate();

#define GuiListViewTemplate_PROPERTIES(F)\
				F(GuiListViewTemplate, TemplateProperty<GuiSelectableButtonTemplate>, BackgroundTemplate)\
				F(GuiListViewTemplate, TemplateProperty<GuiListViewColumnHeaderTemplate>, ColumnHeaderTemplate)\
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
				F(GuiTreeViewTemplate, TemplateProperty<GuiSelectableButtonTemplate>, BackgroundTemplate)\
				F(GuiTreeViewTemplate, TemplateProperty<GuiSelectableButtonTemplate>, ExpandingDecoratorTemplate)\
				F(GuiTreeViewTemplate, Color, TextColor)\

				GuiTreeViewTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiTabTemplate : public GuiControlTemplate, public AggregatableDescription<GuiTabTemplate>
			{
			public:
				GuiTabTemplate();
				~GuiTabTemplate();

#define GuiTabTemplate_PROPERTIES(F)\
				F(GuiTabTemplate, TemplateProperty<GuiSelectableButtonTemplate>, HeaderTemplate)\
				F(GuiTabTemplate, TemplateProperty<GuiButtonTemplate>, DropdownTemplate)\
				F(GuiTabTemplate, TemplateProperty<GuiMenuTemplate>, MenuTemplate)\
				F(GuiTabTemplate, TemplateProperty<GuiToolstripButtonTemplate>, MenuItemTemplate)\
				F(GuiTabTemplate, vint, HeaderPadding)\
				F(GuiTabTemplate, compositions::GuiGraphicsComposition*, HeaderComposition)\

				GuiTabTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiDatePickerTemplate : public GuiControlTemplate, public AggregatableDescription<GuiDatePickerTemplate>
			{
			public:
				GuiDatePickerTemplate();
				~GuiDatePickerTemplate();

#define GuiDatePickerTemplate_PROPERTIES(F)\
				F(GuiDatePickerTemplate, TemplateProperty<GuiSelectableButtonTemplate>, DateButtonTemplate)\
				F(GuiDatePickerTemplate, TemplateProperty<GuiTextListTemplate>, DateTextListTemplate)\
				F(GuiDatePickerTemplate, TemplateProperty<GuiComboBoxTemplate>, DateComboBoxTemplate)\
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
				F(GuiDateComboBoxTemplate, TemplateProperty<GuiDatePickerTemplate>, DatePickerTemplate)\

				GuiDateComboBoxTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

/***********************************************************************
Item Template
***********************************************************************/

			class GuiListItemTemplate : public GuiTemplate, public AggregatableDescription<GuiListItemTemplate>
			{
			protected:
				controls::GuiListControl*	listControl = nullptr;
				bool						initialized = false;

				virtual void				OnInitialize();

			public:
				GuiListItemTemplate();
				~GuiListItemTemplate();
				
#define GuiListItemTemplate_PROPERTIES(F)\
				F(GuiListItemTemplate, bool, Selected)\
				F(GuiListItemTemplate, vint, Index)\

				GuiListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)

				void						BeginEditListItem();
				void						EndEditListItem();
				void						Initialize(controls::GuiListControl* _listControl);
			};

			class GuiTextListItemTemplate : public GuiListItemTemplate, public AggregatableDescription<GuiTextListItemTemplate>
			{
			public:
				GuiTextListItemTemplate();
				~GuiTextListItemTemplate();
				
#define GuiTextListItemTemplate_PROPERTIES(F)\
				F(GuiTextListItemTemplate, Color, TextColor)\
				F(GuiTextListItemTemplate, bool, Checked)\

				GuiTextListItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiTreeItemTemplate : public GuiTextListItemTemplate, public AggregatableDescription<GuiTreeItemTemplate>
			{
			public:
				GuiTreeItemTemplate();
				~GuiTreeItemTemplate();
				
#define GuiTreeItemTemplate_PROPERTIES(F)\
				F(GuiTreeItemTemplate, bool, Expanding)\
				F(GuiTreeItemTemplate, bool, Expandable)\
				F(GuiTreeItemTemplate, vint, Level)\
				F(GuiTreeItemTemplate, Ptr<GuiImageData>, Image)\

				GuiTreeItemTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiGridCellTemplate : public GuiControlTemplate, public AggregatableDescription<GuiGridCellTemplate>
			{
			public:
				GuiGridCellTemplate();
				~GuiGridCellTemplate();

#define GuiGridCellTemplate_PROPERTIES(F)\
				F(GuiGridCellTemplate, Color, PrimaryTextColor)\
				F(GuiGridCellTemplate, Color, SecondaryTextColor)\
				F(GuiGridCellTemplate, Color, ItemSeparatorColor)\
				F(GuiGridCellTemplate, Ptr<GuiImageData>, LargeImage)\
				F(GuiGridCellTemplate, Ptr<GuiImageData>, SmallImage)\
				F(GuiGridCellTemplate, description::Value, RowValue)\
				F(GuiGridCellTemplate, description::Value, CellValue)\

				GuiGridCellTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiGridVisualizerTemplate : public GuiGridCellTemplate, public AggregatableDescription<GuiGridVisualizerTemplate>
			{
			public:
				GuiGridVisualizerTemplate();
				~GuiGridVisualizerTemplate();

#define GuiGridVisualizerTemplate_PROPERTIES(F)\
				F(GuiGridVisualizerTemplate, bool, Selected)\

				GuiGridVisualizerTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};

			class GuiGridEditorTemplate : public GuiGridCellTemplate, public AggregatableDescription<GuiGridEditorTemplate>
			{
			public:
				GuiGridEditorTemplate();
				~GuiGridEditorTemplate();

#define GuiGridEditorTemplate_PROPERTIES(F)\

				GuiGridEditorTemplate_PROPERTIES(GUI_TEMPLATE_PROPERTY_DECL)
			};
		}
	}
}

#endif