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

	auto CreateMultiLevelTree = [&](const WString& rootName, vint level1Count, vint level2Count) -> Ptr<BindableItem>
	{
		auto root = CreateBindableTree(rootName, level1Count);
		for (vint i = 0; i < level1Count; i++)
		{
			auto child = root->children[i];
			for (vint j = 0; j < level2Count; j++)
			{
				auto grandchild = Ptr(new BindableItem());
				grandchild->name = rootName + L".Child" + itow(i + 1) + L".GrandChild" + itow(j + 1);
				child->children.Add(grandchild);
			}
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
			
			// SetItemSource eagerly prepares children, so newCount reflects child count
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=3, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=3, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify children are accessible
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
			auto child1 = rootItem->children[0];
			auto grandChild1 = Ptr(new BindableItem());
			grandChild1->name = L"Root.Child1.GrandChild1";
			auto grandChild2 = Ptr(new BindableItem());
			grandChild2->name = L"Root.Child1.GrandChild2";
			child1->children.Add(grandChild1);
			child1->children.Add(grandChild2);
			
			provider->SetItemSource(BoxValue(rootItem));
			
			// SetItemSource eagerly prepares children at root level (newCount=2)
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=2, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=2, itemReferenceUpdated=true)"
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
			
			// Verify replacement callbacks (count=2 from old tree, newCount=3 from new tree)
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=3, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=3, itemReferenceUpdated=true)"
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
			auto child1 = rootItem->children[0];
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
			
			// UpdateBindingProperties with updateChildrenProperty=true unprepares and re-prepares children
			provider->UpdateBindingProperties(true);
			
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=true)"
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
			
			// UpdateBindingProperties with updateChildrenProperty=false (children already prepared)
			provider->UpdateBindingProperties(false);
			
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)"
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

	TEST_CATEGORY(L"HierarchicalBindingScenarios")
	{
		TEST_CASE(L"ObservableListAddAtGrandchildLevel")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 1);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			// Prepare all children to level 2 to subscribe to ObservableList events
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			callbackLog.Clear();
			
			// Add grandchild to Child1's children (level 2 operation)
			auto child1Item = rootItem->children[0];
			auto newGrandchild = Ptr(new BindableItem());
			newGrandchild->name = L"Root.Child1.GrandChild2";
			child1Item->children.Add(newGrandchild);
			
			// Expect callback on Child1 node (not root), showing actual child count change
			const wchar_t* expected[] = {
				L"Root.Child1->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify tree structure updated correctly
			TEST_ASSERT(child1->GetChildCount() == 2);
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(1).Obj()) == L"Root.Child1.GrandChild2");
		});

		TEST_CASE(L"ObservableListRemoveAtGrandchildLevel")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			callbackLog.Clear();
			
			// Remove first grandchild from Child1
			auto child1Item = rootItem->children[0];
			child1Item->children.RemoveAt(0);
			
			// Expect callback showing prepared child count reduction (2 → 1)
			const wchar_t* expected[] = {
				L"Root.Child1->OnBeforeItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(child1->GetChildCount() == 1);
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(0).Obj()) == L"Root.Child1.GrandChild2");
		});

		TEST_CASE(L"ObservableListClearAtGrandchildLevel")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 3);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			callbackLog.Clear();
			
			// Clear all grandchildren from Child1
			auto child1Item = rootItem->children[0];
			child1Item->children.Clear();
			
			// Expect callback showing all 3 prepared children being removed
			const wchar_t* expected[] = {
				L"Root.Child1->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(child1->GetChildCount() == 0);
		});

		TEST_CASE(L"ObservableListAddAtGreatGrandchildLevel")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			// Create 4-level tree: Root → Child → GrandChild → GreatGrandChild
			auto rootItem = CreateMultiLevelTree(L"Root", 1, 1);
			auto child1Item = rootItem->children[0];
			auto grandchild1Item = child1Item->children[0];
			// Initially no great-grandchildren
			
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			// Prepare to level 3
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			auto grandchild1 = child1->GetChild(0);
			grandchild1->GetChildCount();
			callbackLog.Clear();
			
			// Add great-grandchild at level 3
			auto greatGrandchild = Ptr(new BindableItem());
			greatGrandchild->name = L"Root.Child1.GrandChild1.GreatGrandChild1";
			grandchild1Item->children.Add(greatGrandchild);
			
			// Expect callback on GrandChild1 node
			const wchar_t* expected[] = {
				L"Root.Child1.GrandChild1->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.Child1.GrandChild1->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(grandchild1->GetChildCount() == 1);
			TEST_ASSERT(provider->GetTextValue(grandchild1->GetChild(0).Obj()) == L"Root.Child1.GrandChild1.GreatGrandChild1");
		});

		TEST_CASE(L"ObservableListRemoveRangeAtMultipleLevels")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 3, 4);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child2 = rootNode->GetChild(1);
			child2->GetChildCount();
			callbackLog.Clear();
			
			// RemoveRange on Child2's grandchildren (remove middle 2 out of 4)
			auto child2Item = rootItem->children[1];
			child2Item->children.RemoveRange(1, 2);
			
			// Expect callback showing 2 items removed from position 1
			const wchar_t* expected[] = {
				L"Root.Child2->OnBeforeItemModified(start=1, count=2, newCount=0, itemReferenceUpdated=true)",
				L"Root.Child2->OnAfterItemModified(start=1, count=2, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(child2->GetChildCount() == 2);
			TEST_ASSERT(provider->GetTextValue(child2->GetChild(0).Obj()) == L"Root.Child2.GrandChild1");
			TEST_ASSERT(provider->GetTextValue(child2->GetChild(1).Obj()) == L"Root.Child2.GrandChild4");
		});

		TEST_CASE(L"NestedCallbackNodePathVerification")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			auto child2 = rootNode->GetChild(1);
			child2->GetChildCount();
			callbackLog.Clear();
			
			// Modify at different levels and verify node paths in callbacks
			auto child1Item = rootItem->children[0];
			auto newGrandchild = Ptr(new BindableItem());
			newGrandchild->name = L"Root.Child1.GrandChild3";
			child1Item->children.Add(newGrandchild);
			
			// First callback set should show Child1 as the node
			TEST_ASSERT(callbackLog[0] == L"Root.Child1->OnBeforeItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)");
			TEST_ASSERT(callbackLog[1] == L"Root.Child1->OnAfterItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)");
			
			callbackLog.Clear();
			
			// Now modify Child2
			auto child2Item = rootItem->children[1];
			child2Item->children.RemoveAt(0);
			
			// Second callback set should show Child2 as the node
			const wchar_t* expected[] = {
				L"Root.Child2->OnBeforeItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)",
				L"Root.Child2->OnAfterItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"AddChildrenToExpandedNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 1);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			
			// Expand child1
			child1->SetExpanding(true);
			TEST_ASSERT(child1->GetExpanding());
			callbackLog.Clear();
			
			// Add grandchild while expanded
			auto child1Item = rootItem->children[0];
			auto newGrandchild = Ptr(new BindableItem());
			newGrandchild->name = L"Root.Child1.GrandChild2";
			child1Item->children.Add(newGrandchild);
			
			const wchar_t* expected[] = {
				L"Root.Child1->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify expansion state maintained and new child visible
			TEST_ASSERT(child1->GetExpanding());
			TEST_ASSERT(child1->GetChildCount() == 2);
		});

		TEST_CASE(L"RemoveChildrenFromExpandedNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 3);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			
			// Expand child1
			child1->SetExpanding(true);
			callbackLog.Clear();
			
			// Remove grandchild while expanded
			auto child1Item = rootItem->children[0];
			child1Item->children.RemoveAt(1);
			
			const wchar_t* expected[] = {
				L"Root.Child1->OnBeforeItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=1, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify expansion state maintained and correct children remain
			TEST_ASSERT(child1->GetExpanding());
			TEST_ASSERT(child1->GetChildCount() == 2);
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(0).Obj()) == L"Root.Child1.GrandChild1");
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(1).Obj()) == L"Root.Child1.GrandChild3");
		});

		TEST_CASE(L"ClearChildrenOfExpandedNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 3);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			
			// Expand child1
			child1->SetExpanding(true);
			callbackLog.Clear();
			
			// Clear all grandchildren while expanded
			auto child1Item = rootItem->children[0];
			child1Item->children.Clear();
			
			const wchar_t* expected[] = {
				L"Root.Child1->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Expansion state should be maintained even with no children
			TEST_ASSERT(child1->GetExpanding());
			TEST_ASSERT(child1->GetChildCount() == 0);
		});

		TEST_CASE(L"ExpansionStatePersistsAcrossDynamicChanges")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			
			// Expand child1
			child1->SetExpanding(true);
			TEST_ASSERT(child1->GetExpanding());
			
			auto child1Item = rootItem->children[0];
			
			// Perform multiple operations: add, remove, add again
			callbackLog.Clear();
			auto newGC1 = Ptr(new BindableItem());
			newGC1->name = L"Root.Child1.GrandChild3";
			child1Item->children.Add(newGC1);
			TEST_ASSERT(child1->GetExpanding());
			TEST_ASSERT(child1->GetChildCount() == 3);
			
			child1Item->children.RemoveAt(0);
			TEST_ASSERT(child1->GetExpanding());
			TEST_ASSERT(child1->GetChildCount() == 2);
			
			auto newGC2 = Ptr(new BindableItem());
			newGC2->name = L"Root.Child1.GrandChild4";
			child1Item->children.Add(newGC2);
			TEST_ASSERT(child1->GetExpanding());
			TEST_ASSERT(child1->GetChildCount() == 3);
			
			// Verify final state
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(0).Obj()) == L"Root.Child1.GrandChild2");
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(1).Obj()) == L"Root.Child1.GrandChild3");
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(2).Obj()) == L"Root.Child1.GrandChild4");
		});

		TEST_CASE(L"ReplaceChildrenAtLevel2")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			callbackLog.Clear();
			
			// Replace Child1's entire children ObservableList
			auto child1Item = rootItem->children[0];
			auto gc1 = Ptr(new BindableItem());
			gc1->name = L"Root.Child1.NewGrandChild1";
			auto gc2 = Ptr(new BindableItem());
			gc2->name = L"Root.Child1.NewGrandChild2";
			auto gc3 = Ptr(new BindableItem());
			gc3->name = L"Root.Child1.NewGrandChild3";
			
			child1Item->children.Clear();
			child1Item->children.Add(gc1);
			child1Item->children.Add(gc2);
			child1Item->children.Add(gc3);
			
			// Expect callbacks showing old count (2) and new count (3)
			const wchar_t* expected[] = {
				L"Root.Child1->OnBeforeItemModified(start=0, count=2, newCount=3, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=0, count=2, newCount=3, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify new children are accessible
			TEST_ASSERT(child1->GetChildCount() == 3);
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(0).Obj()) == L"Root.Child1.NewGrandChild1");
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(2).Obj()) == L"Root.Child1.NewGrandChild3");
		});

		TEST_CASE(L"ReplaceChildrenAtMultipleLevels")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			// Replace root's children (level 1)
			auto child1 = Ptr(new BindableItem());
			child1->name = L"Root.NewChild1";
			rootItem->children.Clear();
			rootItem->children.Add(child1);
			
			const wchar_t* expected1[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected1);
			TEST_ASSERT(rootNode->GetChildCount() == 1);
			
			// Prepare new child1 and add grandchildren
			auto newChild1Node = rootNode->GetChild(0);
			newChild1Node->GetChildCount();
			callbackLog.Clear();
			
			// Replace new child1's children (level 2)
			auto gc1 = Ptr(new BindableItem());
			gc1->name = L"Root.NewChild1.GrandChild1";
			auto gc2 = Ptr(new BindableItem());
			gc2->name = L"Root.NewChild1.GrandChild2";
			child1->children.Add(gc1);
			child1->children.Add(gc2);
			
			const wchar_t* expected2[] = {
				L"Root.NewChild1->OnBeforeItemModified(start=0, count=0, newCount=2, itemReferenceUpdated=true)",
				L"Root.NewChild1->OnAfterItemModified(start=0, count=0, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected2);
			TEST_ASSERT(newChild1Node->GetChildCount() == 2);
		});

		TEST_CASE(L"ParentTextChangePreservesChildren")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			callbackLog.Clear();
			
			// Change child1's text property (not children)
			auto child1Item = rootItem->children[0];
			child1Item->name = L"Root.Child1.Modified";
			
			// No callbacks expected for children property (text-only change)
			TEST_ASSERT(callbackLog.Count() == 0);
			
			// Verify text changed but children preserved
			TEST_ASSERT(provider->GetTextValue(child1.Obj()) == L"Root.Child1.Modified");
			TEST_ASSERT(child1->GetChildCount() == 2);
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(0).Obj()) == L"Root.Child1.GrandChild1");
			TEST_ASSERT(provider->GetTextValue(child1->GetChild(1).Obj()) == L"Root.Child1.GrandChild2");
		});

		TEST_CASE(L"ParentPropertyChangeCallbackVerification")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			// Change root's text property
			rootItem->name = L"ModifiedRoot";
			
			// No children modification callbacks expected
			TEST_ASSERT(callbackLog.Count() == 0);
			TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"ModifiedRoot");
			TEST_ASSERT(rootNode->GetChildCount() == 2);
			
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			callbackLog.Clear();
			
			// Change child1's text and verify it doesn't affect its grandchildren
			auto child1Item = rootItem->children[0];
			child1Item->name = L"ModifiedChild";
			
			TEST_ASSERT(callbackLog.Count() == 0);
			TEST_ASSERT(provider->GetTextValue(child1.Obj()) == L"ModifiedChild");
			TEST_ASSERT(child1->GetChildCount() == 2);
		});

		TEST_CASE(L"UpdateBindingPropertiesAffectsAllLevels")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			auto child2 = rootNode->GetChild(1);
			child2->GetChildCount();
			callbackLog.Clear();
			
			// Call UpdateBindingProperties
			provider->UpdateBindingProperties(true);
			
			// Expect callbacks for root and all prepared children showing maintained counts
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
				L"Root.Child1->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
				L"Root.Child2->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
				L"Root.Child2->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
				L"Root.Child1->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
		});

		TEST_CASE(L"UpdateBindingPropertiesWithComplexHierarchy")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			// Create 4-level tree
			auto rootItem = CreateMultiLevelTree(L"Root", 2, 1);
			auto child1Item = rootItem->children[0];
			auto grandchild1Item = child1Item->children[0];
			auto greatGrandchild = Ptr(new BindableItem());
			greatGrandchild->name = L"Root.Child1.GrandChild1.GreatGrandChild1";
			grandchild1Item->children.Add(greatGrandchild);
			
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			// Prepare all 4 levels
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			auto grandchild1 = child1->GetChild(0);
			grandchild1->GetChildCount();
			callbackLog.Clear();
			
			// UpdateBindingProperties should affect all 4 levels
			provider->UpdateBindingProperties(true);
			
			// Verify callbacks fired at all levels with maintained counts
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)",
				L"Root.Child1->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"Root.Child1.GrandChild1->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"Root.Child1.GrandChild1->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"Root.Child1->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"Root.Child2->OnBeforeItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"Root.Child2->OnAfterItemModified(start=0, count=1, newCount=1, itemReferenceUpdated=false)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=false)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify all nodes still accessible after update
			TEST_ASSERT(rootNode->GetChildCount() == 2);
			TEST_ASSERT(child1->GetChildCount() == 1);
			TEST_ASSERT(grandchild1->GetChildCount() == 1);
		});
	});
}
