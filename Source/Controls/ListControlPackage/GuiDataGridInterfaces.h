/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDATAGRIDINTERFACES
#define VCZH_PRESENTATION_CONTROLS_GUIDATAGRIDINTERFACES

#include "GuiListViewControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			namespace list
			{

/***********************************************************************
Datagrid Interfaces
***********************************************************************/

				class IDataVisualizerFactory;
				class IDataVisualizer;
				class IDataEditorCallback;
				class IDataEditorFactory;
				class IDataEditor;
				class IDataProviderCommandExecutor;
				class IDataProvider;

				/// <summary>The visualizer factory.</summary>
				class IDataVisualizerFactory : public virtual IDescriptable, public Description<IDataVisualizerFactory>
				{
				public:
					/// <summary>Create a data visualizer.</summary>
					/// <returns>The created data visualizer.</returns>
					/// <param name="font">The font for the list view control.</param>
					/// <param name="styleProvider">The style provider for the list view control.</param>
					virtual Ptr<IDataVisualizer>						CreateVisualizer(const FontProperties& font, GuiListViewBase::IStyleProvider* styleProvider, const description::Value& viewModelContext)=0;
				};

				/// <summary>The visualizer for each cell in [T:vl.presentation.controls.GuiVirtualDataGrid].</summary>
				class IDataVisualizer : public virtual IDescriptable, public Description<IDataVisualizer>
				{
				public:
					/// <summary>Get the factory object that creates this visualizer.</summary>
					/// <returns>The factory object.</returns>
					virtual IDataVisualizerFactory*						GetFactory()=0;

					/// <summary>Get the composition that renders the data.. The data visualizer should maintain this bounds composition, and delete it when necessary.</summary>
					/// <returns>The composition.</returns>
					virtual compositions::GuiBoundsComposition*			GetBoundsComposition()=0;

					/// <summary>Called before visualizing a cell.</summary>
					/// <param name="dataProvider">The data provider.</param>
					/// <param name="row">The row number of the cell.</param>
					/// <param name="column">The column number of the cell.</param>
					virtual void										BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)=0;

					/// <summary>Get the decorated data visualizer inside the current data visualizer.</summary>
					/// <returns>The decorated data visualizer. Returns null if such a visualizer does not exists.</returns>
					virtual IDataVisualizer*							GetDecoratedDataVisualizer()=0;

					/// <summary>Set the selected state.</summary>
					/// <param name="value">Set to true to make this data visualizer looks selected.</param>
					virtual void										SetSelected(bool value)=0;

					template<typename T>
					T* GetVisualizer()
					{
						IDataVisualizer* visualizer=this;
						while(visualizer)
						{
							T* result=dynamic_cast<T*>(visualizer);
							if(result) return result;
							visualizer=visualizer->GetDecoratedDataVisualizer();
						}
						return 0;
					};
				};

				/// <summary>The editor callback.</summary>
				class IDataEditorCallback : public virtual IDescriptable, public Description<IDataEditorCallback>
				{
				public:
					/// <summary>Called when the editor needs to save the new data to the data provider.</summary>
					virtual void										RequestSaveData()=0;
				};

				/// <summary>The editor factory.</summary>
				class IDataEditorFactory : public virtual IDescriptable, public Description<IDataEditorFactory>
				{
				public:
					/// <summary>Create a data editor.</summary>
					/// <returns>The created data editor.</returns>
					/// <param name="callback">The callback for the created editor to send notification.</param>
					virtual Ptr<IDataEditor>							CreateEditor(IDataEditorCallback* callback, const description::Value& viewModelContext)=0;
				};

				/// <summary>The editor for each cell in [T:vl.presentation.controls.GuiVirtualDataGrid].</summary>
				class IDataEditor : public virtual IDescriptable, public Description<IDataEditor>
				{
				public:
					/// <summary>Get the factory object that creates this editor.</summary>
					/// <returns>The factory object.</returns>
					virtual IDataEditorFactory*							GetFactory()=0;

					/// <summary>Get the composition that holds the editor for a cell. The data editor should maintain this bounds composition, and delete it when necessary.</summary>
					/// <returns>The composition.</returns>
					virtual compositions::GuiBoundsComposition*			GetBoundsComposition()=0;

					/// <summary>Called before editing a cell.</summary>
					/// <param name="dataProvider">The data provider.</param>
					/// <param name="row">The row number of the cell.</param>
					/// <param name="column">The column number of the cell.</param>
					virtual void										BeforeEditCell(IDataProvider* dataProvider, vint row, vint column)=0;

					/// <summary>Called when an editor is reinstalled during editing.</summary>
					virtual void										ReinstallEditor()=0;
				};

				/// <summary>The command executor for [T:vl.presentation.controls.list.IDataProvider] to send notification.</summary>
				class IDataProviderCommandExecutor : public virtual IDescriptable, public Description<IDataProviderCommandExecutor>
				{
				public:
					/// <summary>Called when any column is changed (inserted, removed, text changed, etc.).</summary>
					virtual void										OnDataProviderColumnChanged()=0;

					/// <summary>Called when items in the data provider is modified.</summary>
					/// <param name="start">The index of the first modified row.</param>
					/// <param name="count">The number of all modified rows.</param>
					/// <param name="newCount">The number of new rows. If rows are inserted or removed, newCount may not equals to count.</param>
					virtual void										OnDataProviderItemModified(vint start, vint count, vint newCount)=0;
				};

				/// <summary>The required <see cref="GuiListControl::IItemProvider"/> view for [T:vl.presentation.controls.GuiVirtualDataGrid].</summary>
				class IDataProvider : public virtual IDescriptable, public Description<IDataProvider>
				{
				public:
					/// <summary>The identifier for this view.</summary>
					static const wchar_t* const							Identifier;

					/// <summary>Set the command executor.</summary>
					/// <param name="value">The command executor.</param>
					virtual void										SetCommandExecutor(IDataProviderCommandExecutor* value) = 0;
					/// <summary>Get the view model context. It is used to create data visualizers and data editors.</summary>
					/// <returns>The view model context.</returns>
					virtual description::Value							GetViewModelContext() = 0;
					/// <summary>Get the number of all columns.</summary>
					/// <returns>The number of all columns.</returns>
					virtual vint										GetColumnCount() = 0;
					/// <summary>Get the text for the column.</summary>
					/// <returns>The text for the column.</returns>
					/// <param name="column">The index for the column.</param>
					virtual WString										GetColumnText(vint column) = 0;
					/// <summary>Get the size for the column.</summary>
					/// <returns>The size for the column.</returns>
					/// <param name="column">The index for the column.</param>
					virtual vint										GetColumnSize(vint column) = 0;
					/// <summary>Set the size for the column.</summary>
					/// <param name="column">The index for the column.</param>
					/// <param name="value">The new size for the column.</param>
					virtual void										SetColumnSize(vint column, vint value) = 0;
					/// <summary>Get the popup binded to the column.</summary>
					/// <returns>The popup binded to the column.</returns>
					/// <param name="column">The index of the column.</param>
					virtual GuiMenu*									GetColumnPopup(vint column) = 0;
					/// <summary>Test is a column sortable.</summary>
					/// <returns>Returns true if this column is sortable.</returns>
					/// <param name="column">The index of the column.</param>
					virtual bool										IsColumnSortable(vint column) = 0;
					/// <summary>Set the column sorting state to update the data.</summary>
					/// <param name="column">The index of the column. Set to -1 means go back to the unsorted state.</param>
					/// <param name="ascending">Set to true if the data is sorted in ascending order.</param>
					virtual void										SortByColumn(vint column, bool ascending) = 0;
					/// <summary>Get the sorted columm. If no column is under a sorted state, it returns -1.</summary>
					/// <returns>The column number.</returns>
					virtual vint										GetSortedColumn() = 0;
					/// <summary>Test is the sort order ascending. </summary>
					/// <returns>Returns true if the sort order is ascending.</returns>
					virtual bool										IsSortOrderAscending() = 0;

					/// <summary>Get the number of all rows.</summary>
					/// <returns>The number of all rows.</returns>
					virtual vint										GetRowCount() = 0;
					/// <summary>Get the large image for the row.</summary>
					/// <returns>The large image.</returns>
					/// <param name="row">The row number.</param>
					virtual Ptr<GuiImageData>							GetRowLargeImage(vint row) = 0;
					/// <summary>Get the small image for the row.</summary>
					/// <returns>The small image.</returns>
					/// <param name="row">The row number.</param>
					virtual Ptr<GuiImageData>							GetRowSmallImage(vint row) = 0;
					/// <summary>Get the column span for the cell.</summary>
					/// <returns>The column span for the cell.</returns>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="column">The column number for the cell.</param>
					virtual vint										GetCellSpan(vint row, vint column) = 0;
					/// <summary>Get the text for the cell.</summary>
					/// <returns>The text for the cell.</returns>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="column">The column number for the cell.</param>
					virtual WString										GetCellText(vint row, vint column) = 0;
					/// <summary>Get the data visualizer factory that creates data visualizers for visualizing the cell.</summary>
					/// <returns>The data visualizer factory. The data grid control to use the predefined data visualizer if this function returns null.</returns>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="column">The column number for the cell.</param>
					virtual IDataVisualizerFactory*						GetCellDataVisualizerFactory(vint row, vint column) = 0;
					/// <summary>Called before visualizing the cell.</summary>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="column">The column number for the cell.</param>
					/// <param name="dataVisualizer">The data visualizer to be updated.</param>
					virtual void										VisualizeCell(vint row, vint column, IDataVisualizer* dataVisualizer) = 0;
					/// <summary>Get the data editor factory that creates data editors for editing the cell.</summary>
					/// <returns>The data editor factory. Returns null to disable editing.</returns>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="column">The column number for the cell.</param>
					virtual IDataEditorFactory*							GetCellDataEditorFactory(vint row, vint column) = 0;
					/// <summary>Called before editing the cell.</summary>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="column">The column number for the cell.</param>
					/// <param name="dataEditor">The data editor.</param>
					virtual void										BeforeEditCell(vint row, vint column, IDataEditor* dataEditor) = 0;
					/// <summary>Called when saving data for the editing cell.</summary>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="column">The column number for the cell.</param>
					/// <param name="dataEditor">The data editor.</param>
					virtual void										SaveCellData(vint row, vint column, IDataEditor* dataEditor) = 0;
				};

/***********************************************************************
DataSource Extensions
***********************************************************************/

				/// <summary>The command executor for [T:vl.presentation.controls.list.IStructuredDataFilter] to send notification.</summary>
				class IStructuredDataFilterCommandExecutor : public virtual IDescriptable, public Description<IStructuredDataFilterCommandExecutor>
				{
				public:
					/// <summary>Called when the filter structure or arguments are changed.</summary>
					virtual void										OnFilterChanged()=0;
				};

				/// <summary>Structured data filter.</summary>
				class IStructuredDataFilter : public virtual IDescriptable, public Description<IStructuredDataFilter>
				{
				public:
					/// <summary>Set the command executor.</summary>
					/// <param name="value">The command executor.</param>
					virtual void										SetCommandExecutor(IStructuredDataFilterCommandExecutor* value)=0;
					/// <summary>Filter a row.</summary>
					/// <returns>Returns true when a row is going to display on the control.</returns>
					/// <param name="row">The row number.</param>
					virtual bool										Filter(vint row)=0;
				};

				/// <summary>Structured data sorter.</summary>
				class IStructuredDataSorter : public virtual IDescriptable, public Description<IStructuredDataSorter>
				{
				public:
					/// <summary>Get the order of two rows.</summary>
					/// <returns>Returns 0 if the order doesn't matter. Returns negative number if row1 needs to put before row2. Returns positive number if row1 needs to put after row2.</returns>
					/// <param name="row1">The row number of the first row.</param>
					/// <param name="row2">The row number of the second row.</param>
					virtual vint										Compare(vint row1, vint row2)=0;
				};

				/// <summary>Structure data column.</summary>
				class IStructuredColumnProvider : public virtual IDescriptable, public Description<IStructuredColumnProvider>
				{
				public:
					/// <summary>Get the text for the column.</summary>
					/// <returns>The text of the column.</returns>
					virtual WString										GetText()=0;
					/// <summary>Get the size for the column.</summary>
					/// <returns>The size for the column.</returns>
					virtual vint										GetSize()=0;
					/// <summary>Set the size for the column.</summary>
					/// <param name="value">The new size for the column.</param>
					virtual void										SetSize(vint value)=0;
					/// <summary>Get the sorting state for the column.</summary>
					/// <returns>The sorting state.</returns>
					virtual GuiListViewColumnHeader::ColumnSortingState	GetSortingState()=0;
					/// <summary>Set the sorting state for the column. This state does not affect the row order. The column provider does not have to implement the reordering.</summary>
					/// <param name="value">The sorting state.</param>
					virtual void										SetSortingState(GuiListViewColumnHeader::ColumnSortingState value)=0;
					/// <summary>Get the popup binded to the column.</summary>
					/// <returns>The popup binded to the column.</returns>
					virtual GuiMenu*									GetPopup()=0;
					/// <summary>Get the inherent filter for the column.</summary>
					/// <returns>The inherent filter. Returns null if the column doesn't have a filter.</returns>
					virtual Ptr<IStructuredDataFilter>					GetInherentFilter()=0;
					/// <summary>Get the inherent sorter for the column.</summary>
					/// <returns>The inherent sorter. Returns null if the column doesn't have a sorter.</returns>
					virtual Ptr<IStructuredDataSorter>					GetInherentSorter()=0;
					
					/// <summary>Get the text for the cell.</summary>
					/// <returns>The text for the cell.</returns>
					/// <param name="row">The row number for the cell.</param>
					virtual WString										GetCellText(vint row)=0;
					/// <summary>Get the data visualizer factory that creates data visualizers for visualizing the cell.</summary>
					/// <returns>The data visualizer factory. The data grid control to use the predefined data visualizer if this function returns null.</returns>
					/// <param name="row">The row number for the cell.</param>
					virtual IDataVisualizerFactory*						GetCellDataVisualizerFactory(vint row)=0;
					/// <summary>Called before visualizing the cell.</summary>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="dataVisualizer">The data visualizer to be updated.</param>
					virtual void										VisualizeCell(vint row, IDataVisualizer* dataVisualizer)=0;
					/// <summary>Get the data editor factory that creates data editors for editing the cell.</summary>
					/// <returns>The data editor factory. Returns null to disable editing.</returns>
					/// <param name="row">The row number for the cell.</param>
					virtual IDataEditorFactory*							GetCellDataEditorFactory(vint row)=0;
					/// <summary>Called before editing the cell.</summary>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="dataEditor">The data editor.</param>
					virtual void										BeforeEditCell(vint row, IDataEditor* dataEditor)=0;
					/// <summary>Called when saving data for the editing cell.</summary>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="dataEditor">The data editor.</param>
					virtual void										SaveCellData(vint row, IDataEditor* dataEditor)=0;
				};

				/// <summary>Structured data provider for [T:vl.presentation.controls.GuiVirtualDataGrid].</summary>
				class IStructuredDataProvider : public virtual IDescriptable, public Description<IStructuredDataProvider>
				{
				public:
					/// <summary>Set the command executor.</summary>
					/// <param name="value">The command executor.</param>
					virtual void										SetCommandExecutor(IDataProviderCommandExecutor* value)=0;
					/// <summary>Get the view model context. It is used to create data visualizers and data editors.</summary>
					/// <returns>The view model context.</returns>
					virtual description::Value							GetViewModelContext() = 0;
					/// <summary>Get the number of all columns.</summary>
					/// <returns>The number of all columns.</returns>
					virtual vint										GetColumnCount()=0;
					/// <summary>Get the number of all rows.</summary>
					/// <returns>The number of all rows.</returns>
					virtual vint										GetRowCount()=0;
					/// <summary>Get the <see cref="IStructuredColumnProvider"/> object for the column.</summary>
					/// <returns>The <see cref="IStructuredColumnProvider"/> object.</returns>
					/// <param name="column">The column number.</param>
					virtual IStructuredColumnProvider*					GetColumn(vint column)=0;
					/// <summary>Get the large image for the row.</summary>
					/// <returns>The large image.</returns>
					/// <param name="row">The row number.</param>
					virtual Ptr<GuiImageData>							GetRowLargeImage(vint row)=0;
					/// <summary>Get the small image for the row.</summary>
					/// <returns>The small image.</returns>
					/// <param name="row">The row number.</param>
					virtual Ptr<GuiImageData>							GetRowSmallImage(vint row)=0;
				};
			}
		}
	}
}

#endif