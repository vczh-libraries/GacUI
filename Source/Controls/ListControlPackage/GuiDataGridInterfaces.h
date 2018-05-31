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

				class IDataVisualizer;
				class IDataEditor;

				/// <summary>The data grid context.</summary>
				class IDataGridContext : public virtual IDescriptable, public Description<IDataGridContext>
				{
				public:
					virtual GuiListControl::IItemProvider*				GetItemProvider() = 0;
					virtual templates::GuiListViewTemplate*				GetListViewControlTemplate() = 0;
					virtual void										RequestSaveData() = 0;
				};

				/// <summary>The visualizer factory.</summary>
				class IDataVisualizerFactory : public virtual IDescriptable, public Description<IDataVisualizerFactory>
				{
				public:
					/// <summary>Create a data visualizer.</summary>
					/// <returns>The created data visualizer.</returns>
					/// <param name="dataGridContext">Context information of the data grid.</param>
					virtual Ptr<IDataVisualizer>						CreateVisualizer(IDataGridContext* dataGridContext) = 0;
				};

				/// <summary>The visualizer for each cell in [T:vl.presentation.controls.GuiVirtualDataGrid].</summary>
				class IDataVisualizer : public virtual IDescriptable, public Description<IDataVisualizer>
				{
				public:
					/// <summary>Get the factory object that creates this visualizer.</summary>
					/// <returns>The factory object.</returns>
					virtual IDataVisualizerFactory*						GetFactory() = 0;

					/// <summary>Get the template that renders the data. The data visualizer should maintain this template, and delete it when necessary.</summary>
					/// <returns>The template.</returns>
					virtual templates::GuiGridVisualizerTemplate*		GetTemplate() = 0;
					/// <summary>Notify that the template has been deleted during the deconstruction of UI objects.</summary>
					virtual void										NotifyDeletedTemplate() = 0;

					/// <summary>Called before visualizing a cell.</summary>
					/// <param name="itemProvider">The item provider.</param>
					/// <param name="row">The row number of the cell.</param>
					/// <param name="column">The column number of the cell.</param>
					virtual void										BeforeVisualizeCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column) = 0;

					/// <summary>Set the selected state.</summary>
					/// <param name="value">Set to true to make this data visualizer looks selected.</param>
					virtual void										SetSelected(bool value) = 0;
				};

				/// <summary>The editor factory.</summary>
				class IDataEditorFactory : public virtual IDescriptable, public Description<IDataEditorFactory>
				{
				public:
					/// <summary>Create a data editor.</summary>
					/// <returns>The created data editor.</returns>
					/// <param name="dataGridContext">Context information of the data grid.</param>
					virtual Ptr<IDataEditor>							CreateEditor(IDataGridContext* dataGridContext) = 0;
				};

				/// <summary>The editor for each cell in [T:vl.presentation.controls.GuiVirtualDataGrid].</summary>
				class IDataEditor : public virtual IDescriptable, public Description<IDataEditor>
				{
				public:
					/// <summary>Get the factory object that creates this editor.</summary>
					/// <returns>The factory object.</returns>
					virtual IDataEditorFactory*							GetFactory() = 0;

					/// <summary>Get the template that edit the data. The data editor should maintain this template, and delete it when necessary.</summary>
					/// <returns>The template.</returns>
					virtual templates::GuiGridEditorTemplate*			GetTemplate() = 0;
					/// <summary>Notify that the template has been deleted during the deconstruction of UI objects.</summary>
					virtual void										NotifyDeletedTemplate() = 0;

					/// <summary>Called before editing a cell.</summary>
					/// <param name="itemProvider">The item provider.</param>
					/// <param name="row">The row number of the cell.</param>
					/// <param name="column">The column number of the cell.</param>
					virtual void										BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column) = 0;

					/// <summary>Test if the edit has saved the data.</summary>
					/// <returns>Returns true if the data is saved.</returns>
					virtual bool										GetCellValueSaved() = 0;
				};

				/// <summary>The required <see cref="GuiListControl::IItemProvider"/> view for [T:vl.presentation.controls.GuiVirtualDataGrid].</summary>
				class IDataGridView : public virtual IDescriptable, public Description<IDataGridView>
				{
				public:
					/// <summary>The identifier for this view.</summary>
					static const wchar_t* const							Identifier;

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

					/// <summary>Get the column span for the cell.</summary>
					/// <returns>The column span for the cell.</returns>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="column">The column number for the cell.</param>
					virtual vint										GetCellSpan(vint row, vint column) = 0;
					/// <summary>Get the data visualizer factory that creates data visualizers for visualizing the cell.</summary>
					/// <returns>The data visualizer factory. The data grid control to use the predefined data visualizer if this function returns null.</returns>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="column">The column number for the cell.</param>
					virtual IDataVisualizerFactory*						GetCellDataVisualizerFactory(vint row, vint column) = 0;
					/// <summary>Get the data editor factory that creates data editors for editing the cell.</summary>
					/// <returns>The data editor factory. Returns null to disable editing.</returns>
					/// <param name="row">The row number for the cell.</param>
					/// <param name="column">The column number for the cell.</param>
					virtual IDataEditorFactory*							GetCellDataEditorFactory(vint row, vint column) = 0;
					/// <summary>Get the binding value of a cell.</summary>
					/// <returns>The binding value of cell.</returns>
					/// <param name="row">The row index of the cell.</param>
					/// <param name="column">The column index of the cell.</param>
					virtual description::Value							GetBindingCellValue(vint row, vint column) = 0;
					/// <summary>Set the binding value of a cell.</summary>
					/// <param name="row">The row index of the cell.</param>
					/// <param name="column">The column index of the cell.</param>
					/// <param name="value">The value to set.</param>
					virtual void										SetBindingCellValue(vint row, vint column, const description::Value& value) = 0;
				};
			}
		}
	}
}

#endif
