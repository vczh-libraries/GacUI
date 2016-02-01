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
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiLabel::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)

				presentation::Color GetDefaultTextColor()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetDefaultTextColor);
				}

				void SetTextColor(presentation::Color value)override
				{
					INVOKE_INTERFACE_PROXY(SetTextColor, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiLabel::IStyleController)
			
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiButton::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)

				void Transfer(presentation::controls::GuiButton::ControlState value)override
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

				void SetCommandExecutor(presentation::controls::GuiScroll::ICommandExecutor* value)override
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

				void SetCommandExecutor(presentation::controls::GuiTab::ICommandExecutor* value)override
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

				void SetTabAlt(vint index, const WString& value, presentation::compositions::IGuiAltActionHost* host)override
				{
					INVOKE_INTERFACE_PROXY(SetTabAlt, index, value, host);
				}

				presentation::compositions::IGuiAltAction* GetTabAltAction(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(GetTabAltAction, index);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiTab::IStyleController)
			
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiScrollView::IStyleProvider,
				presentation::controls::GuiControl::IStyleProvider
				)

				presentation::controls::GuiScroll::IStyleController* CreateHorizontalScrollStyle()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateHorizontalScrollStyle);
				}

				presentation::controls::GuiScroll::IStyleController* CreateVerticalScrollStyle()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateVerticalScrollStyle);
				}

				vint GetDefaultScrollSize()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetDefaultScrollSize);
				}

				presentation::compositions::GuiGraphicsComposition* InstallBackground(presentation::compositions::GuiBoundsComposition* boundsComposition)override
				{
					INVOKEGET_INTERFACE_PROXY(InstallBackground, boundsComposition);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiScrollView::IStyleProvider)
			
			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiWindow::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)

				void AttachWindow(presentation::controls::GuiWindow* _window)override
				{
					INVOKE_INTERFACE_PROXY(AttachWindow, _window);
				}

				void InitializeNativeWindowProperties()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(InitializeNativeWindowProperties);
				}

				void SetSizeState(presentation::INativeWindow::WindowSizeState value)override
				{
					INVOKE_INTERFACE_PROXY(SetSizeState, value);
				}

				bool GetMaximizedBox()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetMaximizedBox);
				}

				void SetMaximizedBox(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetMaximizedBox, visible);
				}

				bool GetMinimizedBox()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetMinimizedBox);
				}

				void SetMinimizedBox(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetMinimizedBox, visible);
				}

				bool GetBorder()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBorder);
				}

				void SetBorder(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetBorder, visible);
				}

				bool GetSizeBox()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSizeBox);
				}

				void SetSizeBox(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetSizeBox, visible);
				}

				bool GetIconVisible()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetIconVisible);
				}

				void SetIconVisible(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetIconVisible, visible);
				}

				bool GetTitleBar()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetTitleBar);
				}

				void SetTitleBar(bool visible)override
				{
					INVOKE_INTERFACE_PROXY(SetTitleBar, visible);
				}

				presentation::controls::GuiWindow::IStyleController* CreateTooltipStyle()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateTooltipStyle);
				}

				presentation::controls::GuiLabel::IStyleController* CreateShortcutKeyStyle()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateShortcutKeyStyle);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiWindow::IStyleController)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::GuiListControl::IItemProviderCallback)

				void OnAttached(presentation::controls::GuiListControl::IItemProvider* provider)override
				{
					INVOKE_INTERFACE_PROXY(OnAttached, provider);
				}

				void OnItemModified(vint start, vint count, vint newCount)override
				{
					INVOKE_INTERFACE_PROXY(OnItemModified, start, count, newCount);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemProviderCallback)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(presentation::controls::GuiListControl::IItemPrimaryTextView,
				presentation::controls::GuiListControl::IItemProviderCallback
				)

				WString GetPrimaryTextViewText(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(GetPrimaryTextViewText, itemIndex);
				}

				bool ContainsPrimaryText(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(ContainsPrimaryText, itemIndex);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemPrimaryTextView)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::GuiListControl::IItemBindingView)

				description::Value GetBindingValue(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(GetBindingValue, itemIndex);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemBindingView)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::GuiListControl::IItemProvider)

				bool AttachCallback(presentation::controls::GuiListControl::IItemProviderCallback* value)override
				{
					INVOKEGET_INTERFACE_PROXY(AttachCallback, value);
				}

				bool DetachCallback(presentation::controls::GuiListControl::IItemProviderCallback* value)override
				{
					INVOKEGET_INTERFACE_PROXY(DetachCallback, value);
				}

				vint Count()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(Count);
				}

				IDescriptable* RequestView(const WString& identifier)override
				{
					INVOKEGET_INTERFACE_PROXY(RequestView, identifier);
				}

				void ReleaseView(IDescriptable* view)override
				{
					INVOKE_INTERFACE_PROXY(ReleaseView, view);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemProvider)

			BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR(presentation::controls::GuiListControl::IItemStyleController)

				presentation::controls::GuiListControl::IItemStyleProvider* GetStyleProvider()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetStyleProvider);
				}

				vint GetItemStyleId()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetItemStyleId);
				}

				presentation::compositions::GuiBoundsComposition* GetBoundsComposition()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBoundsComposition);
				}

				bool IsCacheable()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(IsCacheable);
				}

				bool IsInstalled()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(IsInstalled);
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

				void AttachListControl(presentation::controls::GuiListControl* value)override
				{
					INVOKE_INTERFACE_PROXY(AttachListControl, value);
				}

				void DetachListControl()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(DetachListControl);
				}

				vint GetItemStyleId(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(GetItemStyleId, itemIndex);
				}

				presentation::controls::GuiListControl::IItemStyleController* CreateItemStyle(vint styleId)override
				{
					INVOKEGET_INTERFACE_PROXY(CreateItemStyle, styleId);
				}

				void DestroyItemStyle(presentation::controls::GuiListControl::IItemStyleController* style)override
				{
					INVOKE_INTERFACE_PROXY(DestroyItemStyle, style);
				}

				void Install(presentation::controls::GuiListControl::IItemStyleController* style, vint itemIndex)override
				{
					INVOKE_INTERFACE_PROXY(Install, style, itemIndex);
				}

				void SetStyleIndex(presentation::controls::GuiListControl::IItemStyleController* style, vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetStyleIndex, style, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemStyleProvider)
			
			BEGIN_INTERFACE_PROXY_SHAREDPTR(presentation::controls::GuiListControl::IItemArranger,
				presentation::controls::GuiListControl::IItemProviderCallback
				)

				void AttachListControl(presentation::controls::GuiListControl* value)override
				{
					INVOKE_INTERFACE_PROXY(AttachListControl, value);
				}

				void DetachListControl()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(DetachListControl);
				}

				presentation::controls::GuiListControl::IItemArrangerCallback* GetCallback()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetCallback);
				}

				void SetCallback(presentation::controls::GuiListControl::IItemArrangerCallback* value)override
				{
					INVOKE_INTERFACE_PROXY(SetCallback, value);
				}

				presentation::Size GetTotalSize()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetTotalSize);
				}

				presentation::controls::GuiListControl::IItemStyleController* GetVisibleStyle(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(GetVisibleStyle, itemIndex);
				}

				vint GetVisibleIndex(presentation::controls::GuiListControl::IItemStyleController* style)override
				{
					INVOKEGET_INTERFACE_PROXY(GetVisibleIndex, style);
				}

				void OnViewChanged(presentation::Rect bounds)override
				{
					INVOKE_INTERFACE_PROXY(OnViewChanged, bounds);
				}

				vint FindItem(vint itemIndex, presentation::compositions::KeyDirection key)override
				{
					INVOKEGET_INTERFACE_PROXY(FindItem, itemIndex, key);
				}

				bool EnsureItemVisible(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(EnsureItemVisible, itemIndex);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListControl::IItemArranger)
			
			BEGIN_INTERFACE_PROXY_SHAREDPTR(presentation::controls::GuiSelectableListControl::IItemStyleProvider,
				presentation::controls::GuiListControl::IItemStyleProvider
				)

				void SetStyleSelected(presentation::controls::GuiListControl::IItemStyleController* style, bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetStyleSelected, style, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiSelectableListControl::IItemStyleProvider)

			BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR(presentation::controls::list::TextItemStyleProvider::ITextItemStyleProvider)

				presentation::controls::GuiSelectableButton::IStyleController* CreateBackgroundStyleController()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateBackgroundStyleController);
				}

				presentation::controls::GuiSelectableButton::IStyleController* CreateBulletStyleController()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateBulletStyleController);
				}

				presentation::Color GetTextColor()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetTextColor);
				}
			END_INTERFACE_PROXY(presentation::controls::list::TextItemStyleProvider::ITextItemStyleProvider)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(presentation::controls::list::TextItemStyleProvider::ITextItemView,
				presentation::controls::GuiListControl::IItemPrimaryTextView
				)

				WString GetText(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(GetText, itemIndex);
				}

				bool GetChecked(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(GetChecked, itemIndex);
				}

				void SetCheckedSilently(vint itemIndex, bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetCheckedSilently, itemIndex, value);
				}
			END_INTERFACE_PROXY(presentation::controls::list::TextItemStyleProvider::ITextItemView)

			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiListViewBase::IStyleProvider,
				presentation::controls::GuiScrollView::IStyleProvider
				)

				presentation::controls::GuiSelectableButton::IStyleController* CreateItemBackground()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateItemBackground);
				}

				presentation::controls::GuiListViewColumnHeader::IStyleController* CreateColumnStyle()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateColumnStyle);
				}

				presentation::Color GetPrimaryTextColor()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetPrimaryTextColor);
				}

				presentation::Color GetSecondaryTextColor()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSecondaryTextColor);
				}

				presentation::Color GetItemSeparatorColor()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetItemSeparatorColor);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListViewBase::IStyleProvider)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(presentation::controls::list::ListViewItemStyleProvider::IListViewItemView,
				presentation::controls::GuiListControl::IItemPrimaryTextView
				)

				Ptr<presentation::GuiImageData> GetSmallImage(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(GetSmallImage, itemIndex);
				}

				Ptr<presentation::GuiImageData> GetLargeImage(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(GetLargeImage, itemIndex);
				}

				WString GetText(vint itemIndex)override
				{
					INVOKEGET_INTERFACE_PROXY(GetText, itemIndex);
				}

				WString GetSubItem(vint itemIndex, vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(GetSubItem, itemIndex, index);
				}

				vint GetDataColumnCount()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetDataColumnCount);
				}

				vint GetDataColumn(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(GetDataColumn, index);
				}

				vint GetColumnCount()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetColumnCount);
				}

				WString GetColumnText(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(GetColumnText, index);
				}
			END_INTERFACE_PROXY(presentation::controls::list::ListViewItemStyleProvider::IListViewItemView)

			BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR(presentation::controls::list::ListViewItemStyleProvider::IListViewItemContent)

				presentation::compositions::GuiBoundsComposition* GetContentComposition()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetContentComposition);
				}

				presentation::compositions::GuiBoundsComposition* GetBackgroundDecorator()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBackgroundDecorator);
				}

				void Install(presentation::controls::GuiListViewBase::IStyleProvider* styleProvider, presentation::controls::list::ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)override
				{
					INVOKE_INTERFACE_PROXY(Install, styleProvider, view, itemIndex);
				}

				void Uninstall()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(Uninstall);
				}
			END_INTERFACE_PROXY(presentation::controls::list::ListViewItemStyleProvider::IListViewItemContent)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::ListViewItemStyleProvider::IListViewItemContentProvider)

				presentation::compositions::IGuiAxis* CreatePreferredAxis()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreatePreferredAxis);
				}

				presentation::controls::GuiListControl::IItemArranger* CreatePreferredArranger()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreatePreferredArranger);
				}

				presentation::controls::list::ListViewItemStyleProvider::IListViewItemContent* CreateItemContent(const presentation::FontProperties& font)override
				{
					INVOKEGET_INTERFACE_PROXY(CreateItemContent, font);
				}

				void AttachListControl(presentation::controls::GuiListControl* value)override
				{
					INVOKE_INTERFACE_PROXY(AttachListControl, value);
				}

				void DetachListControl()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(DetachListControl);
				}
			END_INTERFACE_PROXY(presentation::controls::list::ListViewItemStyleProvider::IListViewItemContentProvider)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::ListViewColumnItemArranger::IColumnItemView)

				bool AttachCallback(presentation::controls::list::ListViewColumnItemArranger::IColumnItemViewCallback* value)override
				{
					INVOKEGET_INTERFACE_PROXY(AttachCallback, value);
				}

				bool DetachCallback(presentation::controls::list::ListViewColumnItemArranger::IColumnItemViewCallback* value)override
				{
					INVOKEGET_INTERFACE_PROXY(DetachCallback, value);
				}

				vint GetColumnCount()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetColumnCount);
				}

				WString GetColumnText(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(GetColumnText, index);
				}

				vint GetColumnSize(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(GetColumnSize, index);
				}

				void SetColumnSize(vint index, vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetColumnSize, index, value);
				}

				presentation::controls::GuiMenu* GetDropdownPopup(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(GetDropdownPopup, index);
				}

				presentation::controls::GuiListViewColumnHeader::ColumnSortingState GetSortingState(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(GetSortingState, index);
				}
			END_INTERFACE_PROXY(presentation::controls::list::ListViewColumnItemArranger::IColumnItemView)

			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiMenuButton::IStyleController,
				presentation::controls::GuiSelectableButton::IStyleController
				)

				presentation::controls::GuiMenu::IStyleController* CreateSubMenuStyleController()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateSubMenuStyleController);
				}

				void SetSubMenuExisting(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetSubMenuExisting, value);
				}

				void SetSubMenuOpening(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetSubMenuOpening, value);
				}

				presentation::controls::GuiButton* GetSubMenuHost()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSubMenuHost);
				}

				void SetImage(Ptr<presentation::GuiImageData> value)override
				{
					INVOKE_INTERFACE_PROXY(SetImage, value);
				}

				void SetShortcutText(const WString& value)override
				{
					INVOKE_INTERFACE_PROXY(SetShortcutText, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiMenuButton::IStyleController)

			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiListViewColumnHeader::IStyleController,
				presentation::controls::GuiMenuButton::IStyleController
				)

				void SetColumnSortingState(presentation::controls::GuiListViewColumnHeader::ColumnSortingState value)override
				{
					INVOKE_INTERFACE_PROXY(SetColumnSortingState, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiListViewColumnHeader::IStyleController)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::tree::INodeProvider)

				bool GetExpanding()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetExpanding);
				}

				void SetExpanding(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetExpanding, value);
				}

				vint CalculateTotalVisibleNodes()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CalculateTotalVisibleNodes);
				}

				vint GetChildCount()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetChildCount);
				}

				presentation::controls::tree::INodeProvider* GetParent()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetParent);
				}

				presentation::controls::tree::INodeProvider* GetChild(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(GetChild, index);
				}

				void Increase()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(Increase);
				}

				void Release()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(Release);
				}
			END_INTERFACE_PROXY(presentation::controls::tree::INodeProvider)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::tree::INodeRootProvider)

				presentation::controls::tree::INodeProvider* GetRootNode()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetRootNode);
				}

				bool CanGetNodeByVisibleIndex()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CanGetNodeByVisibleIndex);
				}

				presentation::controls::tree::INodeProvider* GetNodeByVisibleIndex(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(GetNodeByVisibleIndex, index);
				}

				bool AttachCallback(presentation::controls::tree::INodeProviderCallback* value)override
				{
					INVOKEGET_INTERFACE_PROXY(AttachCallback, value);
				}

				bool DetachCallback(presentation::controls::tree::INodeProviderCallback* value)override
				{
					INVOKEGET_INTERFACE_PROXY(DetachCallback, value);
				}

				IDescriptable* RequestView(const WString& identifier)override
				{
					INVOKEGET_INTERFACE_PROXY(RequestView, identifier);
				}

				void ReleaseView(IDescriptable* view)override
				{
					INVOKE_INTERFACE_PROXY(ReleaseView, view);
				}
			END_INTERFACE_PROXY(presentation::controls::tree::INodeRootProvider)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(presentation::controls::tree::INodeItemView,
				presentation::controls::GuiListControl::IItemPrimaryTextView
				)

				presentation::controls::tree::INodeProvider* RequestNode(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(RequestNode, index);
				}

				void ReleaseNode(presentation::controls::tree::INodeProvider* node)override
				{
					INVOKE_INTERFACE_PROXY(ReleaseNode, node);
				}

				vint CalculateNodeVisibilityIndex(presentation::controls::tree::INodeProvider* node)override
				{
					INVOKEGET_INTERFACE_PROXY(CalculateNodeVisibilityIndex, node);
				}
			END_INTERFACE_PROXY(presentation::controls::tree::INodeItemView)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::tree::INodeItemPrimaryTextView)

				WString GetPrimaryTextViewText(presentation::controls::tree::INodeProvider* node)override
				{
					INVOKEGET_INTERFACE_PROXY(GetPrimaryTextViewText, node);
				}
			END_INTERFACE_PROXY(presentation::controls::tree::INodeItemPrimaryTextView)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::tree::INodeItemBindingView)

				description::Value GetBindingValue(presentation::controls::tree::INodeProvider* node)override
				{
					INVOKEGET_INTERFACE_PROXY(GetBindingValue, node);
				}
			END_INTERFACE_PROXY(presentation::controls::tree::INodeItemBindingView)

			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::tree::INodeItemStyleController,
				presentation::controls::GuiListControl::IItemStyleController
				)

				presentation::controls::tree::INodeItemStyleProvider* GetNodeStyleProvider()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetNodeStyleProvider);
				}
			END_INTERFACE_PROXY(presentation::controls::tree::INodeItemStyleController)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::tree::INodeItemStyleProvider)

				void BindItemStyleProvider(presentation::controls::GuiListControl::IItemStyleProvider* styleProvider)override
				{
					INVOKE_INTERFACE_PROXY(BindItemStyleProvider, styleProvider);
				}

				presentation::controls::GuiListControl::IItemStyleProvider* GetBindedItemStyleProvider()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBindedItemStyleProvider);
				}

				void AttachListControl(presentation::controls::GuiListControl* value)override
				{
					INVOKE_INTERFACE_PROXY(AttachListControl, value);
				}

				void DetachListControl()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(DetachListControl);
				}

				vint GetItemStyleId(presentation::controls::tree::INodeProvider* node)override
				{
					INVOKEGET_INTERFACE_PROXY(GetItemStyleId, node);
				}

				presentation::controls::tree::INodeItemStyleController* CreateItemStyle(vint styleId)override
				{
					INVOKEGET_INTERFACE_PROXY(CreateItemStyle, styleId);
				}

				void DestroyItemStyle(presentation::controls::tree::INodeItemStyleController* style)override
				{
					INVOKE_INTERFACE_PROXY(DestroyItemStyle, style);
				}

				void Install(presentation::controls::tree::INodeItemStyleController* style, presentation::controls::tree::INodeProvider* node, vint itemIndex)override
				{
					INVOKE_INTERFACE_PROXY(Install, style, node, itemIndex);
				}

				void SetStyleIndex(presentation::controls::tree::INodeItemStyleController* style, vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetStyleIndex, style, value);
				}

				void SetStyleSelected(presentation::controls::tree::INodeItemStyleController* style, bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetStyleSelected, style, value);
				}
			END_INTERFACE_PROXY(presentation::controls::tree::INodeItemStyleProvider)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::tree::IMemoryNodeData)
			END_INTERFACE_PROXY(presentation::controls::tree::IMemoryNodeData)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(presentation::controls::tree::ITreeViewItemView,
				presentation::controls::tree::INodeItemPrimaryTextView
				)

				Ptr<presentation::GuiImageData> GetNodeImage(presentation::controls::tree::INodeProvider* node)override
				{
					INVOKEGET_INTERFACE_PROXY(GetNodeImage, node);
				}

				WString GetNodeText(presentation::controls::tree::INodeProvider* node)override
				{
					INVOKEGET_INTERFACE_PROXY(GetNodeText, node);
				}
			END_INTERFACE_PROXY(presentation::controls::tree::ITreeViewItemView)

			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiVirtualTreeView::IStyleProvider,
				presentation::controls::GuiScrollView::IStyleProvider
				)

				presentation::controls::GuiSelectableButton::IStyleController* CreateItemBackground()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateItemBackground);
				}

				presentation::controls::GuiSelectableButton::IStyleController* CreateItemExpandingDecorator()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateItemExpandingDecorator);
				}

				presentation::Color GetTextColor()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetTextColor);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiVirtualTreeView::IStyleProvider)

			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiComboBoxBase::IStyleController,
				presentation::controls::GuiMenuButton::IStyleController
				)

				void SetCommandExecutor(presentation::controls::GuiComboBoxBase::ICommandExecutor* value)override
				{
					INVOKE_INTERFACE_PROXY(SetCommandExecutor, value);
				}

				void OnItemSelected()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(OnItemSelected);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiComboBoxBase::IStyleController)

			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiSinglelineTextBox::IStyleProvider,
				presentation::controls::GuiControl::IStyleProvider
				)

				presentation::compositions::GuiGraphicsComposition* InstallBackground(presentation::compositions::GuiBoundsComposition* background)override
				{
					INVOKEGET_INTERFACE_PROXY(InstallBackground, background);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiSinglelineTextBox::IStyleProvider)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::IDataVisualizerFactory)

				Ptr<presentation::controls::list::IDataVisualizer> CreateVisualizer(const presentation::FontProperties& font, presentation::controls::GuiListViewBase::IStyleProvider* styleProvider)override
				{
					INVOKEGET_INTERFACE_PROXY(CreateVisualizer, font, styleProvider);
				}
			END_INTERFACE_PROXY(presentation::controls::list::IDataVisualizerFactory)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::IDataVisualizer)

				presentation::controls::list::IDataVisualizerFactory* GetFactory()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetFactory);
				}

				presentation::compositions::GuiBoundsComposition* GetBoundsComposition()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBoundsComposition);
				}

				void BeforeVisualizeCell(presentation::controls::list::IDataProvider* dataProvider, vint row, vint column)override
				{
					INVOKE_INTERFACE_PROXY(BeforeVisualizeCell, dataProvider, row, column);
				}

				presentation::controls::list::IDataVisualizer* GetDecoratedDataVisualizer()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetDecoratedDataVisualizer);
				}

				void SetSelected(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetSelected, value);
				}
			END_INTERFACE_PROXY(presentation::controls::list::IDataVisualizer)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::IDataEditorFactory)

				Ptr<presentation::controls::list::IDataEditor> CreateEditor(presentation::controls::list::IDataEditorCallback* callback)
				{
					INVOKEGET_INTERFACE_PROXY(CreateEditor, callback);
				}
			END_INTERFACE_PROXY(presentation::controls::list::IDataEditorFactory)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::IDataEditor)

				presentation::controls::list::IDataEditorFactory* GetFactory()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetFactory);
				}

				presentation::compositions::GuiBoundsComposition* GetBoundsComposition()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBoundsComposition);
				}

				void BeforeEditCell(presentation::controls::list::IDataProvider* dataProvider, vint row, vint column)override
				{
					INVOKE_INTERFACE_PROXY(BeforeEditCell, dataProvider, row, column);
				}

				void ReinstallEditor()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(ReinstallEditor);
				}
			END_INTERFACE_PROXY(presentation::controls::list::IDataEditor)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::IDataProvider)

				void SetCommandExecutor(presentation::controls::list::IDataProviderCommandExecutor* value)override
				{
					INVOKE_INTERFACE_PROXY(SetCommandExecutor, value);
				}

				vint GetColumnCount()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetColumnCount);
				}

				WString GetColumnText(vint column)override
				{
					INVOKEGET_INTERFACE_PROXY(GetColumnText, column);
				}

				vint GetColumnSize(vint column)override
				{
					INVOKEGET_INTERFACE_PROXY(GetColumnSize, column);
				}

				void SetColumnSize(vint column, vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetColumnSize, column, value);
				}

				presentation::controls::GuiMenu* GetColumnPopup(vint column)override
				{
					INVOKEGET_INTERFACE_PROXY(GetColumnPopup, column);
				}

				bool IsColumnSortable(vint column)override
				{
					INVOKEGET_INTERFACE_PROXY(IsColumnSortable, column);
				}

				void SortByColumn(vint column, bool ascending)override
				{
					INVOKE_INTERFACE_PROXY(SortByColumn, column, ascending);
				}

				vint GetSortedColumn()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSortedColumn);
				}

				bool IsSortOrderAscending()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(IsSortOrderAscending);
				}

				vint GetRowCount()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetRowCount);
				}

				Ptr<presentation::GuiImageData> GetRowLargeImage(vint row)override
				{
					INVOKEGET_INTERFACE_PROXY(GetRowLargeImage, row);
				}

				Ptr<presentation::GuiImageData> GetRowSmallImage(vint row)override
				{
					INVOKEGET_INTERFACE_PROXY(GetRowSmallImage, row);
				}

				vint GetCellSpan(vint row, vint column)override
				{
					INVOKEGET_INTERFACE_PROXY(GetCellSpan, row, column);
				}

				WString GetCellText(vint row, vint column)override
				{
					INVOKEGET_INTERFACE_PROXY(GetCellText, row, column);
				}

				presentation::controls::list::IDataVisualizerFactory* GetCellDataVisualizerFactory(vint row, vint column)override
				{
					INVOKEGET_INTERFACE_PROXY(GetCellDataVisualizerFactory, row, column);
				}

				void VisualizeCell(vint row, vint column, presentation::controls::list::IDataVisualizer* dataVisualizer)override
				{
					INVOKE_INTERFACE_PROXY(VisualizeCell, row, column, dataVisualizer);
				}

				presentation::controls::list::IDataEditorFactory* GetCellDataEditorFactory(vint row, vint column)override
				{
					INVOKEGET_INTERFACE_PROXY(GetCellDataEditorFactory, row, column);
				}

				void BeforeEditCell(vint row, vint column, presentation::controls::list::IDataEditor* dataEditor)override
				{
					INVOKE_INTERFACE_PROXY(BeforeEditCell, row, column, dataEditor);
				}

				void SaveCellData(vint row, vint column, presentation::controls::list::IDataEditor* dataEditor)override
				{
					INVOKE_INTERFACE_PROXY(SaveCellData, row, column, dataEditor);
				}
			END_INTERFACE_PROXY(presentation::controls::list::IDataProvider)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::IStructuredDataFilter)

				void SetCommandExecutor(presentation::controls::list::IStructuredDataFilterCommandExecutor* value)override
				{
					INVOKE_INTERFACE_PROXY(SetCommandExecutor, value);
				}

				bool Filter(vint row)override
				{
					INVOKEGET_INTERFACE_PROXY(Filter, row);
				}
			END_INTERFACE_PROXY(presentation::controls::list::IStructuredDataFilter)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::IStructuredDataSorter)

				vint Compare(vint row1, vint row2)override
				{
					INVOKEGET_INTERFACE_PROXY(Compare, row1, row2);
				}
			END_INTERFACE_PROXY(presentation::controls::list::IStructuredDataSorter)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::IStructuredColumnProvider)

				WString GetText()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetText);
				}

				vint GetSize()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSize);
				}

				void SetSize(vint value)override
				{
					INVOKE_INTERFACE_PROXY(SetSize, value);
				}

				presentation::controls::GuiListViewColumnHeader::ColumnSortingState GetSortingState()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSortingState);
				}

				void SetSortingState(presentation::controls::GuiListViewColumnHeader::ColumnSortingState value)override
				{
					INVOKE_INTERFACE_PROXY(SetSortingState, value);
				}

				presentation::controls::GuiMenu* GetPopup()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetPopup);
				}

				Ptr<presentation::controls::list::IStructuredDataFilter> GetInherentFilter()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetInherentFilter);
				}

				Ptr<presentation::controls::list::IStructuredDataSorter> GetInherentSorter()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetInherentSorter);
				}

				WString GetCellText(vint row)override
				{
					INVOKEGET_INTERFACE_PROXY(GetCellText, row);
				}

				presentation::controls::list::IDataVisualizerFactory* GetCellDataVisualizerFactory(vint row)override
				{
					INVOKEGET_INTERFACE_PROXY(GetCellDataVisualizerFactory, row);
				}

				void VisualizeCell(vint row, presentation::controls::list::IDataVisualizer* dataVisualizer)override
				{
					INVOKE_INTERFACE_PROXY(VisualizeCell, row, dataVisualizer);
				}

				presentation::controls::list::IDataEditorFactory* GetCellDataEditorFactory(vint row)override
				{
					INVOKEGET_INTERFACE_PROXY(GetCellDataEditorFactory, row);
				}

				void BeforeEditCell(vint row, presentation::controls::list::IDataEditor* dataEditor)override
				{
					INVOKE_INTERFACE_PROXY(BeforeEditCell, row, dataEditor);
				}

				void SaveCellData(vint row, presentation::controls::list::IDataEditor* dataEditor)override
				{
					INVOKE_INTERFACE_PROXY(SaveCellData, row, dataEditor);
				}
			END_INTERFACE_PROXY(presentation::controls::list::IStructuredColumnProvider)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(presentation::controls::list::IStructuredDataProvider)

				void SetCommandExecutor(presentation::controls::list::IDataProviderCommandExecutor* value)
				{
					INVOKE_INTERFACE_PROXY(SetCommandExecutor, value);
				}

				vint GetColumnCount()
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetColumnCount);
				}

				vint GetRowCount()
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetRowCount);
				}

				presentation::controls::list::IStructuredColumnProvider* GetColumn(vint column)
				{
					INVOKEGET_INTERFACE_PROXY(GetColumn, column);
				}

				Ptr<presentation::GuiImageData> GetRowLargeImage(vint row)
				{
					INVOKEGET_INTERFACE_PROXY(GetRowLargeImage, row);
				}

				Ptr<presentation::GuiImageData> GetRowSmallImage(vint row)
				{
					INVOKEGET_INTERFACE_PROXY(GetRowSmallImage, row);
				}
			END_INTERFACE_PROXY(presentation::controls::list::IStructuredDataProvider)

			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiDatePicker::IStyleProvider,
				presentation::controls::GuiControl::IStyleProvider
				)

				presentation::controls::GuiSelectableButton::IStyleController* CreateDateButtonStyle()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateDateButtonStyle);
				}

				presentation::controls::GuiTextList* CreateTextList()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateTextList);
				}

				presentation::controls::GuiComboBoxListControl::IStyleController* CreateComboBoxStyle()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateComboBoxStyle);
				}

				presentation::Color GetBackgroundColor()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBackgroundColor);
				}

				presentation::Color GetPrimaryTextColor()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetPrimaryTextColor);
				}

				presentation::Color GetSecondaryTextColor()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetSecondaryTextColor);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiDatePicker::IStyleProvider)

			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiDocumentViewer::IStyleProvider,
				presentation::controls::GuiScrollView::IStyleProvider
				)

				Ptr<presentation::DocumentModel> GetBaselineDocument()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBaselineDocument);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiDocumentViewer::IStyleProvider)

			BEGIN_INTERFACE_PROXY_RAWPTR(presentation::controls::GuiDocumentLabel::IStyleController,
				presentation::controls::GuiControl::IStyleController
				)

				Ptr<presentation::DocumentModel> GetBaselineDocument()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBaselineDocument);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiDocumentLabel::IStyleProvider)
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
