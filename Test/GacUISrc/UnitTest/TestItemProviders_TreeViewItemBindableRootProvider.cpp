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
			
			// Expect callbacks for each operation: Clear + 3 Adds = 4 callback pairs
			const wchar_t* expected[] = {
				L"Root.Child1->OnBeforeItemModified(start=0, count=2, newCount=0, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=0, count=2, newCount=0, itemReferenceUpdated=true)",
				L"Root.Child1->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.Child1->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.Child1->OnBeforeItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.Child1->OnAfterItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
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
			
			// Expect callbacks for each operation: Clear + Add = 2 callback pairs
			const wchar_t* expected1[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
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
			
			// Expect callbacks for each operation: 2 Adds = 2 callback pairs
			const wchar_t* expected2[] = {
				L"Root.NewChild1->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.NewChild1->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.NewChild1->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"Root.NewChild1->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
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
			
			// UpdateBindingProperties only affects root level, not recursively through all descendants
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify children are still accessible (children were unprepared and re-prepared)
			TEST_ASSERT(rootNode->GetChildCount() == 2);
			TEST_ASSERT(child1->GetChildCount() == 2);
			TEST_ASSERT(child2->GetChildCount() == 2);
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
			
			// UpdateBindingProperties only affects root level, not recursively
			provider->UpdateBindingProperties(true);
			
			// Only root-level callbacks expected, not recursive through all descendants
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify all nodes still accessible after update
			TEST_ASSERT(rootNode->GetChildCount() == 2);
			TEST_ASSERT(child1->GetChildCount() == 1);
			TEST_ASSERT(grandchild1->GetChildCount() == 1);
		});
	});

	TEST_CATEGORY(L"EmptyAndNullScenarios")
	{
		TEST_CASE(L"SetItemSourceWithEmptyChildren")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			callbackLog.Clear();
			
			// Create root with explicitly empty children ObservableList
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"RootWithEmptyChildren";
			// rootItem->children is already initialized as empty ObservableList
			
			provider->SetItemSource(BoxValue(rootItem));
			
			// SetItemSource with empty children should show newCount=0 due to no children to prepare eagerly
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify root node has no children
			auto rootNode = provider->GetRootNode();
			TEST_ASSERT(rootNode->GetChildCount() == 0);
			TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"RootWithEmptyChildren");
		});

		TEST_CASE(L"ExpandNodeWithNoChildren")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			// Create root with one child that has no children
			auto rootItem = Ptr(new BindableItem());
			rootItem->name = L"Root";
			auto childItem = Ptr(new BindableItem());
			childItem->name = L"Root.EmptyChild";
			// childItem->children is already initialized as empty ObservableList
			rootItem->children.Add(childItem);
			
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			auto childNode = rootNode->GetChild(0);
			TEST_ASSERT(childNode->GetChildCount() == 0);
			callbackLog.Clear();
			
			// Expand child node with no children should complete safely
			childNode->SetExpanding(true);
			
			// Expansion callback should fire even for empty node
			const wchar_t* expected[] = {
				L"Root.EmptyChild->OnItemExpanded()"
			};
			AssertCallbacks(callbackLog, expected);
			
			TEST_ASSERT(childNode->GetExpanding() == true);
			TEST_ASSERT(childNode->GetChildCount() == 0);
		});
	});

	TEST_CATEGORY(L"SetItemSourceTransitions")
	{
		TEST_CASE(L"TransitionFromNullToHierarchy")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			// Initial state: no ItemSource set
			auto rootNode = provider->GetRootNode();
			TEST_ASSERT(rootNode->GetChildCount() == 0);
			callbackLog.Clear();
			
			// Set ItemSource to multi-level tree
			auto rootItem = CreateMultiLevelTree(L"NewRoot", 3, 2);
			provider->SetItemSource(BoxValue(rootItem));
			
			// Transition shows count=0 (was empty) to newCount=3 (eager preparation of new children)
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=3, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=3, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify new hierarchy is fully accessible
			TEST_ASSERT(rootNode->GetChildCount() == 3);
			TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"NewRoot");
		});

		TEST_CASE(L"TransitionFromHierarchyToNull")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			// Start with populated tree
			auto rootItem = CreateMultiLevelTree(L"Root", 3, 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			// Clear ItemSource
			provider->SetItemSource(Value());
			
			// Transition shows count=3 (old children) to newCount=0 (cleared)
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify root is now empty
			TEST_ASSERT(rootNode->GetChildCount() == 0);
		});

		TEST_CASE(L"TransitionComplexToSimple")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			// Start with multi-level tree (3 children, each with 2 grandchildren)
			auto rootItem1 = CreateMultiLevelTree(L"ComplexRoot", 3, 2);
			provider->SetItemSource(BoxValue(rootItem1));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			// Replace with simple flat tree (2 children, no grandchildren)
			auto rootItem2 = CreateBindableTree(L"SimpleRoot", 2);
			provider->SetItemSource(BoxValue(rootItem2));
			
			// Callbacks show old count=3 to new count=2 due to eager preparation
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=3, newCount=2, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=3, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify new structure
			TEST_ASSERT(rootNode->GetChildCount() == 2);
			TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"SimpleRoot");
			
			// Verify children have no grandchildren
			auto child1 = rootNode->GetChild(0);
			TEST_ASSERT(child1->GetChildCount() == 0);
		});

		TEST_CASE(L"TransitionSimpleToComplex")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			// Start with simple flat tree
			auto rootItem1 = CreateBindableTree(L"SimpleRoot", 2);
			provider->SetItemSource(BoxValue(rootItem1));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			// Replace with complex multi-level tree
			auto rootItem2 = CreateMultiLevelTree(L"ComplexRoot", 3, 2);
			provider->SetItemSource(BoxValue(rootItem2));
			
			// Callbacks show old count=2 to new count=3 due to eager preparation
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=3, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=3, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify new structure
			TEST_ASSERT(rootNode->GetChildCount() == 3);
			TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"ComplexRoot");
			
			// Verify children have grandchildren
			auto child1 = rootNode->GetChild(0);
			TEST_ASSERT(child1->GetChildCount() == 2);
		});

		TEST_CASE(L"RapidConsecutiveTransitions")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			
			// First transition: null -> tree1
			auto tree1 = CreateBindableTree(L"Tree1", 2);
			provider->SetItemSource(BoxValue(tree1));
			callbackLog.Clear();
			
			// Second transition: tree1 -> tree2
			auto tree2 = CreateBindableTree(L"Tree2", 3);
			provider->SetItemSource(BoxValue(tree2));
			
			// Third transition: tree2 -> tree3
			auto tree3 = CreateMultiLevelTree(L"Tree3", 2, 2);
			provider->SetItemSource(BoxValue(tree3));
			
			// Should complete without crashes, proper cleanup at each step
			TEST_ASSERT(rootNode->GetChildCount() == 2);
			TEST_ASSERT(provider->GetTextValue(rootNode.Obj()) == L"Tree3");
			
			// Verify final structure is accessible
			auto child1 = rootNode->GetChild(0);
			TEST_ASSERT(child1->GetChildCount() == 2);
		});
	});

	TEST_CATEGORY(L"ExpansionStateEdgeCases")
	{
			TEST_CASE(L"ExpansionStateWithDynamicClear")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateBindableTree(L"Root", 3);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			// Expand the root node
			rootNode->SetExpanding(true);
			TEST_ASSERT(rootNode->GetExpanding() == true);
			callbackLog.Clear();
			
			// Clear all children dynamically
			rootItem->children.Clear();
			
			// Clear() fires ONE callback pair showing count=3 to newCount=0
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Expansion state persists even with no children
			TEST_ASSERT(rootNode->GetExpanding() == true);
			TEST_ASSERT(rootNode->GetChildCount() == 0);
		});

			TEST_CASE(L"CollapseAndReexpandEmptyNode")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			// Create root with one child that has grandchildren
			auto rootItem = CreateMultiLevelTree(L"Root", 1, 3);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			auto childNode = rootNode->GetChild(0);
			childNode->GetChildCount();
			
			// Expand the child then clear its grandchildren
			childNode->SetExpanding(true);
			auto childItem = rootItem->children[0];
			childItem->children.Clear();
			callbackLog.Clear();
			
			// Collapse the now-empty child node
			childNode->SetExpanding(false);
			
			const wchar_t* collapsedExpected[] = {
				L"Root.Child1->OnItemCollapsed()"
			};
			AssertCallbacks(callbackLog, collapsedExpected);
			TEST_ASSERT(childNode->GetExpanding() == false);
			callbackLog.Clear();
			
			// Re-expand the empty child node
			childNode->SetExpanding(true);
			
			const wchar_t* expandedExpected[] = {
				L"Root.Child1->OnItemExpanded()"
			};
			AssertCallbacks(callbackLog, expandedExpected);
			TEST_ASSERT(childNode->GetExpanding() == true);
			TEST_ASSERT(childNode->GetChildCount() == 0);
		});

		TEST_CASE(L"ExpandNodeThenModifyChildren")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateBindableTree(L"Root", 2);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			
			// Expand the node
			rootNode->SetExpanding(true);
			callbackLog.Clear();
			
			// Add a child
			auto newChild = Ptr(new BindableItem());
			newChild->name = L"Root.Child3";
			rootItem->children.Add(newChild);
			
			// Each Add() fires ONE callback pair
			const wchar_t* addExpected[] = {
				L"[ROOT]->OnBeforeItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, addExpected);
			callbackLog.Clear();
			
			// Remove a child
			rootItem->children.RemoveAt(0);
			
			// RemoveAt() fires ONE callback pair
			const wchar_t* removeExpected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=1, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, removeExpected);
			
			// Expansion state maintained throughout
			TEST_ASSERT(rootNode->GetExpanding() == true);
			TEST_ASSERT(rootNode->GetChildCount() == 2);
		});
	});

	TEST_CATEGORY(L"CallbackDetachment")
	{
		TEST_CASE(L"DetachDuringTreeModification")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateBindableTree(L"Root", 3);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			// Start modification
			auto newChild = Ptr(new BindableItem());
			newChild->name = L"Root.Child4";
			rootItem->children.Add(newChild);
			callbackLog.Clear();
			
			// Detach callback
			provider->DetachCallback(&nodeCallback);
			
			// Verify detachment callback
			const wchar_t* detachExpected[] = {
				L"OnAttached(provider=null)"
			};
			AssertCallbacks(callbackLog, detachExpected);
			callbackLog.Clear();
			
			// Further modifications should not fire callbacks
			auto anotherChild = Ptr(new BindableItem());
			anotherChild->name = L"Root.Child5";
			rootItem->children.Add(anotherChild);
			
			TEST_ASSERT(callbackLog.Count() == 0);
			
			// Tree structure still valid
			TEST_ASSERT(rootNode->GetChildCount() == 5);
		});

		TEST_CASE(L"DetachAfterExpansionChanges")
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
			
			// Expand some nodes
			rootNode->SetExpanding(true);
			child1->SetExpanding(true);
			callbackLog.Clear();
			
			// Detach callback
			provider->DetachCallback(&nodeCallback);
			
			// Verify detachment
			const wchar_t* detachExpected[] = {
				L"OnAttached(provider=null)"
			};
			AssertCallbacks(callbackLog, detachExpected);
			
			// Expansion state preserved
			TEST_ASSERT(rootNode->GetExpanding() == true);
			TEST_ASSERT(child1->GetExpanding() == true);
			
			// Tree structure intact
			TEST_ASSERT(rootNode->GetChildCount() == 2);
			TEST_ASSERT(child1->GetChildCount() == 2);
		});

		TEST_CASE(L"ReattachAfterDetachment")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateBindableTree(L"Root", 2);
			provider->SetItemSource(BoxValue(rootItem));
			
			// First attach
			provider->AttachCallback(&nodeCallback);
			const wchar_t* attachExpected[] = {
				L"OnAttached(provider=valid)"
			};
			AssertCallbacks(callbackLog, attachExpected);
			callbackLog.Clear();
			
			// Detach
			provider->DetachCallback(&nodeCallback);
			const wchar_t* detachExpected[] = {
				L"OnAttached(provider=null)"
			};
			AssertCallbacks(callbackLog, detachExpected);
			callbackLog.Clear();
			
			// Reattach
			provider->AttachCallback(&nodeCallback);
			AssertCallbacks(callbackLog, attachExpected);
			
			// Verify callbacks work after reattach
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			auto newChild = Ptr(new BindableItem());
			newChild->name = L"Root.Child3";
			rootItem->children.Add(newChild);
			
			const wchar_t* modifyExpected[] = {
				L"[ROOT]->OnBeforeItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=2, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, modifyExpected);
		});

		TEST_CASE(L"DetachWithMultipleLevelsPrepared")
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
			
			// Detach with deep hierarchy
			provider->DetachCallback(&nodeCallback);
			
			const wchar_t* detachExpected[] = {
				L"OnAttached(provider=null)"
			};
			AssertCallbacks(callbackLog, detachExpected);
			
			// All nodes remain accessible
			TEST_ASSERT(rootNode->GetChildCount() == 2);
			TEST_ASSERT(child1->GetChildCount() == 1);
			TEST_ASSERT(grandchild1->GetChildCount() == 1);
		});
	});

	TEST_CATEGORY(L"InvalidOperations")
	{
		TEST_CASE(L"InvalidChildIndex")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateBindableTree(L"Root", 3);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			
			// All out-of-bounds indices return nullptr (not treated as error)
			TEST_ASSERT(rootNode->GetChild(-1) == nullptr);
			TEST_ASSERT(rootNode->GetChild(3) == nullptr);
			TEST_ASSERT(rootNode->GetChild(100) == nullptr);
			
			// Valid index should work
			auto child1 = rootNode->GetChild(0);
			TEST_ASSERT(child1 != nullptr);
			TEST_ASSERT(provider->GetTextValue(child1.Obj()) == L"Root.Child1");
		});

		TEST_CASE(L"InvalidNodeOperations")
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
			
			// All out-of-bounds indices return nullptr (not treated as error)
			TEST_ASSERT(child1->GetChild(-1) == nullptr);
			TEST_ASSERT(child1->GetChild(2) == nullptr);
			TEST_ASSERT(child1->GetChild(50) == nullptr);
			
			// Valid access should work
			auto grandchild1 = child1->GetChild(0);
			TEST_ASSERT(grandchild1 != nullptr);
			TEST_ASSERT(provider->GetTextValue(grandchild1.Obj()) == L"Root.Child1.GrandChild1");
		});
	});

	TEST_CATEGORY(L"PropertyBindingEdgeCases")
	{
		TEST_CASE(L"NullTextProperty")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = Ptr(new BindableItem());
			// name is default initialized to empty string, not null
			// This tests empty string handling
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			
			// GetTextValue should handle empty name gracefully
			auto textValue = provider->GetTextValue(rootNode.Obj());
			TEST_ASSERT(textValue == L"");
		});

		TEST_CASE(L"UpdateBindingPropertiesEdgeCases")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			// Create tree where some nodes have empty children
			auto rootItem = CreateBindableTree(L"Root", 2);
			// First child has children, second child has empty children
			auto child1Item = rootItem->children[0];
			child1Item->children.Add(Ptr(new BindableItem()));
			child1Item->children[0]->name = L"Root.Child1.GrandChild1";
			
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			auto child1 = rootNode->GetChild(0);
			child1->GetChildCount();
			auto child2 = rootNode->GetChild(1);
			child2->GetChildCount();
			callbackLog.Clear();
			
			// UpdateBindingProperties should handle mixed scenarios
			provider->UpdateBindingProperties(true);
			
			// Only root-level callbacks (not recursive)
			const wchar_t* expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=2, newCount=2, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, expected);
			
			// Verify structure intact
			TEST_ASSERT(rootNode->GetChildCount() == 2);
			TEST_ASSERT(child1->GetChildCount() == 1);
			TEST_ASSERT(child2->GetChildCount() == 0);
		});
	});

	TEST_CATEGORY(L"ObservableListOperationPatterns")
	{
		TEST_CASE(L"ClearFollowedByAdds")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateBindableTree(L"Root", 3);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			// Clear all children
			rootItem->children.Clear();
			
			// Clear() fires ONE callback pair
			const wchar_t* clearExpected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, clearExpected);
			TEST_ASSERT(rootNode->GetChildCount() == 0);
			callbackLog.Clear();
			
			// Add new children one by one - each Add() fires ONE callback pair
			auto newChild1 = Ptr(new BindableItem());
			newChild1->name = L"Root.NewChild1";
			rootItem->children.Add(newChild1);
			
			const wchar_t* add1Expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, add1Expected);
			callbackLog.Clear();
			
			auto newChild2 = Ptr(new BindableItem());
			newChild2->name = L"Root.NewChild2";
			rootItem->children.Add(newChild2);
			
			const wchar_t* add2Expected[] = {
				L"[ROOT]->OnBeforeItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=1, count=0, newCount=1, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, add2Expected);
			
			// Verify final state
			TEST_ASSERT(rootNode->GetChildCount() == 2);
			TEST_ASSERT(provider->GetTextValue(rootNode->GetChild(0).Obj()) == L"Root.NewChild1");
			TEST_ASSERT(provider->GetTextValue(rootNode->GetChild(1).Obj()) == L"Root.NewChild2");
		});

		TEST_CASE(L"RemoveRangeBoundaries")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateBindableTree(L"Root", 10);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			// RemoveRange at start
			rootItem->children.RemoveRange(0, 3);
			
			const wchar_t* removeStartExpected[] = {
				L"[ROOT]->OnBeforeItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=0, count=3, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, removeStartExpected);
			TEST_ASSERT(rootNode->GetChildCount() == 7);
			callbackLog.Clear();
			
			// RemoveRange in middle
			rootItem->children.RemoveRange(2, 2);
			
			const wchar_t* removeMiddleExpected[] = {
				L"[ROOT]->OnBeforeItemModified(start=2, count=2, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=2, count=2, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, removeMiddleExpected);
			TEST_ASSERT(rootNode->GetChildCount() == 5);
			callbackLog.Clear();
			
			// RemoveRange at end
			rootItem->children.RemoveRange(3, 2);
			
			const wchar_t* removeEndExpected[] = {
				L"[ROOT]->OnBeforeItemModified(start=3, count=2, newCount=0, itemReferenceUpdated=true)",
				L"[ROOT]->OnAfterItemModified(start=3, count=2, newCount=0, itemReferenceUpdated=true)"
			};
			AssertCallbacks(callbackLog, removeEndExpected);
			TEST_ASSERT(rootNode->GetChildCount() == 3);
		});

		TEST_CASE(L"MixedOperations")
		{
			List<WString> callbackLog;
			MockNodeProviderCallback nodeCallback(callbackLog);
			
			auto provider = Ptr(new TreeViewItemBindableRootProvider());
			InitProvider(provider);
			
			auto rootItem = CreateBindableTree(L"Root", 3);
			provider->SetItemSource(BoxValue(rootItem));
			provider->AttachCallback(&nodeCallback);
			
			auto rootNode = provider->GetRootNode();
			rootNode->GetChildCount();
			callbackLog.Clear();
			
			// Sequence: Add, RemoveAt, Add, RemoveRange, Add
			// Each operation fires exactly ONE callback pair
			
			// Add
			auto newChild1 = Ptr(new BindableItem());
			newChild1->name = L"Root.NewChild1";
			rootItem->children.Add(newChild1);
			TEST_ASSERT(callbackLog.Count() == 2);
			callbackLog.Clear();
			
			// RemoveAt
			rootItem->children.RemoveAt(0);
			TEST_ASSERT(callbackLog.Count() == 2);
			callbackLog.Clear();
			
			// Add
			auto newChild2 = Ptr(new BindableItem());
			newChild2->name = L"Root.NewChild2";
			rootItem->children.Add(newChild2);
			TEST_ASSERT(callbackLog.Count() == 2);
			callbackLog.Clear();
			
			// RemoveRange
			rootItem->children.RemoveRange(1, 2);
			TEST_ASSERT(callbackLog.Count() == 2);
			callbackLog.Clear();
			
			// Add
			auto newChild3 = Ptr(new BindableItem());
			newChild3->name = L"Root.NewChild3";
			rootItem->children.Add(newChild3);
			TEST_ASSERT(callbackLog.Count() == 2);
			
			// Verify final structure is correct
			TEST_ASSERT(rootNode->GetChildCount() == 3);
		});
	});
}
