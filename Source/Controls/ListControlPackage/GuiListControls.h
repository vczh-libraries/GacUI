/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUILISTCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUILISTCONTROLS

#include "DataSourceImpl_IItemProvider_ItemProviderBase.h"
#include "../GuiContainerControls.h"
#include "../../GraphicsComposition/GuiGraphicsAxis.h"

namespace vl
{
	namespace presentation
	{
		namespace templates
		{
			class GuiListItemTemplate;
		}

		namespace controls
		{

/***********************************************************************
List Control
***********************************************************************/

			/// <summary>Represents a list control. A list control automatically read data sources and creates corresponding data item control from the item template.</summary>
			class GuiListControl : public GuiScrollView, public Description<GuiListControl>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(ListControlTemplate, GuiScrollView)
			public:
				using ItemStyle = templates::GuiListItemTemplate;
				using ItemStyleBounds = templates::GuiTemplate;
				using ItemStyleRecord = collections::Pair<ItemStyle*, ItemStyleBounds*>;
				using ItemStyleProperty = TemplateProperty<templates::GuiListItemTemplate>;

				//-----------------------------------------------------------
				// IItemArrangerCallback
				//-----------------------------------------------------------

				/// <summary>Item arranger callback. Item arrangers use this interface to communicate with the list control. When setting positions for item controls, functions in this callback object is suggested to call because they use the result from the [T:vl.presentation.controls.compositions.IGuiAxis].</summary>
				class IItemArrangerCallback : public virtual IDescriptable, public Description<IItemArrangerCallback>
				{
				public:
					/// <summary>Create an item control representing an item in the item provider. This function is suggested to call when an item control gets into the visible area.</summary>
					/// <returns>The item control.</returns>
					/// <param name="itemIndex">The index of the item in the item provider.</param>
					virtual ItemStyle*								CreateItem(vint itemIndex)=0;
					/// <summary>Get the most outer bounds from an item control.</summary>
					/// <returns>The most outer bounds. When <see cref="GuiListControl::GetDisplayItemBackground/> returns true, the item is wrapped in other compositions.</returns>
					/// <param name="style">The item control.</param>
					virtual ItemStyleBounds*						GetItemBounds(ItemStyle* style)=0;
					/// <summary>Get the item control from its most outer bounds.</summary>
					/// <returns>The item control.</returns>
					/// <param name="style">The most outer bounds.</param>
					virtual ItemStyle*								GetItem(ItemStyleBounds* bounds)=0;
					/// <summary>Release an item control. This function is suggested to call when an item control gets out of the visible area.</summary>
					/// <param name="style">The item control.</param>
					virtual void									ReleaseItem(ItemStyle* style)=0;
					/// <summary>Update the view location. The view location is the left-top position in the logic space of the list control.</summary>
					/// <param name="value">The new view location.</param>
					virtual void									SetViewLocation(Point value)=0;
					/// <summary>Get the <see cref="compositions::GuiGraphicsComposition"/> that directly contains item controls.</summary>
					/// <returns>The <see cref="compositions::GuiGraphicsComposition"/> that directly contains item controls.</returns>
					virtual compositions::GuiGraphicsComposition*	GetContainerComposition()=0;
					/// <summary>Notify the list control that the total size of all item controls are changed.</summary>
					virtual void									OnTotalSizeChanged()=0;
					/// <summary>Notify the list control that the adopted size of the list control is changed.</summary>
					virtual void									OnAdoptedSizeChanged()=0;
				};

				//-----------------------------------------------------------
				// IItemArranger
				//-----------------------------------------------------------

				/// <summary>EnsureItemVisible result for item arranger.</summary>
				enum class EnsureItemVisibleResult
				{
					/// <summary>The requested item does not exist.</summary>
					ItemNotExists,
					/// <summary>The view location is moved.</summary>
					Moved,
					/// <summary>The view location is not moved.</summary>
					NotMoved,
				};
				
				/// <summary>Item arranger for a <see cref="GuiListControl"/>. Item arranger decides how to arrange and item controls. When implementing an item arranger, <see cref="IItemArrangerCallback"/> is suggested to use when calculating locations and sizes for item controls.</summary>
				class IItemArranger : public virtual list::IItemProviderCallback, public Description<IItemArranger>
				{
				public:
					/// <summary>Called when an item arranger in installed to a <see cref="GuiListControl"/>.</summary>
					/// <param name="value">The list control.</param>
					virtual void								AttachListControl(GuiListControl* value) = 0;
					/// <summary>Called when an item arranger in uninstalled from a <see cref="GuiListControl"/>.</summary>
					virtual void								DetachListControl() = 0;
					/// <summary>Get the binded item arranger callback object.</summary>
					/// <returns>The binded item arranger callback object.</returns>
					virtual IItemArrangerCallback*				GetCallback() = 0;
					/// <summary>Bind the item arranger callback object.</summary>
					/// <param name="value">The item arranger callback object to bind.</param>
					virtual void								SetCallback(IItemArrangerCallback* value) = 0;
					/// <summary>Get the total size of all data controls.</summary>
					/// <returns>The total size.</returns>
					virtual Size								GetTotalSize() = 0;
					/// <summary>Get the item style controller for an visible item index. If an item is not visible, it returns null.</summary>
					/// <returns>The item style controller.</returns>
					/// <param name="itemIndex">The item index.</param>
					virtual ItemStyle*							GetVisibleStyle(vint itemIndex) = 0;
					/// <summary>Get the item index for an visible item style controller. If an item is not visible, it returns -1.</summary>
					/// <returns>The item index.</returns>
					/// <param name="style">The item style controller.</param>
					virtual vint								GetVisibleIndex(ItemStyle* style) = 0;
					/// <summary>Reload all visible items.</summary>
					virtual void								ReloadVisibleStyles() = 0;
					/// <summary>Called when the visible area of item container is changed.</summary>
					/// <param name="bounds">The new visible area.</param>
					virtual void								OnViewChanged(Rect bounds) = 0;
					/// <summary>Find the item by an base item and a key direction.</summary>
					/// <returns>The item index that is found. Returns -1 if this operation failed.</returns>
					/// <param name="itemIndex">The base item index.</param>
					/// <param name="key">The key direction.</param>
					virtual vint								FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key) = 0;
					/// <summary>Adjust the view location to make an item visible.</summary>
					/// <returns>Returns the result of this operation.</returns>
					/// <param name="itemIndex">The item index of the item to be made visible.</param>
					virtual EnsureItemVisibleResult				EnsureItemVisible(vint itemIndex) = 0;
					/// <summary>Get the adopted size for the view bounds.</summary>
					/// <returns>The adopted size, making the vids bounds just enough to display several items.</returns>
					/// <param name="expectedSize">The expected size, to provide a guidance.</param>
					virtual Size								GetAdoptedSize(Size expectedSize) = 0;
				};

			protected:

				//-----------------------------------------------------------
				// ItemCallback
				//-----------------------------------------------------------

				class ItemCallback : public list::IItemProviderCallback, public IItemArrangerCallback
				{
					typedef collections::Dictionary<ItemStyle*, templates::GuiTemplate*>	InstalledStyleMap;
				protected:
					GuiListControl*								listControl = nullptr;
					list::IItemProvider*						itemProvider = nullptr;
					InstalledStyleMap							installedStyles;

					ItemStyleRecord								InstallStyle(ItemStyle* style, vint itemIndex);
					ItemStyleRecord								UninstallStyle(vint index);
				public:
					ItemCallback(GuiListControl* _listControl);
					~ItemCallback();

					void										ClearCache();

					void										OnAttached(list::IItemProvider* provider)override;
					void										OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
					ItemStyle*									CreateItem(vint itemIndex)override;
					ItemStyleBounds*							GetItemBounds(ItemStyle* style)override;
					ItemStyle*									GetItem(ItemStyleBounds* bounds)override;
					void										ReleaseItem(ItemStyle* style)override;
					void										SetViewLocation(Point value)override;
					compositions::GuiGraphicsComposition*		GetContainerComposition()override;
					void										OnTotalSizeChanged()override;
					void										OnAdoptedSizeChanged()override;
				};

				//-----------------------------------------------------------
				// State management
				//-----------------------------------------------------------

				Ptr<ItemCallback>								callback;
				Ptr<list::IItemProvider>						itemProvider;
				ItemStyleProperty								itemStyleProperty;
				Ptr<IItemArranger>								itemArranger;
				Ptr<compositions::IGuiAxis>						axis;
				Size											fullSize;
				Size											adoptedSizeDiffWithScroll = { -1,-1 };
				Size											adoptedSizeDiffWithoutScroll = { -1,-1 };
				bool											displayItemBackground = true;

				virtual void									OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated);
				virtual void									OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly);
				virtual void									OnStyleUninstalled(ItemStyle* style);
				
				void											OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)override;
				void											OnBeforeReleaseGraphicsHost()override;
				Size											QueryFullSize()override;
				void											UpdateView(Rect viewBounds)override;
				
				void											OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void											SetStyleAndArranger(ItemStyleProperty styleProperty, Ptr<IItemArranger> arranger);

				//-----------------------------------------------------------
				// Item event management
				//-----------------------------------------------------------

				class VisibleStyleHelper
				{
				public:
					Ptr<compositions::IGuiGraphicsEventHandler>		leftButtonDownHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		leftButtonUpHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		leftButtonDoubleClickHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		middleButtonDownHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		middleButtonUpHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		middleButtonDoubleClickHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		rightButtonDownHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		rightButtonUpHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		rightButtonDoubleClickHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		mouseMoveHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		mouseEnterHandler;
					Ptr<compositions::IGuiGraphicsEventHandler>		mouseLeaveHandler;
				};
				
				friend class collections::ArrayBase<Ptr<VisibleStyleHelper>>;
				collections::Dictionary<ItemStyle*, Ptr<VisibleStyleHelper>>		visibleStyles;

				void											UpdateDisplayFont()override;
				void											OnVisuallyEnabledChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void											OnContextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void											OnItemMouseEvent(compositions::GuiItemMouseEvent& itemEvent, ItemStyle* style, compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void											OnItemNotifyEvent(compositions::GuiItemNotifyEvent& itemEvent, ItemStyle* style, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void											AttachItemEvents(ItemStyle* style);
				void											DetachItemEvents(ItemStyle* style);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_itemProvider">The item provider as a data source.</param>
				/// <param name="acceptFocus">Set to true if the list control is allowed to have a keyboard focus.</param>
				GuiListControl(theme::ThemeName themeName, list::IItemProvider* _itemProvider, bool acceptFocus=false);
				~GuiListControl();

				/// <summary>Style provider changed event.</summary>
				compositions::GuiNotifyEvent					ItemTemplateChanged;
				/// <summary>Arranger changed event.</summary>
				compositions::GuiNotifyEvent					ArrangerChanged;
				/// <summary>Coordinate transformer changed event.</summary>
				compositions::GuiNotifyEvent					AxisChanged;
				/// <summary>Adopted size invalidated.</summary>
				compositions::GuiNotifyEvent					AdoptedSizeInvalidated;

				/// <summary>Item left mouse button down event.</summary>
				compositions::GuiItemMouseEvent					ItemLeftButtonDown;
				/// <summary>Item left mouse button up event.</summary>
				compositions::GuiItemMouseEvent					ItemLeftButtonUp;
				/// <summary>Item left mouse button double click event.</summary>
				compositions::GuiItemMouseEvent					ItemLeftButtonDoubleClick;
				/// <summary>Item middle mouse button down event.</summary>
				compositions::GuiItemMouseEvent					ItemMiddleButtonDown;
				/// <summary>Item middle mouse button up event.</summary>
				compositions::GuiItemMouseEvent					ItemMiddleButtonUp;
				/// <summary>Item middle mouse button double click event.</summary>
				compositions::GuiItemMouseEvent					ItemMiddleButtonDoubleClick;
				/// <summary>Item right mouse button down event.</summary>
				compositions::GuiItemMouseEvent					ItemRightButtonDown;
				/// <summary>Item right mouse button up event.</summary>
				compositions::GuiItemMouseEvent					ItemRightButtonUp;
				/// <summary>Item right mouse button double click event.</summary>
				compositions::GuiItemMouseEvent					ItemRightButtonDoubleClick;
				/// <summary>Item mouse move event.</summary>
				compositions::GuiItemMouseEvent					ItemMouseMove;
				/// <summary>Item mouse enter event.</summary>
				compositions::GuiItemNotifyEvent				ItemMouseEnter;
				/// <summary>Item mouse leave event.</summary>
				compositions::GuiItemNotifyEvent				ItemMouseLeave;

				/// <summary>Get the item provider.</summary>
				/// <returns>The item provider.</returns>
				virtual list::IItemProvider*					GetItemProvider();
				/// <summary>Get the item style provider.</summary>
				/// <returns>The item style provider.</returns>
				virtual ItemStyleProperty						GetItemTemplate();
				/// <summary>Set the item style provider</summary>
				/// <param name="value">The new item style provider</param>
				virtual void									SetItemTemplate(ItemStyleProperty value);
				/// <summary>Get the item arranger.</summary>
				/// <returns>The item arranger.</returns>
				virtual IItemArranger*							GetArranger();
				/// <summary>Set the item arranger</summary>
				/// <param name="value">The new item arranger</param>
				virtual void									SetArranger(Ptr<IItemArranger> value);
				/// <summary>Get the item coordinate transformer.</summary>
				/// <returns>The item coordinate transformer.</returns>
				virtual compositions::IGuiAxis*					GetAxis();
				/// <summary>Set the item coordinate transformer</summary>
				/// <param name="value">The new item coordinate transformer</param>
				virtual void									SetAxis(Ptr<compositions::IGuiAxis> value);
				/// <summary>Adjust the view location to make an item visible.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="itemIndex">The item index of the item to be made visible.</param>
				virtual bool									EnsureItemVisible(vint itemIndex);
				/// <summary>Get the adopted size for the list control.</summary>
				/// <returns>The adopted size, making the list control just enough to display several items.</returns>
				/// <param name="expectedSize">The expected size, to provide a guidance.</param>
				virtual Size									GetAdoptedSize(Size expectedSize);
				/// <summary>Test if the list control displays predefined item background.</summary>
				/// <returns>Returns true if the list control displays predefined item background.</returns>
				bool											GetDisplayItemBackground();
				/// <summary>Set if the list control displays predefined item background.</summary>
				/// <param name="value">Set to true to display item background.</param>
				void											SetDisplayItemBackground(bool value);
			};

/***********************************************************************
Selectable List Control
***********************************************************************/

			/// <summary>Represents a list control that each item is selectable.</summary>
			class GuiSelectableListControl : public GuiListControl, public Description<GuiSelectableListControl>
			{
			protected:

				collections::SortedList<vint>					selectedItems;
				bool											multiSelect;
				vint											selectedItemIndexStart;
				vint											selectedItemIndexEnd;

				virtual void									NotifySelectionChanged(bool triggeredByItemContentModified);
				void											OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
				void											OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly)override;
				virtual void									OnItemSelectionChanged(vint itemIndex, bool value);
				virtual void									OnItemSelectionCleared();
				void											OnItemLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments);
				void											OnItemRightButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments);

				void											NormalizeSelectedItemIndexStartEnd();
				void											SetMultipleItemsSelectedSilently(vint start, vint end, bool selected);
				void											OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments);

				virtual vint									FindItemByVirtualKeyDirection(vint index, compositions::KeyDirection keyDirection);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_itemProvider">The item provider as a data source.</param>
				GuiSelectableListControl(theme::ThemeName themeName, list::IItemProvider* _itemProvider);
				~GuiSelectableListControl();

				/// <summary>Selection changed event.</summary>
				compositions::GuiNotifyEvent					SelectionChanged;

				/// <summary>Get the multiple selection mode.</summary>
				/// <returns>Returns true if multiple selection is enabled.</returns>
				bool											GetMultiSelect();
				/// <summary>Set the multiple selection mode.</summary>
				/// <param name="value">Set to true to enable multiple selection.</param>
				void											SetMultiSelect(bool value);
				
				/// <summary>Get indices of all selected items.</summary>
				/// <returns>Indices of all selected items.</returns>
				const collections::SortedList<vint>&			GetSelectedItems();
				/// <summary>Get the index of the selected item.</summary>
				/// <returns>Returns the index of the selected item. If there are multiple selected items, or there is no selected item, -1 will be returned.</returns>
				vint											GetSelectedItemIndex();
				/// <summary>Get the text of the selected item.</summary>
				/// <returns>Returns the text of the selected item. If there are multiple selected items, or there is no selected item, an empty string will be returned.</returns>
				WString											GetSelectedItemText();

				/// <summary>Get the selection status of an item.</summary>
				/// <returns>The selection status of an item.</returns>
				/// <param name="itemIndex">The index of the item.</param>
				bool											GetSelected(vint itemIndex);
				/// <summary>Set the selection status of an item.</summary>
				/// <param name="itemIndex">The index of the item.</param>
				/// <param name="value">Set to true to select the item.</param>
				void											SetSelected(vint itemIndex, bool value);
				/// <summary>Set the selection status of an item, and affect other selected item according to key status.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="itemIndex">The index of the item.</param>
				/// <param name="ctrl">Set to true if the control key is pressing.</param>
				/// <param name="shift">Set to true if the shift key is pressing.</param>
				/// <param name="leftButton">Set to true if clicked by left mouse button, otherwise right mouse button.</param>
				bool											SelectItemsByClick(vint itemIndex, bool ctrl, bool shift, bool leftButton);
				/// <summary>Set the selection status using keys.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="code">The key code that is pressing.</param>
				/// <param name="ctrl">Set to true if the control key is pressing.</param>
				/// <param name="shift">Set to true if the shift key is pressing.</param>
				bool											SelectItemsByKey(VKEY code, bool ctrl, bool shift);
				/// <summary>Unselect all items.</summary>
				void											ClearSelection();
			};

/***********************************************************************
Predefined ItemProvider
***********************************************************************/

			namespace list
			{
				template<typename TBase>
				class PredefinedListItemTemplate : public TBase
				{
				protected:
					GuiListControl*							listControl = nullptr;
					virtual void							OnInitialize() = 0;
					virtual void							OnRefresh() = 0;

					void OnAssociatedListControlChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
					{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::list::PredefinedListItemTemplate<TBase>::OnAssociatedListControlChanged(GuiGraphicsComposition*, GuiEventArgs&)#"
						auto value = this->GetAssociatedListControl();
						CHECK_ERROR(value && (!listControl || listControl == value), ERROR_MESSAGE_PREFIX L"GuiListItemTemplate::SetAssociatedListControl cannot be invoked using a different list control instance.");
						if (!listControl)
						{
							listControl = value;
							OnInitialize();
							OnRefresh();
						}
#undef ERROR_MESSAGE_PREFIX
					}
				public:
					PredefinedListItemTemplate()
					{
						this->AssociatedListControlChanged.AttachMethod(this, &PredefinedListItemTemplate<TBase>::OnAssociatedListControlChanged);
					}

					void RefreshItem()
					{
						OnRefresh();
					}
				};
			}
		}
	}
}

#endif
