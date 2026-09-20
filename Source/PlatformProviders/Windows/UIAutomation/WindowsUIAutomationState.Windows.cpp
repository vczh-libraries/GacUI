#include "WindowsUIAutomationProvider.Windows.h"
#include "../../Hosted/GuiHostedApplication.h"
#include <cmath>

#ifdef VCZH_MSVC
namespace vl::presentation::windows
{
	using namespace collections;
	using namespace controls;
	using namespace compositions;
	using Kind = WindowsUIAutomationNodeKind;

	struct WindowsUIAutomationBoundsState : Object
	{
		bool disposed = false;
	};

	struct WindowsUIAutomationBoundsLifetime : Object
	{
		Ptr<WindowsUIAutomationBoundsState> disposed;
		WindowsUIAutomationBoundsLifetime(Ptr<WindowsUIAutomationBoundsState> value) : disposed(value) {}
		~WindowsUIAutomationBoundsLifetime() { disposed->disposed = true; }
	};

	struct WindowsUIAutomationBounds : Object
	{
		GuiGraphicsComposition* composition;
		Ptr<WindowsUIAutomationBoundsState> disposed = Ptr(new WindowsUIAutomationBoundsState);
		Ptr<IGuiGraphicsEventHandler> handler;
		void Detach()
		{
			if (!disposed->disposed)
			{
				composition->CachedBoundsChanged.Detach(handler);
				composition->SetInternalProperty(L"GacUI.Windows.UIAutomation.Bounds", nullptr);
			}
			handler = nullptr;
		}
	};

	EVENTID UiaWindowEvent(GuiControl* control, bool opening)
	{
		if (dynamic_cast<GuiTooltip*>(control)) return opening ? UIA_ToolTipOpenedEventId : UIA_ToolTipClosedEventId;
		if (dynamic_cast<GuiMenu*>(control)) return opening ? UIA_MenuOpenedEventId : UIA_MenuClosedEventId;
		return opening ? UIA_Window_WindowOpenedEventId : UIA_Window_WindowClosedEventId;
	}

	WindowsUIAutomationValue::WindowsUIAutomationValue() { VariantInit(&value); }
	WindowsUIAutomationValue::~WindowsUIAutomationValue() { VariantClear(&value); }

	WindowsUIAutomationContext::WindowsUIAutomationContext(bool isHostedMode)
		: hosted(isHostedMode), dispatcher(Ptr(new WindowsUIAutomationDispatcher))
	{
	}

	void WindowsUIAutomationContext::Stop()
	{
		stopped = true;
		for (auto node : nodes)
		{
			node->Retire();
			node->context = nullptr;
		}
		roots.Clear();
		windows.Clear();
		for (auto hook : boundsHooks) hook->Detach();
		boundsHooks.Clear();
		subscriptions.events.Clear();
		subscriptions.properties.Clear();
		openMenus.Clear();
		menuModeOwner = nullptr;
		nodes.Clear();
		dispatcher->Stop();
	}

	void WindowsUIAutomationContext::BindWindows()
	{
		if (!GetApplication() || !GetApplication()->GetMainWindow()) return;
		for (auto window : GetApplication()->GetWindows())
		{
			auto native = window->GetNativeWindow();
			if (!windows.Keys().Contains(native) || roots.Keys().Contains(native)) continue;
			auto root = Control(window);
			roots.Add(native, root);
			if (Subscribed())
			{
				Scan(window->GetBoundsComposition());
				if (window->GetOpening()) WindowEvent(root, true);
			}
		}
	}

	bool WindowsUIAutomationContext::Subscribed(EVENTID eventId)
	{
		return !stopped && (eventId ? subscriptions.events.Keys().Contains(eventId) : subscriptions.events.Count() > 0);
	}

	void WindowsUIAutomationContext::Raise(Ptr<WindowsUIAutomationNode> node, EVENTID eventId)
	{
		if (Subscribed(eventId) && node->IsLive()) UiaRaiseAutomationEvent(node->Provider().Obj(), eventId);
	}

	void WindowsUIAutomationContext::RemoveSubscriptions(Ptr<WindowsUIAutomationSubscriptions> value)
	{
		if (!value) return;
		if (!stopped)
		{
			for (auto [id, index] : indexed(value->events.Keys())) UiaChangeSubscription(subscriptions.events, id, -value->events.Values()[index]);
			for (auto [id, index] : indexed(value->properties.Keys())) UiaChangeSubscription(subscriptions.properties, id, -value->properties.Values()[index]);
		}
		value->events.Clear();
		value->properties.Clear();
	}

	void WindowsUIAutomationContext::Scan(GuiGraphicsComposition* composition)
	{
		if (!Subscribed()) return;
		auto key = WString::Unmanaged(L"GacUI.Windows.UIAutomation.Bounds");
		if (auto window = composition->GetRelatedControlHost(); window && !composition->GetInternalProperty(key))
		{
			auto hook = Ptr(new WindowsUIAutomationBounds);
			hook->composition = composition;
			composition->SetInternalProperty(key, Ptr(new WindowsUIAutomationBoundsLifetime(hook->disposed)));
			hook->handler = composition->CachedBoundsChanged.AttachLambda([anchor = Control(window)](GuiGraphicsComposition* sender, GuiEventArgs&)
			{
				if (anchor->context)
					if (auto window = sender->GetRelatedControlHost()) anchor->context->Notify(anchor->context->Control(window));
			});
			boundsHooks.Add(hook);
		}
		if (auto control = composition->GetAssociatedControl()) Observe(Control(control));
		for (auto child : composition->Children()) Scan(child);
	}

	void WindowsUIAutomationContext::RefreshObservation()
	{
		if (stopped) return;
		if (Subscribed())
		{
			BindWindows();
			for (auto window : GetApplication()->GetWindows()) Scan(window->GetBoundsComposition());
			// Selection discovery can append logical nodes. Index traversal includes
			// those nodes without invalidating an enumerator.
			for (vint i = 0; i < nodes.Count(); i++)
			{
				UpdateProperties(nodes[i], false);
				UpdateSelection(nodes[i], false);
			}
		}
		Collect();
	}

	void WindowsUIAutomationContext::Collect()
	{
		if (stopped) return;
		if (!Subscribed())
		{
			for (auto hook : boundsHooks) hook->Detach();
			boundsHooks.Clear();
			openMenus.Clear();
			menuModeOwner = nullptr;
		}
		else
		{
			for (vint i = boundsHooks.Count() - 1; i >= 0; i--)
				if (boundsHooks[i]->disposed->disposed) boundsHooks.RemoveAt(i);
		}
		SortedList<GuiControl*> required;
		for (auto node : nodes)
		{
			if (!node->IsLive() || !node->ExistingProvider()) continue;
			if (!required.Contains(node->control)) required.Add(node->control);
			if (auto window = node->Window(); !required.Contains(window)) required.Add(window);
		}
		for (vint i = nodes.Count() - 1; i >= 0; i--)
		{
			auto node = nodes[i];
			if (node->IsLive() && (Subscribed() || required.Contains(node->control))) continue;
			// Retired objects may outlive their UI attachment. Their retained node
			// remains self-contained and never needs the context again.
			node->Retire();
			for (vint r = roots.Count() - 1; r >= 0; r--)
				if (roots.Values()[r] == node) roots.Remove(roots.Keys()[r]);
			node->context = nullptr;
			nodes.RemoveAt(i);
		}
		if (!Subscribed())
		{
			for (auto node : nodes) { node->properties.Clear(); node->selection.Clear(); }
		}
	}

	void WindowsUIAutomationContext::RetireSubtree(GuiGraphicsComposition* composition)
	{
		// Removal is delivered while the detached subtree is still valid. Never
		// retain the event's raw child pointer for the deferred notification.
		for (vint i = boundsHooks.Count() - 1; i >= 0; i--)
		{
			if (boundsHooks[i]->composition == composition)
			{
				boundsHooks[i]->Detach();
				boundsHooks.RemoveAt(i);
			}
		}
		if (auto control = composition->GetAssociatedControl())
		{
			for (auto node : nodes) if (node->control == control) node->Retire();
		}
		for (auto child : composition->Children()) RetireSubtree(child);
	}

	void WindowsUIAutomationContext::WindowEvent(Ptr<WindowsUIAutomationNode> node, bool opening)
	{
		if (!Subscribed()) return;
		if (!dynamic_cast<GuiMenu*>(node->control))
		{
			// Hosted ShowModalAndDelete can retire the provider before a queued callback.
			if (!opening) Raise(node, UiaWindowEvent(node->control, false));
			Notify(node, true, opening ? UiaWindowEvent(node->control, true) : 0);
			return;
		}
		if (opening)
		{
			if (openMenus.Contains(node.Obj())) return;
			if (openMenus.Count() == 0)
			{
				menuModeOwner = Control(GetApplication()->GetMainWindow());
				Raise(menuModeOwner, UIA_MenuModeStartEventId);
			}
			openMenus.Add(node);
			Raise(node, UIA_MenuOpenedEventId);
		}
		else
		{
			if (!openMenus.Remove(node.Obj())) return;
			Raise(node, UIA_MenuClosedEventId);
			if (openMenus.Count() == 0)
			{
				if (menuModeOwner->IsLive()) Raise(menuModeOwner, UIA_MenuModeEndEventId);
				menuModeOwner = nullptr;
			}
		}
		Notify(node, true);
	}

	Ptr<WindowsUIAutomationNode> WindowsUIAutomationContext::Control(GuiControl* control)
	{
		auto key = WString::Unmanaged(L"GacUI.Windows.UIAutomation");
		if (auto lifetime = control->GetInternalProperty(key).Cast<WindowsUIAutomationLifetime>(); lifetime && !lifetime->node->retired) return lifetime->node;
		auto node = Ptr(new WindowsUIAutomationNode);
		node->context = this;
		node->dispatcher = dispatcher;
		node->disposed = control->GetDisposedFlag();
		node->control = control;
		node->id = nextId++;
		nodes.Add(node);
		auto lifetime = Ptr(new WindowsUIAutomationLifetime);
		lifetime->node = node;
		control->SetInternalProperty(key, lifetime);
		return node;
	}

	void WindowsUIAutomationContext::Observe(Ptr<WindowsUIAutomationNode> node)
	{
		if (!node->IsLive()) return;
		if (node->kind != Kind::Control) { Observe(node->owner); return; }
		auto control = node->control;
		auto lifetime = control->GetInternalProperty(L"GacUI.Windows.UIAutomation").Cast<WindowsUIAutomationLifetime>();
		if (lifetime->attached) return;
		lifetime->attached = true;
		if (auto window = node->Window(); window != control) Observe(Control(window));
		auto changed = [node](GuiGraphicsComposition*, GuiEventArgs&)
		{
			if (node->IsLive()) node->context->Notify(node);
		};
		auto structure = [node](GuiGraphicsComposition*, GuiEventArgs&)
		{
			if (node->IsLive())
			{
				node->context->Notify(node, true);
			}
		};
		lifetime->Attach(control, control->TextChanged, [node](GuiGraphicsComposition*, GuiEventArgs&)
		{
			if (!node->IsLive()) return;
			auto document = UiaDocument(node->control);
			node->context->Notify(node, document != nullptr, document ? UIA_Text_TextChangedEventId : 0);
			for (auto target : node->context->nodes)
				if (target->kind == Kind::Control && target->IsLive())
					if (auto metadata = UiaMetadata(target->control); metadata && metadata->label == node->control)
						node->context->Notify(target);
		});
		lifetime->Attach(control, control->VisibleChanged, structure);
		lifetime->Attach(control, control->VisuallyEnabledChanged, changed);
		lifetime->Attach(control, control->ControlTemplateChanged, structure);
		lifetime->Attach(control, control->FocusedChanged, [node](GuiGraphicsComposition*, GuiEventArgs&)
		{
			if (!node->IsLive()) return;
			for (auto parent = node->control->GetParent(); parent; parent = parent->GetParent())
			{
				if (auto calendar = UiaCalendar(parent))
				{
					for (vint r = 0; r < 6; r++) for (vint c = 0; c < 7; c++)
						if (calendar->GetDayButton(r, c) == node->control)
						{
							node->context->Notify(node->context->Item(node->context->Control(parent), Kind::CalendarDay, r, c));
							return;
						}
					break;
				}
			}
			node->context->Notify(node);
		});
		if (auto window = dynamic_cast<GuiWindow*>(control))
		{
			lifetime->Attach(window, window->BoundsChanged, [node](GuiGraphicsComposition*, GuiEventArgs&)
			{
				if (!node->IsLive()) return;
				for (auto child : node->context->nodes)
					if (child->IsLive() && child->Window() == node->control) node->context->Notify(child);
			});
			auto windowChanged = [node](bool opening)
			{
				if (!node->IsLive()) return;
				node->context->WindowEvent(node, opening);
				if (node->context->hosted && node->control != GetApplication()->GetMainWindow())
					node->context->Notify(node->context->Control(GetApplication()->GetMainWindow()), true);
			};
			lifetime->Attach(window, window->WindowOpened, [windowChanged](GuiGraphicsComposition*, GuiEventArgs&) { windowChanged(true); });
			lifetime->Attach(window, window->WindowClosed, [windowChanged](GuiGraphicsComposition*, GuiEventArgs&) { windowChanged(false); });
			lifetime->Attach(window, window->ChildCompositionUpdated, [node](GuiGraphicsComposition*, GuiCompositionUpdateEventArgs& arguments)
			{
				if (!node->IsLive()) return;
				if (arguments.updateType == CompositionUpdateType::Removed)
				{
					node->context->RetireSubtree(arguments.child);
					node->dispatcher->Queue([node]() { if (node->context) node->context->Collect(); });
				}
				else if (arguments.updateType == CompositionUpdateType::Inserted)
				{
					// The application may focus or mutate this child immediately after
					// AddChild returns, before a queued discovery pass can observe it.
					node->context->Scan(arguments.child);
				}
				node->context->Notify(node, true);
			});
		}
		if (auto button = dynamic_cast<GuiButton*>(control))
		{
			lifetime->Attach(button, button->BeforeClicked, [node](GuiGraphicsComposition*, GuiEventArgs&)
			{
				// Commands may open modal windows or destroy their own control.
				if (node->IsLive() && node->Supports(UIA_InvokePatternId)) node->context->Raise(node, UIA_Invoke_InvokedEventId);
			});
		}
		if (auto button = dynamic_cast<GuiSelectableButton*>(control))
		{
			lifetime->Attach(button, button->SelectedChanged, [node](GuiGraphicsComposition*, GuiEventArgs&)
			{
				if (!node->IsLive()) return;
				auto group = UiaRadioGroup(node.Obj());
				if (group) node->context->Notify(group);
				node->context->Notify(node, false, !group && node->Supports(UIA_SelectionItemPatternId) && node->IsSelected() ? UIA_SelectionItem_ElementSelectedEventId : 0);
			});
			lifetime->Attach(button, button->AutoSelectionChanged, changed);
			lifetime->Attach(button, button->GroupControllerChanged, [node](GuiGraphicsComposition*, GuiEventArgs&)
			{
				if (!node->IsLive()) return;
				for (auto group : node->context->nodes) if (group->kind == Kind::RadioGroup && group->owner == node) group->Retire();
				for (auto group : node->context->nodes) if (group->kind == Kind::RadioGroup && group->IsLive()) node->context->Notify(group);
				node->context->Notify(node->context->Control(node->Window()), true);
			});
		}
		if (auto menu = dynamic_cast<GuiMenuButton*>(control))
		{
			lifetime->Attach(menu, menu->SubMenuOpeningChanged, structure);
			lifetime->Attach(menu, menu->ShortcutTextChanged, changed);
		}
		if (auto gallery = dynamic_cast<GuiBindableRibbonGalleryList*>(control))
		{
			lifetime->Attach(gallery->GetSubMenu(), gallery->GetSubMenu()->WindowOpened, structure);
			lifetime->Attach(gallery->GetSubMenu(), gallery->GetSubMenu()->WindowClosed, structure);
		}
		if (auto tab = dynamic_cast<GuiTab*>(control)) lifetime->Attach(tab, tab->SelectedPageChanged, structure);
		if (auto scroll = dynamic_cast<GuiScroll*>(control))
		{
			auto scrollChanged = [node, changed](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				changed(sender, arguments);
				if (!node->IsLive()) return;
				for (auto parent = node->control->GetParent(); parent; parent = parent->GetParent())
				{
					if (auto view = dynamic_cast<GuiScrollView*>(parent))
					{
						node->context->Notify(node->context->Control(view));
						break;
					}
				}
			};
			lifetime->Attach(scroll, scroll->PositionChanged, scrollChanged);
			lifetime->Attach(scroll, scroll->TotalSizeChanged, scrollChanged);
			lifetime->Attach(scroll, scroll->PageSizeChanged, scrollChanged);
		}
		if (auto list = dynamic_cast<GuiListControl*>(control))
		{
			list->GetItemProvider()->AttachCallback(lifetime.Obj());
			lifetime->columns = dynamic_cast<list::IColumnItemView*>(list->GetItemProvider()->RequestView(WString::Unmanaged(list::IColumnItemView::Identifier)));
			if (lifetime->columns) lifetime->columns->AttachCallback(lifetime.Obj());
			lifetime->Attach(list, list->ItemTemplateChanged, structure);
			lifetime->Attach(list, list->ArrangerChanged, structure);
		}
		if (auto list = dynamic_cast<GuiSelectableListControl*>(control))
		{
			lifetime->Attach(list, list->SelectionChanged, [node](GuiGraphicsComposition*, GuiEventArgs&)
			{
				if (node->IsLive()) node->context->Notify(node, false, UIA_Selection_InvalidatedEventId);
			});
		}
		if (auto tree = dynamic_cast<GuiVirtualTreeListControl*>(control))
		{
			// NodeRootProviderBase does not send OnAttached(nullptr) when destroyed.
			// Retain the model until this hook has detached on the UI thread.
			lifetime->tree = Ptr(tree->GetNodeRootProvider());
			lifetime->tree->AttachCallback(lifetime.Obj());
		}
		if (auto grid = dynamic_cast<GuiVirtualDataGrid*>(control)) lifetime->Attach(grid, grid->SelectedCellChanged, structure);
		if (auto date = dynamic_cast<GuiDatePicker*>(control))
		{
			lifetime->Attach(date, date->DateChanged, changed);
			lifetime->Attach(date, date->DateNavigated, structure);
		}
		if (auto document = UiaDocument(control))
		{
			lifetime->Attach(control, document->BeforeActiveHyperlinkExecuted, [node](GuiGraphicsComposition*, GuiEventArgs&)
			{
				if (!node->IsLive()) return;
				auto run = UiaDocument(node->control)->GetActiveHyperlink();
				if (!run) return;
				auto child = node->context->Item(node, Kind::DocumentObject, -1, -1, nullptr, run);
				if (child->IsLive()) node->context->Raise(child, UIA_Invoke_InvokedEventId);
			});
			lifetime->Attach(control, document->EditModeChanged, changed);
			lifetime->Attach(control, document->SelectionChanged, [node](GuiGraphicsComposition*, GuiEventArgs&)
			{
				if (node->IsLive()) node->context->Notify(node, false, UIA_Text_TextSelectionChangedEventId);
			});
		}
		if (auto text = dynamic_cast<GuiSinglelineTextBox*>(control))
		{
			lifetime->Attach(text, text->PasswordCharChanged, [node](GuiGraphicsComposition*, GuiEventArgs&)
			{
				if (!node->IsLive()) return;
				// Drop plaintext before any queued callback can publish it as oldValue.
				node->properties.Remove(UIA_ValueValuePropertyId);
				for (auto child : node->context->nodes)
					if (child->owner == node && child->kind == Kind::DocumentObject) child->Retire();
				node->context->Notify(node, true);
			});
		}
		UpdateProperties(node, false);
		UpdateSelection(node, false);
	}

	Ptr<WindowsUIAutomationNode> WindowsUIAutomationContext::Item(Ptr<WindowsUIAutomationNode> owner, Kind kind, vint row, vint column, Ptr<tree::INodeProvider> treeNode, Ptr<DocumentRun> documentRun)
	{
		for (auto node : nodes)
		{
			if (!node->retired && node->owner == owner && node->kind == kind && node->row == row && node->column == column && node->treeNode == treeNode && node->documentRun == documentRun) return node;
		}
		auto node = Ptr(new WindowsUIAutomationNode);
		node->context = this;
		node->dispatcher = dispatcher;
		node->disposed = owner->disposed;
		node->control = owner->control;
		node->owner = owner;
		node->kind = kind;
		node->row = row;
		node->column = column;
		node->treeNode = treeNode;
		node->documentRun = documentRun;
		node->id = nextId++;
		nodes.Add(node);
		UpdateProperties(node, false);
		return node;
	}

	void WindowsUIAutomationContext::Notify(Ptr<WindowsUIAutomationNode> node, bool structure, EVENTID eventId)
	{
		if (!Subscribed()) return;
		node->structurePending |= structure;
		if (eventId && Subscribed(eventId) && !node->pendingEvents.Contains(eventId)) node->pendingEvents.Add(eventId);
		if (node->notificationPending) return;
		node->notificationPending = true;
		// Post rather than raising inside synchronous provider calls or model callbacks.
		dispatcher->Queue([node]()
		{
			node->notificationPending = false;
			auto structure = node->structurePending;
			node->structurePending = false;
			SortedList<EVENTID> events;
			CopyFrom(events, node->pendingEvents);
			node->pendingEvents.Clear();
			if (!node->IsLive()) return;
			auto context = node->context;
			// Construction and model callbacks can report intermediate compositions.
			// Rescan the final live tree without retaining notification pointers.
			if (structure) context->Scan(node->control->GetBoundsComposition());
			context->UpdateProperties(node, true);
			context->UpdateSelection(node, true);
			for (vint i = 0; i < context->nodes.Count(); i++)
			{
				auto child = context->nodes[i];
				if (child != node && child->IsLive() && (child->owner == node || dynamic_cast<GuiWindow*>(node->control) && child->Window() == node->control || structure && node->IsRoot())) context->UpdateProperties(child, true);
			}
			if (structure && context->Subscribed(UIA_StructureChangedEventId)) UiaRaiseStructureChangedEvent(node->Provider().Obj(), StructureChangeType_ChildrenInvalidated, nullptr, 0);
			for (auto eventId : events) context->Raise(node, eventId);
		});
	}

	void WindowsUIAutomationContext::UpdateSelection(Ptr<WindowsUIAutomationNode> node, bool raiseEvents)
	{
		if (!(Subscribed(UIA_SelectionItem_ElementSelectedEventId) || Subscribed(UIA_SelectionItem_ElementAddedToSelectionEventId) || Subscribed(UIA_SelectionItem_ElementRemovedFromSelectionEventId)) || !node->IsLive() || !node->Supports(UIA_SelectionPatternId) || dynamic_cast<GuiComboBoxListControl*>(node->control)) return;
		List<Ptr<WindowsUIAutomationNode>> selected;
		UiaSelectedChildren(node, selected);
		if (raiseEvents)
		{
			if (selected.Count() == 1)
			{
				if (node->selection.Count() != 1 || node->selection[0] != selected[0]->id) Raise(selected[0], UIA_SelectionItem_ElementSelectedEventId);
			}
			else
			{
				for (auto item : selected) if (!node->selection.Contains(item->id)) Raise(item, UIA_SelectionItem_ElementAddedToSelectionEventId);
				for (auto id : node->selection)
				{
					if (From(selected).Any([=](auto item) { return item->id == id; })) continue;
					for (auto item : nodes) if (item->id == id && item->IsLive()) Raise(item, UIA_SelectionItem_ElementRemovedFromSelectionEventId);
				}
			}
		}
		node->selection.Clear();
		for (auto item : selected) node->selection.Add(item->id);
	}

	void WindowsUIAutomationContext::UpdateProperties(Ptr<WindowsUIAutomationNode> node, bool raiseEvents)
	{
		if (!node->IsLive() || !(subscriptions.properties.Count() || Subscribed(UIA_AutomationFocusChangedEventId))) return;
		const PROPERTYID watched[] =
		{
			UIA_NamePropertyId, UIA_AutomationIdPropertyId, UIA_HelpTextPropertyId, UIA_LabeledByPropertyId, UIA_AcceleratorKeyPropertyId, UIA_ItemStatusPropertyId, UIA_OrientationPropertyId,
			UIA_IsEnabledPropertyId, UIA_IsOffscreenPropertyId, UIA_HasKeyboardFocusPropertyId, UIA_IsContentElementPropertyId,
			UIA_IsPasswordPropertyId, UIA_IsKeyboardFocusablePropertyId, UIA_AccessKeyPropertyId,
			UIA_BoundingRectanglePropertyId, UIA_GridRowCountPropertyId, UIA_GridColumnCountPropertyId,
			UIA_SelectionCanSelectMultiplePropertyId, UIA_SelectionIsSelectionRequiredPropertyId,
			UIA_WindowWindowVisualStatePropertyId, UIA_WindowWindowInteractionStatePropertyId,
			UIA_ValueValuePropertyId, UIA_ValueIsReadOnlyPropertyId, UIA_ToggleToggleStatePropertyId, UIA_SelectionItemIsSelectedPropertyId,
			UIA_ExpandCollapseExpandCollapseStatePropertyId, UIA_RangeValueValuePropertyId, UIA_RangeValueMinimumPropertyId,
			UIA_RangeValueMaximumPropertyId, UIA_RangeValueSmallChangePropertyId, UIA_RangeValueLargeChangePropertyId, UIA_RangeValueIsReadOnlyPropertyId,
			UIA_ScrollHorizontalScrollPercentPropertyId, UIA_ScrollVerticalScrollPercentPropertyId, UIA_ScrollHorizontalViewSizePropertyId,
			UIA_ScrollVerticalViewSizePropertyId, UIA_ScrollHorizontallyScrollablePropertyId, UIA_ScrollVerticallyScrollablePropertyId,
			UIA_MultipleViewCurrentViewPropertyId, UIA_ControlTypePropertyId, UIA_IsInvokePatternAvailablePropertyId, UIA_IsTogglePatternAvailablePropertyId,
			UIA_IsGridPatternAvailablePropertyId, UIA_IsTablePatternAvailablePropertyId, UIA_IsTextPatternAvailablePropertyId, UIA_IsValuePatternAvailablePropertyId,
			UIA_IsSelectionItemPatternAvailablePropertyId, UIA_IsExpandCollapsePatternAvailablePropertyId, UIA_IsTransformPatternAvailablePropertyId,
		};
		for (auto property : watched)
		{
			bool propertyRequested = subscriptions.properties.Keys().Contains(0) || subscriptions.properties.Keys().Contains(property);
			if (!propertyRequested && !(property == UIA_HasKeyboardFocusPropertyId && Subscribed(UIA_AutomationFocusChangedEventId)))
			{
				node->properties.Remove(property);
				continue;
			}
			auto value = Ptr(new WindowsUIAutomationValue);
			if (property == UIA_LabeledByPropertyId)
			{
				// Snapshot identity only. Providers are acquired transiently when an
				// actual subscribed property event is delivered.
				value->value.vt = VT_I8;
				value->value.llVal = 0;
				if (node->kind == Kind::Control)
					if (auto metadata = UiaMetadata(node->control); metadata && metadata->label && !metadata->labelDisposed->IsDisposed())
						value->value.llVal = Control(metadata->label)->id;
			}
			else node->Property(property, &value->value);
			auto index = node->properties.Keys().IndexOf(property);
			if (index != -1)
			{
				auto old = node->properties.Values()[index];
				bool bothEmpty = old->value.vt == VT_EMPTY && value->value.vt == VT_EMPTY;
				bool equal = bothEmpty || VarCmp(&old->value, &value->value, LOCALE_INVARIANT, 0) == VARCMP_EQ;
				if (old->value.vt == VT_UNKNOWN && value->value.vt == VT_UNKNOWN) equal = old->value.punkVal == value->value.punkVal;
				if (old->value.vt == VT_R8 && value->value.vt == VT_R8 && std::isnan(old->value.dblVal) && std::isnan(value->value.dblVal)) equal = true;
				if (property == UIA_BoundingRectanglePropertyId && old->value.vt == (VT_ARRAY | VT_R8) && value->value.vt == (VT_ARRAY | VT_R8))
				{
					equal = true;
					for (LONG i = 0; i < 4; i++)
					{
						double a = 0, b = 0;
						SafeArrayGetElement(old->value.parray, &i, &a);
						SafeArrayGetElement(value->value.parray, &i, &b);
						if (a != b) equal = false;
					}
				}
				if (raiseEvents && !equal)
				{
					if (propertyRequested)
					{
						if (property == UIA_LabeledByPropertyId)
						{
							WindowsUIAutomationValue before, after;
							for (auto label : nodes)
							{
								if (!label->IsLive()) continue;
								if (label->id == old->value.llVal) { before.value.vt = VT_UNKNOWN; label->Provider()->QueryInterface(IID_PPV_ARGS(&before.value.punkVal)); }
								if (label->id == value->value.llVal) { after.value.vt = VT_UNKNOWN; label->Provider()->QueryInterface(IID_PPV_ARGS(&after.value.punkVal)); }
							}
							UiaRaiseAutomationPropertyChangedEvent(node->Provider().Obj(), property, before.value, after.value);
						}
						else UiaRaiseAutomationPropertyChangedEvent(node->Provider().Obj(), property, old->value, value->value);
					}
					if (property == UIA_HasKeyboardFocusPropertyId && value->value.boolVal == VARIANT_TRUE) Raise(node, UIA_AutomationFocusChangedEventId);
				}
			}
			node->properties.Set(property, value);
		}
	}

	void WindowsUIAutomationNode::Retire()
	{
		if (retired) return;
		retired = true;
		treeNode = nullptr;
		documentRun = nullptr;
		properties.Clear();
		selection.Clear();
		pendingEvents.Clear();
		if (kind == Kind::Control && !disposed->IsDisposed())
		{
			auto lifetime = control->GetInternalProperty(L"GacUI.Windows.UIAutomation").Cast<WindowsUIAutomationLifetime>();
			if (lifetime && lifetime->node.Obj() == this)
			{
				lifetime->Detach();
				control->SetInternalProperty(L"GacUI.Windows.UIAutomation", nullptr);
			}
		}
		if (auto saved = ExistingProvider())
		{
			auto implementation = static_cast<WindowsUIAutomationProvider*>(saved.Obj());
			if (context) context->RemoveSubscriptions(implementation->subscriptions);
			// IsLive is already false. A reentrant request cannot receive this
			// attachment, and disconnect never runs inside a SendMessage response.
			dispatcher->Queue([saved]() { UiaDisconnectProvider(saved.Obj()); });
		}
	}

	void WindowsUIAutomationLifetime::Detach()
	{
		attached = false;
		for (auto detach : detachHandlers) detach();
		detachHandlers.Clear();
		if (columns) columns->DetachCallback(this);
		columns = nullptr;
		if (items) items->DetachCallback(this);
		items = nullptr;
		if (tree) tree->DetachCallback(this);
		tree = nullptr;
	}

	WindowsUIAutomationLifetime::~WindowsUIAutomationLifetime()
	{
		Detach();
		if (node->context)
		{
			for (auto item : node->context->nodes) if (item->owner == node) item->Retire();
		}
		node->Retire();
	}

	void WindowsUIAutomationLifetime::OnAttached(list::IItemProvider* provider) { items = provider; if (!provider) columns = nullptr; }
	void WindowsUIAutomationLifetime::OnAttached(tree::INodeRootProvider*) {}

	void WindowsUIAutomationLifetime::OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		if (!node->context) return;
		for (auto item : node->context->nodes)
		{
			if (item->owner != node || item->retired || item->treeNode || item->row < 0) continue;
			if (item->row >= start + count) item->row += newCount - count;
			else if (item->row >= start && (itemReferenceUpdated || count != newCount)) item->Retire();
		}
		if (node->IsLive()) node->context->Notify(node, true);
	}

	void WindowsUIAutomationLifetime::OnBeforeItemModified(tree::INodeProvider* parent, vint start, vint count, vint, bool itemReferenceUpdated)
	{
		if (!itemReferenceUpdated || !node->context) return;
		for (auto item : node->context->nodes)
		{
			if (item->owner != node || !item->treeNode) continue;
			for (auto ancestor = item->treeNode; ancestor; ancestor = ancestor->GetParent())
			{
				if (ancestor->GetParent().Obj() != parent) continue;
				for (vint i = start; i < start + count; i++) if (parent->GetChild(i) == ancestor) item->Retire();
				break;
			}
		}
	}
	void WindowsUIAutomationLifetime::OnAfterItemModified(tree::INodeProvider*, vint, vint, vint, bool)
	{
		if (node->IsLive()) node->context->Notify(node, true);
	}
	void WindowsUIAutomationLifetime::OnItemExpanded(tree::INodeProvider*)
	{
		if (node->IsLive()) node->context->Notify(node, true);
	}
	void WindowsUIAutomationLifetime::OnItemCollapsed(tree::INodeProvider* value) { OnItemExpanded(value); }
	void WindowsUIAutomationLifetime::OnColumnRebuilt()
	{
		if (!node->context) return;
		for (auto item : node->context->nodes)
			if (item->owner == node && (item->kind == Kind::Cell || item->kind == Kind::HeaderItem)) item->Retire();
		if (node->IsLive()) node->context->Notify(node, true);
	}
	void WindowsUIAutomationLifetime::OnColumnChanged(bool)
	{
		if (node->IsLive()) node->context->Notify(node, true);
	}
}
#endif
