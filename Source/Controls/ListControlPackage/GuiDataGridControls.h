/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDATAGRIDCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIDATAGRIDCONTROLS

#include "GuiDataGridInterfaces.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiVirtualDataGrid;

			namespace list
			{

/***********************************************************************
DefaultDataGridItemTemplate
***********************************************************************/
				
				/// <summary>Data grid content provider.</summary>
				class DataGridContentProvider
					: public Object
					, public virtual ListViewItemStyleProvider::IListViewItemContentProvider
					, protected virtual ListViewColumnItemArranger::IColumnItemViewCallback
					, protected virtual GuiListControl::IItemProviderCallback
					, protected virtual IDataEditorCallback
					, public Description<ListViewDetailContentProvider>
				{
				protected:
					class ItemContent : public Object, public virtual ListViewItemStyleProvider::IListViewItemContent
					{
					protected:
						compositions::GuiBoundsComposition*				contentComposition;
						compositions::GuiTableComposition*				textTable;

						DataGridContentProvider*						contentProvider;
						FontProperties									font;

						collections::Array<Ptr<IDataVisualizer>>		dataVisualizers;
						IDataEditor*									currentEditor;

						void											RemoveCellsAndDataVisualizers();
						IDataVisualizerFactory*							GetDataVisualizerFactory(vint row, vint column);
						vint											GetCellColumnIndex(compositions::GuiGraphicsComposition* composition);
						void											OnCellButtonUp(compositions::GuiGraphicsComposition* sender, bool openEditor);
						bool											IsInEditor(compositions::GuiMouseEventArgs& arguments);
						void											OnCellButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
						void											OnCellLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
						void											OnCellRightButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					public:
						ItemContent(DataGridContentProvider* _contentProvider, const FontProperties& _font);
						~ItemContent();

						compositions::GuiBoundsComposition*				GetContentComposition()override;
						compositions::GuiBoundsComposition*				GetBackgroundDecorator()override;
						void											UpdateSubItemSize();
						void											ForceSetEditor(vint column, IDataEditor* editor);
						void											NotifyCloseEditor();
						void											NotifySelectCell(vint column);
						void											Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)override;
						void											Uninstall()override;
					};

					GuiVirtualDataGrid*									dataGrid;
					GuiListControl::IItemProvider*						itemProvider;
					list::IDataProvider*								dataProvider;
					ListViewColumnItemArranger::IColumnItemView*		columnItemView;
					ListViewItemStyleProvider*							listViewItemStyleProvider;

					ListViewMainColumnDataVisualizer::Factory			mainColumnDataVisualizerFactory;
					ListViewSubColumnDataVisualizer::Factory			subColumnDataVisualizerFactory;

					GridPos												currentCell;
					Ptr<IDataEditor>									currentEditor;
					bool												currentEditorRequestingSaveData;
					bool												currentEditorOpening;

					void												OnColumnChanged()override;
					void												OnAttached(GuiListControl::IItemProvider* provider)override;
					void												OnItemModified(vint start, vint count, vint newCount)override;

					void												NotifyCloseEditor();
					void												NotifySelectCell(vint row, vint column);
					void												RequestSaveData()override;
					IDataEditor*										OpenEditor(vint row, vint column, IDataEditorFactory* editorFactory);
					void												CloseEditor(bool forOpenNewEditor);
				public:
					/// <summary>Create the content provider.</summary>
					DataGridContentProvider();
					~DataGridContentProvider();
					
					compositions::IGuiAxis*								CreatePreferredAxis()override;
					GuiListControl::IItemArranger*						CreatePreferredArranger()override;
					ListViewItemStyleProvider::IListViewItemContent*	CreateItemContent(const FontProperties& font)override;
					void												AttachListControl(GuiListControl* value)override;
					void												DetachListControl()override;

					GridPos												GetSelectedCell();
					bool												SetSelectedCell(const GridPos& value, bool openEditor);
				};
			}

/***********************************************************************
GuiVirtualDataGrid
***********************************************************************/

			/// <summary>Data grid control in virtual mode.</summary>
			class GuiVirtualDataGrid : public GuiVirtualListView, public Description<GuiVirtualDataGrid>
			{
				friend class list::DataGridContentProvider;
			protected:
				list::DataGridItemProvider*								itemProvider;
				list::DataGridContentProvider*							contentProvider;
				Ptr<list::IDataProvider>								dataProvider;
				Ptr<list::StructuredDataProvider>						structuredDataProvider;

				void													OnColumnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);
				void													Initialize();
				void													NotifySelectedCellChanged();
			public:
				/// <summary>Create a data grid control in virtual mode.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiVirtualDataGrid(IStyleProvider* _styleProvider, GuiListControl::IItemProvider* _itemProvider);
				~GuiVirtualDataGrid();

				/// <summary>Selected cell changed event.</summary>
				compositions::GuiNotifyEvent							SelectedCellChanged;

				/// <summary>Get the given data provider.</summary>
				/// <returns>The data provider.</returns>
				list::IDataProvider*									GetDataProvider();
				/// <summary>Get the given structured data provider.</summary>
				/// <returns>The structured data provider.</returns>
				list::StructuredDataProvider*							GetStructuredDataProvider();

				/// <summary>Get the row index and column index of the selected cell.</summary>
				/// <returns>The row index and column index of the selected cell.</returns>
				GridPos													GetSelectedCell();
				/// <summary>Set the row index and column index of the selected cell.</summary>
				/// <param name="value">The row index and column index of the selected cell.</param>
				void													SetSelectedCell(const GridPos& value);

				Ptr<GuiListControl::IItemStyleProvider>					SetStyleProvider(Ptr<GuiListControl::IItemStyleProvider> value)override;
				bool													ChangeItemStyle(Ptr<list::ListViewItemStyleProvider::IListViewItemContentProvider> contentProvider)override;
			};

			namespace list
			{

/***********************************************************************
DataGridItemProvider
***********************************************************************/

				/// <summary>Data grid item provider.</summary>
				class DataGridItemProvider
					: public Object
					, public virtual GuiListControl::IItemProvider
					, public virtual IListViewItemView
					, public virtual ListViewColumnItemArranger::IColumnItemView
					, public virtual IDataGridView
					, public Description<DataGridItemProvider>
				{
				protected:
					collections::List<GuiListControl::IItemProviderCallback*>					itemProviderCallbacks;
					collections::List<ListViewColumnItemArranger::IColumnItemViewCallback*>		columnItemViewCallbacks;

					void												InvokeOnItemModified(vint start, vint count, vint newCount);
					void												InvokeOnColumnChanged();
					void												OnDataProviderColumnChanged()override;
					void												OnDataProviderItemModified(vint start, vint count, vint newCount)override;
				public:
					/// <summary>Create the content provider.</summary>
					/// <param name="_dataProvider">The data provider for this control.</param>
					DataGridItemProvider(IDataProvider* _dataProvider);
					~DataGridItemProvider();

					IDataProvider*										GetDataProvider();
					void												SortByColumn(vint column, bool ascending = true);

					// ===================== GuiListControl::IItemProvider =====================

					bool												AttachCallback(GuiListControl::IItemProviderCallback* value)override;
					bool												DetachCallback(GuiListControl::IItemProviderCallback* value)override;
					vint												Count()override;
					WString												GetTextValue(vint itemIndex)override;
					description::Value									GetBindingValue(vint itemIndex)override;
					IDescriptable*										RequestView(const WString& identifier)override;
					void												ReleaseView(IDescriptable* view)override;

					// ===================== list::ListViewItemStyleProvider::IListViewItemView =====================

					Ptr<GuiImageData>									GetSmallImage(vint itemIndex)override;
					Ptr<GuiImageData>									GetLargeImage(vint itemIndex)override;
					WString												GetText(vint itemIndex)override;
					WString												GetSubItem(vint itemIndex, vint index)override;
					vint												GetDataColumnCount()override;
					vint												GetDataColumn(vint index)override;

					// ===================== list::ListViewColumnItemArranger::IColumnItemView =====================

					bool												AttachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)override;
					bool												DetachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)override;
					vint												GetColumnCount()override;
					WString												GetColumnText(vint index)override;
					vint												GetColumnSize(vint index)override;
					void												SetColumnSize(vint index, vint value)override;
					GuiMenu*											GetDropdownPopup(vint index)override;
					GuiListViewColumnHeader::ColumnSortingState			GetSortingState(vint index)override;
				};
			}
		}
	}
}

#endif
