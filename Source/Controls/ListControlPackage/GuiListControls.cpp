#include "GuiListControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;

			const wchar_t* const GuiListControl::IItemPrimaryTextView::Identifier = L"vl::presnetation::controls::GuiListControl::IItemPrimaryTextView";
			const wchar_t* const GuiListControl::IItemBindingView::Identifier = L"vl::presnetation::controls::GuiListControl::IItemBindingView";

/***********************************************************************
GuiListControl::ItemCallback
***********************************************************************/

			void GuiListControl::ItemCallback::OnStyleBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				listControl->CalculateView();
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
				for (vint i = 0; i < cachedStyles.Count(); i++)
				{
					listControl->itemStyleProvider->DestroyItemStyle(cachedStyles[i]);
				}
				for (vint i = 0; i < installedStyles.Count(); i++)
				{
					auto style = installedStyles.Keys()[i];
					auto handler = installedStyles.Values()[i];
					style->GetBoundsComposition()->BoundsChanged.Detach(handler);
					listControl->itemStyleProvider->DestroyItemStyle(style);
				}
				cachedStyles.Clear();
				installedStyles.Clear();
			}

			void GuiListControl::ItemCallback::OnAttached(IItemProvider* provider)
			{
			}

			void GuiListControl::ItemCallback::OnItemModified(vint start, vint count, vint newCount)
			{
				listControl->OnItemModified(start, count, newCount);
			}

			GuiListControl::IItemStyleController* GuiListControl::ItemCallback::RequestItem(vint itemIndex)
			{
				IItemStyleController* style = nullptr;
				if (cachedStyles.Count() > 0)
				{
					vint index = cachedStyles.Count() - 1;
					style = cachedStyles[index];
					cachedStyles.RemoveAt(index);
				}
				else
				{
					style = listControl->itemStyleProvider->CreateItemStyle();
				}

				listControl->itemStyleProvider->Install(style, itemIndex);
				style->OnInstalled();
				auto handler = style->GetBoundsComposition()->BoundsChanged.AttachMethod(this, &ItemCallback::OnStyleBoundsChanged);
				installedStyles.Add(style, handler);
				listControl->GetContainerComposition()->AddChild(style->GetBoundsComposition());
				listControl->OnStyleInstalled(itemIndex, style);
				return style;
			}

			void GuiListControl::ItemCallback::ReleaseItem(IItemStyleController* style)
			{
				vint index = installedStyles.Keys().IndexOf(style);
				if (index != -1)
				{
					listControl->OnStyleUninstalled(style);
					listControl->GetContainerComposition()->RemoveChild(style->GetBoundsComposition());
					auto handler = installedStyles.Values()[index];
					style->GetBoundsComposition()->BoundsChanged.Detach(handler);
					installedStyles.Remove(style);
					style->OnUninstalled();
					if (style->IsCacheable())
					{
						cachedStyles.Add(style);
					}
					else
					{
						listControl->itemStyleProvider->DestroyItemStyle(style);
					}
				}
			}

			void GuiListControl::ItemCallback::SetViewLocation(Point value)
			{
				Rect virtualRect(value, listControl->GetViewSize());
				Rect realRect = listControl->axis->VirtualRectToRealRect(listControl->fullSize, virtualRect);
				listControl->GetHorizontalScroll()->SetPosition(realRect.Left());
				listControl->GetVerticalScroll()->SetPosition(realRect.Top());
			}

			Size GuiListControl::ItemCallback::GetStylePreferredSize(IItemStyleController* style)
			{
				Size size = style->GetBoundsComposition()->GetPreferredBounds().GetSize();
				return listControl->axis->RealSizeToVirtualSize(size);
			}

			void GuiListControl::ItemCallback::SetStyleAlignmentToParent(IItemStyleController* style, Margin margin)
			{
				Margin newMargin = listControl->axis->VirtualMarginToRealMargin(margin);
				style->GetBoundsComposition()->SetAlignmentToParent(newMargin);
			}

			Rect GuiListControl::ItemCallback::GetStyleBounds(IItemStyleController* style)
			{
				Rect bounds = style->GetBoundsComposition()->GetBounds();
				return listControl->axis->RealRectToVirtualRect(listControl->GetViewSize(), bounds);
			}

			void GuiListControl::ItemCallback::SetStyleBounds(IItemStyleController* style, Rect bounds)
			{
				Rect newBounds = listControl->axis->VirtualRectToRealRect(listControl->GetViewSize(), bounds);
				return style->GetBoundsComposition()->SetBounds(newBounds);
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

			void GuiListControl::OnItemModified(vint start, vint count, vint newCount)
			{
			}

			void GuiListControl::OnStyleInstalled(vint itemIndex, IItemStyleController* style)
			{
				AttachItemEvents(style);
			}

			void GuiListControl::OnStyleUninstalled(IItemStyleController* style)
			{
				DetachItemEvents(style);
			}

			void GuiListControl::OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)
			{
				SetStyleProviderAndArranger(itemStyleProvider, itemArranger);
				GuiScrollView::OnRenderTargetChanged(renderTarget);
			}

			void GuiListControl::OnBeforeReleaseGraphicsHost()
			{
				GuiScrollView::OnBeforeReleaseGraphicsHost();
				SetStyleProviderAndArranger(0, 0);
			}

			Size GuiListControl::QueryFullSize()
			{
				Size virtualSize=itemArranger?itemArranger->GetTotalSize():Size(0, 0);
				fullSize=axis->VirtualSizeToRealSize(virtualSize);
				return fullSize;
			}

			void GuiListControl::UpdateView(Rect viewBounds)
			{
				if(itemArranger)
				{
					Rect newBounds=axis->RealRectToVirtualRect(fullSize, viewBounds);
					itemArranger->OnViewChanged(newBounds);
				}
			}

			void GuiListControl::OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					boundsComposition->GetRelatedGraphicsHost()->SetFocus(boundsComposition);
				}
			}

			void GuiListControl::SetStyleProviderAndArranger(Ptr<IItemStyleProvider> styleProvider, Ptr<IItemArranger> arranger)
			{
				if(itemStyleProvider)
				{
					itemStyleProvider->DetachListControl();
				}
				if(itemArranger)
				{
					itemArranger->DetachListControl();
					itemArranger->SetCallback(0);
					itemProvider->DetachCallback(itemArranger.Obj());
				}
				callback->ClearCache();

				itemStyleProvider=styleProvider;
				itemArranger=arranger;
				GetVerticalScroll()->SetPosition(0);
				GetHorizontalScroll()->SetPosition(0);

				if(itemStyleProvider)
				{
					itemStyleProvider->AttachListControl(this);
				}
				if(itemArranger)
				{
					itemProvider->AttachCallback(itemArranger.Obj());
					itemArranger->SetCallback(callback.Obj());
					itemArranger->AttachListControl(this);
				}
				CalculateView();
			}

			void GuiListControl::OnItemMouseEvent(compositions::GuiItemMouseEvent& itemEvent, IItemStyleController* style, compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(itemArranger && GetVisuallyEnabled())
				{
					vint itemIndex=itemArranger->GetVisibleIndex(style);
					if(itemIndex!=-1)
					{
						GuiItemMouseEventArgs redirectArguments;
						(GuiMouseEventArgs&)redirectArguments=arguments;
						redirectArguments.compositionSource=GetBoundsComposition();
						redirectArguments.eventSource=GetBoundsComposition();
						redirectArguments.itemIndex=itemIndex;
						itemEvent.Execute(redirectArguments);
						arguments=redirectArguments;
					}
				}
			}

			void GuiListControl::OnItemNotifyEvent(compositions::GuiItemNotifyEvent& itemEvent, IItemStyleController* style, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(itemArranger && GetVisuallyEnabled())
				{
					vint itemIndex=itemArranger->GetVisibleIndex(style);
					if(itemIndex!=-1)
					{
						GuiItemEventArgs redirectArguments;
						(GuiEventArgs&)redirectArguments=arguments;
						redirectArguments.compositionSource=GetBoundsComposition();
						redirectArguments.eventSource=GetBoundsComposition();
						redirectArguments.itemIndex=itemIndex;
						itemEvent.Execute(redirectArguments);
						arguments=redirectArguments;
					}
				}
			}

#define ATTACH_ITEM_MOUSE_EVENT(EVENTNAME, ITEMEVENTNAME)\
					{\
						Func<void(GuiItemMouseEvent&, IItemStyleController*, GuiGraphicsComposition*, GuiMouseEventArgs&)> func(this, &GuiListControl::OnItemMouseEvent);\
						helper->EVENTNAME##Handler=style->GetBoundsComposition()->GetEventReceiver()->EVENTNAME.AttachFunction(\
							Curry(Curry(func)(ITEMEVENTNAME))(style)\
							);\
					}\

#define ATTACH_ITEM_NOTIFY_EVENT(EVENTNAME, ITEMEVENTNAME)\
					{\
						Func<void(GuiItemNotifyEvent&, IItemStyleController*, GuiGraphicsComposition*, GuiEventArgs&)> func(this, &GuiListControl::OnItemNotifyEvent);\
						helper->EVENTNAME##Handler=style->GetBoundsComposition()->GetEventReceiver()->EVENTNAME.AttachFunction(\
							Curry(Curry(func)(ITEMEVENTNAME))(style)\
							);\
					}\

			void GuiListControl::AttachItemEvents(IItemStyleController* style)
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

#define DETACH_ITEM_EVENT(EVENTNAME) style->GetBoundsComposition()->GetEventReceiver()->EVENTNAME.Detach(helper->EVENTNAME##Handler)

			void GuiListControl::DetachItemEvents(IItemStyleController* style)
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

			GuiListControl::GuiListControl(IStyleProvider* _styleProvider, IItemProvider* _itemProvider, bool acceptFocus)
				:GuiScrollView(_styleProvider)
				,itemProvider(_itemProvider)
			{
				StyleProviderChanged.SetAssociatedComposition(boundsComposition);
				ArrangerChanged.SetAssociatedComposition(boundsComposition);
				AxisChanged.SetAssociatedComposition(boundsComposition);
				
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

				callback=new ItemCallback(this);
				itemProvider->AttachCallback(callback.Obj());
				axis=new GuiDefaultAxis;

				if(acceptFocus)
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
				itemStyleProvider=0;
				itemArranger=0;
			}

			GuiListControl::IItemProvider* GuiListControl::GetItemProvider()
			{
				return itemProvider.Obj();
			}

			GuiListControl::IItemStyleProvider* GuiListControl::GetStyleProvider()
			{
				return itemStyleProvider.Obj();
			}

			Ptr<GuiListControl::IItemStyleProvider> GuiListControl::SetStyleProvider(Ptr<IItemStyleProvider> value)
			{
				Ptr<IItemStyleProvider> old=itemStyleProvider;
				SetStyleProviderAndArranger(value, itemArranger);
				StyleProviderChanged.Execute(GetNotifyEventArguments());
				return old;
			}

			GuiListControl::IItemArranger* GuiListControl::GetArranger()
			{
				return itemArranger.Obj();
			}

			Ptr<GuiListControl::IItemArranger> GuiListControl::SetArranger(Ptr<IItemArranger> value)
			{
				Ptr<IItemArranger> old=itemArranger;
				SetStyleProviderAndArranger(itemStyleProvider, value);
				ArrangerChanged.Execute(GetNotifyEventArguments());
				return old;
			}

			compositions::IGuiAxis* GuiListControl::GetAxis()
			{
				return axis.Obj();
			}

			Ptr<compositions::IGuiAxis> GuiListControl::SetAxis(Ptr<compositions::IGuiAxis> value)
			{
				Ptr<IGuiAxis> old=axis;
				axis=value;
				SetStyleProviderAndArranger(itemStyleProvider, itemArranger);
				AxisChanged.Execute(GetNotifyEventArguments());
				return old;
			}

			bool GuiListControl::EnsureItemVisible(vint itemIndex)
			{
				if(itemIndex<0 || itemIndex>=itemProvider->Count())
				{
					return false;
				}
				return itemArranger?itemArranger->EnsureItemVisible(itemIndex):false;
			}

			Size GuiListControl::GetAdoptedSize(Size expectedSize)
			{
				if (itemArranger)
				{
					Size controlSize = GetBoundsComposition()->GetBounds().GetSize();
					Size viewSize = GetContainerComposition()->GetBounds().GetSize();
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

			void GuiSelectableListControl::OnStyleInstalled(vint itemIndex, IItemStyleController* style)
			{
				GuiListControl::OnStyleInstalled(itemIndex, style);
				selectableStyleProvider->SetStyleSelected(style, selectedItems.Contains(itemIndex));
			}

			void GuiSelectableListControl::OnStyleUninstalled(IItemStyleController* style)
			{
				GuiListControl::OnStyleUninstalled(style);
			}

			void GuiSelectableListControl::OnItemSelectionChanged(vint itemIndex, bool value)
			{
				GuiListControl::IItemStyleController* style=itemArranger->GetVisibleStyle(itemIndex);
				if(style)
				{
					selectableStyleProvider->SetStyleSelected(style, value);
				}
			}

			void GuiSelectableListControl::OnItemSelectionCleared()
			{
				for(vint i=0;i<visibleStyles.Count();i++)
				{
					selectableStyleProvider->SetStyleSelected(visibleStyles.Keys()[i], false);
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
				if(selectedItemIndexStart<0 || selectedItemIndexStart>=itemProvider->Count())
				{
					selectedItemIndexStart=0;
				}
				if(selectedItemIndexEnd<0 || selectedItemIndexEnd>=itemProvider->Count())
				{
					selectedItemIndexEnd=0;
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

			GuiSelectableListControl::GuiSelectableListControl(IStyleProvider* _styleProvider, IItemProvider* _itemProvider)
				:GuiListControl(_styleProvider, _itemProvider, true)
				,multiSelect(false)
				,selectedItemIndexStart(-1)
				,selectedItemIndexEnd(-1)
			{
				SelectionChanged.SetAssociatedComposition(boundsComposition);
				ItemLeftButtonDown.AttachMethod(this, &GuiSelectableListControl::OnItemLeftButtonDown);
				ItemRightButtonDown.AttachMethod(this, &GuiSelectableListControl::OnItemRightButtonDown);
				if(focusableComposition)
				{
					focusableComposition->GetEventReceiver()->keyDown.AttachMethod(this, &GuiSelectableListControl::OnKeyDown);
				}
			}

			GuiSelectableListControl::~GuiSelectableListControl()
			{
			}

			Ptr<GuiListControl::IItemStyleProvider> GuiSelectableListControl::SetStyleProvider(Ptr<GuiListControl::IItemStyleProvider> value)
			{
				selectableStyleProvider = value ? value.Cast<IItemStyleProvider>() : nullptr;
				return GuiListControl::SetStyleProvider(value);
			}

			bool GuiSelectableListControl::GetMultiSelect()
			{
				return multiSelect;
			}

			void GuiSelectableListControl::SetMultiSelect(bool value)
			{
				if(multiSelect!=value)
				{
					multiSelect=value;
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
					if (auto view = dynamic_cast<IItemPrimaryTextView*>(itemProvider->RequestView(IItemPrimaryTextView::Identifier)))
					{
						WString result = view->GetPrimaryTextViewText(index);
						itemProvider->ReleaseView(view);
						return result;
					}
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
				if(0<=itemIndex && itemIndex<itemProvider->Count())
				{
					if(!leftButton)
					{
						if(selectedItems.Contains(itemIndex))
						{
							return true;
						}
					}
					if(!multiSelect)
					{
						shift=false;
						ctrl=false;
					}
					if(shift)
					{
						if(!ctrl)
						{
							SetMultipleItemsSelectedSilently(selectedItemIndexStart, selectedItemIndexEnd, false);
						}
						selectedItemIndexEnd=itemIndex;
						SetMultipleItemsSelectedSilently(selectedItemIndexStart, selectedItemIndexEnd, true);
						NotifySelectionChanged();
					}
					else
					{
						if(ctrl)
						{
							vint index=selectedItems.IndexOf(itemIndex);
							if(index==-1)
							{
								selectedItems.Add(itemIndex);
							}
							else
							{
								selectedItems.RemoveAt(index);
							}
							OnItemSelectionChanged(itemIndex, index==-1);
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
						selectedItemIndexStart=itemIndex;
						selectedItemIndexEnd=itemIndex;
					}
					return true;
				}
				return false;
			}

			bool GuiSelectableListControl::SelectItemsByKey(vint code, bool ctrl, bool shift)
			{
				if(!GetArranger()) return false;

				NormalizeSelectedItemIndexStartEnd();
				KeyDirection keyDirection=KeyDirection::Up;
				switch(code)
				{
				case VKEY_UP:
					keyDirection=KeyDirection::Up;
					break;
				case VKEY_DOWN:
					keyDirection=KeyDirection::Down;
					break;
				case VKEY_LEFT:
					keyDirection=KeyDirection::Left;
					break;
				case VKEY_RIGHT:
					keyDirection=KeyDirection::Right;
					break;
				case VKEY_HOME:
					keyDirection=KeyDirection::Home;
					break;
				case VKEY_END:
					keyDirection=KeyDirection::End;
					break;
				case VKEY_PRIOR:
					keyDirection=KeyDirection::PageUp;
					break;
				case VKEY_NEXT:
					keyDirection=KeyDirection::PageDown;
					break;
				default:
					return false;
				}

				if(GetAxis())
				{
					keyDirection=GetAxis()->RealKeyDirectionToVirtualKeyDirection(keyDirection);
				}
				vint itemIndex=GetArranger()->FindItem(selectedItemIndexEnd, keyDirection);
				if(SelectItemsByClick(itemIndex, ctrl, shift, true))
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
ItemStyleControllerBase
***********************************************************************/

				void ItemStyleControllerBase::Initialize(compositions::GuiBoundsComposition* _boundsComposition, GuiControl* _associatedControl)
				{
					boundsComposition=_boundsComposition;
					associatedControl=_associatedControl;
				}

				void ItemStyleControllerBase::Finalize()
				{
					if(boundsComposition && !isInstalled)
					{
						if(associatedControl)
						{
							delete associatedControl;
						}
						else
						{
							delete boundsComposition;
						}
					}
					boundsComposition=0;
					associatedControl=0;
				}

				ItemStyleControllerBase::ItemStyleControllerBase(GuiListControl::IItemStyleProvider* _provider, vint _styleId)
					:provider(_provider)
					,styleId(_styleId)
					,boundsComposition(0)
					,associatedControl(0)
					,isInstalled(false)
				{
				}

				ItemStyleControllerBase::~ItemStyleControllerBase()
				{
					Finalize();
				}
					
				GuiListControl::IItemStyleProvider* ItemStyleControllerBase::GetStyleProvider()
				{
					return provider;
				}

				vint ItemStyleControllerBase::GetItemStyleId()
				{
					return styleId;
				}

				compositions::GuiBoundsComposition* ItemStyleControllerBase::GetBoundsComposition()
				{
					return boundsComposition;
				}

				bool ItemStyleControllerBase::IsCacheable()
				{
					return true;
				}

				bool ItemStyleControllerBase::IsInstalled()
				{
					return isInstalled;
				}

				void ItemStyleControllerBase::OnInstalled()
				{
					isInstalled=true;
				}

				void ItemStyleControllerBase::OnUninstalled()
				{
					isInstalled=false;
				}

/***********************************************************************
ItemProviderBase
***********************************************************************/

				void ItemProviderBase::InvokeOnItemModified(vint start, vint count, vint newCount)
				{
					for(vint i=0;i<callbacks.Count();i++)
					{
						callbacks[i]->OnItemModified(start, count, newCount);
					}
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
			}
		}
	}
}