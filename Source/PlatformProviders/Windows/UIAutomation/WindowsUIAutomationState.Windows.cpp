#include "WindowsUIAutomationProvider.Windows.h"
#include "../../Hosted/GuiHostedApplication.h"

#ifdef VCZH_MSVC
namespace vl::presentation::windows
{
	using namespace collections;
	using namespace controls;
	using namespace compositions;
	using Kind = WindowsUIAutomationNodeKind;

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
		combos.Clear();
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
			Scan(window->GetBoundsComposition());
			if (window->GetOpening()) Notify(root, true, UiaWindowEvent(window, true));
		}
	}

	void WindowsUIAutomationContext::Scan(GuiGraphicsComposition* composition)
	{
		if (auto control = composition->GetAssociatedControl()) Control(control);
		for (auto child : composition->Children()) Scan(child);
	}

	Ptr<WindowsUIAutomationNode> WindowsUIAutomationContext::Control(GuiControl* control)
	{
		auto key = WString::Unmanaged(L"GacUI.Windows.UIAutomation");
		if (auto lifetime = control->GetInternalProperty(key).Cast<WindowsUIAutomationLifetime>()) return lifetime->node;
		auto node = Ptr(new WindowsUIAutomationNode);
		node->context = this;
		node->dispatcher = dispatcher;
		node->disposed = control->GetDisposedFlag();
		node->control = control;
		node->id = nextId++;
		nodes.Add(node);
		if (dynamic_cast<GuiComboBoxListControl*>(control)) combos.Add(node);
		auto lifetime = Ptr(new WindowsUIAutomationLifetime);
		lifetime->node = node;
		control->SetInternalProperty(key, lifetime);
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
		control->TextChanged.AttachLambda([node](GuiGraphicsComposition*, GuiEventArgs&)
		{
			if (node->IsLive()) node->context->Notify(node, false, UiaDocument(node->control) ? UIA_Text_TextChangedEventId : 0);
		});
		control->VisibleChanged.AttachLambda(structure);
		control->VisuallyEnabledChanged.AttachLambda(changed);
		control->ControlTemplateChanged.AttachLambda(structure);
		control->FocusedChanged.AttachLambda([node](GuiGraphicsComposition*, GuiEventArgs&)
		{
			if (node->IsLive()) node->context->Notify(node, false, node->control->GetFocused() ? UIA_AutomationFocusChangedEventId : 0);
		});
		if (auto window = dynamic_cast<GuiWindow*>(control))
		{
			auto windowChanged = [node](EVENTID eventId)
			{
				if (!node->IsLive()) return;
				node->context->Notify(node, true, eventId);
				if (node->context->hosted && node->control != GetApplication()->GetMainWindow())
					node->context->Notify(node->context->Control(GetApplication()->GetMainWindow()), true);
			};
			window->WindowOpened.AttachLambda([windowChanged, node](GuiGraphicsComposition*, GuiEventArgs&) { windowChanged(UiaWindowEvent(node->control, true)); });
			window->WindowClosed.AttachLambda([windowChanged, node](GuiGraphicsComposition*, GuiEventArgs&) { windowChanged(UiaWindowEvent(node->control, false)); });
			window->ChildCompositionUpdated.AttachLambda([node](GuiGraphicsComposition*, GuiCompositionUpdateEventArgs& arguments)
			{
				if (!node->IsLive()) return;
				node->context->Notify(node, true);
			});
		}
		if (auto button = dynamic_cast<GuiSelectableButton*>(control))
		{
			button->SelectedChanged.AttachLambda(changed);
			button->AutoSelectionChanged.AttachLambda(changed);
			button->GroupControllerChanged.AttachLambda(changed);
		}
		if (auto menu = dynamic_cast<GuiMenuButton*>(control))
		{
			menu->SubMenuOpeningChanged.AttachLambda(structure);
			menu->ShortcutTextChanged.AttachLambda(changed);
		}
		if (auto gallery = dynamic_cast<GuiBindableRibbonGalleryList*>(control))
		{
			gallery->GetSubMenu()->WindowOpened.AttachLambda(structure);
			gallery->GetSubMenu()->WindowClosed.AttachLambda(structure);
		}
		if (auto tab = dynamic_cast<GuiTab*>(control)) tab->SelectedPageChanged.AttachLambda(structure);
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
			scroll->PositionChanged.AttachLambda(scrollChanged);
			scroll->TotalSizeChanged.AttachLambda(scrollChanged);
			scroll->PageSizeChanged.AttachLambda(scrollChanged);
		}
		if (auto list = dynamic_cast<GuiListControl*>(control))
		{
			list->GetItemProvider()->AttachCallback(lifetime.Obj());
			lifetime->columns = dynamic_cast<list::IColumnItemView*>(list->GetItemProvider()->RequestView(WString::Unmanaged(list::IColumnItemView::Identifier)));
			if (lifetime->columns) lifetime->columns->AttachCallback(lifetime.Obj());
			list->ItemTemplateChanged.AttachLambda(structure);
			list->ArrangerChanged.AttachLambda(structure);
		}
		if (auto list = dynamic_cast<GuiSelectableListControl*>(control))
		{
			list->SelectionChanged.AttachLambda([node](GuiGraphicsComposition*, GuiEventArgs&)
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
		if (auto grid = dynamic_cast<GuiVirtualDataGrid*>(control)) grid->SelectedCellChanged.AttachLambda(structure);
		if (auto date = dynamic_cast<GuiDatePicker*>(control))
		{
			date->DateChanged.AttachLambda(changed);
			date->DateNavigated.AttachLambda(structure);
		}
		if (auto document = UiaDocument(control))
		{
			document->SelectionChanged.AttachLambda([node](GuiGraphicsComposition*, GuiEventArgs&)
			{
				if (node->IsLive()) node->context->Notify(node, false, UIA_Text_TextSelectionChangedEventId);
			});
		}
		UpdateProperties(node, false);
		return node;
	}

	Ptr<WindowsUIAutomationNode> WindowsUIAutomationContext::Item(Ptr<WindowsUIAutomationNode> owner, Kind kind, vint row, vint column, Ptr<tree::INodeProvider> treeNode)
	{
		for (auto node : nodes)
		{
			if (!node->retired && node->owner == owner && node->kind == kind && node->row == row && node->column == column && node->treeNode == treeNode) return node;
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
		node->id = nextId++;
		nodes.Add(node);
		UpdateProperties(node, false);
		return node;
	}

	void WindowsUIAutomationContext::Notify(Ptr<WindowsUIAutomationNode> node, bool structure, EVENTID eventId)
	{
		if (stopped) return;
		node->structurePending |= structure;
		if (eventId && !node->pendingEvents.Contains(eventId)) node->pendingEvents.Add(eventId);
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
			for (auto child : context->nodes)
			{
				if (child != node && (child->owner == node || structure && node->IsRoot()) && child->IsLive()) context->UpdateProperties(child, true);
			}
			if (structure) UiaRaiseStructureChangedEvent(node->Provider(), StructureChangeType_ChildrenInvalidated, nullptr, 0);
			for (auto eventId : events) UiaRaiseAutomationEvent(node->Provider(), eventId);
		});
	}

	void WindowsUIAutomationContext::UpdateProperties(Ptr<WindowsUIAutomationNode> node, bool raiseEvents)
	{
		if (!node->IsLive()) return;
		const PROPERTYID watched[] =
		{
			UIA_NamePropertyId, UIA_AcceleratorKeyPropertyId, UIA_ItemStatusPropertyId, UIA_OrientationPropertyId,
			UIA_IsEnabledPropertyId, UIA_IsOffscreenPropertyId, UIA_HasKeyboardFocusPropertyId, UIA_IsContentElementPropertyId,
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
			auto value = Ptr(new WindowsUIAutomationValue);
			node->Property(property, &value->value);
			auto index = node->properties.Keys().IndexOf(property);
			if (index != -1)
			{
				auto old = node->properties.Values()[index];
				bool bothEmpty = old->value.vt == VT_EMPTY && value->value.vt == VT_EMPTY;
				if (raiseEvents && !bothEmpty && VarCmp(&old->value, &value->value, LOCALE_INVARIANT, 0) != VARCMP_EQ)
					UiaRaiseAutomationPropertyChangedEvent(node->Provider(), property, old->value, value->value);
			}
			node->properties.Set(property, value);
		}
	}

	void WindowsUIAutomationNode::Retire()
	{
		if (retired) return;
		retired = true;
		treeNode = nullptr;
		if (provider)
		{
			auto saved = provider;
			provider = nullptr;
			dispatcher->Queue([saved]() { UiaDisconnectProvider(saved.Obj()); });
		}
	}

	WindowsUIAutomationLifetime::~WindowsUIAutomationLifetime()
	{
		if (columns) columns->DetachCallback(this);
		if (items) items->DetachCallback(this);
		if (tree) tree->DetachCallback(this);
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
