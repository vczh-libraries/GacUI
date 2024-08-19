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
			class GuiVirtualDataGrid;

			namespace list
			{

/***********************************************************************
DefaultDataGridItemTemplate
***********************************************************************/

				class DefaultDataGridItemTemplate
					: public DefaultListViewItemTemplate
				{
				protected:
					compositions::GuiTableComposition*						textTable = nullptr;
					collections::Array<IDataVisualizerFactory*>				dataVisualizerFactories;
					collections::Array<Ptr<IDataVisualizer>>				dataVisualizers;
					collections::Array<compositions::GuiCellComposition*>	dataCells;
					IDataEditor*											currentEditor = nullptr;

					IDataVisualizerFactory*									GetDataVisualizerFactory(vint row, vint column);
					IDataEditorFactory*										GetDataEditorFactory(vint row, vint column);
					vint													GetCellColumnIndex(compositions::GuiGraphicsComposition* composition);
					bool													IsInEditor(GuiVirtualDataGrid* dataGrid, compositions::GuiMouseEventArgs& arguments);
					void													OnCellButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					void													OnCellLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					void													OnCellRightButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);

					void													DeleteAllVisualizers();
					void													DeleteVisualizer(vint column);
					void													ResetDataTable(vint columnCount);
					void													OnInitialize()override;
					void													OnRefresh()override;
					void													OnSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void													OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void													OnContextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void													OnVisuallyEnabledChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					DefaultDataGridItemTemplate();
					~DefaultDataGridItemTemplate();

					void													UpdateSubItemSize();
					bool													IsEditorOpened();
					void													NotifyOpenEditor(vint column, IDataEditor* editor);
					void													NotifyCloseEditor();
					void													NotifySelectCell(vint column);
					void													NotifyCellEdited();
				};
			}

/***********************************************************************
GuiVirtualDataGrid
***********************************************************************/

			/// <summary>Data grid control in virtual mode.</summary>
			class GuiVirtualDataGrid
				: public GuiVirtualListView
				, protected compositions::GuiAltActionHostBase
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

				bool													skipOnSelectionChanged = false;
				GridPos													selectedCell{ -1,-1 };
				Ptr<list::IDataEditor>									currentEditor;
				GridPos													currentEditorPos{ -1,-1 };
				bool													currentEditorOpeningEditor = false;

				compositions::IGuiAltActionHost*						GetActivatingAltHost()override;
				void													NotifySelectionChanged(bool triggeredByItemContentModified)override;
				void													OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
				void													OnStyleInstalled(vint index, ItemStyle* style, bool refreshPropertiesOnly)override;
				void													OnStyleUninstalled(ItemStyle* style)override;

				void													NotifyCloseEditor();
				void													NotifySelectCell(vint row, vint column);
				bool													StartEdit(vint row, vint column);
				void													StopEdit();
				void													OnColumnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);
				void													OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments);
				void													OnKeyUp(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments);

			public:
				templates::GuiListViewTemplate*							GetListViewControlTemplate()override;
				void													RequestSaveData()override;

			public:
				/// <summary>Create a data grid control in virtual mode.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiVirtualDataGrid(theme::ThemeName themeName, list::IItemProvider* _itemProvider);
				~GuiVirtualDataGrid();

				/// <summary>Selected cell changed event.</summary>
				compositions::GuiNotifyEvent							SelectedCellChanged;

				list::IItemProvider*									GetItemProvider()override;

				/// <summary>Change the view to data grid's default view.</summary>
				void													SetViewToDefault();

				/// <summary>Get the row index and column index of the selected cell.</summary>
				/// <returns>The row index and column index of the selected cell.</returns>
				GridPos													GetSelectedCell();

				/// <summary>Select a cell.</summary>
				/// <returns>Returns true if the editor is opened.</returns>
				/// <param name="value">The row index and column index of the selected cell.</param>
				/// <param name="openEditor">Set to true to open an editor.</param>
				bool													SelectCell(const GridPos& value, bool openEditor);
			};
		}
	}
}

#endif
