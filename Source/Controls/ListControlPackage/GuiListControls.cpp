#include "GuiListControls.h"
#include "../Templates/GuiControlTemplates.h"
#include "../../Application/GraphicsHost/GuiGraphicsHost.h"
#include "../GuiButtonControls.h"

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

			GuiListControl::ItemStyleRecord GuiListControl::ItemCallback::InstallStyle(ItemStyle* style, vint itemIndex)
			{
				templates::GuiTemplate* bounds = style;
				if (listControl->GetDisplayItemBackground())
				{
					style->SetAlignmentToParent(Margin(0, 0, 0, 0));

					auto backgroundButton = new GuiSelectableButton(theme::ThemeName::ListItemBackground);
					if (auto backgroundStyle = listControl->TypedControlTemplateObject(true)->GetBackgroundTemplate())
					{
						backgroundButton->SetControlTemplate(backgroundStyle);
					}
					backgroundButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					backgroundButton->SetAutoFocus(false);
					backgroundButton->SetAutoSelection(false);
					backgroundButton->SetSelected(style->GetSelected());
					backgroundButton->GetContainerComposition()->AddChild(style);

					bounds = new templates::GuiTemplate;
					bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					bounds->AddChild(backgroundButton->GetBoundsComposition());
					
					style->SelectedChanged.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
					{
						backgroundButton->SetSelected(style->GetSelected());
					});
				}

				listControl->OnStyleInstalled(itemIndex, style, false);
				return { style,bounds };
			}

			GuiListControl::ItemStyleRecord GuiListControl::ItemCallback::UninstallStyle(vint index)
			{
				auto style = installedStyles.Keys()[index];
				auto bounds = installedStyles.Values()[index];
				listControl->OnStyleUninstalled(style);
				return { style,bounds };
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
				// TODO: (enumerable) foreach:indexed
				for (vint i = 0; i < installedStyles.Count(); i++)
				{
					auto [style, bounds] = UninstallStyle(i);
					SafeDeleteComposition(bounds);
				}
				installedStyles.Clear();
			}

			void GuiListControl::ItemCallback::OnAttached(list::IItemProvider* provider)
			{
				itemProvider = provider;
			}

			void GuiListControl::ItemCallback::OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)
			{
				listControl->OnItemModified(start, count, newCount, itemReferenceUpdated);
			}

			GuiListControl::ItemStyle* GuiListControl::ItemCallback::CreateItem(vint itemIndex)
			{
#define ERROR_MESSAGE_PREFIX L"GuiListControl::ItemCallback::RequestItem(vint)#"
				CHECK_ERROR(0 <= itemIndex && itemIndex < itemProvider->Count(), ERROR_MESSAGE_PREFIX L"Index out of range.");
				CHECK_ERROR(listControl->itemStyleProperty, ERROR_MESSAGE_PREFIX L"SetItemTemplate function should be called before adding items to the list control.");

				auto style = listControl->itemStyleProperty(itemProvider->GetBindingValue(itemIndex));
				auto record = InstallStyle(style, itemIndex);
				installedStyles.Add(record);
				return style;
#undef ERROR_MESSAGE_PREFIX
			}

			GuiListControl::ItemStyleBounds* GuiListControl::ItemCallback::GetItemBounds(ItemStyle * style)
			{
#define ERROR_MESSAGE_PREFIX L"GuiListControl::ItemCallback::GetItemBounds(GuiListItemTemplate*)#The style is not created from CreateItem."
				vint index = installedStyles.Keys().IndexOf(style);
				CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX);

				return installedStyles.Values()[index];
#undef ERROR_MESSAGE_PREFIX
			}

			GuiListControl::ItemStyle* GuiListControl::ItemCallback::GetItem(ItemStyleBounds* bounds)
			{
#define ERROR_MESSAGE_PREFIX L"GuiListControl::ItemCallback::GetItem(GuiTemplate*)#The bounds is not created from CreateItem."
				auto style = dynamic_cast<ItemStyle*>(bounds);
				if (style) return style;

				CHECK_ERROR(bounds->Children().Count() == 1, ERROR_MESSAGE_PREFIX);
				auto backgroundButton = dynamic_cast<GuiSelectableButton*>(bounds->Children()[0]->GetAssociatedControl());
				CHECK_ERROR(backgroundButton != nullptr, ERROR_MESSAGE_PREFIX);
				CHECK_ERROR(backgroundButton->GetContainerComposition()->Children().Count() == 1, ERROR_MESSAGE_PREFIX);
				style = dynamic_cast<ItemStyle*>(backgroundButton->GetContainerComposition()->Children()[0]);
				CHECK_ERROR(style != nullptr, ERROR_MESSAGE_PREFIX);

				vint index = installedStyles.Keys().IndexOf(style);
				CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX);
				CHECK_ERROR(installedStyles.Values()[index] == bounds, ERROR_MESSAGE_PREFIX);
				return style;
#undef ERROR_MESSAGE_PREFIX
			}

			void GuiListControl::ItemCallback::ReleaseItem(ItemStyle* style)
			{
#define ERROR_MESSAGE_PREFIX L"GuiListControl::ItemCallback::GetItemBounds(GuiListItemTemplate*)#The style is not created from CreateItem."
				vint index = installedStyles.Keys().IndexOf(style);
				CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX);

				auto bounds = UninstallStyle(index).value;
				installedStyles.Remove(style);
				SafeDeleteComposition(bounds);
#undef ERROR_MESSAGE_PREFIX
			}

			void GuiListControl::ItemCallback::SetViewLocation(Point value)
			{
				listControl->SetViewPosition(value);
			}

			compositions::GuiGraphicsComposition* GuiListControl::ItemCallback::GetContainerComposition()
			{
				return listControl->GetContainerComposition();
			}

			void GuiListControl::ItemCallback::OnTotalSizeChanged()
			{
				listControl->CalculateView();
			}

			void GuiListControl::ItemCallback::OnAdoptedSizeChanged()
			{
				listControl->AdoptedSizeInvalidated.Execute(listControl->GetNotifyEventArguments());
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

			void GuiListControl::OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)
			{
				// this function is executed before RangedItemArrangerBase::OnItemModified
				// but we only handle itemReferenceUpdated==false
				// so RangedItemArrangerBase::GetVisibleStyle is good here
				// even it is possible that the style object will be replaced later
				// OnStyleInstalled will be executed on affected style objects anyway
				if (!itemReferenceUpdated && itemArranger && count == newCount)
				{
					for (vint i = 0; i < newCount; i++)
					{
						vint index = start + i;
						if (auto style = itemArranger->GetVisibleStyle(index))
						{
							OnStyleInstalled(index, style, true);
						}
					}
				}
			}

			void GuiListControl::OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly)
			{
				style->SetFont(GetDisplayFont());
				style->SetContext(GetContext());
				style->SetText(itemProvider->GetTextValue(itemIndex));
				style->SetVisuallyEnabled(GetVisuallyEnabled());
				style->SetSelected(false);
				style->SetIndex(itemIndex);
				style->SetAssociatedListControl(this);

				if (!refreshPropertiesOnly)
				{
					AttachItemEvents(style);
				}
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
				fullSize = itemArranger ? itemArranger->GetTotalSize() : Size(0, 0);
				return fullSize;
			}

			void GuiListControl::UpdateView(Rect viewBounds)
			{
				if (itemArranger)
				{
					itemArranger->OnViewChanged(viewBounds);
				}
			}

			void GuiListControl::OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					SetFocused();
				}
			}

			void GuiListControl::SetStyleAndArranger(ItemStyleProperty styleProperty, Ptr<IItemArranger> arranger)
			{
				if (itemArranger)
				{
					itemProvider->DetachCallback(itemArranger.Obj());
					itemArranger->DetachListControl();
					itemArranger->SetCallback(nullptr);
				}
				callback->ClearCache();

				itemStyleProperty = styleProperty;
				itemArranger = arranger;

				if (itemArranger)
				{
					itemArranger->SetCallback(callback.Obj());
					itemArranger->AttachListControl(this);
					itemProvider->AttachCallback(itemArranger.Obj());
				}

				if (auto scroll = GetVerticalScroll())
				{
					scroll->SetPosition(0);
				}
				if (auto scroll = GetHorizontalScroll())
				{
					scroll->SetPosition(0);
				}
				CalculateView();
			}

			void GuiListControl::UpdateDisplayFont()
			{
				GuiControl::UpdateDisplayFont();
				for (auto style : visibleStyles.Keys())
				{
					style->SetFont(GetDisplayFont());
				}
			}

			void GuiListControl::OnVisuallyEnabledChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				for (auto style : visibleStyles.Keys())
				{
					style->SetVisuallyEnabled(GetVisuallyEnabled());
				}
			}

			void GuiListControl::OnContextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				for (auto style : visibleStyles.Keys())
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
						helper->EVENTNAME##Handler = style->GetEventReceiver()->EVENTNAME.AttachFunction(\
							[this, style](GuiGraphicsComposition* sender, GuiMouseEventArgs& args){ OnItemMouseEvent(ITEMEVENTNAME, style, sender, args); }\
							);\
					}\

#define ATTACH_ITEM_NOTIFY_EVENT(EVENTNAME, ITEMEVENTNAME)\
					{\
						helper->EVENTNAME##Handler = style->GetEventReceiver()->EVENTNAME.AttachFunction(\
							[this, style](GuiGraphicsComposition* sender, GuiEventArgs& args){ OnItemNotifyEvent(ITEMEVENTNAME, style, sender, args); }\
							);\
					}\

			void GuiListControl::AttachItemEvents(ItemStyle* style)
			{
				vint index=visibleStyles.Keys().IndexOf(style);
				if(index==-1)
				{
					auto helper=Ptr(new VisibleStyleHelper);
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

			GuiListControl::GuiListControl(theme::ThemeName themeName, list::IItemProvider* _itemProvider, bool acceptFocus)
				:GuiScrollView(themeName)
				, itemProvider(_itemProvider)
			{
				ContextChanged.AttachMethod(this, &GuiListControl::OnContextChanged);
				VisuallyEnabledChanged.AttachMethod(this, &GuiListControl::OnVisuallyEnabledChanged);

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

				callback = Ptr(new ItemCallback(this));
				itemProvider->AttachCallback(callback.Obj());
				axis = Ptr(new GuiDefaultAxis);

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

			list::IItemProvider* GuiListControl::GetItemProvider()
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
					Size controlSize = boundsComposition->GetCachedBounds().GetSize();
					Size viewSize = containerComposition->GetCachedBounds().GetSize();
					vint dx = controlSize.x - viewSize.x;
					vint dy = controlSize.y - viewSize.y;
					if (dx < 0) dx = 0;
					if (dy < 0) dy = 0;

					auto hscroll = GetHorizontalScroll();
					auto vscroll = GetVerticalScroll();

					if (!vscroll || vscroll->GetBoundsComposition()->GetEventuallyVisible())
					{
						if (adoptedSizeDiffWithScroll.x < dx) adoptedSizeDiffWithScroll.x = dx;
					}
					if (!vscroll || !vscroll->GetBoundsComposition()->GetEventuallyVisible())
					{
						if (adoptedSizeDiffWithoutScroll.x < dx) adoptedSizeDiffWithoutScroll.x = dx;
					}

					if (!hscroll || hscroll->GetBoundsComposition()->GetEventuallyVisible())
					{
						if (adoptedSizeDiffWithScroll.y < dy) adoptedSizeDiffWithScroll.y = dy;
					}
					if (!hscroll || !hscroll->GetBoundsComposition()->GetEventuallyVisible())
					{
						if (adoptedSizeDiffWithoutScroll.y < dy) adoptedSizeDiffWithoutScroll.y = dy;
					}

					vint x = adoptedSizeDiffWithoutScroll.x != -1 ? adoptedSizeDiffWithoutScroll.x : adoptedSizeDiffWithScroll.x;
					vint y = adoptedSizeDiffWithoutScroll.y != -1 ? adoptedSizeDiffWithoutScroll.y : adoptedSizeDiffWithScroll.y;

					Size expectedViewSize(expectedSize.x - x, expectedSize.y - y);
					Size adoptedViewSize = itemArranger->GetAdoptedSize(expectedViewSize);
					Size adoptedSize = Size(adoptedViewSize.x + x, adoptedViewSize.y + y);
					return adoptedSize;
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

			void GuiSelectableListControl::NotifySelectionChanged(bool triggeredByItemContentModified)
			{
				SelectionChanged.Execute(GetNotifyEventArguments());
			}

			void GuiSelectableListControl::OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)
			{
				GuiListControl::OnItemModified(start, count, newCount, itemReferenceUpdated);
				if (count != newCount)
				{
					ClearSelection();
				}
				else if (itemReferenceUpdated)
				{
					if (selectedItems.Count() > 0)
					{
						vint cmin = start;
						vint cmax = start + count - 1;
						vint smin = selectedItems[0];
						vint smax = selectedItems[selectedItems.Count() - 1];
						if (cmin <= smax && smin <= cmax)
						{
							ClearSelection();
						}
					}
				}
				else
				{
					if (GetSelectedItemIndex() == start)
					{
						NotifySelectionChanged(true);
					}
				}
			}

			void GuiSelectableListControl::OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly)
			{
				GuiListControl::OnStyleInstalled(itemIndex, style, refreshPropertiesOnly);
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
				for (auto style : visibleStyles.Keys())
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

			vint GuiSelectableListControl::FindItemByVirtualKeyDirection(vint index, compositions::KeyDirection keyDirection)
			{
				return GetArranger()->FindItemByVirtualKeyDirection(selectedItemIndexEnd, keyDirection);
			}

			GuiSelectableListControl::GuiSelectableListControl(theme::ThemeName themeName, list::IItemProvider* _itemProvider)
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
						NotifySelectionChanged(false);
					}
				}
				else
				{
					if(selectedItems.Remove(itemIndex))
					{
						OnItemSelectionChanged(itemIndex, value);
						NotifySelectionChanged(false);
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
							selectedItems.Clear();
							OnItemSelectionCleared();
						}
						selectedItemIndexEnd = itemIndex;
						SetMultipleItemsSelectedSilently(selectedItemIndexStart, selectedItemIndexEnd, true);
						NotifySelectionChanged(false);
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
							NotifySelectionChanged(false);
						}
						else
						{
							selectedItems.Clear();
							OnItemSelectionCleared();
							selectedItems.Add(itemIndex);
							OnItemSelectionChanged(itemIndex, true);
							NotifySelectionChanged(false);
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
				case VKEY::KEY_UP:
					keyDirection = KeyDirection::Up;
					break;
				case VKEY::KEY_DOWN:
					keyDirection = KeyDirection::Down;
					break;
				case VKEY::KEY_LEFT:
					keyDirection = KeyDirection::Left;
					break;
				case VKEY::KEY_RIGHT:
					keyDirection = KeyDirection::Right;
					break;
				case VKEY::KEY_HOME:
					keyDirection = KeyDirection::Home;
					break;
				case VKEY::KEY_END:
					keyDirection = KeyDirection::End;
					break;
				case VKEY::KEY_PRIOR:
					keyDirection = KeyDirection::PageUp;
					break;
				case VKEY::KEY_NEXT:
					keyDirection = KeyDirection::PageDown;
					break;
				default:
					return false;
				}

				if (GetAxis())
				{
					keyDirection = GetAxis()->RealKeyDirectionToVirtualKeyDirection(keyDirection);
				}
				vint itemIndex = FindItemByVirtualKeyDirection(selectedItemIndexEnd, keyDirection);
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
					selectedItemIndexStart = -1;
					selectedItemIndexEnd = -1;
					OnItemSelectionCleared();
					NotifySelectionChanged(false);
				}
			}
		}
	}
}