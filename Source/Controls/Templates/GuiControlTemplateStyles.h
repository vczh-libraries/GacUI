/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATESTYLES
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLTEMPLATESTYLES

#include "GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{
#pragma warning(push)
#pragma warning(disable:4250)

/***********************************************************************
Control Template
***********************************************************************/

			class GuiControlTemplate_StyleProvider
				: public Object
				, public virtual controls::GuiControl::IStyleController
				, public virtual controls::GuiControl::IStyleProvider
				, public Description<GuiControlTemplate_StyleProvider>
			{
			protected:
				controls::GuiControl::IStyleController*							associatedStyleController;
				GuiControlTemplate*												controlTemplate;

			public:
				GuiControlTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory, description::Value viewModel = description::Value());
				~GuiControlTemplate_StyleProvider();

				compositions::GuiBoundsComposition*								GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*							GetContainerComposition()override;
				void															AssociateStyleController(controls::GuiControl::IStyleController* controller)override;
				void															SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void															SetText(const WString& value)override;
				void															SetFont(const FontProperties& value)override;
				void															SetVisuallyEnabled(bool value)override;
			};

			class GuiLabelTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public controls::GuiLabel::IStyleController
				, public Description<GuiLabelTemplate_StyleProvider>
			{
			protected:
				GuiLabelTemplate*												controlTemplate;

			public:
				GuiLabelTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiLabelTemplate_StyleProvider();

				Color															GetDefaultTextColor()override;
				void															SetTextColor(Color value)override;
			};

			class GuiSinglelineTextBoxTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiSinglelineTextBox::IStyleProvider
				, public Description<GuiSinglelineTextBoxTemplate_StyleProvider>
			{
			protected:
				GuiSinglelineTextBoxTemplate*									controlTemplate;
				
			public:
				GuiSinglelineTextBoxTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiSinglelineTextBoxTemplate_StyleProvider();
				
				void															SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				compositions::GuiGraphicsComposition*							InstallBackground(compositions::GuiBoundsComposition* boundsComposition)override;
			};

			class GuiDocumentLabelTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiDocumentLabel::IStyleController
				, public Description<GuiDocumentLabelTemplate_StyleProvider>
			{
			protected:
				GuiDocumentLabelTemplate*										controlTemplate;
				
			public:
				GuiDocumentLabelTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiDocumentLabelTemplate_StyleProvider();
				
				Ptr<DocumentModel>												GetBaselineDocument()override;
			};

			class GuiMenuTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public controls::GuiWindow::DefaultBehaviorStyleController
				, public Description<GuiMenuTemplate_StyleProvider>
			{
			public:
				GuiMenuTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiMenuTemplate_StyleProvider();
			};

			class GuiWindowTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public controls::GuiWindow::IStyleController
				, public Description<GuiWindowTemplate_StyleProvider>
			{
			protected:
				GuiWindowTemplate*												controlTemplate;
				controls::GuiWindow*											window;
				Ptr<GuiTemplate::IFactory>										tooltipTemplateFactory;
				Ptr<GuiTemplate::IFactory>										shortcutKeyTemplateFactory;

			public:
				GuiWindowTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiWindowTemplate_StyleProvider();

				void															AttachWindow(controls::GuiWindow* _window)override;
				void															InitializeNativeWindowProperties()override;
				void															SetSizeState(INativeWindow::WindowSizeState value)override;
				bool															GetMaximizedBox()override;
				void															SetMaximizedBox(bool visible)override;
				bool															GetMinimizedBox()override;
				void															SetMinimizedBox(bool visible)override;
				bool															GetBorder()override;
				void															SetBorder(bool visible)override;
				bool															GetSizeBox()override;
				void															SetSizeBox(bool visible)override;
				bool															GetIconVisible()override;
				void															SetIconVisible(bool visible)override;
				bool															GetTitleBar()override;
				void															SetTitleBar(bool visible)override;
				controls::GuiWindow::IStyleController*							CreateTooltipStyle()override;
				controls::GuiLabel::IStyleController*							CreateShortcutKeyStyle()override;
			};

			class GuiButtonTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiButton::IStyleController
				, public Description<GuiButtonTemplate_StyleProvider>
			{
			protected:
				GuiButtonTemplate*												controlTemplate;

			public:
				GuiButtonTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiButtonTemplate_StyleProvider();

				void															Transfer(controls::GuiButton::ControlState value)override;
			};

			class GuiSelectableButtonTemplate_StyleProvider
				: public GuiButtonTemplate_StyleProvider
				, public virtual controls::GuiSelectableButton::IStyleController
				, public Description<GuiSelectableButtonTemplate_StyleProvider>
			{
			protected:
				GuiSelectableButtonTemplate*									controlTemplate;

			public:
				GuiSelectableButtonTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiSelectableButtonTemplate_StyleProvider();

				void															SetSelected(bool value)override;
			};

			class GuiToolstripButtonTemplate_StyleProvider
				: public GuiSelectableButtonTemplate_StyleProvider
				, public virtual controls::GuiMenuButton::IStyleController
				, public Description<GuiToolstripButtonTemplate_StyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>										subMenuTemplateFactory;
				GuiToolstripButtonTemplate*										controlTemplate;

			public:
				GuiToolstripButtonTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiToolstripButtonTemplate_StyleProvider();
				
				controls::GuiMenu::IStyleController*							CreateSubMenuStyleController()override;
				void															SetSubMenuExisting(bool value)override;
				void															SetSubMenuOpening(bool value)override;
				controls::GuiButton*											GetSubMenuHost()override;
				void															SetImage(Ptr<GuiImageData> value)override;
				void															SetShortcutText(const WString& value)override;
			};

			class GuiListViewColumnHeaderTemplate_StyleProvider
				: public GuiToolstripButtonTemplate_StyleProvider
				, public virtual controls::GuiListViewColumnHeader::IStyleController
				, public Description<GuiListViewColumnHeaderTemplate_StyleProvider>
			{
			protected:
				GuiListViewColumnHeaderTemplate*								controlTemplate;

			public:
				GuiListViewColumnHeaderTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiListViewColumnHeaderTemplate_StyleProvider();

				void															SetColumnSortingState(controls::GuiListViewColumnHeader::ColumnSortingState value)override;
			};

			class GuiComboBoxTemplate_StyleProvider
				: public GuiToolstripButtonTemplate_StyleProvider
				, public virtual controls::GuiComboBoxListControl::IStyleController
				, public Description<GuiComboBoxTemplate_StyleProvider>
			{
			protected:
				GuiComboBoxTemplate*											controlTemplate;

			public:
				GuiComboBoxTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiComboBoxTemplate_StyleProvider();
				
				void															SetCommandExecutor(controls::GuiComboBoxBase::ICommandExecutor* value)override;
				void															OnItemSelected()override;
				void															SetTextVisible(bool value)override;
			};

			class GuiTextListTemplate_StyleProvider;

			class GuiDatePickerTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiDatePicker::IStyleProvider
				, public Description<GuiDatePickerTemplate_StyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>										dateButtonTemplateFactory;
				Ptr<GuiTemplate::IFactory>										dateTextListTemplateFactory;
				Ptr<GuiTemplate::IFactory>										dateComboBoxTemplateFactory;
				GuiDatePickerTemplate*											controlTemplate;

			public:
				GuiDatePickerTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiDatePickerTemplate_StyleProvider();

				controls::GuiSelectableButton::IStyleController*				CreateDateButtonStyle()override;
				GuiTextListTemplate_StyleProvider*								CreateTextListStyle();
				controls::GuiTextList*											CreateTextList()override;
				controls::GuiComboBoxListControl::IStyleController*				CreateComboBoxStyle()override;
				Color															GetBackgroundColor()override;
				Color															GetPrimaryTextColor()override;
				Color															GetSecondaryTextColor()override;
			};

			class GuiDateComboBoxTemplate_StyleProvider
				: public GuiComboBoxTemplate_StyleProvider
				, public Description<GuiDateComboBoxTemplate_StyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>										datePickerTemplateFactory;
				GuiDateComboBoxTemplate*										controlTemplate;

			public:
				GuiDateComboBoxTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiDateComboBoxTemplate_StyleProvider();

				controls::GuiDatePicker*										CreateArgument();
				controls::GuiDatePicker::IStyleProvider*						CreateDatePickerStyle();
			};

			class GuiScrollTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiScroll::IStyleController
				, public Description<GuiScrollTemplate_StyleProvider>
			{
			protected:
				GuiScrollTemplate*												controlTemplate;

			public:
				GuiScrollTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiScrollTemplate_StyleProvider();

				void															SetCommandExecutor(controls::GuiScroll::ICommandExecutor* value)override;
				void															SetTotalSize(vint value)override;
				void															SetPageSize(vint value)override;
				void															SetPosition(vint value)override;
			};

			class GuiScrollViewTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiScrollView::IStyleProvider
				, public Description<GuiScrollViewTemplate_StyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>										hScrollTemplateFactory;
				Ptr<GuiTemplate::IFactory>										vScrollTemplateFactory;
				GuiScrollViewTemplate*											controlTemplate;
				
			public:
				GuiScrollViewTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiScrollViewTemplate_StyleProvider();
				
				controls::GuiScroll::IStyleController*							CreateHorizontalScrollStyle()override;
				controls::GuiScroll::IStyleController*							CreateVerticalScrollStyle()override;
				vint															GetDefaultScrollSize()override;
				compositions::GuiGraphicsComposition*							InstallBackground(compositions::GuiBoundsComposition* boundsComposition)override;
			};

			class GuiMultilineTextBoxTemplate_StyleProvider
				: public GuiScrollViewTemplate_StyleProvider
				, public Description<GuiMultilineTextBoxTemplate_StyleProvider>
			{
			protected:
				GuiMultilineTextBoxTemplate*									controlTemplate;
				
			public:
				GuiMultilineTextBoxTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiMultilineTextBoxTemplate_StyleProvider();
				
				void															SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
			};

			class GuiDocumentViewerTemplate_StyleProvider
				: public GuiScrollViewTemplate_StyleProvider
				, public virtual controls::GuiDocumentViewer::IStyleProvider
				, public Description<GuiDocumentViewerTemplate_StyleProvider>
			{
			protected:
				GuiDocumentViewerTemplate*										controlTemplate;
				
			public:
				GuiDocumentViewerTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiDocumentViewerTemplate_StyleProvider();
				
				Ptr<DocumentModel>												GetBaselineDocument()override;
			};

			class GuiTextListTemplate_StyleProvider
				: public GuiScrollViewTemplate_StyleProvider
				, public virtual controls::GuiVirtualTextList::IStyleProvider
				, public Description<GuiTextListTemplate_StyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>										backgroundTemplateFactory;
				Ptr<GuiTemplate::IFactory>										bulletTemplateFactory;
				GuiTextListTemplate*											controlTemplate;
				
				class ItemStyleProvider
					: public Object
					, public virtual controls::list::TextItemStyleProvider::IBulletFactory
				{
				protected:
					GuiTextListTemplate_StyleProvider*							styleProvider;

				public:
					ItemStyleProvider(GuiTextListTemplate_StyleProvider* _styleProvider);
					~ItemStyleProvider();

					controls::GuiSelectableButton::IStyleController*			CreateBulletStyleController()override;
				};
			public:
				GuiTextListTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiTextListTemplate_StyleProvider();
				
				controls::GuiSelectableButton::IStyleController*				CreateItemBackground()override;
				Color															GetTextColor()override;
				
				controls::list::TextItemStyleProvider::IBulletFactory*			CreateArgument();
				controls::GuiSelectableButton::IStyleController*				CreateBulletStyle();
			};

			class GuiListViewTemplate_StyleProvider
				: public GuiScrollViewTemplate_StyleProvider
				, public virtual controls::GuiListViewBase::IStyleProvider
				, public Description<GuiListViewTemplate_StyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>										backgroundTemplateFactory;
				Ptr<GuiTemplate::IFactory>										columnHeaderTemplateFactory;
				GuiListViewTemplate*											controlTemplate;
				
			public:
				GuiListViewTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiListViewTemplate_StyleProvider();
				
				controls::GuiSelectableButton::IStyleController*				CreateItemBackground()override;
				controls::GuiListViewColumnHeader::IStyleController*			CreateColumnStyle()override;
				Color															GetPrimaryTextColor()override;
				Color															GetSecondaryTextColor()override;
				Color															GetItemSeparatorColor()override;
			};

			class GuiTreeViewTemplate_StyleProvider
				: public GuiScrollViewTemplate_StyleProvider
				, public virtual controls::GuiVirtualTreeView::IStyleProvider
				, public Description<GuiTreeViewTemplate_StyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>										backgroundTemplateFactory;
				Ptr<GuiTemplate::IFactory>										expandingDecoratorTemplateFactory;
				GuiTreeViewTemplate*											controlTemplate;
				
			public:
				GuiTreeViewTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiTreeViewTemplate_StyleProvider();
				
				controls::GuiSelectableButton::IStyleController*				CreateItemBackground()override;
				controls::GuiSelectableButton::IStyleController*				CreateItemExpandingDecorator()override;
				Color															GetTextColor()override;
			};

			class GuiTabTemplate_StyleProvider
				: public GuiControlTemplate_StyleProvider
				, public virtual controls::GuiTab::IStyleController
				, public Description<GuiTabTemplate_StyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>										headerTemplateFactory;
				Ptr<GuiTemplate::IFactory>										dropdownTemplateFactory;
				Ptr<GuiTemplate::IFactory>										menuTemplateFactory;
				Ptr<GuiTemplate::IFactory>										menuItemTemplateFactory;
				GuiTabTemplate*													controlTemplate;
				
				compositions::GuiTableComposition*								tabBoundsComposition;
				compositions::GuiStackComposition*								tabHeaderComposition;
				compositions::GuiBoundsComposition*								tabContentTopLineComposition;
				controls::GuiTab::ICommandExecutor*								commandExecutor;

				Ptr<controls::GuiSelectableButton::MutexGroupController>		headerController;
				collections::List<controls::GuiSelectableButton*>				headerButtons;
				controls::GuiButton*											headerOverflowButton;
				controls::GuiToolstripMenu*										headerOverflowMenu;
				
				controls::GuiSelectableButton::IStyleController*				CreateHeaderTemplate();
				controls::GuiButton::IStyleController*							CreateDropdownTemplate();
				controls::GuiMenu::IStyleController*							CreateMenuTemplate();
				controls::GuiToolstripButton::IStyleController*					CreateMenuItemTemplate();

				void															OnHeaderButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void															OnTabHeaderBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void															OnHeaderOverflowButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void															OnHeaderOverflowMenuButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

				void															UpdateHeaderOverflowButtonVisibility();
				void															UpdateHeaderZOrder();
				void															UpdateHeaderVisibilityIndex();
				void															UpdateHeaderLayout();

				void															Initialize();
			public:
				GuiTabTemplate_StyleProvider(Ptr<GuiTemplate::IFactory> factory);
				~GuiTabTemplate_StyleProvider();

				void															SetCommandExecutor(controls::GuiTab::ICommandExecutor* value)override;
				void															InsertTab(vint index)override;
				void															SetTabText(vint index, const WString& value)override;
				void															RemoveTab(vint index)override;
				void															MoveTab(vint oldIndex, vint newIndex)override;
				void															SetSelectedTab(vint index)override;
				void															SetTabAlt(vint index, const WString& value, compositions::IGuiAltActionHost* host)override;
				compositions::IGuiAltAction*									GetTabAltAction(vint index)override;
			};

/***********************************************************************
Item Template (GuiControlTemplate)
***********************************************************************/

			class GuiControlTemplate_ItemStyleProvider
				: public Object
				, public virtual controls::GuiComboBoxListControl::IItemStyleProvider
				, public Description<GuiControlTemplate_ItemStyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>							factory;

			public:
				GuiControlTemplate_ItemStyleProvider(Ptr<GuiTemplate::IFactory> _factory);
				~GuiControlTemplate_ItemStyleProvider();

				void												AttachComboBox(controls::GuiComboBoxListControl* value)override;
				void												DetachComboBox()override;
				controls::GuiControl::IStyleController*				CreateItemStyle(description::Value item)override;
			};

/***********************************************************************
Item Template (GuiListItemTemplate)
***********************************************************************/

			class GuiListItemTemplate_ItemStyleProvider
				: public Object
				, public virtual controls::GuiSelectableListControl::IItemStyleProvider
				, public Description<GuiListItemTemplate_ItemStyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>							factory;
				controls::GuiListControl*							listControl;
				controls::GuiListControl::IItemBindingView*			bindingView;

			public:
				GuiListItemTemplate_ItemStyleProvider(Ptr<GuiTemplate::IFactory> _factory);
				~GuiListItemTemplate_ItemStyleProvider();

				void												AttachListControl(controls::GuiListControl* value)override;
				void												DetachListControl()override;
				vint												GetItemStyleId(vint itemIndex)override;
				controls::GuiListControl::IItemStyleController*		CreateItemStyle(vint styleId)override;
				void												DestroyItemStyle(controls::GuiListControl::IItemStyleController* style)override;
				void												Install(controls::GuiListControl::IItemStyleController* style, vint itemIndex)override;
				void												SetStyleIndex(controls::GuiListControl::IItemStyleController* style, vint value)override;
				void												SetStyleSelected(controls::GuiListControl::IItemStyleController* style, bool value)override;
			};

			class GuiListItemTemplate_ItemStyleController
				: public Object
				, public virtual controls::GuiListControl::IItemStyleController
				, public Description<GuiListItemTemplate_ItemStyleController>
			{
			protected:
				GuiListItemTemplate_ItemStyleProvider*				itemStyleProvider;
				GuiListItemTemplate*								itemTemplate;
				bool												installed;

			public:
				GuiListItemTemplate_ItemStyleController(GuiListItemTemplate_ItemStyleProvider* _itemStyleProvider);
				~GuiListItemTemplate_ItemStyleController();

				GuiListItemTemplate*								GetTemplate();
				void												SetTemplate(GuiListItemTemplate* _itemTemplate);

				controls::GuiListControl::IItemStyleProvider*		GetStyleProvider()override;
				vint												GetItemStyleId()override;
				compositions::GuiBoundsComposition*					GetBoundsComposition()override;
				bool												IsCacheable()override;
				bool												IsInstalled()override;
				void												OnInstalled()override;
				void												OnUninstalled()override;
			};

/***********************************************************************
Item Template (GuiTreeItemTemplate)
***********************************************************************/

			class GuiTreeItemTemplate_ItemStyleProvider
				: public Object
				, public virtual controls::tree::INodeItemStyleProvider
				, protected virtual controls::tree::INodeProviderCallback
				, public Description<GuiTreeItemTemplate_ItemStyleProvider>
			{
			protected:
				Ptr<GuiTemplate::IFactory>							factory;
				controls::GuiVirtualTreeListControl*				treeListControl;
				controls::tree::INodeItemBindingView*				bindingView;
				controls::GuiListControl::IItemStyleProvider*		itemStyleProvider;
				
				void												UpdateExpandingButton(controls::tree::INodeProvider* node);
				void												OnAttached(controls::tree::INodeRootProvider* provider)override;
				void												OnBeforeItemModified(controls::tree::INodeProvider* parentNode, vint start, vint count, vint newCount)override;
				void												OnAfterItemModified(controls::tree::INodeProvider* parentNode, vint start, vint count, vint newCount)override;
				void												OnItemExpanded(controls::tree::INodeProvider* node)override;
				void												OnItemCollapsed(controls::tree::INodeProvider* node)override;

			public:
				GuiTreeItemTemplate_ItemStyleProvider(Ptr<GuiTemplate::IFactory> _factory);
				~GuiTreeItemTemplate_ItemStyleProvider();
				
				void												BindItemStyleProvider(controls::GuiListControl::IItemStyleProvider* styleProvider)override;
				controls::GuiListControl::IItemStyleProvider*		GetBindedItemStyleProvider()override;
				void												AttachListControl(controls::GuiListControl* value)override;
				void												DetachListControl()override;
				vint												GetItemStyleId(controls::tree::INodeProvider* node)override;
				controls::tree::INodeItemStyleController*			CreateItemStyle(vint styleId)override;
				void												DestroyItemStyle(controls::tree::INodeItemStyleController* style)override;
				void												Install(controls::tree::INodeItemStyleController* style, controls::tree::INodeProvider* node, vint itemIndex)override;
				void												SetStyleIndex(controls::tree::INodeItemStyleController* style, vint value)override;
				void												SetStyleSelected(controls::tree::INodeItemStyleController* style, bool value)override;
			};
			
			class GuiTreeItemTemplate_ItemStyleController
				: public GuiListItemTemplate_ItemStyleController
				, public virtual controls::tree::INodeItemStyleController
				, public Description<GuiTreeItemTemplate_ItemStyleController>
			{
			protected:
				GuiTreeItemTemplate_ItemStyleProvider*				nodeStyleProvider;

			public:
				GuiTreeItemTemplate_ItemStyleController(GuiTreeItemTemplate_ItemStyleProvider* _nodeStyleProvider);
				~GuiTreeItemTemplate_ItemStyleController();
				
				controls::GuiListControl::IItemStyleProvider*		GetStyleProvider()override;
				controls::tree::INodeItemStyleProvider*				GetNodeStyleProvider()override;
			};

/***********************************************************************
Item Template (GuiGridVisualizerTemplate)
***********************************************************************/

			/// <summary>Data visualizer object for [T:vl.presentation.controls.GuiBindableDataGrid].</summary>
			class GuiBindableDataVisualizer : public controls::list::DataVisualizerBase, public Description<GuiBindableDataVisualizer>
			{
			public:
				class Factory : public controls::list::DataVisualizerFactory<GuiBindableDataVisualizer>
				{
				protected:
					Ptr<GuiTemplate::IFactory>						templateFactory;
					controls::list::BindableDataColumn*				ownerColumn;

				public:
					Factory(Ptr<GuiTemplate::IFactory> _templateFactory, controls::list::BindableDataColumn* _ownerColumn);
					~Factory();

					Ptr<controls::list::IDataVisualizer>			CreateVisualizer(const FontProperties& font, controls::GuiListViewBase::IStyleProvider* styleProvider)override;
				};

				class DecoratedFactory : public controls::list::DataDecoratableVisualizerFactory<GuiBindableDataVisualizer>
				{
				protected:
					Ptr<GuiTemplate::IFactory>						templateFactory;
					controls::list::BindableDataColumn*				ownerColumn;

				public:
					DecoratedFactory(Ptr<GuiTemplate::IFactory> _templateFactory, controls::list::BindableDataColumn* _ownerColumn, Ptr<controls::list::IDataVisualizerFactory> _decoratedFactory);
					~DecoratedFactory();

					Ptr<controls::list::IDataVisualizer>			CreateVisualizer(const FontProperties& font, controls::GuiListViewBase::IStyleProvider* styleProvider)override;
				};

			protected:
				Ptr<GuiTemplate::IFactory>							templateFactory;
				controls::list::BindableDataColumn*					ownerColumn = nullptr;
				GuiGridVisualizerTemplate*							visualizerTemplate = nullptr;

				compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)override;
			public:
				GuiBindableDataVisualizer();
				GuiBindableDataVisualizer(Ptr<controls::list::IDataVisualizer> _decoratedVisualizer);
				~GuiBindableDataVisualizer();

				void												BeforeVisualizeCell(controls::list::IDataProvider* dataProvider, vint row, vint column)override;
				void												SetSelected(bool value)override;
			};

/***********************************************************************
Item Template (GuiGridEditorTemplate)
***********************************************************************/

			class GuiBindableDataEditor : public controls::list::DataEditorBase, public Description<GuiBindableDataEditor>
			{
			public:
				class Factory : public controls::list::DataEditorFactory<GuiBindableDataEditor>
				{
				protected:
					Ptr<GuiTemplate::IFactory>						templateFactory;
					controls::list::BindableDataColumn*				ownerColumn;

				public:
					Factory(Ptr<GuiTemplate::IFactory> _templateFactory, controls::list::BindableDataColumn* _ownerColumn);
					~Factory();

					Ptr<controls::list::IDataEditor>				CreateEditor(controls::list::IDataEditorCallback* callback)override;
				};

			protected:
				Ptr<GuiTemplate::IFactory>							templateFactory;
				controls::list::BindableDataColumn*					ownerColumn = nullptr;
				GuiGridEditorTemplate*								editorTemplate = nullptr;

				compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal()override;
				void												editorTemplate_CellValueChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				GuiBindableDataEditor();
				~GuiBindableDataEditor();

				void												BeforeEditCell(controls::list::IDataProvider* dataProvider, vint row, vint column)override;
				description::Value									GetEditedCellValue();
			};

/***********************************************************************
Helper Functions
***********************************************************************/

			extern void												SplitBySemicolon(const WString& input, collections::List<WString>& fragments);
			extern Ptr<GuiTemplate::IFactory>						CreateTemplateFactory(const WString& typeValues);

#pragma warning(pop)
		}
	}
}

#endif