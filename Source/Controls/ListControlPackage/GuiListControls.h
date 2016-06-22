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
		namespace controls
		{

/***********************************************************************
List Control
***********************************************************************/

			/// <summary>Represents a list control. A list control automatically read data sources and creates corresponding data item control from the item template.</summary>
			class GuiListControl : public GuiScrollView, public Description<GuiListControl>
			{
			public:
				class IItemProvider;
				class IItemStyleController;
				class IItemStyleProvider;

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
					virtual IItemStyleController*					RequestItem(vint itemIndex)=0;
					/// <summary>Release an item control. This function is suggested to call when an item control gets out of the visible area.</summary>
					/// <param name="style">The item control.</param>
					virtual void									ReleaseItem(IItemStyleController* style)=0;
					/// <summary>Update the view location. The view location is the left-top position in the logic space of the list control.</summary>
					/// <param name="value">The new view location.</param>
					virtual void									SetViewLocation(Point value)=0;
					/// <summary>Get the preferred size of an item control.</summary>
					/// <returns>The preferred size of an item control.</returns>
					/// <param name="style">The item control.</param>
					virtual Size									GetStylePreferredSize(IItemStyleController* style)=0;
					/// <summary>Set the alignment of an item control.</summary>
					/// <param name="style">The item control.</param>
					/// <param name="margin">The new alignment.</param>
					virtual void									SetStyleAlignmentToParent(IItemStyleController* style, Margin margin)=0;
					/// <summary>Get the bounds of an item control.</summary>
					/// <returns>The bounds of an item control.</returns>
					/// <param name="style">The item control.</param>
					virtual Rect									GetStyleBounds(IItemStyleController* style)=0;
					/// <summary>Set the bounds of an item control.</summary>
					/// <param name="style">The item control.</param>
					/// <param name="bounds">The new bounds.</param>
					virtual void									SetStyleBounds(IItemStyleController* style, Rect bounds)=0;
					/// <summary>Get the <see cref="compositions::GuiGraphicsComposition"/> that directly contains item controls.</summary>
					/// <returns>The <see cref="compositions::GuiGraphicsComposition"/> that directly contains item controls.</returns>
					virtual compositions::GuiGraphicsComposition*	GetContainerComposition()=0;
					/// <summary>Notify the list control that the total size of all item controls are changed.</summary>
					virtual void									OnTotalSizeChanged()=0;
				};

				//-----------------------------------------------------------
				// Common Views
				//-----------------------------------------------------------

				/// <summary>Primary text view for <see cref="IItemProvider"/>.</summary>
				class IItemPrimaryTextView : public virtual IDescriptable, public Description<IItemPrimaryTextView>
				{
				public:
					/// <summary>The identifier for this view.</summary>
					static const wchar_t* const					Identifier;

					/// <summary>Get the text of an item.</summary>
					/// <returns>The text of an item.</returns>
					/// <param name="itemIndex">The index of the item.</param>
					virtual WString								GetPrimaryTextViewText(vint itemIndex)=0;
					/// <summary>Test does an item contain a text.</summary>
					/// <returns>Returns true if an item contains a text.</returns>
					/// <param name="itemIndex">The index of the item.</param>
					virtual bool								ContainsPrimaryText(vint itemIndex)=0;
				};

				/// <summary>Binding view for <see cref="IItemProvider"/>.</summary>
				class IItemBindingView : public virtual IDescriptable, public Description<IItemPrimaryTextView>
				{
				public:
					/// <summary>The identifier for this view.</summary>
					static const wchar_t* const					Identifier;

					/// <summary>Get the binding value of an item.</summary>
					/// <returns>The binding value of an item.</returns>
					/// <param name="itemIndex">The index of the item.</param>
					virtual description::Value					GetBindingValue(vint itemIndex)=0;
				};

				//-----------------------------------------------------------
				// Provider Interfaces
				//-----------------------------------------------------------

				/// <summary>Item provider for a <see cref="GuiListControl"/>.</summary>
				class IItemProvider : public virtual IDescriptable, public Description<IItemProvider>
				{
				public:
					/// <summary>Attach an item provider callback to this item provider.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The item provider callback.</param>
					virtual bool								AttachCallback(IItemProviderCallback* value)=0;
					/// <summary>Detach an item provider callback from this item provider.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="value">The item provider callback.</param>
					virtual bool								DetachCallback(IItemProviderCallback* value)=0;
					/// <summary>Get the number of items in this item proivder.</summary>
					/// <returns>The number of items in this item proivder.</returns>
					virtual vint								Count()=0;
					/// <summary>Request a view for this item provider. If the specified view is not supported, it returns null. If you want to get a view of type IXXX, use IXXX::Identifier as the identifier. When the view object is no longer needed, Calling to the [M:vl.presentation.controls.GuiListControl.IItemProvider.ReleaseView] is needed.</summary>
					/// <returns>The view object.</returns>
					/// <param name="identifier">The identifier for the requested view.</param>
					virtual IDescriptable*						RequestView(const WString& identifier)=0;
					/// <summary>Release a requested view.</summary>
					/// <param name="view">The view to release.</param>
					virtual void								ReleaseView(IDescriptable* view)=0;
				};
				
				/// <summary>Item style controller for a <see cref="GuiListControl"/>. Item style controller contains all information to render a binded item.</summary>
				class IItemStyleController : public virtual IDescriptable, public Description<IItemStyleController>
				{
				public:
					/// <summary>Get the owner [T:vl.presentation.controls.GuiListControl.IItemStyleProvider].</summary>
					/// <returns>The owner.</returns>
					virtual IItemStyleProvider*					GetStyleProvider()=0;
					/// <summary>Get the item style id for the binded item.</summary>
					/// <returns>The item style id.</returns>
					virtual vint								GetItemStyleId()=0;
					/// <summary>Get the bounds composition that represnets the binded item.</summary>
					/// <returns>The bounds composition</returns>
					virtual compositions::GuiBoundsComposition*	GetBoundsComposition()=0;
					/// <summary>Test is this item style controller cacheable.</summary>
					/// <returns>Returns true if this item style controller is cacheable.</returns>
					virtual bool								IsCacheable()=0;
					/// <summary>Test is there an item binded to this item style controller.</summary>
					/// <returns>Returns true if an item is binded to this item style controller.</returns>
					virtual bool								IsInstalled()=0;
					/// <summary>Called when an item is binded to this item style controller.</summary>
					virtual void								OnInstalled()=0;
					/// <summary>Called when an item is unbinded to this item style controller.</summary>
					virtual void								OnUninstalled()=0;
				};
				
				/// <summary>Item style provider for a <see cref="GuiListControl"/>. When implementing an item style provider, the provider can require the item provider to support a specified view to access data for each item.</summary>
				class IItemStyleProvider : public virtual IDescriptable, public Description<IItemStyleProvider>
				{
				public:
					/// <summary>Called when an item style provider in installed to a <see cref="GuiListControl"/>.</summary>
					/// <param name="value">The list control.</param>
					virtual void								AttachListControl(GuiListControl* value)=0;
					/// <summary>Called when an item style provider in uninstalled from a <see cref="GuiListControl"/>.</summary>
					virtual void								DetachListControl()=0;
					/// <summary>Get a item style id from an item index.</summary>
					/// <returns>The item style id.</returns>
					/// <param name="itemIndex">The item index.</param>
					virtual vint								GetItemStyleId(vint itemIndex)=0;
					/// <summary>Create an item style controller from an item style id.</summary>
					/// <returns>The created item style controller.</returns>
					/// <param name="styleId">The item style id.</param>
					virtual IItemStyleController*				CreateItemStyle(vint styleId)=0;
					/// <summary>Destroy an item style controller.</summary>
					/// <param name="style">The item style controller.</param>
					virtual void								DestroyItemStyle(IItemStyleController* style)=0;
					/// <summary>Bind an item to an item style controller.</summary>
					/// <param name="style">The item style controller.</param>
					/// <param name="itemIndex">The item index.</param>
					virtual void								Install(IItemStyleController* style, vint itemIndex)=0;
					/// <summary>Update the visual affect of an item style controller to a new item index.</summary>
					/// <param name="style">The item style controller.</param>
					/// <param name="value">The new item index.</param>
					virtual void								SetStyleIndex(IItemStyleController* style, vint value)=0;
				};
				
				/// <summary>Item arranger for a <see cref="GuiListControl"/>. Item arranger decides how to arrange and item controls. When implementing an item arranger, <see cref="IItemArrangerCallback"/> is suggested to use when calculating locations and sizes for item controls.</summary>
				class IItemArranger : public virtual IItemProviderCallback, public Description<IItemArranger>
				{
				public:
					/// <summary>Called when an item arranger in installed to a <see cref="GuiListControl"/>.</summary>
					/// <param name="value">The list control.</param>
					virtual void								AttachListControl(GuiListControl* value)=0;
					/// <summary>Called when an item arranger in uninstalled from a <see cref="GuiListControl"/>.</summary>
					virtual void								DetachListControl()=0;
					/// <summary>Get the binded item arranger callback object.</summary>
					/// <returns>The binded item arranger callback object.</returns>
					virtual IItemArrangerCallback*				GetCallback()=0;
					/// <summary>Bind the item arranger callback object.</summary>
					/// <param name="value">The item arranger callback object to bind.</param>
					virtual void								SetCallback(IItemArrangerCallback* value)=0;
					/// <summary>Get the total size of all data controls.</summary>
					/// <returns>The total size.</returns>
					virtual Size								GetTotalSize()=0;
					/// <summary>Get the item style controller for an visible item index. If an item is not visible, it returns null.</summary>
					/// <returns>The item style controller.</returns>
					/// <param name="itemIndex">The item index.</param>
					virtual IItemStyleController*				GetVisibleStyle(vint itemIndex)=0;
					/// <summary>Get the item index for an visible item style controller. If an item is not visible, it returns -1.</summary>
					/// <returns>The item index.</returns>
					/// <param name="style">The item style controller.</param>
					virtual vint								GetVisibleIndex(IItemStyleController* style)=0;
					/// <summary>Called when the visible area of item container is changed.</summary>
					/// <param name="bounds">The new visible area.</param>
					virtual void								OnViewChanged(Rect bounds)=0;
					/// <summary>Find the item by an base item and a key direction.</summary>
					/// <returns>The item index that is found. Returns -1 if this operation failed.</returns>
					/// <param name="itemIndex">The base item index.</param>
					/// <param name="key">The key direction.</param>
					virtual vint								FindItem(vint itemIndex, compositions::KeyDirection key)=0;
					/// <summary>Adjust the view location to make an item visible.</summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="itemIndex">The item index of the item to be made visible.</param>
					virtual bool								EnsureItemVisible(vint itemIndex)=0;
					/// <summary>Get the adopted size for the view bounds.</summary>
					/// <returns>The adopted size, making the vids bounds just enough to display several items.</returns>
					/// <param name="expectedSize">The expected size, to provide a guidance.</param>
					virtual Size								GetAdoptedSize(Size expectedSize)=0;
				};

			protected:

				//-----------------------------------------------------------
				// ItemCallback
				//-----------------------------------------------------------

				class ItemCallback : public IItemProviderCallback, public IItemArrangerCallback
				{
					typedef collections::List<IItemStyleController*>			StyleList;
				protected:
					GuiListControl*								listControl;
					IItemProvider*								itemProvider;
					StyleList									cachedStyles;
					StyleList									installedStyles;

				public:
					ItemCallback(GuiListControl* _listControl);
					~ItemCallback();

					void										ClearCache();

					void										OnAttached(IItemProvider* provider)override;
					void										OnItemModified(vint start, vint count, vint newCount)override;
					IItemStyleController*						RequestItem(vint itemIndex)override;
					void										ReleaseItem(IItemStyleController* style)override;
					void										SetViewLocation(Point value)override;
					Size										GetStylePreferredSize(IItemStyleController* style)override;
					void										SetStyleAlignmentToParent(IItemStyleController* style, Margin margin)override;
					Rect										GetStyleBounds(IItemStyleController* style)override;
					void										SetStyleBounds(IItemStyleController* style, Rect bounds)override;
					compositions::GuiGraphicsComposition*		GetContainerComposition()override;
					void										OnTotalSizeChanged()override;
				};

				//-----------------------------------------------------------
				// State management
				//-----------------------------------------------------------

				Ptr<ItemCallback>								callback;
				Ptr<IItemProvider>								itemProvider;
				Ptr<IItemStyleProvider>							itemStyleProvider;
				Ptr<IItemArranger>								itemArranger;
				Ptr<compositions::IGuiAxis>						axis;
				Size											fullSize;

				virtual void									OnItemModified(vint start, vint count, vint newCount);
				virtual void									OnStyleInstalled(vint itemIndex, IItemStyleController* style);
				virtual void									OnStyleUninstalled(IItemStyleController* style);
				
				void											OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)override;
				void											OnBeforeReleaseGraphicsHost()override;
				Size											QueryFullSize()override;
				void											UpdateView(Rect viewBounds)override;
				
				void											OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void											SetStyleProviderAndArranger(Ptr<IItemStyleProvider> styleProvider, Ptr<IItemArranger> arranger);

				//-----------------------------------------------------------
				// Item event management
				//-----------------------------------------------------------

				class VisibleStyleHelper
				{
				public:
					Ptr<compositions::GuiMouseEvent::IHandler>		leftButtonDownHandler;
					Ptr<compositions::GuiMouseEvent::IHandler>		leftButtonUpHandler;
					Ptr<compositions::GuiMouseEvent::IHandler>		leftButtonDoubleClickHandler;
					Ptr<compositions::GuiMouseEvent::IHandler>		middleButtonDownHandler;
					Ptr<compositions::GuiMouseEvent::IHandler>		middleButtonUpHandler;
					Ptr<compositions::GuiMouseEvent::IHandler>		middleButtonDoubleClickHandler;
					Ptr<compositions::GuiMouseEvent::IHandler>		rightButtonDownHandler;
					Ptr<compositions::GuiMouseEvent::IHandler>		rightButtonUpHandler;
					Ptr<compositions::GuiMouseEvent::IHandler>		rightButtonDoubleClickHandler;
					Ptr<compositions::GuiMouseEvent::IHandler>		mouseMoveHandler;
					Ptr<compositions::GuiNotifyEvent::IHandler>		mouseEnterHandler;
					Ptr<compositions::GuiNotifyEvent::IHandler>		mouseLeaveHandler;
				};
				
				friend class collections::ArrayBase<Ptr<VisibleStyleHelper>>;
				collections::Dictionary<IItemStyleController*, Ptr<VisibleStyleHelper>>		visibleStyles;

				void											OnItemMouseEvent(compositions::GuiItemMouseEvent& itemEvent, IItemStyleController* style, compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void											OnItemNotifyEvent(compositions::GuiItemNotifyEvent& itemEvent, IItemStyleController* style, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void											AttachItemEvents(IItemStyleController* style);
				void											DetachItemEvents(IItemStyleController* style);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="_styleProvider">The style provider.</param>
				/// <param name="_itemProvider">The item provider as a data source.</param>
				/// <param name="acceptFocus">Set to true if the list control is allowed to have a keyboard focus.</param>
				GuiListControl(IStyleProvider* _styleProvider, IItemProvider* _itemProvider, bool acceptFocus=false);
				~GuiListControl();

				/// <summary>Style provider changed event.</summary>
				compositions::GuiNotifyEvent					StyleProviderChanged;
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
				virtual IItemStyleProvider*						GetStyleProvider();
				/// <summary>Set the item style provider</summary>
				/// <returns>The old item style provider</returns>
				/// <param name="value">The new item style provider</param>
				virtual Ptr<IItemStyleProvider>					SetStyleProvider(Ptr<IItemStyleProvider> value);
				/// <summary>Get the item arranger.</summary>
				/// <returns>The item arranger.</returns>
				virtual IItemArranger*							GetArranger();
				/// <summary>Set the item arranger</summary>
				/// <returns>The old item arranger</returns>
				/// <param name="value">The new item arranger</param>
				virtual Ptr<IItemArranger>						SetArranger(Ptr<IItemArranger> value);
				/// <summary>Get the item coordinate transformer.</summary>
				/// <returns>The item coordinate transformer.</returns>
				virtual compositions::IGuiAxis*					GetAxis();
				/// <summary>Set the item coordinate transformer</summary>
				/// <returns>The old item coordinate transformer</returns>
				/// <param name="value">The new item coordinate transformer</param>
				virtual Ptr<compositions::IGuiAxis>				SetAxis(Ptr<compositions::IGuiAxis> value);
				/// <summary>Adjust the view location to make an item visible.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="itemIndex">The item index of the item to be made visible.</param>
				virtual bool									EnsureItemVisible(vint itemIndex);
				/// <summary>Get the adopted size for the list control.</summary>
				/// <returns>The adopted size, making the list control just enough to display several items.</returns>
				/// <param name="expectedSize">The expected size, to provide a guidance.</param>
				virtual Size									GetAdoptedSize(Size expectedSize);
			};

/***********************************************************************
Selectable List Control
***********************************************************************/

			/// <summary>Represents a list control that each item is selectable.</summary>
			class GuiSelectableListControl : public GuiListControl, public Description<GuiSelectableListControl>
			{
			public:
				/// <summary>Item style provider for <see cref="GuiSelectableListControl"/>.</summary>
				class IItemStyleProvider : public virtual GuiListControl::IItemStyleProvider, public Description<IItemStyleProvider>
				{
				public:
					/// <summary>Change the visual affect of an item style controller to be selected or unselected.</summary>
					/// <param name="style">The item style controller.</param>
					/// <param name="value">Set to true if the item is expected to be rendered as selected.</param>
					virtual void								SetStyleSelected(IItemStyleController* style, bool value)=0;
				};
			protected:

				Ptr<IItemStyleProvider>							selectableStyleProvider;
				collections::SortedList<vint>					selectedItems;
				bool											multiSelect;
				vint											selectedItemIndexStart;
				vint											selectedItemIndexEnd;

				void											OnItemModified(vint start, vint count, vint newCount)override;
				void											OnStyleInstalled(vint itemIndex, IItemStyleController* style)override;
				void											OnStyleUninstalled(IItemStyleController* style)override;
				virtual void									OnItemSelectionChanged(vint itemIndex, bool value);
				virtual void									OnItemSelectionCleared();
				void											OnItemLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments);
				void											OnItemRightButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments);

				void											NormalizeSelectedItemIndexStartEnd();
				void											SetMultipleItemsSelectedSilently(vint start, vint end, bool selected);
				void											OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="_styleProvider">The style provider.</param>
				/// <param name="_itemProvider">The item provider as a data source.</param>
				GuiSelectableListControl(IStyleProvider* _styleProvider, IItemProvider* _itemProvider);
				~GuiSelectableListControl();

				/// <summary>Selection changed event.</summary>
				compositions::GuiNotifyEvent					SelectionChanged;

				Ptr<GuiListControl::IItemStyleProvider>			SetStyleProvider(Ptr<GuiListControl::IItemStyleProvider> value)override;

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
				/// <returns>Returns the text of the selected item. If there are multiple selected items, or there is no selected item, or <see cref="GuiListControl::IItemPrimaryTextView"/> is not a valid view for the item provider, an empty string will be returned.</returns>
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
				bool											SelectItemsByKey(vint code, bool ctrl, bool shift);
				/// <summary>Unselect all items.</summary>
				void											ClearSelection();
			};

/***********************************************************************
Predefined ItemArranger
***********************************************************************/

			namespace list
			{
				/// <summary>Ranged item arranger. This arranger implements most of the common functionality for those arrangers that display a continuing subset of item at a time.</summary>
				class RangedItemArrangerBase : public Object, virtual public GuiListControl::IItemArranger, public Description<RangedItemArrangerBase>
				{
					typedef collections::List<GuiListControl::IItemStyleController*>		StyleList;
				protected:
					GuiListControl*								listControl;
					GuiListControl::IItemArrangerCallback*		callback;
					GuiListControl::IItemProvider*				itemProvider;
					Rect										viewBounds;
					vint										startIndex;
					StyleList									visibleStyles;

					void										InvalidateAdoptedSize();
					vint										CalculateAdoptedSize(vint expectedSize, vint count, vint itemSize);

					virtual void								ClearStyles();
					virtual void								OnStylesCleared()=0;
					virtual Size								OnCalculateTotalSize()=0;
					virtual void								OnViewChangedInternal(Rect oldBounds, Rect newBounds)=0;
				public:
					/// <summary>Create the arranger.</summary>
					RangedItemArrangerBase();
					~RangedItemArrangerBase();

					void										OnAttached(GuiListControl::IItemProvider* provider)override;
					void										OnItemModified(vint start, vint count, vint newCount)override;
					void										AttachListControl(GuiListControl* value)override;
					void										DetachListControl()override;
					GuiListControl::IItemArrangerCallback*		GetCallback()override;
					void										SetCallback(GuiListControl::IItemArrangerCallback* value)override;
					Size										GetTotalSize()override;
					GuiListControl::IItemStyleController*		GetVisibleStyle(vint itemIndex)override;
					vint										GetVisibleIndex(GuiListControl::IItemStyleController* style)override;
					void										OnViewChanged(Rect bounds)override;
				};
				
				/// <summary>Fixed height item arranger. This arranger lists all item with the same height value. This value is the maximum height of all minimum heights of displayed items.</summary>
				class FixedHeightItemArranger : public RangedItemArrangerBase, public Description<FixedHeightItemArranger>
				{
				protected:
					vint										rowHeight;
					bool										suppressOnViewChanged;

					virtual void								RearrangeItemBounds();
					virtual vint								GetWidth();
					virtual vint								GetYOffset();
					void										OnStylesCleared()override;
					Size										OnCalculateTotalSize()override;
					void										OnViewChangedInternal(Rect oldBounds, Rect newBounds)override;
				public:
					/// <summary>Create the arranger.</summary>
					FixedHeightItemArranger();
					~FixedHeightItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					bool										EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;
				};

				/// <summary>Fixed size multiple columns item arranger. This arranger adjust all items in multiple lines with the same size. The width is the maximum width of all minimum widths of displayed items. The same to height.</summary>
				class FixedSizeMultiColumnItemArranger : public RangedItemArrangerBase, public Description<FixedSizeMultiColumnItemArranger>
				{
				protected:
					Size										itemSize;
					bool										suppressOnViewChanged;

					virtual void								RearrangeItemBounds();
					void										CalculateRange(Size itemSize, Rect bounds, vint count, vint& start, vint& end);
					void										OnStylesCleared()override;
					Size										OnCalculateTotalSize()override;
					void										OnViewChangedInternal(Rect oldBounds, Rect newBounds)override;
				public:
					/// <summary>Create the arranger.</summary>
					FixedSizeMultiColumnItemArranger();
					~FixedSizeMultiColumnItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					bool										EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;
				};
				
				/// <summary>Fixed size multiple columns item arranger. This arranger adjust all items in multiple columns with the same height. The height is the maximum width of all minimum height of displayed items. Each item will displayed using its minimum width.</summary>
				class FixedHeightMultiColumnItemArranger : public RangedItemArrangerBase, public Description<FixedHeightMultiColumnItemArranger>
				{
				protected:
					vint										itemHeight;
					bool										suppressOnViewChanged;

					virtual void								RearrangeItemBounds();
					void										CalculateRange(vint itemHeight, Rect bounds, vint& rows, vint& startColumn);
					void										OnStylesCleared()override;
					Size										OnCalculateTotalSize()override;
					void										OnViewChangedInternal(Rect oldBounds, Rect newBounds)override;
				public:
					/// <summary>Create the arranger.</summary>
					FixedHeightMultiColumnItemArranger();
					~FixedHeightMultiColumnItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					bool										EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;
				};
			}

/***********************************************************************
Predefined ItemStyleController
***********************************************************************/

			namespace list
			{
				/// <summary>Item style controller base. This class provides common functionalities item style controllers.</summary>
				class ItemStyleControllerBase : public Object, public virtual GuiListControl::IItemStyleController, public Description<ItemStyleControllerBase>
				{
				protected:
					GuiListControl::IItemStyleProvider*			provider;
					vint										styleId;
					compositions::GuiBoundsComposition*			boundsComposition;
					GuiControl*									associatedControl;
					bool										isInstalled;

					void										Initialize(compositions::GuiBoundsComposition* _boundsComposition, GuiControl* _associatedControl);
					void										Finalize();

					/// <summary>Create the item style controller with a specified item style provider and a specified item style id.</summary>
					/// <param name="_provider">The specified item style provider.</param>
					/// <param name="_styleId">The specified item style id.</param>
					ItemStyleControllerBase(GuiListControl::IItemStyleProvider* _provider, vint _styleId);
				public:
					~ItemStyleControllerBase();
					
					GuiListControl::IItemStyleProvider*			GetStyleProvider()override;
					vint										GetItemStyleId()override;
					compositions::GuiBoundsComposition*			GetBoundsComposition()override;
					bool										IsCacheable()override;
					bool										IsInstalled()override;
					void										OnInstalled()override;
					void										OnUninstalled()override;
				};
			}

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

					virtual void								InvokeOnItemModified(vint start, vint count, vint newCount);
				public:
					/// <summary>Create the item provider.</summary>
					ItemProviderBase();
					~ItemProviderBase();

					bool										AttachCallback(GuiListControl::IItemProviderCallback* value);
					bool										DetachCallback(GuiListControl::IItemProviderCallback* value);
				};

				template<typename T>
				class ListProvider : public ItemProviderBase, public ItemsBase<T>
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
