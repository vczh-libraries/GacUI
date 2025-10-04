#include "TestItemProviders.h"

using namespace gacui_unittest_template;

TEST_FILE
{
	auto InitProvider = [](Ptr<TreeViewItemBindableRootProvider> provider)
	{
		provider->textProperty = BindableItem::Prop_name();
		provider->childrenProperty = BindableItem::Prop_children();
	};

	auto CreateBindableTree = [](const WString& rootName, vint childCount) -> Ptr<BindableItem>
	{
		auto root = Ptr(new BindableItem());
		root->name = rootName;
		for (vint i = 0; i < childCount; i++)
		{
			auto child = Ptr(new BindableItem());
			child->name = rootName + L".Child" + itow(i + 1);
			root->children.Add(child);
		}
		return root;
	};

	TEST_CASE(L"AttachCallback")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		provider->AttachCallback(&nodeCallback);
		
		const wchar_t* expected[] = {
			L"OnAttached(provider=valid)"
		};
		AssertCallbacks(callbackLog, expected);
	});

	TEST_CASE(L"SimpleItemAddition")
	{
		List<WString> callbackLog;
		MockNodeProviderCallback nodeCallback(callbackLog);
		
		auto provider = Ptr(new TreeViewItemBindableRootProvider());
		InitProvider(provider);
		
		auto rootItem = Ptr(new BindableItem());
		rootItem->name = L"Root Item";
		provider->SetItemSource(BoxValue(rootItem));
		provider->AttachCallback(&nodeCallback);
		
		// Prepare children to subscribe to ObservableList events
		provider->GetRootNode()->GetChildCount();
		callbackLog.Clear();
		
		// Add child
		auto childItem = Ptr(new BindableItem());
		childItem->name = L"Child Item";
		rootItem->children.Add(childItem);
		
		const wchar_t* expected[] = {
			L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
			L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
		};
		AssertCallbacks(callbackLog, expected);
	});

	TEST_CATEGORY(L"PropertyBindingSetup")
	{
		TEST_CASE(L"PropertyBindingWithSingleItem")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"TestRoot";
			provider->SetItemSource(BoxValue(rootItem));
			
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify textProperty binding works
			TEST_ASSERT(provider->GetTextValue(provider->GetRootNode().Obj()) == L"TestRoot");
		});
		
		TEST_CASE(L"PropertyBindingWithItemHavingChildren")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			auto rootItem = CreateBindableTree(L"Root", 3);
			provider->SetItemSource(BoxValue(rootItem));
			
			// SetItemSource only fires callbacks for root, children not prepared yet
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Access children to trigger preparation
			auto root = provider->GetRootNode();
			TEST_ASSERT(root->GetChildCount() == 3);
			TEST_ASSERT(provider->GetTextValue(root->GetChild(0).Obj()) == L"Root.Child1");
			TEST_ASSERT(provider->GetTextValue(root->GetChild(1).Obj()) == L"Root.Child2");
			TEST_ASSERT(provider->GetTextValue(root->GetChild(2).Obj()) == L"Root.Child3");
		});
	});

	TEST_CATEGORY(L"SetItemSourceOperations")
	{
		TEST_CASE(L"SetItemSourceWithEmptyChildren")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"EmptyRoot";
			// rootItem->children is already an empty ObservableList
			provider->SetItemSource(BoxValue(rootItem));
			
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->GetRootNode()->GetChildCount() == 0);
		});
		
		TEST_CASE(L"SetItemSourceWithMultiLevelHierarchy")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Create multi-level tree: Root -> [Child1, Child2], Child1 -> [GrandChild1, GrandChild2]
			auto rootItem = CreateBindableTree(L"Root", 2);
			auto child1 = UnboxValue<Ptr<BindableItem>>(BoxValue(rootItem->children[0]));
			auto grandChild1 = Ptr(new BindableItem());
			grandChild1->name = L"Root.Child1.GrandChild1";
			auto grandChild2 = Ptr(new BindableItem());
			grandChild2->name = L"Root.Child1.GrandChild2";
			child1->children.Add(grandChild1);
			child1->children.Add(grandChild2);
			
			provider->SetItemSource(BoxValue(rootItem));
			
			// Only root callbacks fire
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Navigate hierarchy
			auto root = provider->GetRootNode();
			TEST_ASSERT(root->GetChildCount() == 2);
			auto firstChild = root->GetChild(0);
			TEST_ASSERT(provider->GetTextValue(firstChild.Obj()) == L"Root.Child1");
			TEST_ASSERT(firstChild->GetChildCount() == 2);
			TEST_ASSERT(provider->GetTextValue(firstChild->GetChild(0).Obj()) == L"Root.Child1.GrandChild1");
			TEST_ASSERT(provider->GetTextValue(firstChild->GetChild(1).Obj()) == L"Root.Child1.GrandChild2");
		});
		
		TEST_CASE(L"SetItemSourceReplacement")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Set initial tree
			auto rootItem1 = CreateBindableTree(L"Root1", 2);
			provider->SetItemSource(BoxValue(rootItem1));
			callbackLog.Clear();
			
			// Replace with new tree
			auto rootItem2 = CreateBindableTree(L"Root2", 3);
			provider->SetItemSource(BoxValue(rootItem2));
			
			// Verify replacement callbacks
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify new tree is active
			TEST_ASSERT(provider->GetTextValue(provider->GetRootNode().Obj()) == L"Root2");
			TEST_ASSERT(provider->GetRootNode()->GetChildCount() == 3);
		});
	});

	TEST_CATEGORY(L"ObservableListIntegration")
	{
		TEST_CASE(L"ObservableListAddChild")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"Root";
			provider->SetItemSource(BoxValue(rootItem));
			
			// Prepare children by accessing
			provider->GetRootNode()->GetChildCount();
			callbackLog.Clear();
			
			// Add child to ObservableList
			auto child = Ptr(new BindableItem());
			child->name = L"NewChild";
			rootItem->children.Add(child);
			
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->GetRootNode()->GetChildCount() == 1);
			TEST_ASSERT(provider->GetTextValue(provider->GetRootNode()->GetChild(0).Obj()) == L"NewChild");
		});
		
		TEST_CASE(L"ObservableListRemoveChild")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = CreateBindableTree(L"Root", 3);
			provider->SetItemSource(BoxValue(rootItem));
			provider->GetRootNode()->GetChildCount(); // Prepare children
			callbackLog.Clear();
			
			// Remove middle child
			rootItem->children.RemoveAt(1);
			
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->GetRootNode()->GetChildCount() == 2);
			TEST_ASSERT(provider->GetTextValue(provider->GetRootNode()->GetChild(0).Obj()) == L"Root.Child1");
			TEST_ASSERT(provider->GetTextValue(provider->GetRootNode()->GetChild(1).Obj()) == L"Root.Child3");
		});
		
		TEST_CASE(L"ObservableListAddMultipleChildren")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"Root";
			provider->SetItemSource(BoxValue(rootItem));
			provider->GetRootNode()->GetChildCount();
			callbackLog.Clear();
			
			// Add multiple children one by one
			for (vint i = 1; i <= 3; i++)
			{
				auto child = Ptr(new BindableItem());
				child->name = L"Child" + itow(i);
				rootItem->children.Add(child);
			}
			
			// Each add fires separate Before/After pair
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnBeforeItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->GetRootNode()->GetChildCount() == 3);
		});
		
		TEST_CASE(L"ObservableListClearChildren")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = CreateBindableTree(L"Root", 5);
			provider->SetItemSource(BoxValue(rootItem));
			provider->GetRootNode()->GetChildCount();
			callbackLog.Clear();
			
			// Clear all children
			rootItem->children.Clear();
			
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=5, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=5, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(provider->GetRootNode()->GetChildCount() == 0);
		});
	});

	TEST_CATEGORY(L"ExpansionCollapseOperations")
	{
		TEST_CASE(L"RootNodeAlwaysExpanded")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"Root";
			provider->SetItemSource(BoxValue(rootItem));
			callbackLog.Clear();
			
			auto root = provider->GetRootNode();
			TEST_ASSERT(root->GetExpanding() == true);
			
			// Try to collapse root (should have no effect)
			root->SetExpanding(false);
			TEST_ASSERT(root->GetExpanding() == true);
			
			// No callbacks should fire
			TEST_ASSERT(callbackLog.Count() == 0);
		});
		
		TEST_CASE(L"ExpandChildNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = CreateBindableTree(L"Root", 2);
			provider->SetItemSource(BoxValue(rootItem));
			callbackLog.Clear();
			
			auto child = provider->GetRootNode()->GetChild(0);
			TEST_ASSERT(child->GetExpanding() == false);
			
			child->SetExpanding(true);
			
			const wchar_t* expected[] = {
				L"Root.Child1->OnItemExpanded()"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(child->GetExpanding() == true);
		});
		
		TEST_CASE(L"CollapseExpandedNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = CreateBindableTree(L"Root", 2);
			provider->SetItemSource(BoxValue(rootItem));
			
			auto child = provider->GetRootNode()->GetChild(0);
			child->SetExpanding(true);
			callbackLog.Clear();
			
			child->SetExpanding(false);
			
			const wchar_t* expected[] = {
				L"Root.Child1->OnItemCollapsed()"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(child->GetExpanding() == false);
		});
		
		TEST_CASE(L"ExpandCollapseCycle")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = CreateBindableTree(L"Root", 1);
			provider->SetItemSource(BoxValue(rootItem));
			callbackLog.Clear();
			
			auto child = provider->GetRootNode()->GetChild(0);
			
			// Expand -> Collapse -> Expand
			child->SetExpanding(true);
			child->SetExpanding(false);
			child->SetExpanding(true);
			
			const wchar_t* expected[] = {
				L"Root.Child1->OnItemExpanded()",
				L"Root.Child1->OnItemCollapsed()",
				L"Root.Child1->OnItemExpanded()"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(child->GetExpanding() == true);
		});
	});

	TEST_CATEGORY(L"NodeLifecycleAndRelationships")
	{
		TEST_CASE(L"ParentChildRelationships")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			// Create multi-level tree
			auto rootItem = CreateBindableTree(L"Root", 2);
			auto child1 = UnboxValue<Ptr<BindableItem>>(BoxValue(rootItem->children[0]));
			auto grandChild = Ptr(new BindableItem());
			grandChild->name = L"Root.Child1.GrandChild";
			child1->children.Add(grandChild);
			
			provider->SetItemSource(BoxValue(rootItem));
			
			// Navigate and verify parent-child relationships
			auto root = provider->GetRootNode();
			TEST_ASSERT(root->GetParent() == nullptr);
			
			auto firstChild = root->GetChild(0);
			TEST_ASSERT(firstChild->GetParent() == root);
			
			auto firstGrandChild = firstChild->GetChild(0);
			TEST_ASSERT(firstGrandChild->GetParent() == firstChild);
		});
		
		TEST_CASE(L"GetBindingValueReturnsItemSource")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = CreateBindableTree(L"Root", 2);
			provider->SetItemSource(BoxValue(rootItem));
			
			// GetBindingValue should return the original BindableItem
			auto root = provider->GetRootNode();
			auto rootValue = provider->GetBindingValue(root.Obj());
			auto retrievedRoot = UnboxValue<Ptr<BindableItem>>(rootValue);
			TEST_ASSERT(retrievedRoot == rootItem);
			TEST_ASSERT(retrievedRoot->name == L"Root");
			
			// Verify for child node too
			auto child = root->GetChild(0);
			auto childValue = provider->GetBindingValue(child.Obj());
			auto retrievedChild = UnboxValue<Ptr<BindableItem>>(childValue);
			TEST_ASSERT(retrievedChild->name == L"Root.Child1");
		});
		
		TEST_CASE(L"ChildAccessTriggersPreparation")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = CreateBindableTree(L"Root", 3);
			provider->SetItemSource(BoxValue(rootItem));
			callbackLog.Clear();
			
			// Before accessing children, they're not prepared
			// Access triggers preparation
			auto root = provider->GetRootNode();
			vint count = root->GetChildCount();
			TEST_ASSERT(count == 3);
			
			// Accessing GetChild also works
			auto child = root->GetChild(0);
			TEST_ASSERT(provider->GetTextValue(child.Obj()) == L"Root.Child1");
			
			// No callbacks fire during lazy preparation (children already existed)
			TEST_ASSERT(callbackLog.Count() == 0);
		});
	});

	TEST_CATEGORY(L"UpdateBindingProperties")
	{
		TEST_CASE(L"UpdateBindingPropertiesWithChildrenPropertyChange")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = CreateBindableTree(L"Root", 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->GetRootNode()->GetChildCount(); // Prepare children
			callbackLog.Clear();
			
			// UpdateBindingProperties with updateChildrenProperty=true
			provider->UpdateBindingProperties(true);
			
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify children still accessible
			TEST_ASSERT(provider->GetRootNode()->GetChildCount() == 2);
		});
		
		TEST_CASE(L"UpdateBindingPropertiesWithoutChildrenPropertyChange")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootItem = CreateBindableTree(L"Root", 2);
			provider->SetItemSource(BoxValue(rootItem));
			callbackLog.Clear();
			
			// UpdateBindingProperties with updateChildrenProperty=false
			provider->UpdateBindingProperties(false);
			
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=false)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});
	});

	TEST_CATEGORY(L"InterfaceMethods")
	{
		TEST_CASE(L"RequestViewReturnsITreeViewItemView")
		{
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"Root";
			provider->SetItemSource(BoxValue(rootItem));
			
			// RequestView should return valid ITreeViewItemView
			auto view = dynamic_cast<ITreeViewItemView*>(provider->RequestView(ITreeViewItemView::Identifier));
			TEST_ASSERT(view != nullptr);
			TEST_ASSERT(view == provider.Obj());
		});
		
		TEST_CASE(L"RequestViewReturnsNullForInvalidIdentifier")
		{
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"Root";
			provider->SetItemSource(BoxValue(rootItem));
			
			// Invalid identifier should return nullptr
			auto view = provider->RequestView(WString::Unmanaged(L"InvalidIdentifier"));
			TEST_ASSERT(view == nullptr);
		});
		
		TEST_CASE(L"GetTextValueUsesTextProperty")
		{
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"TestName";
			provider->SetItemSource(BoxValue(rootItem));
			
			// GetTextValue should use textProperty binding
			auto textValue = provider->GetTextValue(provider->GetRootNode().Obj());
			TEST_ASSERT(textValue == L"TestName");
			
			// Test with child
			auto child = Ptr(new BindableItem());
			child->name = L"ChildName";
			rootItem->children.Add(child);
			auto childNode = provider->GetRootNode()->GetChild(0);
			TEST_ASSERT(provider->GetTextValue(childNode.Obj()) == L"ChildName");
		});
		
		TEST_CASE(L"GetRootNodeReturnsValidNode")
		{
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"Root";
			provider->SetItemSource(BoxValue(rootItem));
			
			// GetRootNode should return valid node
			auto root = provider->GetRootNode();
			TEST_ASSERT(root != nullptr);
			TEST_ASSERT(provider->GetTextValue(root.Obj()) == L"Root");
		});
	});
}