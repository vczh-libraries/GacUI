#include "GuiListControls.h"
#include "../Templates/GuiControlTemplates.h"
#include "../../GraphicsHost/GuiGraphicsHost.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;

/***********************************************************************
GuiListControl::ItemCallback
***********************************************************************/

			Ptr<GuiListControl::ItemCallback::BoundsChangedHandler> GuiListControl::ItemCallback::InstallStyle(ItemStyle* style, vint itemIndex, compositions::GuiBoundsComposition* itemComposition)
			{
				auto handler = style->BoundsChanged.AttachMethod(this, &ItemCallback::OnStyleBoundsChanged);
				listControl->GetContainerComposition()->AddChild(itemComposition ? itemComposition : style);
				listControl->OnStyleInstalled(itemIndex, style);
				return handler;
			}

			GuiListControl::ItemStyle* GuiListControl::ItemCallback::UninstallStyle(vint index)
			{
				auto style = installedStyles.Keys()[index];
				auto handler = installedStyles.Values()[index];
				listControl->OnStyleUninstalled(style);
				listControl->GetContainerComposition()->RemoveChild(style);
				style->BoundsChanged.Detach(handler);
				return style;
			}

			void GuiListControl::ItemCallback::OnStyleBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				listControl->InvokeOrDelayIfRendering([=]()
				{
					listControl->CalculateView();
				});
			}

			GuiListControl::ItemCallback::ItemCallback(GuiListControl* _listControl)
				:listControl(_listControl)
			{
			}

			GuiListControl::ItemCallback::~ItemCallback()
			{
				ClearCache();
			}

			void GuiListControl::ItemCallback::ClearCache()
			{
				for (vint i = 0; i < installedStyles.Count(); i++)
				{
					auto style = UninstallStyle(i);
					SafeDeleteComposition(style);
				}
				installedStyles.Clear();
			}

			void GuiListControl::ItemCallback::OnAttached(IItemProvider* provider)
			{
				itemProvider = provider;
			}

			void GuiListControl::ItemCallback::OnItemModified(vint start, vint count, vint newCount)
			{
				listControl->OnItemModified(start, count, newCount);
			}

			GuiListControl::ItemStyle* GuiListControl::ItemCallback::RequestItem(vint itemIndex, compositions::GuiBoundsComposition* itemComposition)
			{
				CHECK_ERROR(0 <= itemIndex && itemIndex < itemProvider->Count(), L"GuiListControl::ItemCallback::RequestItem(vint)#Index out of range.");
				CHECK_ERROR(listControl->itemStyleProperty, L"GuiListControl::ItemCallback::RequestItem(vint)#SetItemTemplate function should be called before adding items to the list control.");

				auto style = listControl->itemStyleProperty(itemProvider->GetBindingValue(itemIndex));
				auto handler = InstallStyle(style, itemIndex, itemComposition);
				installedStyles.Add(style, handler);
				return style;
			}

			void GuiListControl::ItemCallback::ReleaseItem(ItemStyle* style)
			{
				vint index = installedStyles.Keys().IndexOf(style);
				if (index != -1)
				{
					auto style = UninstallStyle(index);
					installedStyles.Remove(style);
					SafeDeleteComposition(style);
				}
			}

			void GuiListControl::ItemCallback::SetViewLocation(Point value)
			{
				Rect virtualRect(value, listControl->GetViewSize());
				Rect realRect = listControl->axis->VirtualRectToRealRect(listControl->fullSize, virtualRect);
				listControl->SetViewPosition(realRect.LeftTop());
			}

			Size GuiListControl::ItemCallback::GetStylePreferredSize(compositions::GuiBoundsComposition* style)
			{
				Size size = style->GetPreferredBounds().GetSize();
				return listControl->axis->RealSizeToVirtualSize(size);
			}

			void GuiListControl::ItemCallback::SetStyleAlignmentToParent(compositions::GuiBoundsComposition* style, Margin margin)
			{
				Margin newMargin = listControl->axis->VirtualMarginToRealMargin(margin);
				style->SetAlignmentToParent(newMargin);
			}

			Rect GuiListControl::ItemCallback::GetStyleBounds(compositions::GuiBoundsComposition* style)
			{
				Rect bounds = style->GetBounds();
				return listControl->axis->RealRectToVirtualRect(listControl->GetViewSize(), bounds);
			}

			void GuiListControl::ItemCallback::SetStyleBounds(compositions::GuiBoundsComposition* style, Rect bounds)
			{
				Rect newBounds = listControl->axis->VirtualRectToRealRect(listControl->GetViewSize(), bounds);
				return style->SetBounds(newBounds);
			}

			compositions::GuiGraphicsComposition* GuiListControl::ItemCallback::GetContainerComposition()
			{
				return listControl->GetContainerComposition();
			}

			void GuiListControl::ItemCallback::OnTotalSizeChanged()
			{
				listControl->CalculateView();
			}

/***********************************************************************
GuiListControl
***********************************************************************/

			void GuiListControl::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiListControl::AfterControlTemplateInstalled_(bool initialize)
			{
				if (itemArranger)
				{
					itemArranger->ReloadVisibleStyles();
					CalculateView();
				}
			}

			void GuiListControl::OnItemModified(vint start, vint count, vint newCount)
			{
			}

			void GuiListControl::OnStyleInstalled(vint itemIndex, ItemStyle* style)
			{
				style->SetFont(GetDisplayFont());
				style->SetContext(GetContext());
				style->SetText(itemProvider->GetTextValue(itemIndex));
				style->SetVisuallyEnabled(GetVisuallyEnabled());
				style->SetSelected(false);
				style->SetIndex(itemIndex);
				style->Initialize(this);
				AttachItemEvents(style);
			}

			void GuiListControl::OnStyleUninstalled(ItemStyle* style)
			{
				DetachItemEvents(style);
			}

			void GuiListControl::OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)
			{
				SetStyleAndArranger(itemStyleProperty, itemArranger);
				GuiScrollView::OnRenderTargetChanged(renderTarget);
			}

			void GuiListControl::OnBeforeReleaseGraphicsHost()
			{
				GuiScrollView::OnBeforeReleaseGraphicsHost();
				SetStyleAndArranger({}, nullptr);
			}

			Size GuiListControl::QueryFullSize()
			{
				Size virtualSize = itemArranger ? itemArranger->GetTotalSize() : Size(0, 0);
				fullSize = axis->VirtualSizeToRealSize(virtualSize);
				return fullSize;
			}

			void GuiListControl::UpdateView(Rect viewBounds)
			{
				if (itemArranger)
				{
					Rect newBounds = axis->RealRectToVirtualRect(fullSize, viewBounds);
					itemArranger->OnViewChanged(newBounds);
				}
			}

			void GuiListControl::OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					SetFocus();
				}
			}

			void GuiListControl::SetStyleAndArranger(ItemStyleProperty styleProperty, Ptr<IItemArranger> arranger)
			{
				if (itemArranger)
				{
					itemArranger->DetachListControl();
					itemArranger->SetCallback(nullptr);
					itemProvider->DetachCallback(itemArranger.Obj());
				}
				callback->ClearCache();

				itemStyleProperty = styleProperty;
				itemArranger = arranger;

				if (auto scroll = GetVerticalScroll())
				{
					scroll->SetPosition(0);
				}
				if (auto scroll = GetHorizontalScroll())
				{
					scroll->SetPosition(0);
				}

				if (itemArranger)
				{
					itemProvider->AttachCallback(itemArranger.Obj());
					itemArranger->SetCallback(callback.Obj());
					itemArranger->AttachListControl(this);
				}
				CalculateView();
			}

			void GuiListControl::UpdateDisplayFont()
			{
				GuiControl::UpdateDisplayFont();
				FOREACH(ItemStyle*, style, visibleStyles.Keys())
				{
					style->SetFont(GetDisplayFont());
				}
			}

			void GuiListControl::OnClientBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				auto args = GetNotifyEventArguments();
				AdoptedSizeInvalidated.Execute(args);
			}

			void GuiListControl::OnVisuallyEnabledChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				FOREACH(ItemStyle*, style, visibleStyles.Keys())
				{
					style->SetVisuallyEnabled(GetVisuallyEnabled());
				}
			}

			void GuiListControl::OnContextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				FOREACH(ItemStyle*, style, visibleStyles.Keys())
				{
					style->SetContext(GetContext());
				}
			}

			void GuiListControl::OnItemMouseEvent(compositions::GuiItemMouseEvent& itemEvent, ItemStyle* style, compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if (itemArranger && GetVisuallyEnabled())
				{
					vint itemIndex = itemArranger->GetVisibleIndex(style);
					if (itemIndex != -1)
					{
						GuiItemMouseEventArgs redirectArguments;
						(GuiMouseEventArgs&)redirectArguments = arguments;
						redirectArguments.compositionSource = boundsComposition;
						redirectArguments.eventSource = boundsComposition;
						redirectArguments.itemIndex = itemIndex;
						itemEvent.Execute(redirectArguments);
						arguments = redirectArguments;
					}
				}
			}

			void GuiListControl::OnItemNotifyEvent(compositions::GuiItemNotifyEvent& itemEvent, ItemStyle* style, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (itemArranger && GetVisuallyEnabled())
				{
					vint itemIndex = itemArranger->GetVisibleIndex(style);
					if (itemIndex != -1)
					{
						GuiItemEventArgs redirectArguments;
						(GuiEventArgs&)redirectArguments = arguments;
						redirectArguments.compositionSource = boundsComposition;
						redirectArguments.eventSource = boundsComposition;
						redirectArguments.itemIndex = itemIndex;
						itemEvent.Execute(redirectArguments);
						arguments = redirectArguments;
					}
				}
			}

#define ATTACH_ITEM_MOUSE_EVENT(EVENTNAME, ITEMEVENTNAME)\
					{\
						Func<void(GuiItemMouseEvent&, ItemStyle*, GuiGraphicsComposition*, GuiMouseEventArgs&)> func(this, &GuiListControl::OnItemMouseEvent);\
						helper->EVENTNAME##Handler = style->GetEventReceiver()->EVENTNAME.AttachFunction(\
							Curry(Curry(func)(ITEMEVENTNAME))(style)\
							);\
					}\

#define ATTACH_ITEM_NOTIFY_EVENT(EVENTNAME, ITEMEVENTNAME)\
					{\
						Func<void(GuiItemNotifyEvent&, ItemStyle*, GuiGraphicsComposition*, GuiEventArgs&)> func(this, &GuiListControl::OnItemNotifyEvent);\
						helper->EVENTNAME##Handler = style->GetEventReceiver()->EVENTNAME.AttachFunction(\
							Curry(Curry(func)(ITEMEVENTNAME))(style)\
							);\
					}\

			void GuiListControl::AttachItemEvents(ItemStyle* style)
			{
				vint index=visibleStyles.Keys().IndexOf(style);
				if(index==-1)
				{
					Ptr<VisibleStyleHelper> helper=new VisibleStyleHelper;
					visibleStyles.Add(style, helper);

					ATTACH_ITEM_MOUSE_EVENT(leftButtonDown, ItemLeftButtonDown);
					ATTACH_ITEM_MOUSE_EVENT(leftButtonUp, ItemLeftButtonUp);
					ATTACH_ITEM_MOUSE_EVENT(leftButtonDoubleClick, ItemLeftButtonDoubleClick);
					ATTACH_ITEM_MOUSE_EVENT(middleButtonDown, ItemMiddleButtonDown);
					ATTACH_ITEM_MOUSE_EVENT(middleButtonUp, ItemMiddleButtonUp);
					ATTACH_ITEM_MOUSE_EVENT(middleButtonDoubleClick, ItemMiddleButtonDoubleClick);
					ATTACH_ITEM_MOUSE_EVENT(rightButtonDown, ItemRightButtonDown);
					ATTACH_ITEM_MOUSE_EVENT(rightButtonUp, ItemRightButtonUp);
					ATTACH_ITEM_MOUSE_EVENT(rightButtonDoubleClick, ItemRightButtonDoubleClick);
					ATTACH_ITEM_MOUSE_EVENT(mouseMove, ItemMouseMove);
					ATTACH_ITEM_NOTIFY_EVENT(mouseEnter, ItemMouseEnter);
					ATTACH_ITEM_NOTIFY_EVENT(mouseLeave, ItemMouseLeave);
				}
			}

#undef ATTACH_ITEM_MOUSE_EVENT
#undef ATTACH_ITEM_NOTIFY_EVENT

#define DETACH_ITEM_EVENT(EVENTNAME) style->GetEventReceiver()->EVENTNAME.Detach(helper->EVENTNAME##Handler)

			void GuiListControl::DetachItemEvents(ItemStyle* style)
			{
				vint index=visibleStyles.Keys().IndexOf(style);
				if(index!=-1)
				{
					Ptr<VisibleStyleHelper> helper=visibleStyles.Values().Get(index);
					visibleStyles.Remove(style);
					
					DETACH_ITEM_EVENT(leftButtonDown);
					DETACH_ITEM_EVENT(leftButtonUp);
					DETACH_ITEM_EVENT(leftButtonDoubleClick);
					DETACH_ITEM_EVENT(middleButtonDown);
					DETACH_ITEM_EVENT(middleButtonUp);
					DETACH_ITEM_EVENT(middleButtonDoubleClick);
					DETACH_ITEM_EVENT(rightButtonDown);
					DETACH_ITEM_EVENT(rightButtonUp);
					DETACH_ITEM_EVENT(rightButtonDoubleClick);
					DETACH_ITEM_EVENT(mouseMove);
					DETACH_ITEM_EVENT(mouseEnter);
					DETACH_ITEM_EVENT(mouseLeave);
				}
			}

#undef DETACH_ITEM_EVENT

			GuiListControl::GuiListControl(theme::ThemeName themeName, IItemProvider* _itemProvider, bool acceptFocus)
				:GuiScrollView(themeName)
				, itemProvider(_itemProvider)
			{
				ContextChanged.AttachMethod(this, &GuiListControl::OnContextChanged);
				VisuallyEnabledChanged.AttachMethod(this, &GuiListControl::OnVisuallyEnabledChanged);
				containerComposition->BoundsChanged.AttachMethod(this, &GuiListControl::OnClientBoundsChanged);

				ItemTemplateChanged.SetAssociatedComposition(boundsComposition);
				ArrangerChanged.SetAssociatedComposition(boundsComposition);
				AxisChanged.SetAssociatedComposition(boundsComposition);
				AdoptedSizeInvalidated.SetAssociatedComposition(boundsComposition);

				ItemLeftButtonDown.SetAssociatedComposition(boundsComposition);
				ItemLeftButtonUp.SetAssociatedComposition(boundsComposition);
				ItemLeftButtonDoubleClick.SetAssociatedComposition(boundsComposition);
				ItemMiddleButtonDown.SetAssociatedComposition(boundsComposition);
				ItemMiddleButtonUp.SetAssociatedComposition(boundsComposition);
				ItemMiddleButtonDoubleClick.SetAssociatedComposition(boundsComposition);
				ItemRightButtonDown.SetAssociatedComposition(boundsComposition);
				ItemRightButtonUp.SetAssociatedComposition(boundsComposition);
				ItemRightButtonDoubleClick.SetAssociatedComposition(boundsComposition);
				ItemMouseMove.SetAssociatedComposition(boundsComposition);
				ItemMouseEnter.SetAssociatedComposition(boundsComposition);
				ItemMouseLeave.SetAssociatedComposition(boundsComposition);

				callback = new ItemCallback(this);
				itemProvider->AttachCallback(callback.Obj());
				axis = new GuiDefaultAxis;

				if (acceptFocus)
				{
					boundsComposition->GetEventReceiver()->leftButtonDown.AttachMethod(this, &GuiListControl::OnBoundsMouseButtonDown);
					boundsComposition->GetEventReceiver()->middleButtonDown.AttachMethod(this, &GuiListControl::OnBoundsMouseButtonDown);
					boundsComposition->GetEventReceiver()->rightButtonDown.AttachMethod(this, &GuiListControl::OnBoundsMouseButtonDown);
					SetFocusableComposition(boundsComposition);
				}
			}

			GuiListControl::~GuiListControl()
			{
				if(itemArranger)
				{
					itemProvider->DetachCallback(itemArranger.Obj());
				}
				callback->ClearCache();
				itemStyleProperty = {};
				itemArranger = nullptr;
			}

			GuiListControl::IItemProvider* GuiListControl::GetItemProvider()
			{
				return itemProvider.Obj();
			}

			GuiListControl::ItemStyleProperty GuiListControl::GetItemTemplate()
			{
				return itemStyleProperty;
			}

			void GuiListControl::SetItemTemplate(ItemStyleProperty value)
			{
				SetStyleAndArranger(value, itemArranger);
				ItemTemplateChanged.Execute(GetNotifyEventArguments());
			}

			GuiListControl::IItemArranger* GuiListControl::GetArranger()
			{
				return itemArranger.Obj();
			}

			void GuiListControl::SetArranger(Ptr<IItemArranger> value)
			{
				SetStyleAndArranger(itemStyleProperty, value);
				ArrangerChanged.Execute(GetNotifyEventArguments());
			}

			compositions::IGuiAxis* GuiListControl::GetAxis()
			{
				return axis.Obj();
			}

			void GuiListControl::SetAxis(Ptr<compositions::IGuiAxis> value)
			{
				Ptr<IGuiAxis> old = axis;
				axis = value;
				SetStyleAndArranger(itemStyleProperty, itemArranger);
				AxisChanged.Execute(GetNotifyEventArguments());
			}

			bool GuiListControl::EnsureItemVisible(vint itemIndex)
			{
				if (itemIndex < 0 || itemIndex >= itemProvider->Count())
				{
					return false;
				}

				if (!itemArranger) return false;
				auto result = itemArranger->EnsureItemVisible(itemIndex);
				if (result == EnsureItemVisibleResult::Moved)
				{
					if (auto host = GetBoundsComposition()->GetRelatedGraphicsHost())
					{
						auto flag = GetDisposedFlag();
						host->InvokeAfterRendering([=]()
						{
							if (!flag->IsDisposed())
							{
								EnsureItemVisible(itemIndex);
							}
						}, { this,0 });
					}
				}
				return result != EnsureItemVisibleResult::ItemNotExists;
			}

			Size GuiListControl::GetAdoptedSize(Size expectedSize)
			{
				if (itemArranger)
				{
					Size controlSize = boundsComposition->GetBounds().GetSize();
					Size viewSize = containerComposition->GetBounds().GetSize();
					vint x = controlSize.x - viewSize.x;
					vint y = controlSize.y - viewSize.y;

					Size expectedViewSize(expectedSize.x - x, expectedSize.y - y);
					if (axis)
					{
						expectedViewSize = axis->RealSizeToVirtualSize(expectedViewSize);
					}
					Size adoptedViewSize = itemArranger->GetAdoptedSize(expectedViewSize);
					if (axis)
					{
						adoptedViewSize = axis->VirtualSizeToRealSize(adoptedViewSize);
					}
					return Size(adoptedViewSize.x + x, adoptedViewSize.y + y);
				}
				return expectedSize;
			}
			
			bool GuiListControl::GetDisplayItemBackground()
			{
				return displayItemBackground;
			}

			void GuiListControl::SetDisplayItemBackground(bool value)
			{
				if (displayItemBackground != value)
				{
					displayItemBackground = value;
					SetStyleAndArranger(itemStyleProperty, itemArranger);
				}
			}

/***********************************************************************
GuiSelectableListControl
***********************************************************************/

			void GuiSelectableListControl::NotifySelectionChanged()
			{
				SelectionChanged.Execute(GetNotifyEventArguments());
			}

			void GuiSelectableListControl::OnItemModified(vint start, vint count, vint newCount)
			{
				GuiListControl::OnItemModified(start, count, newCount);
				if(count!=newCount)
				{
					ClearSelection();
				}
			}

			void GuiSelectableListControl::OnStyleInstalled(vint itemIndex, ItemStyle* style)
			{
				GuiListControl::OnStyleInstalled(itemIndex, style);
				style->SetSelected(selectedItems.Contains(itemIndex));
			}

			void GuiSelectableListControl::OnItemSelectionChanged(vint itemIndex, bool value)
			{
				if(auto style = itemArranger->GetVisibleStyle(itemIndex))
				{
					style->SetSelected(value);
				}
			}

			void GuiSelectableListControl::OnItemSelectionCleared()
			{
				FOREACH(ItemStyle*, style, visibleStyles.Keys())
				{
					style->SetSelected(false);
				}
			}

			void GuiSelectableListControl::OnItemLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					SelectItemsByClick(arguments.itemIndex, arguments.ctrl, arguments.shift, true);
				}
			}

			void GuiSelectableListControl::OnItemRightButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					SelectItemsByClick(arguments.itemIndex, arguments.ctrl, arguments.shift, false);
				}
			}

			void GuiSelectableListControl::NormalizeSelectedItemIndexStartEnd()
			{
				if (selectedItemIndexStart < 0 || selectedItemIndexStart >= itemProvider->Count())
				{
					selectedItemIndexStart = 0;
				}
				if (selectedItemIndexEnd < 0 || selectedItemIndexEnd >= itemProvider->Count())
				{
					selectedItemIndexEnd = 0;
				}
			}

			void GuiSelectableListControl::SetMultipleItemsSelectedSilently(vint start, vint end, bool selected)
			{
				if(start>end)
				{
					vint temp=start;
					start=end;
					end=temp;
				}
				vint count=itemProvider->Count();
				if(start<0) start=0;
				if(end>=count) end=count-1;
				for(vint i=start;i<=end;i++)
				{
					if(selected)
					{
						if(!selectedItems.Contains(i))
						{
							selectedItems.Add(i);
						}
					}
					else
					{
						selectedItems.Remove(i);
					}
					OnItemSelectionChanged(i, selected);
				}
			}

			void GuiSelectableListControl::OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					if(SelectItemsByKey(arguments.code, arguments.ctrl, arguments.shift))
					{
						arguments.handled=true;
					}
				}
			}

			GuiSelectableListControl::GuiSelectableListControl(theme::ThemeName themeName, IItemProvider* _itemProvider)
				:GuiListControl(themeName, _itemProvider, true)
				, multiSelect(false)
				, selectedItemIndexStart(-1)
				, selectedItemIndexEnd(-1)
			{
				SelectionChanged.SetAssociatedComposition(boundsComposition);
				ItemLeftButtonDown.AttachMethod(this, &GuiSelectableListControl::OnItemLeftButtonDown);
				ItemRightButtonDown.AttachMethod(this, &GuiSelectableListControl::OnItemRightButtonDown);
				if (focusableComposition)
				{
					focusableComposition->GetEventReceiver()->keyDown.AttachMethod(this, &GuiSelectableListControl::OnKeyDown);
				}
			}

			GuiSelectableListControl::~GuiSelectableListControl()
			{
			}

			bool GuiSelectableListControl::GetMultiSelect()
			{
				return multiSelect;
			}

			void GuiSelectableListControl::SetMultiSelect(bool value)
			{
				if (multiSelect != value)
				{
					multiSelect = value;
					ClearSelection();
				}
			}

			const collections::SortedList<vint>& GuiSelectableListControl::GetSelectedItems()
			{
				return selectedItems;
			}

			vint GuiSelectableListControl::GetSelectedItemIndex()
			{
				return selectedItems.Count() == 1 ? selectedItems[0] : -1;
			}

			WString GuiSelectableListControl::GetSelectedItemText()
			{
				vint index = GetSelectedItemIndex();
				if (index != -1)
				{
					return itemProvider->GetTextValue(index);
				}
				return L"";
			}

			bool GuiSelectableListControl::GetSelected(vint itemIndex)
			{
				return selectedItems.Contains(itemIndex);
			}

			void GuiSelectableListControl::SetSelected(vint itemIndex, bool value)
			{
				if(value)
				{
					if(!selectedItems.Contains(itemIndex))
					{
						if(!multiSelect)
						{
							selectedItems.Clear();
							OnItemSelectionCleared();
						}
						selectedItems.Add(itemIndex);
						OnItemSelectionChanged(itemIndex, value);
						NotifySelectionChanged();
					}
				}
				else
				{
					if(selectedItems.Remove(itemIndex))
					{
						OnItemSelectionChanged(itemIndex, value);
						NotifySelectionChanged();
					}
				}
			}

			bool GuiSelectableListControl::SelectItemsByClick(vint itemIndex, bool ctrl, bool shift, bool leftButton)
			{
				NormalizeSelectedItemIndexStartEnd();
				if (0 <= itemIndex && itemIndex < itemProvider->Count())
				{
					if (!leftButton)
					{
						if (selectedItems.Contains(itemIndex))
						{
							return true;
						}
					}
					if (!multiSelect)
					{
						shift = false;
						ctrl = false;
					}
					if (shift)
					{
						if (!ctrl)
						{
							SetMultipleItemsSelectedSilently(selectedItemIndexStart, selectedItemIndexEnd, false);
						}
						selectedItemIndexEnd = itemIndex;
						SetMultipleItemsSelectedSilently(selectedItemIndexStart, selectedItemIndexEnd, true);
						NotifySelectionChanged();
					}
					else
					{
						if (ctrl)
						{
							vint index = selectedItems.IndexOf(itemIndex);
							if (index == -1)
							{
								selectedItems.Add(itemIndex);
							}
							else
							{
								selectedItems.RemoveAt(index);
							}
							OnItemSelectionChanged(itemIndex, index == -1);
							NotifySelectionChanged();
						}
						else
						{
							selectedItems.Clear();
							OnItemSelectionCleared();
							selectedItems.Add(itemIndex);
							OnItemSelectionChanged(itemIndex, true);
							NotifySelectionChanged();
						}
						selectedItemIndexStart = itemIndex;
						selectedItemIndexEnd = itemIndex;
					}
					return true;
				}
				return false;
			}

			bool GuiSelectableListControl::SelectItemsByKey(VKEY code, bool ctrl, bool shift)
			{
				if (!GetArranger()) return false;

				NormalizeSelectedItemIndexStartEnd();
				KeyDirection keyDirection = KeyDirection::Up;
				switch (code)
				{
				case VKEY::_UP:
					keyDirection = KeyDirection::Up;
					break;
				case VKEY::_DOWN:
					keyDirection = KeyDirection::Down;
					break;
				case VKEY::_LEFT:
					keyDirection = KeyDirection::Left;
					break;
				case VKEY::_RIGHT:
					keyDirection = KeyDirection::Right;
					break;
				case VKEY::_HOME:
					keyDirection = KeyDirection::Home;
					break;
				case VKEY::_END:
					keyDirection = KeyDirection::End;
					break;
				case VKEY::_PRIOR:
					keyDirection = KeyDirection::PageUp;
					break;
				case VKEY::_NEXT:
					keyDirection = KeyDirection::PageDown;
					break;
				default:
					return false;
				}

				if (GetAxis())
				{
					keyDirection = GetAxis()->RealKeyDirectionToVirtualKeyDirection(keyDirection);
				}
				vint itemIndex = GetArranger()->FindItem(selectedItemIndexEnd, keyDirection);
				if (SelectItemsByClick(itemIndex, ctrl, shift, true))
				{
					return EnsureItemVisible(itemIndex);
				}
				else
				{
					return false;
				}
			}

			void GuiSelectableListControl::ClearSelection()
			{
				if(selectedItems.Count()>0)
				{
					selectedItems.Clear();
					OnItemSelectionCleared();
					NotifySelectionChanged();
				}
			}

			namespace list
			{

/***********************************************************************
ItemProviderBase
***********************************************************************/

				void ItemProviderBase::InvokeOnItemModified(vint start, vint count, vint newCount)
				{
					CHECK_ERROR(!callingOnItemModified, L"ItemProviderBase::InvokeOnItemModified(vint, vint, vint)#Canning modify the observable data source during its item modified event, which will cause this event to be executed recursively.");
					callingOnItemModified = true;
					for (vint i = 0; i < callbacks.Count(); i++)
					{
						callbacks[i]->OnItemModified(start, count, newCount);
					}
					callingOnItemModified = false;
				}

				ItemProviderBase::ItemProviderBase()
				{
				}

				ItemProviderBase::~ItemProviderBase()
				{
					for(vint i=0;i<callbacks.Count();i++)
					{
						callbacks[i]->OnAttached(0);
					}
				}

				bool ItemProviderBase::AttachCallback(GuiListControl::IItemProviderCallback* value)
				{
					if(callbacks.Contains(value))
					{
						return false;
					}
					else
					{
						callbacks.Add(value);
						value->OnAttached(this);
						return true;
					}
				}

				bool ItemProviderBase::DetachCallback(GuiListControl::IItemProviderCallback* value)
				{
					vint index=callbacks.IndexOf(value);
					if(index==-1)
					{
						return false;
					}
					else
					{
						value->OnAttached(0);
						callbacks.Remove(value);
						return true;
					}
				}

				void ItemProviderBase::PushEditing()
				{
					editingCounter++;
				}

				bool ItemProviderBase::PopEditing()
				{
					if (editingCounter == 0)return false;
					editingCounter--;
					return true;
				}

				bool ItemProviderBase::IsEditing()
				{
					return editingCounter > 0;
				}
			}
		}
	}
}