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
					, public ListViewColumnItemArranger::IColumnItemViewCallback
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
					void												OnContextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					DefaultDataGridItemTemplate();
					~DefaultDataGridItemTemplate();

					void												UpdateSubItemSize();
					bool												IsEditorOpened();
					void												NotifyOpenEditor(vint column, IDataEditor* editor);
					void												NotifyCloseEditor();
					void												NotifySelectCell(vint column);
					void												NotifyCellEdited();
				};
			}

/***********************************************************************
GuiVirtualDataGrid
***********************************************************************/

			/// <summary>Data grid control in virtual mode.</summary>
			class GuiVirtualDataGrid
				: public GuiVirtualListView
				, private list::IDataGridContext
				, public Description<GuiVirtualDataGrid>
			{
				friend class list::DefaultDataGridItemTemplate;
			protected:
				list::IListViewItemView*								listViewItemView = nullptr;
				list::ListViewColumnItemArranger::IColumnItemView*		columnItemView = nullptr;
				list::IDataGridView*									dataGridView = nullptr;
				Ptr<list::IDataVisualizerFactory>						defaultMainColumnVisualizerFactory;
				Ptr<list::IDataVisualizerFactory>						defaultSubColumnVisualizerFactory;

				GridPos													selectedCell{ -1,-1 };
				Ptr<list::IDataEditor>									currentEditor;
				GridPos													currentEditorPos{ -1,-1 };
				bool													currentEditorOpeningEditor = false;

				void													OnItemModified(vint start, vint count, vint newCount)override;
				void													OnStyleUninstalled(ItemStyle* style)override;

				void													NotifyCloseEditor();
				void													NotifySelectCell(vint row, vint column);
				bool													StartEdit(vint row, vint column);
				void													StopEdit(bool forOpenNewEditor);
				void													OnColumnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);

			public:
				templates::GuiListViewTemplate*							GetListViewControlTemplate()override;
				void													RequestSaveData()override;

			public:
				/// <summary>Create a data grid control in virtual mode.</summary>
				/// <param name="_controlTemplate">The control template for this control.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiVirtualDataGrid(theme::ThemeName themeName, GuiListControl::IItemProvider* _itemProvider);
				~GuiVirtualDataGrid();

				/// <summary>Selected cell changed event.</summary>
				compositions::GuiNotifyEvent							SelectedCellChanged;

				IItemProvider*											GetItemProvider()override;

				/// <summary>Change the view to data grid's default view.</summary>
				void													SetViewToDefault();

				/// <summary>Get the row index and column index of the selected cell.</summary>
				/// <returns>The row index and column index of the selected cell.</returns>
				GridPos													GetSelectedCell();
				/// <summary>Set the row index and column index of the selected cell.</summary>
				/// <param name="value">The row index and column index of the selected cell.</param>
				void													SetSelectedCell(const GridPos& value);
			};
		}
	}
}

#endif
