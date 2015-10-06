/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDATAGRIDCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIDATAGRIDCONTROLS

#include "GuiDataGridInterfaces.h"
#include "GuiDataGridStructured.h"
#include "GuiDataGridExtensions.h"

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
Datagrid ItemProvider
***********************************************************************/
				
				/// <summary>Data grid item provider.</summary>
				class DataGridItemProvider
					: public Object
					, public virtual GuiListControl::IItemProvider
					, public virtual GuiListControl::IItemPrimaryTextView
					, public virtual ListViewItemStyleProvider::IListViewItemView
					, public virtual ListViewColumnItemArranger::IColumnItemView
					, protected virtual IDataProviderCommandExecutor
					, public Description<DataGridItemProvider>
				{
				protected:
					IDataProvider*																dataProvider;
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
					void												SortByColumn(vint column, bool ascending=true);

					// ===================== GuiListControl::IItemProvider =====================

					bool												AttachCallback(GuiListControl::IItemProviderCallback* value)override;
					bool												DetachCallback(GuiListControl::IItemProviderCallback* value)override;
					vint												Count()override;
					IDescriptable*										RequestView(const WString& identifier)override;
					void												ReleaseView(IDescriptable* view)override;

					// ===================== GuiListControl::IItemPrimaryTextView =====================

					WString												GetPrimaryTextViewText(vint itemIndex)override;
					bool												ContainsPrimaryText(vint itemIndex)override;

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

/***********************************************************************
Datagrid ContentProvider
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
					void												RequestSaveData();
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
Virtual DataGrid Control
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
				/// <param name="_dataProvider">The data provider for this control.</param>
				GuiVirtualDataGrid(IStyleProvider* _styleProvider, list::IDataProvider* _dataProvider);
				/// <summary>Create a data grid control in virtual mode.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				/// <param name="_dataProvider">The data provider for this control.</param>
				GuiVirtualDataGrid(IStyleProvider* _styleProvider, list::IStructuredDataProvider* _dataProvider);
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

/***********************************************************************
StringGrid Control
***********************************************************************/

			class GuiStringGrid;

			namespace list
			{
				class StringGridProvider;

				class StringGridItem
				{
				public:
					collections::List<WString>							strings;
				};
				
				/// <summary>Data visualizer that displays a text for <see cref="GuiStringGrid"/>. Use StringGridDataVisualizer::Factory as the factory class.</summary>
				class StringGridDataVisualizer : public ListViewSubColumnDataVisualizer
				{
				public:
					typedef DataVisualizerFactory<StringGridDataVisualizer>				Factory;
				public:
					/// <summary>Create the data visualizer.</summary>
					StringGridDataVisualizer();

					void												BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)override;
					void												SetSelected(bool value)override;
				};

				class StringGridColumn : public StrongTypedColumnProviderBase<Ptr<StringGridItem>, WString>
				{
				protected:
					StringGridProvider*									provider;

				public:
					StringGridColumn(StringGridProvider* _provider);
					~StringGridColumn();

					void												GetCellData(const Ptr<StringGridItem>& rowData, WString& cellData)override;
					void												SetCellData(const Ptr<StringGridItem>& rowData, const WString& cellData);
					WString												GetCellDataText(const WString& cellData)override;

					void												BeforeEditCell(vint row, IDataEditor* dataEditor)override;
					void												SaveCellData(vint row, IDataEditor* dataEditor)override;
				};

				/// <summary>Data source for <see cref="GuiStringGrid"/>.</summary>
				class StringGridProvider : private StrongTypedDataProvider<Ptr<StringGridItem>>, public Description<StringGridProvider>
				{
					friend class StringGridColumn;
					friend class vl::presentation::controls::GuiStringGrid;
				protected:
					bool												readonly;
					collections::List<Ptr<StringGridItem>>				items;
					Ptr<IDataVisualizerFactory>							visualizerFactory;
					Ptr<IDataEditorFactory>								editorFactory;

					void												GetRowData(vint row, Ptr<StringGridItem>& rowData)override;
					bool												GetReadonly();
					void												SetReadonly(bool value);
				public:
					StringGridProvider();
					~StringGridProvider();

					vint												GetRowCount()override;
					vint												GetColumnCount()override;

					/// <summary>Insert an empty row.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="row">The row index.</param>
					bool												InsertRow(vint row);
					/// <summary>Add an empty row.</summary>
					/// <returns>The new row index.</returns>
					vint												AppendRow();
					/// <summary>Move a row.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="source">The old row index.</param>
					/// <param name="target">The new row index.</param>
					bool												MoveRow(vint source, vint target);
					/// <summary>Remove a row.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="row">The row index.</param>
					bool												RemoveRow(vint row);
					/// <summary>Clear all rows.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					bool												ClearRows();
					/// <summary>Get the text for a cell.</summary>
					/// <returns>The text.</returns>
					/// <param name="row">The row index.</param>
					/// <param name="column">The column index.</param>
					WString												GetGridString(vint row, vint column);
					/// <summary>Set the text for a cell.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="row">The row index.</param>
					/// <param name="column">The column index.</param>
					/// <param name="value">The text.</param>
					bool												SetGridString(vint row, vint column, const WString& value);

					/// <summary>Insert a column.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="column">The column index.</param>
					/// <param name="text">The text.</param>
					/// <param name="size">The size.</param>
					bool												InsertColumn(vint column, const WString& text, vint size=0);
					/// <summary>Add a column.</summary>
					/// <returns>The new column index.</returns>
					/// <param name="text">The text.</param>
					/// <param name="size">The size.</param>
					vint												AppendColumn(const WString& text, vint size=0);
					/// <summary>Move a column.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="source">The old column index.</param>
					/// <param name="target">The new column index.</param>
					bool												MoveColumn(vint source, vint target);
					/// <summary>Remove a column.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="column">The new column index.</param>
					bool												RemoveColumn(vint column);
					/// <summary>Clear all columns.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					bool												ClearColumns();
					/// <summary>Get the text for a column.</summary>
					/// <returns>The text.</returns>
					/// <param name="column">The new column index.</param>
					WString												GetColumnText(vint column);
					/// <summary>Set the text for a column.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="column">The column index.</param>
					/// <param name="value">The text.</param>
					bool												SetColumnText(vint column, const WString& value);
				};
			}

			/// <summary>String grid control in virtual mode.</summary>
			class GuiStringGrid : public GuiVirtualDataGrid, public Description<GuiStringGrid>
			{
			protected:
				list::StringGridProvider*								grids;
			public:
				/// <summary>Create a string grid control.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				GuiStringGrid(IStyleProvider* _styleProvider);
				~GuiStringGrid();

				/// <summary>Get the grid data in this control.</summary>
				/// <returns>The grid data.</returns>
				list::StringGridProvider&								Grids();

				/// <summary>Get the readonly mode.</summary>
				/// <returns>Returns true if the string grid is readonly.</returns>
				bool													GetReadonly();
				/// <summary>Set the readonly mode.</summary>
				/// <param name="value">Set to true to make the string grid readonly.</param>
				void													SetReadonly(bool value);
			};
		}
	}
}

#endif