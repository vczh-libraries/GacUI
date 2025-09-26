/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUILISTVIEWCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUILISTVIEWCONTROLS

#include "ItemProvider_IListViewItemView.h"
#include "ItemTemplate_IListViewItemView.h"
#include "GuiListControlItemArrangers.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			///<summary>List view column header control for detailed view.</summary>
			class GuiListViewColumnHeader : public GuiMenuButton, public Description<GuiListViewColumnHeader>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(ListViewColumnHeaderTemplate, GuiMenuButton)
			protected:
				ColumnSortingState								columnSortingState = ColumnSortingState::NotSorted;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiListViewColumnHeader(theme::ThemeName themeName);
				~GuiListViewColumnHeader();

				bool											IsAltAvailable()override;

				/// <summary>Get the column sorting state.</summary>
				/// <returns>The column sorting state.</returns>
				ColumnSortingState								GetColumnSortingState();
				/// <summary>Set the column sorting state.</summary>
				/// <param name="value">The new column sorting state.</param>
				void											SetColumnSortingState(ColumnSortingState value);
			};

			/// <summary>List view base control. All list view controls inherit from this class.</summary>
			class GuiListViewBase : public GuiSelectableListControl, public Description<GuiListViewBase>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(ListViewTemplate, GuiSelectableListControl)
			public:
				/// <summary>Create a list view base control.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiListViewBase(theme::ThemeName themeName, list::IItemProvider* _itemProvider);
				~GuiListViewBase();

				/// <summary>Column clicked event.</summary>
				compositions::GuiItemNotifyEvent				ColumnClicked;
			};

			namespace list
			{

/***********************************************************************
ListViewColumnItemArranger
***********************************************************************/

				/// <summary>List view column item arranger. This arranger contains column headers. When an column header is resized, all items will be notified via the [T:vl.presentation.controls.list.ListViewColumnItemArranger.IColumnItemView] for <see cref="GuiListControl::IItemProvider"/>.</summary>
				class ListViewColumnItemArranger : public VirtualRepeatRangedItemArrangerBase<compositions::GuiRepeatFixedHeightItemComposition>, public Description<ListViewColumnItemArranger>
				{
					using TBase = VirtualRepeatRangedItemArrangerBase<compositions::GuiRepeatFixedHeightItemComposition>;
					typedef collections::List<GuiListViewColumnHeader*>					ColumnHeaderButtonList;
					typedef collections::List<compositions::GuiBoundsComposition*>		ColumnHeaderSplitterList;
				public:
					static const vint							SplitterWidth = 8;
				protected:
					class ColumnItemViewCallback : public Object, public virtual IColumnItemViewCallback
					{
					protected:
						ListViewColumnItemArranger*				arranger = nullptr;

					public:
						ColumnItemViewCallback(ListViewColumnItemArranger* _arranger);
						~ColumnItemViewCallback();

						void									OnColumnRebuilt() override;
						void									OnColumnChanged(bool needToRefreshItems) override;
					};

					class ColumnItemArrangerRepeatComposition : public TBase::ArrangerRepeatComposition
					{
					protected:
						ListViewColumnItemArranger*				arranger = nullptr;

						void									Layout_EndLayout(bool totalSizeUpdated) override;
						void									Layout_CalculateTotalSize(Size& full, Size& minimum) override;
					public:
						ColumnItemArrangerRepeatComposition(ListViewColumnItemArranger* _arranger);
					};

					GuiListViewBase*							listView = nullptr;
					IListViewItemView*							listViewItemView = nullptr;
					IColumnItemView*							columnItemView = nullptr;
					Ptr<ColumnItemViewCallback>					columnItemViewCallback;
					compositions::GuiStackComposition*			columnHeaders = nullptr;
					ColumnHeaderButtonList						columnHeaderButtons;
					ColumnHeaderSplitterList					columnHeaderSplitters;
					bool										splitterDragging = false;
					vint										splitterLatestX = 0;

					void										OnViewLocationChanged(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments);
					void										ColumnClicked(vint index, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void										ColumnCachedBoundsChanged(vint index, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void										ColumnHeaderSplitterLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					void										ColumnHeaderSplitterLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					void										ColumnHeaderSplitterMouseMove(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
					void										ColumnHeadersCachedBoundsChanged(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments);

					void										FixColumnsAfterViewLocationChanged();
					void										FixColumnsAfterLayout();
					vint										GetColumnsWidth();
					vint										GetColumnsYOffset();
					void										DeleteColumnButtons();
					void										RebuildColumns();
					void										UpdateRepeatConfig();
					void										RefreshColumns();
				public:
					ListViewColumnItemArranger();
					~ListViewColumnItemArranger();

					Size										GetTotalSize()override;
					void										AttachListControl(GuiListControl* value)override;
					void										DetachListControl()override;

					/// <summary>Get all column buttons for the Detail view.</summary>
					/// <returns>All column buttons</returns>
					const ColumnHeaderButtonList&				GetColumnButtons();
					/// <summary>Get all column splitters for the Detail view.</summary>
					/// <returns>All column spitters</returns>
					const ColumnHeaderSplitterList&				GetColumnSplitters();
				};
			}

/***********************************************************************
GuiVirtualListView
***********************************************************************/

			enum class ListViewView
			{
				BigIcon,
				SmallIcon,
				List,
				Tile,
				Information,
				Detail,
				Unknown,
			};
			
			/// <summary>List view control in virtual mode.</summary>
			class GuiVirtualListView : public GuiListViewBase, public Description<GuiVirtualListView>
			{
			protected:
				ListViewView											view = ListViewView::Unknown;

				void													OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly)override;
				void													OnItemTemplateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a list view control in virtual mode.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiVirtualListView(theme::ThemeName themeName, list::IItemProvider* _itemProvider);
				~GuiVirtualListView();

				/// <summary>Get the current view.</summary>
				/// <returns>The current view. After [M:vl.presentation.controls.GuiListControl.SetItemTemplate] is called, the current view is reset to Unknown.</returns>
				ListViewView											GetView();
				/// <summary>Set the current view.</summary>
				/// <param name="_view">The current view.</param>
				void													SetView(ListViewView _view);
			};

/***********************************************************************
GuiListView
***********************************************************************/
			
			/// <summary>List view control in virtual mode.</summary>
			class GuiListView : public GuiVirtualListView, public Description<GuiListView>
			{
			protected:
				list::ListViewItemProvider*								items;
			public:
				/// <summary>Create a list view control.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiListView(theme::ThemeName themeName);
				~GuiListView();
				
				/// <summary>Get all list view items.</summary>
				/// <returns>All list view items.</returns>
				list::ListViewItemProvider&								GetItems();
				/// <summary>Get all data columns indices in columns.</summary>
				/// <returns>All data columns indices in columns.</returns>
				list::ListViewDataColumns&								GetDataColumns();
				/// <summary>Get all columns.</summary>
				/// <returns>All columns.</returns>
				list::ListViewColumns&									GetColumns();

				/// <summary>Get the selected item.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				Ptr<list::ListViewItem>									GetSelectedItem();
			};
		}
	}
}

#endif