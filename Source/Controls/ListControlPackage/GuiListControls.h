/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUILISTCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUILISTCONTROLS

#include "../GuiContainerControls.h"

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
				class IItemProvider;

				using ItemStyle = templates::GuiListItemTemplate;
				using ItemStyleProperty = TemplateProperty<templates::GuiListItemTemplate>;

				//-----------------------------------------------------------
				// Callback Interfaces
				//-----------------------------------------------------------

				/// <summary>Item provider callback. Item providers use this interface to notify item modification.</summary>
				class IItemProviderCallback : public virtual IDescriptable, public Description<IItemProviderCallback>
				{
				public:
					/// <summary>Called when an item provider callback object is attached to an item provider.</summary>
					/// <param name="provider">The item provider.</param>
					virtual void								OnAttached(IItemProvider* provider)=0;
					/// <summary>Called when items in the item provider is modified.</summary>
					/// <param name="start">The index of the first modified item.</param>
					/// <param name="count">The number of all modified items.</param>
					/// <param name="newCount">The number of new items. If items are inserted or removed, newCount may not equals to count.</param>
					virtual void								OnItemModified(vint start, vint count, vint newCount)=0;
				};

				/// <summary>Item arranger callback. Item arrangers use this interface to communicate with the list control. When setting positions for item controls, functions in this callback object is suggested to call because they use the result from the [T:vl.presentation.controls.compositions.IGuiAxis].</summary>
				class IItemArrangerCallback : public virtual IDescriptable, public Description<IItemArrangerCallback>
				{
				public:
					/// <summary>Request an item control representing an item in the item provider. This function is suggested to call when an item control gets into the visible area.</summary>
					/// <returns>The item control.</returns>
					/// <param name="itemIndex">The index of the item in the item provider.</param>
					/// <param name="itemComposition">The composition that represents the item. Set to null if the item style is expected to be put directly into the list control.</param>
					virtual ItemStyle*								RequestItem(vint itemIndex, compositions::GuiBoundsComposition* itemComposition)=0;
					/// <summary>Release an item control. This function is suggested to call when an item control gets out of the visible area.</summary>
					/// <param name="style">The item control.</param>
					virtual void									ReleaseItem(ItemStyle* style)=0;
					/// <summary>Update the view location. The view location is the left-top position in the logic space of the list control.</summary>
					/// <param name="value">The new view location.</param>
					virtual void									SetViewLocation(Point value)=0;
					/// <summary>Get the preferred size of an item control.</summary>
					/// <returns>The preferred size of an item control.</returns>
					/// <param name="style">The item control.</param>
					virtual Size									GetStylePreferredSize(compositions::GuiBoundsComposition* style)=0;
					/// <summary>Set the alignment of an item control.</summary>
					/// <param name="style">The item control.</param>
					/// <param name="margin">The new alignment.</param>
					virtual void									SetStyleAlignmentToParent(compositions::GuiBoundsComposition* style, Margin margin)=0;
					/// <summary>Get the bounds of an item control.</summary>
					/// <returns>The bounds of an item control.</returns>
					/// <param name="style">The item control.</param>
					virtual Rect									GetStyleBounds(compositions::GuiBoundsComposition* style)=0;
					/// <summary>Set the bounds of an item control.</summary>
					/// <param name="style">The item control.</param>
					/// <param name="bounds">The new bounds.</param>
					virtual void									SetStyleBounds(compositions::GuiBoundsComposition* style, Rect bounds)=0;
					/// <summary>Get the <see cref="compositions::GuiGraphicsComposition"/> that directly contains item controls.</summary>
					/// <returns>The <see cref="compositions::GuiGraphicsComposition"/> that directly contains item controls.</returns>
					virtual compositions::GuiGraphicsComposition*	GetContainerComposition()=0;
					/// <summary>Notify the list control that the total size of all item controls are changed.</summary>
					virtual void									OnTotalSizeChanged()=0;
				};

				//-----------------------------------------------------------
				// Data Source Interfaces
				//-----------------------------------------------------------

				/// <summary>Item provider for a <see cref="GuiListControl"/>.</summary>
				class IItemProvider : public virtual IDescriptable, public Description<IItemProvider>
				{
				public:
					/// <summary>Attach an item provider callback to this item provider.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The item provider callback.</param>
					virtual bool								AttachCallback(IItemProviderCallback* value) = 0;
					/// <summary>Detach an item provider callback from this item provider.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The item provider callback.</param>
					virtual bool								DetachCallback(IItemProviderCallback* value) = 0;
					/// <summary>Increase the editing counter indicating that an [T:vl.presentation.templates.GuiListItemTemplate] is editing an item.</summary>
					virtual void								PushEditing() = 0;
					/// <summary>Decrease the editing counter indicating that an [T:vl.presentation.templates.GuiListItemTemplate] has stopped editing an item.</summary>
					/// <returns>Returns false if there is no supression before calling this function.</returns>
					virtual bool								PopEditing() = 0;
					/// <summary>Test if an [T:vl.presentation.templates.GuiListItemTemplate] is editing an item.</summary>
					/// <returns>Returns false if there is no editing.</returns>
					virtual bool								IsEditing() = 0;
					/// <summary>Get the number of items in this item proivder.</summary>
					/// <returns>The number of items in this item proivder.</returns>
					virtual vint								Count() = 0;

					/// <summary>Get the text representation of an item.</summary>
					/// <returns>The text representation of an item.</returns>
					/// <param name="itemIndex">The index of the item.</param>
					virtual WString								GetTextValue(vint itemIndex) = 0;
					/// <summary>Get the binding value of an item.</summary>
					/// <returns>The binding value of an item.</returns>
					/// <param name="itemIndex">The index of the item.</param>
					virtual description::Value					GetBindingValue(vint itemIndex) = 0;

					/// <summary>Request a view for this item provider. If the specified view is not supported, it returns null. If you want to get a view of type IXXX, use IXXX::Identifier as the identifier.</summary>
					/// <returns>The view object.</returns>
					/// <param name="identifier">The identifier for the requested view.</param>
					virtual IDescriptable*						RequestView(const WString& identifier) = 0;
				};

				//-----------------------------------------------------------
				// Item Layout Interfaces
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
				class IItemArranger : public virtual IItemProviderCallback, public Description<IItemArranger>
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
					virtual vint								FindItem(vint itemIndex, compositions::KeyDirection key) = 0;
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

				class ItemCallback : public IItemProviderCallback, public IItemArrangerCallback
				{
					typedef compositions::IGuiGraphicsEventHandler							BoundsChangedHandler;
					typedef collections::List<ItemStyle*>									StyleList;
					typedef collections::Dictionary<ItemStyle*, Ptr<BoundsChangedHandler>>	InstalledStyleMap;
				protected:
					GuiListControl*								listControl = nullptr;
					IItemProvider*								itemProvider = nullptr;
					InstalledStyleMap							installedStyles;

					Ptr<BoundsChangedHandler>					InstallStyle(ItemStyle* style, vint itemIndex, compositions::GuiBoundsComposition* itemComposition);
					ItemStyle*									UninstallStyle(vint index);
					void										OnStyleBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					ItemCallback(GuiListControl* _listControl);
					~ItemCallback();

					void										ClearCache();

					void										OnAttached(IItemProvider* provider)override;
					void										OnItemModified(vint start, vint count, vint newCount)override;
					ItemStyle*									RequestItem(vint itemIndex, compositions::GuiBoundsComposition* itemComposition)override;
					void										ReleaseItem(ItemStyle* style)override;
					void										SetViewLocation(Point value)override;
					Size										GetStylePreferredSize(compositions::GuiBoundsComposition* style)override;
					void										SetStyleAlignmentToParent(compositions::GuiBoundsComposition* style, Margin margin)override;
					Rect										GetStyleBounds(compositions::GuiBoundsComposition* style)override;
					void										SetStyleBounds(compositions::GuiBoundsComposition* style, Rect bounds)override;
					compositions::GuiGraphicsComposition*		GetContainerComposition()override;
					void										OnTotalSizeChanged()override;
				};

				//-----------------------------------------------------------
				// State management
				//-----------------------------------------------------------

				Ptr<ItemCallback>								callback;
				Ptr<IItemProvider>								itemProvider;
				ItemStyleProperty								itemStyleProperty;
				Ptr<IItemArranger>								itemArranger;
				Ptr<compositions::IGuiAxis>						axis;
				Size											fullSize;
				bool											displayItemBackground = true;

				virtual void									OnItemModified(vint start, vint count, vint newCount);
				virtual void									OnStyleInstalled(vint itemIndex, ItemStyle* style);
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
				void											OnClientBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
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
				GuiListControl(theme::ThemeName themeName, IItemProvider* _itemProvider, bool acceptFocus=false);
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
				virtual IItemProvider*							GetItemProvider();
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

				void											NotifySelectionChanged();
				void											OnItemModified(vint start, vint count, vint newCount)override;
				void											OnStyleInstalled(vint itemIndex, ItemStyle* style)override;
				virtual void									OnItemSelectionChanged(vint itemIndex, bool value);
				virtual void									OnItemSelectionCleared();
				void											OnItemLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments);
				void											OnItemRightButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments);

				void											NormalizeSelectedItemIndexStartEnd();
				void											SetMultipleItemsSelectedSilently(vint start, vint end, bool selected);
				void											OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_itemProvider">The item provider as a data source.</param>
				GuiSelectableListControl(theme::ThemeName themeName, IItemProvider* _itemProvider);
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
				/// <summary>Item provider base. This class provider common functionalities for item providers.</summary>
				class ItemProviderBase : public Object, public virtual GuiListControl::IItemProvider, public Description<ItemProviderBase>
				{
				protected:
					collections::List<GuiListControl::IItemProviderCallback*>	callbacks;
					vint														editingCounter = 0;
					bool														callingOnItemModified = false;

					virtual void								InvokeOnItemModified(vint start, vint count, vint newCount);
				public:
					/// <summary>Create the item provider.</summary>
					ItemProviderBase();
					~ItemProviderBase();

					bool										AttachCallback(GuiListControl::IItemProviderCallback* value)override;
					bool										DetachCallback(GuiListControl::IItemProviderCallback* value)override;
					void										PushEditing()override;
					bool										PopEditing()override;
					bool										IsEditing()override;
				};

				template<typename T>
				class ListProvider : public ItemProviderBase, public collections::ObservableListBase<T>
				{
				protected:
					void NotifyUpdateInternal(vint start, vint count, vint newCount)override
					{
						InvokeOnItemModified(start, count, newCount);
					}
				public:
					vint Count()override
					{
						return this->items.Count();
					}
				};
			}
		}
	}
}

#endif
