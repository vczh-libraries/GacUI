/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDATAGRIDCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIDATAGRIDCONTROLS

#include "GuiDataGridInterfaces.h"
#include "GuiListViewItemTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			namespace list
			{

/***********************************************************************
DefaultDataGridItemTemplate
***********************************************************************/

				class DefaultDataGridItemTemplate
					: public DefaultListViewItemTemplate
					, private ListViewColumnItemArranger::IColumnItemViewCallback
				{
				protected:
					compositions::GuiTableComposition*					textTable = nullptr;
					collections::Array<Ptr<IDataVisualizer>>			dataVisualizers;
					IDataEditor*										currentEditor = nullptr;

					IDataVisualizerFactory*								GetDataVisualizerFactory(vint row, vint column);
					IDataEditorFactory*									GetDataEditorFactory(vint row, vint column);
					vint												GetCellColumnIndex(compositions::GuiGraphicsComposition* composition);
					void												OnCellButtonUp(compositions::GuiGraphicsComposition* sender, bool openEditor);
					bool												IsInEditor(compositions::GuiMouseEventArgs& arguments);
					void												OnCellButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					void												OnCellLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					void												OnCellRightButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);

					void												OnColumnChanged()override;
					void												OnInitialize()override;
					void												OnSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					DefaultDataGridItemTemplate();
					~DefaultDataGridItemTemplate();

					void												UpdateSubItemSize();
					void												ForceSetEditor(vint column, IDataEditor* editor);
					void												NotifyCloseEditor();
					void												NotifySelectCell(vint column);
				};
			}

/***********************************************************************
GuiVirtualDataGrid
***********************************************************************/

			/// <summary>Data grid control in virtual mode.</summary>
			class GuiVirtualDataGrid
				: public GuiVirtualListView
				, public Description<GuiVirtualDataGrid>
			{
				friend class list::DefaultDataGridItemTemplate;
			protected:
				list::IListViewItemView*								listViewItemView = nullptr;
				list::ListViewColumnItemArranger::IColumnItemView*		columnItemView = nullptr;
				list::IDataGridView*									dataGridView = nullptr;

				GridPos													selectedCell{ -1,-1 };
				Ptr<list::IDataEditor>									currentEditor;
				GridPos													currentEditorPos{ -1,-1 };
				bool													currentEditorRequestingSaveData = false;
				bool													currentEditorOpeningEditor = false;

				void													OnItemModified(vint start, vint count, vint newCount)override;

				void													NotifyCloseEditor();
				void													NotifySelectCell(vint row, vint column);
				void													RequestSaveData();
				list::IDataEditor*										OpenEditor(vint row, vint column, list::IDataEditorFactory* editorFactory);
				void													CloseEditor(bool forOpenNewEditor);

				void													OnColumnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);
			public:
				/// <summary>Create a data grid control in virtual mode.</summary>
				/// <param name="_styleProvider">The style provider for this control.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiVirtualDataGrid(IStyleProvider* _styleProvider, GuiListControl::IItemProvider* _itemProvider);
				~GuiVirtualDataGrid();

				/// <summary>Selected cell changed event.</summary>
				compositions::GuiNotifyEvent							SelectedCellChanged;


				/// <summary>Get the row index and column index of the selected cell.</summary>
				/// <returns>The row index and column index of the selected cell.</returns>
				GridPos													GetSelectedCell();
				/// <summary>Set the row index and column index of the selected cell.</summary>
				/// <param name="value">The row index and column index of the selected cell.</param>
				void													SetSelectedCell(const GridPos& value);
			};

			namespace list
			{

/***********************************************************************
DataGridItemProvider
***********************************************************************/
				/*
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
				*/
			}
		}
	}
}

#endif
