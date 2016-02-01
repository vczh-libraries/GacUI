/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Basic

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONCONTROLS
#define VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONCONTROLS

#include "GuiReflectionElements.h"
#include "GuiReflectionCompositions.h"
#include "../../Controls/TextEditorPackage/LanguageService/GuiLanguageColorizer.h"
#include "../../Controls/TextEditorPackage/LanguageService/GuiLanguageAutoComplete.h"

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

#define GUIREFLECTIONCONTROLS_TYPELIST(F)\
			F(presentation::controls::GuiApplication)\
			F(presentation::theme::ITheme)\
			F(presentation::controls::GuiInstanceRootObject)\
			F(presentation::controls::GuiDialogBase)\
			F(presentation::controls::GuiMessageDialog)\
			F(presentation::controls::GuiColorDialog)\
			F(presentation::controls::GuiFontDialog)\
			F(presentation::controls::GuiFileDialogBase)\
			F(presentation::controls::GuiOpenFileDialog)\
			F(presentation::controls::GuiSaveFileDialog)\
			F(presentation::controls::GuiCustomControl)\
			F(presentation::controls::GuiLabel)\
			F(presentation::controls::GuiLabel::IStyleController)\
			F(presentation::controls::GuiButton)\
			F(presentation::controls::GuiButton::ControlState)\
			F(presentation::controls::GuiButton::IStyleController)\
			F(presentation::controls::GuiSelectableButton)\
			F(presentation::controls::GuiSelectableButton::IStyleController)\
			F(presentation::controls::GuiSelectableButton::GroupController)\
			F(presentation::controls::GuiSelectableButton::MutexGroupController)\
			F(presentation::controls::GuiScroll)\
			F(presentation::controls::GuiScroll::ICommandExecutor)\
			F(presentation::controls::GuiScroll::IStyleController)\
			F(presentation::controls::GuiTabPage)\
			F(presentation::controls::GuiTab)\
			F(presentation::controls::GuiTab::ICommandExecutor)\
			F(presentation::controls::GuiTab::IStyleController)\
			F(presentation::controls::GuiScrollView)\
			F(presentation::controls::GuiScrollView::IStyleProvider)\
			F(presentation::controls::GuiScrollContainer)\
			F(presentation::controls::GuiWindow)\
			F(presentation::controls::GuiWindow::IStyleController)\
			F(presentation::controls::GuiPopup)\
			F(presentation::controls::GuiTooltip)\
			F(presentation::controls::GuiListControl)\
			F(presentation::controls::GuiListControl::IItemProviderCallback)\
			F(presentation::controls::GuiListControl::IItemArrangerCallback)\
			F(presentation::controls::GuiListControl::IItemPrimaryTextView)\
			F(presentation::controls::GuiListControl::IItemBindingView)\
			F(presentation::controls::GuiListControl::IItemProvider)\
			F(presentation::controls::GuiListControl::IItemStyleController)\
			F(presentation::controls::GuiListControl::IItemStyleProvider)\
			F(presentation::controls::GuiListControl::IItemArranger)\
			F(presentation::controls::GuiSelectableListControl)\
			F(presentation::controls::GuiSelectableListControl::IItemStyleProvider)\
			F(presentation::controls::list::RangedItemArrangerBase)\
			F(presentation::controls::list::FixedHeightItemArranger)\
			F(presentation::controls::list::FixedSizeMultiColumnItemArranger)\
			F(presentation::controls::list::FixedHeightMultiColumnItemArranger)\
			F(presentation::controls::list::ItemStyleControllerBase)\
			F(presentation::controls::list::TextItemStyleProvider)\
			F(presentation::controls::list::TextItemStyleProvider::ITextItemStyleProvider)\
			F(presentation::controls::list::TextItemStyleProvider::ITextItemView)\
			F(presentation::controls::list::TextItemStyleProvider::TextItemStyleController)\
			F(presentation::controls::list::TextItem)\
			F(presentation::controls::GuiVirtualTextList)\
			F(presentation::controls::GuiTextList)\
			F(presentation::controls::list::ListViewItemStyleProviderBase)\
			F(presentation::controls::list::ListViewItemStyleProviderBase::ListViewItemStyleController)\
			F(presentation::controls::GuiListViewColumnHeader)\
			F(presentation::controls::GuiListViewColumnHeader::ColumnSortingState)\
			F(presentation::controls::GuiListViewColumnHeader::IStyleController)\
			F(presentation::controls::GuiListViewBase)\
			F(presentation::controls::GuiListViewBase::IStyleProvider)\
			F(presentation::controls::list::ListViewItemStyleProvider)\
			F(presentation::controls::list::ListViewItemStyleProvider::IListViewItemView)\
			F(presentation::controls::list::ListViewItemStyleProvider::IListViewItemContent)\
			F(presentation::controls::list::ListViewItemStyleProvider::IListViewItemContentProvider)\
			F(presentation::controls::list::ListViewItemStyleProvider::ListViewContentItemStyleController)\
			F(presentation::controls::list::ListViewBigIconContentProvider)\
			F(presentation::controls::list::ListViewSmallIconContentProvider)\
			F(presentation::controls::list::ListViewListContentProvider)\
			F(presentation::controls::list::ListViewTileContentProvider)\
			F(presentation::controls::list::ListViewInformationContentProvider)\
			F(presentation::controls::list::ListViewColumnItemArranger)\
			F(presentation::controls::list::ListViewColumnItemArranger::IColumnItemViewCallback)\
			F(presentation::controls::list::ListViewColumnItemArranger::IColumnItemView)\
			F(presentation::controls::list::ListViewDetailContentProvider)\
			F(presentation::controls::list::ListViewItem)\
			F(presentation::controls::list::ListViewColumn)\
			F(presentation::controls::GuiVirtualListView)\
			F(presentation::controls::GuiListView)\
			F(presentation::controls::IGuiMenuService)\
			F(presentation::controls::IGuiMenuService::Direction)\
			F(presentation::controls::GuiMenu)\
			F(presentation::controls::GuiMenuBar)\
			F(presentation::controls::GuiMenuButton)\
			F(presentation::controls::GuiMenuButton::IStyleController)\
			F(presentation::controls::tree::INodeProviderCallback)\
			F(presentation::controls::tree::INodeProvider)\
			F(presentation::controls::tree::INodeRootProvider)\
			F(presentation::controls::tree::INodeItemView)\
			F(presentation::controls::tree::INodeItemPrimaryTextView)\
			F(presentation::controls::tree::INodeItemBindingView)\
			F(presentation::controls::tree::INodeItemStyleController)\
			F(presentation::controls::tree::INodeItemStyleProvider)\
			F(presentation::controls::tree::NodeItemStyleProvider)\
			F(presentation::controls::tree::IMemoryNodeData)\
			F(presentation::controls::tree::MemoryNodeProvider)\
			F(presentation::controls::tree::NodeRootProviderBase)\
			F(presentation::controls::tree::MemoryNodeRootProvider)\
			F(presentation::controls::GuiVirtualTreeListControl)\
			F(presentation::controls::tree::ITreeViewItemView)\
			F(presentation::controls::tree::TreeViewItem)\
			F(presentation::controls::tree::TreeViewItemRootProvider)\
			F(presentation::controls::GuiVirtualTreeView)\
			F(presentation::controls::GuiVirtualTreeView::IStyleProvider)\
			F(presentation::controls::GuiTreeView)\
			F(presentation::controls::tree::TreeViewNodeItemStyleProvider)\
			F(presentation::controls::GuiComboBoxBase)\
			F(presentation::controls::GuiComboBoxBase::ICommandExecutor)\
			F(presentation::controls::GuiComboBoxBase::IStyleController)\
			F(presentation::controls::GuiComboBoxListControl)\
			F(presentation::controls::GuiToolstripCommand)\
			F(presentation::controls::GuiToolstripMenu)\
			F(presentation::controls::GuiToolstripMenuBar)\
			F(presentation::controls::GuiToolstripToolBar)\
			F(presentation::controls::GuiToolstripButton)\
			F(presentation::controls::GuiDocumentItem)\
			F(presentation::controls::GuiDocumentCommonInterface)\
			F(presentation::controls::GuiDocumentCommonInterface::EditMode)\
			F(presentation::controls::GuiDocumentViewer)\
			F(presentation::controls::GuiDocumentViewer::IStyleProvider)\
			F(presentation::controls::GuiDocumentLabel)\
			F(presentation::controls::GuiDocumentLabel::IStyleController)\
			F(presentation::controls::GuiTextBoxCommonInterface)\
			F(presentation::controls::GuiMultilineTextBox)\
			F(presentation::controls::GuiSinglelineTextBox)\
			F(presentation::controls::GuiSinglelineTextBox::IStyleProvider)\
			F(presentation::controls::list::IDataVisualizerFactory)\
			F(presentation::controls::list::IDataVisualizer)\
			F(presentation::controls::list::IDataEditorCallback)\
			F(presentation::controls::list::IDataEditorFactory)\
			F(presentation::controls::list::IDataEditor)\
			F(presentation::controls::list::IDataProviderCommandExecutor)\
			F(presentation::controls::list::IDataProvider)\
			F(presentation::controls::list::IStructuredDataFilterCommandExecutor)\
			F(presentation::controls::list::IStructuredDataFilter)\
			F(presentation::controls::list::IStructuredDataSorter)\
			F(presentation::controls::list::IStructuredColumnProvider)\
			F(presentation::controls::list::IStructuredDataProvider)\
			F(presentation::controls::list::DataGridContentProvider)\
			F(presentation::controls::GuiVirtualDataGrid)\
			F(presentation::controls::list::StructuredDataFilterBase)\
			F(presentation::controls::list::StructuredDataMultipleFilter)\
			F(presentation::controls::list::StructuredDataAndFilter)\
			F(presentation::controls::list::StructuredDataOrFilter)\
			F(presentation::controls::list::StructuredDataNotFilter)\
			F(presentation::controls::list::StructuredDataMultipleSorter)\
			F(presentation::controls::list::StructuredDataReverseSorter)\
			F(presentation::controls::list::StructuredDataProvider)\
			F(presentation::controls::list::StructuredColummProviderBase)\
			F(presentation::controls::list::StructuredDataProviderBase)\
			F(presentation::controls::list::ListViewMainColumnDataVisualizer)\
			F(presentation::controls::list::ListViewMainColumnDataVisualizer::Factory)\
			F(presentation::controls::list::ListViewSubColumnDataVisualizer)\
			F(presentation::controls::list::ListViewSubColumnDataVisualizer::Factory)\
			F(presentation::controls::list::HyperlinkDataVisualizer)\
			F(presentation::controls::list::HyperlinkDataVisualizer::Factory)\
			F(presentation::controls::list::ImageDataVisualizer)\
			F(presentation::controls::list::ImageDataVisualizer::Factory)\
			F(presentation::controls::list::CellBorderDataVisualizer)\
			F(presentation::controls::list::CellBorderDataVisualizer::Factory)\
			F(presentation::controls::list::NotifyIconDataVisualizer)\
			F(presentation::controls::list::NotifyIconDataVisualizer::Factory)\
			F(presentation::controls::list::TextBoxDataEditor)\
			F(presentation::controls::list::TextBoxDataEditor::Factory)\
			F(presentation::controls::list::TextComboBoxDataEditor)\
			F(presentation::controls::list::TextComboBoxDataEditor::Factory)\
			F(presentation::controls::list::DateComboBoxDataEditor)\
			F(presentation::controls::list::DateComboBoxDataEditor::Factory)\
			F(presentation::controls::GuiDatePicker)\
			F(presentation::controls::GuiDatePicker::IStyleProvider)\
			F(presentation::controls::GuiDateComboBox)\
			F(presentation::controls::GuiStringGrid)\
			F(presentation::controls::list::StringGridProvider)\
			F(presentation::controls::GuiBindableTextList)\
			F(presentation::controls::GuiBindableListView)\
			F(presentation::controls::GuiBindableTreeView)\
			F(presentation::controls::list::BindableDataColumn)\
			F(presentation::controls::GuiBindableDataGrid)\

			GUIREFLECTIONCONTROLS_TYPELIST(DECL_TYPE_INFO)

/***********************************************************************
Interface Proxy
***********************************************************************/

#pragma warning(push)
#pragma warning(disable:4250)
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiCustomControl::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)
			END_INTERFACE_PROXY(presentation::controls::GuiCustomControl::IStyleController)
			
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiLabel::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)

				Color GetDefaultTextColor()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetDefaultTextColor);
				}

				void SetTextColor(Color value)override
				{
					INVOKE_INTERFACE_PROXY(SetTextColor, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiLabel::IStyleController)
			
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiButton::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)

				void Transfer(GuiButton::ControlState value)override
				{
					INVOKE_INTERFACE_PROXY(Transfer, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiButton::IStyleController)
			
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiSelectableButton::IStyleController,
				presentation::controls::GuiButton::IStyleController
				)

				void SetSelected(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetSelected, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiSelectableButton::IStyleController)
			
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiScroll::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)

				void SetCommandExecutor(GuiScroll::ICommandExecutor* value)override
				{
					INVOKE_INTERFACE_PROXY(SetCommandExecutor, value);
				}

				void SetTotalSize(vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetTotalSize, value);
				}

				void SetPageSize(vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetPageSize, value);
				}

				void SetPosition(vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetPosition, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiScroll::IStyleController)
			
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiTab::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)

				void SetCommandExecutor(GuiTab::ICommandExecutor* value)override
				{
					INVOKE_INTERFACE_PROXY(SetCommandExecutor, value);
				}

				void InsertTab(vint index)override
				{
					INVOKE_INTERFACE_PROXY(InsertTab, index);
				}

				void SetTabText(vint index, const WString& value)override
				{
					INVOKE_INTERFACE_PROXY(SetTabText, index, value);
				}

				void RemoveTab(vint index)override
				{
					INVOKE_INTERFACE_PROXY(RemoveTab, index);
				}

				void MoveTab(vint oldIndex, vint newIndex)override
				{
					INVOKE_INTERFACE_PROXY(MoveTab, oldIndex, newIndex);
				}

				void SetSelectedTab(vint index)override
				{
					INVOKE_INTERFACE_PROXY(SetSelectedTab, index);
				}

				void SetTabAlt(vint index, const WString& value, compositions::IGuiAltActionHost* host)override
				{
					INVOKE_INTERFACE_PROXY(SetTabAlt, index, value, host);
				}

				compositions::IGuiAltAction* GetTabAltAction(vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetTabAltAction, index);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiTab::IStyleController)
			
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiScrollView::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)

				GuiScroll::IStyleController* CreateHorizontalScrollStyle()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateHorizontalScrollStyle);
				}

				GuiScroll::IStyleController* CreateVerticalScrollStyle()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateVerticalScrollStyle);
				}

				vint GetDefaultScrollSize()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetDefaultScrollSize);
				}

				compositions::GuiGraphicsComposition* InstallBackground(compositions::GuiBoundsComposition* boundsComposition)override
				{
					return INVOKEGET_INTERFACE_PROXY(InstallBackground, boundsComposition);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiScrollView::IStyleController)
			
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiWindow::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)

				void AttachWindow(GuiWindow* _window)override
				{
					INVOKE_INTERFACE_PROXY(ActiveWindow, _window);
				}

				void InitializeNativeWindowProperties()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(InitializeNativeWindowProperties);
				}

				void SetSizeState(INativeWindow::WindowSizeState value)override
				{
					INVOKE_INTERFACE_PROXY(SetSizeState, value);
				}

				bool GetMaximizedBox()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetMaximizedBox);
				}

				void SetMaximizedBox(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetMaximizedBox, visible);
				}

				bool GetMinimizedBox()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetMinimizedBox);
				}

				void SetMinimizedBox(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetMinimizedBox, visible);
				}

				bool GetBorder()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBorder);
				}

				void SetBorder(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetBorder, visible);
				}

				bool GetSizeBox()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSizeBox);
				}

				void SetSizeBox(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetSizeBox, visible);
				}

				bool GetIconVisible()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetIconVisible);
				}

				void SetIconVisible(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetIconVisible, visible);
				}

				bool GetTitleBar()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetTitleBar);
				}

				void SetTitleBar(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetTitleBar, visible);
				}

				GuiWindow::IStyleController* CreateTooltipStyle()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateTooltipStyle);
				}

				GuiLabel::IStyleController* CreateShortcutKeyStyle()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateShortcutKeyStyle);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiWindow::IStyleController)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::GuiListControl::IItemProviderCallback)

				void OnAttached(GuiListControl::IItemProvider* provider)override
				{
					INVOKE_INTERFACE_PROXY(OnAttached, provider);
				}

				void OnItemModified(vint start, vint count, vint newCount)override
				{
					INVOKE_INTERFACE_PROXY(OnItemModified, start, count, newCount);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemProviderCallback)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(presentation::controls::GuiListControl::IItemPrimaryTextView
				presentation::controls::GuiListControl::IItemProviderCallback
				)

				WString GetPrimaryTextViewText(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetPrimaryTextViewText, itemIndex);
				}

				bool ContainsPrimaryText(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(ContainsPrimaryText, itemIndex);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemPrimaryTextView)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::GuiListControl::IItemBindingView)

				description::Value GetBindingValue(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetBindingValue, itemIndex);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemBindingView)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::GuiListControl::IItemProvider)

				bool AttachCallback(GuiListControl::IItemProviderCallback* value)override
				{
					return INVOKEGET_INTERFACE_PROXY(AttachCallback, value);
				}

				bool DetachCallback(GuiListControl::IItemProviderCallback* value)override
				{
					return INVOKEGET_INTERFACE_PROXY(DetachCallback, value);
				}

				vint Count()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(Count);
				}

				IDescriptable* RequestView(const WString& identifier)override
				{
					return INVOKEGET_INTERFACE_PROXY(RequestView, identifier);
				}

				void ReleaseView(IDescriptable* view)override
				{
					INVOKE_INTERFACE_PROXY(ReleaseView, view);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemProvider)

			BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR(presentation::controls::GuiListControl::IItemStyleController)

				GuiListControl::IItemStyleProvider* GetStyleProvider()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetStyleProvider);
				}

				vint GetItemStyleId()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetItemStyleId);
				}

				compositions::GuiBoundsComposition* GetBoundsComposition()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBoundsComposition);
				}

				bool IsCacheable()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(IsCacheable);
				}

				bool IsInstalled()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(IsInstalled);
				}

				void OnInstalled()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(OnInstalled);
				}

				void OnUninstalled()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(OnUninstalled);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemStyleController)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::GuiListControl::IItemStyleProvider)

				void AttachListControl(GuiListControl* value)override
				{
					INVOKE_INTERFACE_PROXY(AttachListControl, value);
				}

				void DetachListControl()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(DetachListControl);
				}

				vint GetItemStyleId(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetItemStyleId, itemIndex);
				}

				GuiListControl::IItemStyleController* CreateItemStyle(vint styleId)override
				{
					return INVOKEGET_INTERFACE_PROXY(CreateItemStyle, styleId);
				}

				void DestroyItemStyle(GuiListControl::IItemStyleController* style)override
				{
					INVOKE_INTERFACE_PROXY(DestroyItemStyle, style);
				}

				void Install(GuiListControl::IItemStyleController* style, vint itemIndex)override
				{
					INVOKE_INTERFACE_PROXY(Install, style, itemIndex);
				}

				void SetStyleIndex(GuiListControl::IItemStyleController* style, vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetStyleIndex, style, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemStyleProvider)
			
			BEGIN_INTERFACE_PROXY_SHAREDPTR(presentation::controls::GuiListControl::IItemArranger,
				presentation::controls::GuiListControl::IItemProviderCallback
				)

				void AttachListControl(GuiListControl* value)override
				{
					INVOKE_INTERFACE_PROXY(AttachListControl, value);
				}

				void DetachListControl()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(DetachListControl);
				}

				GuiListControl::IItemArrangerCallback* GetCallback()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetCallback);
				}

				void SetCallback(GuiListControl::IItemArrangerCallback* value)override
				{
					INVOKE_INTERFACE_PROXY(SetCallback, value);
				}

				Size GetTotalSize()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetTotalSize);
				}

				GuiListControl::IItemStyleController* GetVisibleStyle(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetVisibleStyle, itemIndex);
				}

				vint GetVisibleIndex(GuiListControl::IItemStyleController* style)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetVisibleIndex, style);
				}

				void OnViewChanged(Rect bounds)override
				{
					INVOKE_INTERFACE_PROXY(OnViewChanged, bounds);
				}

				vint FindItem(vint itemIndex, compositions::KeyDirection key)override
				{
					return INVOKEGET_INTERFACE_PROXY(FindItem, itemIndex, key);
				}

				bool EnsureItemVisible(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(EnsureItemVisible, itemIndex);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemArranger)
			
			BEGIN_INTERFACE_PROXY_SHAREDPTR(presentation::controls::GuiSelectableListControl::IItemStyleProvider,
				presentation::controls::GuiListControl::IItemStyleProvider
				)

				void SetStyleSelected(GuiListControl::IItemStyleController* style, bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetStyleSelected, style, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiSelectableListControl::IItemStyleProvider)

			BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR(presentation::controls::list::TextItemStyleProvider::ITextItemStyleProvider)

				GuiSelectableButton::IStyleController* CreateBackgroundStyleController()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateBackgroundStyleController);
				}

				GuiSelectableButton::IStyleController* CreateBulletStyleController()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateBulletStyleController);
				}

				Color GetTextColor()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetTextColor);
				}
			END_INTERFACE_PROXY(presentation::controls::list::TextItemStyleProvider::ITextItemStyleProvider)

			class TextItemStyleProvider_ITextItemView : public virtual GuiListControl_IItemPrimaryTextView, public virtual list::TextItemStyleProvider::ITextItemView
			{
			public:
				TextItemStyleProvider_ITextItemView(Ptr<IValueInterfaceProxy> _proxy)
					:GuiListControl_IItemPrimaryTextView(_proxy)
				{
				}

				static Ptr<list::TextItemStyleProvider::ITextItemView> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new TextItemStyleProvider_ITextItemView(proxy);
				}

				WString GetText(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetText, itemIndex);
				}

				bool GetChecked(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetChecked, itemIndex);
				}

				void SetCheckedSilently(vint itemIndex, bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetCheckedSilently, itemIndex, value);
				}
			};

			class GuiListViewBase_IStyleProvider : public virtual GuiScrollView_IStyleProvider, public virtual GuiListViewBase::IStyleProvider
			{
			public:
				GuiListViewBase_IStyleProvider(Ptr<IValueInterfaceProxy> _proxy)
					:GuiControl_IStyleProvider(_proxy)
					,GuiScrollView_IStyleProvider(_proxy)
				{
				}

				static GuiListViewBase::IStyleProvider* Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new GuiListViewBase_IStyleProvider(proxy);
				}

				GuiSelectableButton::IStyleController* CreateItemBackground()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateItemBackground);
				}

				GuiListViewColumnHeader::IStyleController* CreateColumnStyle()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateColumnStyle);
				}

				Color GetPrimaryTextColor()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetPrimaryTextColor);
				}

				Color GetSecondaryTextColor()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSecondaryTextColor);
				}

				Color GetItemSeparatorColor()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetItemSeparatorColor);
				}
			};

			class ListViewItemStyleProvider_IListViewItemView : public virtual GuiListControl_IItemPrimaryTextView, public virtual list::ListViewItemStyleProvider::IListViewItemView
			{
			public:
				ListViewItemStyleProvider_IListViewItemView(Ptr<IValueInterfaceProxy> _proxy)
					:GuiListControl_IItemPrimaryTextView(_proxy)
				{
				}

				static Ptr<list::ListViewItemStyleProvider::IListViewItemView> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new ListViewItemStyleProvider_IListViewItemView(proxy);
				}

				Ptr<GuiImageData> GetSmallImage(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetSmallImage, itemIndex);
				}

				Ptr<GuiImageData> GetLargeImage(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetLargeImage, itemIndex);
				}

				WString GetText(vint itemIndex)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetText, itemIndex);
				}

				WString GetSubItem(vint itemIndex, vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetSubItem, itemIndex, index);
				}

				vint GetDataColumnCount()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetDataColumnCount);
				}

				vint GetDataColumn(vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetDataColumn, index);
				}

				vint GetColumnCount()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetColumnCount);
				}

				WString GetColumnText(vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetColumnText, index);
				}
			};

			class ListViewItemStyleProvider_IListViewItemContent : public ValueInterfaceRoot, public virtual list::ListViewItemStyleProvider::IListViewItemContent
			{
			public:
				ListViewItemStyleProvider_IListViewItemContent(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static list::ListViewItemStyleProvider::IListViewItemContent* Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new ListViewItemStyleProvider_IListViewItemContent(proxy);
				}

				compositions::GuiBoundsComposition* GetContentComposition()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetContentComposition);
				}

				compositions::GuiBoundsComposition* GetBackgroundDecorator()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBackgroundDecorator);
				}

				void Install(GuiListViewBase::IStyleProvider* styleProvider, list::ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)override
				{
					INVOKE_INTERFACE_PROXY(Install, styleProvider, view, itemIndex);
				}

				void Uninstall()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(Uninstall);
				}
			};

			class ListViewItemStyleProvider_IListViewItemContentProvider : public ValueInterfaceRoot, public virtual list::ListViewItemStyleProvider::IListViewItemContentProvider
			{
			public:
				ListViewItemStyleProvider_IListViewItemContentProvider(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::ListViewItemStyleProvider::IListViewItemContentProvider> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new ListViewItemStyleProvider_IListViewItemContentProvider(proxy);
				}

				compositions::IGuiAxis* CreatePreferredAxis()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreatePreferredAxis);
				}

				GuiListControl::IItemArranger* CreatePreferredArranger()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreatePreferredArranger);
				}

				list::ListViewItemStyleProvider::IListViewItemContent* CreateItemContent(const FontProperties& font)override
				{
					return INVOKEGET_INTERFACE_PROXY(CreateItemContent, font);
				}

				void AttachListControl(GuiListControl* value)override
				{
					INVOKE_INTERFACE_PROXY(AttachListControl, value);
				}

				void DetachListControl()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(DetachListControl);
				}
			};

			class ListViewColumnItemArranger_IColumnItemView : public ValueInterfaceRoot, public virtual list::ListViewColumnItemArranger::IColumnItemView
			{
			public:
				ListViewColumnItemArranger_IColumnItemView(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::ListViewColumnItemArranger::IColumnItemView> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new ListViewColumnItemArranger_IColumnItemView(proxy);
				}

				bool AttachCallback(list::ListViewColumnItemArranger::IColumnItemViewCallback* value)override
				{
					return INVOKEGET_INTERFACE_PROXY(AttachCallback, value);
				}

				bool DetachCallback(list::ListViewColumnItemArranger::IColumnItemViewCallback* value)override
				{
					return INVOKEGET_INTERFACE_PROXY(DetachCallback, value);
				}

				vint GetColumnCount()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetColumnCount);
				}

				WString GetColumnText(vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetColumnText, index);
				}

				vint GetColumnSize(vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetColumnSize, index);
				}

				void SetColumnSize(vint index, vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetColumnSize, index, value);
				}

				GuiMenu* GetDropdownPopup(vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetDropdownPopup, index);
				}

				GuiListViewColumnHeader::ColumnSortingState GetSortingState(vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetSortingState, index);
				}
			};

			class GuiMenuButton_IStyleController : public virtual GuiSelectableButton_IStyleController, public virtual GuiMenuButton::IStyleController
			{
			public:
				GuiMenuButton_IStyleController(Ptr<IValueInterfaceProxy> _proxy)
					:GuiControl_IStyleController(_proxy)
					,GuiButton_IStyleController(_proxy)
					,GuiSelectableButton_IStyleController(_proxy)
				{
				}

				static GuiMenuButton::IStyleController* Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new GuiMenuButton_IStyleController(proxy);
				}

				GuiMenu::IStyleController* CreateSubMenuStyleController()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateSubMenuStyleController);
				}

				void SetSubMenuExisting(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetSubMenuExisting, value);
				}

				void SetSubMenuOpening(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetSubMenuOpening, value);
				}

				GuiButton* GetSubMenuHost()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSubMenuHost);
				}

				void SetImage(Ptr<GuiImageData> value)override
				{
					INVOKE_INTERFACE_PROXY(SetImage, value);
				}

				void SetShortcutText(const WString& value)override
				{
					INVOKE_INTERFACE_PROXY(SetShortcutText, value);
				}
			};

			class GuiListViewColumnHeader_IStyleController : public virtual GuiMenuButton_IStyleController, public virtual GuiListViewColumnHeader::IStyleController
			{
			public:
				GuiListViewColumnHeader_IStyleController(Ptr<IValueInterfaceProxy> _proxy)
					:GuiControl_IStyleController(_proxy)
					,GuiButton_IStyleController(_proxy)
					,GuiSelectableButton_IStyleController(_proxy)
					,GuiMenuButton_IStyleController(_proxy)
				{
				}

				static GuiListViewColumnHeader::IStyleController* Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new GuiListViewColumnHeader_IStyleController(proxy);
				}

				void SetColumnSortingState(GuiListViewColumnHeader::ColumnSortingState value)override
				{
					INVOKE_INTERFACE_PROXY(SetColumnSortingState, value);
				}
			};

			class tree_INodeProvider : public ValueInterfaceRoot, public virtual tree::INodeProvider
			{
			public:
				tree_INodeProvider(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<tree::INodeProvider> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new tree_INodeProvider(proxy);
				}

				bool GetExpanding()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetExpanding);
				}

				void SetExpanding(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetExpanding, value);
				}

				vint CalculateTotalVisibleNodes()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CalculateTotalVisibleNodes);
				}

				vint GetChildCount()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetChildCount);
				}

				tree::INodeProvider* GetParent()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetParent);
				}

				tree::INodeProvider* GetChild(vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetChild, index);
				}

				void Increase()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(Increase);
				}

				void Release()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(Release);
				}
			};

			class tree_INodeRootProvider : public ValueInterfaceRoot, public virtual tree::INodeRootProvider
			{
			public:
				tree_INodeRootProvider(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<tree::INodeRootProvider> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new tree_INodeRootProvider(proxy);
				}

				tree::INodeProvider* GetRootNode()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetRootNode);
				}

				bool CanGetNodeByVisibleIndex()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CanGetNodeByVisibleIndex);
				}

				tree::INodeProvider* GetNodeByVisibleIndex(vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetNodeByVisibleIndex, index);
				}

				bool AttachCallback(tree::INodeProviderCallback* value)override
				{
					return INVOKEGET_INTERFACE_PROXY(AttachCallback, value);
				}

				bool DetachCallback(tree::INodeProviderCallback* value)override
				{
					return INVOKEGET_INTERFACE_PROXY(DetachCallback, value);
				}

				IDescriptable* RequestView(const WString& identifier)override
				{
					return INVOKEGET_INTERFACE_PROXY(RequestView, identifier);
				}

				void ReleaseView(IDescriptable* view)override
				{
					INVOKE_INTERFACE_PROXY(ReleaseView, view);
				}
			};

			class tree_INodeItemView : public virtual GuiListControl_IItemPrimaryTextView, public virtual tree::INodeItemView
			{
			public:
				tree_INodeItemView(Ptr<IValueInterfaceProxy> _proxy)
					:GuiListControl_IItemPrimaryTextView(_proxy)
				{
				}

				static Ptr<tree::INodeItemView> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new tree_INodeItemView(proxy);
				}

				tree::INodeProvider* RequestNode(vint index)override
				{
					return INVOKEGET_INTERFACE_PROXY(RequestNode, index);
				}

				void ReleaseNode(tree::INodeProvider* node)override
				{
					INVOKE_INTERFACE_PROXY(ReleaseNode, node);
				}

				vint CalculateNodeVisibilityIndex(tree::INodeProvider* node)override
				{
					return INVOKEGET_INTERFACE_PROXY(CalculateNodeVisibilityIndex, node);
				}
			};

			class tree_INodeItemPrimaryTextView : public ValueInterfaceRoot, public virtual tree::INodeItemPrimaryTextView
			{
			public:
				tree_INodeItemPrimaryTextView(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<tree::INodeItemPrimaryTextView> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new tree_INodeItemPrimaryTextView(proxy);
				}

				WString GetPrimaryTextViewText(tree::INodeProvider* node)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetPrimaryTextViewText, node);
				}
			};

			class tree_INodeItemBindingView: public ValueInterfaceRoot, public virtual tree::INodeItemBindingView
			{
			public:
				tree_INodeItemBindingView(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<tree::INodeItemBindingView> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new tree_INodeItemBindingView(proxy);
				}

				description::Value GetBindingValue(tree::INodeProvider* node)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetBindingValue, node);
				}
			};

			class tree_INodeItemStyleController: public virtual GuiListControl_IItemStyleController, public virtual tree::INodeItemStyleController
			{
			public:
				tree_INodeItemStyleController(Ptr<IValueInterfaceProxy> _proxy)
					:GuiListControl_IItemStyleController(_proxy)
				{
				}

				static tree::INodeItemStyleController* Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new tree_INodeItemStyleController(proxy);
				}

				tree::INodeItemStyleProvider* GetNodeStyleProvider()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetNodeStyleProvider);
				}
			};

			class tree_INodeItemStyleProvider : public ValueInterfaceRoot, public virtual tree::INodeItemStyleProvider
			{
			public:
				tree_INodeItemStyleProvider(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<tree::INodeItemStyleProvider> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new tree_INodeItemStyleProvider(proxy);
				}

				void BindItemStyleProvider(GuiListControl::IItemStyleProvider* styleProvider)override
				{
					INVOKE_INTERFACE_PROXY(BindItemStyleProvider, styleProvider);
				}

				GuiListControl::IItemStyleProvider* GetBindedItemStyleProvider()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBindedItemStyleProvider);
				}

				void AttachListControl(GuiListControl* value)override
				{
					INVOKE_INTERFACE_PROXY(AttachListControl, value);
				}

				void DetachListControl()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(DetachListControl);
				}

				vint GetItemStyleId(tree::INodeProvider* node)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetItemStyleId, node);
				}

				tree::INodeItemStyleController* CreateItemStyle(vint styleId)override
				{
					return INVOKEGET_INTERFACE_PROXY(CreateItemStyle, styleId);
				}

				void DestroyItemStyle(tree::INodeItemStyleController* style)override
				{
					INVOKE_INTERFACE_PROXY(DestroyItemStyle, style);
				}

				void Install(tree::INodeItemStyleController* style, tree::INodeProvider* node, vint itemIndex)override
				{
					INVOKE_INTERFACE_PROXY(Install, style, node, itemIndex);
				}

				void SetStyleIndex(tree::INodeItemStyleController* style, vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetStyleIndex, style, value);
				}

				void SetStyleSelected(tree::INodeItemStyleController* style, bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetStyleSelected, style, value);
				}
			};

			class tree_IMemoryNodeData : public ValueInterfaceRoot, public virtual tree::IMemoryNodeData
			{
			public:
				tree_IMemoryNodeData(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<tree::IMemoryNodeData> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new tree_IMemoryNodeData(proxy);
				}
			};

			class tree_ITreeViewItemView : public virtual tree_INodeItemPrimaryTextView, public virtual tree::ITreeViewItemView
			{
			public:
				tree_ITreeViewItemView(Ptr<IValueInterfaceProxy> proxy)
					:tree_INodeItemPrimaryTextView(proxy)
				{
				}

				static Ptr<tree::ITreeViewItemView> Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new tree_ITreeViewItemView(proxy);
				}

				Ptr<GuiImageData> GetNodeImage(tree::INodeProvider* node)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetNodeImage, node);
				}

				WString GetNodeText(tree::INodeProvider* node)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetNodeText, node);
				}
			};

			class GuiVirtualTreeView_IStyleProvider : public virtual GuiScrollView_IStyleProvider, public virtual GuiVirtualTreeView::IStyleProvider
			{
			public:
				GuiVirtualTreeView_IStyleProvider(Ptr<IValueInterfaceProxy> _proxy)
					:GuiControl_IStyleProvider(_proxy)
					,GuiScrollView_IStyleProvider(_proxy)
				{
				}

				static GuiVirtualTreeView::IStyleProvider* Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new GuiVirtualTreeView_IStyleProvider(proxy);
				}

				GuiSelectableButton::IStyleController* CreateItemBackground()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateItemBackground);
				}

				GuiSelectableButton::IStyleController* CreateItemExpandingDecorator()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateItemExpandingDecorator);
				}

				Color GetTextColor()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetTextColor);
				}
			};

			class GuiComboBoxBase_IStyleController : public virtual GuiMenuButton_IStyleController, public virtual GuiComboBoxBase::IStyleController
			{
			public:
				GuiComboBoxBase_IStyleController(Ptr<IValueInterfaceProxy> _proxy)
					:GuiControl_IStyleController(_proxy)
					,GuiButton_IStyleController(_proxy)
					,GuiSelectableButton_IStyleController(_proxy)
					,GuiMenuButton_IStyleController(_proxy)
				{
				}

				static GuiComboBoxBase::IStyleController* Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new GuiComboBoxBase_IStyleController(proxy);
				}

				void SetCommandExecutor(GuiComboBoxBase::ICommandExecutor* value)override
				{
					INVOKE_INTERFACE_PROXY(SetCommandExecutor, value);
				}

				void OnItemSelected()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(OnItemSelected);
				}
			};

			class GuiSinglelineTextBox_IStyleProvider : public virtual GuiControl_IStyleProvider, public virtual GuiSinglelineTextBox::IStyleProvider
			{
			public:
				GuiSinglelineTextBox_IStyleProvider(Ptr<IValueInterfaceProxy> _proxy)
					:GuiControl_IStyleProvider(_proxy)
				{
				}

				static GuiSinglelineTextBox::IStyleProvider* Create(Ptr<IValueInterfaceProxy> proxy)
				{
					return new GuiSinglelineTextBox_IStyleProvider(proxy);
				}

				compositions::GuiGraphicsComposition* InstallBackground(compositions::GuiBoundsComposition* background)override
				{
					return INVOKEGET_INTERFACE_PROXY(InstallBackground, background);
				}
			};

			class list_IDataVisualizerFactory : public ValueInterfaceRoot, public virtual list::IDataVisualizerFactory
			{
			public:
				list_IDataVisualizerFactory(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::IDataVisualizerFactory> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new list_IDataVisualizerFactory(_proxy);
				}

				Ptr<list::IDataVisualizer> CreateVisualizer(const FontProperties& font, GuiListViewBase::IStyleProvider* styleProvider)override
				{
					return INVOKEGET_INTERFACE_PROXY(CreateVisualizer, font, styleProvider);
				}
			};

			class list_IDataVisualizer : public ValueInterfaceRoot, public virtual list::IDataVisualizer
			{
			public:
				list_IDataVisualizer(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::IDataVisualizer> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new list_IDataVisualizer(_proxy);
				}

				list::IDataVisualizerFactory* GetFactory()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetFactory);
				}

				compositions::GuiBoundsComposition* GetBoundsComposition()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBoundsComposition);
				}

				void BeforeVisualizeCell(list::IDataProvider* dataProvider, vint row, vint column)override
				{
					INVOKE_INTERFACE_PROXY(dataProvider, row, column);
				}

				list::IDataVisualizer* GetDecoratedDataVisualizer()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetDecoratedDataVisualizer);
				}

				void SetSelected(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetSelected, value);
				}
			};

			class list_IDataEditorFactory : public ValueInterfaceRoot, public virtual list::IDataEditorFactory
			{
			public:
				list_IDataEditorFactory(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::IDataEditorFactory> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new list_IDataEditorFactory(_proxy);
				}

				Ptr<list::IDataEditor> CreateEditor(list::IDataEditorCallback* callback)
				{
					return INVOKEGET_INTERFACE_PROXY(CreateEditor, callback);
				}
			};

			class list_IDataEditor : public ValueInterfaceRoot, public virtual list::IDataEditor
			{
			public:
				list_IDataEditor(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::IDataEditor> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new list_IDataEditor(_proxy);
				}

				list::IDataEditorFactory* GetFactory()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetFactory);
				}

				compositions::GuiBoundsComposition* GetBoundsComposition()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBoundsComposition);
				}

				void BeforeEditCell(list::IDataProvider* dataProvider, vint row, vint column)override
				{
					INVOKE_INTERFACE_PROXY(BeforeEditCell, dataProvider, row, column);
				}

				void ReinstallEditor()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(ReinstallEditor);
				}
			};

			class list_IDataProvider : public ValueInterfaceRoot, public virtual list::IDataProvider
			{
			public:
				list_IDataProvider(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::IDataProvider> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new list_IDataProvider(_proxy);
				}

				void SetCommandExecutor(list::IDataProviderCommandExecutor* value)override
				{
					INVOKE_INTERFACE_PROXY(SetCommandExecutor, value);
				}

				vint GetColumnCount()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetColumnCount);
				}

				WString GetColumnText(vint column)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetColumnText, column);
				}

				vint GetColumnSize(vint column)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetColumnSize, column);
				}

				void SetColumnSize(vint column, vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetColumnSize, column, value);
				}

				GuiMenu* GetColumnPopup(vint column)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetColumnPopup, column);
				}

				bool IsColumnSortable(vint column)override
				{
					return INVOKEGET_INTERFACE_PROXY(IsColumnSortable, column);
				}

				void SortByColumn(vint column, bool ascending)override
				{
					INVOKE_INTERFACE_PROXY(SortByColumn, column, ascending);
				}

				vint GetSortedColumn()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSortedColumn);
				}

				bool IsSortOrderAscending()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(IsSortOrderAscending);
				}

				vint GetRowCount()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetRowCount);
				}

				Ptr<GuiImageData> GetRowLargeImage(vint row)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetRowLargeImage, row);
				}

				Ptr<GuiImageData> GetRowSmallImage(vint row)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetRowSmallImage, row);
				}

				vint GetCellSpan(vint row, vint column)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetCellSpan, row, column);
				}

				WString GetCellText(vint row, vint column)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetCellText, row, column);
				}

				list::IDataVisualizerFactory* GetCellDataVisualizerFactory(vint row, vint column)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetCellDataVisualizerFactory, row, column);
				}

				void VisualizeCell(vint row, vint column, list::IDataVisualizer* dataVisualizer)override
				{
					INVOKE_INTERFACE_PROXY(VisualizeCell, row, column, dataVisualizer);
				}

				list::IDataEditorFactory* GetCellDataEditorFactory(vint row, vint column)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetCellDataEditorFactory, row, column);
				}

				void BeforeEditCell(vint row, vint column, list::IDataEditor* dataEditor)override
				{
					INVOKE_INTERFACE_PROXY(BeforeEditCell, row, column, dataEditor);
				}

				void SaveCellData(vint row, vint column, list::IDataEditor* dataEditor)override
				{
					INVOKE_INTERFACE_PROXY(SaveCellData, row, column, dataEditor);
				}
			};

			class list_IStructuredDataFilter : public ValueInterfaceRoot, public virtual list::IStructuredDataFilter
			{
			public:
				list_IStructuredDataFilter(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::IStructuredDataFilter> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new list_IStructuredDataFilter(_proxy);
				}

				void SetCommandExecutor(list::IStructuredDataFilterCommandExecutor* value)override
				{
					INVOKE_INTERFACE_PROXY(SetCommandExecutor, value);
				}

				bool Filter(vint row)override
				{
					return INVOKEGET_INTERFACE_PROXY(Filter, row);
				}
			};

			class list_IStructuredDataSorter : public ValueInterfaceRoot, public virtual list::IStructuredDataSorter
			{
			public:
				list_IStructuredDataSorter(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::IStructuredDataSorter> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new list_IStructuredDataSorter(_proxy);
				}

				vint Compare(vint row1, vint row2)override
				{
					return INVOKEGET_INTERFACE_PROXY(Compare, row1, row2);
				}
			};

			class list_IStructuredColumnProvider : public ValueInterfaceRoot, public virtual list::IStructuredColumnProvider
			{
			public:
				list_IStructuredColumnProvider(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::IStructuredColumnProvider> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new list_IStructuredColumnProvider(_proxy);
				}

				WString GetText()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetText);
				}

				vint GetSize()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSize);
				}

				void SetSize(vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetSize, value);
				}

				GuiListViewColumnHeader::ColumnSortingState GetSortingState()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSortingState);
				}

				void SetSortingState(GuiListViewColumnHeader::ColumnSortingState value)override
				{
					INVOKE_INTERFACE_PROXY(SetSortingState, value);
				}

				GuiMenu* GetPopup()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetPopup);
				}

				Ptr<list::IStructuredDataFilter> GetInherentFilter()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetInherentFilter);
				}

				Ptr<list::IStructuredDataSorter> GetInherentSorter()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetInherentSorter);
				}

				WString GetCellText(vint row)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetCellText, row);
				}

				list::IDataVisualizerFactory* GetCellDataVisualizerFactory(vint row)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetCellDataVisualizerFactory, row);
				}

				void VisualizeCell(vint row, list::IDataVisualizer* dataVisualizer)override
				{
					INVOKE_INTERFACE_PROXY(VisualizeCell, row, dataVisualizer);
				}

				list::IDataEditorFactory* GetCellDataEditorFactory(vint row)override
				{
					return INVOKEGET_INTERFACE_PROXY(GetCellDataEditorFactory, row);
				}

				void BeforeEditCell(vint row, list::IDataEditor* dataEditor)override
				{
					INVOKE_INTERFACE_PROXY(BeforeEditCell, row, dataEditor);
				}

				void SaveCellData(vint row, list::IDataEditor* dataEditor)override
				{
					INVOKE_INTERFACE_PROXY(SaveCellData, row, dataEditor);
				}
			};

			class list_IStructuredDataProvider : public ValueInterfaceRoot, public virtual list::IStructuredDataProvider
			{
			public:
				list_IStructuredDataProvider(Ptr<IValueInterfaceProxy> _proxy)
					:ValueInterfaceRoot(_proxy)
				{
				}

				static Ptr<list::IStructuredDataProvider> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new list_IStructuredDataProvider(_proxy);
				}

				void SetCommandExecutor(list::IDataProviderCommandExecutor* value)
				{
					INVOKE_INTERFACE_PROXY(SetCommandExecutor, value);
				}

				vint GetColumnCount()
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetColumnCount);
				}

				vint GetRowCount()
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetRowCount);
				}

				list::IStructuredColumnProvider* GetColumn(vint column)
				{
					return INVOKEGET_INTERFACE_PROXY(GetColumn, column);
				}

				Ptr<GuiImageData> GetRowLargeImage(vint row)
				{
					return INVOKEGET_INTERFACE_PROXY(GetRowLargeImage, row);
				}

				Ptr<GuiImageData> GetRowSmallImage(vint row)
				{
					return INVOKEGET_INTERFACE_PROXY(GetRowSmallImage, row);
				}
			};

			class GuiDatePicker_IStyleProvider : public GuiControl_IStyleProvider, public virtual GuiDatePicker::IStyleProvider
			{
			public:
				GuiDatePicker_IStyleProvider(Ptr<IValueInterfaceProxy> _proxy)
					:GuiControl_IStyleProvider(_proxy)
				{
				}

				static GuiDatePicker::IStyleProvider* Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new GuiDatePicker_IStyleProvider(_proxy);
				}

				GuiSelectableButton::IStyleController* CreateDateButtonStyle()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateDateButtonStyle);
				}

				GuiTextList* CreateTextList()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateTextList);
				}

				GuiComboBoxListControl::IStyleController* CreateComboBoxStyle()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateComboBoxStyle);
				}

				Color GetBackgroundColor()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBackgroundColor);
				}

				Color GetPrimaryTextColor()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetPrimaryTextColor);
				}

				Color GetSecondaryTextColor()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSecondaryTextColor);
				}
			};

			class GuiDocumentViewer_IStyleProvider : public GuiScrollView_IStyleProvider, public virtual GuiDocumentViewer::IStyleProvider
			{
			public:
				GuiDocumentViewer_IStyleProvider(Ptr<IValueInterfaceProxy> _proxy)
					:GuiControl_IStyleProvider(_proxy)
					,GuiScrollView_IStyleProvider(_proxy)
				{
				}

				static Ptr<GuiDocumentViewer::IStyleProvider> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new GuiDocumentViewer_IStyleProvider(_proxy);
				}

				Ptr<DocumentModel> GetBaselineDocument()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBaselineDocument);
				}
			};

			class GuiDocumentLabel_IStyleController : public GuiControl_IStyleController, public virtual GuiDocumentLabel::IStyleController
			{
			public:
				GuiDocumentLabel_IStyleController(Ptr<IValueInterfaceProxy> _proxy)
					:GuiControl_IStyleController(_proxy)
				{
				}

				static Ptr<GuiDocumentLabel::IStyleController> Create(Ptr<IValueInterfaceProxy> _proxy)
				{
					return new GuiDocumentLabel_IStyleController(_proxy);
				}

				Ptr<DocumentModel> GetBaselineDocument()override
				{
					return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBaselineDocument);
				}
			};
#pragma warning(pop)

/***********************************************************************
Type Loader
***********************************************************************/

#endif

			extern bool						LoadGuiControlTypes();
		}
	}
}

#endif
